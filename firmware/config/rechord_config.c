/*
 * rechord_config.c — parser/serializer/atomic store for the ReChord config
 * schema (see rechord_config.h for the contract and the WHYs).
 *
 * Structure: one descriptor table (key -> type/offset) drives the tolerant
 * text parser, the serializer AND the generic config_get_* / config_set_*
 * accessors, so the on-disk schema and the struct cannot drift apart.
 *
 * No third-party code; no dynamic memory; no reentrancy (static buffers in
 * load/save, by design for a freestanding target).
 */
#include "rechord_config.h"
#include "fs.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Whole-file buffer for load/save. The schema is ~600 bytes serialized.
 * SHARED by load and save on purpose: both are main-loop, non-reentrant
 * operations (documented in the header), and the AP RAM window is tight. */
#define CONFIG_TEXT_MAX   2048
static char s_text[CONFIG_TEXT_MAX];
/* Line scratch for the streaming parser. */
#define CONFIG_LINE_MAX   256

/* ---- small parsers (strict: whole string must be consumed) -------------- */

/* Strict decimal int: optional sign, digits only. 0 on success, -1 on
 * malformed input or overflow (guard works on 32-bit long too). */
static int parse_int(const char *s, int *out)
{
    long v = 0;
    int neg = 0, any = 0;

    if (s == NULL || out == NULL) {
        return -1;
    }
    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        if (v > (0x7FFFFFFFL - 9) / 10) {
            return -1;
        }
        v = v * 10 + (*s - '0');
        s++;
        any = 1;
    }
    if (!any || *s != '\0') {
        return -1;
    }
    *out = neg ? (int)-v : (int)v;
    return 0;
}

/* Strict unsigned 32-bit: digits only, value <= 0xFFFFFFFF. */
static int parse_u32(const char *s, uint32_t *out)
{
    unsigned long v = 0;
    int any = 0;

    if (s == NULL || out == NULL) {
        return -1;
    }
    while (*s >= '0' && *s <= '9') {
        unsigned long d = (unsigned long)(*s - '0');
        if (v > (0xFFFFFFFFUL - d) / 10UL) {
            return -1;
        }
        v = v * 10UL + d;
        s++;
        any = 1;
    }
    if (!any || *s != '\0') {
        return -1;
    }
    *out = (uint32_t)v;
    return 0;
}

/* Bools accept 0/1 plus the Rockbox-style words. -1 = unrecognized. */
static int parse_bool(const char *s, int *out)
{
    if (s == NULL || out == NULL) {
        return -1;
    }
    if (strcmp(s, "1") == 0 || strcmp(s, "true") == 0 || strcmp(s, "on") == 0) {
        *out = 1;
        return 0;
    }
    if (strcmp(s, "0") == 0 || strcmp(s, "false") == 0 || strcmp(s, "off") == 0) {
        *out = 0;
        return 0;
    }
    return -1;
}

/* Parse 1..8 hex digits (the crc32 trailer, optionally zero-padded). */
static int parse_hex8(const char *s, unsigned long *out)
{
    unsigned long v = 0;
    int any = 0;

    if (s == NULL || out == NULL) {
        return -1;
    }
    while (*s != '\0') {
        int d;
        char c = *s;

        if (c >= '0' && c <= '9') {
            d = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            d = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            d = c - 'A' + 10;
        } else {
            return -1;
        }
        if (any == 8) {
            return -1;               /* longer than 32 bits */
        }
        v = (v << 4) | (unsigned long)d;
        any++;
        s++;
    }
    if (!any) {
        return -1;
    }
    *out = v & 0xFFFFFFFFUL;
    return 0;
}

