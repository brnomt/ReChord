/* output_mem — see output_mem.h. */
#include <stdio.h>
#include <string.h>

#include "output_mem.h"
#include "fs.h"
#include "rechord_files.h"

static const char k_hex[] = "0123456789ABCDEF";

void om_key_wired(char *key)
{
    strcpy(key, "wired");
}

void om_key_bt(const uint8_t addr[6], char *key)
{
    int i;

    key[0] = 'b';
    key[1] = 't';
    key[2] = ':';
    for (i = 0; i < 6; i++) {
        key[3 + 2 * i] = k_hex[addr[i] >> 4];
        key[4 + 2 * i] = k_hex[addr[i] & 15];
    }
    key[15] = '\0';
}

/* ---- tiny parsers ------------------------------------------------------- */

static const char *p_int(const char *s, int *out)
{
    int neg = 0, v = 0, any = 0;

    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        v = v * 10 + (*s++ - '0');
        any = 1;
    }
    if (!any)
        return 0;
    *out = neg ? -v : v;
    return s;
}

/* "f,q,g[,t] f,q,g[,t] ..." -> curve bands (t = EQ_TYPE_*, BRNOMT-77;
 * absent = positional). Returns the band count or -1. */
static int p_bands(const char *s, eq_curve_t *c)
{
    int n = 0;

    while (*s != '\0' && *s != ';' && *s != '\n') {
        int f, q, g, t = EQ_TYPE_AUTO;

        if (*s == ' ') {
            s++;
            continue;
        }
        if (n >= EQ_BANDS_MAX)
            return -1;
        if ((s = p_int(s, &f)) == 0 || *s++ != ',')
            return -1;
        if ((s = p_int(s, &q)) == 0 || *s++ != ',')
            return -1;
        if ((s = p_int(s, &g)) == 0)
            return -1;
        if (*s == ',' && ((s = p_int(s + 1, &t)) == 0 || t < 0 ||
                          t > EQ_TYPE_MAX))
            return -1;
        c->band[n].freq_hz = (uint16_t)f;
        c->band[n].q_x100 = (uint16_t)q;
        c->band[n].gain_tdb = (int16_t)g;
        c->band[n].type = (uint8_t)t;
        n++;
    }
    return n;
}

/* Does the line starting at `ln` carry `key`? ("o=<key>;") */
static int line_has_key(const char *ln, const char *key)
{
    size_t k = strlen(key);

    return ln[0] == 'o' && ln[1] == '=' && strncmp(ln + 2, key, k) == 0 &&
           ln[2 + k] == ';';
}

static const char *line_end(const char *ln)
{
    while (*ln != '\0' && *ln != '\n')
        ln++;
    return ln;
}

int om_find(const char *text, const char *key, om_state_t *out)
{
    const char *ln = text;

    while (*ln != '\0') {
        const char *end = line_end(ln);

        if (line_has_key(ln, key)) {
            const char *p = ln + 2 + strlen(key) + 1;   /* first field */

            memset(out, 0, sizeof *out);
            while (p < end) {
                if (p[0] == 'v' && p[1] == '=') {
                    if ((p = p_int(p + 2, &out->vol_db)) == 0)
                        return 0;
                    out->has_vol = 1;
                } else if (p[0] == 'e' && p[1] == '=') {
                    if ((p = p_int(p + 2, &out->eq_on)) == 0)
                        return 0;
                    out->has_eq = 1;
                } else if (p[0] == 'n' && p[1] == '=') {
                    size_t n = 0;

                    p += 2;
                    while (p < end && *p != ';' && n < OM_NAME_MAX - 1)
                        out->name[n++] = *p++;
                    out->name[n] = '\0';
                } else if (p[0] == 'b' && p[1] == '=') {
                    int n = p_bands(p + 2, &out->curve);

                    if (n < EQ_BANDS_MIN)
                        return 0;
                    out->curve.count = (uint8_t)n;
                    out->curve.enabled = (uint8_t)(out->eq_on ? 1 : 0);
                    while (p < end && *p != ';')
                        p++;
                } else {
                    while (p < end && *p != ';')
                        p++;
                }
                if (p == 0)
                    return 0;
                if (p < end && *p == ';')
                    p++;
            }
            if (out->has_eq && !eq_curve_valid(&out->curve))
                out->has_eq = 0;               /* bad curve: keep the volume */
            return out->has_vol || out->has_eq;
        }
        ln = (*end == '\n') ? end + 1 : end;
    }
    return 0;
}

