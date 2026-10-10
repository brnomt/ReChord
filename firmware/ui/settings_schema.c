/*
 * settings_schema.c — the 12-key schema table + value helpers.
 *
 * Key names exactly match the on-device config schema (facts from
 * docs/re/refcfw-analysis.md §6c); option texts are the observed values plus
 * a small documented option set of our own design.  Defaults are seeded
 * from the observed on-device REFCFW.CFG values so a fresh install behaves
 * like the device users already know.
 */
#include <stdio.h>
#include <string.h>

#include "settings_schema.h"

/* ---- option lists (our own design around the observed values) ---------- */

static const setting_opt_t opts_play[] = {
    { "repeat_track",   0 },
    { "repeat_folder",  1 },
    { "repeat_all",     2 },
    { "shuffle",        3 },
};

static const setting_opt_t opts_screen_off[] = {
    { "off",  0 },
    { "10",  10 },
    { "30",  30 },   /* observed default */
    { "60",  60 },
    { "120",120 },
};

static const setting_opt_t opts_auto_off[] = {
    { "off",  0 },
    { "5",    5 },
    { "15",  15 },   /* observed default */
    { "30",  30 },
    { "60",  60 },
};

static const setting_opt_t opts_usb[] = {
    { "ask",    0 },   /* observed default */
    { "charge", 1 },
    { "msc",    2 },
};

static const setting_opt_t opts_resume[] = {
    { "off",   0 },
    { "folder",1 },   /* observed default */
    { "track", 2 },
};

static const setting_opt_t opts_off_on[] = {
    { "off", 0 },
    { "on",  1 },
};

static const setting_opt_t opts_cpu[] = {
    { "auto", 0 },   /* observed default */
    { "low",  1 },
    { "high", 2 },
};

/* ---- the schema table (canonical key order) --------------------------- */

const setting_schema_t setting_schema[SETTING_KEY_COUNT] = {
    { "play",       SETTING_KIND_ENUM, offsetof(rechord_settings_t, play_mode),
      { .e = { opts_play, 4 } } },
    { "brightness", SETTING_KIND_INT,  offsetof(rechord_settings_t, brightness),
      { .r = { 0, 10, 1 } } },
    { "screen_off", SETTING_KIND_ENUM, offsetof(rechord_settings_t, screen_off_s),
      { .e = { opts_screen_off, 5 } } },
    { "auto_off",   SETTING_KIND_ENUM, offsetof(rechord_settings_t, auto_off_min),
      { .e = { opts_auto_off, 5 } } },
    { "usb",        SETTING_KIND_ENUM, offsetof(rechord_settings_t, usb_mode),
      { .e = { opts_usb, 3 } } },
    { "resume",     SETTING_KIND_ENUM, offsetof(rechord_settings_t, resume_mode),
      { .e = { opts_resume, 3 } } },
    { "strip",      SETTING_KIND_ENUM, offsetof(rechord_settings_t, strip),
      { .e = { opts_off_on, 2 } } },
    { "tags",       SETTING_KIND_ENUM, offsetof(rechord_settings_t, tags),
      { .e = { opts_off_on, 2 } } },
    { "cpu",        SETTING_KIND_ENUM, offsetof(rechord_settings_t, cpu_mode),
      { .e = { opts_cpu, 3 } } },
    { "volume",     SETTING_KIND_INT,  offsetof(rechord_settings_t, volume_db),
      { .r = { -60, 0, 1 } } },
    { "browse",     SETTING_KIND_STR,  offsetof(rechord_settings_t, browse),
      { .s = { RECHORD_BROWSE_PATH_MAX } } },
    { "cursor",     SETTING_KIND_INT,  offsetof(rechord_settings_t, cursor),
      { .r = { 0, 9999, 1 } } },
};

/* ---- helpers ----------------------------------------------------------- */

