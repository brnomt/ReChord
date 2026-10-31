/*
 * bluetooth_screen.c — Bluetooth on/off, devices and pairing.
 *
 * Rows: the on/off switch, the remembered devices (most recent first, with
 * their link state), and "Pair new device". SELECT toggles / connects /
 * disconnects / starts a search; holding SELECT on a device forgets it.
 *
 * The policy (services/bt_link.c) owns every decision; this screen only maps
 * keys to actions and draws the live state.
 */
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "../theme.h"
#include "internal/hal.h"

#include "../../services/bt_link.h"

static menu_view_t s_view;

static int row_count(void)
{
    return bt_link_state()->count + 2;   /* switch + devices + pair */
}

static const char *row_value(int row)
{
    const bt_proto_t *bt = bt_link_state();
    int devices = bt->count;

    if (row == 0)
        return bt_link_enabled() ? "On" : "Off";
    if (row == devices + 1)
        return (bt->state == BT_STATE_SCANNING) ? "Searching..." : NULL;
    {
        int i = row - 1;

        if (bt->state == BT_STATE_CONNECTED && bt->connected == i)
            return bt->streaming ? "Playing" : "Connected";
        if (bt->state == BT_STATE_CONNECTING && bt->target == i)
            return "Connecting...";
        if (bt->found_mask & (1u << i))
            return "Nearby";
    }
    return NULL;
}

static const char *row_label(int row, char *scratch, unsigned cap)
{
    const bt_proto_t *bt = bt_link_state();
    int devices = bt->count;

    if (row == 0)
        return "Bluetooth";
    if (row == devices + 1)
        return "Pair new device";
    {
        const bt_device_t *d = bt_proto_device(bt, row - 1);

        if (d == 0 || d->name[0] == '\0') {
            snprintf(scratch, cap, "Device %d", row);
            return scratch;
        }
        return d->name;
    }
}

static void bt_on_enter(void)
{
    menu_view_init(&s_view);
    bt_link_screen_opened(log_now_ms());
}

static void bt_on_exit(void)
{
    bt_link_screen_closed(log_now_ms());
}

static void bt_draw(void)
{
    const bt_proto_t *bt = bt_link_state();
    const char *status = bt_link_status_text();
    int n = row_count();
    int page = menu_page_size();
    int row;

    display_clear(THEME_COLOR_BG);
    display_fill_rect(0, 0, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_TITLE_BG);
    display_draw_text(THEME_PAD, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                      "Bluetooth", THEME_COLOR_ACCENT, THEME_COLOR_TITLE_BG);
    display_fill_rect(0, THEME_TITLE_H - 1, DISPLAY_W - 1, THEME_TITLE_H - 1,
                      THEME_COLOR_FG);
    if (status != 0 && status[0] != '\0') {
        int tx = DISPLAY_W - THEME_PAD -
                 THEME_TEXT_W((int)strlen(status));

        if (tx > THEME_PAD)
            display_draw_text(tx, THEME_TITLE_TEXT_Y + THEME_TEXT_YOFF,
                              status, THEME_COLOR_DIM, THEME_COLOR_TITLE_BG);
    }

    for (row = 0; row < page; row++) {
        int idx = s_view.top + row;
        char scratch[24];
        int y = THEME_MENU_TOP + row * THEME_ITEM_H;

        if (idx >= n)
            break;
        menu_draw_item(y, row_label(idx, scratch, sizeof scratch),
                       row_value(idx), idx == s_view.cursor);
    }

    if (n <= 1 && bt->state != BT_STATE_SCANNING) {
        display_draw_text(THEME_PAD, THEME_MENU_TOP + page * THEME_ITEM_H,
                          "Turn Bluetooth on to pair", THEME_COLOR_DIM,
                          THEME_COLOR_BG);
    }
}

static void bt_select(void)
{
    unsigned long now = log_now_ms();
    const bt_proto_t *bt = bt_link_state();
    int row = s_view.cursor;
    int devices = bt->count;

    if (row == 0) {
        bt_link_set_enabled(!bt_link_enabled(), now);
    } else if (row >= 1 && row <= devices) {
        int i = row - 1;

        if (bt->state == BT_STATE_CONNECTED && bt->connected == i)
            bt_link_disconnect(now);
        else
            bt_link_connect(i, now);
    } else {
        bt_link_scan(now);
    }
    ui_invalidate_all();
}

static void bt_handle_event(const input_event_t *ev)
{
    unsigned long now = log_now_ms();
    const bt_proto_t *bt = bt_link_state();
    int n = row_count();
    int devices = bt->count;

    if (ev->kind == INPUT_LONG) {
        /* Hold on a device: forget it. */
        if (ev->type == KEY_SELECT && s_view.cursor >= 1 &&
            s_view.cursor <= devices) {
            bt_link_forget(s_view.cursor - 1, now);
            menu_view_move(&s_view, 0, n, menu_page_size());
            ui_invalidate_all();
        }
        return;
    }
    if (ev->kind == INPUT_RELEASE)
        return;

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_view, -1, n, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_view, +1, n, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        bt_select();
        break;
    case KEY_BACK:
        ui_pop_screen();
        break;
    default:
        break;
    }
    (void)bt;
}

const ui_screen_t bluetooth_screen = {
    "bluetooth",
    bt_on_enter,
    bt_on_exit,
    bt_draw,
    bt_handle_event,
};
