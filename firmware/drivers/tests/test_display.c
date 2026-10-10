/*
 * test_display.c -- host tests for the display driver:
 *   1. geometry contract (320x170, 640-byte row pitch)
 *   2. display_fill_rect: fills, inclusive corners, bounds clipping
 *   3. row math: flush range clamping, no-op ranges, pixel round-trip
 *
 * Uses the panel_io host simulator; every drawing fact is checked on the
 * simulated panel after flushing, exactly as it would appear on hardware.
 *
 * Build: see run_tests.sh (cc -std=c99 -Wall -Werror).
 */

#include <stdint.h>

#include "display.h"
#include "panel_io_sim.h"
#include "test_util.h"

#define RED   0xF800u
#define GREEN 0x07E0u
#define BLUE  0x001Fu
#define WHITE 0xFFFFu
#define BLACK 0x0000u

static int sim_window[4];

static void check_window(int xs, int ys, int xe, int ye)
{
    panel_io_sim_last_window(sim_window);
    CHECK(sim_window[0] == xs);
    CHECK(sim_window[1] == ys);
    CHECK(sim_window[2] == xe);
    CHECK(sim_window[3] == ye);
}

static void test_geometry(void)
{
    CHECK(DISPLAY_W == 320);            /* hardware contract: 320 px wide */
    CHECK(DISPLAY_H == 170);            /* documented boot-area height    */
    CHECK(DISPLAY_ROW_BYTES == 640);    /* contract: 640 bytes/row        */
    CHECK(DISPLAY_W * 2 == DISPLAY_ROW_BYTES);
}

static void test_init(void)
{
    display_init();

    /* display_init = transport + panel init (no-op) + clear + full flush,
     * i.e. exactly one CASET/RASET/RAMWR triple on the wire. */
    CHECK(panel_io_sim_cmd_count() == 3);
    CHECK(panel_io_sim_cmd_at(0) == 0x2A);
    CHECK(panel_io_sim_cmd_at(1) == 0x2B);
    CHECK(panel_io_sim_cmd_at(2) == 0x2C);
    check_window(0, 0, DISPLAY_W - 1, DISPLAY_H - 1);
    CHECK(panel_io_sim_pixel(0, 0) == BLACK);
    CHECK(panel_io_sim_pixel(DISPLAY_W - 1, DISPLAY_H - 1) == BLACK);
    CHECK(panel_io_sim_overflow() == 0);
}

