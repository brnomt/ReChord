/*
 * test_menu_nav.c — menu navigation state machine + menu_screen behavior.
 */
#include "test_util.h"
#include "ui.h"
#include "screens/screens.h"
#include "theme.h"
#include "mock_display.h"
#include "mock_services.h"

#include <stdio.h>
#include <string.h>

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
    e.kind = INPUT_PRESS;        /* screens act on PRESS/REPEAT (BRNOMT-93) */
    e.long_press = 0;
    return e;
}

static void test_menu_view_move(void)
{
    menu_view_t v;

    menu_view_init(&v);
    CHECK_EQ_INT(v.cursor, 0);
    CHECK_EQ_INT(v.top, 0);

    /* wrap-around (BRNOMT-93): UP at the top lands on the last item and
     * scrolls to show the last page */
    menu_view_move(&v, -1, 12, 5);
    CHECK_EQ_INT(v.cursor, 11);
    CHECK_EQ_INT(v.top, 7);

    /* DOWN at the bottom lands on the first item and scrolls to the top */
    menu_view_move(&v, +1, 12, 5);
    CHECK_EQ_INT(v.cursor, 0);
    CHECK_EQ_INT(v.top, 0);

    /* single step */
    menu_view_move(&v, +1, 12, 5);
    CHECK_EQ_INT(v.cursor, 1);
    CHECK_EQ_INT(v.top, 0);

    /* only ±1 wraps; a large delta still clamps */
    menu_view_move(&v, +100, 12, 5);
    CHECK_EQ_INT(v.cursor, 11);
    CHECK_EQ_INT(v.top, 7);
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

    /* UP from the top wraps to the last item (BRNOMT-93) */
    e = ev(KEY_UP);
    ui_handle_event(&e);
    CHECK_EQ_INT(menu_screen_view()->cursor, 0);
    e = ev(KEY_UP);
    ui_handle_event(&e);
    CHECK_EQ_INT(menu_screen_view()->cursor, 2);
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

    /* BRNOMT-72: the selected row ("Two", cursor 1) must be a TRUE INVERSION —
     * a solid foreground bar with the label drawn in the background color —
     * and a plain row must keep background cells with dim text. */
    {
        int i, bar = 0, sel_label = 0, plain_label = 0;

        for (i = 0; i < mock_display_rect_count(); i++) {
            const mock_rect_t *r = mock_display_rect(i);

            if (r->color == THEME_COLOR_FG && r->x0 == 0 &&
                r->x1 == DISPLAY_W - 1 &&
                r->y0 == THEME_MENU_TOP + THEME_ITEM_H)
                bar = 1;
        }
        for (i = 0; i < mock_display_text_count(); i++) {
            const mock_text_t *t = mock_display_text(i);

            if (strcmp(t->text, "Two") == 0 &&
                t->bg == THEME_COLOR_FG && t->fg == THEME_COLOR_BG)
                sel_label = 1;
            if (strcmp(t->text, "One") == 0 &&
                t->bg == THEME_COLOR_BG && t->fg == THEME_COLOR_DIM)
                plain_label = 1;
        }
        CHECK(bar);
        CHECK(sel_label);
        CHECK(plain_label);
    }
}

/* BRNOMT-72: the face band is reserved ONLY by the list that paints the face
 * (the main menu). Plain lists start right under the title bar, and the
 * footer-bearing lists stop above the i/N row. */
static void test_face_band_only_on_main_menu(void)
{
    static const menu_item_t items[1] = { { "X", NULL, NULL } };
    static const menu_model_t plain = { "Library", items, 1, NULL, 0 };
    static const menu_model_t faced = { "Main",    items, 1, NULL, 1 };
    menu_view_t v;
    FILE *f;
    int idx;

    f = fopen("rechord_test_layout.cfg", "w");
    CHECK(f != NULL);
    if (f == NULL)
        return;
    fputs("face_h=52\npad=6\ntitle_h=14\nitem_h=14\n", f);
    fclose(f);
    CHECK_EQ_INT(theme_layout_load("rechord_test_layout.cfg"), 0);
    remove("rechord_test_layout.cfg");

    CHECK_EQ_INT(menu_page_size_for(&plain),
                 (DISPLAY_H - THEME_MENU_TOP) / THEME_ITEM_H);
    CHECK_EQ_INT(menu_page_size_for(&faced),
                 (DISPLAY_H - THEME_FACE_TOP) / THEME_ITEM_H);
    CHECK_EQ_INT(browser_page_rows(),
                 (DISPLAY_H - THEME_ITEM_H - THEME_MENU_TOP) / THEME_ITEM_H);

    menu_view_init(&v);
    mock_display_reset();
    menu_draw(&plain, &v);
    idx = mock_display_text_find("X");
    CHECK(idx >= 0);
    if (idx >= 0)
        CHECK_EQ_INT(mock_display_text(idx)->y,
                     THEME_MENU_TOP + THEME_TEXT_YOFF);

    mock_display_reset();
    menu_draw(&faced, &v);
    idx = mock_display_text_find("X");
    CHECK(idx >= 0);
    if (idx >= 0)
        CHECK_EQ_INT(mock_display_text(idx)->y,
                     THEME_FACE_TOP + THEME_TEXT_YOFF);

    theme_layout_reset();
}

void test_menu_nav(void)
{
    test_menu_view_move();
    test_menu_screen_keys();
    test_face_band_only_on_main_menu();
}