static int *field_int(const setting_schema_t *sch, rechord_settings_t *s)
{
    return (int *)((char *)s + sch->offset);
}

void settings_view_defaults(rechord_settings_t *s)
{
    /* observed on-device values (docs/re/refcfw-analysis.md §6c) */
    s->play_mode = 0;      /* repeat_track */
    s->brightness = 8;
    s->screen_off_s = 30;
    s->auto_off_min = 15;
    s->usb_mode = 0;       /* ask */
    s->resume_mode = 1;    /* folder */
    s->strip = 0;          /* off */
    s->tags = 1;           /* on */
    s->cpu_mode = 0;       /* auto */
    s->volume_db = -23;
    s->browse[0] = '\0';
    s->cursor = 0;
}

static void cycle_enum(const setting_schema_t *sch, rechord_settings_t *s,
                       int dir)
{
    const setting_opt_t *opts = sch->u.e.opts;
    int count = sch->u.e.count;
    int cur = *field_int(sch, s);
    int i, idx = 0;

    for (i = 0; i < count; i++) {
        if (opts[i].value == cur) {
            idx = i;
            break;
        }
    }
    idx = (idx + dir) % count;
    if (idx < 0)
        idx += count;
    *field_int(sch, s) = opts[idx].value;
}

static void cycle_int(const setting_schema_t *sch, rechord_settings_t *s,
                      int dir)
{
    int v = *field_int(sch, s) + dir * sch->u.r.step;
    int min = sch->u.r.min;
    int max = sch->u.r.max;

    while (v > max)
        v = min + (v - max - 1);   /* wrap into range */
    while (v < min)
        v = max - (min - v - 1);
    *field_int(sch, s) = v;
}

void settings_cycle(const setting_schema_t *sch, rechord_settings_t *s,
                    int dir)
{
    if (sch == NULL || s == NULL)
        return;
    switch (sch->kind) {
    case SETTING_KIND_ENUM:
        cycle_enum(sch, s, dir);
        break;
    case SETTING_KIND_INT:
        cycle_int(sch, s, dir);
        break;
    case SETTING_KIND_STR:
        /* strings have no cycle: clear (reset to empty) */
        ((char *)s + sch->offset)[0] = '\0';
        break;
    }
}

void settings_format(const setting_schema_t *sch, const rechord_settings_t *s,
                     char *buf, int cap)
{
    if (buf == NULL || cap <= 0)
        return;
    buf[0] = '\0';
    if (sch == NULL || s == NULL)
        return;

    switch (sch->kind) {
    case SETTING_KIND_ENUM: {
        const setting_opt_t *opts = sch->u.e.opts;
        int cur = *(const int *)((const char *)s + sch->offset);
        int i;

        snprintf(buf, (size_t)cap, "??");
        for (i = 0; i < sch->u.e.count; i++) {
            if (opts[i].value == cur) {
                snprintf(buf, (size_t)cap, "%s", opts[i].text);
                break;
            }
        }
        break;
    }
    case SETTING_KIND_INT:
        snprintf(buf, (size_t)cap, "%d",
                 *(const int *)((const char *)s + sch->offset));
        break;
    case SETTING_KIND_STR:
        snprintf(buf, (size_t)cap, "%s", (const char *)s + sch->offset);
        break;
    }
}

const setting_schema_t *settings_schema_find(const char *name)
{
    int i;

    if (name == NULL)
        return NULL;
    for (i = 0; i < SETTING_KEY_COUNT; i++) {
        if (strcmp(setting_schema[i].name, name) == 0)
            return &setting_schema[i];
    }
    return NULL;
}


/* ---- storage <-> view adapters (see settings_schema.h) ---------------- */
#include <string.h>

