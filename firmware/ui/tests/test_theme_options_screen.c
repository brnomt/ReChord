/*
 * test_theme_options_screen.c — Settings > Interface > Theme, long press.
 *
 * The Theme row keeps its short-press behaviour (cycle the package) and gains
 * a long press that opens "Theme options" for the ACTIVE package:
 *   - it must not switch the theme,
 *   - a press on a row cycles that option and persists it,
 *   - BACK returns to the group screen,
 *   - a package without options never opens the screen,
 *   - the screen draws through the settings skin when the package has one and
 *     through the built-in list otherwise.
 *
 * The theme layout is injected directly (theme_layout_set) so the test owns
 * the declaration; the package itself is data (see test_theme_options.c).
 */
#include <stdio.h>
#include <string.h>

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

/* One event. `long_hold` marks a held press: PRESS, LONG, then a RELEASE with
 * long_press=1, which is what services/input.c emits. */
static input_event_t ev2(input_key_t key, input_kind_t kind, int long_hold)
{
    input_event_t e;

    e.type = key;
    e.adc_value = -1;
    e.kind = kind;
    e.long_press = (kind == INPUT_RELEASE) ? long_hold : 0;
    return e;
}

static input_event_t ev(input_key_t key, input_kind_t kind)
{
    return ev2(key, kind, 0);
}

/* A complete short press: PRESS then RELEASE{long_press=0}. */
static void tap(input_key_t key)
{
    input_event_t e = ev(key, INPUT_PRESS);

    ui_handle_event(&e);
    e = ev(key, INPUT_RELEASE);
    ui_handle_event(&e);
}

/* A complete long press: PRESS, LONG, RELEASE{long_press=1}. */
static void hold(input_key_t key)
{
    input_event_t e = ev(key, INPUT_PRESS);

    ui_handle_event(&e);
    e = ev2(key, INPUT_LONG, 1);
    ui_handle_event(&e);
    e = ev2(key, INPUT_RELEASE, 1);
    ui_handle_event(&e);
}

/* Install a layout that declares two options (Face, Refresh). */
static void install_options(int declare)
{
    theme_layout_t l;

    theme_layout_defaults(&l);
    if (declare) {
        (void)theme_layout_parse(
            "opt_face=Face,Clock|Artwork\n"
            "opt_fps=Refresh,20 fps|60 fps\n", &l);
    }
    theme_layout_set(&l);
}

/* Open Settings > Interface and put the cursor on the Theme row (index 2). */
static void open_interface_theme_row(void)
{
    input_event_t e;

    ui_init();
    mock_display_reset();
    mock_services_reset();
    theme_ui_disarm();                   /* no package skin: built-in list */
    settings_screen_reload();
    CHECK(ui_push_screen(&settings_screen));

    e = ev(KEY_DOWN, INPUT_PRESS);       /* Playback -> Interface */
    ui_handle_event(&e);
    e = ev(KEY_SELECT, INPUT_PRESS);     /* enter Interface */
    ui_handle_event(&e);
    CHECK_EQ_INT(ui_stack_depth(), 2);

    e = ev(KEY_DOWN, INPUT_PRESS);
    ui_handle_event(&e);
    e = ev(KEY_DOWN, INPUT_PRESS);
    ui_handle_event(&e);
    CHECK_EQ_INT(settings_group_view()->cursor, 2);   /* "Theme" */
    CHECK_STR_EQ(settings_group_model()->items[2].label, "Theme");
}

