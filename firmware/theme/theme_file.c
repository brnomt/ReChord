/*
 * theme_file.c — file-based theme override loader (`key=value` format).
 *
 * WHY a plain text format: user-facing config is a plain key=value file
 * (architecture.md §3). Users edit it on the SD card with any editor, and a
 * typo must degrade gracefully (skip the line) instead of bricking the UI —
 * hence the tolerant parser below.
 *
 * Format (one pair per line):
 *     # comment          ; comment
 *     background=0x1083  ; colors: 0x-prefixed hex or decimal RGB565
 *     spacing=8          ; metrics: decimal (or hex)
 *     name=My Theme      ; name: free text up to THEME_NAME_MAX-1 chars
 *
 * Semantics are OVERRIDE (inout): only keys present in the text are written,
 * every other field of the target theme is preserved. A user file can
 * therefore override a single color of a built-in theme:
 *     theme_t t = *theme_get();            // or a built-in by name
 *     theme_file_parse(text, &t);          // apply overrides
 *     theme_set(&t);
 */
#include <stdio.h>
#include <string.h>

#include "theme.h"

#define LINE_MAX 512   /* longer lines are skipped whole (see loader) */

static int is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

static int key_eq(const char *k, int klen, const char *name)
{
    int i;
    for (i = 0; i < klen; i++) {
        char c = k[i];
        char d = name[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - ('a' - 'A'));
        if (d >= 'a' && d <= 'z') d = (char)(d - ('a' - 'A'));
        if (d == '\0' || c != d)
            return 0;
    }
    return name[klen] == '\0';
}

/* Parse an unsigned integer within [s, s+len): 0x hex or decimal.
 * Tolerant but strict enough to reject garbage ("12abc" is malformed —
 * silently guessing would corrupt colors). Returns 0 on success. */
static int parse_u32(const char *s, int len, unsigned *out)
{
    int i = 0;
    unsigned v = 0;

    while (i < len && is_space(s[i])) i++;
    if (i + 1 < len && s[i] == '0' && (s[i + 1] == 'x' || s[i + 1] == 'X')) {
        i += 2;
        if (i >= len)
            return -1;
        for (; i < len; i++) {
            char c = s[i];
            unsigned d;
            if (c >= '0' && c <= '9') d = (unsigned)(c - '0');
            else if (c >= 'a' && c <= 'f') d = (unsigned)(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') d = (unsigned)(c - 'A' + 10);
            else break;
            v = v * 16u + d;
        }
    } else {
        int any = 0;
        for (; i < len; i++) {
            char c = s[i];
            if (c < '0' || c > '9')
                break;
            v = v * 10u + (unsigned)(c - '0');
            any = 1;
        }
        if (!any)
            return -1;
    }
    while (i < len && is_space(s[i])) i++;
    if (i != len)
        return -1;   /* trailing junk: treat the line as malformed */
    *out = v;
    return 0;
}

/* Apply one key/value span. Returns 1 if applied, 0 if skipped. */
static int apply_kv(theme_t *t, const char *k, int klen,
                    const char *v, int vlen)
{
    unsigned num;

#define MATCH_COLOR(field)                                                   \
    if (key_eq(k, klen, #field)) {                                           \
        if (parse_u32(v, vlen, &num) != 0 || num > 0xFFFFu)                  \
            return 0;                                                        \
        t->field = (uint16_t)num;                                            \
        return 1;                                                            \
    }
#define MATCH_INT(field)                                                     \
    if (key_eq(k, klen, #field)) {                                           \
        if (parse_u32(v, vlen, &num) != 0 || num > 0x7FFFFFFFu)              \
            return 0;                                                        \
        t->field = (int)num;                                                 \
        return 1;                                                            \
    }

    MATCH_COLOR(background)
    MATCH_COLOR(surface)
    MATCH_COLOR(text)
    MATCH_COLOR(text_dim)
    MATCH_COLOR(accent)
    MATCH_COLOR(danger)
    MATCH_COLOR(success)
    MATCH_INT(spacing)
    MATCH_INT(radius)
    MATCH_INT(font_id)

#undef MATCH_COLOR
#undef MATCH_INT

    if (key_eq(k, klen, "name")) {
        int i;
        /* Trim surrounding spaces, copy what fits, always NUL-terminate. */
        while (vlen > 0 && is_space(*v)) { v++; vlen--; }
        while (vlen > 0 && is_space(v[vlen - 1])) vlen--;
        if (vlen == 0)
            return 0;
        for (i = 0; i < vlen && i < THEME_NAME_MAX - 1; i++)
            t->name[i] = v[i];
        t->name[i] = '\0';
        return 1;
    }

    return 0;   /* unknown key — skipped on purpose (forward compatibility) */
}

/*
 * Parse one `key=value` line whose extent is [start, end) (newline excluded).
 * Returns 1 if a pair was applied, 0 otherwise. Never fails on content.
 */
static int parse_line(theme_t *t, const char *start, const char *end)
{
    const char *p = start;
    const char *eq, *keystart, *keyend, *valstart, *valend;

    while (p < end && is_space(*p)) p++;
    if (p == end || *p == '#' || *p == ';')
        return 0;   /* blank or comment */

    for (eq = p; eq < end && *eq != '='; eq++)
        ;
    if (eq == end)
        return 0;   /* no '=' — malformed line, skipped */

    keystart = p;
    keyend = eq;
    while (keyend > keystart && is_space(keyend[-1])) keyend--;

    valstart = eq + 1;
    valend = end;
    while (valstart < valend && is_space(*valstart)) valstart++;
    while (valend > valstart && is_space(valend[-1])) valend--;

    return apply_kv(t, keystart, (int)(keyend - keystart),
                    valstart, (int)(valend - valstart));
}

int theme_file_parse(const char *text, theme_t *inout)
{
    const char *p;
    int applied = 0;

    if (text == 0 || inout == 0)
        return -1;

    p = text;
    while (*p != '\0') {
        const char *line_end = p;
        while (*line_end != '\0' && *line_end != '\n')
            line_end++;
        applied += parse_line(inout, p, line_end);
        p = (*line_end == '\n') ? line_end + 1 : line_end;
    }
    return applied;
}

int theme_file_load(const char *path, theme_t *inout)
{
    FILE *f;
    char line[LINE_MAX];
    int applied = 0;

    if (path == 0 || inout == 0)
        return -1;

    f = fopen(path, "r");
    if (f == 0)
        return -2;

    while (fgets(line, (int)sizeof(line), f) != 0) {
        int truncated = (strchr(line, '\n') == 0 && !feof(f));
        if (truncated) {
            /* Over-long line: consume and ignore the whole line rather than
             * parsing a chopped value (which could corrupt a color). */
            int c;
            while ((c = fgetc(f)) != EOF && c != '\n')
                ;
            continue;
        }
        applied += theme_file_parse(line, inout);
    }

    fclose(f);
    return applied;
}
