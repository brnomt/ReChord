/*
 * test_bluetooth_screen.c — Settings > Bluetooth.
 *
 * The policy is mocked (mock_services.c); this covers the row layout and the
 * key mapping: toggle, pair, connect, forget on hold, back.
 */
#include "test_util.h"
#include "ui.h"
#include "screens/screens.h"
#include "mock_display.h"
#include "mock_services.h"
#include "../../services/bt_link.h"

static input_event_t ev(input_key_t key, input_kind_t kind)
{
    input_event_t e;

    e.type = key;
    e.adc_value = 0;
    e.kind = kind;
    e.long_press = 0;
    return e;
}

static void press(input_key_t key)
{
    input_event_t e = ev(key, INPUT_PRESS);

    ui_handle_event(&e);
}

void test_bluetooth_screen(void)
{
    mock_display_reset();
    mock_bt_reset();
    ui_init();

    CHECK(ui_push_screen(&bluetooth_screen));
    ui_render();
    CHECK(mock_display_text_find("Bluetooth") >= 0);
    CHECK(mock_display_text_find("Off") >= 0);
    CHECK(mock_display_text_find("Pair new device") >= 0);
    CHECK_EQ_INT(mock_bt_screen_opens(), 1);

    /* Row 0: the on/off switch. */
    press(KEY_SELECT);
    CHECK_EQ_INT(bt_link_enabled(), 1);
    ui_render();
    CHECK(mock_display_text_find("On") >= 0);

    /* With no devices, row 1 is Pair new device. */
    press(KEY_DOWN);
    press(KEY_SELECT);
    CHECK_EQ_INT(bt_link_state()->state, BT_STATE_SCANNING);
    ui_render();
    CHECK(mock_display_text_find("Searching...") >= 0);

    /* Two remembered devices: select connects, hold forgets. */
    mock_display_reset();
    mock_bt_reset();
    mock_bt_set(1, BT_STATE_READY, 2);
    mock_bt_set_device(0, "Buds");
    mock_bt_set_device(1, "Speaker");
    ui_init();
    CHECK(ui_push_screen(&bluetooth_screen));
    ui_render();
    CHECK(mock_display_text_find("Buds") >= 0);
    CHECK(mock_display_text_find("Speaker") >= 0);

    press(KEY_DOWN);                 /* device 0 */
    press(KEY_SELECT);
    CHECK_EQ_INT(mock_bt_last_device(), 0);

    {
        input_event_t e = ev(KEY_SELECT, INPUT_LONG);

        ui_handle_event(&e);
    }
    CHECK_EQ_INT(mock_bt_last_device(), 0);

    press(KEY_BACK);
    CHECK_EQ_INT(ui_stack_depth(), 0);
}
