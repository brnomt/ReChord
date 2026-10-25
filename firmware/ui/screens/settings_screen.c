/*
 * settings_screen.c — config schema bound to a categorized menu.
 *
 * Top level: one row per category (Playing, Interface). Selecting a category
 * pushes settings_group_screen, which lists that category's schema keys (and
 * the live Theme picker/Equalizer/Controls under Interface/Playback). The
 * on-disk 19-key format is unchanged; the categories only decide what the
 * user sees and in what order.
 *
 * Playing:   Volume, Shuffle, Repeat, Resume, Fade, Auto-off, ReplayGain,
 *            Equalizer
 * Interface: Brightness, Theme, Controls (BRNOMT-107)
 *
 * The System diagnostics menu stays in the root menu (it is a debug view, not
 * a configuration group). BACK on a group returns to the category list; BACK
 * on the category list saves and pops, exactly as before.
 */
#include <stdio.h>
#include <string.h>

#include "screens.h"
#include "theme_ui.h"
#include "../theme.h"
#include "../../theme/theme_runtime.h"
#include "../../services/rechord_files.h"
#include "../../services/eq_profile.h"
#include "../../player/player.h"
#include "../internal/hal.h"

static rechord_settings_t s_settings;

/* ---- per-key callbacks (indexed by the schema table) -------------------- */

/* value_text callbacks must return stable storage. One shared slot is enough:
 * menu_draw() consumes it immediately per row, and set_value() copies it. */
static char s_value_buf[32];

static const char *value_text_for(int k)
{
    settings_format(&setting_schema[k], &s_settings,
                    s_value_buf, (int)sizeof s_value_buf);
    return s_value_buf;
}

#define VALUE_TEXT(n) \
    static const char *value_text_##n(void) { return value_text_for(n); }
VALUE_TEXT(0)
VALUE_TEXT(1)
VALUE_TEXT(2)
VALUE_TEXT(3)
VALUE_TEXT(4)
VALUE_TEXT(5)
VALUE_TEXT(6)
VALUE_TEXT(7)
VALUE_TEXT(8)
VALUE_TEXT(9)
VALUE_TEXT(10)
VALUE_TEXT(11)
VALUE_TEXT(12)
VALUE_TEXT(13)
VALUE_TEXT(14)
VALUE_TEXT(15)
#undef VALUE_TEXT

/* The active theme's name (from theme.c, no scratch buffer needed). */
static const char *value_text_theme(void) { return theme_get()->name; }

/* Active EQ profile label (BRNOMT-76; the curve itself lives in eq_profile). */
static const char *value_text_eq(void) { return eq_profile_name(); }

static void on_select_eq(void)
{
    ui_push_screen(&eq_screen);
}

static void on_select_controls(void)
{
    ui_push_screen(&controls_screen);
}

static const char *(*const s_value_text_fn[SETTING_KEY_COUNT])(void) = {
    value_text_0,  value_text_1,  value_text_2,  value_text_3,
    value_text_4,  value_text_5,  value_text_6,  value_text_7,
    value_text_8,  value_text_9,  value_text_10, value_text_11,
    value_text_12, value_text_13, value_text_14, value_text_15,
};

static void on_select_0(void)  { settings_cycle(&setting_schema[0],  &s_settings, +1); }
static void on_select_1(void) {
    settings_cycle(&setting_schema[1], &s_settings, +1);
    display_set_brightness(s_settings.brightness);   /* live backlight PWM */
}
static void on_select_2(void)  { settings_cycle(&setting_schema[2],  &s_settings, +1); }
static void on_select_3(void)  { settings_cycle(&setting_schema[3],  &s_settings, +1); }
static void on_select_4(void)  { settings_cycle(&setting_schema[4],  &s_settings, +1); }
static void on_select_5(void)  { settings_cycle(&setting_schema[5],  &s_settings, +1); }
static void on_select_6(void)  { settings_cycle(&setting_schema[6],  &s_settings, +1); }
static void on_select_7(void)  { settings_cycle(&setting_schema[7],  &s_settings, +1); }
static void on_select_8(void)  { settings_cycle(&setting_schema[8],  &s_settings, +1); }
static void on_select_9(void) {
    settings_cycle(&setting_schema[9], &s_settings, +1);
    player_set_volume(s_settings.volume_db);     /* apply while playing too */
}
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
static void on_select_14(void) {
    settings_cycle(&setting_schema[14], &s_settings, +1);
    player_set_fade_ms(s_settings.fade_ms);      /* BRNOMT-67: apply now */
}
static void on_select_15(void) {
    settings_cycle(&setting_schema[15], &s_settings, +1);
    player_set_replaygain_mode(s_settings.replaygain_mode);  /* BRNOMT-106 */
}

