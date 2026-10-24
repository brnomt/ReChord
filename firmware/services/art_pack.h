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

/* art_pack_color() selectors (RGB565). The first four are the cover's
 * dominant/secondary/mean colours and the mockup's accent; the rest are the
 * derived shades (docs/ART_PACK.md §"Palette") so themes never do colour
 * maths on the device. */
enum {
    ART_C_DOMINANT = 0,     /* art_c1   */
    ART_C_SECONDARY,        /* art_c2   */
    ART_C_MEAN,             /* art_mean */
    ART_C_ACCENT,           /* art_accent (S .45-.90, V .75-.98) */
    ART_C_BG_A,             /* art_bg_a   dominant at V=0.42 */
    ART_C_BG_B,             /* art_bg_b   secondary at V=0.24 */
    ART_C_GLOW,             /* art_glow   mix(dom,sec) at V=0.55 */
    ART_C_PANEL,            /* art_panel  mean at V=0.16 */
    ART_C_LABEL,            /* art_label  cream mixed 10% with mean */
    ART_C_INK,              /* art_ink     #221E1C (fixed) */
    ART_C_INK_DIM           /* art_ink_dim #706860 (fixed) */
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
