/*
 * library_screen.c — All Tracks, read from the media index (BRNOMT-78).
 *
 * The browser walks the FAT; this walks the index: one page of records is
 * fetched when the cursor moves (never in draw, same rule as the browser),
 * and SELECT opens the track. If there is no index it says so and points at
 * "Rebuild index" — it never falls back to scanning, which is the whole point
 * of having the index.
 */
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "../theme.h"
#include "../internal/hal.h"

#include "../../services/media_db.h"

/* One page = one window: 10 rows of 12 px from THEME_MENU_TOP end at y=136,
 * clear of the footer row at DISPLAY_H - THEME_ITEM_H (158), and 10 records x
 * 184 B keeps the .bss margin under the 0x0304A000 gate. (menu_page_size()
 * counts the footer row too, so it must not be used for the window.) */
#define LIB_ROWS 10

typedef struct {
    int          top;          /* first visible record */
    int          cursor;       /* absolute cursor */
    uint32_t     total;
    int          win_top;      /* first record in the window */
    int          win_n;
    media_rec_t  win[LIB_ROWS];
} lib_state_t;

static lib_state_t s_lib;

static void lib_fetch(void)
{
    int i;

    s_lib.win_top = s_lib.top;
    s_lib.win_n = 0;
    for (i = 0; i < LIB_ROWS; i++) {
        uint32_t idx = (uint32_t)(s_lib.top + i);

        if (idx >= s_lib.total || media_db_get(idx, &s_lib.win[i]) != FS_OK)
            break;
        s_lib.win_n++;
    }
}

static void lib_on_enter(void)
{
    int r = media_db_open();

    s_lib.total = (r == FS_OK) ? media_db_count() : 0u;
    s_lib.top = 0;
    s_lib.cursor = 0;
    lib_fetch();
    ui_invalidate_all();
}

static void lib_on_exit(void)
{
    media_db_close();
}

static void lib_draw(void)
{
    int rows = LIB_ROWS;
    int row;
    char footer[32];

    display_clear(THEME_COLOR_BG);
    display_fill_rect(0, 0, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_TITLE_BG);
    display_draw_text(THEME_PAD, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                      "Library", THEME_COLOR_ACCENT, THEME_COLOR_TITLE_BG);

    if (s_lib.total == 0u) {
        menu_draw_item(THEME_MENU_TOP, "no index: Rebuild index", NULL, false);
        return;
    }
    for (row = 0; row < rows; row++) {
        int abs_idx = s_lib.top + row;
        int widx = abs_idx - s_lib.win_top;
        const media_rec_t *r;

        if (abs_idx >= (int)s_lib.total || widx < 0 || widx >= s_lib.win_n)
            break;
        r = &s_lib.win[widx];
        /* Title only: at 320 px the artist column was truncated mid-word.
         * The full tags are in Now Playing. */
        menu_draw_item(THEME_MENU_TOP + row * THEME_ITEM_H,
                       r->title, NULL, abs_idx == s_lib.cursor);
    }
    snprintf(footer, sizeof footer, "%u / %u",
             (unsigned)(s_lib.cursor + 1), (unsigned)s_lib.total);
    menu_draw_item(DISPLAY_H - THEME_ITEM_H, footer, NULL, false);
}

static void lib_play(const media_rec_t *r)
{
    char full[MEDIA_PATH_MAX + 2];
    int n = (int)strlen(r->path);

    if (n + 1 >= (int)sizeof full)
        return;
    full[0] = '/';
    memcpy(full + 1, r->path, (size_t)n + 1u);
    if (player_open(full) == PLAYER_OK) {
        track_info_t ti;

        player_screen_set_status(NULL);
        if (player_get_track(&ti) == PLAYER_OK)
            player_screen_set_track(ti.title,
                                    ti.duration_ms > 0
                                        ? (uint32_t)(ti.duration_ms / 1000) : 0);
        (void)player_play();
        ui_push_screen(&player_screen);   /* asking for a track means seeing it */
    } else {
        player_screen_set_status("cannot open track");
    }
}

static void lib_handle_event(const input_event_t *ev)
{
    int rows = LIB_ROWS;

    switch (ev->type) {
    case KEY_UP:
        if (s_lib.cursor > 0) {
            s_lib.cursor--;
            if (s_lib.cursor < s_lib.top)
                s_lib.top = s_lib.cursor;
            lib_fetch();
            ui_invalidate_all();
        }
        break;
    case KEY_DOWN:
        if (s_lib.total != 0u && s_lib.cursor + 1 < (int)s_lib.total) {
            s_lib.cursor++;
            if (s_lib.cursor >= s_lib.top + rows)
                s_lib.top = s_lib.cursor - rows + 1;
            lib_fetch();
            ui_invalidate_all();
        }
        break;
    case KEY_SELECT: {
        int widx = s_lib.cursor - s_lib.win_top;

        if (s_lib.total != 0u && widx >= 0 && widx < s_lib.win_n)
            lib_play(&s_lib.win[widx]);
        break;
    }
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
    lib_on_exit,
    lib_draw,
    lib_handle_event,
};
