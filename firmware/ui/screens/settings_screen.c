/*
 * settings_screen.c — binds the config schema to a menu.
 *
 * One menu row per schema key (canonical order: play, brightness,
 * screen_off, auto_off, usb, resume, strip, tags, cpu, volume, browse,
 * cursor, shuffle, repeat_mode).
 * The row label is the config key name, the right-aligned value is
 * the key's current value formatted exactly like the config file writes it.
 *
 * Keys: UP/DOWN move, SELECT cycles the value (enum wraps through options,
 * int wraps its range, browse resets to empty), BACK saves and pops.
 */
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "theme_ui.h"
#include "../theme.h"
#include "../../theme/theme_runtime.h"
#include "../../services/rechord_files.h"
#include "../internal/hal.h"

static rechord_settings_t s_settings;
static menu_view_t        s_view;
/* The menu shows the live Theme row plus only the settings that are wired
 * (`visible` in the schema). The on-disk 14-key format is unchanged: hidden
 * keys keep round-tripping and are never offered as dead controls. */
#define SETTING_MAX_ROWS (SETTING_KEY_COUNT + 1)

static menu_item_t        s_items[SETTING_MAX_ROWS];
static int                s_row_count = 1; /* Theme + visible keys        */
static int                s_use_skin;

static const char *set_label(int index, char *buf, int cap)
{
    if (index < 0 || index >= s_row_count)
        return "";
    snprintf(buf, (size_t)cap, "%s", s_items[index].label);
    return buf;
}

static const char *set_value(int index, char *buf, int cap)
{
    const char *v = "";

    if (index < 0 || index >= s_row_count)
        return "";
    if (s_items[index].value_text != 0)
        v = s_items[index].value_text();
    snprintf(buf, (size_t)cap, "%s", v != 0 ? v : "");
    return buf;
}

static theme_skin_list_source_t s_source = { 0, set_label, set_value };
static unsigned           s_flush_at_ms;   /* volume flush deadline      */
static int                s_volume_dirty;  /* a volume save is pending   */

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
static const char *value_text_12(void) { return s_value_buf[12]; }
static const char *value_text_13(void) { return s_value_buf[13]; }

/* The active theme's name (from theme.c, no scratch buffer needed). */
static const char *value_text_theme(void) { return theme_get()->name; }

