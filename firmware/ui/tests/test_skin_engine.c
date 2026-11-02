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
        "color=65535\nbg=0\n"
        "\n"
        "[btmark]\n"
        "type=fill\nx=200\ny=60\nw=1\nh=1\ncolor=63488\nvisible=bt\n"
        "\n"
        /* && / || must consume their right operand even when the left side
         * already decides the result, or a following ?: is left unparsed. */
        "[bool1]\n"
        "type=text\nx=0\ny=40\ntext=((1<2&&1<2)?\"OKA\":\"NOA\")\n"
        "color=65535\nbg=0\n"
        "[bool2]\n"
        "type=text\nx=0\ny=50\ntext=((1<2||1>2)?\"OKB\":\"NOB\")\n"
        "color=65535\nbg=0\n"
        "[bool3]\n"
        "type=text\nx=0\ny=60\ntext=((1>2&&1<2)?\"OKC\":\"NOC\")\n"
        "color=65535\nbg=0\n"
        "[bool4]\n"
        "type=text\nx=0\ny=70\ntext=((1>2||1>2)?\"OKD\":\"NOD\")\n"
        "color=65535\nbg=0\n",
        f);
    fclose(f);
    return 0;
}

static int has_text_exact(const char *s)
{
    int i, n = mock_display_text_count();

    for (i = 0; i < n; i++)
        if (strcmp(mock_display_text(i)->text, s) == 0)
            return 1;
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

    /* ---- Bluetooth state variable (bt: 0 off, 1 on, 2 connected) -------- */
    {
        theme_skin_state_t bst = st;

        bst.bt = 0;
        mock_display_reset();
        theme_skin_render(2000, 0, 0, 0, &bst);
        CHECK(pixel_at(200, 60) < 0);   /* bt=0 hides the indicator */

        bst.bt = 2;
        mock_display_reset();
        theme_skin_render(2000, 0, 0, 0, &bst);
        CHECK_EQ_INT(pixel_at(200, 60), 63488);  /* bt=2 shows it */
    }

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

    /* ---- && / || inside a ?: condition (parser regression) ------------- */
    CHECK(has_text_exact("OKA"));
    CHECK(has_text_exact("OKB"));
    CHECK(has_text_exact("NOC"));
    CHECK(has_text_exact("NOD"));

    /* ---- static/dynamic split: dyn=1 sections and their rects ---------- */
    {
        FILE *f2 = fopen("rechord_dyn.skin", "w");

        if (f2 != 0) {
            fputs("[skin]\n"
                  "refresh_ms=50\n"
                  "dyn=2,2,6,6\n"
                  "dyn2=10,10,12,12\n"
                  "[still]\n"
                  "type=fill\nx=0\ny=0\nw=2\nh=2\ncolor=63488\n"
                  "[moving]\n"
                  "type=fill\nx=3\ny=3\nw=2\nh=2\ndyn=1\ncolor=2016\n",
                  f2);
            fclose(f2);
            CHECK_EQ_INT(theme_skin_load(".", "", "rechord_dyn.skin"), 0);
            {
                int x0, y0, x1, y1;

                CHECK_EQ_INT(theme_skin_dyn_rect(0, &x0, &y0, &x1, &y1), 0);
                CHECK_EQ_INT(x0, 2);
                CHECK_EQ_INT(y0, 2);
                CHECK_EQ_INT(x1, 6);
                CHECK_EQ_INT(y1, 6);
                CHECK_EQ_INT(theme_skin_dyn_rect(1, &x0, &y0, &x1, &y1), 0);
                CHECK_EQ_INT(x1, 12);
                CHECK_EQ_INT(theme_skin_dyn_rect(2, &x0, &y0, &x1, &y1), -1);
            }
            mock_display_reset();
            theme_skin_set_dynamic_only(1);
            theme_skin_render(1000, 0, 0, 0, &st);
            theme_skin_set_dynamic_only(0);
            /* only the dyn section reached the display */
            {
                int i, n = mock_display_rect_count(), saw_still = 0;
                int saw_moving = 0;

                for (i = 0; i < n; i++) {
                    const mock_rect_t *r = mock_display_rect(i);

                    if (r->color == 63488)
                        saw_still = 1;
                    if (r->color == 2016)
                        saw_moving = 1;
                }
                CHECK_EQ_INT(saw_still, 0);
                CHECK_EQ_INT(saw_moving, 1);
            }
            /* theme_skin_dyn_rect() is only meaningful for the loaded skin;
             * reloading the engine skin in the later checks keeps the rest of
             * this test valid. */
            CHECK_EQ_INT(theme_skin_load(".", "", "rechord_engine.skin"), 0);
        }
    }


    /* ---- audio tap: lvl_l/peak_r and wl()/wr() waveform lookups ---------- */
    {
        FILE *fa = fopen("rechord_audio.skin", "w");
        static const short wl[3] = { 100, -200, 300 };
        static const short wr[3] = { -7, 8, 9 };
        theme_skin_state_t as;

        if (fa != 0) {
            fputs("[a]\ntype=text\nx=0\ny=0\n"
                  "text=lvl_l + \"/\" + peak_r + \"/\" + wl(1) + \"/\" + wr(2)"
                  " + \"/\" + wl(5) + \"/\" + wl(-1)\ncolor=65535\nbg=0\n", fa);
            fclose(fa);
        }
        memset(&as, 0, sizeof as);
        as.lvl_l = 640;
        as.peak_r = 912;
        as.wave_l = wl;
        as.wave_r = wr;
        as.wave_n = 3;
        CHECK_EQ_INT(theme_skin_load(".", "", "rechord_audio.skin"), 0);
        mock_display_reset();
        theme_skin_render(0, 0, 0, 0, &as);
        CHECK(has_text_exact("640/912/-200/9/0/0"));
        as.wave_l = 0;                   /* no tap: lookups are 0, no crash */
        mock_display_reset();
        theme_skin_render(0, 0, 0, 0, &as);
        CHECK(has_text_exact("640/912/0/9/0/0"));
        (void)remove("rechord_audio.skin");
    }


    (void)remove("rechord_engine.skin");
    (void)remove("rechord_dyn.skin");
}
