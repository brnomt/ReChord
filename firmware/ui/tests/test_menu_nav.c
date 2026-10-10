/*
 * test_menu_nav.c — menu navigation state machine + menu_screen behavior.
 */
#include "test_util.h"
#include "ui.h"
#include "screens/screens.h"
#include "mock_display.h"
#include "mock_services.h"

static int s_selected[3];

static void select_0(void) { s_selected[0]++; }
static void select_1(void) { s_selected[1]++; }

static const char *value_1(void) { return "v1"; }

static const menu_item_t s_items[3] = {
    { "One",   NULL,    select_0 },
    { "Two",   value_1, select_1 },
    { "Three", NULL,    NULL     },
};

static const menu_model_t s_model = { "Test menu", s_items, 3 };

static input_event_t ev(input_key_t key)
{
    input_event_t e;
    e.type = key;
    e.adc_value = 0;
    return e;
}

static void test_menu_view_move(void)
{
    menu_view_t v;

    menu_view_init(&v);
    CHECK_EQ_INT(v.cursor, 0);
    CHECK_EQ_INT(v.top, 0);

    /* clamped at the start */
    menu_view_move(&v, -1, 12, 5);
    CHECK_EQ_INT(v.cursor, 0);
    CHECK_EQ_INT(v.top, 0);

    /* single step */
    menu_view_move(&v, +1, 12, 5);
    CHECK_EQ_INT(v.cursor, 1);
    CHECK_EQ_INT(v.top, 0);

    /* clamp at the end + scroll to keep cursor on the last page */
    menu_view_move(&v, +100, 12, 5);
    CHECK_EQ_INT(v.cursor, 11);
    CHECK_EQ_INT(v.top, 7);

    /* back to the start */
    menu_view_move(&v, -100, 12, 5);
    CHECK_EQ_INT(v.cursor, 0);
    CHECK_EQ_INT(v.top, 0);

    /* minimal scroll: page 5, moving to item 5 pushes top to 1 */
    menu_view_init(&v);
    menu_view_move(&v, +4, 12, 5);
    CHECK_EQ_INT(v.cursor, 4);
    CHECK_EQ_INT(v.top, 0);
    menu_view_move(&v, +1, 12, 5);
    CHECK_EQ_INT(v.cursor, 5);
    CHECK_EQ_INT(v.top, 1);
    menu_view_move(&v, +1, 12, 5);
    CHECK_EQ_INT(v.cursor, 6);
    CHECK_EQ_INT(v.top, 2);

    /* moving up inside the window does not scroll */
    menu_view_move(&v, -1, 12, 5);
    CHECK_EQ_INT(v.cursor, 5);
    CHECK_EQ_INT(v.top, 2);

    /* moving above the window scrolls up minimally */
    menu_view_move(&v, -3, 12, 5);
    CHECK_EQ_INT(v.cursor, 2);
    CHECK_EQ_INT(v.top, 2);
    menu_view_move(&v, -1, 12, 5);
    CHECK_EQ_INT(v.cursor, 1);
    CHECK_EQ_INT(v.top, 1);

    /* empty list */
    menu_view_init(&v);
    menu_view_move(&v, +1, 0, 5);
    CHECK_EQ_INT(v.cursor, 0);
    CHECK_EQ_INT(v.top, 0);
}

static void test_menu_screen_keys(void)
{
    input_event_t e;

    s_selected[0] = s_selected[1] = 0;

    ui_init();
    mock_display_reset();
    mock_services_reset();

    menu_screen_bind(&s_model);
    CHECK(menu_screen_model() == &s_model);
    CHECK(ui_push_screen(&menu_screen));
    CHECK_EQ_INT(menu_screen_view()->cursor, 0);
    CHECK_EQ_INT(menu_screen_view()->top, 0);

    /* DOWN moves, SELECT fires the item's action */
    e = ev(KEY_DOWN);
    ui_handle_event(&e);
    CHECK_EQ_INT(menu_screen_view()->cursor, 1);

    e = ev(KEY_SELECT);
    ui_handle_event(&e);
    CHECK_EQ_INT(s_selected[1], 1);
    CHECK_EQ_INT(s_selected[0], 0);

    /* UP clamps at the top */
    e = ev(KEY_UP);
    ui_handle_event(&e);
    e = ev(KEY_UP);
    ui_handle_event(&e);
    CHECK_EQ_INT(menu_screen_view()->cursor, 0);
    CHECK_EQ_INT(s_selected[0], 0);   /* no stray select */

    /* BACK pops */
    e = ev(KEY_BACK);
    ui_handle_event(&e);
    CHECK_EQ_INT(ui_stack_depth(), 0);

    /* rendering: title, items and the selected row's value show up */
    menu_screen_bind(&s_model);
    CHECK(ui_push_screen(&menu_screen));
    e = ev(KEY_DOWN);
    ui_handle_event(&e);
    mock_display_reset();
    ui_render();
    CHECK(mock_display_text_find("Test menu") >= 0);
    CHECK(mock_display_text_find("One") >= 0);
    CHECK(mock_display_text_find("Two") >= 0);
    CHECK(mock_display_text_find("v1") >= 0);
}

void test_menu_nav(void)
{
    test_menu_view_move();
    test_menu_screen_keys();
}
