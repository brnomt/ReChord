/*
 * art_pack.h — precomputed album art (read-only, no image decoding).
 *
 * The PC tool (tools/artpack) turns each track's cover into RGB565 at two
 * sizes plus a palette and writes <internal>/.rechord/ARTPACK.BIN. This module
 * only reads that file: it looks up the current track's key (header + key
 * table sectors), keeps the palette in a few bytes, and streams the pixels
 * straight into the framebuffer on demand.
 *
 * It never writes to storage, never allocates, and never blocks in the audio
 * path (the lookup is deferred to the app loop, after the PCM ring is filled).
 *
 * Format: docs/ART_PACK.md. All integers little-endian; every section is
 * 512-byte aligned.
 */
#ifndef RECHORD_SERVICES_ART_PACK_H
#define RECHORD_SERVICES_ART_PACK_H

#include <stdint.h>

/* art_pack_dim() / art_pack_draw() size selectors. */
#define ART_SIZE_SMALL 0
#define ART_SIZE_LARGE 1

/* art_pack_color() selectors (RGB565). */
enum {
    ART_C_DOMINANT = 0,
    ART_C_SECONDARY,
    ART_C_MEAN,
    ART_C_ACCENT
};

/* Key for a device track path ("/Internal/Music/...") and file size:
 * FNV-1a 64 over the volume-relative path, then over the 4 LE size bytes.
 * Exposed for tests (the tool's vectors are shared). */
uint64_t art_pack_key(const char *device_path, uint32_t file_bytes);

/* Look up `device_path` in ARTPACK.BIN: opens the pack, validates the header
 * and key-table CRC, binary-searches the key (plus the file size, stat'ed
 * here) and loads the palette. Returns 0 when the current track has a cover,
 * < 0 otherwise (missing file, bad magic/version/CRC, key not found). On
 * failure the art state is cleared and everything else keeps working. */
int  art_pack_set_track(const char *device_path);
void art_pack_clear(void);

int  art_pack_ok(void);          /* 1 = current track has a cover */
int  art_pack_color(int which);  /* ART_C_* -> RGB565 */
int  art_pack_dim(int large);    /* stored size in px (0 when unknown) */

/* Stream the current cover into the framebuffer at (x,y), clipped, in
 * `large` (ART_SIZE_*) resolution. Opens the pack for the duration of the
 * draw (no permanent handle, so the codec/bfs handle pool is untouched).
 * Returns 0 when drawn, < 0 when unavailable (caller draws a placeholder). */
int  art_pack_draw(int x, int y, int large);

#endif /* RECHORD_SERVICES_ART_PACK_H */