static void test_fill_rect(void)
{
    /* Center fill with inclusive corners. */
    display_fill_rect(10, 20, 19, 29, RED);
    display_flush_rows(0, DISPLAY_H - 1);
    CHECK(panel_io_sim_pixel(10, 20) == RED);
    CHECK(panel_io_sim_pixel(19, 29) == RED);
    CHECK(panel_io_sim_pixel(14, 25) == RED);
    CHECK(panel_io_sim_pixel(9, 20) == BLACK);   /* just outside each edge */
    CHECK(panel_io_sim_pixel(20, 29) == BLACK);
    CHECK(panel_io_sim_pixel(10, 19) == BLACK);
    CHECK(panel_io_sim_pixel(10, 30) == BLACK);

    /* Clipping at the top-left corner. */
    display_fill_rect(-5, -5, 4, 4, GREEN);
    /* Clipping at the bottom-right corner. */
    display_fill_rect(DISPLAY_W - 4, DISPLAY_H - 4,
                      DISPLAY_W + 100, DISPLAY_H + 100, BLUE);
    /* Reversed rectangle draws nothing. */
    display_fill_rect(50, 50, 40, 60, WHITE);
    /* Fully off-screen rectangles draw nothing. */
    display_fill_rect(-10, -10, -1, -1, WHITE);
    display_fill_rect(DISPLAY_W, DISPLAY_H, DISPLAY_W + 5, DISPLAY_H + 5,
                      WHITE);
    /* Single-pixel rectangle is valid. */
    display_fill_rect(60, 60, 60, 60, WHITE);

    display_flush_rows(0, DISPLAY_H - 1);
    CHECK(panel_io_sim_pixel(0, 0) == GREEN);
    CHECK(panel_io_sim_pixel(4, 4) == GREEN);
    CHECK(panel_io_sim_pixel(5, 5) == BLACK);           /* clip edge      */
    CHECK(panel_io_sim_pixel(50, 55) == BLACK);         /* reversed rect  */
    CHECK(panel_io_sim_pixel(45, 55) == BLACK);
    CHECK(panel_io_sim_pixel(DISPLAY_W - 1, DISPLAY_H - 1) == BLUE);
    CHECK(panel_io_sim_pixel(DISPLAY_W - 4, DISPLAY_H - 4) == BLUE);
    CHECK(panel_io_sim_pixel(DISPLAY_W - 5, DISPLAY_H - 5) == BLACK);
    CHECK(panel_io_sim_pixel(60, 60) == WHITE);
    CHECK(panel_io_sim_pixel(0, 0) == GREEN);           /* untouched      */
    CHECK(panel_io_sim_overflow() == 0);

    /* Full clear. */
    display_clear(BLACK);
    display_flush_rows(0, DISPLAY_H - 1);
    {
        int x, y, dirty = 0;
        for (y = 0; y < DISPLAY_H; y++) {
            for (x = 0; x < DISPLAY_W; x++) {
                if (panel_io_sim_pixel(x, y) != BLACK) {
                    dirty++;
                }
            }
        }
        CHECK(dirty == 0);
    }
}

static void test_row_math(void)
{
    /* Row content: rows 0..2 red, rows 5..H-1 green, rest black. */
    display_clear(BLACK);
    display_fill_rect(0, 0, DISPLAY_W - 1, 2, RED);
    display_fill_rect(0, 5, DISPLAY_W - 1, DISPLAY_H - 1, GREEN);

    /* Flush with negative start: clamped to row 0. */
    panel_io_sim_reset();
    display_flush_rows(-3, 2);
    CHECK(panel_io_sim_cmd_count() == 3);
    CHECK(panel_io_sim_cmd_at(0) == 0x2A);
    CHECK(panel_io_sim_cmd_at(1) == 0x2B);
    CHECK(panel_io_sim_cmd_at(2) == 0x2C);
    check_window(0, 0, DISPLAY_W - 1, 2);
    CHECK(panel_io_sim_pixel(0, 0) == RED);
    CHECK(panel_io_sim_pixel(DISPLAY_W - 1, 2) == RED);
    CHECK(panel_io_sim_pixel(0, 3) == BLACK);   /* row 3 was not flushed  */
    CHECK(panel_io_sim_overflow() == 0);

    /* Flush with huge end: clamped to the last row. */
    display_flush_rows(5, 1000);
    check_window(0, 5, DISPLAY_W - 1, DISPLAY_H - 1);
    CHECK(panel_io_sim_pixel(0, 5) == GREEN);
    CHECK(panel_io_sim_pixel(DISPLAY_W - 1, DISPLAY_H - 1) == GREEN);
    CHECK(panel_io_sim_pixel(0, 4) == BLACK);   /* row 4 not in this flush */
    CHECK(panel_io_sim_overflow() == 0);

    /* Empty ranges are wire-level no-ops. */
    panel_io_sim_reset();
    display_flush_rows(10, 5);
    CHECK(panel_io_sim_cmd_count() == 0);
    display_flush_rows(-5, -1);
    CHECK(panel_io_sim_cmd_count() == 0);
    display_flush_rows(DISPLAY_H, DISPLAY_H + 50);
    CHECK(panel_io_sim_cmd_count() == 0);
}

int main(void)
{
    test_geometry();
    test_init();
    test_fill_rect();
    test_row_math();
    TEST_END("test_display");
}
