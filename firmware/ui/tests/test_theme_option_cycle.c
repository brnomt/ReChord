/*
 * test_theme_option_cycle.c — the option-change path through the REAL screen.
 *
 * The other options test injects the layout directly, which cannot catch the
 * bug this file exists for: settings_screen_theme_option_apply() calls
 * theme_pkg_apply(), which RE-READS theme.cfg and re-initialises every option
 * to choice 0. The new value only survives if the change is SAVED first and
 * the provider restores it. With the two calls the other way round, the press
 * looked like it did nothing — reported as "I press the refresh rate and it
 * doesn't switch to 60". Neither Glifo nor Oscilloscope could change options.
 *
 * This drives the real screen: a package in a temporary tree, a long press on
 * the Theme row, then taps on the option row, asserting that the value, the
 * persisted `theme_opts` text and the skin's own `opt_fps` all moved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "test_util.h"
#include "ui.h"
#include "settings_schema.h"
#include "screens/screens.h"
#include "screens/theme_ui.h"
#include "../../theme/theme.h"
#include "../../theme/theme_pkg.h"
#include "../../theme/theme_runtime.h"
#include "../../theme/theme_skin.h"
#include "mock_display.h"
#include "mock_services.h"

#define ROOT "opt_cycle_root"
#define ID   "cyc"

static input_event_t ev(input_key_t key, input_kind_t kind, int long_press)
{
    input_event_t e;

    e.type = key;
    e.adc_value = -1;
    e.kind = kind;
    e.long_press = long_press;
    return e;
}

/* A complete short press: PRESS then RELEASE{long_press=0}. */
static void tap(input_key_t key)
{
    input_event_t e = ev(key, INPUT_PRESS, 0);

    ui_handle_event(&e);
    e = ev(key, INPUT_RELEASE, 0);
    ui_handle_event(&e);
}

/* A complete hold: PRESS, LONG, RELEASE{long_press=1}. */
static void hold(input_key_t key)
{
    input_event_t e = ev(key, INPUT_PRESS, 0);

    ui_handle_event(&e);
    e = ev(key, INPUT_LONG, 1);
    ui_handle_event(&e);
    e = ev(key, INPUT_RELEASE, 1);
    ui_handle_event(&e);
}

static int write_file(const char *path, const char *text)
{
    FILE *f = fopen(path, "w");

    if (f == 0)
        return -1;
    if (fputs(text, f) < 0) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

static void make_package(void)
{
    char d[64];

    if (system("rm -rf " ROOT) != 0) { /* best effort */ }
    CHECK_EQ_INT(mkdir(ROOT, 0777), 0);
    snprintf(d, sizeof d, ROOT "/" ID);
    CHECK_EQ_INT(mkdir(d, 0777), 0);
    CHECK_EQ_INT(write_file(ROOT "/" ID "/theme.cfg",
        "format=1\nname=Cycle\nhome=skin\nskin=home.skin\n"
        "opt_fps=Refresh,20 fps|60 fps\n"), 0);
    /* The skin's own view of the option: refresh_ms follows it. */
    CHECK_EQ_INT(write_file(ROOT "/" ID "/home.skin",
        "[skin]\nrefresh_ms=(opt_fps?16:50)\n"
        "[a]\ntype=fill\nx=0\ny=0\nw=2\nh=2\ncolor=text\n"), 0);
}

/* Settings > Interface, cursor on "Theme" (row 2). */
static void open_theme_row(void)
{
    ui_init();
    mock_display_reset();
    mock_services_reset();
    theme_ui_disarm();
    settings_screen_reload();
    settings_screen_theme_options_init();     /* installs the provider */
    settings_screen_set_themes_root(ROOT);

    CHECK(ui_push_screen(&settings_screen));
    tap(KEY_DOWN);                 /* Playback -> Interface */
    tap(KEY_SELECT);               /* enter Interface */
    CHECK_EQ_INT(ui_stack_depth(), 2);
    tap(KEY_DOWN);
    tap(KEY_DOWN);
    CHECK_EQ_INT(settings_group_view()->cursor, 2);
    CHECK_STR_EQ(settings_group_model()->items[2].label, "Theme");
}

void test_theme_option_cycle(void)
{
    make_package();

    /* Apply as the app does at boot, with the provider already installed. */
    ui_init();
    mock_services_reset();
    settings_screen_reload();
    settings_screen_theme_options_init();
    CHECK_EQ_INT(theme_pkg_apply(ROOT, ID), 0);
    CHECK_EQ_INT(theme_opt_value("fps"), 0);
    CHECK_EQ_INT(theme_skin_refresh_ms(), 50);

    open_theme_row();

    /* A hold opens the options; it must NOT cycle the theme (the other
     * reported bug) — the active id has to still be ours. */
    hold(KEY_SELECT);
    CHECK_EQ_INT(ui_stack_depth(), 3);
    CHECK_STR_EQ(ui_top_screen()->name, "theme-options");
    CHECK_STR_EQ(theme_pkg_active_id(), ID);
    CHECK(theme_options_open() == 1);   /* still the same screen, no push */

    /* Tap the row: the value and the cfg must both move. (The engine is left
     * holding whichever skin the screen loaded, so the skin-level check is
     * done below by loading home.skin, exactly as the home screen would.) */
    tap(KEY_SELECT);
    CHECK_EQ_INT(theme_opt_value("fps"), 1);
    CHECK_EQ_INT(theme_skin_load(ROOT, ID, "home.skin"), 0);
    CHECK_EQ_INT(theme_skin_refresh_ms(), 16);
    {
        rechord_settings_t saved;

        mock_settings_get(&saved);
        CHECK_STR_EQ(saved.theme_opts, "cyc:1;");
    }

    /* Tap again: wraps back to the first choice, not stuck. */
    tap(KEY_SELECT);
    CHECK_EQ_INT(theme_opt_value("fps"), 0);
    CHECK_EQ_INT(theme_skin_load(ROOT, ID, "home.skin"), 0);
    CHECK_EQ_INT(theme_skin_refresh_ms(), 50);

    /* A HOLD on the row changes nothing (a 2-choice value used to flip back
     * and forth under REPEAT). */
    hold(KEY_SELECT);
    CHECK_EQ_INT(theme_opt_value("fps"), 0);

    /* Persistence across a fresh apply, which is what a power cycle does. */
    tap(KEY_SELECT);
    CHECK_EQ_INT(theme_opt_value("fps"), 1);
    CHECK_EQ_INT(theme_pkg_apply(ROOT, ID), 0);
    CHECK_EQ_INT(theme_opt_value("fps"), 1);
    /* theme_pkg_apply() loads the package's MAIN skin itself. */
    CHECK_EQ_INT(theme_skin_refresh_ms(), 16);

    settings_screen_set_themes_root(0);
    if (system("rm -rf " ROOT) != 0) { /* best effort */ }
}
