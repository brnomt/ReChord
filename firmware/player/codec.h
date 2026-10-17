/*
 * codec.h — ReChord codec interface (build-from-source codecs only).
 *
 * CODEC ABSTRACTION LAYER (CAL): every codec is a plain `codec_t` vtable
 * plus a caller-allocated state block (`state_size` bytes; the player
 * carves it from a static pool, no malloc on the target). Codecs decode to
 * interleaved 16-bit PCM frames — the standardized pipeline contract (this
 * is the "raw interleaved PCM" boundary: sample_rate / bits / channels are
 * reported via codec_format_t). Codecs are PURE SOFTWARE TRANSFORMS: no
 * I2C registers, no I2S clocks, no DMA channels, no hardware drivers —
 * all I/O goes through the codec_io abstraction.
 *
 * Vtable mapping to the canonical codec interface names (documented
 * contract; the shorter names are kept so codecs/tests stay stable):
 *   init()                 -> codec_t.init   (optional; one-time setup)
 *   open(file)             -> codec_t.open
 *   get_format(sr,bits,ch) -> codec_t.format (codec_format_t carries all
 *                             three: sample_rate, bits, channels)
 *   decode_frame(pcm, max) -> codec_t.decode (returns frames produced)
 *   close()                -> codec_t.close
 * plus optional seek/duration_ms/gapless/tags extensions.
 *
 * Codec implementations must be open-source-clean or written by us
 * (docs/rewrite/architecture.md §4):
 *   - wav  : codecs/wav.c      — ours (reference codec)
 *   - mp3  : codecs/mp3.c      — adapter over vendored minimp3
 *   - flac : codecs/flac.c     — adapter over vendored dr_flac
 *   - opus : RESERVED SLOT (docs/rewrite/player.md) — drop-in codec_t in
 *            codecs/opus/ when ported; no core changes required.
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

/* Per-open codec instance state pool (bytes) — the player carves this from
 * its static codec_mem and zeroes it before open(). Adapters must fit
 * their state (and static_assert it) to this budget; large per-track
 * buffers (minimp3 residual/feed, dr_flac window) live as file-statics in
 * their adapters instead — there is one open track at a time. Sized:
 * mp3dec_t is 6668 B + adapter ctx. Static pool — no malloc on target. */
#define PLAYER_CODEC_STATE_MAX  7296   /* +128 for the extended tag fields (year/label/album artist/composer) */

/* ---- codec format + gapless ---- */

/* Stream format report (sample-rate reporting for the sink). */
typedef struct
{
    int sample_rate;   /* Hz                                              */
    int channels;      /* 1 = mono, 2 = stereo, interleaved                */
    int bits;          /* bits per sample; our decoders output 16           */
    long total_samples; /* exact decoder sample count (0 = unknown)         */
} codec_format_t;

/* Optional gapless info returned by codec_t::gapless(). NULL == not
 * gapless-aware (delay/padding treated as 0: a hard boundary between
 * tracks). delay_samples = leading samples to drop from the first decoded
 * frame (encoder delay); padding_samples = trailing samples to drop at
 * end-of-stream (encoder padding / encoder-tail). Used by the player to
 * trim PCM so adjacent tracks join sample-accurately with no silence.
 */
typedef struct
{
    int delay_samples;
    int padding_samples;
} codec_gapless_t;

/* Optional ReplayGain from tags. Gain is stored in dB * 10 (so -600 =
 * -60.0 dB, matching track_info_t). Peak is a fraction in Q1.15
 * (32767 = 1.0). A NULL tags() callback, or one returning 0/0, means
 * "no ReplayGain" (no adjustment, clip-prevention off). */
typedef struct
{
    int track_gain_tenth_db;
    int album_gain_tenth_db;
    int track_peak_q15;
    int album_peak_q15;
} codec_replaygain_t;

/* Text metadata from the container's tag block (ID3v2 text frames for mp3,
 * Vorbis comments for flac; see tags.h). Every field is NUL-terminated and
 * EMPTY when the file carries no such tag — "no tags" is not an error, it
 * just leaves the fallbacks (filename for the title) in charge. `track` is
 * the tag track number, 0 = unknown. */
typedef struct
{
    char title[64];
    char artist[64];
    char album[64];
    char genre[32];
    char album_artist[24];
    char composer[24];
    char label[16];
    int  track;
    int  year;      /* 0 = unknown/invalid (the UI shows "Undefined") */
} codec_meta_t;

typedef struct codec_s
{
    const char *name;                              /* short id, e.g. "wav" */
    /* Primary file extension this codec dispatches on (registry key,
     * lower-case, no dot). Kept in the descriptor so a codec is fully
     * self-describing: drop its .c from the build and nothing else
     * references it (conditional compilation, Makefile-level). */
    const char *extensions;

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

    /* Extension (ReChord, optional — may be NULL): report gapless trim info.
     * delay_samples = leading samples (encoder delay) to drop from the first
     * decoded frame; padding_samples = trailing samples to drop at end of
     * stream. A NULL query, or one returning 0/0, means "no trimming" and
     * the player will still produce a hard boundary between tracks. */
    int (*gapless)(void *st, codec_gapless_t *out);

    /* Extension (ReChord, optional — may be NULL): report ReplayGain tags.
     * The player picks one of track/album gain (see player_set_replaygain_mode)
     * and applies it as a fixed gain before the PCM ring, composed with the
     * user volume on the B side. */
    int (*tags)(void *st, codec_replaygain_t *out);

    /* Extension (ReChord, optional — may be NULL): report the text metadata
     * parsed at open() (see codec_meta_t). Like tags(), always succeeds:
     * absent fields stay empty, the caller decides on the fallbacks. The
     * parse happens inside the existing tag/header pass, so this costs no
     * extra I/O. */
    int (*meta)(void *st, codec_meta_t *out);

    /* Extension (ReChord, optional — may be NULL): one-time codec-global
     * initialization, called once at player_register_codec(). 0 = ok.
     * Place for lookup-table setup (e.g. a future fixed-point Opus port);
     * NULL = no-op. Must not touch hardware. */
    int (*init)(void);
} codec_t;

#endif /* RECHORD_CODEC_H */
