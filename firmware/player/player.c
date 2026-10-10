/*
 * player.c — ReChord playback state machine (see player.h for semantics).
 *
 * Data path: codec.h decoder -> interleaved int16 PCM -> ipc_b ring
 * (ipc_b.h) -> B core. The B core mock (ipc_b_mock.c) or the target IPC
 * (ipc_b_target.c) is the audio sink; nothing else in the player touches
 * hardware, so the whole service runs in host tests.
 *
 * Freestanding-friendly: no libc string/alloc — small helpers instead.
 * The codec instance state is carved from a static pool (no malloc on the
 * target).
 */
#include "player.h"
#include "codec.h"
#include "ipc_b.h"

/* Per-open codec instance state pool (bytes). Must fit wav/mp3/flac state;
 * minimp3/dr_flac adapters size against this when vendored in. */
#define PLAYER_CODEC_STATE_MAX  4096

#define PLAYER_CODEC_MAX        8     /* registered codecs (ext table)    */
#define PLAYER_QUEUE_LEN        8     /* next-track queue depth           */
#define PLAYER_PUMP_FRAMES      512   /* decode staging buffer (frames)   */
#define PLAYER_PUMP_MIN_FRAMES  128   /* min ring room to bother decoding */
#define PLAYER_PUMP_ROUNDS      8     /* bounded pump loop per process()  */
#define PLAYER_PATH_MAX         128
#define PLAYER_VOL_MIN_DB       (-60)
#define PLAYER_VOL_MAX_DB       20

typedef struct
{
    char            ext[8];
    const codec_t  *codec;
} codec_slot_t;

typedef struct
{
    int             state;            /* PLAYER_*                        */
    int             track_valid;
    track_info_t    track;

    const codec_t  *codec;            /* codec of the open track          */
    int             codec_active;     /* codec->open() succeeded          */
    uint8_t         codec_mem[PLAYER_CODEC_STATE_MAX];
    codec_format_t  fmt;
    long            duration_ms;

    long            base_ms;          /* playhead at last seek/rewind     */
    long            frames_consumed;  /* cache from last ipc_b poll       */
    int             eof;              /* decoder hit end of stream        */

    int             volume_db;

    player_event_fn cb;
    void           *cb_user;

    codec_slot_t    codecs[PLAYER_CODEC_MAX];
    int             ncodecs;

    char            queue[PLAYER_QUEUE_LEN][PLAYER_PATH_MAX];
    int             q_head;
    int             q_count;
    int             track_no;         /* monotonically increasing         */

    int16_t         pcm[PLAYER_PUMP_FRAMES * 2];   /* staging (stereo)   */
} player_t;

static player_t g;

/* Built-in codecs (build-from-source: codec_wav is ours and complete;
 * mp3/flac are adapter scaffolds until their codecs are vendored in). */
extern const codec_t codec_wav;
extern const codec_t codec_mp3;
extern const codec_t codec_flac;

/* ---- tiny libc-free helpers ---- */

static void mem_zero(void *p, unsigned n)
{
    uint8_t *b = (uint8_t *)p;
    unsigned i;
    for (i = 0; i < n; i++)
        b[i] = 0;
}

