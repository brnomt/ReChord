/*
 * test_library_extras_ui.c — the three extra Library entries.
 *
 * The catalog is mocked (empty index), so this checks the wiring: the rows
 * exist in the Library menu, selecting one opens the category screen with its
 * own query (no names step) and BACK returns to the Library.
 */
#include "test_util.h"
#include "ui.h"
#include "screens/screens.h"
#include "mock_display.h"
#include "mock_services.h"

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

void test_library_extras_ui(void)
{
    int i;

    mock_display_reset();
    mock_services_reset();
    ui_init();

    CHECK(ui_push_screen(&library_screen));
    ui_render();
    /* The three extra rows sit after the eight tag categories. */
    for (i = 0; i < 8; i++)
        press(KEY_DOWN);
    ui_render();
    CHECK(mock_display_text_find("Favorites") >= 0);
    CHECK(mock_display_text_find("Recently played") >= 0);
    CHECK(mock_display_text_find("Most played") >= 0);

    press(KEY_SELECT);                  /* Favorites */
    CHECK(ui_top_screen() == &library_cat_screen);
    ui_render();
    CHECK(mock_display_text_find("Favorites") >= 0);

    press(KEY_BACK);                    /* special views have no names step */
    CHECK(ui_top_screen() == &library_screen);

    /* Recently played opens the same way. */
    for (i = 0; i < 2; i++)
        press(KEY_DOWN);
    press(KEY_SELECT);
    CHECK(ui_top_screen() == &library_cat_screen);
    ui_render();
    CHECK(mock_display_text_find("Recently played") >= 0);
    press(KEY_BACK);
}