static void (*const s_on_select_fn[SETTING_KEY_COUNT])(void) = {
    on_select_0,  on_select_1,  on_select_2,  on_select_3,
    on_select_4,  on_select_5,  on_select_6,  on_select_7,
    on_select_8,  on_select_9,  on_select_10, on_select_11,
    on_select_12, on_select_13, on_select_14, on_select_15,
};

/* ---- list source shared by the category and group screens -------------- */

static const menu_item_t *s_list_items;   /* whichever list is on top     */
static int                s_list_count;   /* rows in the list being drawn */
static int                s_use_skin;
static unsigned           s_flush_at_ms;  /* volume flush deadline        */
static int                s_volume_dirty; /* a volume save is pending     */

static const char *set_label(int index, char *buf, int cap)
{
    if (s_list_items == 0 || index < 0 || index >= s_list_count)
        return "";
    snprintf(buf, (size_t)cap, "%s", s_list_items[index].label);
    return buf;
}

static const char *set_value(int index, char *buf, int cap)
{
    const char *v = "";

    if (s_list_items == 0 || index < 0 || index >= s_list_count)
        return "";
    if (s_list_items[index].value_text != 0)
        v = s_list_items[index].value_text();
    snprintf(buf, (size_t)cap, "%s", v != 0 ? v : "");
    return buf;
}

static theme_skin_list_source_t s_source = { 0, set_label, set_value };

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
        s_source.count = s_list_count;
        if (s_use_skin)
            theme_ui_list(&s_source);
        ui_invalidate_all();
        return;
    }
}

/* ---- categories --------------------------------------------------------- */

typedef enum {
    GROUP_KEY = 0,     /* schema key in `key`                                */
    GROUP_THEME,       /* live theme picker                                  */
    GROUP_EQ,          /* Equalizer screen (BRNOMT-76)                       */
    GROUP_CONTROLS     /* volume-key mapping (BRNOMT-107)                    */
} setting_group_kind_t;

typedef struct {
    const char *label;
    const char *key;   /* schema key; NULL for the special rows */
    setting_group_kind_t kind;
} setting_group_item_t;

/* Playback keeps the controls and the two audio features together. */
static const setting_group_item_t s_playback_items[] = {
    { "Volume",  "volume", GROUP_KEY },
    { "Shuffle", "shuffle", GROUP_KEY },
    { "Repeat",  "repeat_mode", GROUP_KEY },
    { "Resume",  "resume", GROUP_KEY },
    { "Fade",    "fade", GROUP_KEY },
    { "Auto-off", "auto_off", GROUP_KEY },
    { "ReplayGain", "replaygain", GROUP_KEY },
    { "Equalizer", 0, GROUP_EQ },
    { 0, 0, GROUP_KEY },
};

/* Interface: live backlight brightness, the theme picker and the volume-key
 * mapping (screen_off is still hidden until it actually controls something). */
static const setting_group_item_t s_interface_items[] = {
    { "Brightness", "brightness", GROUP_KEY },
    { "Theme", 0, GROUP_THEME },
    { "Controls", 0, GROUP_CONTROLS },
    { 0, 0, GROUP_KEY },
};

static const setting_group_item_t *const s_group_items_spec[] = {
    s_playback_items, s_interface_items,
};
static const char *const s_group_titles[] = { "Playback", "Interface" };
#define SETTING_GROUP_COUNT 2

static menu_item_t s_cat_items[SETTING_GROUP_COUNT];
static menu_view_t s_view;
static menu_model_t s_model = { "Settings", s_cat_items, SETTING_GROUP_COUNT, NULL };

static menu_item_t s_group_items[8];   /* Playback has exactly 8 rows */
static menu_model_t s_group_model = { "Playback", s_group_items, 0, NULL };
static menu_view_t s_group_view;
static int         s_group;

static void open_playing(void);
static void open_interface(void);

