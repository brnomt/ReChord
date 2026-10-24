/*
 * settings_schema.h — ReChord configuration schema.
 *
 * The user-facing config is a plain key=value file. The 15 schema keys match
 * the documented on-device schema (docs/re/refcfw-analysis.md §6c shows a live
 * REFCFW.CFG with `play=repeat_track brightness=8 screen_off=30 auto_off=15
 * usb=ask resume=folder strip=off tags=on cpu=auto volume=-23 browse=
 * cursor=0`); `theme` is a ReChord addition persisted in the same file. We
 * keep the same key names so config files stay portable; the parsing/
 * formatting code is ours (settings service implements it).
 *
 * Not every stored key is a useful control: `visible` marks the subset the
 * Settings menu offers (the rest are playback duplicates or hardware features
 * that are not wired yet). Hidden keys still round-trip through the file.
 *
 * This header is the single source of truth for:
 *   - the in-memory settings struct (rechord_settings_t),
 *   - the per-key metadata table (name, value kind, field offset, options),
 *   - default values (seeded from the observed on-device config; auto_off is
 *     the one exception: ReChord's standby timer defaults to 5 min).
 *
 * Part of the frontend workstream (settings_screen binds this schema to a
 * menu; the settings service in firmware/services/ persists the struct).
 */
#ifndef RECHORD_UI_SETTINGS_SCHEMA_H
#define RECHORD_UI_SETTINGS_SCHEMA_H

#include <stddef.h>
#include "../services/settings.h"

#define RECHORD_BROWSE_PATH_MAX 64
#define RECHORD_THEME_ID_MAX    32
#define SETTING_KEY_COUNT       16

/* In-memory settings.  Field order is our own; persistence must go through
 * the schema table (or the key names), never through raw offsets on disk. */
typedef struct rechord_settings {
    int  play_mode;                   /* key "play"       (enum value)      */
    int  brightness;                  /* key "brightness" (0..10)           */
    int  screen_off_s;                /* key "screen_off" (opt value, minutes; field name is historical) */
    int  auto_off_min;                /* key "auto_off"   (opt value, minutes, 0=off) */
    int  usb_mode;                    /* key "usb"        (enum value)      */
    int  resume_mode;                 /* key "resume"     (enum value)      */
    int  strip;                       /* key "strip"      (0=off, 1=on)     */
    int  tags;                        /* key "tags"       (0=off, 1=on)     */
    int  cpu_mode;                    /* key "cpu"        (enum value)      */
    int  volume_db;                   /* key "volume"     (dB, -60..0)      */
    char browse[RECHORD_BROWSE_PATH_MAX]; /* key "browse"  (last path)      */
    int  cursor;                      /* key "cursor"     (last position)   */
    int  shuffle;                     /* key "shuffle"    (0=off, 1=on)     */
    int  repeat_mode;                 /* key "repeat_mode"(0=off,1=all,2=one)*/
    int  fade_ms;                     /* key "fade"       (0/150/300/500 ms) */
    char theme[RECHORD_THEME_ID_MAX]; /* key "theme"      (package id)      */
    int  replaygain_mode;             /* key "replaygain" (0=off..3=auto)   */
} rechord_settings_t;

typedef enum setting_kind {
    SETTING_KIND_ENUM = 0,  /* one of a fixed option list (stored as value) */
    SETTING_KIND_INT,       /* integer range with step (wraps when cycled)  */
    SETTING_KIND_STR        /* short string (cycle = clear)                 */
} setting_kind_t;

typedef struct setting_opt {
    const char *text;   /* value as written in the config file              */
    int         value;  /* value as stored in rechord_settings_t            */
} setting_opt_t;

typedef struct setting_schema {
    const char   *name;   /* exact config key name                          */
    setting_kind_t kind;
    size_t         offset;   /* offsetof() of the field in rechord_settings_t */
    /* 1 = shown in a Settings category. The rest stay in the schema so the
     * on-disk 15-key format is preserved and older configs keep round-
     * tripping, but they are not offered to the user (duplicates of the
     * playback controls, or hardware features that are not wired yet). */
    int            visible;
    union {
        struct { const setting_opt_t *opts; int count; } e; /* KIND_ENUM     */
        struct { int min; int max; int step; }           r; /* KIND_INT      */
        struct { int cap; }                              s; /* KIND_STR      */
    } u;
} setting_schema_t;

/* The schema table, in canonical key order:
 * play, brightness, screen_off, auto_off, usb, resume,
 * strip, tags, cpu, volume, browse, cursor, shuffle, repeat_mode, fade,
 * replaygain (16 keys).
 *
 * `visible` marks the functional subset the Settings groups offer. Today:
 * resume, volume, shuffle, repeat_mode, fade. `theme` is a live picker
 * outside this table (persisted in the same file under its own key). */
extern const setting_schema_t setting_schema[SETTING_KEY_COUNT];

/* The visible subset (Settings menu order). `index` is 0..count-1. */
int settings_visible_count(void);
const setting_schema_t *settings_visible_at(int index);

/* Reset every field to its default (the observed on-device values). */
void settings_view_defaults(rechord_settings_t *s);

/* Storage<->view adapters (integration 2026-10-09): services/settings.h owns
 * the on-disk `settings_t` (string values); this header owns the UI VIEW
 * (typed enums for the settings screen). Function names are distinct
 * (`settings_view_*`) so both can link in one image. The target glue lives in
 * firmware/ui/settings_view.c; the UI host tests substitute it. */
int  settings_view_load(rechord_settings_t *s);
int  settings_view_save(const rechord_settings_t *s);
void settings_view_from_store(const settings_t *in, rechord_settings_t *out);
void settings_view_to_store(const rechord_settings_t *in, settings_t *out);

/* Advance the key's value by dir (+1 / -1): enums wrap through the option
 * list, ints wrap inside their range, strings clear on any cycle. */
void settings_cycle(const setting_schema_t *sch, rechord_settings_t *s,
                    int dir);

/* Format the key's current value as text ("repeat_track", "8", ...). */
void settings_format(const setting_schema_t *sch, const rechord_settings_t *s,
                     char *buf, int cap);

/* Look up a schema entry by config key name; NULL if unknown. */
const setting_schema_t *settings_schema_find(const char *name);

#endif /* RECHORD_UI_SETTINGS_SCHEMA_H */
