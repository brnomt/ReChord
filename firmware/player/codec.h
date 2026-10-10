/*
 * codec.h — ReChord codec interface (build-from-source codecs only).
 *
 * Every codec is a plain `codec_t` vtable plus a caller-allocated state
 * block (`state_size` bytes; the player carves it from a static pool, no
 * malloc on the target). Codecs decode to interleaved 16-bit PCM frames —
 * the format the ring buffer ships to the B core and the DAC path.
 *
 * Codec implementations must be open-source-clean or written by us
 * (docs/rewrite/architecture.md §4):
 *   - wav  : codec_wav.c      — ours, fully implemented (reference codec)
 *   - mp3  : codec_mp3_stub.c — adapter scaffold for minimp3 (to vendor in)
 *   - flac : codec_flac_stub.c — adapter scaffold for dr_flac (to vendor in)
 *
 * Return-code convention (all callbacks):
 *   >= 0  success (decode: number of PCM frames produced, 0 = end of file)
 *   <  0  error, one of the CODEC_ERR_* values below.
 */
#ifndef RECHORD_CODEC_H
#define RECHORD_CODEC_H

#include <stdint.h>

#define CODEC_OK               0
#define CODEC_ERR_IO          -1   /* read/seek failure                     */
#define CODEC_ERR_FORMAT      -2   /* malformed container/headers           */
#define CODEC_ERR_UNSUPPORTED -3   /* valid but outside the supported subset*/
#define CODEC_ERR_STATE       -4   /* bad call order / bad state pointer    */

/* Stream format report (sample-rate reporting for the sink). */
typedef struct
{
    int sample_rate;   /* Hz                                              */
    int channels;      /* 1 = mono, 2 = stereo                            */
    int bits;          /* bits per sample; our decoders output 16         */
} codec_format_t;

typedef struct codec_s
{
    const char *name;                              /* short id, e.g. "wav" */

    /* Open `path` (via the codec_io abstraction) and parse headers into
     * `st` (state_size bytes, zeroed by the caller). 0 on success. */
    int (*open)(void *st, const char *path);

    /* Decode up to `max_frames` interleaved int16 frames into `pcm`.
     * Returns frames produced, 0 at end of stream, < 0 on error. */
    int (*decode)(void *st, int16_t *pcm, int max_frames);

    /* Seek to `ms` from the start of the track (clamped to the duration).
     * 0 on success, < 0 if not seekable / error. */
    int (*seek)(void *st, long ms);

    /* Total track length in milliseconds, or < 0 if unknown. */
    long (*duration_ms)(void *st);

    /* Release any resources held by `st` (idempotent). */
    void (*close)(void *st);

    int state_size;                                /* bytes of per-instance state */

    /* Extension (ReChord, optional — may be NULL): report the decoded
     * stream format. Without it the player assumes 44100 Hz / 2 ch. */
    int (*format)(void *st, codec_format_t *out);
} codec_t;

#endif /* RECHORD_CODEC_H */