static void build_categories(void)
{
    s_cat_items[0].label = "Playback";
    s_cat_items[0].value_text = 0;
    s_cat_items[0].on_select = open_playing;
    s_cat_items[1].label = "Interface";
    s_cat_items[1].value_text = 0;
    s_cat_items[1].on_select = open_interface;
    s_model.item_count = SETTING_GROUP_COUNT;
}

static void build_group_items(void)
{
    const setting_group_item_t *spec = s_group_items_spec[s_group];
    int n = 0;

    while (spec[n].label != 0 && n < (int)(sizeof s_group_items /
                                           sizeof s_group_items[0])) {
        menu_item_t *it = &s_group_items[n];

        it->label = spec[n].label;
        if (spec[n].kind == GROUP_THEME) {
            it->value_text = value_text_theme;
            it->on_select = on_select_theme;
        } else if (spec[n].kind == GROUP_EQ) {
            it->value_text = value_text_eq;
            it->on_select = on_select_eq;
        } else if (spec[n].kind == GROUP_CONTROLS) {
            it->value_text = 0;
            it->on_select = on_select_controls;
        } else {
            const setting_schema_t *sch = settings_schema_find(spec[n].key);
            int k = (sch != 0) ? (int)(sch - setting_schema) : -1;

            it->value_text = (k >= 0) ? s_value_text_fn[k] : 0;
            it->on_select = (k >= 0) ? s_on_select_fn[k] : 0;
        }
        n++;
    }
    s_group_model.title = s_group_titles[s_group];
    s_group_model.item_count = n;
}

/* ---- persistence ------------------------------------------------------- */

