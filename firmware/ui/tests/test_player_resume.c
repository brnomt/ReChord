/*
 * test_player_resume.c — Now Playing must recover from the transient
 * "nothing playing" state once the resume restore has loaded a track.
 *
 * The app loop calls player_screen_refresh() every pass; before the resume
 * restore runs there is no track, so the screen shows "nothing playing".
 * When the restore opens the saved track (STOPPED, seeked), the status must
 * clear and SELECT must resume playback.
 */
#include "test_util.h"
#include "ui.h"
#include "mock_display.h"
#include "mock_services.h"
#include "screens/screens.h"

static input_event_t ev(input_key_t key)
{
    input_event_t e;

    e.type = key;
    e.adc_value = -1;
    e.kind = INPUT_PRESS;
    e.long_press = 0;
    return e;
}

void test_player_resume(void)
{
    input_event_t e;

    ui_init();
    mock_display_reset();
    mock_services_reset();
    player_screen_set_status(NULL);

    /* Boot pass before the deferred resume restore: no track yet. */
    mock_player_set_track_present(0);
    player_screen_refresh();
    CHECK(ui_push_screen(&player_screen));
    mock_display_reset();
    ui_render();
    CHECK(mock_display_text_find("nothing playing") >= 0);

    /* Resume restore loaded the saved track and seeked it, still STOPPED. */
    mock_player_set_track_present(1);
    mock_player_set_state(PLAYER_STOPPED);
    mock_player_set_position(42000);
    player_screen_refresh();
    mock_display_reset();
    ui_render();
    CHECK(mock_display_text_find("nothing playing") < 0);
    CHECK(mock_display_text_find("Paused") >= 0);
    CHECK(mock_display_text_find("0:42") >= 0);

    /* SELECT is no longer blocked by the stale error view: it resumes. */
    e = ev(KEY_SELECT);
    ui_handle_event(&e);
    CHECK_EQ_INT(mock_player_play_count(), 1);

    e = ev(KEY_BACK);
    ui_handle_event(&e);
    CHECK_EQ_INT(ui_stack_depth(), 0);

    /* The state line follows player transitions even when the 10 Hz poll is
     * not driving it (BRNOMT-105 follow-up: "Playing" after a pause). */
    CHECK(ui_push_screen(&player_screen));
    mock_player_set_state(PLAYER_PLAYING);
    player_screen_sync_state();
    mock_display_reset();
    ui_render();
    CHECK(mock_display_text_find("Playing") >= 0);

    mock_player_set_state(PLAYER_PAUSED);
    player_screen_sync_state();
    mock_display_reset();
    ui_render();
    CHECK(mock_display_text_find("Paused") >= 0);
    CHECK(mock_display_text_find("Playing") < 0);
}
