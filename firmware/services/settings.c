/*
 * settings.c — parser/serializer for the 12-key "\RECHORD.CFG" schema.
 *
 * Written from the documented FORMAT FACTS only (docs/re/refcfw-analysis.md):
 * key=value lines, '#' comments, unknown keys ignored. No third-party code is
 * used or paraphrased; this is a fresh implementation of the documented
 * on-disk format.
 *
 * Parser tolerance rules:
 *   - blank lines and lines whose first non-space char is '#' are skipped,
 *   - a key is the text before the first '=', trimmed of surrounding spaces,
 *   - a value is the rest of the line, trimmed of spaces and trailing CR,
 *   - lines without '=' are ignored,
 *   - unknown keys are ignored,
 *   - invalid integers keep the previous (default) value,
 *   - values longer than their field are truncated to fit (NUL-terminated).
 */
#include "settings.h"
#include "fs.h"

#include <string.h>

/* Read buffer for the whole config file; the format is tiny by design. */
#define SETTINGS_FILE_MAX 2048

/* Line scratch for streaming parse. */
#define SETTINGS_LINE_MAX 128

void settings_defaults(settings_t *s)
{
    if (s == NULL) {
        return;
    }
    memset(s, 0, sizeof(*s));
    /* Defaults = the documented sample configuration (drop-in familiar). */
    memcpy(s->play, "repeat_track", 12);
    s->brightness = 8;
    s->screen_off_min = 30;
    s->auto_off_min = 15;
    memcpy(s->usb, "ask", 3);
    memcpy(s->resume, "folder", 6);
    memcpy(s->strip, "off", 3);
    memcpy(s->tags, "on", 2);
    memcpy(s->cpu, "auto", 4);
    s->volume = -23;
    s->browse[0] = '\0';
    s->cursor = 0;
}

/* Copy src into dst (capacity cap), truncating safely. */
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

/* Strict decimal parse: optional sign, digits only, whole string consumed.
 * Returns 0 on success, -1 on malformed input. */
