/*
 * test_home_theme.c — the package-selected home renderer (BRNOMT-69).
 *
 * A companion package must render the face + item band in the axis its
 * manifest declares; a menu package must stay a plain list (no face band).
 */
#include "test_util.h"
#include "mock_display.h"
#include "mock_services.h"
#include "screens/screens.h"
#include "screens/companion.h"
#include "theme.h"

static input_event_t key(int type)
{
    input_event_t e;

    e.type = type;
    e.adc_value = -1;
    e.kind = INPUT_PRESS;
    e.long_press = 0;
    return e;
}

static void apply_layout(const char *text)
{
    theme_layout_t l;

    theme_layout_defaults(&l);
    CHECK_EQ_INT(theme_layout_parse(text, &l), 0);
    theme_layout_set(&l);
}

void test_home_theme(void)
{
    int idx;

    ui_init();
    mock_display_reset();
    mock_services_reset();

    /* ---- companion home, horizontal items --------------------------- */
    apply_layout("format=1\nhome=companion\nhome_nav=horizontal\n"
                 "face_h=52\nhome_bar_y=150\nhome_bar_h=14\n"
                 "pad=6\ntitle_h=14\nitem_h=14\n");
    CHECK(ui_push_screen(&companion_home_screen));
    mock_display_reset();
    ui_render();

    idx = mock_display_text_find("LIBRARY");
    CHECK(idx >= 0);
    if (idx >= 0)
        CHECK_EQ_INT(mock_display_text(idx)->y, 150 + THEME_TEXT_YOFF);
    idx = mock_display_text_find("NOW PLAYING");
    CHECK(idx >= 0);
    if (idx >= 0)
        CHECK_EQ_INT(mock_display_text(idx)->bg, THEME_COLOR_FG);  /* selected */
    idx = mock_display_text_find("SETTINGS");
    CHECK(idx >= 0);

    /* DOWN moves the selection to the last item (same band row). */
    {
        input_event_t e = key(KEY_DOWN);

        ui_handle_event(&e);
        mock_display_reset();
        ui_render();
        idx = mock_display_text_find("SETTINGS");
        CHECK(idx >= 0);
        if (idx >= 0) {
            CHECK_EQ_INT(mock_display_text(idx)->bg, THEME_COLOR_FG);
            CHECK_EQ_INT(mock_display_text(idx)->y, 150 + THEME_TEXT_YOFF);
        }
    }

    /* BACK returns to the classic menu underneath. */
    {
        input_event_t e = key(KEY_BACK);

        ui_handle_event(&e);
        CHECK_EQ_INT(ui_stack_depth(), 0);
    }

    /* ---- companion home, vertical items ----------------------------- */
    ui_init();
    apply_layout("format=1\nhome=companion\nhome_nav=vertical\n"
                 "face_h=52\nhome_bar_y=110\nhome_bar_h=14\n"
                 "pad=6\ntitle_h=14\nitem_h=14\n");
    CHECK(ui_push_screen(&companion_home_screen));
    mock_display_reset();
    ui_render();

    idx = mock_display_text_find("LIBRARY");
    CHECK(idx >= 0);
    if (idx >= 0)
        CHECK_EQ_INT(mock_display_text(idx)->y, 110 + THEME_TEXT_YOFF);
    idx = mock_display_text_find("NOW PLAYING");
    CHECK(idx >= 0);
    if (idx >= 0)
        CHECK_EQ_INT(mock_display_text(idx)->y, 124 + THEME_TEXT_YOFF);
    idx = mock_display_text_find("SETTINGS");
    CHECK(idx >= 0);
    if (idx >= 0)
        CHECK_EQ_INT(mock_display_text(idx)->y, 138 + THEME_TEXT_YOFF);

    theme_layout_reset();
}
