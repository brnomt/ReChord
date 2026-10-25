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
    CHECK(l.format == 1);
    CHECK(l.home == THEME_HOME_MENU && l.home_nav == THEME_NAV_VERTICAL);
    CHECK(l.input == THEME_INPUT_ENGINE);
    CHECK(l.pad == 4 && l.title_h == 16 && l.item_h == 12);
    CHECK(l.home_bar_y == 152 && l.home_bar_h == 14);
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

    /* 7. the package selects the generic skin renderer + its file. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("home=skin\nhome_nav=horizontal\n"
                             "skin=home.skin\n"
                             "face_h=112\nhome_bar_y=150\nhome_bar_h=14\n",
                             &l) == 0);
    CHECK(l.home == THEME_HOME_SKIN);
    CHECK(l.home_nav == THEME_NAV_HORIZONTAL);
    CHECK(strcmp(l.skin, "home.skin") == 0);
    CHECK(l.face_h == 112 && l.home_bar_y == 150 && l.home_bar_h == 14);

    /* `companion` stays accepted as a legacy alias for the same interpreter. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("home=companion\nskin=old.skin\n", &l) == 0);
    CHECK(l.home == THEME_HOME_SKIN && strcmp(l.skin, "old.skin") == 0);

    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("home=menu\nhome_nav=vertical\n", &l) == 0);
    CHECK(l.home == THEME_HOME_MENU && l.home_nav == THEME_NAV_VERTICAL);

    /* BRNOMT-107: input ownership is opt-in and validated atomically. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("input=theme\n", &l) == 0);
    CHECK(l.input == THEME_INPUT_THEME);
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("input=engine\n", &l) == 0);
    CHECK(l.input == THEME_INPUT_ENGINE);
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("input=junk\n", &l) != 0);
    CHECK(l.input == THEME_INPUT_ENGINE);

    /* 8. an unknown renderer/value rejects the whole layout. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("home=coverflow\n", &l) != 0);
    CHECK(l.home == THEME_HOME_MENU);
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("home=skin\nskin=home.skin\nhome_nav=diagonal\n",
                             &l) != 0);
    CHECK(l.home == THEME_HOME_MENU && l.home_nav == THEME_NAV_VERTICAL);

    /* 9. skin without a skin file, and unsafe paths, are rejected. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("home=skin\n", &l) != 0);
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("home=skin\nskin=../home.skin\n", &l) != 0);

    /* 9b. per-screen skins are independent paths; `skin_main` aliases the
     * home skin. Unsafe paths reject the whole package. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("skin_main=main.skin\n"
                             "skin_now_playing=np.skin\n"
                             "skin_library=lib.skin\n"
                             "skin_library_cat=cat.skin\n"
                             "skin_browser=files.skin\n"
                             "skin_settings=settings.skin\n", &l) == 0);
    CHECK(strcmp(l.skin, "main.skin") == 0);
    CHECK(strcmp(l.skin_now_playing, "np.skin") == 0);
    CHECK(strcmp(l.skin_library, "lib.skin") == 0);
    CHECK(strcmp(l.skin_library_cat, "cat.skin") == 0);
    CHECK(strcmp(l.skin_browser, "files.skin") == 0);
    CHECK(strcmp(l.skin_settings, "settings.skin") == 0);
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("skin_settings=../settings.skin\n", &l) != 0);

    /* 10. legacy manifests without a skin file stay on the classic menu. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("face_h=52\n", &l) == 0);
    CHECK(l.home == THEME_HOME_MENU);

    /* 11. package assets are declared, bounded and validated. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("asset_eye_open=assets/eye_open.bin,64,64\n"
                             "asset_eye_closed=assets/eye_closed.bin,64,6\n",
                             &l) == 0);
    CHECK(l.asset_count == 2);
    CHECK(strcmp(l.assets[0].name, "eye_open") == 0);
    CHECK(strcmp(l.assets[0].path, "assets/eye_open.bin") == 0);
    CHECK(l.assets[0].w == 64 && l.assets[0].h == 64);
    CHECK(l.assets[1].w == 64 && l.assets[1].h == 6);

    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("asset_x=assets/x.bin,65,8\n", &l) != 0);
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("asset_x=../x.bin,8,8\n", &l) != 0);
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("asset_x=a.bin,8,8\nasset_x=b.bin,8,8\n",
                             &l) != 0);

    /* 12. unsupported format versions reject. */
    theme_layout_defaults(&l);
    CHECK(theme_layout_parse("format=2\n", &l) != 0);
    CHECK(l.format == 1);

    /* 13. theme_layout_set commits a validated struct atomically. */
    theme_layout_set(&l);
    CHECK(theme_layout()->format == 1);
    theme_layout_defaults(&l);
    l.input = THEME_INPUT_THEME;
    theme_layout_set(&l);
    CHECK(theme_input_owns_keys() == 1);
    theme_layout_defaults(&l);
    theme_layout_set(&l);
    CHECK(theme_input_owns_keys() == 0);

    printf("%d checks, %d failures\n", checks, fails);
    return fails == 0 ? 0 : 1;
}