/* Trim leading/trailing spaces, tabs and CR in place; returns trimmed start. */
static char *trim(char *s)
{
    char *end;

    while (*s == ' ' || *s == '\t' || *s == '\r') {
        s++;
    }
    end = s + strlen(s);
    while (end > s &&
           (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r')) {
        end--;
    }
    *end = '\0';
    return s;
}

/* Copy src into dst (capacity cap), truncating safely (NUL always). */
static void copy_value(char *dst, unsigned long cap, const char *src)
{
    unsigned long n;

    if (cap == 0) {
        return;
    }
    n = (unsigned long)strlen(src);
    if (n >= cap) {
        n = cap - 1;
    }
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/* ---- CRC-32 (IEEE 802.3, reflected) — table-less, tiny flash cost ------ */

static unsigned long crc32_update(unsigned long crc, const char *s,
                                  unsigned long n)
{
    unsigned long i;
    int bit;

    for (i = 0; i < n; i++) {
        crc ^= (unsigned long)(unsigned char)s[i];
        for (bit = 0; bit < 8; bit++) {
            crc = (crc >> 1) ^ (0xEDB88320UL & (0UL - (crc & 1UL)));
        }
    }
    return crc & 0xFFFFFFFFUL;
}

/* ---- schema descriptor table ------------------------------------------- */

typedef enum {
    CFG_T_INT, CFG_T_BOOL, CFG_T_U8, CFG_T_U16, CFG_T_U32,
    CFG_T_ENUM, CFG_T_STR, CFG_T_INTLIST
} cfg_type_t;

typedef struct {
    const char       *key;     /* on-disk key */
    cfg_type_t        type;
    unsigned short    offset;  /* byte offset into rechord_config_t */
    unsigned short    size;    /* STR: cap incl NUL; INTLIST: element count */
    const char *const *names;  /* ENUM: NULL-terminated name table */
} cfg_field_t;

/* Enum name tables (on-disk values; numeric also accepted on load). */
static const char *const k_rg_names[]   = { "off", "track", "album", NULL };
static const char *const k_repeat_names[] = { "off", "one", "all", NULL };

/* offsetof() needs a non-null pointer expression; the idiom is standard. */
#define CFG_FLD(k, t, member, sz, names) \
    { k, t, (unsigned short)offsetof(rechord_config_t, member), sz, names }
#define CFG_STRCAP(member) ((unsigned short)sizeof(((rechord_config_t *)0)->member))

static const cfg_field_t k_fields[] = {
    CFG_FLD("volume",             CFG_T_INT,     volume, 0, NULL),
    CFG_FLD("balance",            CFG_T_INT,     balance, 0, NULL),
    CFG_FLD("replaygain_mode",    CFG_T_ENUM,    replaygain_mode, 0, k_rg_names),
    CFG_FLD("replaygain_preamp",  CFG_T_INT,     replaygain_preamp, 0, NULL),
    CFG_FLD("autoeq_enabled",     CFG_T_BOOL,    autoeq_enabled, 0, NULL),
    CFG_FLD("autoeq_profile",     CFG_T_STR,     autoeq_profile_path,
            CFG_STRCAP(autoeq_profile_path), NULL),
    CFG_FLD("eq_bands",           CFG_T_INTLIST, eq_bands_gain,
            RECHORD_EQ_BANDS, NULL),
    CFG_FLD("theme",              CFG_T_STR,     theme_name,
            CFG_STRCAP(theme_name), NULL),
    CFG_FLD("brightness",         CFG_T_U8,      brightness, 0, NULL),
    CFG_FLD("screen_timeout_sec", CFG_T_U16,     screen_timeout_sec, 0, NULL),
    CFG_FLD("language",           CFG_T_STR,     language,
            CFG_STRCAP(language), NULL),
    CFG_FLD("shuffle",            CFG_T_BOOL,    shuffle, 0, NULL),
    CFG_FLD("repeat_mode",        CFG_T_ENUM,    repeat_mode, 0, k_repeat_names),
    CFG_FLD("resume_track",       CFG_T_STR,     resume_track_path,
            CFG_STRCAP(resume_track_path), NULL),
    CFG_FLD("resume_offset_ms",   CFG_T_U32,     resume_offset_ms, 0, NULL),
};
#define CONFIG_FIELD_COUNT \
    ((int)(sizeof(k_fields) / sizeof(k_fields[0])))

static const cfg_field_t *find_field(const char *key)
{
    int i;

    for (i = 0; i < CONFIG_FIELD_COUNT; i++) {
        if (strcmp(k_fields[i].key, key) == 0) {
            return &k_fields[i];
        }
    }
    return NULL;
}

/* Enum name for an index, NULL when out of range. */
static const char *enum_name(const cfg_field_t *f, int idx)
{
    if (f->names == NULL || idx < 0) {
        return NULL;
    }
    return f->names[idx];
}

static int enum_count(const cfg_field_t *f)
{
    int n = 0;

    if (f->names != NULL) {
        while (f->names[n] != NULL) {
            n++;
        }
    }
    return n;
}

/* ---- apply one key=value to the struct (tolerant: bad values ignored) -- */

/* Comma-separated int16 list ("eq_bands"). Short lists zero the rest,
 * garbage bands become 0; values clamp to int16. */
static void apply_intlist(int16_t *dst, unsigned count, const char *val)
{
    unsigned i;

    for (i = 0; i < count; i++) {
        long v = 0;
        int neg = 0, any = 0;

        while (*val == ' ') {
            val++;
        }
        if (*val == '-') {
            neg = 1;
            val++;
        }
        while (*val >= '0' && *val <= '9') {
            if (v < 32767) {
                v = v * 10 + (*val - '0');
            }
            if (v > 32767) {
                v = 32767;           /* clamp, keep consuming digits */
            }
            val++;
            any = 1;
        }
        dst[i] = (int16_t)(neg ? -v : v);
        while (*val == ' ') {
            val++;
        }
        if (!any || *val != ',') {
            for (i++; i < count; i++) {
                dst[i] = 0;          /* short/garbled tail -> rest default */
            }
            break;
        }
        val++;
    }
}

static void apply_field(rechord_config_t *cfg, const char *key,
                        const char *val)
{
    const cfg_field_t *f = find_field(key);
    char *p;
    int iv;
    uint32_t u32;
    uint16_t u16v;
    uint8_t u8v;

    if (f == NULL) {
        return;                      /* unknown key: ignored by design */
    }
    p = (char *)cfg + f->offset;
    switch (f->type) {
    case CFG_T_STR:
        copy_value(p, f->size, val);
        break;
    case CFG_T_INT:
        if (parse_int(val, &iv) == 0) {
            *(int *)p = iv;
        }
        break;
    case CFG_T_BOOL:
        if (parse_bool(val, &iv) == 0) {
            *(int *)p = iv;
        }
        break;
    case CFG_T_U8:
        if (parse_int(val, &iv) == 0) {
            u8v = (uint8_t)(iv < 0 ? 0 : (iv > 255 ? 255 : iv));
            *(uint8_t *)p = u8v;
        }
        break;
    case CFG_T_U16:
        if (parse_int(val, &iv) == 0) {
            u16v = (uint16_t)(iv < 0 ? 0 : (iv > 65535 ? 65535 : iv));
            *(uint16_t *)p = u16v;
        }
        break;
    case CFG_T_U32:
        if (parse_u32(val, &u32) == 0) {
            *(uint32_t *)p = u32;
        }
        break;
    case CFG_T_ENUM:
        if (parse_int(val, &iv) == 0 && iv >= 0 && iv < enum_count(f)) {
            *(int *)p = iv;          /* numeric index */
        } else {
            int n;
            for (n = 0; n < enum_count(f); n++) {
                if (strcmp(val, f->names[n]) == 0) {
                    *(int *)p = n;
                    break;
                }
            }
        }
        break;
    case CFG_T_INTLIST:
        apply_intlist((int16_t *)p, f->size, val);
        break;
    default:
        break;
    }
}

/* ---- defaults ----------------------------------------------------------- */

void config_set_defaults(rechord_config_t *cfg)
{
    if (cfg == NULL) {
        return;
    }
    memset(cfg, 0, sizeof(*cfg));
    /* Defaults follow the stock device behaviour where one exists. */
    cfg->volume = -23;                /* stock sample config */
    cfg->brightness = 8;
    cfg->screen_timeout_sec = 30 * 60; /* stock screen_off = 30 min */
    memcpy(cfg->language, "en", 2);
    memcpy(cfg->theme_name, "default", 7);
}

/* ---- parse (pure, host-testable) ---------------------------------------- */

int config_parse_text(rechord_config_t *cfg, const char *text)
{
    char line[CONFIG_LINE_MAX];
    unsigned long n = 0;          /* bytes in `line` */
    unsigned long i = 0;          /* scan index into `text` */
    unsigned long line_start = 0; /* raw offset of the current line */
    unsigned long len;
    unsigned long crc_start = 0;
    unsigned long crc_expect = 0;
    int have_crc = 0;
    int skipping = 0;             /* over-long line: ignore it entirely */

    if (cfg == NULL || text == NULL) {
        return CONFIG_ERR_ARG;
    }
    config_set_defaults(cfg);
    len = (unsigned long)strlen(text);

    for (;;) {
        char ch = (i < len) ? text[i] : '\0';

        if (ch != '\0' && ch != '\n') {
            if (n < sizeof(line) - 1) {
                line[n++] = ch;
            } else {
                skipping = 1;
            }
            i++;
            continue;
        }
        /* Full line in `line` (or skipped). */
        line[n] = '\0';
        if (!skipping) {
            char *key = trim(line);
            char *eq = strchr(key, '=');

            if (key[0] != '\0' && key[0] != '#' && eq != NULL) {
                *eq = '\0';
                key = trim(key);   /* re-trim: " key " before '=' */
                if (strcmp(key, "crc32") == 0) {
                    /* Integrity trailer. CRC covers text[0 .. line_start):
                     * everything before the raw (untrimmed) start of this
                     * line, i.e. the body emitted by config_save(). */
                    unsigned long v = 0;
                    if (parse_hex8(trim(eq + 1), &v) == 0) {
                        crc_expect = v;
                        crc_start = line_start;
                        have_crc = 1;
                    }
                } else {
                    apply_field(cfg, key, trim(eq + 1));
                }
            }
        }
        n = 0;
        skipping = 0;
        if (ch == '\0') {
            break;
        }
        i++;
        line_start = i;
    }

    if (have_crc) {
        unsigned long crc = crc32_update(0xFFFFFFFFUL, text, crc_start);

        if ((crc ^ 0xFFFFFFFFUL) != crc_expect) {
            config_set_defaults(cfg);  /* torn/corrupt: refuse the values */
            return CONFIG_ERR_CRC;
        }
    }
    return CONFIG_OK;
}

/* ---- serialize (pure, host-testable) ------------------------------------ */

/* Append text to *pos within cap; keeps *pos <= cap. 0 on success. */
static int emit(char *buf, unsigned long cap, unsigned long *pos,
                const char *text)
{
    unsigned long n = (unsigned long)strlen(text);

    if (*pos + n + 1 > cap) {
        return -1;
    }
    memcpy(buf + *pos, text, n);
    *pos += n;
    buf[*pos] = '\0';
    return 0;
}

static int emit_int(char *buf, unsigned long cap, unsigned long *pos, int v)
{
    char digits[16];
    unsigned long u = (v < 0)
        ? (unsigned long)(-(long)(v + 1)) + 1UL  /* safe for INT_MIN */
        : (unsigned long)v;
    int i = (int)sizeof(digits) - 1;

    digits[i] = '\0';
    do {
        digits[--i] = (char)('0' + (u % 10UL));
        u /= 10UL;
    } while (u != 0 && i > 0);
    if (v < 0) {
        digits[--i] = '-';
    }
    return emit(buf, cap, pos, &digits[i]);
}

static int emit_uint(char *buf, unsigned long cap, unsigned long *pos,
                     unsigned long u)
{
    char digits[12];
    int i = (int)sizeof(digits) - 1;

    digits[i] = '\0';
    do {
        digits[--i] = (char)('0' + (u % 10UL));
        u /= 10UL;
    } while (u != 0 && i > 0);
    return emit(buf, cap, pos, &digits[i]);
}

/* Emit one field as "key=value\n" (INTLIST as comma-joined). 0 on success. */
static int emit_field(char *buf, unsigned long cap, unsigned long *pos,
                      const cfg_field_t *f, const rechord_config_t *cfg)
{
    const char *p = (const char *)cfg + f->offset;
    char scratch[16];
    unsigned i;

    if (emit(buf, cap, pos, f->key) != 0 || emit(buf, cap, pos, "=") != 0) {
        return -1;
    }
    switch (f->type) {
    case CFG_T_STR:
        if (emit(buf, cap, pos, p) != 0) {
            return -1;
        }
        break;
    case CFG_T_INT:
        if (emit_int(buf, cap, pos, *(const int *)p) != 0) {
            return -1;
        }
        break;
    case CFG_T_BOOL:
        if (emit(buf, cap, pos, (*(const int *)p) ? "on" : "off") != 0) {
            return -1;
        }
        break;
    case CFG_T_U8:
        if (emit_uint(buf, cap, pos, (unsigned long)*(const uint8_t *)p) != 0) {
            return -1;
        }
        break;
    case CFG_T_U16:
        if (emit_uint(buf, cap, pos, (unsigned long)*(const uint16_t *)p) != 0) {
            return -1;
        }
        break;
    case CFG_T_U32:
        if (emit_uint(buf, cap, pos, (unsigned long)*(const uint32_t *)p) != 0) {
            return -1;
        }
        break;
    case CFG_T_ENUM: {
        const char *name = enum_name(f, *(const int *)p);

        if (name == NULL) {
            name = "off";            /* out-of-range: emit a valid value */
        }
        if (emit(buf, cap, pos, name) != 0) {
            return -1;
        }
        break;
    }
    case CFG_T_INTLIST:
        for (i = 0; i < f->size; i++) {
            if (i != 0 && emit(buf, cap, pos, ",") != 0) {
                return -1;
            }
            (void)snprintf(scratch, sizeof(scratch), "%d",
                           (int)((const int16_t *)p)[i]);
            if (emit(buf, cap, pos, scratch) != 0) {
                return -1;
            }
        }
        break;
    default:
        return -1;
    }
    return emit(buf, cap, pos, "\n");
}

int config_serialize_text(const rechord_config_t *cfg, char *buf,
                          unsigned long cap)
{
    unsigned long pos = 0;
    unsigned long body_len;
    unsigned long crc;
    char trailer[24];
    int i;

    if (cfg == NULL || buf == NULL || cap == 0) {
        return CONFIG_ERR_ARG;
    }
    buf[0] = '\0';

    if (emit(buf, cap, &pos,
             "# rechord config v1 - key=value, '#' comments, unknown keys\n"
             "# are ignored on load. If you edit by hand, DELETE the\n"
             "# trailing crc32 line (it is a whole-file integrity check).\n") != 0) {
        return CONFIG_ERR_FULL;
    }
    for (i = 0; i < CONFIG_FIELD_COUNT; i++) {
        if (emit_field(buf, cap, &pos, &k_fields[i], cfg) != 0) {
            return CONFIG_ERR_FULL;
        }
    }
    body_len = pos;
    crc = crc32_update(0xFFFFFFFFUL, buf, body_len) ^ 0xFFFFFFFFUL;
    (void)snprintf(trailer, sizeof(trailer), "crc32=%08lx\n", crc);
    if (emit(buf, cap, &pos, trailer) != 0) {
        return CONFIG_ERR_FULL;
    }
    return (int)pos;
}

/* ---- store (atomic save over the fs abstraction) ------------------------ */

rechord_config_t *config_current(void)
{
    static rechord_config_t s_cfg;   /* zeroed at boot; see header */
    return &s_cfg;
}

int config_init(void)
{
    return config_load(config_current(), NULL);
}

int config_load(rechord_config_t *cfg, const char *path)
{
    fs_file_t *f;
    long got, size;
    int rc;

    if (cfg == NULL) {
        return CONFIG_ERR_ARG;
    }
    config_set_defaults(cfg);
    if (path == NULL) {
        path = CONFIG_PATH;
    }
    f = fs_open_read(path);
    if (f == NULL) {
        return CONFIG_ERR_OPEN;      /* defaults already applied */
    }
    size = fs_file_size(f);
    got = fs_read(f, s_text, (unsigned long)sizeof(s_text) - 1);
    fs_close(f);
    if (got < 0) {
        return CONFIG_ERR_IO;
    }
    if (size > (long)sizeof(s_text) - 1) {
        got = (long)sizeof(s_text) - 1; /* over-long: parse the part we hold */
    }
    s_text[got] = '\0';

    rc = config_parse_text(cfg, s_text);
    /* CONFIG_ERR_CRC: parse already reset *cfg to defaults. */
    return rc;
}

int config_save(const rechord_config_t *cfg, const char *path)
{
    char tmp[FS_NAME_MAX];
    fs_file_t *f;
    long wrote;
    int len;

    if (cfg == NULL) {
        return CONFIG_ERR_ARG;
    }
    if (path == NULL) {
        path = CONFIG_PATH;
    }
    len = config_serialize_text(cfg, s_text, (unsigned long)sizeof(s_text));
    if (len < 0) {
        return len;
    }
    if (snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp)) {
        return CONFIG_ERR_ARG;
    }
    /* Step 1: full write to the temp file (fs_close flushes in the
     * backends; a durable fsync arrives with the FAT write work). */
    f = fs_open_write(tmp);
    if (f == NULL) {
        return CONFIG_ERR_OPEN;
    }
    wrote = fs_write(f, s_text, (unsigned long)len);
    fs_close(f);
    if (wrote != (long)len) {
        (void)fs_remove(tmp);
        return CONFIG_ERR_IO;
    }
    /* Step 2: atomically replace the real file. Until the rename lands, the
     * previous config is fully intact, so a power cut here is survivable. */
    if (fs_rename(tmp, path) != FS_OK) {
        (void)fs_remove(tmp);
        return CONFIG_ERR_IO;
    }
    return CONFIG_OK;
}

/* ---- generic keyed access over the singleton ---------------------------- */

/* Numeric read of a field; STR/INTLIST cannot round-trip through int and
 * report default_val. */
static int cfg_get_int(const rechord_config_t *cfg, const cfg_field_t *f,
                       int default_val)
{
    const char *p = (const char *)cfg + f->offset;
    uint32_t u32;

    switch (f->type) {
    case CFG_T_INT:
        return *(const int *)p;
    case CFG_T_BOOL:
        return *(const int *)p ? 1 : 0;
    case CFG_T_U8:
        return (int)*(const uint8_t *)p;
    case CFG_T_U16:
        return (int)*(const uint16_t *)p;
    case CFG_T_U32:
        u32 = *(const uint32_t *)p;
        return (u32 <= 0x7FFFFFFFUL) ? (int)u32 : default_val;
    case CFG_T_ENUM:
        return *(const int *)p;      /* enum index */
    default:
        return default_val;
    }
}

/* Numeric write with structural clamping. STR/INTLIST reject int writes. */
static int cfg_set_int(rechord_config_t *cfg, const cfg_field_t *f, int v)
{
    char *p = (char *)cfg + f->offset;

    switch (f->type) {
    case CFG_T_INT:
        *(int *)p = v;
        return 0;
    case CFG_T_BOOL:
        *(int *)p = v ? 1 : 0;
        return 0;
    case CFG_T_U8:
        *(uint8_t *)p = (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v));
        return 0;
    case CFG_T_U16:
        *(uint16_t *)p = (uint16_t)(v < 0 ? 0 : (v > 65535 ? 65535 : v));
        return 0;
    case CFG_T_U32:
        *(uint32_t *)p = (v < 0) ? 0u : (uint32_t)v;
        return 0;
    case CFG_T_ENUM:
        if (v < 0 || v >= enum_count(f)) {
            return -1;
        }
        *(int *)p = v;
        return 0;
    default:
        return -1;
    }
}

