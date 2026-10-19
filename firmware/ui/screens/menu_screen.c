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
 *   KEY_UP/KEY_DOWN move the cursor by one with cyclic wrap-around (UP at the
 *   top goes to the bottom, DOWN at the bottom goes to the top, BRNOMT-93),
 *   and the view scrolls minimally so the cursor stays inside the visible
 *   page; KEY_SELECT runs the item's on_select; KEY_BACK pops the screen
 *   stack. Only PRESS/REPEAT act — LONG/RELEASE are for other screens.
 */
#include <string.h>

#include "screens.h"
#include "companion.h"
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
    if (delta == 1 && v->cursor == count - 1)
        target = 0;                   /* cyclic wrap-around (BRNOMT-93) */
    else if (delta == -1 && v->cursor == 0)
        target = count - 1;
    else {
        if (target < 0)
            target = 0;
        if (target > count - 1)
            target = count - 1;
    }
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

/* First row y for a model: only a list that paints the face reserves its
 * band; every other list starts right under the title bar (BRNOMT-72). */
static int menu_top(const menu_model_t *model)
{
    return THEME_MENU_TOP +
           ((model != 0 && model->face != 0) ? theme_layout()->face_h : 0);
}

int menu_page_size_for(const menu_model_t *model)
{
    int top = menu_top(model);

    if (top < 0)
        top = 0;
    return (DISPLAY_H - top) / THEME_ITEM_H;
}

int menu_page_size(void)
{
    return menu_page_size_for(0);
}

/* ---- rendering --------------------------------------------------------- */

void menu_draw_item(int y, const char *label, const char *value, bool selected)
{
    int vy = y + THEME_TEXT_YOFF;
    uint16_t row_bg = THEME_COLOR_BG;
    uint16_t label_fg = THEME_COLOR_DIM;
    uint16_t value_fg = THEME_COLOR_FG;

    if (selected) {
        /* True INVERSION (solid bar, glyphs punched out of it), the one focus
         * treatment every list shares (BRNOMT-72). A tinted highlight cannot
         * express this with the monochrome palette. */
        display_fill_rect(0, y, DISPLAY_W - 1, y + THEME_ITEM_H - 1,
                          THEME_COLOR_FG);
        row_bg = THEME_COLOR_FG;
        label_fg = THEME_COLOR_BG;
        value_fg = THEME_COLOR_BG;
    }

    display_draw_text(THEME_PAD, vy,
                      label ? label : "",
                      label_fg, row_bg);
    if (value && value[0] != '\0') {
        int x = DISPLAY_W - THEME_PAD - THEME_TEXT_W((int)strlen(value));
        if (x < THEME_PAD)
            x = THEME_PAD;
        display_draw_text(x, vy, value, value_fg, row_bg);
    }
}

void menu_draw(const menu_model_t *model, const menu_view_t *view)
{
    int page = menu_page_size_for(model);
    int top = menu_top(model);
    int row;

    display_clear(THEME_COLOR_BG);

    /* title bar */
    display_fill_rect(0, 0, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_TITLE_BG);
    display_draw_text(THEME_PAD, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                      model->title ? model->title : "",
                      THEME_COLOR_ACCENT, THEME_COLOR_TITLE_BG);
    /* One-pixel rule under the title: grouping without a colored band. */
    display_fill_rect(0, THEME_TITLE_H - 1, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_FG);

    /* optional right-aligned status on the title row (BRNOMT-72) */
    if (model->status_text != 0) {
        const char *st = model->status_text();

        if (st != 0 && st[0] != '\0') {
            int tx = DISPLAY_W - THEME_PAD - THEME_TEXT_W((int)strlen(st));

            if (tx > THEME_PAD)
                display_draw_text(tx, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                                  st, THEME_COLOR_DIM, THEME_COLOR_TITLE_BG);
        }
    }

    /* Procedural companion face in the reserved band (companion-bot theme):
     * only the root model asks for it; the band is always part of the layout. */
    if (model->face != 0 && theme_layout()->face_h > 0) {
        companion_draw(0, THEME_TITLE_H, DISPLAY_W, theme_layout()->face_h,
                       player_get_state() == PLAYER_PLAYING, view->cursor);
    }

    /* visible rows */
    for (row = 0; row < page; row++) {
        int idx = view->top + row;
        int y;
        const menu_item_t *it;

        if (idx >= model->item_count)
            break;
        y = top + row * THEME_ITEM_H;
        it = &model->items[idx];
        menu_draw_item(y, it->label,
                       it->value_text ? it->value_text() : NULL,
                       idx == view->cursor);
    }
}

/* ---- screen vtable ----------------------------------------------------- */

static void menu_screen_on_enter(void)
{
    /* Keep cursor/scroll when a child screen pops back (BRNOMT-93);
     * menu_screen_bind() resets it for a new model. */
}

static void menu_screen_draw(void)
{
    if (s_model)
        menu_draw(s_model, &s_view);
}

static void menu_screen_handle_event(const input_event_t *ev)
{
    if (s_model == NULL || ev->kind == INPUT_LONG ||
        ev->kind == INPUT_RELEASE)
        return;                       /* listings act on PRESS/REPEAT only */

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_view, -1, s_model->item_count,
                       menu_page_size_for(s_model));
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_view, +1, s_model->item_count,
                       menu_page_size_for(s_model));
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
