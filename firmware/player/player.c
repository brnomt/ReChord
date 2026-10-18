/*
 * player.c — ReChord playback state machine (see player.h for semantics).
 *
 * Data path: codec.h decoder -> interleaved int32 PCM (left-justified to
 * 32 bits; see codec.h) -> ipc_b ring (ipc_b.h) -> B core. The B core mock
 * (ipc_b_mock.c) or the target IPC (ipc_b_target.c) is the audio sink;
 * nothing else in the player touches hardware, so the whole service runs in
 * host tests.
 *
 * Freestanding-friendly: no libc string/alloc — small helpers instead.
 * The codec instance state is carved from a static pool (no malloc on the
 * target).
 */
#include "player.h"
#include "codec.h"
#include "codecs/codec_registry.h"
#include "ipc_b.h"
#include "bfs.h"
#include "playlist.h"
#include "rg_math.h"
#include "tags.h"

#define PLAYER_QUEUE_LEN        8     /* next-track queue depth           */
#define PLAYER_PUMP_FRAMES      512   /* decode staging buffer (frames)   */
#define PLAYER_PUMP_MIN_FRAMES  128   /* min ring room to bother decoding */
#define PLAYER_PUMP_ROUNDS      8     /* bounded pump loop per process()  */
#define PLAYER_PATH_MAX         128
#define PLAYER_VOL_MIN_DB       (-60)
#define PLAYER_VOL_MAX_DB       20

typedef struct
{
    int             state;            /* PLAYER_*                        */
    int             track_valid;
    track_info_t    track;

    const codec_t  *codec;            /* codec of the open track          */
    int             codec_active;     /* codec->open() succeeded          */
    uint8_t         codec_mem[PLAYER_CODEC_STATE_MAX];
    codec_format_t  fmt;
    codec_gapless_t gapless;          /* delay/padding (0/0 if unaware)   */
    long            total_samples;    /* decoder frame count (for trim)   */
    long            decoded_samples;  /* frames emitted this track so far */
    long            emitted_samples;  /* frames pushed to ring (post-trim)*/
    int             leading_trimmed;  /* have we dropped the leading delay*/
    long            duration_ms;

    long            base_ms;          /* playhead at last seek/rewind     */
    long            frames_consumed;  /* cache from last ipc_b poll       */
    int             eof;              /* decoder hit end of stream        */

    int             volume_db;
    int             rg_mode;
    int32_t         rg_gain_q16;          /* cached Q16 gain for open track   */

    player_event_fn cb;
    void           *cb_user;

    char            queue[PLAYER_QUEUE_LEN][PLAYER_PATH_MAX];
    int             q_head;
    int             q_count;
    int             track_no;         /* monotonically increasing         */

    playlist_t      list;             /* BRNOMT-90 playback list          */

    int32_t         pcm[PLAYER_PUMP_FRAMES * 2];   /* staging (stereo)   */
} player_t;

static player_t g;

/* Resolve a playlist record index to a path (installed by the app; the host
 * tests install a mock). Without it a playlist cannot advance. */
static int (*s_resolve)(uint16_t idx, char *path, int cap);

/* Built-in codecs (self-describing descriptors, one file each under
 * player/codecs/: wav is ours; mp3 wraps minimp3, flac wraps dr_flac —
 * both vendored and license-pinned next to their headers, see
 * player/codecs/minimp3/LICENSE and player/codecs/dr_flac/LICENSE). */
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

/* Byte-by-byte move handles overlapping regions (used by gapless trim,
 * which shifts PCM samples toward the front of the staging buffer). */
