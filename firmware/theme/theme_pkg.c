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