static void str_copy(char *dst, unsigned cap, const char *src)
{
    unsigned i = 0;
    if (cap == 0)
        return;
    while (src[i] != '\0' && i < cap - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static char lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

static int str_eq_i(const char *a, const char *b)
{
    unsigned i = 0;
    while (lower(a[i]) == lower(b[i])) {
        if (a[i] == '\0')
            return 1;
        i++;
    }
    return 0;
}

/* ---- events / state ---- */

static void emit(int ev)
{
    if (g.cb != 0)
        g.cb(ev, g.cb_user);
}

static void set_state(int s)
{
    if (g.state != s) {
        g.state = s;
        emit(PLAYER_EV_STATE);
    }
}

static void *codec_state(void)
{
    return (void *)g.codec_mem;
}

static void do_error(void)
{
    ipc_b_stop();
    set_state(PLAYER_ERROR);
    emit(PLAYER_EV_ERROR);
}

/* ---- codec registry ---- */

static const char *ext_of(const char *path)
{
    const char *dot = 0;
    unsigned i;

    for (i = 0; path[i] != '\0'; i++) {
        if (path[i] == '.')
            dot = path + i + 1;
    }
    return dot;
}

static const codec_t *find_codec(const char *path)
{
    const char *ext = ext_of(path);
    int i;

    if (ext == 0)
        return 0;
    for (i = 0; i < g.ncodecs; i++) {
        if (str_eq_i(ext, g.codecs[i].ext))
            return g.codecs[i].codec;
    }
    return 0;
}

int player_register_codec(const char *ext, const codec_t *codec)
{
    int i;

    if (ext == 0 || codec == 0 || ext[0] == '\0')
        return PLAYER_ERR;

    for (i = 0; i < g.ncodecs; i++) {
        if (str_eq_i(ext, g.codecs[i].ext)) {
            g.codecs[i].codec = codec;   /* override */
            return PLAYER_OK;
        }
    }
    if (g.ncodecs >= PLAYER_CODEC_MAX)
        return PLAYER_ERR;
    str_copy(g.codecs[g.ncodecs].ext,
             (unsigned)sizeof(g.codecs[g.ncodecs].ext), ext);
    g.codecs[g.ncodecs].codec = codec;
    g.ncodecs++;
    return PLAYER_OK;
}

/* ---- track info ---- */

/* Title heuristic until tag parsing lands with the mp3/flac codecs:
 * the file's basename without extension. */
static void title_from_path(char *dst, unsigned cap, const char *path)
{
    unsigned base = 0, i, n;

    for (i = 0; path[i] != '\0'; i++) {
        if (path[i] == '/' || path[i] == '\\')
            base = i + 1;
    }
    n = 0;
    while (path[base + n] != '\0' && path[base + n] != '.' &&
           n < cap - 1) {
        dst[n] = path[base + n];
        n++;
    }
    dst[n] = '\0';
}

/* ---- queue ---- */

static int queue_pop(char *out)
{
    if (g.q_count == 0)
        return -1;
    str_copy(out, PLAYER_PATH_MAX, g.queue[g.q_head]);
    g.q_head = (g.q_head + 1) % PLAYER_QUEUE_LEN;
    g.q_count--;
    return 0;
}

int player_enqueue(const char *path)
{
    int idx;

    if (path == 0 || path[0] == '\0')
        return PLAYER_ERR;
    if (g.q_count >= PLAYER_QUEUE_LEN)
        return PLAYER_ERR;

    idx = (g.q_head + g.q_count) % PLAYER_QUEUE_LEN;
    str_copy(g.queue[idx], PLAYER_PATH_MAX, path);
    g.q_count++;
    return PLAYER_OK;
}

/* ---- track lifecycle ---- */

static void close_track(void)
{
    if (g.codec_active && g.codec != 0)
        g.codec->close(codec_state());
    g.codec_active = 0;
    g.codec = 0;
    if (g.state == PLAYER_PLAYING || g.state == PLAYER_PAUSED) {
        ipc_b_stop();
        ipc_b_flush();
    }
}

/* Open `path` as the current track (closes any previous one).
 * Leaves the player in PLAYER_STOPPED on success. */
static int open_track(const char *path)
{
    const codec_t *c;
    codec_format_t fmt;
    int rc;

    close_track();

    c = find_codec(path);
    if (c == 0)
        return PLAYER_ERR_UNSUPPORTED;
    if (c->state_size <= 0 || c->state_size > PLAYER_CODEC_STATE_MAX)
        return PLAYER_ERR;

    mem_zero(codec_state(), (unsigned)c->state_size);
    rc = c->open(codec_state(), path);
    if (rc < 0) {
        do_error();
        return rc;
    }

    g.codec = c;
    g.codec_active = 1;

    fmt.sample_rate = 44100;
    fmt.channels = 2;
    fmt.bits = 16;
    if (c->format != 0 && c->format(codec_state(), &fmt) < 0) {
        close_track();
        do_error();
        return PLAYER_ERR;
    }
    if (fmt.channels < 1 || fmt.channels > 2 || fmt.sample_rate <= 0) {
        close_track();
        do_error();
        return PLAYER_ERR;
    }
    g.fmt = fmt;

    g.duration_ms = (c->duration_ms != 0) ? c->duration_ms(codec_state()) : -1;
    if (g.duration_ms < 0)
        g.duration_ms = 0;

    g.base_ms = 0;
    g.frames_consumed = 0;
    g.eof = 0;

    g.track_no++;
    g.track.track_no = g.track_no;
    g.track.duration_ms = g.duration_ms;
    str_copy(g.track.path, (unsigned)sizeof(g.track.path), path);
    title_from_path(g.track.title, (unsigned)sizeof(g.track.title), path);
    g.track.artist[0] = '\0';   /* TODO: ID3/Vorbis tags (mp3/flac) */
    g.track_valid = 1;

    set_state(PLAYER_STOPPED);
    emit(PLAYER_EV_TRACK);
    return PLAYER_OK;
}

/* Auto/manual advance to the next queued track (plays it). */
static int advance_queue(void)
{
    char path[PLAYER_PATH_MAX];
    int rc;

    if (queue_pop(path) != 0) {
        player_stop();
        emit(PLAYER_EV_QUEUE_EMPTY);
        return PLAYER_ERR_EMPTY;
    }

    rc = open_track(path);
    if (rc != PLAYER_OK) {
        emit(PLAYER_EV_ERROR);
        return rc;
    }
    return player_play();
}

/* ---- public API ---- */

void player_set_callback(player_event_fn fn, void *user)
{
    g.cb = fn;
    g.cb_user = user;
}

int player_init(void)
{
    mem_zero(&g, (unsigned)sizeof(g));

    player_register_codec("wav", &codec_wav);
    player_register_codec("mp3", &codec_mp3);
    player_register_codec("flac", &codec_flac);

    return ipc_b_init();
}

int player_open(const char *path)
{
    if (path == 0 || path[0] == '\0')
        return PLAYER_ERR;
    return open_track(path);
}

int player_play(void)
{
    ipc_b_track_t t;

    if (!g.codec_active || !g.track_valid)
        return PLAYER_ERR_EMPTY;
    if (g.state == PLAYER_ERROR)
        return PLAYER_ERR_STATE;
    if (g.state == PLAYER_PLAYING)
        return PLAYER_OK;

    if (g.state == PLAYER_PAUSED) {
        ipc_b_pause(0);
    } else {
        /* STOPPED: bring the B core up for this track ("B start ... at
         * 400 MHz", then the mock/target negotiates its steady clock). */
        t.sample_rate = g.fmt.sample_rate;
        t.channels = g.fmt.channels;
        t.duration_ms = g.duration_ms;
        t.path = g.track.path;
        if (ipc_b_start(&t) < 0) {
            do_error();
            return PLAYER_ERR;
        }
        g.frames_consumed = 0;   /* per-stream counter, post-start */
    }

    set_state(PLAYER_PLAYING);
    return PLAYER_OK;
}

int player_pause(void)
{
    if (g.state != PLAYER_PLAYING)
        return PLAYER_ERR_STATE;

    ipc_b_pause(1);
    set_state(PLAYER_PAUSED);
    return PLAYER_OK;
}

int player_stop(void)
{
    if (g.state == PLAYER_STOPPED)
        return PLAYER_OK;

    ipc_b_stop();
    ipc_b_flush();

    /* Rewind semantics: stop then play restarts the track. */
    if (g.codec_active && g.codec != 0 && g.codec->seek != 0)
        g.codec->seek(codec_state(), 0);
    g.base_ms = 0;
    g.frames_consumed = 0;
    g.eof = 0;

    set_state(PLAYER_STOPPED);
    return PLAYER_OK;
}

int player_seek(long ms)
{
    int rc;

    if (!g.codec_active || !g.track_valid)
        return PLAYER_ERR_EMPTY;
    if (g.state == PLAYER_ERROR)
        return PLAYER_ERR_STATE;

    if (ms < 0)
        ms = 0;
    if (g.duration_ms > 0 && ms > g.duration_ms)
        ms = g.duration_ms;

    if (g.codec->seek != 0) {
        rc = g.codec->seek(codec_state(), ms);
        if (rc < 0)
            return rc;
    }

    g.base_ms = ms;
    g.frames_consumed = 0;
    g.eof = 0;

    /* Buffered PCM is at the old position — drop it. The flush also
     * zeroes the sink-side consumed counter, keeping
     * player_get_position_ms() exact. */
    ipc_b_flush();
    return PLAYER_OK;
}

int player_next(void)
{
    return advance_queue();
}

int player_get_state(void)
{
    return g.state;
}

long player_get_position_ms(void)
{
    long pos;

    if (!g.track_valid)
        return 0;

    pos = g.base_ms;
    if (g.fmt.sample_rate > 0)
        pos += (g.frames_consumed * 1000) / g.fmt.sample_rate;
    return pos;
}

int player_set_volume(int db)
{
    if (db < PLAYER_VOL_MIN_DB)
        db = PLAYER_VOL_MIN_DB;
    if (db > PLAYER_VOL_MAX_DB)
        db = PLAYER_VOL_MAX_DB;

    g.volume_db = db;
    return ipc_b_set_volume(db);
}

int player_get_volume(void)
{
    return g.volume_db;
}

int player_get_track(track_info_t *out)
{
    if (out == 0)
        return PLAYER_ERR;
    if (!g.track_valid)
        return PLAYER_ERR_EMPTY;

    *out = g.track;
    return PLAYER_OK;
}

int player_process(void)
{
    ipc_b_status_t st;
    int i, frame_b, want, got, filled;
    long free_bytes;

    if (g.state != PLAYER_PLAYING)
        return PLAYER_OK;

    if (ipc_b_poll_state(&st) < 0)
        return PLAYER_ERR;
    g.frames_consumed = st.frames_consumed;

    if (st.state == IPC_B_FAULT) {
        do_error();
        return PLAYER_ERR;
    }

    frame_b = g.fmt.channels * 2;
    for (i = 0; i < PLAYER_PUMP_ROUNDS; i++) {
        if (g.eof)
            break;

        free_bytes = (long)IPC_B_RING_BYTES - (st.ring_wr - st.ring_rd);
        if (free_bytes < 0)
            free_bytes = 0;
        if (free_bytes / frame_b < PLAYER_PUMP_MIN_FRAMES)
            break;

        want = (int)(free_bytes / frame_b);
        if (want > PLAYER_PUMP_FRAMES)
            want = PLAYER_PUMP_FRAMES;

        got = g.codec->decode(codec_state(), g.pcm, want);
        if (got > 0) {
            filled = ipc_b_ring_fill(g.pcm, got);
            st.ring_wr += (long)filled * frame_b;   /* loop bookkeeping */
        } else if (got == 0) {
            g.eof = 1;               /* end of stream */
        } else {
            do_error();
            return got;
        }
    }

    /* End of track: decoder drained AND the sink consumed everything. */
    if (g.eof && st.ring_wr == st.ring_rd) {
        if (g.q_count > 0) {
            int rc = advance_queue();
            if (rc != PLAYER_OK)
                return rc;           /* events already emitted */
        } else {
            player_stop();
            emit(PLAYER_EV_END_OF_TRACK);
        }
    }
    return PLAYER_OK;
}
