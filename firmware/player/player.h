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

typedef struct
{
    char title[64];
    char artist[64];
    char path[128];
    int  track_no;
    long duration_ms;
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

/* Pump one streaming step: refill the ipc_b ring from the codec and run
 * end-of-track/auto-advance bookkeeping. Call periodically (UI loop / the
 * audio task on target). No-op unless PLAYING. */
int   player_process(void);

void  player_set_callback(player_event_fn fn, void *user);
int   player_get_volume(void);

#endif /* RECHORD_PLAYER_H */
