/*
 * test_font_ppm.c -- host tests for the font layer and PPM rendering:
 *   1. glyph data contract (own bitmaps, ASCII 0x20..0x7E, '?' fallback)
 *   2. font_draw_text: pixel-exact blitting, offsets, clipping, canaries
 *   3. full-screen font render through the display driver to a PPM file
 *
 * Build: see run_tests.sh (cc -std=c99 -Wall -Werror).
 * Usage: test_font_ppm [output.ppm]   (default: font_render.ppm)
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "font.h"
#include "panel_io_sim.h"
#include "test_util.h"

#define FG 0xFFFFu
#define BG 0x0000u

static void test_glyph_data(void)
{
    const uint8_t *g;
    int i, nonzero;

    CHECK(FONT_GLYPH_COUNT == 95);
    CHECK(FONT_CELL_W == 8 && FONT_CELL_H == 8);

    /* 'A' from our own design (tools/gen_font8x8.py): */
    g = font_glyph('A');
    CHECK(g[0] == 0x20);   /* ..#..... */
    CHECK(g[3] == 0x88);   /* #...#... */
    CHECK(g[4] == 0xFE);   /* #######. */
    CHECK(g[7] == 0x00);   /* gap row  */

    /* '!' from our own design: */
    g = font_glyph('!');
    CHECK(g[0] == 0x20 && g[1] == 0x20 && g[5] == 0x00 && g[7] == 0x00);

    /* Space is completely empty. */
    g = font_glyph(' ');
    for (i = 0; i < FONT_CELL_H; i++) {
        CHECK(g[i] == 0x00);
    }

    /* Out-of-range codes render as '?'. */
    CHECK(font_glyph(0x00) == font_glyph('?'));
    CHECK(font_glyph(0x7F) == font_glyph('?'));
    CHECK(font_glyph(0xFF) == font_glyph('?'));

    /* Every glyph is reachable and every one except space has pixels. */
    for (i = FONT_FIRST_CODE; i <= FONT_LAST_CODE; i++) {
        int r;
        nonzero = 0;
        g = font_glyph((uint8_t)i);
        CHECK(g != NULL);
        for (r = 0; r < FONT_CELL_H; r++) {
            if (g[r] != 0) {
                nonzero = 1;
            }
        }
        if (i != ' ') {
            CHECK(nonzero == 1);
        }
    }
}

/* Reference pixel of drawing "Hi" at (-3, -3) onto a 20x12 surface
 * pre-filled with 0x1234 -- expectations derived by hand from our
 * glyph bitmaps (see gen_font8x8.py):
 *   'H' cell at (-3,-3): visible = rows 3..7, cols 3..7 of the glyph
 *   'i' cell at ( 5,-3): visible = rows 3..7, cols 0..7 of the glyph
 */
static uint16_t expected_pixel(int px, int py)
{
    const uint8_t *gh = font_glyph('H');
    const uint8_t *gi = font_glyph('i');
    int gx = px + 3;   /* glyph column inside the 'H' cell */
    int gy = py + 3;   /* glyph row inside both cells      */

    if (py < 0 || py > 4) {
        return 0x1234u;              /* below/above the visible band */
    }
    if (px <= 4) {                   /* 'H' cell (pen at x=-3, width 8) */
        return (gh[gy] & (uint8_t)(0x80u >> gx)) ? FG : BG;
    }
    if (px <= 12) {                  /* 'i' cell (pen at x=5, width 8) */
        int ix = px - 5;
        return (gi[gy] & (uint8_t)(0x80u >> ix)) ? FG : BG;
    }
    return 0x1234u;                  /* right of both cells: untouched */
}

static void test_font_draw_text(void)
{
    /* Backing store with canary rows/columns beyond the visible surface. */
    uint16_t buf[14][24];
    font_surface_t dst;
    int x, y;

    for (y = 0; y < 14; y++) {
        for (x = 0; x < 24; x++) {
            buf[y][x] = 0x1234u;
        }
    }
    dst.fb = &buf[0][0];
    dst.w = 20;
    dst.h = 12;
    dst.pitch_px = 24;

    /* Offsets in both axes exercise per-pixel clipping. */
    font_draw_text(&dst, -3, -3, "Hi", FG, BG);

    for (y = 0; y < 12; y++) {
        for (x = 0; x < 20; x++) {
            if (buf[y][x] != expected_pixel(x, y)) {
                CHECK(0);   /* one failure per mismatching pixel */
            }
        }
    }
    /* Nothing outside the surface was touched. */
    for (x = 0; x < 24; x++) {
        CHECK(buf[12][x] == 0x1234u);
        CHECK(buf[13][x] == 0x1234u);
    }
    for (y = 0; y < 14; y++) {
        CHECK(buf[y][20] == 0x1234u);
        CHECK(buf[y][21] == 0x1234u);
        CHECK(buf[y][22] == 0x1234u);
        CHECK(buf[y][23] == 0x1234u);
    }

    /* Fully off-screen text is a clean no-op. */
    font_draw_text(&dst, -100, -100, "Hi", FG, BG);
    font_draw_text(&dst, 100, 100, "Hi", FG, BG);
    for (y = 0; y < 12; y++) {
        for (x = 0; x < 20; x++) {
            if (buf[y][x] != expected_pixel(x, y)) {
                CHECK(0);
            }
        }
    }

    /* NULL arguments are safe no-ops. */
    font_draw_text(NULL, 0, 0, "Hi", FG, BG);
    font_draw_text(&dst, 0, 0, NULL, FG, BG);
}

