/*
 * menu_screen.c — generic list menu: title + scrollable items + selection.
 *
 * Reusable in two ways:
 *   1. as a full screen (`menu_screen`) bound to a menu_model_t;
 *   2. as rendering/navigation helpers (menu_draw / menu_view_*) reused by
 *      settings_screen and browser_screen so every list looks and scrolls
 *      the same.
 *
 * Navigation contract (host-tested in tests/test_menu_nav.c):
 *   KEY_UP/KEY_DOWN move the cursor by one, clamped at the list ends (no
 *   wrap — predictable on a device with a 5-key surface), and the view
 *   scrolls minimally so the cursor stays inside the visible page;
 *   KEY_SELECT runs the item's on_select; KEY_BACK pops the screen stack.
 */
#include <string.h>

#include "screens.h"
#include "../theme.h"
#include "../internal/hal.h"

static const menu_model_t *s_model;
static menu_view_t         s_view;

/* ---- pure navigation --------------------------------------------------- */

void menu_view_init(menu_view_t *v)
{
    v->cursor = 0;
    v->top = 0;
}

void menu_view_move(menu_view_t *v, int delta, int count, int page)
{
    int target;

    if (count <= 0) {
        v->cursor = 0;
        v->top = 0;
        return;
    }
    if (page < 1)
        page = 1;

    target = v->cursor + delta;
    if (target < 0)
        target = 0;
    if (target > count - 1)
        target = count - 1;
    v->cursor = target;

    /* minimal scroll to keep the cursor visible */
    if (v->cursor < v->top)
        v->top = v->cursor;
    if (v->cursor > v->top + page - 1)
        v->top = v->cursor - page + 1;
    if (v->top > count - 1)
        v->top = count - 1;
    if (v->top < 0)
        v->top = 0;
}

int menu_page_size(void)
{
    return (DISPLAY_H - THEME_MENU_TOP) / THEME_ITEM_H;
}

/* ---- rendering --------------------------------------------------------- */

void menu_draw_item(int y, const char *label, const char *value, bool selected)
{
    int vy = y + THEME_TEXT_YOFF;

    display_fill_rect(0, y, DISPLAY_W - 1, y + THEME_ITEM_H - 1,
                      selected ? THEME_COLOR_SELECT_BG : THEME_COLOR_BG);
    display_draw_text(THEME_PAD, vy,
                      label ? label : "",
                      selected ? THEME_COLOR_FG : THEME_COLOR_DIM,
                      selected ? THEME_COLOR_SELECT_BG : THEME_COLOR_BG);
    if (value && value[0] != '\0') {
        int x = DISPLAY_W - THEME_PAD - THEME_TEXT_W((int)strlen(value));
        if (x < THEME_PAD)
            x = THEME_PAD;
        display_draw_text(x, vy, value,
                          selected ? THEME_COLOR_ACCENT : THEME_COLOR_FG,
                          selected ? THEME_COLOR_SELECT_BG : THEME_COLOR_BG);
    }
}

void menu_draw(const menu_model_t *model, const menu_view_t *view)
{
    int page = menu_page_size();
    int row;

    display_clear(THEME_COLOR_BG);

    /* title bar */
    display_fill_rect(0, 0, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_TITLE_BG);
    display_draw_text(THEME_PAD, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                      model->title ? model->title : "",
                      THEME_COLOR_ACCENT, THEME_COLOR_TITLE_BG);

    /* visible rows */
    for (row = 0; row < page; row++) {
        int idx = view->top + row;
        int y;
        const menu_item_t *it;

        if (idx >= model->item_count)
            break;
        y = THEME_MENU_TOP + row * THEME_ITEM_H;
        it = &model->items[idx];
        menu_draw_item(y, it->label,
                       it->value_text ? it->value_text() : NULL,
                       idx == view->cursor);
    }
}

/* ---- screen vtable ----------------------------------------------------- */

static void menu_screen_on_enter(void)
{
    menu_view_init(&s_view);
}

static void menu_screen_draw(void)
{
    if (s_model)
        menu_draw(s_model, &s_view);
}

static void menu_screen_handle_event(const input_event_t *ev)
{
    if (s_model == NULL)
        return;

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_view, -1, s_model->item_count, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_view, +1, s_model->item_count, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (s_view.cursor >= 0 && s_view.cursor < s_model->item_count &&
            s_model->items[s_view.cursor].on_select)
            s_model->items[s_view.cursor].on_select();
        ui_invalidate_all();
        break;
    case KEY_BACK:
        ui_pop_screen();
        break;
    default:
        break;   /* KEY_POWER etc.: core/global policy, ignore here */
    }
}

const ui_screen_t menu_screen = {
    "menu",
    menu_screen_on_enter,
    NULL,
    menu_screen_draw,
    menu_screen_handle_event,
};

void menu_screen_bind(const menu_model_t *model)
{
    s_model = model;
    menu_view_init(&s_view);
}

const menu_model_t *menu_screen_model(void)
{
    return s_model;
}

const menu_view_t *menu_screen_view(void)
{
    return &s_view;
}
