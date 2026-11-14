/*
 * theme_pkg.c — the layout half of a theme package (BRNOMT-69 B/C/D).
 *
 * See theme_pkg.h. Reads are read-only (stdio shim); the parser is pure, so
 * the whole format is host-testable.
 */
#include <stdio.h>
#include <string.h>

#include "theme_pkg.h"

typedef struct
{
    const char *key;
    int         field;      /* offsetof-free: index into the int array */
    int         min, max;
} layout_key_t;

/* Order matters: the parser writes l->field[index] through this table. */
enum {
    F_FORMAT, F_PAD, F_TITLE_H, F_ITEM_H, F_FACE_H,
    F_HOME_BAR_Y, F_HOME_BAR_H,
    F_NP_TITLE_Y, F_NP_ARTIST_Y, F_NP_STATE_Y, F_NP_INFO_Y,
    F_NP_BAR_Y, F_NP_BAR_H, F_NP_TIME_Y, F_NP_MODE_Y,
    F_NP_VOL_Y, F_NP_VOL_H, F_COUNT
};

#define W THEME_LAYOUT_MAX_W
#define H THEME_LAYOUT_MAX_H

static const layout_key_t k_keys[] = {
    { "format",       F_FORMAT,      1, 1   },
    { "pad",          F_PAD,         0, 16  },
    { "title_h",      F_TITLE_H,     0, 32  },
    { "item_h",       F_ITEM_H,      8, 24  },
    { "face_h",       F_FACE_H,      0, H   },
    { "home_bar_y",   F_HOME_BAR_Y,  0, H   },
    { "home_bar_h",   F_HOME_BAR_H,  8, 32  },
    { "np_title_y",   F_NP_TITLE_Y,  0, H   },
    { "np_artist_y",  F_NP_ARTIST_Y, 0, H   },
    { "np_state_y",   F_NP_STATE_Y,  0, H   },
    { "np_info_y",    F_NP_INFO_Y,   0, H   },
    { "np_bar_y",     F_NP_BAR_Y,    0, H   },
    { "np_bar_h",     F_NP_BAR_H,    1, 8   },
    { "np_time_y",    F_NP_TIME_Y,   0, H   },
    { "np_mode_y",    F_NP_MODE_Y,   0, H   },
    { "np_vol_y",     F_NP_VOL_Y,    0, H   },
    { "np_vol_h",     F_NP_VOL_H,    1, 8   },
};
#define KEY_COUNT ((int)(sizeof k_keys / sizeof k_keys[0]))

static theme_layout_t s_layout;
static int s_ready;

/* Map the struct fields to the enum order without offsetof tricks. */
static int *field_ptr(theme_layout_t *l, int f)
{
    switch (f) {
    case F_FORMAT:      return &l->format;
    case F_PAD:         return &l->pad;
    case F_TITLE_H:     return &l->title_h;
    case F_ITEM_H:      return &l->item_h;
    case F_FACE_H:      return &l->face_h;
    case F_HOME_BAR_Y:  return &l->home_bar_y;
    case F_HOME_BAR_H:  return &l->home_bar_h;
    case F_NP_TITLE_Y:  return &l->np_title_y;
    case F_NP_ARTIST_Y: return &l->np_artist_y;
    case F_NP_STATE_Y:  return &l->np_state_y;
    case F_NP_INFO_Y:   return &l->np_info_y;
    case F_NP_BAR_Y:    return &l->np_bar_y;
    case F_NP_BAR_H:    return &l->np_bar_h;
    case F_NP_TIME_Y:   return &l->np_time_y;
    case F_NP_MODE_Y:   return &l->np_mode_y;
    case F_NP_VOL_Y:    return &l->np_vol_y;
    case F_NP_VOL_H:    return &l->np_vol_h;
    default:            return 0;
    }
}

