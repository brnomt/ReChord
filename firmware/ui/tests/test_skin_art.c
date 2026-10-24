/*
 * test_skin_art.c — the `art` meter and the art_* variables.
 *
 * The engine never touches storage: the UI adapter registers a source with a
 * palette and a draw() hook. This test drives a fake source and checks the
 * element contract (position/size handed to the hook, art_c1/art_c2/... in
 * color= expressions, placeholder when there is no art or draw() fails).
 */
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "ui.h"
#include "mock_display.h"
#include "mock_services.h"
#include "../../theme/theme.h"
#include "../../theme/theme_skin.h"

static theme_skin_art_t s_src;
static int s_draw_calls;
static int s_draw_x, s_draw_y, s_draw_large;
static int s_draw_rc;

static int fake_draw(int x, int y, int large)
{
    int dim = large ? s_src.large_px : s_src.small_px;

    s_draw_calls++;
    s_draw_x = x;
    s_draw_y = y;
    s_draw_large = large;
    if (s_draw_rc == 0 && dim > 0)
        display_fill_rect(x, y, x + dim - 1, y + dim - 1, 0xBEEF);
    return s_draw_rc;
}

static int write_skin(void)
{
    FILE *f = fopen("rechord_art.skin", "w");

    if (f == 0)
        return -1;
    fputs(
        "[bg]\n"
        "type=fill\nx=0\ny=0\nw=W\nh=4\ncolor=art_c1\nvisible=art_ok\n"
        "\n"
        "[cover]\n"
        "type=art\nx=10\ny=20\nsize=large\n"
        "\n"
        "[cover_small]\n"
        "type=art\nx=200\ny=100\nsize=small\n"
        "\n"
        "[dot]\n"
        "type=fill\nx=0\ny=6\nw=2\nh=2\ncolor=art_accent\n",
        f);
    fclose(f);
    return 0;
}

/* Is there a recorded rect exactly covering (x0,y0)..(x1,y1) in `color`? */
static int has_rect(int x0, int y0, int x1, int y1, uint16_t color)
{
    int i, n = mock_display_rect_count();

    for (i = 0; i < n; i++) {
        const mock_rect_t *r = mock_display_rect(i);

        if (r->x0 == x0 && r->y0 == y0 && r->x1 == x1 && r->y1 == y1 &&
            r->color == color)
            return 1;
    }
    return 0;
}

static int has_rect_color(uint16_t color)
{
    int i, n = mock_display_rect_count();

    for (i = 0; i < n; i++)
        if (mock_display_rect(i)->color == color)
            return 1;
    return 0;
}

/* A rounded rect is emitted one row at a time; a middle row is full width. */
static int has_full_row(int x0, int y, int x1, uint16_t color)
{
    return has_rect(x0, y, x1, y, color);
}

static void render(void)
{
    theme_skin_state_t st;

    memset(&st, 0, sizeof st);
    mock_display_reset();
    theme_skin_render(1000, 0, 0, 0, &st);
}

void test_skin_art(void)
{
    ui_init();
    mock_display_reset();
    mock_services_reset();
    CHECK_EQ_INT(write_skin(), 0);
    CHECK_EQ_INT(theme_skin_load(".", "", "rechord_art.skin"), 0);

    memset(&s_src, 0, sizeof s_src);
    s_src.ok = 1;
    s_src.c1 = 0x1234;
    s_src.c2 = 0x5678;
    s_src.mean = 0x9ABC;
    s_src.accent = 0xDEF0;
    s_src.large_px = 8;
    s_src.small_px = 4;
    s_src.draw = fake_draw;
    theme_skin_set_art(&s_src);
    s_draw_calls = 0;
    s_draw_rc = 0;

    /* ---- art available: both sizes are handed to the hook -------------- */
    render();
    CHECK_EQ_INT(s_draw_calls, 2);
    CHECK_EQ_INT(s_draw_x, 200);          /* last call: the small cover */
    CHECK_EQ_INT(s_draw_y, 100);
    CHECK_EQ_INT(s_draw_large, 0);
    CHECK(has_rect(10, 20, 17, 27, 0xBEEF));     /* fake large pixels  */
    CHECK(has_rect(200, 100, 203, 103, 0xBEEF)); /* fake small pixels  */

    /* art_c1 in color=, art_accent in color= */
    CHECK(has_rect(0, 0, DISPLAY_W - 1, 3, 0x1234));
    CHECK(has_rect(0, 6, 1, 7, 0xDEF0));

    /* ---- no art for this track: placeholder, no background ------------- */
    s_src.ok = 0;
    s_draw_calls = 0;
    render();
    CHECK_EQ_INT(s_draw_calls, 0);
    CHECK(!has_rect_color(0x1234));       /* visible=art_ok is false */
    CHECK(has_full_row(10, 21, 17, theme_get()->surface));       /* 8x8  */
    CHECK(has_rect(200, 100, 203, 103, theme_get()->surface));   /* 4x4  */
    CHECK(has_rect(0, 6, 1, 7, 0xDEF0));  /* art_* still readable */

    /* ---- art present but the stream fails: placeholder too -------------- */
    s_src.ok = 1;
    s_draw_calls = 0;
    s_draw_rc = -1;
    render();
    CHECK_EQ_INT(s_draw_calls, 2);
    CHECK(has_full_row(10, 21, 17, theme_get()->surface));
    CHECK(has_rect(0, 0, DISPLAY_W - 1, 3, 0x1234));

    /* ---- no source registered at all: art_ok is 0, default sizes -------- */
    theme_skin_set_art(0);
    s_draw_rc = 0;
    render();
    CHECK(!has_rect_color(0x1234));
    CHECK(has_full_row(10, 80, 10 + 120 - 1, theme_get()->surface));
    CHECK(has_full_row(200, 124, 200 + 48 - 1, theme_get()->surface));

    (void)remove("rechord_art.skin");
}
