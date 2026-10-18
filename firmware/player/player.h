/*
 * player.h — ReChord audio player service (public API).
 *
 * Playback state machine (open -> play -> pause -> seek -> stop), a small
 * next-track queue, and an event/callback hook for the UI. Audio flows
 * player -> codec -> int16 PCM -> ipc_b ring -> B core (see ipc_b.h).
 *
 * Host-testable by design: the codec comes from the codec.h registry and
 * the audio sink is the ipc_b layer (ipc_b_mock.c on the host, a simulated
 * B core that consumes the ring).
 *
 * State machine (player_get_state()):
 *
 *   [none] --open--> STOPPED --play--> PLAYING --pause--> PAUSED
 *                      ^                 |  ^               |
 *                      |                 |  +-----play------+
 *                      +------stop-------+        |
 *                      ^                         v
 *                   ERROR <----(decode/B fault)---+
 *   ERROR --stop()/open()--> STOPPED
 *
 * Notes on semantics (crisp, so tests and UI agree):
 *   - open()  : loads a track (codec open, headers parsed). State STOPPED.
 *               A previous track is closed (implicit stop).
 *   - play()  : STOPPED/PAUSED -> PLAYING; from STOPPED (re)starts the B
 *               core stream; from PAUSED resumes it. Idempotent in PLAYING.
 *   - pause() : PLAYING -> PAUSED; buffered PCM is kept.
 *   - stop()  : any -> STOPPED; halts the B core, rewinds to position 0.
 *               The track stays open (stop then play restarts it).
 *   - seek()  : allowed in STOPPED/PLAYING/PAUSED; clamps to [0, duration];
 *               discards buffered PCM and repositions the codec.
 *   - next()  : pops the next-track queue: open + play. Empty queue ->
 *               stop + PLAYER_EV_QUEUE_EMPTY + PLAYER_ERR_EMPTY.
 *   - EOF     : when the codec is drained and the ring runs empty, the
 *               next queued track auto-plays (open + play); with an empty
 *               queue playback stops with PLAYER_EV_END_OF_TRACK.
 *   - decode error / B-core fault: -> PLAYER_ERROR (PLAYER_EV_ERROR);
 *               stop() or open() clears it.
 *
 * All functions return PLAYER_OK (0) or a negative PLAYER_ERR_* code.
 */
#ifndef RECHORD_PLAYER_H
#define RECHORD_PLAYER_H

#include "codec.h"   /* player_register_codec() takes codec descriptors   */

/* Longest track path the player carries (device path from the media index).
 * Matches MEDIA_PATH_MAX + 1: real libraries have paths well over 128 chars
 * (BRNOMT-92), and a truncated path opens the wrong file. */
#define PLAYER_PATH_MAX 208

typedef struct
{
    char title[64];
    char artist[64];
    char album[64];
    char genre[32];
    char path[PLAYER_PATH_MAX];
    int  track_no;              /* player's open-sequence counter (not ID3) */
    int  tag_track_no;          /* TRCK / TRACKNUMBER tag, 0 = unknown      */
    long duration_ms;

    /* Gapless trim (codec gapless query, 0/0 if not gapless-aware). The
     * player drops `gapless_delay` leading samples and `gapless_padding`
     * trailing samples when producing sample-accurate transitions. */
    int  gapless_delay;
    int  gapless_padding;
    long total_samples;         /* decoder frame count for this track       */

    /* ReplayGain (dB * 10, e.g. -600 = -60.0 dB; 0 = no adjustment).
     * Parsed from tags (docs/rewrite/audio.md §"ReplayGain"); the player
     * forwards the chosen value to the B side as the per-track gain. */
    int  replaygain_track_gain_tenth_db;
    int  replaygain_album_gain_tenth_db;
    int  replaygain_track_peak_q15;    /* 0..32767 = 0..1.0               */
    int  replaygain_album_peak_q15;

    /* Stream format of the open track (for the Now-playing line). */
    int  bits;                  /* source depth: 16/24/32 */
    long sample_rate;           /* Hz                     */

    /* Playback list (BRNOMT-90): 1-based position and total, 0 when the
     * track was opened without a list. */
    int  playlist_pos;
    int  playlist_count;
} track_info_t;

/* Player states (player_get_state()). */
enum
{
    PLAYER_STOPPED = 0,
    PLAYER_PLAYING,
    PLAYER_PAUSED,
    PLAYER_ERROR
};

