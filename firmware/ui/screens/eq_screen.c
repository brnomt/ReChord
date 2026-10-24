/*
 * eq_screen.c — Equalizer settings (BRNOMT-76).
 *
 *   Enabled   on/off (the effect is bypassed on Core B)
 *   Profile   opens the picker (5 built-ins + the custom slots on disk)
 *   Band N    SELECT enters edit mode; UP/DOWN move the band by 0.5 dB
 *             (the normal hold-to-repeat ladder gives fast sweeps);
 *             SELECT or BACK leaves edit mode.
 *
 * The AP only edits/publishes the curve; Core B applies it to the decoded
 * PCM block. Edits reach the DAC immediately; the file is persisted on
 * BACK / profile selection.
 */
#include <string.h>

#include "screens.h"
#include "../theme.h"
#include "../internal/hal.h"
#include "../../services/eq_profile.h"

#define EQ_ROW_ENABLED 0
#define EQ_ROW_PROFILE 1
#define EQ_ROW_BAND0   2

static menu_view_t s_view;
static uint8_t     s_editing;
static uint8_t     s_reenter_reset = 1;
static char        s_value_buf[12];

static const char s_band_label[EQ_BANDS_MAX][8] = {
    "Band 1", "Band 2", "Band 3", "Band 4", "Band 5",
    "Band 6", "Band 7", "Band 8", "Band 9", "Band 10"
};

static const char *eq_value_enabled(void)
{
    return eq_profile_enabled() ? "On" : "Off";
}

static const char *eq_value_profile(void)
{
    return eq_profile_name();
}

static const char *band_value(int band)
{
    const eq_curve_t *c = eq_profile_curve();

    if (band < 0 || band >= (int)c->count)
        return "";
    (void)eq_profile_format_gain(c->band[band].gain_tdb, s_value_buf,
                                 (unsigned)sizeof s_value_buf);
    return s_value_buf;
}

static const char *eq_value_band_0(void) { return band_value(0); }
static const char *eq_value_band_1(void) { return band_value(1); }
static const char *eq_value_band_2(void) { return band_value(2); }
static const char *eq_value_band_3(void) { return band_value(3); }
static const char *eq_value_band_4(void) { return band_value(4); }
static const char *eq_value_band_5(void) { return band_value(5); }
static const char *eq_value_band_6(void) { return band_value(6); }
static const char *eq_value_band_7(void) { return band_value(7); }
static const char *eq_value_band_8(void) { return band_value(8); }
static const char *eq_value_band_9(void) { return band_value(9); }

static const char *(*const s_band_value_fn[EQ_BANDS_MAX])(void) = {
    eq_value_band_0, eq_value_band_1, eq_value_band_2, eq_value_band_3,
    eq_value_band_4, eq_value_band_5, eq_value_band_6, eq_value_band_7,
    eq_value_band_8, eq_value_band_9,
};

static const char *eq_status(void)
{
    return s_editing ? "EDIT" : 0;
}

static void eq_on_enter(void)
{
    if (!eq_profile_ready())
        (void)eq_profile_boot();
    s_editing = 0;
    if (s_reenter_reset) {
        menu_view_init(&s_view);
        s_reenter_reset = 0;
    }
}

static void eq_on_exit(void)
{
    s_editing = 0;
    s_reenter_reset = 1;
}

static void eq_draw(void)
{
    menu_item_t items[EQ_BANDS_MAX + 2];
    menu_model_t model;
    const eq_curve_t *c = eq_profile_curve();
    int i, n = 0;

    items[n].label = "Enabled";
    items[n].value_text = eq_value_enabled;
    items[n].on_select = 0;
    n++;
    items[n].label = "Profile";
    items[n].value_text = eq_value_profile;
    items[n].on_select = 0;
    n++;

    for (i = 0; i < (int)c->count && i < EQ_BANDS_MAX; i++) {
        items[n].label = s_band_label[i];
        items[n].value_text = s_band_value_fn[i];
        items[n].on_select = 0;
        n++;
    }

    model.title = "Equalizer";
    model.items = items;
    model.item_count = n;
    model.status_text = eq_status;
    menu_draw(&model, &s_view);
}

