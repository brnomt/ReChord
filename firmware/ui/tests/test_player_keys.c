/*
 * test_player_keys.c — BRNOMT-107 Now Playing hold behaviour.
 *
 * Seek mode: LONG seeks ±5 s; REPEAT is rate-limited to one step per 200 ms;
 * RELEASE{long_press=1} does not change the volume.
 * Skip mode: VOL+ next; VOL- keeps the >3 s rewind / previous-track rule.
 */
#include "test_util.h"
#include "ui.h"
#include "mock_display.h"
#include "mock_services.h"
#include "screens/screens.h"

static input_event_t ev(input_key_t key, input_kind_t kind, int long_press,
                        unsigned now_ms)
{
    input_event_t e;

    e.type = key;
    e.adc_value = -1;
    e.kind = kind;
    e.long_press = long_press;
    e.now_ms = now_ms;
    return e;
}

void test_player_keys(void)
{
    input_event_t e;

    ui_init();
    mock_display_reset();
    mock_services_reset();
    settings_screen_reload();
    mock_player_set_track_present(1);
    mock_player_set_state(PLAYER_PLAYING);
    mock_player_set_position(60000);

    settings_screen_set_hold_vol(HOLD_VOL_SEEK);
    CHECK(ui_push_screen(&player_screen));

    /* LONG: immediate +5 s; repeats under 200 ms are ignored. */
    e = ev(KEY_UP, INPUT_LONG, 1, 1000);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_seek_count(), 1);
    CHECK_EQ_INT(mock_player_last_seek_ms(), 65000);

    e = ev(KEY_UP, INPUT_REPEAT, 1, 1100);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_seek_count(), 1);

    e = ev(KEY_UP, INPUT_REPEAT, 1, 1210);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_seek_count(), 2);
    CHECK_EQ_INT(mock_player_last_seek_ms(), 70000);

    /* A long release must not also bump the volume. */
    e = ev(KEY_UP, INPUT_RELEASE, 1, 1220);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_volume_db(), -23);

    /* A short release still does. */
    e = ev(KEY_UP, INPUT_RELEASE, 0, 1300);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_volume_db(), -22);

    /* Skip mode: VOL+ next, VOL- >3 s rewinds, inside 3 s goes previous. */
    settings_screen_set_hold_vol(HOLD_VOL_SKIP);
    mock_player_set_position(60000);
    e = ev(KEY_UP, INPUT_LONG, 1, 2000);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_next_count(), 1);

    e = ev(KEY_DOWN, INPUT_LONG, 1, 2100);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_seek_count(), 3);
    CHECK_EQ_INT(mock_player_last_seek_ms(), 0);

    mock_player_set_position(1000);
    e = ev(KEY_DOWN, INPUT_LONG, 1, 2200);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_prev_count(), 1);

    e = ev(KEY_DOWN, INPUT_RELEASE, 1, 2300);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_volume_db(), -22);
}
