/*
 * font.h -- ReChord clean-room bitmap font renderer (8x8 cell).
 *
 * The glyph bitmaps are our own designs (see tools/gen_font8x8.py and
 * font8x8.c). Rendering is plain integer pixel blitting into an RGB565
 * surface supplied by the caller, so the font layer never touches hardware
 * and is fully host-testable.
 *
 * Contract:
 *   - fixed 8x8 cell, ASCII 0x20..0x7E, no kerning
 *   - glyphs outside the range render as '?'
 *   - drawing is opaque: every cell pixel is written (fg or bg)
 *   - all coordinates clip against the destination surface
 */

#ifndef RECHORD_FONT_H
#define RECHORD_FONT_H

#include <stdint.h>

#define FONT_CELL_W     8           /* pixels per glyph cell, horizontally   */
#define FONT_CELL_H     8           /* pixels per glyph cell, vertically     */
#define FONT_FIRST_CODE 0x20        /* first encoded ASCII code (' ')        */
#define FONT_LAST_CODE  0x7E        /* last encoded ASCII code ('~')         */
#define FONT_GLYPH_COUNT (FONT_LAST_CODE - FONT_FIRST_CODE + 1) /* 95       */

/*
 * A destination surface: an RGB565 pixel buffer with a rectangular clip box.
 * fb[row * pitch_px + col] is the pixel at (col, row); only 0 <= x < w and
 * 0 <= y < h is ever written. pitch_px may be larger than w (padding rows).
 */
typedef struct {
    uint16_t *fb;
    int w;          /* clip width in pixels  */
    int h;          /* clip height in pixels */
    int pitch_px;   /* row pitch in pixels   */
} font_surface_t;

/*
 * Glyph lookup: returns 8 row-bytes for the cell of 'ch' (bit 7 = leftmost
 * pixel). Codes outside 0x20..0x7E return the '?' glyph. Never NULL.
 */
const uint8_t *font_glyph(uint8_t ch);

/*
 * Draw an ASCII string with its top-left cell at (x, y). Opaque: cell
 * pixels get 'fg' where the glyph bit is set, 'bg' where it is clear.
 * Positions outside the surface are clipped per pixel. NULL args are no-ops.
 */
void font_draw_text(const font_surface_t *dst, int x, int y,
                    const char *s, uint16_t fg, uint16_t bg);

#endif /* RECHORD_FONT_H */
