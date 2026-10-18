/*
 * resume.c — the /.rechord/state file (BRNOMT-87).
 *
 * Reads through the read-only fs.h path; saves go through
 * fs_target_file_rewrite() (the same class of write as the config: existing
 * clusters + dirent size, never an allocation or a FAT update). Buffers are
 * stack-only so this costs 0 B of the target .bss budget.
 */
#include <stdio.h>
#include <string.h>

#include "resume.h"
#include "rechord_files.h"
#include "fs.h"
#include "fs_target.h"

/* Longest state text accepted/emitted: 2 x 208-byte paths + numbers + keys. */
#define STATE_TEXT_MAX 768

void resume_default(resume_state_t *s)
{
    if (s == 0)
        return;
    memset(s, 0, sizeof *s);
}

/* Copy [from, to) into a bounded NUL-terminated field (over-long values are
 * truncated rather than rejected: a path that no longer fits is still better
 * than losing the whole line). */
static void copy_value(char *dst, unsigned cap, const char *from,
                       const char *to)
{
    unsigned n = 0;

    if (dst == 0 || cap == 0)
        return;
    while (from < to && n + 1u < cap)
        dst[n++] = *from++;
    dst[n] = '\0';
}

static unsigned long parse_ulong(const char *from, const char *to)
{
    unsigned long v = 0;
    int seen = 0;

    while (from < to && (*from == ' ' || *from == '\t'))
        from++;
    while (from < to && *from >= '0' && *from <= '9') {
        v = v * 10u + (unsigned long)(*from - '0');
        from++;
        seen = 1;
    }
    return seen ? v : 0;
}

/* Walk the key=value text. Unparseable lines are skipped by construction. */
static void parse_text(const char *text, resume_state_t *s)
{
    const char *p = text;

    while (p != 0 && *p != '\0') {
        const char *eol = p;
        const char *eq;
        size_t klen;

        while (*eol != '\0' && *eol != '\n' && *eol != '\r')
            eol++;
        for (eq = p; eq < eol && *eq != '='; eq++)
            ;
        if (eq < eol) {
            klen = (size_t)(eq - p);

            if (klen == 5u && strncmp(p, "track", 5) == 0)
                copy_value(s->track, (unsigned)sizeof s->track, eq + 1, eol);
            else if (klen == 6u && strncmp(p, "pos_ms", 6) == 0)
                s->pos_ms = parse_ulong(eq + 1, eol);
            else if (klen == 6u && strncmp(p, "folder", 6) == 0)
                copy_value(s->folder, (unsigned)sizeof s->folder, eq + 1, eol);
            else if (klen == 3u && strncmp(p, "row", 3) == 0)
                s->row = (int)parse_ulong(eq + 1, eol);
        }
        if (*eol == '\r' && eol[1] == '\n')
            p = eol + 2;
        else if (*eol != '\0')
            p = eol + 1;
        else
            p = eol;
    }
}

int resume_load(resume_state_t *s)
{
    char buf[STATE_TEXT_MAX];
    fs_file_t *f;
    long got;

    if (s == 0)
        return -1;
    resume_default(s);

    f = fs_open_read(RECHORD_STATE);
    if (f == 0)
        return -1;
    got = fs_read(f, buf, (unsigned long)sizeof(buf) - 1u);
    fs_close(f);
    if (got < 0) {
        resume_default(s);
        return -1;
    }
    buf[got] = '\0';
    parse_text(buf, s);
    return 0;
}

int resume_save(const resume_state_t *s)
{
    char text[STATE_TEXT_MAX];
    int n;

    if (s == 0)
        return -1;
    n = snprintf(text, sizeof text, "track=%s\npos_ms=%lu\nfolder=%s\nrow=%d\n",
                 s->track, s->pos_ms, s->folder, s->row);
    if (n < 0 || (unsigned)n >= sizeof text)
        return -1;                       /* would not round-trip: keep old */
    if (fs_target_file_rewrite(RECHORD_STATE, text, (uint32_t)n) != FS_OK)
        return -1;
    return 0;
}
