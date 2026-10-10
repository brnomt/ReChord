/*
 * settings_screen.c — binds the 12-key config schema to a menu.
 *
 * One menu row per schema key (canonical order: play, brightness,
 * screen_off, auto_off, usb, resume, strip, tags, cpu, volume, browse,
 * cursor).  The row label is the config key name, the right-aligned value is
 * the key's current value formatted exactly like the config file writes it.
 *
 * Keys: UP/DOWN move, SELECT cycles the value (enum wraps through options,
 * int wraps its range, browse resets to empty), BACK saves and pops.
 */
#include <string.h>

#include "screens.h"
#include "../theme.h"
#include "../internal/hal.h"

static rechord_settings_t s_settings;
static menu_view_t        s_view;
static menu_item_t        s_items[SETTING_KEY_COUNT];

/* value_text callbacks must return stable storage: one scratch slot per key */
static char s_value_buf[SETTING_KEY_COUNT][32];

static const char *value_text_0(void)  { return s_value_buf[0]; }
static const char *value_text_1(void)  { return s_value_buf[1]; }
static const char *value_text_2(void)  { return s_value_buf[2]; }
static const char *value_text_3(void)  { return s_value_buf[3]; }
static const char *value_text_4(void)  { return s_value_buf[4]; }
static const char *value_text_5(void)  { return s_value_buf[5]; }
static const char *value_text_6(void)  { return s_value_buf[6]; }
static const char *value_text_7(void)  { return s_value_buf[7]; }
static const char *value_text_8(void)  { return s_value_buf[8]; }
static const char *value_text_9(void)  { return s_value_buf[9]; }
static const char *value_text_10(void) { return s_value_buf[10]; }
static const char *value_text_11(void) { return s_value_buf[11]; }

static const char *(*const s_value_text_fn[SETTING_KEY_COUNT])(void) = {
    value_text_0,  value_text_1,  value_text_2,  value_text_3,
    value_text_4,  value_text_5,  value_text_6,  value_text_7,
    value_text_8,  value_text_9,  value_text_10, value_text_11,
};

static void on_select_0(void)  { settings_cycle(&setting_schema[0],  &s_settings, +1); }
static void on_select_1(void)  { settings_cycle(&setting_schema[1],  &s_settings, +1); }
static void on_select_2(void)  { settings_cycle(&setting_schema[2],  &s_settings, +1); }
static void on_select_3(void)  { settings_cycle(&setting_schema[3],  &s_settings, +1); }
static void on_select_4(void)  { settings_cycle(&setting_schema[4],  &s_settings, +1); }
static void on_select_5(void)  { settings_cycle(&setting_schema[5],  &s_settings, +1); }
static void on_select_6(void)  { settings_cycle(&setting_schema[6],  &s_settings, +1); }
static void on_select_7(void)  { settings_cycle(&setting_schema[7],  &s_settings, +1); }
static void on_select_8(void)  { settings_cycle(&setting_schema[8],  &s_settings, +1); }
static void on_select_9(void)  { settings_cycle(&setting_schema[9],  &s_settings, +1); }
static void on_select_10(void) { settings_cycle(&setting_schema[10], &s_settings, +1); }
static void on_select_11(void) { settings_cycle(&setting_schema[11], &s_settings, +1); }

static void (*const s_on_select_fn[SETTING_KEY_COUNT])(void) = {
    on_select_0,  on_select_1,  on_select_2,  on_select_3,
    on_select_4,  on_select_5,  on_select_6,  on_select_7,
    on_select_8,  on_select_9,  on_select_10, on_select_11,
};

static const menu_model_t s_model = {
    "Settings",
    s_items,
    SETTING_KEY_COUNT,
};

static void refresh_values(void)
{
    int i;

    for (i = 0; i < SETTING_KEY_COUNT; i++)
        settings_format(&setting_schema[i], &s_settings,
                        s_value_buf[i], (int)sizeof(s_value_buf[i]));
}

static void build_items(void)
{
    int i;

    for (i = 0; i < SETTING_KEY_COUNT; i++) {
        s_items[i].label = setting_schema[i].name;
        s_items[i].value_text = s_value_text_fn[i];
        s_items[i].on_select = s_on_select_fn[i];
    }
}

/* ---- persistence ------------------------------------------------------- */

void settings_screen_reload(void)
{
    settings_view_defaults(&s_settings);
    if (settings_view_load(&s_settings) != 0) {
        log_printf("settings: load failed, using defaults");
        settings_view_defaults(&s_settings);
    }
    refresh_values();
}

void settings_screen_store(void)
{
    if (settings_view_save(&s_settings) != 0)
        log_printf("settings: save failed");
}

/* ---- screen vtable ----------------------------------------------------- */

static void settings_screen_on_enter(void)
{
    build_items();
    menu_view_init(&s_view);
    settings_screen_reload();
}

static void settings_screen_on_exit(void)
{
    settings_screen_store();
}

static void settings_screen_draw(void)
{
    refresh_values();
    menu_draw(&s_model, &s_view);
}

static void settings_screen_handle_event(const input_event_t *ev)
{
    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_view, -1, SETTING_KEY_COUNT, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_view, +1, SETTING_KEY_COUNT, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (s_view.cursor >= 0 && s_view.cursor < SETTING_KEY_COUNT)
            s_items[s_view.cursor].on_select();
        ui_invalidate_all();
        break;
    case KEY_BACK:
        ui_pop_screen();   /* on_exit saves */
        break;
    default:
        break;
    }
}

const ui_screen_t settings_screen = {
    "settings",
    settings_screen_on_enter,
    settings_screen_on_exit,
    settings_screen_draw,
    settings_screen_handle_event,
};

const rechord_settings_t *settings_screen_get(void)
{
    return &s_settings;
}

const menu_model_t *settings_screen_model(void)
{
    build_items();
    return &s_model;
}

const menu_view_t *settings_screen_view(void)
{
    return &s_view;
}