static void eq_handle_event(const input_event_t *ev)
{
    const eq_curve_t *c = eq_profile_curve();
    int rows = EQ_ROW_BAND0 + (int)c->count;
    int band;

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;

    switch (ev->type) {
    case KEY_UP:
    case KEY_DOWN:
        if (s_editing) {
            band = s_view.cursor - EQ_ROW_BAND0;
            if (band >= 0)
                eq_profile_cycle_band(band, ev->type == KEY_UP ? +1 : -1);
        } else {
            menu_view_move(&s_view, ev->type == KEY_UP ? -1 : +1, rows,
                           menu_page_size());
        }
        ui_invalidate_all();
        return;

    case KEY_SELECT:
        if (s_editing) {
            s_editing = 0;
        } else if (s_view.cursor == EQ_ROW_ENABLED) {
            eq_profile_set_enabled(!eq_profile_enabled());
        } else if (s_view.cursor == EQ_ROW_PROFILE) {
            ui_push_screen(&eq_profile_screen);
        } else if (s_view.cursor >= EQ_ROW_BAND0) {
            s_editing = 1;
        }
        ui_invalidate_all();
        return;

    case KEY_BACK:
        if (s_editing) {
            s_editing = 0;
            ui_invalidate_all();
        } else {
            (void)eq_profile_save();
            ui_pop_screen();
        }
        return;

    default:
        return;
    }
}

const ui_screen_t eq_screen = {
    "eq",
    eq_on_enter,
    eq_on_exit,
    eq_draw,
    eq_handle_event,
};

/* ---- profile picker ---------------------------------------------------- */

static menu_view_t s_prof_view;
static uint8_t     s_store_mode;

static int custom_count(void)
{
    int n = eq_profile_count() - 5;

    return (n > 0) ? n : 0;
}

static void prof_on_enter(void)
{
    s_store_mode = 0;
    (void)eq_profile_scan();          /* files added from a PC show up here */
    menu_view_init(&s_prof_view);
}

static void prof_draw(void)
{
    menu_item_t items[16];
    menu_model_t model;
    int i, n = 0;

    if (s_store_mode) {
        int customs = custom_count();

        for (i = 0; i < customs; i++) {
            items[n].label = eq_profile_entry_name(5 + i);
            items[n].value_text = 0;
            items[n].on_select = 0;
            n++;
        }
        if (n == 0) {
            items[n].label = "(no slots)";
            items[n].value_text = 0;
            items[n].on_select = 0;
            n++;
        }
        model.title = "Save to slot";
    } else {
        int count = eq_profile_count();

        for (i = 0; i < count; i++) {
            items[n].label = eq_profile_entry_name(i);
            items[n].value_text = 0;
            items[n].on_select = 0;
            n++;
        }
        if (custom_count() > 0) {
            items[n].label = "Save current...";
            items[n].value_text = 0;
            items[n].on_select = 0;
            n++;
        }
        model.title = "Profile";
    }

    model.items = items;
    model.item_count = n;
    model.status_text = 0;
    menu_draw(&model, &s_prof_view);
}

static void prof_handle_event(const input_event_t *ev)
{
    int rows;

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;

    rows = s_store_mode ? custom_count()
                        : eq_profile_count() +
                              (custom_count() > 0 ? 1 : 0);
    if (rows < 1)
        rows = 1;

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_prof_view, -1, rows, menu_page_size());
        ui_invalidate_all();
        return;
    case KEY_DOWN:
        menu_view_move(&s_prof_view, +1, rows, menu_page_size());
        ui_invalidate_all();
        return;
    case KEY_SELECT:
        if (s_store_mode) {
            if (s_prof_view.cursor < custom_count() &&
                eq_profile_save_to_row(5 + s_prof_view.cursor) == 0)
                ui_pop_screen();
        } else if (s_prof_view.cursor == eq_profile_count()) {
            if (custom_count() > 0) {
                s_store_mode = 1;
                menu_view_init(&s_prof_view);
            }
        } else if (eq_profile_select(s_prof_view.cursor) == 0) {
            ui_pop_screen();
        }
        ui_invalidate_all();
        return;
    case KEY_BACK:
        if (s_store_mode) {
            s_store_mode = 0;
            menu_view_init(&s_prof_view);
            ui_invalidate_all();
        } else {
            ui_pop_screen();
        }
        return;
    default:
        return;
    }
}

const ui_screen_t eq_profile_screen = {
    "eq_profile",
    prof_on_enter,
    0,
    prof_draw,
    prof_handle_event,
};
