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

#define EQ_BUILTIN_COUNT 5
#define EQ_TEXT_MAX      512

/* ISO 10-band centre frequencies (Hz). */
static const uint16_t s_iso_freq[EQ_BANDS_MAX] = {
    31, 62, 125, 250, 500, 1000, 2000, 4000, 8000, 16000
};

/* Built-in curves in tenths of dB. Project data (standard graphic-EQ shapes),
 * not copied code; Q is the default 0.70 for every band. */
static const int8_t s_builtin_gain[EQ_BUILTIN_COUNT][EQ_BANDS_MAX] = {
    /* Flat  */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    /* Rock  */ { 40, 30, 10, -10, -20, -10, 10, 30, 40, 30 },
    /* Jazz  */ { 30, 20, 10, 20, -10, -10, 0, 10, 20, 20 },
    /* EDM   */ { 50, 40, 20, 0, -10, -10, 0, 10, 20, 20 },
    /* Vocal */ { -20, -10, 0, 10, 30, 30, 20, 10, 0, -10 }
};

static const char s_builtin_name[EQ_BUILTIN_COUNT][8] = {
    "Flat", "Rock", "Jazz", "EDM", "Vocal"
};

/* ---- AP state (kept small: the AP .bss ceiling is the tight budget) ---- */
static eq_curve_t s_active;
static char       s_name[EQ_PROFILE_NAME_MAX];
static char       s_custom_name[EQ_PROFILE_SLOTS][EQ_PROFILE_NAME_MAX];
static uint8_t    s_custom_mask;          /* bit N-1 = customN.cfg present */
static uint8_t    s_enabled;
static uint8_t    s_dirty;
static uint8_t    s_booted;
static eq_publish_fn s_publish;

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

/* "freq,Q*100,gain" -> band; ranges are checked later by eq_curve_valid. */
static int parse_triplet(char *v, eq_band_t *b)
{
    char *c1 = 0, *c2 = 0;
    char *p;
    int freq, q, gain;

    for (p = v; *p != '\0'; p++) {
        if (*p == ',') {
            if (c1 == 0) {
                c1 = p;
            } else if (c2 == 0) {
                c2 = p;
            } else {
                return -1;
            }
        }
    }
    if (c1 == 0 || c2 == 0)
        return -1;

    *c1 = '\0';
    *c2 = '\0';
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
        c.band[i].q_x100 = 70;
        c.band[i].gain_tdb = 0;
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
            emit_int(buf, cap, &pos, (int)curve->band[i].gain_tdb) != 0 ||
            emit(buf, cap, &pos, "\n") != 0)
            return -1;
    }
    return (int)pos;
}

/* ---- file I/O (fs.h: target FAT + host backend) ---- */

static int read_text(const char *path, char *buf, unsigned cap)
{
    fs_file_t *f;
    long n;

    if (cap < 2)
        return -1;
    f = fs_open_read(path);
    if (f == 0)
        return -1;
    n = fs_read(f, buf, cap - 1);
    fs_close(f);
    if (n < 0)
        return -1;
    buf[n] = '\0';
    return (int)n;
}

static int write_file(const char *path, const char *text, int len)
{
    if (len <= 0)
        return -1;
    /* NUL-terminated in place; the files are preallocated, so a shorter
     * rewrite leaves stale bytes after the terminator (the parser stops). */
    return (fs_patch(path, 0, text, (uint32_t)len + 1u) == FS_OK) ? 0 : -1;
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

static int scan_cb(const char *name, int is_dir)
{
    char path[48];
    char text[EQ_TEXT_MAX];
    char label[EQ_PROFILE_NAME_MAX];
    int slot, n;

    if (is_dir)
        return 0;
    slot = slot_from_name(name);
    if (slot == 0)
        return 0;
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
        c->band[i].q_x100 = 70;
        c->band[i].gain_tdb = s_builtin_gain[idx][i];
    }
}

int eq_profile_apply(void)
{
    s_active.enabled = s_enabled ? 1u : 0u;
    if (s_publish != 0)
        s_publish((int)s_enabled, &s_active);
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

int eq_profile_count(void)
{
    int count = EQ_BUILTIN_COUNT;
    int i;

    for (i = 0; i < EQ_PROFILE_SLOTS; i++)
        if (s_custom_mask & (1u << i))
            count++;
    return count;
}

const char *eq_profile_entry_name(int index)
{
    int slot;

    if (index < 0 || index >= eq_profile_count())
        return "";
    if (index < EQ_BUILTIN_COUNT)
        return s_builtin_name[index];
    slot = custom_slot_of_row(index);
    return (slot != 0) ? s_custom_name[slot - 1] : "";
}

int eq_profile_entry_is_custom(int index)
{
    return index >= EQ_BUILTIN_COUNT && index < eq_profile_count();
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