static int format_line(char *buf, unsigned cap, const char *key,
                       const om_state_t *st)
{
    int n = snprintf(buf, cap, "o=%s;", key);
    int i;

    if (st->has_vol && n > 0 && (unsigned)n < cap)
        n += snprintf(buf + n, cap - (unsigned)n, "v=%d;", st->vol_db);
    if (st->has_eq && n > 0 && (unsigned)n < cap) {
        char nm[OM_NAME_MAX];
        unsigned j = 0;

        for (i = 0; st->name[i] != '\0' && j < sizeof nm - 1; i++)
            nm[j++] = (st->name[i] == ';' || st->name[i] == '\n')
                          ? ' ' : st->name[i];
        nm[j] = '\0';
        n += snprintf(buf + n, cap - (unsigned)n, "e=%d;n=%s;b=",
                      st->eq_on ? 1 : 0, nm);
        for (i = 0; i < st->curve.count && n > 0 && (unsigned)n < cap; i++) {
            n += snprintf(buf + n, cap - (unsigned)n, "%s%u,%u,%d",
                          i ? " " : "", (unsigned)st->curve.band[i].freq_hz,
                          (unsigned)st->curve.band[i].q_x100,
                          (int)st->curve.band[i].gain_tdb);
            if (st->curve.band[i].type != EQ_TYPE_AUTO && n > 0 &&
                (unsigned)n < cap)
                n += snprintf(buf + n, cap - (unsigned)n, ",%u",
                              (unsigned)st->curve.band[i].type);
        }
    }
    if (n <= 0 || (unsigned)n + 2 > cap)
        return -1;
    buf[n++] = '\n';
    buf[n] = '\0';
    return n;
}

static void drop_range(char *text, char *from, char *to)
{
    memmove(from, to, strlen(to) + 1);
}

int om_upsert(char *text, unsigned cap, const char *key, const om_state_t *st)
{
    char line[256];                      /* 10 typed bands + key/name fit */
    int ln_len = format_line(line, sizeof line, key, st);
    char *p;
    int lines = 0;

    if (ln_len < 0 || (unsigned)ln_len + 1 > cap)
        return -1;

    /* Remove the existing entry for this key. */
    p = text;
    while (*p != '\0') {
        char *end = (char *)line_end(p);
        char *next = (*end == '\n') ? end + 1 : end;

        if (line_has_key(p, key)) {
            drop_range(text, p, next);
            continue;
        }
        p = next;
    }
    /* Evict the oldest lines while the new one would not fit / too many. */
    for (;;) {
        unsigned len = (unsigned)strlen(text);

        lines = 0;
        for (p = text; *p != '\0'; p = (char *)line_end(p) +
                                       (*line_end(p) == '\n' ? 1 : 0))
            lines++;
        if (len + (unsigned)ln_len + 1 <= cap && lines < OM_MAX_OUTPUTS)
            break;
        if (text[0] == '\0')
            break;
        {
            char *end = (char *)line_end(text);

            drop_range(text, text, (*end == '\n') ? end + 1 : end);
        }
    }
    {
        unsigned len = (unsigned)strlen(text);

        if (len + (unsigned)ln_len + 1 > cap)
            return -1;
        memcpy(text + len, line, (size_t)ln_len + 1);
        return (int)(len + (unsigned)ln_len);
    }
}

/* ---- file wrappers ------------------------------------------------------ */

int om_load(const char *key, om_state_t *out)
{
    char text[OM_FILE_MAX];

    if (fs_load_file(RECHORD_OUTPUT, text, sizeof text) <= 0)
        return 0;
    return om_find(text, key, out);
}

int om_save(const char *key, const om_state_t *st)
{
    char text[OM_FILE_MAX];
    int n = fs_load_file(RECHORD_OUTPUT, text, sizeof text);

    if (n < 0)
        text[0] = '\0';                  /* missing/empty: start fresh */
    n = om_upsert(text, sizeof text, key, st);
    if (n < 0)
        return -1;
    return fs_store_file(RECHORD_OUTPUT, text, (uint32_t)n) == FS_OK ? 0 : -1;
}
