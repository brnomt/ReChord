/*
 * display.h -- ReChord clean-room RGB565 display driver (public API).
 *
 * Hardware contract (docs/re/route-b-minimum.md §3, "Display contract"):
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
 * boot area of the reference firmware (0..0xA8 inclusive; see
 * docs/re/route-b-minimum.md §3). The panel may be taller; the true
 * height only becomes observable once panel_init_sequence() is filled in
 * (open item) or the boot area is measured again on hardware. Raising
 * this define grows the framebuffer and everything follows automatically.
 */
#define DISPLAY_H 170

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
 * Push framebuffer rows y0..y1 (INCLUSIVE) to the panel via
 * CASET/RASET/RAMWR. Row indexes are clamped to 0..DISPLAY_H-1; a
 * reversed or empty range is a no-op. Sends the full row width.
 */
void display_flush_rows(int y0, int y1);

/*
 * Fill the whole framebuffer with 'rgb565'. Framebuffer only -- flush
 * to make visible.
 */
void display_clear(uint16_t rgb565);

/*
 * Panel power-on init sequence (DCS 0x11 sleep-out, 0x3A pixel format,
 * 0x29 display-on, vendor-specific power/gamma commands, ...).
 *
 * >>> OPEN ITEM — SAFE NO-OP BY DEFAULT <<<
 * The exact sequence for the Echo Mini panel still needs to be extracted
 * from the reference firmware's panel_init (hardware CONTRACT data, not
 * code). Once known, fill in the panel_init_table[] steps in display.c
 * (the hook is already wired: commands, parameters, per-step delays).
 * Until then this function sends nothing, which is safe both against an
 * already-initialized panel (hybrid boot path) and the host simulator.
 */
void panel_init_sequence(void);

#endif /* RECHORD_DISPLAY_H */
