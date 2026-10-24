/*
 * test_skin_engine.c — the small generic additions the cassette theme needs:
 *   - `gradient` (vertical/diagonal, 4x4 dithering, rect-bounded)
 *   - sqrt() in expressions
 *   - `maxc` on the text meter (hard truncation with '~')
 */
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "ui.h"
#include "mock_display.h"
#include "mock_services.h"
#include "../../theme/theme_skin.h"

static int write_skin(void)
{
    FILE *f = fopen("rechord_engine.skin", "w");

    if (f == 0)
        return -1;
    fputs(
        "[grad]\n"
        "type=gradient\nx=100\ny=0\nw=8\nh=4\n"
        "from=0\nto=65535\ndir=vertical\n"
        "\n"
        "[diag]\n"
        "type=gradient\nx=10\ny=40\nw=8\nh=4\n"
        "from=0\nto=65535\ndir=diagonal\n"
        "\n"
        "[sq]\n"
        "type=fill\nx=sqrt(169)\ny=sqrt(81)\nw=2\nh=2\ncolor=63488\n"
        "\n"
        "[txt]\n"
        "type=text\nx=0\ny=10\ntext=\"ABCDEFGHIJ\"\nmaxc=5\n"
        "color=65535\nbg=0\n"
        "\n"
        "[txt2]\n"
        "type=text\nx=0\ny=20\ntext=\"AB\"\nmaxc=5\n"
        "color=65535\nbg=0\n"
        "\n"
        "[txt3]\n"
        "type=text\nx=100\ny=30\ntext=\"WXYZ\"\nalign=right\n"
        "color=65535\nbg=0\n",
        f);
    fclose(f);
    return 0;
}

static int pixel_at(int x, int y)
{
    int i, n = mock_display_rect_count();

    for (i = 0; i < n; i++) {
        const mock_rect_t *r = mock_display_rect(i);

        if (r->x0 == x && r->y0 == y && r->x1 == x && r->y1 == y)
            return r->color;
    }
    return -1;
}

void test_skin_engine(void)
{
    theme_skin_state_t st;

    ui_init();
    mock_display_reset();
    mock_services_reset();
    CHECK_EQ_INT(write_skin(), 0);
    CHECK_EQ_INT(theme_skin_load(".", "", "rechord_engine.skin"), 0);
    memset(&st, 0, sizeof st);

    mock_display_reset();
    theme_skin_render(1000, 0, 0, 0, &st);

    /* ---- sqrt(): sqrt(169)=13, sqrt(81)=9 -> the 2x2 red square --------- */
    {
        int i, found = 0;
        int n = mock_display_rect_count();

        for (i = 0; i < n; i++) {
            const mock_rect_t *r = mock_display_rect(i);

            if (r->x0 == 13 && r->y0 == 9 && r->x1 == 14 && r->y1 == 10 &&
                r->color == 63488)
                found = 1;
        }
        CHECK(found);
    }

    /* ---- vertical gradient: dark at the top, light at the bottom -------- */
    CHECK(pixel_at(100, 0) >= 0 && pixel_at(100, 0) < 0x0100);
    CHECK(pixel_at(100, 3) > 0xE000);     /* dithering keeps it just below FFFF */
    /* only inside its rect */
    CHECK_EQ_INT(pixel_at(99, 0), -1);
    CHECK_EQ_INT(pixel_at(108, 0), -1);
    CHECK_EQ_INT(pixel_at(100, 4), -1);

    /* ---- diagonal gradient: the far corner is the brightest ------------- */
    {
        int a = pixel_at(10, 40);
        int b = pixel_at(17, 43);

        CHECK(a >= 0 && a < 0x0100);
        CHECK(b > 0xE000);
    }

    /* ---- maxc: long text is cut to 4 chars + '~'; short text untouched -- */
    {
        int i;
        int found = 0, plain = 0, right = 0;

        for (i = 0; i < mock_display_text_count(); i++) {
            const mock_text_t *t = mock_display_text(i);

            if (t->y == 10 && strcmp(t->text, "ABCD~") == 0)
                found = 1;
            if (t->y == 20 && strcmp(t->text, "AB") == 0)
                plain = 1;
            /* align=right: x is the right edge -> 100 - 4*8 = 68 */
            if (t->y == 30 && strcmp(t->text, "WXYZ") == 0 && t->x == 68)
                right = 1;
        }
        CHECK(found);
        CHECK(plain);
        CHECK(right);
    }

    (void)remove("rechord_engine.skin");
}
