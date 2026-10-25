/*
 * controls_screen.c — Settings > Interface > Controls (BRNOMT-107).
 *
 * Two persisted choices:
 *   VOL keys : Normal / Swapped  (menu navigation only; Now Playing keeps
 *               VOL+ = louder)
 *   Hold VOL : Skip track / Seek (long-press VOL+ / VOL- on Now Playing)
 *
 * The values live in rechord_settings_t outside the schema table; the setters
 * save immediately, so a power-off from this screen cannot lose the choice.
 */
#include <stdio.h>

#include "screens.h"
#include "../internal/hal.h"

static menu_view_t s_view;

static const char *value_vol(void)
{
    return settings_screen_get()->vol_orient == VOL_ORIENT_SWAPPED
               ? "Swapped" : "Normal";
}

static const char *value_hold(void)
{
    return settings_screen_get()->hold_vol == HOLD_VOL_SEEK
               ? "Seek" : "Skip track";
}

static void on_select_vol(void)
{
    int swapped = settings_screen_get()->vol_orient == VOL_ORIENT_SWAPPED;

    settings_screen_set_vol_orient(!swapped);
    ui_invalidate_all();
}

static void on_select_hold(void)
{
    int seek = settings_screen_get()->hold_vol == HOLD_VOL_SEEK;

    settings_screen_set_hold_vol(!seek);
    ui_invalidate_all();
}

static void controls_on_enter(void)
{
    s_view.cursor = 0;
    s_view.top = 0;
    ui_invalidate_all();
}

static void controls_draw(void)
{
    menu_item_t items[2] = {
        { "VOL keys", value_vol, on_select_vol },
        { "Hold VOL", value_hold, on_select_hold },
    };
    menu_model_t model = { "Controls", items, 2, NULL };

    menu_draw(&model, &s_view);
}

static void controls_handle_event(const input_event_t *ev)
{
    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_view, -1, 2, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_view, +1, 2, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (s_view.cursor == 0)
            on_select_vol();
        else
            on_select_hold();
        break;
    case KEY_BACK:
        ui_pop_screen();
        break;
    default:
        break;
    }
}

const ui_screen_t controls_screen = {
    "controls",
    controls_on_enter,
    NULL,
    controls_draw,
    controls_handle_event,
    0,
};
