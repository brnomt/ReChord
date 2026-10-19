/*
 * library_cat_screen.c — one library category, two modes (BRNOMT-78).
 *
 *   names  : the unique values of the category ("Undefined" first)
 *   songs  : the records matching the chosen value (or the whole library)
 *
 * Deliberately thin: all the list work lives in services/media_query.c, which
 * is host-tested and puts its arrays in the shared scratch region instead of
 * statics (the AP has almost no .bss margin).
 */
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "../theme.h"
#include "../internal/hal.h"

#include "../../services/media_db.h"
#include "../../services/media_query.h"

/* Rows that fit between the list top and the footer row; page == window. */
static int cat_rows(void)
{
    int rows = (DISPLAY_H - THEME_ITEM_H - THEME_MENU_TOP) / THEME_ITEM_H;

    return rows < 1 ? 1 : rows;
}

typedef struct {
    int  cat;
    int  mode;                 /* 0 = names, 1 = songs */
    int  cursor;
    int  top;
    char value[24];            /* the chosen name (songs mode) */
} cat_state_t;

static cat_state_t s_cat;

/* Called by the Library menu before pushing this screen. */
void library_cat_open(int cat)
{
    s_cat.cat = cat;
    s_cat.mode = (cat == MQ_ALL) ? 1 : 0;
    s_cat.cursor = 0;
    s_cat.top = 0;
    s_cat.value[0] = '\0';
    if (s_cat.mode)
        (void)mq_filter_init(s_cat.cat, 0);
    else
        (void)mq_names_init(s_cat.cat);
}

static unsigned cat_total(void)
{
    return s_cat.mode ? mq_filter_count() : mq_names_count();
}

static void cat_on_enter(void)
{
    if (s_cat.mode)
        (void)mq_filter_init(s_cat.cat, s_cat.value[0] ? s_cat.value : 0);
    else
        (void)mq_names_init(s_cat.cat);
    ui_invalidate_all();
}

static void cat_on_exit(void)
{
    mq_done();
}

static void cat_draw(void)
{
    unsigned total = cat_total();
    char title[48];
    char footer[24];
    int row;

    display_clear(THEME_COLOR_BG);
    display_fill_rect(0, 0, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_TITLE_BG);
    if (s_cat.mode)
        snprintf(title, sizeof title, "%s / %s",
                 mq_category_name(s_cat.cat), s_cat.value);
    else
        snprintf(title, sizeof title, "%s", mq_category_name(s_cat.cat));
    display_draw_text(THEME_PAD, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF, title,
                      THEME_COLOR_ACCENT, THEME_COLOR_TITLE_BG);

    if (total == 0u) {
        menu_draw_item(THEME_MENU_TOP,
                       s_cat.mode ? "no matches" : "empty index", NULL, false);
        return;
    }
    for (row = 0; row < cat_rows(); row++) {
        int abs_idx = s_cat.top + row;
        media_rec_t rec;
        const char *label;

        if (abs_idx >= (int)total)
            break;
        label = s_cat.mode ? (mq_filter_get((unsigned)abs_idx, &rec) == FS_OK
                                  ? rec.title : "?")
                           : mq_names_get((unsigned)abs_idx);
        if (label == 0)
            break;
        menu_draw_item(THEME_MENU_TOP + row * THEME_ITEM_H, label, NULL,
                       abs_idx == s_cat.cursor);
    }
    snprintf(footer, sizeof footer, "%u / %u",
             (unsigned)(s_cat.cursor + 1), total);
    menu_draw_item(DISPLAY_H - THEME_ITEM_H, footer, NULL, false);
}

static void cat_play(int idx)
{
    unsigned n = 0;
    const uint16_t *list = mq_filter_indices(&n);

    if (list == 0 || n == 0)
        return;
    /* BRNOMT-90: play the whole filtered list, starting here — the player
     * shows "i/N" and auto-advances through the rest. */
    if (player_play_list(list, (int)n, idx) == PLAYER_OK) {
        track_info_t ti;

        player_screen_set_status(NULL);
        if (player_get_track(&ti) == PLAYER_OK)
            player_screen_set_track(ti.title,
                                    ti.duration_ms > 0
                                        ? (uint32_t)(ti.duration_ms / 1000) : 0);
        ui_push_screen(&player_screen);
    } else {
        player_screen_set_status("cannot open track");
    }
}

/* Keep the cursor inside the visible window, clamped to the list (BRNOMT-93:
 * the wrap itself happens in the key handler; this just fixes the viewport). */
static void cat_ensure_visible(unsigned total)
{
    int rows = cat_rows();

    if (total == 0u) {
        s_cat.cursor = 0;
        s_cat.top = 0;
        return;
    }
    if (s_cat.cursor < 0)
        s_cat.cursor = 0;
    if (s_cat.cursor >= (int)total)
        s_cat.cursor = (int)total - 1;
    if (s_cat.cursor < s_cat.top)
        s_cat.top = s_cat.cursor;
    if (s_cat.cursor >= s_cat.top + rows)
        s_cat.top = s_cat.cursor - rows + 1;
    if (s_cat.top < 0)
        s_cat.top = 0;
    if (s_cat.top > (int)total - rows)
        s_cat.top = ((int)total > rows) ? (int)total - rows : 0;
}

static void cat_handle_event(const input_event_t *ev)
{
    unsigned total = cat_total();

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;                       /* listings act on PRESS/REPEAT only */

    switch (ev->type) {
    case KEY_UP:
        if (total != 0u) {
            /* cyclic wrap-around (BRNOMT-93) */
            s_cat.cursor = (s_cat.cursor <= 0) ? (int)total - 1
                                               : s_cat.cursor - 1;
            cat_ensure_visible(total);
            ui_invalidate_all();
        }
        break;
    case KEY_DOWN:
        if (total != 0u) {
            s_cat.cursor = (s_cat.cursor >= (int)total - 1) ? 0
                                                            : s_cat.cursor + 1;
            cat_ensure_visible(total);
            ui_invalidate_all();
        }
        break;
    case KEY_SELECT:
        if (total == 0u)
            break;
        if (s_cat.mode) {
            cat_play(s_cat.cursor);
        } else {
            const char *v = mq_names_get((unsigned)s_cat.cursor);

            if (v == 0)
                break;
            snprintf(s_cat.value, sizeof s_cat.value, "%s", v);
            s_cat.mode = 1;
            s_cat.cursor = 0;
            s_cat.top = 0;
            (void)mq_filter_init(s_cat.cat, s_cat.value);
            ui_invalidate_all();
        }
        break;
    case KEY_BACK:
        if (s_cat.mode && s_cat.cat != MQ_ALL) {
            /* back to the names of this category */
            s_cat.mode = 0;
            s_cat.value[0] = '\0';
            s_cat.cursor = 0;
            s_cat.top = 0;
            (void)mq_names_init(s_cat.cat);
            ui_invalidate_all();
        } else {
            ui_pop_screen();
        }
        break;
    default:
        break;
    }
}

const ui_screen_t library_cat_screen = {
    "library-cat",
    cat_on_enter,
    cat_on_exit,
    cat_draw,
    cat_handle_event,
};