void theme_layout_defaults(theme_layout_t *l)
{
    if (l == 0)
        return;
    memset(l, 0, sizeof *l);
    l->format = 1;
    l->home = THEME_HOME_MENU;
    l->home_nav = THEME_NAV_VERTICAL;
    l->input = THEME_INPUT_ENGINE;
    l->pad = 4;
    l->title_h = 16;
    l->item_h = 12;
    l->face_h = 0;
    l->home_bar_y = 152;
    l->home_bar_h = 14;
    l->asset_count = 0;
    l->np_title_y = 24;
    l->np_artist_y = 36;
    l->np_state_y = 48;
    l->np_info_y = 60;
    l->np_bar_y = 74;
    l->np_bar_h = 2;
    l->np_time_y = 84;
    l->np_mode_y = 148;
    l->np_vol_y = 162;
    l->np_vol_h = 2;
}

static int parse_int(const char *s, int len, int *out)
{
    int i = 0;
    long v = 0;
    int neg = 0;

    while (i < len && (s[i] == ' ' || s[i] == '\t'))
        i++;
    if (i < len && (s[i] == '-' || s[i] == '+')) {
        neg = (s[i] == '-');
        i++;
    }
    if (i >= len)
        return -1;
    for (; i < len; i++) {
        if (s[i] < '0' || s[i] > '9')
            return -1;
        v = v * 10 + (s[i] - '0');
        if (v > 100000)
            return -1;
    }
    *out = (int)(neg ? -v : v);
    return 0;
}

/* Trimmed, case-insensitive one-word match. */
static int word_is(const char *s, int len, const char *word)
{
    int i = 0;
    int end;
    int n;

    while (i < len && (s[i] == ' ' || s[i] == '\t'))
        i++;
    end = i;
    while (end < len && s[end] != ' ' && s[end] != '\t')
        end++;
    n = end - i;
    while (end < len && (s[end] == ' ' || s[end] == '\t'))
        end++;
    if (end != len || n != (int)strlen(word))
        return 0;
    for (int k = 0; k < n; k++) {
        char a = s[i + k];
        char b = word[k];

        if (a >= 'A' && a <= 'Z')
            a = (char)(a - ('A' - 'a'));
        if (b >= 'A' && b <= 'Z')
            b = (char)(b - ('A' - 'a'));
        if (a != b)
            return 0;
    }
    return 1;
}

static int key_is(const char *k, int klen, const char *word)
{
    return (int)strlen(word) == klen && strncmp(k, word, (size_t)klen) == 0;
}

/* Trimmed integer span: surrounding spaces/tabs are ignored. */
static int span_int(const char *s, int len, int *out)
{
    int start = 0;
    int end = len;

    while (start < end && (s[start] == ' ' || s[start] == '\t'))
        start++;
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t'))
        end--;
    return parse_int(s + start, end - start, out);
}

/* Relative asset path: no absolute paths, no '\\', no "..". */
static int valid_asset_path(const char *p, int len)
{
    int i;

    if (p == 0 || len <= 0 || len >= THEME_ASSET_PATH_MAX || p[0] == '/')
        return 0;
    for (i = 0; i < len; i++) {
        if (p[i] == '\\')
            return 0;
        if (p[i] == '.' && i + 1 < len && p[i + 1] == '.')
            return 0;
    }
    return 1;
}

/* Trimmed relative path value -> fixed buffer. */
static int parse_path_value(const char *v, int vlen, char *out)
{
    int start = 0;
    int end = vlen;

    while (start < end && (v[start] == ' ' || v[start] == '\t'))
        start++;
    while (end > start && (v[end - 1] == ' ' || v[end - 1] == '\t'))
        end--;
    if (!valid_asset_path(v + start, end - start))
        return -1;
    memcpy(out, v + start, (size_t)(end - start));
    out[end - start] = '\0';
    return 0;
}

