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
/* PLAYER_PATH_MAX is the public path capacity (player.h) — the queues, the
 * resolver call and track_info_t.path share it. */
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
    long            frames_at_start;  /* frames_consumed when this track  */
                                      /* started (gapless: the counter is */
                                      /* NOT reset, so position is relative)*/
    int             eof;              /* decoder hit end of stream        */
    int             restart_pending;  /* deferred hard restart (format    */
                                      /* change on Core B): wait for the  */
                                      /* ring to drain, then stop/start   */
    int             preload_done;     /* next-file pre-open already tried */

    /* Next track opened ahead of the boundary (BRNOMT-73). While `pending_valid`
     * the codec in codec_mem IS the next track's, but g.track/g.fmt keep
     * describing the PLAYING one so the UI does not jump early. advance_to()
     * adopts pending_* when its path matches; close_codec_only() drops it. */
    int             pending_valid;
    track_info_t    pending_track;
    codec_format_t  pending_fmt;
    codec_gapless_t pending_gapless;
    long            pending_total;
    long            pending_duration;
    int32_t         pending_rg_gain;

    /* Next record index resolved during the pre-open window (BRNOMT-73): the
     * resolver is a media-DB lookup (~47 ms on the target) and must not run
     * inside the ring's cover at the boundary. Reused only when the index the
     * playlist actually lands on matches. */
    int             peek_valid;
    uint16_t        peek_ridx;
    char            peek_path[PLAYER_PATH_MAX];

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

static int str_eq(const char *a, const char *b)
{
    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
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
    g.pending_valid = 0;   /* the open codec is gone: no pending metadata */
}

static void close_track(void)
{
    close_codec_only();
    g.peek_valid = 0;               /* a new track invalidates the resolution */
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
    ipc_b_set_prescan(0);        /* a new open invalidates the old geometry */

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
    g.preload_done = 0;

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

    /* Share what this open learned with Core B, so its re-open of the same
     * file can size its decode pool without repeating the header walk
     * (BRNOMT-73: that walk is inside the gapless boundary). */
    if (c->prescan_info != 0) {
        codec_prescan_t pi;

        mem_zero(&pi, (unsigned)sizeof pi);
        if (c->prescan_info(codec_state(), &pi) == 1)
            ipc_b_set_prescan(&pi);
        else
            ipc_b_set_prescan(0);
    } else {
        ipc_b_set_prescan(0);   /* codec has nothing to share */
    }
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
    g.frames_at_start = 0;
    g.leading_trimmed = (g.gapless.delay_samples == 0) ? 1 : 0;

    set_state(PLAYER_STOPPED);
    emit(PLAYER_EV_TRACK);
    return PLAYER_OK;
}

/* Build the ipc_b track descriptor for the currently-open track (g.fmt /
 * g.gapless / g.rg_gain_q16). Shared by player_play() and the gapless
 * hand-off so both describe a track identically. */
static void build_ipc_track(ipc_b_track_t *t)
{
    t->sample_rate = g.fmt.sample_rate;
    t->channels = g.fmt.channels;
    t->bits = g.fmt.bits;
    t->duration_ms = g.duration_ms;
    t->path = g.track.path;
    t->total_samples = g.total_samples;
    t->gapless_delay = g.gapless.delay_samples;
    t->gapless_padding = g.gapless.padding_samples;
    t->rg_gain_q16 = g.rg_gain_q16;
}

/* Peek the next track's device path WITHOUT consuming the queue/list.
 * Returns 1 when there is one, 0 otherwise. Used to pre-open the next file
 * while the current track still plays (BRNOMT-73). */
static int next_path_peek(char *out, int cap)
{
    char p[PLAYER_PATH_MAX];

    if (playlist_count(&g.list) > 0) {
        uint16_t ridx;

        if (!playlist_peek_next(&g.list, &ridx) || ridx == PLAYLIST_IDX_NONE)
            return 0;
        if (s_resolve == 0 || s_resolve(ridx, p, (int)sizeof p) != 0)
            return 0;
        /* Remember the resolution: the boundary must not repeat the lookup. */
        g.peek_ridx = ridx;
        str_copy(g.peek_path, (unsigned)sizeof g.peek_path, p);
        g.peek_valid = 1;
        str_copy(out, (unsigned)cap, p);
        return 1;
    }
    if (g.q_count > 0) {
        str_copy(out, (unsigned)cap, g.queue[g.q_head]);
        return 1;
    }
    return 0;
}

