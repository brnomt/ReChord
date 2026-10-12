/*
 * rechord_log.c — implementation of the circular logger (see rechord_log.h
 * for the context rules and line format).
 *
 * Ring layout: two monotonic counters, s_head (bytes ever committed by the
 * producer) and s_tail (bytes ever consumed by the flusher); pending =
 * s_head - s_tail (mod 2^32). The producer copies into free space and bumps
 * s_head LAST, so the consumer never sees a half-written line. Single
 * producer, single consumer, no locks.
 */
#include "rechord_log.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>

static char s_buf[RLOG_BUF_SIZE];
static volatile unsigned long s_head;    /* committed bytes (monotonic) */
static volatile unsigned long s_tail;    /* flushed bytes (monotonic) */
static unsigned long s_dropped;          /* lines dropped since last flush */
static unsigned long s_file_bytes;       /* tracked log-file size */
static rlog_clock_fn s_clock;            /* ms tick source; NULL = frozen */
static rlog_level_t s_min_level = RLOG_INFO;
static unsigned long s_limit = RLOG_FILE_MAX_BYTES;
static const char *s_path = RLOG_PATH;
static const char *s_old_path = RLOG_OLD_PATH;

#define FILE_SIZE_UNKNOWN 0xFFFFFFFFUL

static const char *level_name(rlog_level_t lvl)
{
    switch (lvl) {
    case RLOG_DEBUG: return "DEBUG";
    case RLOG_INFO:  return "INFO";
    case RLOG_WARN:  return "WARN";
    default:         return "ERROR";
    }
}

void rlog_init(rlog_clock_fn clock_ms)
{
    s_clock = clock_ms;
    s_head = 0;
    s_tail = 0;
    s_dropped = 0;
    s_file_bytes = FILE_SIZE_UNKNOWN;
}

void rlog_set_level(rlog_level_t min_level)
{
    s_min_level = min_level;
}

void rlog_set_file_limit(unsigned long bytes)
{
    s_limit = bytes;
}

void rlog_set_paths(const char *log_path, const char *old_path)
{
    if (log_path != NULL) {
        s_path = log_path;
    }
    if (old_path != NULL) {
        s_old_path = old_path;
    }
}

unsigned long rlog_pending(void)
{
    return s_head - s_tail;
}

unsigned long rlog_dropped(void)
{
    return s_dropped;
}

int rlog_should_flush(void)
{
    return rlog_pending() >= ((unsigned long)RLOG_BUF_SIZE / 4u) * 3u;
}

/* Producer side: append `len` bytes as ONE unit, or count a drop. Copies
 * first, commits s_head last (single-producer visibility rule). */
static void append_unit(const char *s, unsigned long len)
{
    unsigned long pending = s_head - s_tail;
    unsigned long pos, first;

    if (len > (unsigned long)RLOG_BUF_SIZE - pending) {
        s_dropped++;
        return;
    }
    pos = s_head % (unsigned long)RLOG_BUF_SIZE;
    first = (unsigned long)RLOG_BUF_SIZE - pos;
    if (first > len) {
        first = len;
    }
    memcpy(s_buf + pos, s, first);
    if (len > first) {
        memcpy(s_buf, s + first, len - first);
    }
    s_head += len;                    /* commit AFTER the copy */
}

void rlog_printf(rlog_level_t level, const char *subsystem, const char *fmt,
                 ...)
{
    char line[RLOG_LINE_MAX];
    unsigned long ms;
    va_list ap;
    int n;

    if (fmt == NULL || level < s_min_level) {
        return;
    }
    ms = (s_clock != NULL) ? s_clock() : 0UL;
    n = snprintf(line, sizeof(line), "[%lu][%s][%s] ", ms, level_name(level),
                 (subsystem != NULL) ? subsystem : "-");
    if (n < 0) {
        return;
    }
    if ((unsigned)n >= (unsigned)sizeof(line)) {
        n = (int)sizeof(line) - 1;
    }
    va_start(ap, fmt);
    (void)vsnprintf(line + n, sizeof(line) - (unsigned)n, fmt, ap);
    va_end(ap);
    line[sizeof(line) - 1] = '\0';

    /* Line + '\n' is one ring unit: a full ring drops whole lines. */
    {
        unsigned long len = (unsigned long)strlen(line);

        if (len + 1u <= (unsigned long)RLOG_LINE_MAX) {
            line[len] = '\n';
            append_unit(line, len + 1u);
        } else {
            line[sizeof(line) - 2] = '\n';
            line[sizeof(line) - 1] = '\0';
            append_unit(line, sizeof(line) - 1u);
        }
    }
}

/* Rotate the log file: drop the previous .old, rename the current file.
 * The file must be CLOSED (fs layer has no rename-while-open contract). */
static void rotate(void)
{
    (void)fs_remove(s_old_path);      /* best effort: FAT may lack replace */
    if (fs_rename(s_path, s_old_path) == FS_OK) {
        s_file_bytes = 0;
    }
    /* On failure the size stays over the limit and we retry next flush;
     * logs keep flowing to the oversized file rather than being lost. */
}

int rlog_flush(void)
{
    fs_file_t *f;
    int rc = RLOG_OK;

    if (rlog_pending() == 0 && s_dropped == 0) {
        return RLOG_OK;               /* nothing to do; skip the open */
    }
    /* Size check BEFORE the write so a rotation boundary never splits a
     * flush across two files. */
    if (s_file_bytes != FILE_SIZE_UNKNOWN && s_limit != 0 &&
        s_file_bytes >= s_limit) {
        rotate();
    }

    f = fs_open_append(s_path);
    if (f == NULL) {
        return RLOG_ERR_OPEN;         /* ring kept; retry at next flush */
    }
    if (s_file_bytes == FILE_SIZE_UNKNOWN) {
        long sz = fs_file_size(f);    /* first flush after boot: stat */
        s_file_bytes = (sz > 0) ? (unsigned long)sz : 0UL;
    }
    while (rlog_pending() > 0) {
        unsigned long pos = s_tail % (unsigned long)RLOG_BUF_SIZE;
        unsigned long chunk = (unsigned long)RLOG_BUF_SIZE - pos;

        if (chunk > rlog_pending()) {
            chunk = rlog_pending();
        }
        if (fs_write(f, s_buf + pos, chunk) != (long)chunk) {
            rc = RLOG_ERR_IO;         /* unwritten tail stays in the ring */
            break;
        }
        s_tail += chunk;              /* consumed only after a good write */
        s_file_bytes += chunk;
    }
    if (rc == RLOG_OK && s_dropped > 0) {
        /* Report the overflow at the point where it is recovered. */
        char drop[RLOG_LINE_MAX];
        unsigned long ms = (s_clock != NULL) ? s_clock() : 0UL;
        int n = snprintf(drop, sizeof(drop),
                         "[%lu][WARN][log] %lu lines dropped (ring full)\n",
                         ms, s_dropped);

        if (n > 0) {
            unsigned long len = ((unsigned long)n < sizeof(drop))
                ? (unsigned long)n : sizeof(drop) - 1u;
            if (fs_write(f, drop, len) != (long)len) {
                rc = RLOG_ERR_IO;
            } else {
                s_file_bytes += len;
                s_dropped = 0;
            }
        }
    }
    fs_close(f);

    if (rc == RLOG_OK && s_limit != 0 && s_file_bytes >= s_limit) {
        rotate();
    }
    return rc;
}