/* Events delivered to the hook set by player_set_callback(). */
enum
{
    PLAYER_EV_STATE = 0,      /* any state transition                     */
    PLAYER_EV_TRACK,          /* a new track was opened (player_open/next) */
    PLAYER_EV_END_OF_TRACK,   /* playback drained and the queue is empty  */
    PLAYER_EV_QUEUE_EMPTY,    /* next requested but the queue is empty    */
    PLAYER_EV_ERROR           /* decode error or B-core fault             */
};

/* Return codes. */
#define PLAYER_OK                0
#define PLAYER_ERR              -1   /* generic failure                   */
#define PLAYER_ERR_STATE        -2   /* call not legal in current state   */
#define PLAYER_ERR_EMPTY        -3   /* no track / empty queue            */
#define PLAYER_ERR_UNSUPPORTED  -4   /* no codec for the file extension   */

/* UI event hook: called synchronously from the player API. */
typedef void (*player_event_fn)(int event, void *user);

/* ---- mandated API ---- */

int   player_init(void);
int   player_open(const char *path);
int   player_play(void);
int   player_pause(void);
int   player_stop(void);
int   player_seek(long ms);
int   player_next(void);
int   player_get_state(void);
long  player_get_position_ms(void);
int   player_set_volume(int db);
int   player_get_track(track_info_t *out);

/* ---- extensions (UI glue / host tests) ---- */

/* Register `codec` for files with extension `ext` ("wav", "mp3", ...).
 * Overrides an earlier registration for the same extension. Built-ins
 * (wav, mp3, flac) are registered by player_init(). */
int   player_register_codec(const char *ext, const codec_t *codec);

/* Append `path` to the next-track queue (FIFO, PLAYER_QUEUE_LEN deep). */
int   player_enqueue(const char *path);

/* ---- playback list (BRNOMT-90): album/artist/…/folder as a queue ---- */

/* Resolve a list record index to a path. The app installs one backed by the
 * media index (media_db_get); without it a list cannot advance. */
void  player_set_resolver(int (*fn)(uint16_t idx, char *path, int cap));

/* Play `n` record indices starting at `start` (0-based). Opens that track and
 * plays; on end-of-stream the player advances within the list. Returns
 * PLAYER_OK or a negative code. */
int   player_play_list(const uint16_t *idx, int n, int start);

/* Rockbox-style shuffle: reorders the list (current track kept playing).
 * Repeat: PLAYLIST_REPEAT_OFF/ALL/ONE (see playlist.h). */
void  player_set_shuffle(int on);
int   player_get_shuffle(void);
void  player_set_repeat(int mode);
int   player_get_repeat(void);

/* Text tags for a file WITHOUT opening it as a track — for listings (the file
 * browser reads the visible page) and for the scan a future metadata index
 * runs. Reads only the container header through codec_io, so no decoder state
 * is touched. `out` is zeroed first; returns PLAYER_OK when at least one
 * field was parsed, PLAYER_ERR otherwise (no tags, unsupported container,
 * unreadable file). */
int   player_read_meta(const char *path, codec_meta_t *out);

/* Pump one streaming step: refill the ipc_b ring from the codec and run
 * end-of-track/auto-advance bookkeeping. Call periodically (UI loop / the
 * audio task on target). No-op unless PLAYING. */
int   player_process(void);

void  player_set_callback(player_event_fn fn, void *user);
int   player_get_volume(void);

/* ---- ReplayGain ---- */

/* Which ReplayGain value the player applies as a fixed gain on the decoded
 * PCM (just before the ipc_b ring), composed with the user-set volume on
 * the B side. */
enum
{
    PLAYER_RG_OFF = 0,      /* neutral (no adjustment)                       */
    PLAYER_RG_TRACK,        /* track gain, peak-limited                       */
    PLAYER_RG_ALBUM         /* album gain, peak-limited                       */
};

/* Select the active ReplayGain mode. Default after player_init() is OFF. */
void  player_set_replaygain_mode(int mode);
int   player_get_replaygain_mode(void);

/* Effective gain the player is applying right now (dB * 10), or 0 if off.
 * Host-testable hook for verifying gain selection without decoding. */
int   player_get_replaygain_gain_tenth_db(void);

#endif /* RECHORD_PLAYER_H */