int config_get_int(const char *key, int default_val)
{
    const cfg_field_t *f = (key != NULL) ? find_field(key) : NULL;

    if (f == NULL) {
        return default_val;
    }
    return cfg_get_int(config_current(), f, default_val);
}

int config_set_int(const char *key, int value)
{
    const cfg_field_t *f = (key != NULL) ? find_field(key) : NULL;

    if (f == NULL) {
        return -1;
    }
    return cfg_set_int(config_current(), f, value);
}

int config_get_bool(const char *key, int default_val)
{
    const cfg_field_t *f = (key != NULL) ? find_field(key) : NULL;

    if (f == NULL) {
        return default_val ? 1 : 0;
    }
    return cfg_get_int(config_current(), f, 0) ? 1 : 0;
}

int config_set_bool(const char *key, int value)
{
    return config_set_int(key, value ? 1 : 0);
}

int config_get_string(const char *key, char *out, unsigned long cap,
                      const char *default_val)
{
    const cfg_field_t *f = (key != NULL) ? find_field(key) : NULL;
    const char *src;

    if (out == NULL || cap == 0) {
        return CONFIG_ERR_ARG;
    }
    if (f == NULL || f->type != CFG_T_STR) {
        copy_value(out, cap, (default_val != NULL) ? default_val : "");
        return (int)strlen(out);
    }
    src = (const char *)config_current() + f->offset;
    copy_value(out, cap, src);
    return (int)strlen(out);
}

int config_set_string(const char *key, const char *value)
{
    const cfg_field_t *f = (key != NULL) ? find_field(key) : NULL;
    char *p;

    if (f == NULL || f->type != CFG_T_STR) {
        return -1;
    }
    p = (char *)config_current() + f->offset;
    copy_value(p, f->size, (value != NULL) ? value : "");
    return (int)strlen(p);           /* length actually stored */
}