static const char *(*const s_value_text_fn[SETTING_KEY_COUNT])(void) = {
    value_text_0,  value_text_1,  value_text_2,  value_text_3,
    value_text_4,  value_text_5,  value_text_6,  value_text_7,
    value_text_8,  value_text_9,  value_text_10, value_text_11,
    value_text_12, value_text_13,
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
static void on_select_12(void) {
    settings_cycle(&setting_schema[12], &s_settings, +1);
    player_set_shuffle(s_settings.shuffle);      /* BRNOMT-90: apply now */
}
static void on_select_13(void) {
    settings_cycle(&setting_schema[13], &s_settings, +1);
    player_set_repeat(s_settings.repeat_mode);
}

/* The app registers this so a live theme change can swap the home renderer
 * under the open Settings screen without leaving it (BRNOMT-69). */
static void (*s_theme_applied)(void);

void settings_screen_set_theme_applied(void (*cb)(void))
{
    s_theme_applied = cb;
}

/* BRNOMT-69: cycle the installed packages (not just the compiled palettes).
 * A package is applied atomically (palette + layout + home renderer). The
 * choice is written to RECHORD.CFG like every other visible setting (and to
 * active_theme as the boot fallback for older configs), then the app syncs
 * the home screen that the new package declares. */
static void on_select_theme(void)
{
    static theme_pkg_info_t themes[THEME_PKG_MAX];
    const char *active = theme_pkg_active_id();
    int n, i, cur = -1;

    n = theme_pkg_scan(RECHORD_THEMES_DIR, themes, THEME_PKG_MAX);
    log_printf("theme: scan %d active '%s'", n, active);
    if (n <= 0)
        return;

    for (i = 0; i < n; i++) {
        if (strcmp(themes[i].id, active) == 0) {
            cur = i;
            break;
        }
    }

    for (i = 1; i <= n; i++) {
        int idx = (cur + i + n) % n;

        if (theme_pkg_apply(RECHORD_THEMES_DIR, themes[idx].id) != 0) {
            log_printf("theme: apply '%s' failed", themes[idx].id);
            continue;                 /* broken package: try the next one */
        }
        (void)theme_pkg_store(RECHORD_ACTIVE_THEME, themes[idx].id);
        snprintf(s_settings.theme, sizeof s_settings.theme, "%s",
                 themes[idx].id);
        settings_screen_store();
        log_printf("theme: applied '%s' home %d", themes[idx].id,
                   theme_layout()->home);
        if (s_theme_applied != 0)
            s_theme_applied();
        /* The settings screen is still on top: load the new theme's settings
         * skin (if any) instead of drawing the previous one. */
        s_use_skin = theme_ui_load(THEME_UI_SETTINGS);
        s_source.count = s_row_count;
        if (s_use_skin)
            theme_ui_list(&s_source);
        ui_invalidate_all();
        return;
    }
}

static void (*const s_on_select_fn[SETTING_KEY_COUNT])(void) = {
    on_select_0,  on_select_1,  on_select_2,  on_select_3,
    on_select_4,  on_select_5,  on_select_6,  on_select_7,
    on_select_8,  on_select_9,  on_select_10, on_select_11,
    on_select_12, on_select_13,
};

static menu_model_t s_model = {
    "Settings",
    s_items,
    0,
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
    int i, n = settings_visible_count();

    /* Theme first: it is a live picker (its own cfg key, not a schema row). */
    s_items[0].label = "Theme";
    s_items[0].value_text = value_text_theme;
    s_items[0].on_select = on_select_theme;

    for (i = 0; i < n; i++) {
        const setting_schema_t *sch = settings_visible_at(i);
        int k = (int)(sch - setting_schema);   /* scratch/callback index */

        s_items[i + 1].label = sch->name;
        s_items[i + 1].value_text = s_value_text_fn[k];
        s_items[i + 1].on_select = s_on_select_fn[k];
    }
    s_row_count = n + 1;
    s_model.item_count = s_row_count;
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
    /* This save covers any pending runtime-state write (volume), otherwise a
     * later flush would write the same values a second time. */
    s_volume_dirty = 0;
    s_flush_at_ms = 0;
    if (settings_view_save(&s_settings) != 0)
        log_printf("settings: save failed");
}

/*
 * Runtime-state writes (volume). The value is adopted immediately — the view
 * is what the config file is written from — but the save is armed instead of
 * executed, so holding vol+/- writes once when the keys go quiet.
 */
void settings_screen_volume_changed(int volume_db, unsigned now_ms)
{
    if (volume_db < -60)
        volume_db = -60;
    if (volume_db > 20)
        volume_db = 20;
    s_settings.volume_db = volume_db;
    s_flush_at_ms = now_ms + SETTINGS_FLUSH_QUIET_MS;
    s_volume_dirty = 1;
}

void settings_screen_flush_pending(unsigned now_ms)
{
    if (!s_volume_dirty)
        return;
    /* `now_ms - s_flush_at_ms` wraps correctly even past the 32-bit ms wrap,
     * and a zero deadline means the caller's clock is not usable: save now. */
    if (s_flush_at_ms == 0 ||
        (int)(now_ms - s_flush_at_ms) >= 0) {
        s_volume_dirty = 0;
        settings_screen_store();
    }
}

/* ---- screen vtable ----------------------------------------------------- */

static void settings_screen_on_enter(void)
{
    build_items();
    menu_view_init(&s_view);
    settings_screen_reload();
    s_use_skin = theme_ui_load(THEME_UI_SETTINGS);
    s_source.count = s_row_count;
    if (s_use_skin)
        theme_ui_list(&s_source);
}

static void settings_screen_on_exit(void)
{
    settings_screen_store();
}

static void settings_screen_draw(void)
{
    refresh_values();
    if (s_use_skin) {
        theme_skin_state_t st;

        s_source.count = s_row_count;
        theme_ui_list(&s_source);
        theme_ui_fill_state(&st, "Settings");
        theme_ui_draw(&st, s_view.cursor, s_view.top);
        return;
    }
    menu_draw(&s_model, &s_view);
}

static void settings_screen_handle_event(const input_event_t *ev)
{
    int nav;
    int rows;

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;                       /* listings act on PRESS/REPEAT only */

    if (s_use_skin) {
        theme_ui_note_input();
        s_source.count = s_row_count;
        theme_ui_list(&s_source);
        rows = theme_ui_rows();
        if (rows <= 0)
            rows = menu_page_size();
        nav = theme_ui_nav(ev->type);
        if (nav == THEME_SKIN_PREV || nav == THEME_SKIN_NEXT) {
            menu_view_move(&s_view, nav == THEME_SKIN_PREV ? -1 : +1,
                           s_row_count, rows);
            ui_invalidate_all();
        } else if (nav == THEME_SKIN_SELECT) {
            if (s_view.cursor >= 0 && s_view.cursor < s_row_count)
                s_items[s_view.cursor].on_select();
            ui_invalidate_all();
        } else if (nav == THEME_SKIN_BACK) {
            ui_pop_screen();   /* on_exit saves */
        }
        return;
    }

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_view, -1, s_row_count, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_view, +1, s_row_count, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (s_view.cursor >= 0 && s_view.cursor < s_row_count)
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
