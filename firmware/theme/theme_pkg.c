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
    F_PAD, F_TITLE_H, F_ITEM_H, F_FACE_H,
    F_NP_TITLE_Y, F_NP_ARTIST_Y, F_NP_STATE_Y, F_NP_INFO_Y,
    F_NP_BAR_Y, F_NP_BAR_H, F_NP_TIME_Y, F_NP_MODE_Y,
    F_NP_VOL_Y, F_NP_VOL_H, F_COUNT
};

#define W THEME_LAYOUT_MAX_W
#define H THEME_LAYOUT_MAX_H

static const layout_key_t k_keys[] = {
    { "pad",          F_PAD,         0, 16  },
    { "title_h",      F_TITLE_H,     0, 32  },
    { "item_h",       F_ITEM_H,      8, 24  },
    { "face_h",       F_FACE_H,      0, 80  },
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
    case F_PAD:         return &l->pad;
    case F_TITLE_H:     return &l->title_h;
    case F_ITEM_H:      return &l->item_h;
    case F_FACE_H:      return &l->face_h;
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
    l->pad = 4;
    l->title_h = 16;
    l->item_h = 12;
    l->face_h = 0;
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

int theme_layout_parse(const char *text, theme_layout_t *l)
{
    theme_layout_t tmp;
    const char *p = text;

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
            int i;

            for (i = 0; i < KEY_COUNT; i++) {
                const layout_key_t *k = &k_keys[i];

                if ((int)strlen(k->key) == klen &&
                    strncmp(p, k->key, (size_t)klen) == 0) {
                    int v = 0;

                    if (parse_int(eq + 1, (int)(eol - (eq + 1)), &v) != 0 ||
                        v < k->min || v > k->max)
                        return -1;            /* reject the whole layout */
                    *field_ptr(&tmp, k->field) = v;
                    break;
                }
            }
        }
        p = (*eol != '\0') ? eol + 1 : eol;
    }
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

void theme_layout_reset(void)
{
    theme_layout_defaults(&s_layout);
    s_ready = 1;
}