void settings_view_from_store(const settings_t *in, rechord_settings_t *out)
{
    settings_view_defaults(out);
    /* ENUM-typed view fields decode the stored text through the schema's
     * option tables; direct integers map 1:1 except screen_off (minutes ->
     * seconds). */
    for (int si = 0; si < SETTING_KEY_COUNT; si++) {
        const setting_schema_t *sc = &setting_schema[si];
        const char *txt = NULL;
        int iv = 0;
        if (strcmp(sc->name, "play") == 0)          { txt = in->play; }
        else if (strcmp(sc->name, "usb") == 0)      { txt = in->usb; }
        else if (strcmp(sc->name, "resume") == 0)   { txt = in->resume; }
        else if (strcmp(sc->name, "strip") == 0)    { txt = in->strip; }
        else if (strcmp(sc->name, "tags") == 0)     { txt = in->tags; }
        else if (strcmp(sc->name, "cpu") == 0)      { txt = in->cpu; }
        else if (strcmp(sc->name, "brightness") == 0) { iv = in->brightness; }
        else if (strcmp(sc->name, "screen_off") == 0) { iv = in->screen_off_min * 60; }
        else if (strcmp(sc->name, "auto_off") == 0)   { iv = in->auto_off_min; }
        else if (strcmp(sc->name, "volume") == 0)     { iv = in->volume; }
        else if (strcmp(sc->name, "cursor") == 0)     { iv = in->cursor; }
        else if (strcmp(sc->name, "browse") == 0) {
            char *dst = (char *)((char *)out + sc->offset);
            snprintf(dst, RECHORD_BROWSE_PATH_MAX, "%s", in->browse);
            continue;
        }
        if (txt != NULL) {
            int *dst = (int *)((char *)out + sc->offset);
            *dst = 0;
            for (int i = 0; i < sc->u.e.count; i++)
                if (strcmp(sc->u.e.opts[i].text, txt) == 0) { *dst = sc->u.e.opts[i].value; break; }
        } else {
            int *dst = (int *)((char *)out + sc->offset);
            *dst = iv;
        }
    }
}

void settings_view_to_store(const rechord_settings_t *in, settings_t *out)
{
    settings_defaults(out);
    for (int si = 0; si < SETTING_KEY_COUNT; si++) {
        const setting_schema_t *sc = &setting_schema[si];
        const int *src = (const int *)((const char *)in + sc->offset);
        if (strcmp(sc->name, "browse") == 0) {
            snprintf(out->browse, sizeof out->browse, "%s",
                     (const char *)in + sc->offset);
            continue;
        }
        if (sc->kind == SETTING_KIND_ENUM) {
            const char *txt = sc->u.e.count > 0 ? sc->u.e.opts[0].text : "";
            for (int i = 0; i < sc->u.e.count; i++)
                if (sc->u.e.opts[i].value == *src) { txt = sc->u.e.opts[i].text; break; }
            if (strcmp(sc->name, "play") == 0)       snprintf(out->play, sizeof out->play, "%s", txt);
            else if (strcmp(sc->name, "usb") == 0)   snprintf(out->usb, sizeof out->usb, "%s", txt);
            else if (strcmp(sc->name, "resume") == 0)snprintf(out->resume, sizeof out->resume, "%s", txt);
            else if (strcmp(sc->name, "strip") == 0) snprintf(out->strip, sizeof out->strip, "%s", txt);
            else if (strcmp(sc->name, "tags") == 0)  snprintf(out->tags, sizeof out->tags, "%s", txt);
            else if (strcmp(sc->name, "cpu") == 0)   snprintf(out->cpu, sizeof out->cpu, "%s", txt);
        } else if (strcmp(sc->name, "brightness") == 0) out->brightness = *src;
        else if (strcmp(sc->name, "screen_off") == 0)   out->screen_off_min = *src / 60;
        else if (strcmp(sc->name, "auto_off") == 0)     out->auto_off_min = *src;
        else if (strcmp(sc->name, "volume") == 0)       out->volume = *src;
        else if (strcmp(sc->name, "cursor") == 0)       out->cursor = *src;
    }
}