/* Open the NEXT track's codec during the pre-open window so the gapless
 * boundary pays no AP-side SD open + header parse (BRNOMT-73: field log showed
 * ~84 ms between B's EOF and ipc_b_next, which pushed the boundary past the
 * ring's ~180 ms of cover). The opened codec (codec_mem / g.codec) belongs to
 * the NEXT track while g.track/g.fmt keep describing the PLAYING one, so the
 * Now Playing screen does not jump ~3 s early. advance_to() adopts the pending
 * metadata when the path matches; any other codec activity drops it (its first
 * act is close_codec_only(), which clears pending_valid). */
static int preopen_next_codec(const char *path)
{
    track_info_t    cur_track = g.track;
    codec_format_t  cur_fmt = g.fmt;
    codec_gapless_t cur_gap = g.gapless;
    long cur_total = g.total_samples;
    long cur_dur = g.duration_ms;
    int32_t cur_rg = g.rg_gain_q16;
    int cur_track_no = g.track_no;
    int cur_valid = g.track_valid;
    long cur_base = g.base_ms;
    long cur_cons = g.frames_consumed;
    long cur_at = g.frames_at_start;
    long cur_dec = g.decoded_samples;
    long cur_emit = g.emitted_samples;
    int cur_lead = g.leading_trimmed;
    int cur_eof = g.eof;
    int cur_pl = g.preload_done;
    int ok;

    ok = (open_codec_only(path) == PLAYER_OK);
    if (ok) {
        /* g now describes the NEXT track, and codec_mem holds its decoder. */
        g.pending_track = g.track;
        g.pending_fmt = g.fmt;
        g.pending_gapless = g.gapless;
        g.pending_total = g.total_samples;
        g.pending_duration = g.duration_ms;
        g.pending_rg_gain = g.rg_gain_q16;
        g.pending_valid = 1;
    }

    /* Restore everything that describes the PLAYING track (open_codec_only
     * resets the cursors, bumps track_no and clears preload_done). On failure
     * the probe also closed the playing codec, so bring it back. */
    if (!ok && cur_valid)
        (void)open_codec_only(cur_track.path);

    g.track = cur_track;
    g.track_valid = cur_valid;
    g.fmt = cur_fmt;
    g.gapless = cur_gap;
    g.total_samples = cur_total;
    g.duration_ms = cur_dur;
    g.rg_gain_q16 = cur_rg;
    g.track_no = cur_track_no;
    g.base_ms = cur_base;
    g.frames_consumed = cur_cons;
    g.frames_at_start = cur_at;
    g.decoded_samples = cur_dec;
    g.emitted_samples = cur_emit;
    g.leading_trimmed = cur_lead;
    g.eof = cur_eof;
    g.preload_done = cur_pl;
    return ok ? 0 : -1;
}

/* Hard boundary for the ALREADY-OPEN codec: stop the B core / drain the ring,
 * reset the per-track cursors and start the stream fresh (player_play() ->
 * ipc_b_start(), which re-locks the DAC to g.fmt). Used for a format change
 * and for a user-driven next(). */
static int restart_stream(void)
{
    ipc_b_stop();
    ipc_b_flush();

    g.base_ms = 0;
    g.frames_consumed = 0;
    g.frames_at_start = 0;
    g.eof = 0;
    g.decoded_samples = 0;
    g.emitted_samples = 0;
    g.leading_trimmed = (g.gapless.delay_samples == 0) ? 1 : 0;
    g.restart_pending = 0;

    set_state(PLAYER_STOPPED);   /* player_play() needs a non-PLAYING state */
    return player_play();
}

/* Advance to the next track (BRNOMT-73: gapless on Core B).
 *
 * The next codec is opened FIRST so the two formats can be compared (the old
 * codec is closed; every path below plays the new track). Then:
 *   - format change (or a user-driven next()): hard boundary. When Core B
 *     holds the decoder the stop/start is DEFERRED to the ring-empty handler
 *     (restart_pending) so the previous track's tail plays out first — a new
 *     format cannot continue on the old DAC stream, but it must not truncate.
 *   - same format, Core B decoding: ipc_b_next() reopens B's codec on the LIVE
 *     PCM ring (bfs_open + OPEN) — the ring cursors and the DAC are untouched,
 *     so the boundary is sample-accurate.
 *   - same format, AP decoding (host mock / B fallback): ipc_b_track_change()
 *     extends the live stream length (the phase-2 gapless path). */
