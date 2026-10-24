/*
 * settings.c — parser/serializer for the "\RECHORD.CFG" settings schema.
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

#include <string.h>

/* Line scratch for streaming parse. */
#define SETTINGS_LINE_MAX 128

void settings_defaults(settings_t *s)
{
    if (s == NULL) {
        return;
    }
    memset(s, 0, sizeof(*s));
    /* Defaults = the documented sample configuration, except auto_off: the
     * standby timer defaults to 5 min instead of the sample's 15. */
    memcpy(s->play, "repeat_track", 12);
    s->brightness = 8;
    s->screen_off_min = 30;
    s->auto_off_min = 5;    /* standby auto-off (ReChord default) */
    memcpy(s->usb, "ask", 3);
    memcpy(s->resume, "folder", 6);
    memcpy(s->strip, "off", 3);
    memcpy(s->tags, "on", 2);
    memcpy(s->cpu, "auto", 4);
    s->volume = -23;
    s->browse[0] = '\0';
    s->cursor = 0;
    memcpy(s->shuffle, "off", 3);
    memcpy(s->repeat_mode, "off", 3);
    memcpy(s->fade, "300", 3);   /* BRNOMT-67: 150/300/500 ms, off=0 */
    s->theme[0] = '\0';          /* empty: boot falls back to active_theme */
    memcpy(s->replaygain, "auto", 5);   /* BRNOMT-106: off/track/album/auto */
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

    /* Fixed key schema. Field sizes are the struct's array sizes minus 1;
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
    } else if (strcmp(key, "shuffle") == 0) {
        copy_value(s->shuffle, sizeof(s->shuffle), val);
    } else if (strcmp(key, "repeat_mode") == 0) {
        copy_value(s->repeat_mode, sizeof(s->repeat_mode), val);
    } else if (strcmp(key, "fade") == 0) {
        copy_value(s->fade, sizeof(s->fade), val);
    } else if (strcmp(key, "theme") == 0) {
        copy_value(s->theme, sizeof(s->theme), val);
    } else if (strcmp(key, "replaygain") == 0) {
        copy_value(s->replaygain, sizeof(s->replaygain), val);
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

/* The I/O halves of the service live next to their backend:
 *   settings_fs.c    — host/tools: whole-file read/write through fs.h
 *   settings_store.c — target: read + in-place rewrite of the preallocated
 *                      /.rechord/rechord.cfg (no FAT metadata writes).
 * Keeping the parser here means both backends share one format. */

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
        emit(buf, cap, &pos, "\nshuffle=") != 0 ||
        emit(buf, cap, &pos, s->shuffle) != 0 ||
        emit(buf, cap, &pos, "\nrepeat_mode=") != 0 ||
        emit(buf, cap, &pos, s->repeat_mode) != 0 ||
        emit(buf, cap, &pos, "\nfade=") != 0 ||
        emit(buf, cap, &pos, s->fade) != 0 ||
        emit(buf, cap, &pos, "\ntheme=") != 0 ||
        emit(buf, cap, &pos, s->theme) != 0 ||
        emit(buf, cap, &pos, "\nreplaygain=") != 0 ||
        emit(buf, cap, &pos, s->replaygain) != 0 ||
        emit(buf, cap, &pos, "\n") != 0) {
        return SETTINGS_ERR_FULL;
    }
    return (int)pos;
}