void settings_screen_reload(void)
{
    settings_view_defaults(&s_settings);
    if (settings_view_load(&s_settings) != 0) {
        log_printf("settings: load failed, using defaults");
        settings_view_defaults(&s_settings);
    }
    menu_view_init(&s_view);          /* a fresh visit starts at the top */
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

/* BRNOMT-107: Controls values are persisted like the theme (outside the
 * schema table). The Controls screen calls these and they save immediately,
 * so leaving via power-off cannot lose the choice. */
void settings_screen_set_vol_orient(int swapped)
{
    s_settings.vol_orient = swapped ? VOL_ORIENT_SWAPPED : VOL_ORIENT_NORMAL;
    settings_screen_store();
}

void settings_screen_set_hold_vol(int seek)
{
    s_settings.hold_vol = seek ? HOLD_VOL_SEEK : HOLD_VOL_SKIP;
    settings_screen_store();
}

/* Quick settings applies these immediately and persists them (the Settings
 * screen's own rows defer the write until BACK). */
void settings_screen_set_shuffle(int on)
{
    s_settings.shuffle = on ? 1 : 0;
    player_set_shuffle(s_settings.shuffle);
    settings_screen_store();
}

void settings_screen_set_repeat(int mode)
{
    if (mode < 0 || mode > 2)
        mode = 0;
    s_settings.repeat_mode = mode;
    player_set_repeat(mode);
    settings_screen_store();
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

/* ---- category screen --------------------------------------------------- */

static void settings_screen_on_enter(void)
{
    build_categories();
    s_list_items = s_cat_items;
    s_list_count = s_model.item_count;
    s_use_skin = theme_ui_load(THEME_UI_SETTINGS);
    s_source.count = s_list_count;
    if (s_use_skin)
        theme_ui_list(&s_source);
}

static void settings_screen_on_exit(void)
{
    settings_screen_store();
}

static void settings_screen_draw(void)
{
    s_list_items = s_cat_items;
    s_list_count = s_model.item_count;
    if (s_use_skin) {
        theme_skin_state_t st;

        s_source.count = s_list_count;
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
        s_source.count = s_model.item_count;
        theme_ui_list(&s_source);
        rows = theme_ui_rows();
        if (rows <= 0)
            rows = menu_page_size();
        nav = theme_ui_nav(ev->type);
        if (nav == THEME_SKIN_PREV || nav == THEME_SKIN_NEXT) {
            menu_view_move(&s_view, nav == THEME_SKIN_PREV ? -1 : +1,
                           s_model.item_count, rows);
            ui_invalidate_all();
        } else if (nav == THEME_SKIN_SELECT) {
            if (s_view.cursor >= 0 && s_view.cursor < s_model.item_count &&
                s_cat_items[s_view.cursor].on_select != 0)
                s_cat_items[s_view.cursor].on_select();
            ui_invalidate_all();
        } else if (nav == THEME_SKIN_BACK) {
            ui_pop_screen();   /* on_exit saves */
        }
        return;
    }

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_view, -1, s_model.item_count, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_view, +1, s_model.item_count, menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (s_view.cursor >= 0 && s_view.cursor < s_model.item_count &&
            s_cat_items[s_view.cursor].on_select != 0)
            s_cat_items[s_view.cursor].on_select();
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
    1,
};

/* ---- group screen ------------------------------------------------------ */

static void settings_group_on_enter(void)
{
    build_group_items();
    menu_view_init(&s_group_view);
    s_list_items = s_group_items;
    s_list_count = s_group_model.item_count;
    s_use_skin = theme_ui_load(THEME_UI_SETTINGS);
    s_source.count = s_list_count;
    if (s_use_skin)
        theme_ui_list(&s_source);
}

static void settings_group_draw(void)
{
    s_list_items = s_group_items;
    s_list_count = s_group_model.item_count;
    if (s_use_skin) {
        theme_skin_state_t st;

        s_source.count = s_list_count;
        theme_ui_list(&s_source);
        theme_ui_fill_state(&st, s_group_model.title);
        theme_ui_draw(&st, s_group_view.cursor, s_group_view.top);
        return;
    }
    menu_draw(&s_group_model, &s_group_view);
}

static void settings_group_handle_event(const input_event_t *ev)
{
    int nav;
    int rows;

    if (ev->kind == INPUT_LONG || ev->kind == INPUT_RELEASE)
        return;                       /* listings act on PRESS/REPEAT only */

    if (s_use_skin) {
        theme_ui_note_input();
        s_source.count = s_group_model.item_count;
        theme_ui_list(&s_source);
        rows = theme_ui_rows();
        if (rows <= 0)
            rows = menu_page_size();
        nav = theme_ui_nav(ev->type);
        if (nav == THEME_SKIN_PREV || nav == THEME_SKIN_NEXT) {
            menu_view_move(&s_group_view, nav == THEME_SKIN_PREV ? -1 : +1,
                           s_group_model.item_count, rows);
            ui_invalidate_all();
        } else if (nav == THEME_SKIN_SELECT) {
            if (s_group_view.cursor >= 0 &&
                s_group_view.cursor < s_group_model.item_count &&
                s_group_items[s_group_view.cursor].on_select != 0)
                s_group_items[s_group_view.cursor].on_select();
            ui_invalidate_all();
        } else if (nav == THEME_SKIN_BACK) {
            ui_pop_screen();   /* back to the category list */
        }
        return;
    }

    switch (ev->type) {
    case KEY_UP:
        menu_view_move(&s_group_view, -1, s_group_model.item_count,
                       menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_DOWN:
        menu_view_move(&s_group_view, +1, s_group_model.item_count,
                       menu_page_size());
        ui_invalidate_all();
        break;
    case KEY_SELECT:
        if (s_group_view.cursor >= 0 &&
            s_group_view.cursor < s_group_model.item_count &&
            s_group_items[s_group_view.cursor].on_select != 0)
            s_group_items[s_group_view.cursor].on_select();
        ui_invalidate_all();
        break;
    case KEY_BACK:
        ui_pop_screen();   /* back to the category list */
        break;
    default:
        break;
    }
}

static const ui_screen_t settings_group_screen = {
    "settings-group",
    settings_group_on_enter,
    NULL,
    settings_group_draw,
    settings_group_handle_event,
    1,
};

static void open_group(int group)
{
    s_group = group;
    ui_push_screen(&settings_group_screen);
}

static void open_playing(void)   { open_group(0); }
static void open_interface(void) { open_group(1); }

/* ---- accessors --------------------------------------------------------- */

const rechord_settings_t *settings_screen_get(void)
{
    return &s_settings;
}

const menu_model_t *settings_screen_model(void)
{
    build_categories();
    return &s_model;
}

const menu_view_t *settings_screen_view(void)
{
    return &s_view;
}

const menu_model_t *settings_group_model(void)
{
    build_group_items();
    return &s_group_model;
}

const menu_view_t *settings_group_view(void)
{
    return &s_group_view;
}
