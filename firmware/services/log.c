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
static log_write_fn s_writer;    /* whole-file writer; NULL = stdio append */

/* Assembly buffer for the writer path. Sized to the ring plus the drop note,
 * so it is bounded by LOG_BUFFER_LINES: on the target that constant is
 * smaller, which keeps this (and the ring) inside the .bss budget. */
static char s_text[LOG_BUFFER_LINES * LOG_LINE_MAX + 64];

void log_init(log_clock_fn clock_ms)
{
    s_clock = clock_ms;
}

void log_set_writer(log_write_fn wr)
{
    s_writer = wr;
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

/* Public ms clock (same source the timestamps use): callers that need real
 * elapsed time — not a loop-iteration count — measure with this. */
unsigned long log_now_ms(void)
{
    return now_ms();
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

/* Assemble the pending ring (and the drop note) into s_text. Returns the
 * length, or LOG_ERR_IO when it does not fit (the buffer is sized so it
 * always does). */
static int assemble(void)
{
    unsigned i, len = 0;

    for (i = 0; i < s_count; i++) {
        int n = snprintf(s_text + len, sizeof s_text - len, "%s\n",
                         s_lines[(s_head + i) % (unsigned)LOG_BUFFER_LINES]);

        if (n < 0 || (unsigned)n >= sizeof s_text - len)
            return LOG_ERR_IO;
        len += (unsigned)n;
    }
    if (s_dropped > 0) {
        unsigned long ms = now_ms();
        int n = snprintf(s_text + len, sizeof s_text - len,
                         "%lu.%03lu log: %lu lines dropped (buffer full)\n",
                         ms / 1000UL, ms % 1000UL, s_dropped);

        if (n < 0 || (unsigned)n >= sizeof s_text - len)
            return LOG_ERR_IO;
        len += (unsigned)n;
    }
    return (int)len;
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

    if (s_writer != NULL) {
        /* Target path: one whole-file rewrite (the log file lives in the
         * device's own folder and is created at first boot). */
        int len = assemble();

        if (len < 0)
            return LOG_ERR_IO;           /* keep the buffer for the retry */
        if (s_writer(LOG_PATH, s_text, (unsigned)len) != 0)
            return LOG_ERR_IO;
        s_head = 0;
        s_count = 0;
        s_dropped = 0;
        return LOG_OK;
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
