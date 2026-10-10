/*
 * test_log.c — host tests for the ring-buffered logger.
 *
 * Covers: "%lu.%03lu %s" line format, flush-with-tag marker, append-to-file
 * behavior, ring overflow with drop counter reporting, and the flush error
 * path (buffer kept for retry).
 */
#include "log.h"
#include "fs.h"
#include "fs_host.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

/* Deterministic clock for timestamps. */
static unsigned long s_now_ms;
static unsigned long mock_clock(void)
{
    return s_now_ms;
}

static void log_file_reset(void)
{
    (void)remove(".work/ECHOLOG.TXT");
}

static void read_log(char *buf, unsigned long cap)
{
    FILE *fp = fopen(".work/ECHOLOG.TXT", "rb");
    long n;

    CHECK(fp != NULL);
    if (fp == NULL) {
        buf[0] = '\0';
        return;
    }
    n = (long)fread(buf, 1, (size_t)cap - 1, fp);
    CHECK(n >= 0);
    buf[(n > 0) ? n : 0] = '\0';
    (void)fclose(fp);
}

static int count_lines(const char *text)
{
    int n = 0;
    for (; *text != '\0'; text++) {
        if (*text == '\n') {
            n++;
        }
    }
    return n;
}

static void test_format_and_flush_tag(void)
{
    char buf[1024];

    log_file_reset();
    log_init(mock_clock);

    s_now_ms = 12345;
    log_printf("hello %s", "world");
    s_now_ms = 13000;
    log_printf("second %d", 2);
    CHECK(log_pending() == 2);
    CHECK(log_dropped() == 0);

    CHECK(log_flush("boot") == LOG_OK);
    CHECK(log_pending() == 0);
    CHECK(log_dropped() == 0);

    read_log(buf, sizeof(buf));
    /* Exact line format: seconds.milliseconds + space + message. */
    CHECK(strcmp(buf,
        "12.345 hello world\n"
        "13.000 second 2\n"
        "13.000 log flush (boot)\n") == 0);
}

static void test_append(void)
{
    char buf[1024];

    s_now_ms = 14000;
    log_printf("appended");
    CHECK(log_flush(NULL) == LOG_OK);   /* NULL tag: no marker line */

    read_log(buf, sizeof(buf));
    CHECK(strstr(buf, "14.000 appended\n") != NULL);
    CHECK(strstr(buf, "12.345 hello world\n") != NULL);   /* not truncated */
    CHECK(count_lines(buf) == 4);
}

static void test_overflow(void)
{
    char buf[4096];
    char expect[64];
    int i;

    log_file_reset();
    log_init(mock_clock);
    s_now_ms = 0;

    /* Fill the ring past capacity. */
    for (i = 0; i < LOG_BUFFER_LINES + 5; i++) {
        log_printf("msg %d", i);
    }
    CHECK(log_pending() == (unsigned)LOG_BUFFER_LINES);
    CHECK(log_dropped() == 5);

    CHECK(log_flush(NULL) == LOG_OK);
    CHECK(log_pending() == 0);
    CHECK(log_dropped() == 0);

    read_log(buf, sizeof(buf));
    /* 32 buffered lines + 1 drop-counter line. */
    CHECK(count_lines(buf) == LOG_BUFFER_LINES + 1);
    CHECK(strstr(buf, "0.000 msg 0\n") != NULL);
    snprintf(expect, sizeof(expect), "0.000 msg %d\n", LOG_BUFFER_LINES - 1);
    CHECK(strstr(buf, expect) != NULL);
    CHECK(strstr(buf, "msg 32") == NULL);          /* dropped, not written */
    CHECK(strstr(buf, "0.000 log: 5 lines dropped (buffer full)\n") != NULL);
}

static void test_flush_error_keeps_buffer(void)
{
    char buf[1024];
    char good_root[512];

    /* Copy: fs_host_set_root() rewrites the static buffer in place. */
    snprintf(good_root, sizeof(good_root), "%s", fs_host_get_root());

    log_file_reset();
    log_init(mock_clock);
    s_now_ms = 77000;
    log_printf("keepme");

    fs_host_set_root(".work/nonexistent-subdir");
    CHECK(log_flush("boot") == LOG_ERR_OPEN);
    /* Marker was buffered before the open failed; nothing is lost. */
    CHECK(log_pending() == 2);
    CHECK(log_dropped() == 0);

    fs_host_set_root(good_root);
    CHECK(log_flush(NULL) == LOG_OK);
    CHECK(log_pending() == 0);

    read_log(buf, sizeof(buf));
    CHECK(strcmp(buf,
        "77.000 keepme\n"
        "77.000 log flush (boot)\n") == 0);
}

static void test_args(void)
{
    log_printf(NULL);                   /* must not crash */
    CHECK(1);
}

int main(void)
{
    if (mkdir(".work", 0777) != 0 && errno != EEXIST) {
        printf("FAIL cannot create .work\n");
        return 1;
    }
    fs_host_set_root(".work");

    test_format_and_flush_tag();
    test_append();
    test_overflow();
    test_flush_error_keeps_buffer();
    test_args();

    printf("test_log: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
