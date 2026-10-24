/*
 * display.c -- ReChord clean-room RGB565 display driver.
 *
 * Implements the display.h contract against the panel_io.h transport.
 * Pure C99, no hardware headers: everything target-specific is behind
 * panel_io (panel_io_rknanoc.c on target, panel_io_sim.c on host).
 *
 * Framebuffer model (hardware contract): a single RGB565 buffer with a
 * fixed 640-byte row pitch (DISPLAY_ROW_BYTES = 320 px * 2 B). Row y
 * starts at byte offset y * DISPLAY_ROW_BYTES. Draw calls only touch the
 * buffer; display_flush_rows() pushes row ranges over DCS 0x2A/0x2B/0x2C.
 */

#include "display.h"
#include "font.h"
#include "panel_io.h"

/* DCS command bytes used by the flush path. */
#define DCS_CASET  0x2A  /* column address set */
#define DCS_RASET  0x2B  /* row address set    */
#define DCS_RAMWR  0x2C  /* memory write       */

/* The whole screen lives in RAM; the linker script owns placement.
 * (170 rows * 640 B = 108,800 B -- see docs/rewrite/drivers.md.) */
static uint16_t display_fb[DISPLAY_H][DISPLAY_W];

/* Send a 16-bit parameter big-endian (high byte first), as DCS expects. */
static void send_u16(uint16_t v)
{
    panel_io_send_data((uint8_t)(v >> 8));
    panel_io_send_data((uint8_t)(v & 0xFFu));
}

/* ---------------------------------------------------------------------------
 * Panel power-on init -- see panel_init_rknanoc.c.
 *
 * The DCS init sequence extracted from the reference firmware lives in
 * panel_init_rknanoc.c (byte-exact tables for the two panel variants the
 * reference supports, emitted through panel_io). This file only calls
 * panel_init_sequence() from display_init() below.
 * ------------------------------------------------------------------------ */

/* ---------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------ */

void display_init(void)
{
    backlight_on();
    panel_io_init();
    panel_init_sequence();
    /* Observed draw_init contract: panel_init + clear screen + flush. */
    display_clear(0x0000);
    display_flush_rows(0, DISPLAY_H - 1);
}

void display_fill_rect(int x0, int y0, int x1, int y1, uint16_t rgb565)
{
    int x, y;

    /* Clip to the screen; coordinates are inclusive on all edges. */
    if (x0 < 0) { x0 = 0; }
    if (y0 < 0) { y0 = 0; }
    if (x1 > DISPLAY_W - 1) { x1 = DISPLAY_W - 1; }
    if (y1 > DISPLAY_H - 1) { y1 = DISPLAY_H - 1; }
    if (x0 > x1 || y0 > y1) {
        return;   /* empty after clipping (incl. reversed input) */
    }

    for (y = y0; y <= y1; y++) {
        uint16_t *row = &display_fb[y][x0];
        for (x = x0; x <= x1; x++) {
            *row++ = rgb565;
        }
    }
}

void display_draw_text(int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
    font_surface_t dst;

    dst.fb = &display_fb[0][0];
    dst.w = DISPLAY_W;
    dst.h = DISPLAY_H;
    dst.pitch_px = DISPLAY_W;
    font_draw_text(&dst, x, y, s, fg, bg);
}

void display_bitmap1(int x, int y, int w, int h, int stride,
                     const uint8_t *bits, uint16_t fg, uint16_t bg)
{
    int row, col;

    if (bits == 0 || w <= 0 || h <= 0 || stride <= 0)
        return;
    for (row = 0; row < h; row++) {
        int py = y + row;

        if (py < 0 || py >= DISPLAY_H)
            continue;
        for (col = 0; col < w; col++) {
            int px = x + col;
            uint8_t byte;

            if (px < 0 || px >= DISPLAY_W)
                continue;
            byte = bits[row * stride + (col >> 3)];
            display_fb[py][px] = (byte & (uint8_t)(0x80u >> (col & 7)))
                                     ? fg : bg;
        }
    }
}

void display_flush_rows(int y0, int y1)
{
    int x, y;

    if (y0 < 0) { y0 = 0; }
    if (y1 > DISPLAY_H - 1) { y1 = DISPLAY_H - 1; }
    if (y0 > y1) {
        return;   /* empty range */
    }

    /* CASET: full row width (extracted contract: columns 0..0x013F). */
    panel_io_send_cmd(DCS_CASET);
    send_u16(0);
    send_u16((uint16_t)(DISPLAY_W - 1));

    /* RASET: the inclusive row range y0..y1, mapped to physical panel
     * rows by PANEL_ROW_OFFSET (extracted contract: the reference's
     * flush routine sends row + 35 -- see display.h). */
    panel_io_send_cmd(DCS_RASET);
    send_u16((uint16_t)(y0 + PANEL_ROW_OFFSET));
    send_u16((uint16_t)(y1 + PANEL_ROW_OFFSET));

    /* RAMWR: the pixels, row-major, high byte first (panel_io contract). */
    panel_io_send_cmd(DCS_RAMWR);
    for (y = y0; y <= y1; y++) {
        const uint16_t *row = &display_fb[y][0];
        for (x = 0; x < DISPLAY_W; x++) {
            panel_io_send_pixel(row[x]);
        }
    }
}

void display_clear(uint16_t rgb565)
{
    display_fill_rect(0, 0, DISPLAY_W - 1, DISPLAY_H - 1, rgb565);
}

void display_blit_span(int x, int y, int w, const uint16_t *src)
{
    int x0, x1, skip, i;

    if (src == 0 || w <= 0 || y < 0 || y >= DISPLAY_H)
        return;
    x0 = x;
    x1 = x + w - 1;
    skip = 0;
    if (x0 < 0) {
        skip = -x0;
        x0 = 0;
    }
    if (x1 > DISPLAY_W - 1)
        x1 = DISPLAY_W - 1;
    if (x0 > x1)
        return;
    for (i = x0; i <= x1; i++)
        display_fb[y][i] = src[skip++];
}
