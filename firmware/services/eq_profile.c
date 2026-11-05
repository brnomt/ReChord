/*
 * eq_profile.c — EQ profile storage/parsing for BRNOMT-76.
 *
 * Format: the same key=value dialect as theme.cfg / rechord.cfg. Parsing is
 * strict and atomic: a malformed profile is rejected whole and the active
 * curve is not touched. Only the AP runs this code — the filter engine is
 * Core B's (firmware/dsp/eq_bank.c); the curve leaves through `s_publish`.
 */
#include <stdint.h>
#include <string.h>

#include "eq_profile.h"
#include "fs.h"
#include "rechord_files.h"

#define EQ_BUILTIN_COUNT EQ_PROFILE_BUILTINS
#define EQ_TEXT_MAX      512
#define EQ_IMPORT_TEXT_MAX 1536   /* an AutoEQ ParametricEQ.txt is ~0.5 KB */

/* Default band layout. Band 0 is a LOW SHELF and the last a HIGH SHELF (see
 * eq_bank.h), and an RBJ shelf only reaches full gain about an octave away
 * from its corner. The old ISO layout put the corners at 31 Hz and 16 kHz, so
 * "+4 dB at 31 Hz" was inaudible; these corners sit where headphones and small
 * drivers actually respond. The middle bands are one-octave peaks. */
static const uint16_t s_iso_freq[EQ_BANDS_MAX] = {
    80, 160, 320, 640, 1250, 2500, 4000, 6300, 10000, 14000
};

/* Shelves use slope 0.70; the peaking bands use Q 1.40 (one octave wide), so
 * neighbouring faders add up smoothly instead of two octaves of mush. */
static unsigned default_q(int band, int count)
{
    return (band == 0 || band == count - 1) ? 70u : 140u;
}

/* Built-in curves in tenths of dB, for the layout above. Project data, tuned by
 * measuring the real filter chain (tools: sine sweep through eq_bank) so each
 * shape comes out smooth and loudness-neutral to within the preamp. */
