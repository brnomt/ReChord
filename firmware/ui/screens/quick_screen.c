/*
 * quick_screen.c — MENU-button quick settings + the current-queue view
 * (BRNOMT-105).
 *
 * The MENU button opens this instead of jumping to the root menu:
 *   Shuffle  off/on (applied and persisted immediately)
 *   Repeat   off/all/one
 *   Playlist opens quick_playlist_screen: the live queue with the current
 *            track marked; SELECT jumps to the chosen entry and plays it.
 *
 * These are overlay screens pushed on the normal screen stack, so BACK (or
 * MENU again) returns to whatever was underneath.
 */
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "../theme.h"
#include "../../player/player.h"
#include "../internal/hal.h"

/* ---- quick settings ---------------------------------------------------- */

static const char *shuffle_value(void)
{
    return player_get_shuffle() ? "on" : "off";
}

static const char *repeat_value(void)
{
    int r = player_get_repeat();

    return (r == 1) ? "all" : (r == 2) ? "one" : "off";
}

static const char *playlist_value(void)
{
    static char buf[32];
    int n = player_list_count();
    int p = player_list_position();

    if (n <= 0)
        return "empty";
    snprintf(buf, sizeof buf, "%d/%d", p >= 0 ? p + 1 : 0, n);
    return buf;
}

static void on_shuffle(void)
{
    settings_screen_set_shuffle(!player_get_shuffle());
}

static void on_repeat(void)
{
    settings_screen_set_repeat((player_get_repeat() + 1) % 3);
}

static void on_playlist(void)
{
    ui_push_screen(&quick_playlist_screen);
}

static const menu_item_t s_quick_items[] = {
    { "Shuffle",  shuffle_value,  on_shuffle },
    { "Repeat",   repeat_value,   on_repeat },
    { "Playlist", playlist_value, on_playlist },
};

static const menu_model_t s_quick_model = {
    "Quick settings", s_quick_items, 3, NULL
};

static menu_view_t s_quick_view;

static void quick_on_enter(void)
{
    menu_view_init(&s_quick_view);
}

static void quick_draw(void)
{
    menu_draw(&s_quick_model, &s_quick_view);
}

static void quick_handle_event(const input_event_t *ev)
{
    (void)menu_handle_event(&s_quick_model, &s_quick_view, ev);
}

const ui_screen_t quick_screen = {
    "quick",
    quick_on_enter,
    NULL,
    quick_draw,
    quick_handle_event,
};

/* ---- current queue ----------------------------------------------------- */

static menu_view_t s_playlist_view;

static void playlist_on_enter(void)
{
    int n = player_list_count();
    int p = player_list_position();

    menu_view_init(&s_playlist_view);
    if (n > 0) {
        s_playlist_view.cursor = (p >= 0 && p < n) ? p : 0;
        menu_view_move(&s_playlist_view, 0, n, menu_page_size());
    }
}

static void playlist_draw(void)
{
    int n = player_list_count();
    int p = player_list_position();
    int page = menu_page_size();
    char footer[32];
    int tx;
    int row;

    display_clear(THEME_COLOR_BG);
    display_fill_rect(0, 0, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_TITLE_BG);
    display_draw_text(THEME_PAD, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                      "Playlist", THEME_COLOR_ACCENT, THEME_COLOR_TITLE_BG);
    display_fill_rect(0, THEME_TITLE_H - 1, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_FG);

    snprintf(footer, sizeof footer, "%d / %d", p >= 0 ? p + 1 : 0, n);
    tx = DISPLAY_W - THEME_PAD - THEME_TEXT_W((int)strlen(footer));
    if (tx > THEME_PAD)
        display_draw_text(tx, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                          footer, THEME_COLOR_DIM, THEME_COLOR_TITLE_BG);

    if (n <= 0) {
        display_draw_text(THEME_PAD, THEME_MENU_TOP + THEME_TEXT_YOFF,
                          "No queue", THEME_COLOR_DIM, THEME_COLOR_BG);
        return;
    }

    for (row = 0; row < page; row++) {
        int idx = s_playlist_view.top + row;
        char label[32];
        int y = THEME_MENU_TOP + row * THEME_ITEM_H;

        if (idx >= n)
            break;
        if (player_list_title(idx, label, (int)sizeof label) != 0 ||
            label[0] == '\0')
            snprintf(label, sizeof label, "Track %d", idx + 1);
        menu_draw_item(y, label, (idx == p) ? ">" : NULL,
                       idx == s_playlist_view.cursor);
    }
}

static void playlist_handle_event(const input_event_t *ev)
{
    int n = player_list_count();

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;                       /* listings act on PRESS/REPEAT only */

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_playlist_view, -1, n, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_playlist_view, +1, n, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (n > 0 && s_playlist_view.cursor >= 0 &&
            s_playlist_view.cursor < n &&
            player_play_at(s_playlist_view.cursor) == PLAYER_OK)
            ui_pop_screen();           /* back to Quick settings */
        break;
    case KEY_BACK:
        ui_pop_screen();
        break;
    default:
        break;
    }
}

const ui_screen_t quick_playlist_screen = {
    "quick-playlist",
    playlist_on_enter,
    NULL,
    playlist_draw,
    playlist_handle_event,
};
