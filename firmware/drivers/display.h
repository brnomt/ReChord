/*
 * display.h -- ReChord clean-room RGB565 display driver (public API).
 *
 * Hardware contract (docs/re/route-b-minimum.md section 3,
 * "Display contract"):
 *   - DCS-style panel driven over the MCU command/data interface
 *   - windowing by 0x2A CASET (column set), 0x2B RASET (row set),
 *     pixel stream by 0x2C RAMWR
 *   - RGB565 pixels, 320 px wide, framebuffer row pitch 640 bytes
 *     (320 px * 2 B) -- see DISPLAY_ROW_BYTES
 *
 * Model: draw into an in-RAM framebuffer with the draw calls, then push
 * whole rows to the panel with display_flush_rows(). All draw calls clip
 * against the screen; rectangles use INCLUSIVE coordinates (so the full
 * screen is display_fill_rect(0, 0, DISPLAY_W - 1, DISPLAY_H - 1, c)).
 *
 * The panel transport lives behind panel_io.h (panel_io_rknanoc.c on
 * target, panel_io_sim.c on the host), so this file's logic is unit-
 * testable with plain `cc`.
 */

#ifndef RECHORD_DISPLAY_H
#define RECHORD_DISPLAY_H

#include <stdint.h>

/* Screen width in pixels (hardware contract: 320 px, x range 0..0x13F). */
#define DISPLAY_W 320

/*
 * Screen height in pixels.
 *
 * SINGLE CONFIG KNOB, documented: 170 rows is the height observed in the
 * boot area of the reference firmware (0..0xA9 inclusive; see
 * docs/re/route-b-minimum.md section 3), and the extracted init windows
 * (docs/rewrite/drivers.md) confirm a 170-row visible band on the panel.
 * Raising this define grows the framebuffer and everything follows
 * automatically -- but check PANEL_ROW_OFFSET first.
 */
#define DISPLAY_H 170

/*
 * Physical panel row index of logical row 0 (hardware contract,
 * extracted from the reference flush routine at 0x03051D40: every RASET
 * parameter is the row index + 35). The visible 170-row band sits at
 * panel rows 35..204 (0x23..0xCC); the extracted variant-B init window
 * 0x2B = 00 23 00 CC corroborates exactly this band. Do not change
 * without hardware evidence.
 */
#define PANEL_ROW_OFFSET 35

/* Framebuffer row pitch in BYTES -- fixed by the hardware contract
 * ("640 bytes/row framebuffer layout"). Changing DISPLAY_W changes this;
 * do not change it independently. */
#define DISPLAY_ROW_BYTES (DISPLAY_W * 2)

/*
 * Bring up the panel transport and the panel itself (panel power-on init),
 * clear the framebuffer to black and flush it, mirroring the observed
 * one-shot draw_init contract (panel_init + clear screen + flush).
 * Safe to call once at boot; drawing afterwards is visible only after
 * display_flush_rows().
 */
void display_init(void);

/*
 * Fill the inclusive rectangle (x0,y0)..(x1,y1) with 'rgb565'.
 * Coordinates are clipped to the screen; an empty (or fully off-screen)
 * rectangle draws nothing. Framebuffer only -- flush to make visible.
 */
void display_fill_rect(int x0, int y0, int x1, int y1, uint16_t rgb565);

/*
 * Draw an ASCII string with its top-left glyph cell at (x, y), 8x8 font
 * (font.h), opaque (fg = glyph pixels, bg = cell background). Clipped.
 * Framebuffer only -- flush to make visible.
 */
void display_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg);

/*
 * Blit a 1-bit bitmap (MSB-first, row-major, `stride` bytes per row) with
 * its top-left corner at (x, y). Set bits use `fg`, clear bits use `bg`.
 * Clipped pixel by pixel against the screen; NULL/short input is a no-op.
 * Framebuffer only -- flush to make visible.
 */
void display_bitmap1(int x, int y, int w, int h, int stride,
                     const uint8_t *bits, uint16_t fg, uint16_t bg);

/*
 * Push framebuffer rows y0..y1 (INCLUSIVE) to the panel via
 * CASET/RASET/RAMWR. Row indexes are clamped to 0..DISPLAY_H-1; a
 * reversed or empty range is a no-op. Sends the full row width.
 * Logical row y goes to physical panel row y + PANEL_ROW_OFFSET (the
 * extracted hardware row mapping, see PANEL_ROW_OFFSET above).
 */
void display_flush_rows(int y0, int y1);

/* Flush an inclusive rect. The panel's CASET/RASET windows make a partial
 * column range free, so a skin can refresh only its animated primitives
 * instead of the whole band. */
void display_flush_rect(int x0, int y0, int x1, int y1);

/*
 * Fill the whole framebuffer with 'rgb565'. Framebuffer only -- flush to make
 * visible.
 */
void display_clear(uint16_t rgb565);

/*
 * Copy `w` RGB565 pixels into framebuffer row `y`, starting at column `x`.
 * The source is one row segment (native uint16_t, i.e. the same layout
 * display_fb uses). Both ends are clipped: pixels left of 0 or right of
 * DISPLAY_W-1 are skipped, and an off-screen row is a no-op. This is the
 * streaming hook for image data (album art) that is decoded/read row by row —
 * it never allocates and never buffers the whole image.
 */
void display_blit_span(int x, int y, int w, const uint16_t *src);

/*
 * Panel power-on init sequence: emits the DCS command stream extracted
 * from the reference firmware's panel bring-up (sleep out + delay, vendor
 * power/gamma commands, MADCTL 0x36, COLMOD 0x3A, INVON 0x21, DISPON
 * 0x29). Implemented in panel_init_rknanoc.c (byte-exact tables plus
 * per-command documentation; docs/rewrite/drivers.md has the full
 * trace).
 *
 * The reference has TWO init variants selected by a probed panel ID;
 * since panel_io has no bus-read primitive yet, the variant is chosen at
 * compile time (PANEL_INIT_VARIANT, default 1 = variant B -- UNVERIFIED
 * on hardware). display_init() calls this automatically.
 */
void panel_init_sequence(void);

/*
 * Emit one specific extracted init variant (0 = variant A, 1 = variant
 * B) regardless of the compile-time choice; returns 0 on success, -1 for
 * an unknown variant number. Bring-up/test seam so both streams can be
 * exercised (e.g. from a debug menu, or by the host tests).
 */
int panel_init_run_variant(int variant);

#endif /* RECHORD_DISPLAY_H */
