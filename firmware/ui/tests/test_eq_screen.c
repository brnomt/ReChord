/*
 * test_eq_screen.c — Equalizer screen + profile picker (BRNOMT-76).
 *
 * The EQ service is mocked (mock_eq.c); this covers the screen contract:
 * rows, edit mode with the repeat-friendly UP/DOWN adjustment, persistence
 * on BACK, and the load/store flows of the profile picker.
 */
#include "test_util.h"
#include "ui.h"
#include "screens/screens.h"
#include "mock_display.h"
#include "eq_profile.h"

void mock_eq_reset(void);
extern int mock_eq_save_calls;
extern int mock_eq_select_row;
extern int mock_eq_store_row;

static input_event_t ev(input_key_t key)
{
    input_event_t e;

    e.type = key;
    e.adc_value = 0;
    e.kind = INPUT_PRESS;
    e.long_press = 0;
    return e;
}

static void press(input_key_t key)
{
    input_event_t e = ev(key);

    ui_handle_event(&e);
}

static void key_n(input_key_t key, int n)
{
    while (n-- > 0)
        press(key);
}

void test_eq_screen(void)
{
    mock_display_reset();
    mock_eq_reset();
    ui_init();

    CHECK(ui_push_screen(&eq_screen));
    ui_render();
    CHECK(mock_display_text_find("Equalizer") >= 0);
    CHECK(mock_display_text_find("Enabled") >= 0);
    CHECK(mock_display_text_find("Off") >= 0);
    CHECK(mock_display_text_find("Profile") >= 0);
    CHECK(mock_display_text_find("31 Hz") >= 0);
    CHECK(mock_display_text_find("1 kHz") >= 0);
    CHECK(mock_display_text_find("0.0") >= 0);

    /* Enabled row: SELECT toggles the effect. */
    press(KEY_SELECT);
    CHECK_EQ_INT(eq_profile_enabled(), 1);
    ui_render();
    CHECK(mock_display_text_find("On") >= 0);

    /* Move to Band 1 (rows: Enabled, Profile, Band 1). */
    key_n(KEY_DOWN, 2);
    press(KEY_SELECT);                 /* enter edit mode */
    ui_render();
    CHECK(mock_display_text_find("EDIT") >= 0);

    press(KEY_UP);
    CHECK_EQ_INT(eq_profile_curve()->band[0].gain_tdb, 5);
    ui_render();
    CHECK(mock_display_text_find("+0.5") >= 0);

    key_n(KEY_DOWN, 2);
    CHECK_EQ_INT(eq_profile_curve()->band[0].gain_tdb, -5);
    ui_render();
    CHECK(mock_display_text_find("-0.5") >= 0);

    press(KEY_SELECT);                 /* leave edit mode */
    mock_display_reset();
    ui_render();
    CHECK(mock_display_text_find("EDIT") < 0);

    /* LONG/RELEASE must not double-apply edits (the input service sends both
     * for a long press). */
    {
        input_event_t e = ev(KEY_DOWN);

        e.kind = INPUT_RELEASE;
        ui_handle_event(&e);
    }
    CHECK_EQ_INT(eq_profile_curve()->band[0].gain_tdb, -5);

    /* BACK leaves the screen and persists the curve. */
    press(KEY_BACK);
    CHECK_EQ_INT(ui_stack_depth(), 0);
    CHECK(mock_eq_save_calls >= 1);

    /* Profile picker: open from the Profile row, load a built-in. */
    mock_eq_select_row = -1;
    CHECK(ui_push_screen(&eq_screen));
    press(KEY_DOWN);                   /* Profile */
    press(KEY_SELECT);
    ui_render();
    CHECK_EQ_INT(ui_stack_depth(), 2);
    CHECK(mock_display_text_find("Rock") >= 0);
    CHECK(mock_display_text_find("Custom 1") >= 0);

    press(KEY_SELECT);                 /* Flat (row 0) */
    CHECK_EQ_INT(mock_eq_select_row, 0);
    CHECK_EQ_INT(ui_stack_depth(), 1);

    /* Save-current flow: move to the "Save current..." row, then pick a slot. */
    mock_eq_store_row = -1;
    press(KEY_SELECT);                 /* cursor still on Profile: picker */
    key_n(KEY_DOWN, eq_profile_count());   /* last row = Save current... */
    press(KEY_SELECT);                 /* enter store mode */
    ui_render();
    CHECK(mock_display_text_find("Save to slot") >= 0);
    CHECK(mock_display_text_find("Custom 1") >= 0);
    press(KEY_SELECT);                 /* store into custom 1 (row 5) */
    CHECK_EQ_INT(mock_eq_store_row, 5);
    CHECK_EQ_INT(ui_stack_depth(), 1);
}
