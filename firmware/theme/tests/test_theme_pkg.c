/*
 * test_theme_pkg.c — host test for the layout DSL (BRNOMT-69).
 *
 * Covers: the built-in defaults, partial overrides, comments/sections and
 * unknown keys being ignored, and out-of-range/malformed values rejecting the
 * WHOLE layout (the caller keeps the previous one).
 *
 * Compile (host):
 *   cc -Wall -Werror -O2 -o test_theme_pkg \
 *      firmware/theme/tests/test_theme_pkg.c firmware/theme/theme_pkg.c
 */
#include <stdio.h>
#include <string.h>

#include "../theme_pkg.h"

static int checks;
static int fails;

#define CHECK(cond) do {                                                   \
    checks++;                                                              \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        fails++;                                                           \
    }                                                                      \
} while (0)

int main(void)
{
    theme_layout_t l;

    printf("== theme layout (package DSL) ==\n");

    /* 1. defaults are the pre-package geometry. */
    theme_layout_defaults(&l);
    CHECK(l.pad == 4 && l.title_h == 16 && l.item_h == 12);
    CHECK(l.np_title_y == 24 && l.np_artist_y == 36 && l.np_state_y == 48);
    CHECK(l.np_info_y == 60 && l.np_bar_y == 74 && l.np_bar_h == 2);
    CHECK(l.np_time_y == 84 && l.np_mode_y == 148);
    CHECK(l.np_vol_y == 162 && l.np_vol_h == 2);

    /* 2. partial override: only the declared keys move. */
    CHECK(theme_layout_parse("# c\n[now]\npad=6\nnp_bar_h=4\nunknown=1\n",
                             &l) == 0);
    CHECK(l.pad == 6 && l.np_bar_h == 4);
    CHECK(l.title_h == 16 && l.item_h == 12);      /* untouched */

    /* 3. out-of-range rejects the whole layout. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("pad=6\nnp_bar_h=0\n", &l) != 0);
    CHECK(l.pad == 4 && l.np_bar_h == 2);          /* unchanged */

    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("pad=99\n", &l) != 0);
    CHECK(l.pad == 4);

    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("np_title_y=999\n", &l) != 0);
    CHECK(l.np_title_y == 24);

    /* 4. malformed number rejects (no silent guessing). */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("item_h=12x\n", &l) != 0);
    CHECK(l.item_h == 12);

    /* 5. a full package parses and applies. */
    {
        static const char *pkg =
            "format=1\nname=Nothing Mono\nbackground=0x0000\ntext=0xFFFF\n"
            "pad=5\ntitle_h=18\nitem_h=13\nnp_title_y=26\nnp_artist_y=38\n"
            "np_state_y=50\nnp_info_y=62\nnp_bar_y=76\nnp_bar_h=3\n"
            "np_time_y=88\nnp_mode_y=150\nnp_vol_y=164\nnp_vol_h=3\n";

        theme_layout_defaults(&l);
        CHECK(theme_layout_parse(pkg, &l) == 0);
        CHECK(l.pad == 5 && l.title_h == 18 && l.item_h == 13);
        CHECK(l.np_bar_y == 76 && l.np_bar_h == 3 && l.np_vol_y == 164);
    }

    /* 6. a missing file leaves the ACTIVE layout at the defaults. */
    CHECK(theme_layout_load("/nonexistent/rechord/theme.cfg") < 0);
    CHECK(theme_layout() != 0);
    CHECK(theme_layout()->pad == 4 && theme_layout()->np_bar_h == 2);

    printf("%d checks, %d failures\n", checks, fails);
    return fails == 0 ? 0 : 1;
}