static const int8_t s_builtin_gain[EQ_BUILTIN_COUNT][EQ_BANDS_MAX] = {
    /* Flat      */ {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
    /* Bass Boost*/ {  90,  30,  10,   0,   0,   0,   0,   0,   0,   0 },
    /* Rock      */ {  60,  20,   0, -20, -30, -10,  20,  40,  40,  30 },
    /* Pop       */ { -20,   0,  20,  30,  30,  10, -10, -10, -20, -20 },
    /* Jazz      */ {  30,  10,  10,  20, -10, -10,   0,  10,  20,  20 },
    /* EDM       */ {  90,  30,   0,   0, -10, -10,   0,  20,  40,  40 },
    /* Hip-Hop   */ {  80,  40,  10,   0, -10,   0,  10,  10,  20,  20 },
    /* Vocal     */ { -30, -10,   0,  10,  30,  40,  30,  10, -10, -20 },
    /* Treble    */ {   0,   0,   0,   0,   0,  10,  30,  50,  60,  60 },
    /* Loudness  */ {  60,  20,   0, -10, -10, -10,   0,  10,  30,  40 }
};

static const char s_builtin_name[EQ_BUILTIN_COUNT][12] = {
    "Flat", "Bass Boost", "Rock", "Pop", "Jazz", "EDM", "Hip-Hop", "Vocal",
    "Treble", "Loudness"
};

/* ---- AP state (kept small: the AP .bss ceiling is the tight budget) ---- */
static eq_curve_t s_active;
static char       s_name[EQ_PROFILE_NAME_MAX];
static char       s_custom_name[EQ_PROFILE_SLOTS][EQ_PROFILE_NAME_MAX];
static uint8_t    s_custom_mask;          /* bit N-1 = customN.cfg present */
/* BRNOMT-77: AutoEQ / Equalizer APO .txt files dropped into the EQ folder,
 * listed after the custom slots (read-only rows). */
static char       s_import_name[EQ_PROFILE_IMPORTS][EQ_PROFILE_NAME_MAX];
static uint8_t    s_import_n;
static uint8_t    s_enabled;
static uint8_t    s_dirty;
static uint8_t    s_booted;
static eq_publish_fn s_publish;
static void (*s_observer)(void);
static uint8_t s_quiet;            /* restore in progress: do not notify */

/* ---- small text helpers (same shape as settings.c) ---- */

static void copy_str(char *dst, unsigned cap, const char *src)
{
    unsigned n;

    if (cap == 0)
        return;
    n = (unsigned)strlen(src);
    if (n >= cap)
        n = cap - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static char *trim(char *s)
{
    char *end;

    while (*s == ' ' || *s == '\t' || *s == '\r')
        s++;
    end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r'))
        *--end = '\0';
    return s;
}

static int parse_int(const char *s, int *out)
{
    long v = 0;
    int neg = 0, any = 0;

    if (s == 0 || out == 0)
        return -1;
    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        if (v > (0x7FFFFFFFL - 9) / 10)
            return -1;
        v = v * 10 + (*s - '0');
        s++;
        any = 1;
    }
    if (!any || *s != '\0')
        return -1;
    *out = neg ? (int)-v : (int)v;
    return 0;
}

static unsigned long abs_int(int v)
{
    return (v < 0) ? (unsigned long)(-(long)(v + 1)) + 1UL : (unsigned long)v;
}

static int emit(char *buf, unsigned long cap, unsigned long *pos,
                const char *text)
{
    unsigned long n = (unsigned long)strlen(text);

    if (*pos + n + 1 > cap)
        return -1;
    memcpy(buf + *pos, text, n);
    *pos += n;
    buf[*pos] = '\0';
    return 0;
}

static int emit_int(char *buf, unsigned long cap, unsigned long *pos, int v)
{
    char digits[16];
    unsigned long u = abs_int(v);
    int i = (int)sizeof(digits) - 1;

    digits[i] = '\0';
    do {
        digits[--i] = (char)('0' + (u % 10UL));
        u /= 10UL;
    } while (u != 0 && i > 0);
    if (v < 0)
        digits[--i] = '-';
    return emit(buf, cap, pos, &digits[i]);
}

/* "bandN" -> N, or -1. */
static int band_index(const char *key)
{
    int idx;

    if (strncmp(key, "band", 4) != 0)
        return -1;
    key += 4;
    if (*key < '0' || *key > '9')
        return -1;
    idx = *key++ - '0';
    if (*key >= '0' && *key <= '9')
        idx = idx * 10 + (*key++ - '0');
    return (*key == '\0') ? idx : -1;
}

/* "freq,Q*100,gain[,type]" -> band; ranges are checked later by
 * eq_curve_valid. The type (BRNOMT-77) is optional: files without it keep the
 * positional layout. */
static int parse_triplet(char *v, eq_band_t *b)
{
    char *c1 = 0, *c2 = 0, *c3 = 0;
    char *p;
    int freq, q, gain, type = EQ_TYPE_AUTO;

    for (p = v; *p != '\0'; p++) {
        if (*p == ',') {
            if (c1 == 0) {
                c1 = p;
            } else if (c2 == 0) {
                c2 = p;
            } else if (c3 == 0) {
                c3 = p;
            } else {
                return -1;
            }
        }
    }
    if (c1 == 0 || c2 == 0)
        return -1;

    *c1 = '\0';
    *c2 = '\0';
    if (c3 != 0) {
        *c3 = '\0';
        if (parse_int(trim(c3 + 1), &type) != 0 || type < 0 ||
            type > EQ_TYPE_MAX)
            return -1;
    }
    if (parse_int(trim(v), &freq) != 0 ||
        parse_int(trim(c1 + 1), &q) != 0 ||
        parse_int(trim(c2 + 1), &gain) != 0)
        return -1;
    if (freq <= 0 || freq > 0xFFFF || q < 0 || q > 0xFFFF ||
        gain < -32768 || gain > 32767)
        return -1;

    b->freq_hz = (uint16_t)freq;
    b->q_x100 = (uint16_t)q;
    b->gain_tdb = (int16_t)gain;
    b->type = (uint8_t)type;
    return 0;
}

/* Locate `name=` without validating the curve (used to label a slot). */
static void find_name(const char *text, int len, char *out, unsigned cap)
{
    int pos = 0;

    out[0] = '\0';
    while (pos < len && text[pos] != '\0') {
        char line[96];
        int n = 0;

        while (pos < len && text[pos] != '\n' && text[pos] != '\0' &&
               n < (int)sizeof(line) - 1)
            line[n++] = text[pos++];
        if (pos < len && text[pos] == '\n')
            pos++;
        line[n] = '\0';
        {
            char *p = trim(line);
            char *eq;

            if (*p == '\0' || *p == '#' || *p == ';' || *p == '[')
                continue;
            for (eq = p; *eq != '\0' && *eq != '='; eq++)
                ;
            if (*eq != '=')
                continue;
            *eq = '\0';
            if (strcmp(trim(p), "name") == 0)
                copy_str(out, cap, trim(eq + 1));
        }
    }
}

int eq_profile_parse(const char *text, int len, eq_curve_t *out,
                     char *name, unsigned name_cap)
{
    eq_curve_t c;
    uint16_t seen = 0;
    int maxidx = -1, pos = 0, i;

    if (text == 0 || out == 0 || len <= 0)
        return -1;
    if (name != 0 && name_cap > 0)
        name[0] = '\0';

    c.count = EQ_BANDS_MAX;
    c.enabled = 1;
    for (i = 0; i < EQ_BANDS_MAX; i++) {
        c.band[i].freq_hz = s_iso_freq[i];
        c.band[i].q_x100 = (uint16_t)default_q(i, EQ_BANDS_MAX);
        c.band[i].gain_tdb = 0;
        c.band[i].type = EQ_TYPE_AUTO;
    }

    while (pos < len && text[pos] != '\0') {
        char line[96];
        int n = 0;
        char *p, *eq, *key, *value;
        int idx;

        while (pos < len && text[pos] != '\n' && text[pos] != '\0' &&
               n < (int)sizeof(line) - 1)
            line[n++] = text[pos++];
        if (n == (int)sizeof(line) - 1 && pos < len &&
            text[pos] != '\n' && text[pos] != '\0')
            return -1;                   /* over-long line: reject the file */
        if (pos < len && text[pos] == '\n')
            pos++;
        line[n] = '\0';

        p = trim(line);
        if (*p == '\0' || *p == '#' || *p == ';' || *p == '[')
            continue;

        for (eq = p; *eq != '\0' && *eq != '='; eq++)
            ;
        if (*eq != '=')
            continue;                    /* unknown line: ignored */
        *eq = '\0';
        key = trim(p);
        value = trim(eq + 1);

        if (strcmp(key, "format") == 0) {
            int fmt;

            if (parse_int(value, &fmt) != 0 || fmt != 1)
                return -1;
            continue;
        }
        if (strcmp(key, "enabled") == 0) {
            int on;

            if (parse_int(value, &on) != 0 || (on != 0 && on != 1))
                return -1;
            c.enabled = (uint8_t)on;
            continue;
        }
        if (strcmp(key, "name") == 0) {
            if (name != 0 && name_cap > 0)
                copy_str(name, name_cap, value);
            continue;
        }

        idx = band_index(key);
        if (idx < 0) {
            if (strncmp(key, "band", 4) == 0)
                return -1;               /* malformed bandN key */
            continue;                    /* unknown key: ignored */
        }
        if (idx >= EQ_BANDS_MAX)
            return -1;
        if (parse_triplet(value, &c.band[idx]) != 0)
            return -1;
        seen = (uint16_t)(seen | (1u << idx));
        if (idx > maxidx)
            maxidx = idx;
    }

    if (maxidx < 0)
        return -1;
    for (i = 0; i <= maxidx; i++)
        if ((seen & (1u << i)) == 0)
            return -1;                   /* gaps are rejected whole */
    c.count = (uint8_t)(maxidx + 1);

    if (!eq_curve_valid(&c))
        return -1;

    *out = c;
    return 0;
}

/* ---- AutoEQ / Equalizer APO import (BRNOMT-77) ----------------------------
 *
 *     Preamp: -6.1 dB
 *     Filter 1: ON LSC Fc 105 Hz Gain 6.4 dB Q 0.70
 *     Filter 2: ON PK Fc 8800 Hz Gain 5.1 dB Q 1.42
 *
 * Types: PK/PEQ (peak), LSC/LS (low shelf), HSC/HS (high shelf), all with a
 * true Q. OFF filters are skipped, other types reject the file. The Preamp
 * line is ignored: the engine sets its own headroom from the curve and has a
 * soft knee. Filters are sorted by frequency (a biquad cascade commutes), gains
 * are clamped to +/-12 dB and Q to 0.10..10.0, and a profile with fewer than 5
 * filters is padded with flat peaks. */

/* "-6.1" -> -61 with `digits` = 1 (rounded half away from zero). */
static int parse_fixed(const char *s, int digits, int *out)
{
    long v = 0;
    int neg = 0, any = 0, frac = 0, round = 0;

    if (*s == '-' || *s == '+')
        neg = (*s++ == '-');
    while (*s >= '0' && *s <= '9') {
        if (v > 100000L)
            return -1;
        v = v * 10 + (*s++ - '0');
        any = 1;
    }
    if (*s == '.') {
        s++;
        while (*s >= '0' && *s <= '9') {
            if (frac < digits) {
                v = v * 10 + (*s - '0');
                frac++;
            } else if (frac == digits) {
                round = (*s >= '5');
                frac++;
            }
            s++;
            any = 1;
        }
    }
    if (!any || *s != '\0')
        return -1;
    for (; frac < digits; frac++)
        v *= 10;
    v += round;
    *out = neg ? (int)-v : (int)v;
    return 0;
}

static int clamp_int(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

/* One "Filter N: ..." line. Returns 1 = band, 0 = skipped (OFF), -1 = bad. */
static int autoeq_filter(char *line, eq_band_t *b)
{
    char *tok[16];
    int n = 0, i, type, have_f = 0, have_g = 0, have_q = 0;
    int f = 0, g = 0, q = 70;
    char *p = line;

    while (*p != '\0' && n < 16) {
        while (*p == ' ' || *p == '\t')
            *p++ = '\0';
        if (*p == '\0')
            break;
        tok[n++] = p;
        while (*p != '\0' && *p != ' ' && *p != '\t')
            p++;
    }
    /* tok: Filter N: ON TYPE Fc f Hz Gain g dB Q q */
    if (n < 4)
        return -1;
    if (strcmp(tok[2], "OFF") == 0)
        return 0;
    if (strcmp(tok[2], "ON") != 0)
        return -1;
    if (strcmp(tok[3], "PK") == 0 || strcmp(tok[3], "PEQ") == 0)
        type = EQ_TYPE_PEAK;
    else if (strcmp(tok[3], "LSC") == 0 || strcmp(tok[3], "LS") == 0)
        type = EQ_TYPE_LSHELF;
    else if (strcmp(tok[3], "HSC") == 0 || strcmp(tok[3], "HS") == 0)
        type = EQ_TYPE_HSHELF;
    else
        return -1;                       /* LP/HP/notch/...: not supported */

    for (i = 4; i + 1 < n; i++) {
        if (strcmp(tok[i], "Fc") == 0) {
            if (parse_fixed(tok[++i], 0, &f) != 0)
                return -1;
            have_f = 1;
        } else if (strcmp(tok[i], "Gain") == 0) {
            if (parse_fixed(tok[++i], 1, &g) != 0)
                return -1;
            have_g = 1;
        } else if (strcmp(tok[i], "Q") == 0) {
            if (parse_fixed(tok[++i], 2, &q) != 0)
                return -1;
            have_q = 1;
        }
    }
    if (!have_f || !have_g || (type == EQ_TYPE_PEAK && !have_q))
        return -1;

    b->freq_hz = (uint16_t)clamp_int(f, EQ_FREQ_MIN_HZ, EQ_FREQ_MAX_HZ);
    b->gain_tdb = (int16_t)clamp_int(g, -EQ_GAIN_MAX_TDB, EQ_GAIN_MAX_TDB);
    b->q_x100 = (uint16_t)clamp_int(q, EQ_Q_MIN_X100, EQ_Q_MAX_X100);
    b->type = (uint8_t)type;
    return 1;
}

int eq_profile_parse_autoeq(const char *text, int len, eq_curve_t *out)
{
    eq_curve_t c;
    int pos = 0, n = 0, i, j;

    if (text == 0 || out == 0 || len <= 0)
        return -1;
    while (pos < len && text[pos] != '\0') {
        char line[128];
        int k = 0;

        while (pos < len && text[pos] != '\n' && text[pos] != '\0' &&
               k < (int)sizeof(line) - 1)
            line[k++] = text[pos++];
        if (k == (int)sizeof(line) - 1 && pos < len &&
            text[pos] != '\n' && text[pos] != '\0')
            return -1;                   /* over-long line */
        if (pos < len && text[pos] == '\n')
            pos++;
        line[k] = '\0';
        if (strncmp(trim(line), "Filter", 6) != 0)
            continue;                    /* Preamp, comments, blanks */
        if (n >= EQ_BANDS_MAX)
            return -1;                   /* more filters than the engine has */
        {
            int r = autoeq_filter(trim(line), &c.band[n]);

            if (r < 0)
                return -1;
            n += r;
        }
    }
    if (n == 0)
        return -1;

    /* Ascending, strictly increasing frequencies (eq_curve_valid). */
    for (i = 1; i < n; i++) {
        eq_band_t t = c.band[i];

        for (j = i; j > 0 && c.band[j - 1].freq_hz > t.freq_hz; j--)
            c.band[j] = c.band[j - 1];
        c.band[j] = t;
    }
    for (i = 1; i < n; i++)
        if (c.band[i].freq_hz <= c.band[i - 1].freq_hz)
            c.band[i].freq_hz = (uint16_t)(c.band[i - 1].freq_hz + 1u);
    while (n < EQ_BANDS_MIN) {           /* pad with flat (inactive) peaks */
        c.band[n].freq_hz = (uint16_t)(c.band[n - 1].freq_hz + 1u);
        c.band[n].q_x100 = 70;
        c.band[n].gain_tdb = 0;
        c.band[n].type = EQ_TYPE_PEAK;
        n++;
    }
    c.count = (uint8_t)n;
    c.enabled = 1;
    if (!eq_curve_valid(&c))
        return -1;
    *out = c;
    return 0;
}

int eq_profile_format(const eq_curve_t *curve, const char *name,
                      char *buf, unsigned cap)
{
    unsigned long pos = 0;
    int i;

    if (curve == 0 || buf == 0 || cap == 0 || !eq_curve_valid(curve))
        return -1;

    buf[0] = '\0';
    if (emit(buf, cap, &pos,
             "# ReChord EQ profile (BRNOMT-76)\n"
             "# key=value; bands: freq_hz,Q*100,gain_tenths_db\n"
             "format=1\n") != 0)
        return -1;
    if (emit(buf, cap, &pos, "enabled=") != 0 ||
        emit_int(buf, cap, &pos, curve->enabled) != 0 ||
        emit(buf, cap, &pos, "\nname=") != 0 ||
        emit(buf, cap, &pos, name != 0 ? name : "") != 0 ||
        emit(buf, cap, &pos, "\n") != 0)
        return -1;

    for (i = 0; i < curve->count; i++) {
        if (emit(buf, cap, &pos, "band") != 0 ||
            emit_int(buf, cap, &pos, i) != 0 ||
            emit(buf, cap, &pos, "=") != 0 ||
            emit_int(buf, cap, &pos, (int)curve->band[i].freq_hz) != 0 ||
            emit(buf, cap, &pos, ",") != 0 ||
            emit_int(buf, cap, &pos, (int)curve->band[i].q_x100) != 0 ||
            emit(buf, cap, &pos, ",") != 0 ||
            emit_int(buf, cap, &pos, (int)curve->band[i].gain_tdb) != 0)
            return -1;
        if (curve->band[i].type != EQ_TYPE_AUTO &&
            (emit(buf, cap, &pos, ",") != 0 ||
             emit_int(buf, cap, &pos, (int)curve->band[i].type) != 0))
            return -1;
        if (emit(buf, cap, &pos, "\n") != 0)
            return -1;
    }
    return (int)pos;
}

/* ---- file I/O (fs.h: target FAT + host backend) ---- */

static int read_text(const char *path, char *buf, unsigned cap)
{
    int n = fs_load_file(path, buf, cap);

    return n < 0 ? -1 : n;
}

static int write_file(const char *path, const char *text, int len)
{
    if (len <= 0)
        return -1;
    /* fs_store_file rewrites the preallocated file in place AND sets its size
     * to len. This used to be fs_patch(), which never changes the size: eq.cfg
     * is created with its (short) default template as its size, so every save
     * longer than that template was rejected with FS_ERR_ARG and the EQ state
     * was silently never persisted. */
    return (fs_store_file(path, text, (uint32_t)len) == FS_OK) ? 0 : -1;
}

static int slot_path(int slot, char *buf, unsigned cap)
{
    static const char prefix[] = RECHORD_EQ_DIR "/custom";
    unsigned n = (unsigned)strlen(prefix);

    if (slot < 1 || slot > EQ_PROFILE_SLOTS || n + 6u > cap)
        return -1;
    memcpy(buf, prefix, n);
    buf[n] = (char)('0' + slot);
    buf[n + 1] = '.';
    buf[n + 2] = 'c';
    buf[n + 3] = 'f';
    buf[n + 4] = 'g';
    buf[n + 5] = '\0';
    return 0;
}

static int slot_from_name(const char *name)
{
    int slot;

    if (strncmp(name, "custom", 6) != 0)
        return 0;
    name += 6;
    if (*name < '1' || *name > '0' + EQ_PROFILE_SLOTS)
        return 0;
    slot = *name++ - '0';
    if (strcmp(name, ".cfg") != 0)
        return 0;
    return slot;
}

/* "Sennheiser HD 650 ParametricEQ.txt" -> 1 and "Sennheiser HD 650". */
static int import_label(const char *name, char *label, unsigned cap)
{
    static const char k_tail[] = " ParametricEQ";
    unsigned n = (unsigned)strlen(name), t = sizeof k_tail - 1u, i;
    char buf[EQ_PROFILE_NAME_MAX + sizeof k_tail + 4];

    if (n < 5 || name[n - 4] != '.' ||
        (name[n - 3] | 0x20) != 't' || (name[n - 2] | 0x20) != 'x' ||
        (name[n - 1] | 0x20) != 't')
        return 0;
    n -= 4;
    if (n >= t) {
        for (i = 0; i < t; i++)
            if ((name[n - t + i] | 0x20) != (k_tail[i] | 0x20))
                break;
        if (i == t)
            n -= t;
    }
    if (n == 0)
        return 0;
    if (n > sizeof buf - 1u)
        n = sizeof buf - 1u;
    memcpy(buf, name, n);
    buf[n] = '\0';
    copy_str(label, cap, buf);
    return 1;
}

/* fs_list_dir_each() has no user pointer: the k-th import lookup. */
static int   s_find_k;
static char *s_find_out;
static unsigned s_find_cap;

static int find_import_cb(const char *name, int is_dir)
{
    char label[EQ_PROFILE_NAME_MAX];

    if (is_dir || !import_label(name, label, sizeof label))
        return 0;
    if (s_find_k-- != 0)
        return 0;
    copy_str(s_find_out, s_find_cap, name);
    return 1;                            /* stop */
}

static int scan_cb(const char *name, int is_dir)
{
    char path[48];
    char text[EQ_TEXT_MAX];
    char label[EQ_PROFILE_NAME_MAX];
    int slot, n;

    if (is_dir)
        return 0;
    slot = slot_from_name(name);
    if (slot == 0) {
        if (s_import_n < EQ_PROFILE_IMPORTS &&
            import_label(name, s_import_name[s_import_n],
                         EQ_PROFILE_NAME_MAX))
            s_import_n++;
        return 0;
    }
    if (slot_path(slot, path, (unsigned)sizeof path) != 0)
        return 0;

    label[0] = '\0';
    n = read_text(path, text, (unsigned)sizeof text);
    if (n > 0)
        find_name(text, n, label, (unsigned)sizeof label);
    if (label[0] == '\0') {
        copy_str(label, sizeof label, "Custom");
        {
            unsigned l = (unsigned)strlen(label);

            label[l] = ' ';
            label[l + 1] = (char)('0' + slot);
            label[l + 2] = '\0';
        }
    }
    copy_str(s_custom_name[slot - 1], EQ_PROFILE_NAME_MAX, label);
    s_custom_mask = (uint8_t)(s_custom_mask | (1u << (slot - 1)));
    return 0;
}

int eq_profile_scan(void)
{
    unsigned i;

    for (i = 0; i < EQ_PROFILE_SLOTS; i++)
        s_custom_name[i][0] = '\0';
    s_custom_mask = 0;
    s_import_n = 0;
    (void)fs_list_dir_each(RECHORD_EQ_DIR, scan_cb);
    return eq_profile_count();
}

/* ---- active curve / UI accessors ---- */

static void builtin_curve(int idx, eq_curve_t *c)
{
    int i;

    c->count = EQ_BANDS_MAX;
    c->enabled = 1;
    for (i = 0; i < EQ_BANDS_MAX; i++) {
        c->band[i].freq_hz = s_iso_freq[i];
        c->band[i].q_x100 = (uint16_t)default_q(i, EQ_BANDS_MAX);
        c->band[i].gain_tdb = s_builtin_gain[idx][i];
        c->band[i].type = EQ_TYPE_AUTO;
    }
}

int eq_profile_apply(void)
{
    s_active.enabled = s_enabled ? 1u : 0u;
    if (s_publish != 0)
        s_publish((int)s_enabled, &s_active);
    if (s_observer != 0 && !s_quiet)
        s_observer();
    return 0;
}

void eq_profile_set_observer(void (*fn)(void))
{
    s_observer = fn;
}

int eq_profile_restore(const eq_curve_t *curve, const char *name, int enabled)
{
    eq_curve_t c;

    if (curve == 0)
        return -1;
    c = *curve;
    c.enabled = enabled ? 1u : 0u;
    if (!eq_curve_valid(&c))
        return -1;
    s_active = c;
    s_enabled = c.enabled;
    copy_str(s_name, sizeof s_name, name != 0 ? name : "");
    s_dirty = 1;
    s_quiet = 1;                       /* restoring is not a user edit */
    (void)eq_profile_save();
    eq_profile_apply();
    s_quiet = 0;
    return 0;
}

void eq_profile_set_publisher(eq_publish_fn fn)
{
    s_publish = fn;
}

int eq_profile_save(void)
{
    char text[EQ_TEXT_MAX];
    int n = eq_profile_format(&s_active, s_name, text, (unsigned)sizeof text);

    if (n <= 0 || write_file(RECHORD_EQ_CFG, text, n) != 0)
        return -1;
    s_dirty = 0;
    return 0;
}

static int custom_slot_of_row(int row)
{
    int i, seen = 0;

    if (row < EQ_BUILTIN_COUNT)
        return 0;
    for (i = 0; i < EQ_PROFILE_SLOTS; i++) {
        if ((s_custom_mask & (1u << i)) == 0)
            continue;
        if (seen == row - EQ_BUILTIN_COUNT)
            return i + 1;
        seen++;
    }
    return 0;
}

int eq_profile_select(int row)
{
    eq_curve_t c;
    char label[EQ_PROFILE_NAME_MAX];

    label[0] = '\0';
    if (row < 0 || row >= eq_profile_count())
        return -1;

    if (row < EQ_BUILTIN_COUNT) {
        builtin_curve(row, &c);
        copy_str(label, sizeof label, s_builtin_name[row]);
    } else if (row >= EQ_BUILTIN_COUNT + eq_profile_custom_count()) {
        int k = row - EQ_BUILTIN_COUNT - eq_profile_custom_count();
        char file[FS_NAME_MAX];
        char path[sizeof RECHORD_EQ_DIR + FS_NAME_MAX + 1];
        char text[EQ_IMPORT_TEXT_MAX];
        char got[EQ_PROFILE_NAME_MAX];
        unsigned d = (unsigned)strlen(RECHORD_EQ_DIR);
        int n;

        s_find_k = k;
        s_find_out = file;
        s_find_cap = sizeof file;
        file[0] = '\0';
        (void)fs_list_dir_each(RECHORD_EQ_DIR, find_import_cb);
        /* The folder changed since the scan (USB): refuse a different file. */
        if (file[0] == '\0' || !import_label(file, got, sizeof got) ||
            strcmp(got, s_import_name[k]) != 0)
            return -1;
        memcpy(path, RECHORD_EQ_DIR, d);
        path[d] = '/';
        copy_str(path + d + 1, sizeof path - d - 1u, file);
        n = read_text(path, text, (unsigned)sizeof text);
        /* A full buffer may be a cut file: reject rather than half-parse. */
        if (n <= 0 || n >= (int)sizeof text - 1 ||
            eq_profile_parse_autoeq(text, n, &c) != 0)
            return -1;
        copy_str(label, sizeof label, s_import_name[k]);
    } else {
        char path[48];
        char text[EQ_TEXT_MAX];
        int slot = custom_slot_of_row(row);
        int n;

        if (slot == 0 || slot_path(slot, path, (unsigned)sizeof path) != 0)
            return -1;
        n = read_text(path, text, (unsigned)sizeof text);
        if (n <= 0 || eq_profile_parse(text, n, &c, label, sizeof label) != 0)
            return -1;
        if (label[0] == '\0')
            copy_str(label, sizeof label, s_custom_name[slot - 1]);
    }

    s_active = c;
    s_active.enabled = s_enabled ? 1u : 0u;
    copy_str(s_name, sizeof s_name, label);
    s_dirty = 1;
    (void)eq_profile_save();
    eq_profile_apply();
    return 0;
}

int eq_profile_ready(void)
{
    return s_booted != 0;
}

int eq_profile_boot(void)
{
    char text[EQ_TEXT_MAX];
    char label[EQ_PROFILE_NAME_MAX];
    eq_curve_t c;
    int n;

    builtin_curve(0, &c);                    /* Flat */
    copy_str(label, sizeof label, s_builtin_name[0]);
    s_enabled = 0;

    n = read_text(RECHORD_EQ_CFG, text, (unsigned)sizeof text);
    if (n > 0 && eq_profile_parse(text, n, &c, label, sizeof label) == 0)
        s_enabled = c.enabled;
    else
        builtin_curve(0, &c);
    if (label[0] == '\0')
        copy_str(label, sizeof label, s_builtin_name[0]);

    s_active = c;
    s_active.enabled = s_enabled;
    copy_str(s_name, sizeof s_name, label);
    s_dirty = 0;
    s_booted = 1;
    (void)eq_profile_scan();
    eq_profile_apply();
    return 0;
}

int eq_profile_save_to_row(int row)
{
    char path[48];
    char text[EQ_TEXT_MAX];
    int slot = custom_slot_of_row(row);
    int n;

    if (slot == 0 || slot_path(slot, path, (unsigned)sizeof path) != 0)
        return -1;
    n = eq_profile_format(&s_active, s_custom_name[slot - 1], text,
                          (unsigned)sizeof text);
    if (n <= 0 || write_file(path, text, n) != 0)
        return -1;
    copy_str(s_name, sizeof s_name, s_custom_name[slot - 1]);
    s_dirty = 1;
    (void)eq_profile_save();
    eq_profile_apply();
    return 0;
}

int eq_profile_custom_count(void)
{
    int count = 0;
    int i;

    for (i = 0; i < EQ_PROFILE_SLOTS; i++)
        if (s_custom_mask & (1u << i))
            count++;
    return count;
}

int eq_profile_count(void)
{
    return EQ_BUILTIN_COUNT + eq_profile_custom_count() + s_import_n;
}

const char *eq_profile_entry_name(int index)
{
    int slot;

    if (index < 0 || index >= eq_profile_count())
        return "";
    if (index < EQ_BUILTIN_COUNT)
        return s_builtin_name[index];
    if (index >= EQ_BUILTIN_COUNT + eq_profile_custom_count())
        return s_import_name[index - EQ_BUILTIN_COUNT -
                             eq_profile_custom_count()];
    slot = custom_slot_of_row(index);
    return (slot != 0) ? s_custom_name[slot - 1] : "";
}

int eq_profile_entry_is_custom(int index)
{
    return index >= EQ_BUILTIN_COUNT &&
           index < EQ_BUILTIN_COUNT + eq_profile_custom_count();
}

const eq_curve_t *eq_profile_curve(void)
{
    return &s_active;
}

const char *eq_profile_name(void)
{
    return s_name;
}

int eq_profile_enabled(void)
{
    return s_enabled != 0;
}

int eq_profile_is_dirty(void)
{
    return s_dirty != 0;
}

void eq_profile_set_enabled(int on)
{
    s_enabled = on ? 1u : 0u;
    s_active.enabled = s_enabled;
    s_dirty = 1;
    (void)eq_profile_save();
    eq_profile_apply();
}

void eq_profile_set_band_gain(int band, int gain_tdb)
{
    if (band < 0 || band >= (int)s_active.count)
        return;
    if (gain_tdb > EQ_GAIN_MAX_TDB)
        gain_tdb = EQ_GAIN_MAX_TDB;
    if (gain_tdb < -EQ_GAIN_MAX_TDB)
        gain_tdb = -EQ_GAIN_MAX_TDB;
    if (s_active.band[band].gain_tdb == gain_tdb)
        return;
    s_active.band[band].gain_tdb = (int16_t)gain_tdb;
    s_dirty = 1;
    eq_profile_apply();
}

void eq_profile_cycle_band(int band, int dir)
{
    int gain;

    if (band < 0 || band >= (int)s_active.count)
        return;
    gain = (int)s_active.band[band].gain_tdb + (dir >= 0 ? 5 : -5);
    eq_profile_set_band_gain(band, gain);
}

int eq_profile_format_gain(int gain_tdb, char *buf, unsigned cap)
{
    unsigned v = (unsigned)abs_int(gain_tdb);
    char digits[8];
    unsigned n = 0, i, whole;

    if (buf == 0 || cap < 6)
        return -1;
    if (gain_tdb < 0)
        buf[n++] = '-';
    else if (gain_tdb > 0)
        buf[n++] = '+';

    whole = v / 10u;
    i = (unsigned)sizeof digits - 1;
    digits[i] = '\0';
    do {
        digits[--i] = (char)('0' + (whole % 10u));
        whole /= 10u;
    } while (whole != 0);

    {
        const char *p = &digits[i];

        while (*p != '\0' && n + 3 < cap)
            buf[n++] = *p++;
    }
    buf[n++] = '.';
    buf[n++] = (char)('0' + (v % 10u));
    buf[n] = '\0';
    return (int)n;
}