static int parse_asset_key(const char *key, int klen, const char *v, int vlen,
                           theme_layout_t *l)
{
    const char *name = key + 6;          /* after "asset_" */
    int nlen = klen - 6;
    int c1 = -1, c2 = -1;
    int start, end, w, h, i;

    if (nlen <= 0 || nlen >= THEME_ASSET_NAME_MAX ||
        l->asset_count >= THEME_ASSET_MAX)
        return -1;
    for (i = 0; i < vlen; i++) {
        if (v[i] == ',') {
            if (c1 < 0)
                c1 = i;
            else if (c2 < 0)
                c2 = i;
            else
                return -1;
        }
    }
    if (c1 < 0 || c2 < 0)
        return -1;

    start = 0;
    end = c1;
    while (start < end && (v[start] == ' ' || v[start] == '\t'))
        start++;
    while (end > start && (v[end - 1] == ' ' || v[end - 1] == '\t'))
        end--;
    if (!valid_asset_path(v + start, end - start))
        return -1;
    if (span_int(v + c1 + 1, c2 - c1 - 1, &w) != 0 ||
        span_int(v + c2 + 1, vlen - c2 - 1, &h) != 0)
        return -1;
    if (w < 1 || w > 64 || h < 1 || h > 64 || (w + 7) / 8 > 8)
        return -1;

    for (i = 0; i < l->asset_count; i++) {
        if ((int)strlen(l->assets[i].name) == nlen &&
            strncmp(l->assets[i].name, name, (size_t)nlen) == 0)
            return -1;                   /* duplicate asset name */
    }

    memcpy(l->assets[l->asset_count].name, name, (size_t)nlen);
    l->assets[l->asset_count].name[nlen] = '\0';
    memcpy(l->assets[l->asset_count].path, v + start,
           (size_t)(end - start));
    l->assets[l->asset_count].path[end - start] = '\0';
    l->assets[l->asset_count].w = w;
    l->assets[l->asset_count].h = h;
    l->asset_count++;
    return 0;
}

/* ---- per-theme options -------------------------------------------------- */

static int s_opt_rejected;

int theme_opt_rejected(void)
{
    return s_opt_rejected;
}

/* 1 when `id` contains a byte the theme_opts text format uses as a
 * separator (the freestanding libc has no strpbrk). */
static int has_sep(const char *id)
{
    for (; *id != '\0'; id++)
        if (*id == ':' || *id == ';')
            return 1;
    return 0;
}

/* Copy a trimmed span into a fixed buffer; -1 when it does not fit or is
 * empty. */
static int span_copy(char *dst, int cap, const char *s, int len)
{
    int start = 0;
    int end = len;

    while (start < end && (s[start] == ' ' || s[start] == '\t'))
        start++;
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t'))
        end--;
    if (end <= start || end - start > cap - 1)
        return -1;
    memcpy(dst, s + start, (size_t)(end - start));
    dst[end - start] = '\0';
    return 0;
}

/* `opt_<id>=<Label>,<choice>|<choice>[|...]`
 *
 * Limits: id is a lowercase identifier (a-z0-9_, <= 15 chars) because the
 * skin reads it as `opt_<id>`; label <= 23 chars; 2..4 choices of <= 15
 * chars; at most THEME_OPT_MAX options. Any violation drops the line (the
 * package stays usable) and bumps the rejected counter. */
static int parse_opt_key(const char *key, int klen, const char *v, int vlen,
                         theme_layout_t *l)
{
    const char *id = key + 4;            /* after "opt_" */
    int idlen = klen - 4;
    const char *comma;
    int i;

    if (idlen <= 0 || idlen >= THEME_OPT_NAME_MAX)
        return -1;
    if (id[0] < 'a' || id[0] > 'z')
        return -1;                       /* must lex as an expression ident */
    for (i = 0; i < idlen; i++) {
        char c = id[i];

        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'))
            return -1;
    }
    if (l->opt_count >= THEME_OPT_MAX)
        return -1;
    for (i = 0; i < l->opt_count; i++) {
        if ((int)strlen(l->opts[i].id) == idlen &&
            strncmp(l->opts[i].id, id, (size_t)idlen) == 0)
            return -1;                   /* duplicate id */
    }
    for (comma = v; comma < v + vlen && *comma != ','; comma++)
        ;
    if (comma >= v + vlen)
        return -1;                       /* no label separator */

    {
        theme_opt_decl_t opt;
        const char *p = comma + 1;
        const char *end = v + vlen;

        memset(&opt, 0, sizeof opt);
        memcpy(opt.id, id, (size_t)idlen);
        opt.id[idlen] = '\0';
        if (span_copy(opt.label, (int)sizeof opt.label, v,
                      (int)(comma - v)) != 0)
            return -1;
        for (;;) {
            const char *bar = p;

            while (bar < end && *bar != '|')
                bar++;
            if (opt.count >= THEME_OPT_CHOICES_MAX)
                return -1;
            if (span_copy(opt.choice[opt.count],
                          (int)sizeof opt.choice[opt.count], p,
                          (int)(bar - p)) != 0)
                return -1;
            opt.count++;
            if (bar >= end)
                break;
            p = bar + 1;
        }
        if (opt.count < 2)
            return -1;                   /* a one-choice option is dead weight */
        opt.value = 0;                   /* persistence overrides it later */
        l->opts[l->opt_count++] = opt;
    }
    return 0;
}

