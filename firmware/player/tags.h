/*
 * tags.h — container tag readers (ID3v2 text + Vorbis comments).
 *
 * Two containers are supported, the ones our source-clean codecs use:
 *   - ID3v2 (v2.3/v2.4) frames -> MP3
 *   - Vorbis comment block     -> FLAC
 *
 * Every reader works on the codec_io abstraction (codec_io.h), so the same
 * code runs against stdio (host tests) and the storage driver (target). One
 * pass fills both output structs: the text metadata (title/artist/album/
 * genre/track) and the ReplayGain values.
 *
 * Bounded by construction: no allocation, the tag region is limited by the
 * container's own declared size, every field is truncated to its
 * destination capacity, and oversized frames/entries are skipped rather
 * than buffered. A malformed tag is never fatal — the reader reports "no
 * fields found" and the caller falls back to the filename.
 */
#ifndef RECHORD_TAGS_H
#define RECHORD_TAGS_H

#include "codec.h"   /* codec_meta_t, codec_replaygain_t */
#include "codec_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* What a reader found (bit flags). 0 = nothing usable / malformed. */
#define TAG_HAVE_META 1   /* at least one text field was filled */
#define TAG_HAVE_RG   2   /* at least one ReplayGain field was filled */

/* Read an ID3v2.3/2.4 tag occupying `io` (callers may leave `io` anywhere;
 * this seeks to 0 and walks only the tag region). Fills `meta` and/or `rg`
 * (either may be NULL) after zeroing them: TIT2/TPE1/TALB/TCON/TRCK text
 * frames plus TXXX REPLAYGAIN_*. v2.2 tags and tags with the unsync or
 * extended-header flag are reported as "nothing found" instead of being
 * misparsed (their frame layout differs). Returns TAG_HAVE_* flags. */
int tags_read_id3v2(codec_io_t *io, codec_meta_t *meta, codec_replaygain_t *rg);

/* Read a Vorbis comment block at [offset, offset+length) in `io` — the
 * spec layout vendor_length + vendor + count + entries (a FLAC codec passes
 * the VORBIS_COMMENT block body). Fills `meta` and/or `rg` (either may be
 * NULL). Returns TAG_HAVE_* flags. */
int tags_read_vorbis(codec_io_t *io, long offset, long length,
                     codec_meta_t *meta, codec_replaygain_t *rg);

/* Lightweight per-file tag read for listings (file browser, and the
 * metadata index BRNOMT-74 will build): opens `path` through codec_io,
 * picks the container by extension (.mp3 -> ID3v2, .flac -> Vorbis) and
 * walks only the header region — no codec/decoder state, no PCM. Returns
 * TAG_HAVE_* flags (0 for unknown extensions / unreadable files / no
 * tags). */
int tags_read_file(const char *path, codec_meta_t *meta, codec_replaygain_t *rg);

/* ReplayGain-only entry points (thin wrappers over the readers above;
 * kept so existing callers/tests keep their contract). Return 0 when at
 * least one ReplayGain value was parsed, < 0 otherwise. */
int tags_replaygain_id3v2(codec_io_t *io, codec_replaygain_t *out);
int tags_replaygain_vorbis(codec_io_t *io, long offset, long length,
                           codec_replaygain_t *out);

#ifdef __cplusplus
}
#endif

#endif /* RECHORD_TAGS_H */
