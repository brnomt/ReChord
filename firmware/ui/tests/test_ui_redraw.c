/*
 * test_ui_redraw.c — dirty-region redraw model (ui.c): row bands merge and
 * only dirty rows reach display_flush_rows().
 */
#include "test_util.h"
#include "ui.h"
#include "theme.h"
#include "mock_display.h"

static int s_draws;

static void draw_count(void)
{
    s_draws++;
}

static const ui_screen_t draw_screen = { "draw", NULL, NULL, draw_count, NULL };

void test_ui_redraw(void)
{
    int y0 = -1, y1 = -1;

    ui_init();
    mock_display_reset();
    s_draws = 0;

    CHECK(ui_push_screen(&draw_screen));

    /* pushing a screen invalidates the whole panel */
    CHECK(ui_is_dirty());
    CHECK_EQ_INT(ui_dirty_band_count(), 1);
    CHECK(ui_dirty_band(0, &y0, &y1));
    CHECK_EQ_INT(y0, 0);
    CHECK_EQ_INT(y1, DISPLAY_H - 1);

    /* render: one draw pass, one flush of the dirty span */
    ui_render();
    CHECK_EQ_INT(s_draws, 1);
    CHECK_EQ_INT(mock_display_flush_count(), 1);
    CHECK_EQ_INT(mock_display_flush(0)->y0, 0);
    CHECK_EQ_INT(mock_display_flush(0)->y1, DISPLAY_H - 1);
    CHECK(!ui_is_dirty());

    /* nothing dirty: no draw, no flush */
    ui_render();
    CHECK_EQ_INT(s_draws, 1);
    CHECK_EQ_INT(mock_display_flush_count(), 1);

    /* two disjoint bands flush separately */
    mock_display_reset();
    ui_invalidate(0, 10, DISPLAY_W - 1, 20);
    ui_invalidate(0, 100, DISPLAY_W - 1, 110);
    CHECK_EQ_INT(ui_dirty_band_count(), 2);
    ui_render();
    CHECK_EQ_INT(s_draws, 2);
    CHECK_EQ_INT(mock_display_flush_count(), 2);
    CHECK_EQ_INT(mock_display_flush(0)->y0, 10);
    CHECK_EQ_INT(mock_display_flush(0)->y1, 20);
    CHECK_EQ_INT(mock_display_flush(1)->y0, 100);
    CHECK_EQ_INT(mock_display_flush(1)->y1, 110);

    /* overlapping bands merge into one */
    mock_display_reset();
    ui_invalidate(0, 10, DISPLAY_W - 1, 20);
    ui_invalidate(0, 18, DISPLAY_W - 1, 30);
    CHECK_EQ_INT(ui_dirty_band_count(), 1);
    ui_render();
    CHECK_EQ_INT(mock_display_flush_count(), 1);
    CHECK_EQ_INT(mock_display_flush(0)->y0, 10);
    CHECK_EQ_INT(mock_display_flush(0)->y1, 30);

    /* adjacent bands (touching rows) merge too */
    mock_display_reset();
    ui_invalidate(0, 10, DISPLAY_W - 1, 20);
    ui_invalidate(0, 21, DISPLAY_W - 1, 30);
    CHECK_EQ_INT(ui_dirty_band_count(), 1);
    CHECK(ui_dirty_band(0, &y0, &y1));
    CHECK_EQ_INT(y0, 10);
    CHECK_EQ_INT(y1, 30);

    /* out-of-range rows are clamped */
    ui_render();               /* clear pending dirty state first */
    mock_display_reset();
    ui_invalidate(0, -5, DISPLAY_W - 1, 3);
    ui_invalidate(0, DISPLAY_H - 2, DISPLAY_W - 1, DISPLAY_H + 99);
    CHECK_EQ_INT(ui_dirty_band_count(), 2);
    CHECK(ui_dirty_band(0, &y0, &y1));
    CHECK_EQ_INT(y0, 0);
    CHECK_EQ_INT(y1, 3);
    CHECK(ui_dirty_band(1, &y0, &y1));
    CHECK_EQ_INT(y0, DISPLAY_H - 2);
    CHECK_EQ_INT(y1, DISPLAY_H - 1);

    /* band-set overflow degrades to fewer bounding bands, never > MAX */
    ui_render();               /* clear pending dirty state first */
    mock_display_reset();
    for (y0 = 0; y0 < 20; y0 += 2)
        ui_invalidate(0, y0, DISPLAY_W - 1, y0);
    CHECK(ui_dirty_band_count() <= 8);
    CHECK(ui_dirty_band(0, &y0, &y1));
    CHECK_EQ_INT(y0, 0);
    CHECK(ui_dirty_band(ui_dirty_band_count() - 1, &y0, &y1));
    CHECK_EQ_INT(y1, 18);
}
