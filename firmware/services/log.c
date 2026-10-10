/*
 * log.c — ring-buffered logger (see log.h for the contract).
 *
 * Written from documented format facts only (the on-device log line format
 * and the flush-with-tag convention); no third-party code is used.
 */
#include "log.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>

static char s_lines[LOG_BUFFER_LINES][LOG_LINE_MAX];
static unsigned s_head;          /* index of the oldest buffered line */
static unsigned s_count;         /* lines currently buffered */
static unsigned long s_dropped;  /* lines dropped since last good flush */
static log_clock_fn s_clock;     /* ms tick source; NULL = frozen at 0 */

void log_init(log_clock_fn clock_ms)
{
    s_clock = clock_ms;
}

unsigned long log_dropped(void)
{
    return s_dropped;
}

unsigned log_pending(void)
{
    return s_count;
}

static unsigned long now_ms(void)
{
    return (s_clock != NULL) ? s_clock() : 0UL;
}

/* Copy one finished line into the ring, or count it as dropped. */
static void append_line(const char *line)
{
    char *slot;
    unsigned long n;

    if (s_count >= (unsigned)LOG_BUFFER_LINES) {
        s_dropped++;
        return;
    }
    slot = s_lines[(s_head + s_count) % (unsigned)LOG_BUFFER_LINES];
    /* log_printf guarantees the line fits; this copy is bounded anyway. */
    n = (unsigned long)strlen(line);
    if (n > (unsigned long)LOG_LINE_MAX - 1) {
        n = (unsigned long)LOG_LINE_MAX - 1;
    }
    memcpy(slot, line, n);
    slot[n] = '\0';
    s_count++;
}

void log_printf(const char *fmt, ...)
{
    char line[LOG_LINE_MAX];
    unsigned long ms;
    va_list ap;
    int n;

    if (fmt == NULL) {
        return;
    }
    ms = now_ms();
    n = snprintf(line, sizeof(line), "%lu.%03lu ",
                 ms / 1000UL, ms % 1000UL);
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
    append_line(line);
}

/* Write one NUL-terminated line plus '\n'. Returns 0 or FS_ERR_*. */
static int write_line(fs_file_t *f, const char *line)
{
    unsigned long n = (unsigned long)strlen(line);

    if (fs_write(f, line, n) != (long)n) {
        return FS_ERR_IO;
    }
    if (fs_write(f, "\n", 1) != 1) {
        return FS_ERR_IO;
    }
    return FS_OK;
}

int log_flush(const char *tag)
{
    fs_file_t *f;
    unsigned i;
    int rc = LOG_OK;

    if (tag != NULL) {
        /* Flush markers follow the same line format as everything else. */
        log_printf("log flush (%s)", tag);
    }

    f = fs_open_append(LOG_PATH);
    if (f == NULL) {
        return LOG_ERR_OPEN;    /* keep the buffer; retry at next flush */
    }
    for (i = 0; i < s_count; i++) {
        const char *ln = s_lines[(s_head + i) % (unsigned)LOG_BUFFER_LINES];
        if (write_line(f, ln) != FS_OK) {
            rc = LOG_ERR_IO;
            break;
        }
    }
    if (rc == LOG_OK && s_dropped > 0) {
        /* Report the overflow at the point where it is recovered. */
        char drop_line[LOG_LINE_MAX];
        unsigned long ms = now_ms();
        (void)snprintf(drop_line, sizeof(drop_line),
                       "%lu.%03lu log: %lu lines dropped (buffer full)",
                       ms / 1000UL, ms % 1000UL, s_dropped);
        if (write_line(f, drop_line) != FS_OK) {
            rc = LOG_ERR_IO;
        }
    }
    fs_close(f);

    if (rc == LOG_OK) {
        s_head = 0;
        s_count = 0;
        s_dropped = 0;
    }
    return rc;
}
