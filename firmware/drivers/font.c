/*
 * font.c -- ReChord clean-room bitmap font renderer.
 *
 * Pure integer pixel blitting into a caller-supplied RGB565 surface
 * (font_surface_t). No globals, no hardware access: the display driver
 * hands it the framebuffer, the host tests hand it plain arrays.
 */

#include <stddef.h>

#include "font.h"

/* Glyph table, generated from our own designs (see tools/gen_font8x8.py). */
extern const uint8_t font8x8_data[FONT_GLYPH_COUNT][FONT_CELL_H];

const uint8_t *font_glyph(uint8_t ch)
{
    if (ch < FONT_FIRST_CODE || ch > FONT_LAST_CODE) {
        ch = (uint8_t)'?';   /* replacement glyph for control/8-bit codes */
    }
    return font8x8_data[ch - FONT_FIRST_CODE];
}

void font_draw_text(const font_surface_t *dst, int x, int y,
                    const char *s, uint16_t fg, uint16_t bg)
{
    int pen;   /* pen position of the current cell, in pixels */

    if (dst == NULL || dst->fb == NULL || s == NULL) {
        return;
    }

    for (pen = x; *s != '\0'; s++, pen += FONT_CELL_W) {
        const uint8_t *glyph = font_glyph((uint8_t)*s);
        int row;

        /* Early-out: a cell fully outside the clip box writes nothing. */
        if (pen + FONT_CELL_W <= 0 || pen >= dst->w ||
            y + FONT_CELL_H <= 0 || y >= dst->h) {
            continue;
        }

        for (row = 0; row < FONT_CELL_H; row++) {
            uint8_t bits = glyph[row];
            int py = y + row;
            int col;

            if (py < 0 || py >= dst->h) {
                continue;   /* row clipped: skip whole row */
            }
            for (col = 0; col < FONT_CELL_W; col++) {
                int px = pen + col;
                if (px < 0 || px >= dst->w) {
                    continue;   /* column clipped */
                }
                dst->fb[py * dst->pitch_px + px] =
                    (bits & (uint8_t)(0x80u >> col)) ? fg : bg;
            }
        }
    }
}
