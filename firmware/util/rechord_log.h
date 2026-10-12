/*
 * rechord_log.h — circular in-memory logger with leveled output, non-blocking
 * flush and file rotation for ReChord.
 *
 * WHY this exists next to services/log.c: the stock logger drops lines at 32
 * buffered lines, has no levels and no rotation. This module is the ReChord
 * replacement: a byte-granular ring (4 KB by default, 8 KB-capable) sized to
 * absorb log bursts — including from the audio path — without ever touching
 * storage on the producer side.
 *
 * Context rules (bare-metal contract):
 *   - rlog_printf() may be called from ANY single context, including the
 *     audio ISR: it only formats to a small stack scratch and memcpy()s into
 *     the ring (single-producer; commit after copy). No locks, no I/O.
 *     If two contexts log concurrently, the caller must serialize them.
 *   - rlog_flush() does storage I/O: MAIN-LOOP context only, never an ISR.
 *     Call it when idle (rlog_should_flush() or a periodic timer).
 *
 * Line format (documented on-device convention, extended with level):
 *   "[<ms>][<LEVEL>][<subsystem>] <message>\n"
 *   e.g. "[12345][WARN][player] underrun, 2304 samples late\n"
 *
 * Drop policy: a line that does not fit in the free ring space is dropped as
 * a unit (no torn lines) and counted; the count is reported in the log at
 * the next successful flush.
 *
 * Rotation: after a flush, if the log file has grown past the limit
 * (256 KiB default, runtime-adjustable) it is renamed to "<path>.old"
 * (previous .old is deleted first). The size counter survives reboots by
 * stat-ing the file on the first flush.
 */
#ifndef RECHORD_UTIL_RECHORD_LOG_H
#define RECHORD_UTIL_RECHORD_LOG_H

#include <stdarg.h>

/* Ring size in bytes (power of two not required). The default is 4 KiB to
 * fit the tight AP RAM window; builds with more room (or the BB half) can
 * raise it to 8 KiB with -DRLOG_BUF_SIZE=8192. */
#ifndef RLOG_BUF_SIZE
#define RLOG_BUF_SIZE 4096u
#endif

/* Longest formatted line, NUL excluded (message is truncated to fit). */
#ifndef RLOG_LINE_MAX
#define RLOG_LINE_MAX 160u
#endif

/* Log file locations (volume-rooted, DOS-style; see services/fs.h). Kept
 * distinct from the stock ECHOLOG.TXT that services/log.c writes. */
#ifndef RLOG_PATH
#define RLOG_PATH "\\RECHORD.LOG"
#endif
#ifndef RLOG_OLD_PATH
#define RLOG_OLD_PATH "\\RECHORD.OLD"
#endif

/* Rotate once the file exceeds this size. Runtime-adjustable via
 * rlog_set_file_limit(). */
#ifndef RLOG_FILE_MAX_BYTES
#define RLOG_FILE_MAX_BYTES (256UL * 1024UL)
#endif

typedef enum {
    RLOG_DEBUG = 0,
    RLOG_INFO,
    RLOG_WARN,
    RLOG_ERROR
} rlog_level_t;

/* Return codes for rlog_flush(). */
#define RLOG_OK        0
#define RLOG_ERR_OPEN (-2)  /* log file could not be opened; ring kept */
#define RLOG_ERR_IO   (-3)  /* write failed; unwritten part of ring kept */

/* Millisecond clock source: returns ms since boot. */
typedef unsigned long (*rlog_clock_fn)(void);

/* Install the ms clock source (NULL = frozen at 0). */
void rlog_init(rlog_clock_fn clock_ms);

/* Minimum level emitted (default RLOG_INFO; lower levels are dropped at the
 * producer, before they touch the ring). */
void rlog_set_level(rlog_level_t min_level);

/* Rotation limit in bytes (0 = disable rotation). */
void rlog_set_file_limit(unsigned long bytes);

/* Override the log/rotation paths (NULL keeps the current value). */
void rlog_set_paths(const char *log_path, const char *old_path);

/* Format one leveled line into the ring. Safe from any single context
 * (including an audio ISR); never touches storage. */
#if defined(__GNUC__)
void rlog_printf(rlog_level_t level, const char *subsystem, const char *fmt,
                 ...) __attribute__((format(printf, 3, 4)));
#else
void rlog_printf(rlog_level_t level, const char *subsystem, const char *fmt,
                 ...);
#endif

/* Bytes currently buffered in the ring. */
unsigned long rlog_pending(void);

/* Lines dropped since the last successful flush. */
unsigned long rlog_dropped(void);

/* Watermark test (>= 75% of the ring): the main loop polls this and calls
 * rlog_flush() when it returns nonzero. */
int rlog_should_flush(void);

/* Append the ring contents to the log file, rotating first if the file is
 * over the limit. MAIN-LOOP CONTEXT ONLY. On I/O failure the unwritten part
 * of the ring is kept for the next attempt. Returns RLOG_OK or an error. */
int rlog_flush(void);

#endif /* RECHORD_UTIL_RECHORD_LOG_H */