static int parse_int(const char *s, int *out)
{
    long v = 0;
    int neg = 0;
    int any = 0;

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
            return -1;          /* overflow guard (works on 32-bit long too) */
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

/* Apply one "key=value" line. Returns 1 if a known key was applied,
 * 0 if the line was ignored. */
static int parse_line(char *line, settings_t *s)
{
    char *eq;
    char *key;
    char *val;

    key = trim(line);
    if (key[0] == '\0' || key[0] == '#') {
        return 0;
    }
    eq = strchr(key, '=');
    if (eq == NULL) {
        return 0;
    }
    *eq = '\0';
    val = trim(eq + 1);
    key = trim(key);            /* re-trim: " key " before '=' */

    /* Fixed 12-key schema. Field sizes are the struct's array sizes minus 1;
     * copy_value truncates anything longer. */
    if (strcmp(key, "play") == 0) {
        copy_value(s->play, sizeof(s->play), val);
    } else if (strcmp(key, "brightness") == 0) {
        (void)parse_int(val, &s->brightness);
    } else if (strcmp(key, "screen_off") == 0) {
        (void)parse_int(val, &s->screen_off_min);
    } else if (strcmp(key, "auto_off") == 0) {
        (void)parse_int(val, &s->auto_off_min);
    } else if (strcmp(key, "usb") == 0) {
        copy_value(s->usb, sizeof(s->usb), val);
    } else if (strcmp(key, "resume") == 0) {
        copy_value(s->resume, sizeof(s->resume), val);
    } else if (strcmp(key, "strip") == 0) {
        copy_value(s->strip, sizeof(s->strip), val);
    } else if (strcmp(key, "tags") == 0) {
        copy_value(s->tags, sizeof(s->tags), val);
    } else if (strcmp(key, "cpu") == 0) {
        copy_value(s->cpu, sizeof(s->cpu), val);
    } else if (strcmp(key, "volume") == 0) {
        (void)parse_int(val, &s->volume);
    } else if (strcmp(key, "browse") == 0) {
        copy_value(s->browse, sizeof(s->browse), val);
    } else if (strcmp(key, "cursor") == 0) {
        (void)parse_int(val, &s->cursor);
    } else {
        return 0;               /* unknown key: ignored by design */
    }
    return 1;
}

int settings_parse_text(const char *text, settings_t *s)
{
    char line[SETTINGS_LINE_MAX];
    unsigned long n = 0;
    int skipping = 0;           /* over-long line: skip tail, parse nothing */

    if (text == NULL || s == NULL) {
        return SETTINGS_ERR_ARG;
    }
    for (;;) {
        char ch = *text;
        if (ch != '\0' && ch != '\n') {
            if (n < sizeof(line) - 1) {
                line[n++] = ch;
            } else {
                skipping = 1;   /* line too long: ignore it entirely */
            }
        } else {
            line[n] = '\0';
            if (!skipping) {
                (void)parse_line(line, s);
            }
            n = 0;
            skipping = 0;
            if (ch == '\0') {
                break;
            }
        }
        if (ch != '\0') {
            text++;
        }
    }
    return SETTINGS_OK;
}

int settings_load(settings_t *s)
{
    static char buf[SETTINGS_FILE_MAX];
    fs_file_t *f;
    long got;
    long size;
    int rc = SETTINGS_OK;

    if (s == NULL) {
        return SETTINGS_ERR_ARG;
    }
    settings_defaults(s);

    f = fs_open_read(SETTINGS_PATH);
    if (f == NULL) {
        return SETTINGS_ERR_OPEN;
    }
    size = fs_file_size(f);
    got = fs_read(f, buf, (unsigned long)sizeof(buf) - 1);
    if (got < 0) {
        rc = SETTINGS_ERR_IO;
        got = 0;
    } else if (size > (long)sizeof(buf) - 1) {
        /* Over-long file: parse the part we hold (all 12 keys live in the
         * first lines by format convention). Not an error — tolerant. */
        got = (long)sizeof(buf) - 1;
    }
    buf[got] = '\0';
    fs_close(f);

    if (rc == SETTINGS_OK) {
        rc = settings_parse_text(buf, s);
    }
    return rc;
}

/* Absolute value that is safe for INT_MIN. */
static unsigned long abs_int(int v)
{
    return (v < 0) ? (unsigned long)(-(long)(v + 1)) + 1UL : (unsigned long)v;
}

/* Append "text" to *pos within buf; keeps *pos <= cap. Returns 0 on success. */
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
    unsigned long u = abs_int(v);
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

int settings_serialize_text(const settings_t *s, char *buf, unsigned long cap)
{
    unsigned long pos = 0;

    if (s == NULL || buf == NULL || cap == 0) {
        return SETTINGS_ERR_ARG;
    }
    buf[0] = '\0';

    /* Comment header (cosmetic; parsers must ignore it). */
    if (emit(buf, cap, &pos,
             "# RECHORD.CFG - ReChord settings\n"
             "# key=value, one per line. Unknown keys are ignored.\n") != 0) {
        return SETTINGS_ERR_FULL;
    }

    /* Fixed schema order (drop-in familiar layout). */
    if (emit(buf, cap, &pos, "play=") != 0 ||
        emit(buf, cap, &pos, s->play) != 0 ||
        emit(buf, cap, &pos, "\nbrightness=") != 0 ||
        emit_int(buf, cap, &pos, s->brightness) != 0 ||
        emit(buf, cap, &pos, "\nscreen_off=") != 0 ||
        emit_int(buf, cap, &pos, s->screen_off_min) != 0 ||
        emit(buf, cap, &pos, "\nauto_off=") != 0 ||
        emit_int(buf, cap, &pos, s->auto_off_min) != 0 ||
        emit(buf, cap, &pos, "\nusb=") != 0 ||
        emit(buf, cap, &pos, s->usb) != 0 ||
        emit(buf, cap, &pos, "\nresume=") != 0 ||
        emit(buf, cap, &pos, s->resume) != 0 ||
        emit(buf, cap, &pos, "\nstrip=") != 0 ||
        emit(buf, cap, &pos, s->strip) != 0 ||
        emit(buf, cap, &pos, "\ntags=") != 0 ||
        emit(buf, cap, &pos, s->tags) != 0 ||
        emit(buf, cap, &pos, "\ncpu=") != 0 ||
        emit(buf, cap, &pos, s->cpu) != 0 ||
        emit(buf, cap, &pos, "\nvolume=") != 0 ||
        emit_int(buf, cap, &pos, s->volume) != 0 ||
        emit(buf, cap, &pos, "\nbrowse=") != 0 ||
        emit(buf, cap, &pos, s->browse) != 0 ||
        emit(buf, cap, &pos, "\ncursor=") != 0 ||
        emit_int(buf, cap, &pos, s->cursor) != 0 ||
        emit(buf, cap, &pos, "\n") != 0) {
        return SETTINGS_ERR_FULL;
    }
    return (int)pos;
}

int settings_save(const settings_t *s)
{
    char text[768];
    fs_file_t *f;
    int len;
    long wrote;

    if (s == NULL) {
        return SETTINGS_ERR_ARG;
    }
    len = settings_serialize_text(s, text, (unsigned long)sizeof(text));
    if (len < 0) {
        return len;
    }
    f = fs_open_write(SETTINGS_PATH);
    if (f == NULL) {
        return SETTINGS_ERR_OPEN;
    }
    wrote = fs_write(f, text, (unsigned long)len);
    fs_close(f);
    return (wrote == (long)len) ? SETTINGS_OK : SETTINGS_ERR_IO;
}
