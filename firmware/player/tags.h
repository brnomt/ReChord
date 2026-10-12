/*
 * tags.h — minimal ReplayGain tag readers (build-from-source friendly).
 *
 * Two containers are supported, the ones our source-clean codecs use:
 *   - ID3v2 (v2.3/v2.4) TXXX frames  -> MP3  (codec_mp3_stub.c)
 *   - Vorbis comments                  -> FLAC (codec_flac_stub.c)
 *
 * Both scanners operate on the codec_io abstraction (codec_io.h), so they
 * run identically against stdio (host tests) and the storage driver
 * (target). They fill a codec_replaygain_t; callers pick track vs. album
 * gain in player.c.
 *
 * Gain values are text in dB ("-6.0 dB" / "1.5"); peaks are text in the
 * 0.0..1.0 range ("0.8323"). Parsing is deliberately lenient: it accepts
 * the canonical "RGAD"/"REPLAYGAIN_*" framing and falls back to the
 * ID3v2 RVA2 frame for MP3 if no TXXX gain is present.
 */
#ifndef RECHORD_TAGS_H
#define RECHORD_TAGS_H

#include "codec.h"   /* codec_replaygain_t                  */
#include "codec_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Scan an ID3v2 tag (v2.3/v2.4) occupying `io` (callers may leave `io`
 * positioned anywhere; this seeks to 0). Fills `out` for any keys found
 * (zero fields are left untouched). Returns 0 if at least one ReplayGain
 * value was parsed, < 0 if none / on error. */
int tags_replaygain_id3v2(codec_io_t *io, codec_replaygain_t *out);

/* Scan a Vorbis comment block at [offset, offset+len) in `io`. A FLAC
 * codec seeks to the comment metadata block and calls this. Fills `out`.
 * Returns 0 if >= 1 value parsed, < 0 otherwise. */
int tags_replaygain_vorbis(codec_io_t *io, long offset, long length,
                           codec_replaygain_t *out);

#ifdef __cplusplus
}
#endif

#endif /* RECHORD_TAGS_H */