static void rgb565_to_rgb888(uint16_t v, unsigned char rgb[3])
{
    rgb[0] = (unsigned char)((((v >> 11) & 0x1F) * 255u) / 31u);
    rgb[1] = (unsigned char)((((v >> 5) & 0x3F) * 255u) / 63u);
    rgb[2] = (unsigned char)(((v & 0x1F) * 255u) / 31u);
}

static void test_ppm_render(const char *path)
{
    enum { SHEET_COLS = 32 };
    const uint16_t bg = 0x1082u;   /* boot-screen grey (contract sample) */
    char hdr[32];
    char cell[2];
    int i, x, y;
    long hdr_len, expected_size;
    FILE *f;

    display_init();
    display_clear(bg);
    display_draw_text(4, 4, "ReChord 0.9.2", FG, bg);

    /* Full printable-ASCII sheet, 32 glyph cells per row. */
    for (i = 0; i < FONT_GLYPH_COUNT; i++) {
        cell[0] = (char)(FONT_FIRST_CODE + i);
        cell[1] = '\0';
        display_draw_text(4 + (i % SHEET_COLS) * 9,
                          24 + (i / SHEET_COLS) * 9, cell, FG, bg);
    }
    display_flush_rows(0, DISPLAY_H - 1);
    CHECK(panel_io_sim_overflow() == 0);

    /* Visual artifact for humans (and CI inspection). */
    CHECK(panel_io_sim_write_ppm(path) == 0);

    /* Structure check: binary PPM (P6), exact header + RGB888 payload. */
    sprintf(hdr, "P6\n%d %d\n255\n", PANEL_SIM_W, PANEL_SIM_H);
    hdr_len = (long)strlen(hdr);
    expected_size = hdr_len + 3L * PANEL_SIM_W * PANEL_SIM_H;

    f = fopen(path, "rb");
    CHECK(f != NULL);
    if (f == NULL) {
        return;
    }
    {
        char got[32];
        CHECK(fread(got, 1, (size_t)hdr_len, f) == (size_t)hdr_len);
        CHECK(memcmp(got, hdr, (size_t)hdr_len) == 0);
        CHECK(fseek(f, 0, SEEK_END) == 0);
        CHECK(ftell(f) == expected_size);
    }

    /* Content spot checks through the file: the 'R' of "ReChord" has a
     * lit pixel at (4,4); a far corner is plain background. */
    {
        unsigned char px[3];
        unsigned char want[3];

        CHECK(fseek(f, hdr_len + ((4L * PANEL_SIM_W) + 4) * 3, SEEK_SET) == 0);
        CHECK(fread(px, 1, 3, f) == 3);
        rgb565_to_rgb888(FG, want);
        CHECK(memcmp(px, want, 3) == 0);

        CHECK(fseek(f, hdr_len + ((160L * PANEL_SIM_W) + 300) * 3,
                    SEEK_SET) == 0);
        CHECK(fread(px, 1, 3, f) == 3);
        rgb565_to_rgb888(bg, want);
        CHECK(memcmp(px, want, 3) == 0);
    }
    fclose(f);

    /* And the same facts hold on the simulated panel itself. */
    CHECK(panel_io_sim_pixel(4, 4) == FG);
    CHECK(panel_io_sim_pixel(300, 160) == bg);
    for (y = 0; y < 4; y++) {           /* headline cell rows are drawn */
        for (x = 0; x < 8; x++) {
            uint16_t p = panel_io_sim_pixel(4 + x, 4 + y);
            CHECK(p == FG || p == bg);
        }
    }
}

int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "font_render.ppm";

    test_glyph_data();
    test_font_draw_text();
    test_ppm_render(path);
    TEST_END("test_font_ppm");
}