static int advance_to(const char *path, int hard)
{
    codec_format_t prev;
    int prev_valid = g.track_valid;
    int same_fmt;
    int rc;

    g.peek_valid = 0;               /* the peek only serves one advance */
    if (prev_valid)
        prev = g.fmt;

    if (g.pending_valid && str_eq(g.pending_track.path, path)) {
        /* The codec was opened during the pre-open window: adopt its metadata
         * without touching storage (this is the whole point of BRNOMT-73's
         * pending open — no SD open / header parse on the boundary). */
        g.pending_valid = 0;
        g.track = g.pending_track;
        g.track_valid = 1;
        g.fmt = g.pending_fmt;
        g.gapless = g.pending_gapless;
        g.total_samples = g.pending_total;
        g.duration_ms = g.pending_duration;
        g.rg_gain_q16 = g.pending_rg_gain;
        g.track_no = g.pending_track.track_no;
        g.eof = 0;
        g.decoded_samples = 0;
        g.emitted_samples = 0;
        g.leading_trimmed = (g.gapless.delay_samples == 0) ? 1 : 0;
        g.preload_done = 0;
        rc = PLAYER_OK;
    } else {
        /* No (matching) pending codec: the first thing open_codec_only() does
         * is close the current one, which also drops a stale pending. */
        rc = open_codec_only(path);
        if (rc != PLAYER_OK) {
            emit(PLAYER_EV_ERROR);
            return rc;
        }
    }
    same_fmt = prev_valid &&
               prev.sample_rate == g.fmt.sample_rate &&
               prev.channels    == g.fmt.channels &&
               prev.bits        == g.fmt.bits;

    if (hard || !same_fmt) {
        emit(PLAYER_EV_TRACK);
        if (ipc_b_b_decodes() && !hard) {
            g.restart_pending = 1;   /* let the tail drain, then stop/start */
            return PLAYER_OK;
        }
        return restart_stream();
    }

    if (ipc_b_b_decodes()) {
        ipc_b_track_t t;

        /* Hand off BEFORE announcing: ipc_b_next() pumps bfs_poll() while Core
         * B reopens from the pre-filled ring, so an app event that touches the
         * index or the UI must not sit between B's EOF and its reopen
         * (BRNOMT-73: that gap is inside the ring's cover budget). */
        build_ipc_track(&t);
        if (ipc_b_next(&t) != 0) {
            emit(PLAYER_EV_TRACK);
            return restart_stream();     /* B refused: fall back to hard */
        }
        emit(PLAYER_EV_TRACK);
    } else {
        emit(PLAYER_EV_TRACK);
        ipc_b_track_change(g.duration_ms);
    }
    g.frames_at_start = g.frames_consumed;   /* per-track position base */
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
 * list is done — stop + END_OF_TRACK are already emitted. `hard` forces a hard
 * boundary (a user-driven next()). */
static int advance_playlist(int hard)
{
    char path[PLAYER_PATH_MAX];
    uint16_t ridx;

    if (playlist_advance(&g.list) < 0) {
        player_stop();
        emit(PLAYER_EV_END_OF_TRACK);
        return PLAYER_ERR_EMPTY;
    }
    ridx = playlist_current(&g.list);
    if (g.peek_valid && g.peek_ridx == ridx) {
        /* Resolved during the pre-open window: skip the media-DB lookup, which
         * would otherwise run inside the ring's cover (BRNOMT-73). */
        str_copy(path, (unsigned)sizeof path, g.peek_path);
    } else if (s_resolve == 0 || ridx == PLAYLIST_IDX_NONE ||
               s_resolve(ridx, path, (int)sizeof path) != 0) {
        g.peek_valid = 0;
        player_stop();
        emit(PLAYER_EV_ERROR);
        return PLAYER_ERR;
    }
    g.peek_valid = 0;
    return advance_to(path, hard);
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
        build_ipc_track(&t);
        if (ipc_b_start(&t) < 0) {
            do_error();
            return PLAYER_ERR;
        }
        g.frames_consumed = 0;   /* per-stream counter, post-start */
        g.frames_at_start = 0;
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

    if (g.pending_valid) {
        /* The open codec is the pre-opened NEXT track: restore the playing
         * track's codec so the rewind below acts on the right stream. */
        int tn = g.track_no;

        if (open_codec_only(g.track.path) == PLAYER_OK)
            g.track_no = tn;
    }

    /* Rewind semantics: stop then play restarts the track. */
    if (g.codec_active && g.codec != 0 && g.codec->seek != 0)
        g.codec->seek(codec_state(), 0);
    g.base_ms = 0;
    g.frames_consumed = 0;
    g.frames_at_start = 0;
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

    if (g.pending_valid) {
        /* The open codec is the pre-opened NEXT track: bring the playing one
         * back before repositioning it (BRNOMT-73 pending open). */
        int tn = g.track_no;

        if (open_codec_only(g.track.path) != PLAYER_OK)
            return PLAYER_ERR;
        g.track_no = tn;
    }

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
    g.frames_at_start = 0;
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
        return advance_playlist(1);         /* BRNOMT-90: next in the list */
    return advance_queue(1);   /* user "next": hard boundary w/ B-core restart */
}

int player_prev(void)
{
    char path[PLAYER_PATH_MAX];
    uint16_t ridx;

    if (playlist_count(&g.list) <= 0 || playlist_prev_manual(&g.list) < 0)
        return PLAYER_ERR_EMPTY;   /* the legacy queue has no history */

    ridx = playlist_current(&g.list);
    if (s_resolve == 0 || ridx == PLAYLIST_IDX_NONE ||
        s_resolve(ridx, path, (int)sizeof path) != 0) {
        emit(PLAYER_EV_ERROR);
        return PLAYER_ERR;
    }
    return advance_to(path, 1);    /* user-driven: hard boundary */
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
    if (g.fmt.sample_rate > 0) {
        /* Count from this track's start: on a gapless Core-B boundary the
         * sink's frames counter is NOT reset (the stream is continuous), so
         * frames_at_start carries the baseline for the new track. */
        int64_t frames = (int64_t)g.frames_consumed - (int64_t)g.frames_at_start;

        if (frames < 0)
            frames = 0;
        /* int64 math: frames*1000 wraps a 32-bit `long` after ~49 s at
         * 44.1 kHz, which pushed the playhead negative -> the Now Playing
         * bar slammed to full and the elapsed time went huge (BRNOMT-66). */
        pos += (long)((frames * 1000) / g.fmt.sample_rate);
    }
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
        /* Phase 3c: Core B decodes. Keep serving its file reads. When B's
         * decoder drains, hand the next track off WHILE THE RING STILL HOLDS
         * THE TAIL (~85 ms) so the boundary is sample-accurate — do NOT wait
         * for the ring to empty (that was the 2-3 s hard boundary). A format
         * change defers its hard restart to the ring-empty handler
         * (restart_pending) so the previous tail is not truncated. */
        if (st.b_error) {
            do_error();
            return PLAYER_ERR;
        }
        if (st.b_eof && !g.eof) {
            g.eof = 1;
            if (playlist_count(&g.list) > 0) {
                if (playlist_has_next(&g.list)) {
                    int rc = advance_playlist(0);
                    if (rc != PLAYER_OK)
                        return rc;      /* events already emitted */
                    return PLAYER_OK;  /* gapless (or deferred) hand-off */
                }
            } else if (g.q_count > 0) {
                int rc = advance_queue(0);
                if (rc != PLAYER_OK)
                    return rc;
                return PLAYER_OK;
            }
            /* No next track: let the ring drain -> END_OF_TRACK below. */
        }

        /* Refill Core B's file ring. Kept AFTER the EOF hand-off because that
         * hand-off pumps bfs_poll() itself while B reopens — on the boundary
         * pass the SD read must not sit between B's EOF and its reopen
         * (BRNOMT-73: it eats the ring's cover budget). */
        bfs_poll();

        /* BRNOMT-73: ~3 s before the end, pre-open the next file into the
         * other ring so the boundary does not pay the SD open. Once per
         * track; a stale pre-open is rejected at the boundary. */
        if (!g.preload_done && !g.eof && g.duration_ms > 0) {
            long pos = player_get_position_ms();

            if (g.duration_ms - pos <= 3000) {
                char np[PLAYER_PATH_MAX];

                g.preload_done = 1;
                if (next_path_peek(np, (int)sizeof np)) {
                    (void)ipc_b_preopen(np);       /* B's next-file ring    */
                    (void)preopen_next_codec(np);  /* AP's pending codec    */
                }
            }
        }
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

    /* Final boundary: the decoder drained AND the sink consumed everything
     * (AP-decode path), or a deferred Core-B hard restart is due. */
    if ((g.eof || g.restart_pending) && st.ring_wr == st.ring_rd) {
        if (g.restart_pending)
            return restart_stream();     /* format change: drain done */
        if (playlist_count(&g.list) > 0) {
            int rc = advance_playlist(0);
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
