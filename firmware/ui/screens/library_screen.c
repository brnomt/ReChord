/*
 * library_screen.c — the Library menu (BRNOMT-78).
 *
 * The Rockbox-style entry point: pick a category, then a value, then a song.
 * All Songs opens the flat list (no name step); every other row opens the
 * category screen, which builds its name list on entry (services/media_query).
 */
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "theme_ui.h"
#include "../theme.h"
#include "../internal/hal.h"

#include "../../services/media_query.h"

static menu_view_t s_view;
static int         s_use_skin;

static void open_cat(int cat)
{
    library_cat_open(cat);
    ui_push_screen(&library_cat_screen);
}

static void open_all(void)          { open_cat(MQ_ALL); }
static void open_album(void)        { open_cat(MQ_ALBUM); }
static void open_artist(void)       { open_cat(MQ_ARTIST); }
static void open_genre(void)        { open_cat(MQ_GENRE); }
static void open_year(void)         { open_cat(MQ_YEAR); }
static void open_label(void)        { open_cat(MQ_LABEL); }
static void open_album_artist(void) { open_cat(MQ_ALBUM_ARTIST); }
static void open_composer(void)     { open_cat(MQ_COMPOSER); }

static const menu_item_t k_items[MQ_COUNT] = {
    { "All Songs",    NULL, open_all },
    { "Album",        NULL, open_album },
    { "Artist",       NULL, open_artist },
    { "Genre",        NULL, open_genre },
    { "Year",         NULL, open_year },
    { "Label",        NULL, open_label },
    { "Album Artist", NULL, open_album_artist },
    { "Composer",     NULL, open_composer },
};

static const menu_model_t k_model = { "Library", k_items, MQ_COUNT };

static const char *lib_label(int index, char *buf, int cap)
{
    if (index < 0 || index >= MQ_COUNT)
        return "";
    snprintf(buf, (size_t)cap, "%s", k_items[index].label);
    return buf;
}

static const theme_skin_list_source_t k_source = {
    MQ_COUNT, lib_label, 0
};

static void lib_on_enter(void)
{
    s_use_skin = theme_ui_load(THEME_UI_LIBRARY);
    if (s_use_skin)
        theme_ui_list(&k_source);
    /* Preserve the category cursor when returning from a category or the
     * player screen (BRNOMT-93); library_cat_open() resets on a new entry. */
    ui_invalidate_all();
}

static void lib_draw(void)
{
    if (s_use_skin) {
        theme_skin_state_t st;

        theme_ui_list(&k_source);
        theme_ui_fill_state(&st, "Library");
        theme_ui_draw(&st, s_view.cursor, s_view.top);
        return;
    }
    menu_draw(&k_model, &s_view);
}

static void lib_handle_event(const input_event_t *ev)
{
    int nav;
    int rows;

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;                       /* listings act on PRESS/REPEAT only */

    if (s_use_skin) {
        theme_ui_note_input();
        theme_ui_list(&k_source);
        rows = theme_ui_rows();
        if (rows <= 0)
            rows = menu_page_size();
        nav = theme_ui_nav(ev->type);
        if (nav == THEME_SKIN_PREV || nav == THEME_SKIN_NEXT) {
            menu_view_move(&s_view, nav == THEME_SKIN_PREV ? -1 : +1,
                           MQ_COUNT, rows);
            ui_invalidate_all();
        } else if (nav == THEME_SKIN_SELECT) {
            if (s_view.cursor >= 0 && s_view.cursor < MQ_COUNT)
                k_items[s_view.cursor].on_select();
        } else if (nav == THEME_SKIN_BACK) {
            ui_pop_screen();
        }
        return;
    }

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_view, -1, MQ_COUNT, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_view, +1, MQ_COUNT, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (s_view.cursor >= 0 && s_view.cursor < MQ_COUNT)
            k_items[s_view.cursor].on_select();
        break;
    case KEY_BACK:
        ui_pop_screen();
        break;
    default:
        break;
    }
}

const ui_screen_t library_screen = {
    "library",
    lib_on_enter,
    NULL,
    lib_draw,
    lib_handle_event,
};
