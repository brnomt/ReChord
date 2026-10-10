/*
 * log.h — ReChord ring-buffered logger.
 *
 * Design (why): on the device, writing to storage is slow and can happen only
 * when the FS is mounted, so log_printf() only formats into an in-RAM ring
 * buffer. log_flush(tag) appends a marker line and writes everything to the
 * log file in one go. When the ring is full, new lines are dropped and
 * counted; the drop count is reported in the log at the next flush.
 *
 * Line format (documented on-device log format fact):
 *   "%lu.%03lu %s"  = seconds.milliseconds since boot, space, message
 *   e.g. "1.034 pll: switching to 200 MHz"
 *
 * Clock: the board layer injects a millisecond tick via log_init(); without
 * it, timestamps stay at 0.000 (deterministic for host tests).
 */
#ifndef RECHORD_SERVICES_LOG_H
#define RECHORD_SERVICES_LOG_H

#include <stdarg.h>

/* Log file location: root of the user volume (drop-in name for tooling that
 * pulls the device log). Override at compile time if needed. */
#ifndef LOG_PATH
#define LOG_PATH "\\ECHOLOG.TXT"
#endif

/* Ring capacity (lines). Tune per build; keep small for the 64 KiB-class RAM
 * windows we run services in. */
#ifndef LOG_BUFFER_LINES
#define LOG_BUFFER_LINES 32
#endif

#ifndef LOG_LINE_MAX
#define LOG_LINE_MAX 160
#endif

/* Return codes for log_flush(). */
#define LOG_OK        0
#define LOG_ERR_ARG (-1)
#define LOG_ERR_OPEN (-2)   /* log file could not be opened; buffer kept */
#define LOG_ERR_IO  (-3)   /* write failed; buffer kept */

/* Millisecond clock source: returns ms since boot. */
typedef unsigned long (*log_clock_fn)(void);

/* Install the ms clock source (NULL = frozen at 0.000). */
void log_init(log_clock_fn clock_ms);

/* Format one timestamped line into the ring buffer. Overflows are dropped
 * and counted (see log_dropped()). */
#if defined(__GNUC__)
void log_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#else
void log_printf(const char *fmt, ...);
#endif

/* Append "log flush (<tag>)" (skipped when tag is NULL), then write the whole
 * buffer to LOG_PATH in append mode and reset the buffer and drop counter.
 * On failure the buffer is kept for the next attempt. Returns LOG_OK or an
 * error code. */
int log_flush(const char *tag);

/* Number of lines dropped since the last successful flush. */
unsigned long log_dropped(void);

/* Number of lines currently buffered. */
unsigned log_pending(void);

#endif /* RECHORD_SERVICES_LOG_H */