static void mem_move(void *dst, const void *src, unsigned n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    unsigned i;

    if (d == s || n == 0)
        return;
    if (d < s) {
        for (i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        for (i = n; i > 0; i--)
            d[i - 1] = s[i - 1];
    }
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

static void recalc_rg_gain(void);   /* defined with the player API below */

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

/* ---- track info ---- */

/* Title heuristic (filename basename; ID3/Vorbis titles are a later item). */
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

/* Tag read for listings (file browser, and the scan a metadata index runs):
 * header-only, no codec state, no PCM. Zeroes `out` first, so callers can
 * use whatever the reader leaves even when it finds nothing. */
int player_read_meta(const char *path, codec_meta_t *out)
{
    if (out == 0)
        return PLAYER_ERR;
    return (tags_read_file(path, out, 0) & TAG_HAVE_META) ? PLAYER_OK
                                                          : PLAYER_ERR;
}

/* ---- track lifecycle ---- */

/* Close the current codec WITHOUT touching the B core (no ipc_b_stop).
 * Used for a gapless transition: the ring and the B-core stream must keep
 * flowing while we swap in the next codec. */
static void close_codec_only(void)
{
    if (g.codec_active && g.codec != 0)
        g.codec->close(codec_state());
    g.codec_active = 0;
    g.codec = 0;
}

static void close_track(void)
{
    close_codec_only();
    if (g.state == PLAYER_PLAYING || g.state == PLAYER_PAUSED) {
        ipc_b_stop();
        ipc_b_flush();
    }
}

/* Open `path` as the current codec and populate format/gapless/track meta.
 * Resets the per-track decode cursors but touches neither the player state
 * nor the B core — the caller decides whether to play, stop, or gapless-
 * continue. Returns PLAYER_OK or a negative code (and does not emit here;
 * TRACK is emitted by the caller so hard- and gapless-advance share it). */
static int open_codec_only(const char *path)
{
    const codec_t *c;
    codec_format_t fmt;
    int rc;

    close_codec_only();

    c = codec_registry_find(path);
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
        close_codec_only();
        do_error();
        return PLAYER_ERR;
    }
    if (fmt.channels < 1 || fmt.channels > 2 || fmt.sample_rate <= 0) {
        close_codec_only();
        do_error();
        return PLAYER_ERR;
    }
    g.fmt = fmt;

    /* Gapless: query the codec; NULL/zero means "no trimming." */
    g.gapless.delay_samples = 0;
    g.gapless.padding_samples = 0;
    if (c->gapless != 0) {
        codec_gapless_t gg = {0, 0};
        if (c->gapless(codec_state(), &gg) == 0) {
            g.gapless = gg;
        }
    }

    /* ReplayGain: query the codec's tags (NULL => no adjustment). */
    g.track.replaygain_track_gain_tenth_db = 0;
    g.track.replaygain_album_gain_tenth_db = 0;
    g.track.replaygain_track_peak_q15 = 0;
    g.track.replaygain_album_peak_q15 = 0;
    if (c->tags != 0) {
        codec_replaygain_t rg = {0, 0, 0, 0};
        if (c->tags(codec_state(), &rg) == 0) {
            g.track.replaygain_track_gain_tenth_db = rg.track_gain_tenth_db;
            g.track.replaygain_album_gain_tenth_db = rg.album_gain_tenth_db;
            g.track.replaygain_track_peak_q15 = rg.track_peak_q15;
            g.track.replaygain_album_peak_q15 = rg.album_peak_q15;
        }
    }

    g.duration_ms = (c->duration_ms != 0) ? c->duration_ms(codec_state()) : -1;
    if (g.duration_ms < 0)
        g.duration_ms = 0;

    /* Prefer the codec's exact frame count; fall back to duration*SR when the
     * codec doesn't report one (e.g. raw streams). */
    g.total_samples = g.fmt.total_samples;
    if (g.total_samples <= 0)
        g.total_samples = (long)((long long)fmt.sample_rate * g.duration_ms / 1000);

    /* Per-track decode cursors: the new track starts at its (trimmed) start. */
    g.eof = 0;
    g.decoded_samples = 0;
    g.emitted_samples = 0;
    g.leading_trimmed = (g.gapless.delay_samples == 0) ? 1 : 0;

    g.track_no++;
    g.track.track_no = g.track_no;
    g.track.duration_ms = g.duration_ms;
    g.track.gapless_delay = g.gapless.delay_samples;
    g.track.gapless_padding = g.gapless.padding_samples;
    g.track.total_samples = g.total_samples;
    g.track.bits = g.fmt.bits;
    g.track.sample_rate = g.fmt.sample_rate;
    str_copy(g.track.path, (unsigned)sizeof(g.track.path), path);
    title_from_path(g.track.title, (unsigned)sizeof(g.track.title), path);

    /* Text tags (ID3v2 / Vorbis comments): the codec parsed them with the
     * header walk it already does, so this costs no extra I/O. Tag values
     * win; the filename heuristic above stays the fallback, and a missing
     * or malformed tag simply leaves the fields empty. */
    g.track.artist[0] = '\0';
    g.track.album[0] = '\0';
    g.track.genre[0] = '\0';
    g.track.tag_track_no = 0;
    if (g.codec != 0 && g.codec->meta != 0) {
        codec_meta_t m;

        mem_zero(&m, (unsigned)sizeof(m));
        if (g.codec->meta(codec_state(), &m) == 0) {
            if (m.title[0])
                str_copy(g.track.title, (unsigned)sizeof(g.track.title),
                         m.title);
            if (m.artist[0])
                str_copy(g.track.artist, (unsigned)sizeof(g.track.artist),
                         m.artist);
            if (m.album[0])
                str_copy(g.track.album, (unsigned)sizeof(g.track.album),
                         m.album);
            if (m.genre[0])
                str_copy(g.track.genre, (unsigned)sizeof(g.track.genre),
                         m.genre);
            g.track.tag_track_no = m.track;
        }
    }
    g.track_valid = 1;

    recalc_rg_gain();   /* cache Q16 gain for the open track */
    return PLAYER_OK;
}

/* Open `path` as the current track (stops the B core first).
 * Leaves the player in PLAYER_STOPPED on success. */
static int open_track(const char *path)
{
    int rc;

    close_track();

    rc = open_codec_only(path);
    if (rc != PLAYER_OK)
        return rc;

    g.base_ms = 0;
    g.frames_consumed = 0;
    g.leading_trimmed = (g.gapless.delay_samples == 0) ? 1 : 0;

    set_state(PLAYER_STOPPED);
    emit(PLAYER_EV_TRACK);
    return PLAYER_OK;
}

/* Auto/manual advance to the next queued track.
 *
 * Gapless rule (docs/re/write/audio.md §3): on the AUTO-advance (EOF) path,
 * if the next track's format matches the current one, the B core is NOT
 * restarted — the codec is swapped with the ring kept live and
 * ipc_b_track_change() announces the new length, so adjacent tracks join
 * sample-accurately. On a format change (or a user-driven next()), a hard
 * boundary is taken: ipc_b_stop + ipc_b_start (the DAC re-locks to the new
 * rate). `hard` forces the hard path (used by player_next()). */
static int advance_to(const char *path, int hard)
{
    codec_format_t prev;

    /* Phase 3c: when Core B holds the decoder, a "gapless continuation" is not
     * possible through the shared ring — B's codec must be reopened for the new
     * file. Force the hard path (open_track + player_play -> ipc_b_start ->
     * bfs_open + OPEN) so B actually switches tracks instead of staying at EOF
     * and making the player skip ahead. */
    if (ipc_b_b_decodes())
        hard = 1;

    if (g.track_valid)
        prev = g.fmt;
    else
        prev.sample_rate = prev.channels = 0;

    if (hard || prev.sample_rate == 0 ||
        prev.sample_rate != g.fmt.sample_rate ||
        prev.channels != g.fmt.channels) {
        /* Hard boundary (format change, or a user-driven next()): stop the
         * old stream and start the new one so the B core relocks to the new
         * format. open_track + player_play() does the full restart. */
        int rc = open_track(path);
        if (rc != PLAYER_OK) {
            emit(PLAYER_EV_ERROR);
            return rc;
        }
        return player_play();
    }

    /* Seamless gapless continuation: the B core is already RUNNING. Swap in
     * the next codec WITHOUT stopping the ring (close_codec_only), then
     * announce the new length on the live stream — no ipc_b_start. */
    {
        int rc = open_codec_only(path);
        if (rc != PLAYER_OK) {
            emit(PLAYER_EV_ERROR);
            return rc;
        }
    }
    emit(PLAYER_EV_TRACK);
    ipc_b_track_change(g.duration_ms);
    set_state(PLAYER_PLAYING);
    return PLAYER_OK;
}

static int advance_queue(int hard)
{
    char path[PLAYER_PATH_MAX];

    if (queue_pop(path) != 0) {
        player_stop();
        emit(PLAYER_EV_QUEUE_EMPTY);
        return PLAYER_ERR_EMPTY;
    }
    return advance_to(path, hard);
}

/* Advance to the next playlist track (BRNOMT-90): resolve the record index to
 * a path and open it (gapless when the format matches). Returns < 0 when the
 * list is done — stop + END_OF_TRACK are already emitted. */
static int advance_playlist(void)
{
    char path[PLAYER_PATH_MAX];
    uint16_t ridx;

    if (playlist_advance(&g.list) < 0) {
        player_stop();
        emit(PLAYER_EV_END_OF_TRACK);
        return PLAYER_ERR_EMPTY;
    }
    ridx = playlist_current(&g.list);
    if (s_resolve == 0 || ridx == PLAYLIST_IDX_NONE ||
        s_resolve(ridx, path, (int)sizeof path) != 0) {
        player_stop();
        emit(PLAYER_EV_ERROR);
        return PLAYER_ERR;
    }
    return advance_to(path, 0);
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

    /* Register the built-ins keyed by their own descriptor extensions
     * (self-describing codecs; the registry in codecs/codec_registry.c). */
    player_register_codec(codec_wav.extensions, &codec_wav);
    player_register_codec(codec_mp3.extensions, &codec_mp3);
    player_register_codec(codec_flac.extensions, &codec_flac);

    return ipc_b_init();
}

int player_open(const char *path)
{
    int rc;
    if (path == 0 || path[0] == '\0')
        return PLAYER_ERR;
    playlist_reset(&g.list);        /* a single open ends any playback list */
    rc = open_track(path);
    return rc;
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
        t.bits = g.fmt.bits;
        t.duration_ms = g.duration_ms;
        t.path = g.track.path;
        t.total_samples = g.total_samples;
        t.gapless_delay = g.gapless.delay_samples;
        t.gapless_padding = g.gapless.padding_samples;
        t.rg_gain_q16 = g.rg_gain_q16;
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
    g.decoded_samples = 0;
    g.emitted_samples = 0;
    g.leading_trimmed = (g.gapless.delay_samples == 0) ? 1 : 0;

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
    /* Gapless: a seek repositions the audible stream, so the trim cursors
     * must reset — the leading delay is re-applied from the new position and
     * the trailing trim recounts from here. */
    g.decoded_samples = 0;
    g.emitted_samples = 0;
    g.leading_trimmed = (g.gapless.delay_samples == 0) ? 1 : 0;

    /* Buffered PCM is at the old position — drop it. The flush also
     * zeroes the sink-side consumed counter, keeping
     * player_get_position_ms() exact. */
    if (ipc_b_b_decodes())
        ipc_b_seek(ms);         /* B owns the decoder: reposition it too */
    else
        ipc_b_flush();
    return PLAYER_OK;
}

int player_next(void)
{
    if (playlist_count(&g.list) > 0)
        return advance_playlist();          /* BRNOMT-90: next in the list */
    return advance_queue(1);   /* user "next": hard boundary w/ B-core restart */
}

/* ---- playlist (BRNOMT-90) ---- */

void player_set_resolver(int (*fn)(uint16_t idx, char *path, int cap))
{
    s_resolve = fn;
}

void player_set_shuffle(int on)
{
    playlist_set_shuffle(&g.list, on);
}

int player_get_shuffle(void)
{
    return g.list.shuffle;
}

void player_set_repeat(int mode)
{
    playlist_set_repeat(&g.list, mode);
}

int player_get_repeat(void)
{
    return g.list.repeat;
}

int player_play_list(const uint16_t *idx, int n, int start)
{
    char path[PLAYER_PATH_MAX];
    uint16_t ridx;
    int rc;

    if (idx == 0 || n <= 0 || s_resolve == 0)
        return PLAYER_ERR;
    if (n > PLAYLIST_MAX)
        n = PLAYLIST_MAX;
    if (start < 0)
        start = 0;
    if (start >= n)
        start = n - 1;

    ridx = idx[start];
    if (s_resolve(ridx, path, (int)sizeof path) != 0)
        return PLAYER_ERR;

    rc = player_open(path);         /* clears any previous list */
    if (rc != PLAYER_OK)
        return rc;
    playlist_load(&g.list, idx, n, start);
    return player_play();
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
        /* int64 math: frames*1000 wraps a 32-bit `long` after ~49 s at
         * 44.1 kHz, which pushed the playhead negative -> the Now Playing
         * bar slammed to full and the elapsed time went huge (BRNOMT-66). */
        pos += (long)(((int64_t)g.frames_consumed * 1000) / g.fmt.sample_rate);
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

void player_set_replaygain_mode(int mode)
{
    g.rg_mode = mode;
    recalc_rg_gain();   /* gain may change mid-track when toggling modes */
}

int player_get_replaygain_mode(void)
{
    return g.rg_mode;
}

/* Effective Q16 gain for the open track under the active RG mode, or 0
 * (neutral) if RG is off or the track has no ReplayGain tags. */
int player_get_replaygain_gain_tenth_db(void)
{
    if (g.rg_mode == PLAYER_RG_OFF || !g.track_valid)
        return 0;
    if (g.rg_mode == PLAYER_RG_TRACK)
        return g.track.replaygain_track_gain_tenth_db;
    return g.track.replaygain_album_gain_tenth_db;
}

/* Recompute the cached Q16 gain for the open track from the active RG mode. */
static void recalc_rg_gain(void)
{
    int gain_tenth_db;
    int peak_q15;

    if (!g.track_valid) {
        g.rg_gain_q16 = 32768;                 /* 1.0x */
        return;
    }
    if (g.rg_mode == PLAYER_RG_TRACK) {
        gain_tenth_db = g.track.replaygain_track_gain_tenth_db;
        peak_q15 = g.track.replaygain_track_peak_q15;
    } else if (g.rg_mode == PLAYER_RG_ALBUM) {
        gain_tenth_db = g.track.replaygain_album_gain_tenth_db;
        peak_q15 = g.track.replaygain_album_peak_q15;
    } else {
        g.rg_gain_q16 = 32768;                 /* OFF: neutral 1.0x */
        return;
    }

    if (gain_tenth_db == 0 && peak_q15 == 0)
        g.rg_gain_q16 = 32768;                 /* no tags: neutral */
    else
        g.rg_gain_q16 = rg_effective_gain_q16(gain_tenth_db, peak_q15);

    /* Core B decodes and applies the gain; keep it in sync on mode changes. */
    ipc_b_set_replaygain(g.rg_gain_q16);
}

int player_get_track(track_info_t *out)
{
    if (out == 0)
        return PLAYER_ERR;
    if (!g.track_valid)
        return PLAYER_ERR_EMPTY;

    *out = g.track;
    out->playlist_pos = playlist_position(&g.list);      /* BRNOMT-90: "i/N" */
    out->playlist_count = playlist_count(&g.list);
    return PLAYER_OK;
}

int player_process(void)
{
    ipc_b_status_t st;
    int i, frame_b, want, got, filled, lead, tail_target, emit_now;
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

    if (ipc_b_b_decodes()) {
        /* Phase 3c: Core B decodes. Keep serving its file reads and end the
         * track when B's decoder drains (the ring then empties behind it). */
        bfs_poll();
        if (st.b_error) {
            do_error();
            return PLAYER_ERR;
        }
        if (st.b_eof)
            g.eof = 1;
    } else {
    frame_b = g.fmt.channels * 4;   /* int32 left-justified per sample */
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
        if (got <= 0) {
            if (got == 0)
                g.eof = 1;               /* end of stream */
            else
                do_error();          /* sets PLAYER_ERROR */
            break;
        }

        /* Gapless: drop the leading encoder-delay samples on the first
         * decoded batch. The delay is consumed first; if the batch is
         * shorter than the remaining delay, decode more (this round only). */
        lead = g.leading_trimmed ? 0 : g.gapless.delay_samples;
        if (lead > 0) {
            if (lead >= got) {
                /* entire batch was padding — keep the pump spinning */
                continue;
            }
            /* Shift the surviving (got-lead) frames to the front of g.pcm,
             * byte-wise so the int16* offset math is unambiguous. */
            {
                uint8_t *base = (uint8_t *)g.pcm;
                mem_move(base, base + (long)lead * frame_b,
                         (unsigned)((long)(got - lead) * frame_b));
            }
            got -= lead;
            g.leading_trimmed = 1;
        }

        g.decoded_samples += got;
        g.emitted_samples += got;

        /* Gapless: stop at total_samples - delay - padding (the last audible
         * sample). emitted_samples is counted AFTER the leading delay was
         * trimmed, so the audible tail is total-delay-pad frames deep. The
         * last emitted frame is exactly the last audible one => clean
         * boundary => EOF (seamless for the next track). */
        if (g.total_samples > 0) {
            tail_target = g.total_samples
                        - g.gapless.delay_samples
                        - g.gapless.padding_samples;
            if (tail_target < 0)
                tail_target = 0;
            if (g.emitted_samples >= tail_target) {
                emit_now = tail_target - (g.emitted_samples - got);
                if (emit_now < 0)
                    emit_now = 0;
                got = emit_now;
                g.eof = 1;
            }
        }

        if (got > 0) {
            /* ReplayGain: apply the track's fixed gain (Q16) to the post-
             * gapless-trim PCM before it enters the B-core ring. Neutral
             * (1.0x) when RG is off or the track carried no tags. */
            if (g.rg_gain_q16 != 32768)
                rg_scale_pcm(g.pcm, (long)got * g.fmt.channels, g.rg_gain_q16);
            filled = ipc_b_ring_fill(g.pcm, got);
            st.ring_wr += (long)filled * frame_b;   /* loop bookkeeping */
            g.emitted_samples -= (got - filled);
        }
    }

    } /* else: AP decodes (phase-2 fallback) */

    /* End of track: decoder drained AND the sink consumed everything. */
    if (g.eof && st.ring_wr == st.ring_rd) {
        if (playlist_count(&g.list) > 0) {
            int rc = advance_playlist();  /* BRNOMT-90: next in the list */
            if (rc != PLAYER_OK)
                return rc;           /* events already emitted */
        } else if (g.q_count > 0) {
            int rc = advance_queue(0);   /* auto-advance: gapless if same fmt */
            if (rc != PLAYER_OK)
                return rc;           /* events already emitted */
        } else {
            player_stop();
            emit(PLAYER_EV_END_OF_TRACK);
        }
    }
    if (g.state == PLAYER_ERROR)
        return PLAYER_ERR;           /* decode/IO error surfaced this pass */
    return PLAYER_OK;
}
