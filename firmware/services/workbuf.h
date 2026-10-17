/*
 * workbuf.h — one shared RAM region for the heavy, MUTUALLY-EXCLUSIVE users
 * that must NOT live in services/scratch (media index/query use that one and
 * can run WHILE a track plays).
 *
 * Owners here never overlap in time:
 *   - FLAC decode pool   (a whole FLAC frame must be buffered: window =
 *                         max_block * 4 bytes * channels, so the pool has to
 *                         hold an 8192-sample stereo block ~= 69.9 KiB)
 *   - mp3 working buffers (feed window + residual frame)
 *   - fw_update staging  (runs at boot, before playback/USB/index)
 *
 * The region is exactly the sum of the three statics it replaces, so the
 * .bss total is unchanged — it does NOT raise the 0x0304A000 ceiling
 * (DFIU R-8cd5fb0eb24a: union the mutually-exclusive big buffers instead of
 * raising the ceiling; FLAC pool / USB-MSC chunk / fw_update staging).
 *
 * Ownership is a single owner id (0 = free); claim returns NULL when taken.
 */
#ifndef RECHORD_WORKBUF_H
#define RECHORD_WORKBUF_H

/* 80896 = old FLAC pool 57344 + fw_update staging 16384 + mp3 buffers 7168. */
#define WORKBUF_BYTES 80896u

#define WORKBUF_OWNER_NONE  0u
#define WORKBUF_OWNER_FLAC  1u
#define WORKBUF_OWNER_MP3   2u
#define WORKBUF_OWNER_FWUPD 3u

/* Take the region for `owner` (the owner id, not NONE). Returns its base
 * address, or NULL when another owner holds it. Re-claiming by the current
 * owner returns the same base. */
void *workbuf_claim(unsigned owner);

/* Release the region (only the current owner can; anything else is ignored). */
void workbuf_release(unsigned owner);

/* Current owner, WORKBUF_OWNER_NONE when free. */
unsigned workbuf_owner(void);

/* The region when `owner` is its current owner, else NULL. */
void *workbuf_for(unsigned owner);

#endif /* RECHORD_WORKBUF_H */