void test_theme_options_screen(void)
{
    input_event_t e;

    /* ---- a package with options: long press opens the list ------------- */
    install_options(1);
    open_interface_theme_row();

    /* The key driver emits PRESS immediately and LONG only ~500 ms later, so
     * a hold arrives as PRESS, LONG, RELEASE{long_press=1}. A short press
     * cycles the theme; a hold must NOT (that was the reported bug: holding
     * PLAY switched the package and then opened the wrong theme's options). */
    hold(KEY_SELECT);
    CHECK_EQ_INT(ui_stack_depth(), 3);
    CHECK_STR_EQ(ui_top_screen()->name, "theme-options");
    /* It must NOT have cycled the theme on the way in. */
    CHECK_STR_EQ(theme_pkg_active_id(), "808");
    CHECK_EQ_INT(theme_layout()->opt_count, 2);

    /* The list shows the declared labels and current choices, built-in
     * renderer because the test package has no settings skin. */
    CHECK_EQ_INT(theme_skin_text() == 0, 1);
    mock_display_reset();
    ui_invalidate_all();
    ui_render();
    CHECK(mock_display_text_find("Face") >= 0);
    CHECK(mock_display_text_find("Clock") >= 0);
    CHECK(mock_display_text_find("Refresh") >= 0);
    CHECK(mock_display_text_find("20 fps") >= 0);

    /* Tap on the first row cycles Face -> Artwork and persists. */
    tap(KEY_SELECT);
    CHECK_EQ_INT(theme_opt_value("face"), 1);
    /* ...and the re-apply it triggers must not reset the value it just set:
     * theme_pkg_apply() re-reads the manifest, so the selection has to come
     * back from the persisted digits, not from the freshly parsed ones. */
    CHECK_STR_EQ(theme_layout()->opts[0].choice[theme_opt_value("face")],
                 "Artwork");
    CHECK_EQ_INT(theme_opt_value("fps"), 0);       /* other rows untouched */
    CHECK(mock_settings_save_count() >= 1);
    {
        rechord_settings_t saved;

        mock_settings_get(&saved);
        CHECK_STR_EQ(saved.theme_opts, "808:10;");
    }
    mock_display_reset();
    ui_invalidate_all();
    ui_render();
    CHECK(mock_display_text_find("Artwork") >= 0);

    /* Moving down and tapping cycles Refresh, keeping Face. */
    e = ev(KEY_DOWN, INPUT_PRESS);
    ui_handle_event(&e);
    tap(KEY_SELECT);
    CHECK_EQ_INT(theme_opt_value("fps"), 1);
    CHECK_EQ_INT(theme_opt_value("face"), 1);
    mock_display_reset();
    ui_invalidate_all();
    ui_render();
    CHECK(mock_display_text_find("60 fps") >= 0);

    /* Wrapping: another tap goes back to the first choice. */
    tap(KEY_SELECT);
    CHECK_EQ_INT(theme_opt_value("fps"), 0);

    /* A HOLD on a row must change nothing: with a 2-choice value the repeats
     * used to flip it back and forth. */
    hold(KEY_SELECT);
    CHECK_EQ_INT(theme_opt_value("fps"), 0);
    CHECK_EQ_INT(ui_stack_depth(), 3);

    /* BACK returns to the Settings group (whose on_enter resets the cursor,
     * exactly like every other Settings sub-screen). */
    e = ev(KEY_BACK, INPUT_PRESS);
    ui_handle_event(&e);
    CHECK_EQ_INT(ui_stack_depth(), 2);
    CHECK_STR_EQ(ui_top_screen()->name, "settings-group");
    CHECK_EQ_INT(settings_group_view()->cursor, 0);

    /* Long press is inert on rows that are not Theme (no stray screen).
     * BACK resets the cursor to the top, so walk DOWN to "Brightness". */
    e = ev(KEY_BACK, INPUT_PRESS);
    ui_handle_event(&e);              /* Interface -> Settings categories */
    e = ev(KEY_SELECT, INPUT_PRESS);
    ui_handle_event(&e);              /* back into Interface */
    CHECK_EQ_INT(settings_group_view()->cursor, 0);   /* Brightness */
    CHECK_STR_EQ(settings_group_model()->items[0].label, "Brightness");
    hold(KEY_SELECT);
    CHECK_EQ_INT(ui_stack_depth(), 2);
    CHECK_STR_EQ(ui_top_screen()->name, "settings-group");

    /* ---- a package without options: the long press does nothing ------- */
    install_options(0);
    open_interface_theme_row();
    hold(KEY_SELECT);
    CHECK_EQ_INT(ui_stack_depth(), 2);
    CHECK_EQ_INT(theme_options_open(), 0);

    theme_layout_reset();
}