int theme_layout_parse(const char *text, theme_layout_t *l)
{
    theme_layout_t tmp;
    const char *p = text;
    int home_seen = 0;
    int nav_seen = 0;
    int skin_seen = 0;

    if (l == 0)
        return -1;
    /* All-or-nothing: parse into a copy so a rejected value leaves the caller's
     * layout exactly as it was (a malformed package must never half-apply). */
    tmp = *l;
    /* Options are declared only by this text: a previous parse's declarations
     * (and their selected values) must not leak into it. */
    tmp.opt_count = 0;
    s_opt_rejected = 0;

    while (p != 0 && *p != '\0') {
        const char *eol = p;
        const char *eq;

        while (*eol != '\0' && *eol != '\n' && *eol != '\r')
            eol++;
        if (*p == '#' || *p == ';' || *p == '[') {   /* comment / section */
            p = (*eol != '\0') ? eol + 1 : eol;
            continue;
        }
        for (eq = p; eq < eol && *eq != '='; eq++)
            ;
        if (eq < eol) {
            int klen = (int)(eq - p);
            const char *v = eq + 1;
            int vlen = (int)(eol - v);
            int i;

            /* String-valued keys that select engine behavior. Unknown values
             * reject the whole layout: a theme must never silently fall back
             * to a renderer it did not ask for. */
            if (key_is(p, klen, "home")) {
                if (word_is(v, vlen, "menu"))
                    tmp.home = THEME_HOME_MENU;
                else if (word_is(v, vlen, "skin") ||
                         word_is(v, vlen, "companion"))
                    tmp.home = THEME_HOME_SKIN;
                else
                    return -1;
                home_seen = 1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            if (key_is(p, klen, "home_nav")) {
                if (word_is(v, vlen, "vertical"))
                    tmp.home_nav = THEME_NAV_VERTICAL;
                else if (word_is(v, vlen, "horizontal"))
                    tmp.home_nav = THEME_NAV_HORIZONTAL;
                else
                    return -1;
                nav_seen = 1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            if (key_is(p, klen, "input")) {
                if (word_is(v, vlen, "engine"))
                    tmp.input = THEME_INPUT_ENGINE;
                else if (word_is(v, vlen, "theme"))
                    tmp.input = THEME_INPUT_THEME;
                else
                    return -1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            if (key_is(p, klen, "skin") ||
                key_is(p, klen, "skin_main")) {
                if (parse_path_value(v, vlen, tmp.skin) != 0)
                    return -1;
                skin_seen = 1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            if (key_is(p, klen, "skin_now_playing")) {
                if (parse_path_value(v, vlen, tmp.skin_now_playing) != 0)
                    return -1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            if (key_is(p, klen, "skin_library_cat")) {
                if (parse_path_value(v, vlen, tmp.skin_library_cat) != 0)
                    return -1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            if (key_is(p, klen, "skin_library")) {
                if (parse_path_value(v, vlen, tmp.skin_library) != 0)
                    return -1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            if (key_is(p, klen, "skin_browser")) {
                if (parse_path_value(v, vlen, tmp.skin_browser) != 0)
                    return -1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            if (key_is(p, klen, "skin_settings")) {
                if (parse_path_value(v, vlen, tmp.skin_settings) != 0)
                    return -1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            if (klen > 6 && strncmp(p, "asset_", 6) == 0) {
                if (parse_asset_key(p, klen, v, vlen, &tmp) != 0)
                    return -1;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }
            /* Malformed options are dropped, not fatal (see theme_pkg.h). */
            if (klen > 4 && strncmp(p, "opt_", 4) == 0) {
                if (parse_opt_key(p, klen, v, vlen, &tmp) != 0)
                    s_opt_rejected++;
                p = (*eol != '\0') ? eol + 1 : eol;
                continue;
            }

            for (i = 0; i < KEY_COUNT; i++) {
                const layout_key_t *k = &k_keys[i];

                if ((int)strlen(k->key) == klen &&
                    strncmp(p, k->key, (size_t)klen) == 0) {
                    int n = 0;

                    if (parse_int(v, vlen, &n) != 0 ||
                        n < k->min || n > k->max)
                        return -1;            /* reject the whole layout */
                    *field_ptr(&tmp, k->field) = n;
                    break;
                }
            }
        }
        p = (*eol != '\0') ? eol + 1 : eol;
    }

    /* Backward-compatible default: a non-zero face band only selects the
     * skin renderer when the package actually ships a skin file. Old
     * companion manifests without one fall back to the classic menu. */
    if (!home_seen && tmp.face_h > 0 && skin_seen)
        tmp.home = THEME_HOME_SKIN;
    if (!nav_seen && tmp.home == THEME_HOME_SKIN)
        tmp.home_nav = THEME_NAV_HORIZONTAL;

    /* The generic skin renderer needs its declarative file. */
    if (tmp.home == THEME_HOME_SKIN && tmp.skin[0] == '\0')
        return -1;

    *l = tmp;                             /* commit only on success */
    return 0;
}

int theme_layout_load(const char *path)
{
    theme_layout_t scratch;
    char buf[2048];
    char line[128];
    size_t used = 0;
    FILE *f;

    theme_layout_defaults(&scratch);

    f = (path != 0) ? fopen(path, "r") : 0;
    if (f == 0) {
        s_layout = scratch;
        s_ready = 1;
        return -1;
    }
    /* fgets-based (the freestanding stdio shim implements the same calls the
     * theme palette loader uses); bounded by the buffer. */
    buf[0] = '\0';
    while (fgets(line, (int)sizeof(line), f) != 0) {
        size_t n = strlen(line);

        if (used + n + 1u > sizeof(buf))
            break;
        memcpy(buf + used, line, n);
        used += n;
    }
    fclose(f);
    buf[used] = '\0';

    if (theme_layout_parse(buf, &scratch) != 0) {
        s_layout = scratch;
        s_ready = 1;
        return -1;                        /* keep the default layout */
    }
    s_layout = scratch;
    s_ready = 1;
    return 0;
}

const theme_layout_t *theme_layout(void)
{
    if (!s_ready) {
        theme_layout_defaults(&s_layout);
        s_ready = 1;
    }
    return &s_layout;
}

int theme_input_owns_keys(void)
{
    return theme_layout()->input == THEME_INPUT_THEME;
}

void theme_layout_set(const theme_layout_t *l)
{
    if (l == 0)
        return;
    s_layout = *l;
    s_ready = 1;
}

void theme_layout_reset(void)
{
    theme_layout_defaults(&s_layout);
    s_ready = 1;
}

/* ---- per-theme options -------------------------------------------------- */

int theme_opt_value(const char *id)
{
    int i;

    if (id == 0)
        return -1;
    for (i = 0; i < s_layout.opt_count; i++)
        if (strcmp(s_layout.opts[i].id, id) == 0)
            return s_layout.opts[i].value;
    return -1;
}

int theme_opt_set(const char *id, int value)
{
    int i;

    if (id == 0)
        return -1;
    for (i = 0; i < s_layout.opt_count; i++) {
        theme_opt_decl_t *o = &s_layout.opts[i];

        if (strcmp(o->id, id) != 0)
            continue;
        if (value < 0)
            value = 0;
        if (value >= o->count)
            value = o->count - 1;
        o->value = value;
        return value;
    }
    return -1;
}

/* ---- persisted selections ------------------------------------------------
 * ONE cfg value holds every theme's choices, so a theme switch never has to
 * touch a second file:
 *
 *     theme_opts=glifo:1000;oscilloscope:0100;
 *
 * `<id>:<d><d><d><d>;` — one digit per declared option (0..3), padded with
 * zeros so the format stays fixed-width and a theme that adds options later
 * still parses. Unknown ids and unknown digits are ignored, never fatal. A
 * directory name can never contain ':' or ';' (theme_pkg's id grammar), so
 * the separators are unambiguous.
 */

static int opts_id_end(const char *p)
{
    int n = 0;

    while (p[n] != '\0' && p[n] != ':' && p[n] != ';')
        n++;
    return n;
}

int theme_opts_decode(const char *text, const char *id, int *out, int max)
{
    int idlen, n = 0;

    if (text == 0 || id == 0 || out == 0 || max <= 0)
        return -1;
    idlen = (int)strlen(id);
    if (idlen == 0 || has_sep(id))
        return -1;                       /* cannot be encoded unambiguously */
    while (*text != '\0') {
        int len = opts_id_end(text);
        const char *body = text + len;

        if (len == idlen && strncmp(text, id, (size_t)idlen) == 0 &&
            *body == ':') {
            int i = 0;

            body++;
            while (i < max && body[i] >= '0' && body[i] <= '9') {
                out[i] = body[i] - '0';
                i++;
            }
            n = i;
            break;
        }
        while (*text != '\0' && *text != ';')
            text++;
        if (*text == ';')
            text++;
    }
    {
        int got = n;

        while (n < max)
            out[n++] = 0;
        return got;                      /* digits found (rest are zeros) */
    }
}

int theme_opts_encode_entry(char *text, int cap, const char *id,
                           const int *vals, int n)
{
    int idlen, len = 0, i;

    if (text == 0 || id == 0 || cap <= 0 || n <= 0)
        return -1;
    idlen = (int)strlen(id);
    if (idlen == 0 || has_sep(id))
        return -1;                       /* cannot be encoded unambiguously */
    /* Copy every entry except this theme's, bounded by `cap`. */
    {
        const char *p = text;

        while (*p != '\0') {
            const char *start = p;

            while (*p != '\0' && *p != ';')
                p++;
            if (*p == ';')
                p++;
            if (opts_id_end(start) == idlen &&
                strncmp(start, id, (size_t)idlen) == 0)
                continue;                /* drop the stale entry */
            if (len + (int)(p - start) >= cap)
                return -1;
            memcpy(text + len, start, (size_t)(p - start));
            len += (int)(p - start);
        }
    }
    if (len + idlen + 1 + n + 2 > cap)
        return -1;
    memcpy(text + len, id, (size_t)idlen);
    len += idlen;
    text[len++] = ':';
    for (i = 0; i < n; i++) {
        int v = vals[i];

        if (v < 0)
            v = 0;
        if (v > 9)
            v = 9;
        text[len++] = (char)('0' + v);
    }
    text[len++] = ';';
    text[len] = '\0';
    return len;
}

void theme_opts_apply(const int *vals, int n)
{
    int i;

    if (vals == 0)
        return;
    for (i = 0; i < s_layout.opt_count && i < n; i++) {
        int v = vals[i];

        if (v < 0)
            v = 0;
        if (v >= s_layout.opts[i].count)
            v = s_layout.opts[i].count - 1;
        s_layout.opts[i].value = v;
    }
}

void theme_opts_values(int *out, int max)
{
    int i;

    if (out == 0)
        return;
    for (i = 0; i < max; i++)
        out[i] = i < s_layout.opt_count ? s_layout.opts[i].value : 0;
}
