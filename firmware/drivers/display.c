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
 * Panel power-on init -- OPEN ITEM, hook ready to fill (see display.h).
 *
 * TODO(docs/rewrite/drivers.md "open items"): extract the panel's real
 * power-on DCS sequence from the reference firmware's panel_init
 * (route-b-minimum.md §3, panel_init @ 0x03051EA8). That extraction gives
 * us a hardware CONTRACT (a list of DCS commands + delays), which we then
 * encode as rows of panel_init_table[] below -- no reference code is ever
 * copied. Expected shape of the answer (typical DCS panels, UNVERIFIED for
 * this device): 0x11 sleep-out + ~120 ms, vendor power/gamma set, 0x3A
 * pixel format 0x55 (RGB565), 0x29 display-on.
 *
 * The table is executed by panel_init_sequence() below. A row with
 * cmd == PANEL_INIT_END (0x00, DCS NOP) terminates the table, so the
 * default table is a single harmless terminator = safe no-op.
 * ------------------------------------------------------------------------ */

#define PANEL_INIT_MAX_DATA 8

typedef struct {
    uint8_t  cmd;                    /* DCS command byte                  */
    uint8_t  nbytes;                 /* parameter bytes in data[] (0..8)  */
    uint8_t  data[PANEL_INIT_MAX_DATA];
    uint16_t delay_ms;               /* wait AFTER this step              */
} panel_init_step_t;

#define PANEL_INIT_END 0x00  /* terminator (DCS NOP is a safe no-op) */

static const panel_init_step_t panel_init_table[] = {
    /* FILL HERE when the panel_init extraction lands, e.g.:
     *   { 0x11, 0, { 0 },          120 },   sleep out, then wait
     *   { 0x3A, 1, { 0x55 },       0   },   RGB565 pixel format
     *   { 0x29, 0, { 0 },          20  },   display on
     */
    { PANEL_INIT_END, 0, { 0 }, 0 }
};

void panel_init_sequence(void)
{
    const panel_init_step_t *step;

    for (step = panel_init_table; step->cmd != PANEL_INIT_END; step++) {
        int i;

        panel_io_send_cmd(step->cmd);
        for (i = 0; i < step->nbytes && i < PANEL_INIT_MAX_DATA; i++) {
            panel_io_send_data(step->data[i]);
        }
        if (step->delay_ms > 0) {
            panel_io_delay_ms(step->delay_ms);
        }
    }
}

/* ---------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------ */

void display_init(void)
{
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

void display_flush_rows(int y0, int y1)
{
    int x, y;

    if (y0 < 0) { y0 = 0; }
    if (y1 > DISPLAY_H - 1) { y1 = DISPLAY_H - 1; }
    if (y0 > y1) {
        return;   /* empty range */
    }

    /* CASET: full row width. */
    panel_io_send_cmd(DCS_CASET);
    send_u16(0);
    send_u16((uint16_t)(DISPLAY_W - 1));

    /* RASET: the inclusive row range y0..y1. */
    panel_io_send_cmd(DCS_RASET);
    send_u16((uint16_t)y0);
    send_u16((uint16_t)y1);

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
