/*
 * test_log.c — host tests for the circular logger.
 *
 * Covers: exact line format "[ms][LEVEL][subsystem] msg", level filtering at
 * the producer, the 75% flush watermark + drain, whole-line drops with the
 * drop-counter report at flush, file rotation to .OLD with a fresh log
 * afterwards, and the flush-error path (ring kept for retry).
 */
#include "rechord_log.h"
#include "fs_host.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

static void files_reset(void)
{
    (void)remove(".work/RECHORD.LOG");
    (void)remove(".work/RECHORD.OLD");
}

static void read_text(const char *path, char *buf, unsigned long cap)
{
    FILE *fp = fopen(path, "rb");
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

static int file_size(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0) ? (int)st.st_size : -1;
}

/* Read the TAIL of a file (drop reports land at the end of the log). */
static void read_text_tail(const char *path, char *buf, unsigned long cap)
{
    struct stat st;
    FILE *fp = fopen(path, "rb");
    long n, skip = 0;

    CHECK(fp != NULL);
    if (fp == NULL) {
        buf[0] = '\0';
        return;
    }
    if (stat(path, &st) == 0 && st.st_size > (long)(cap - 1)) {
        skip = st.st_size - (long)(cap - 1);
    }
    CHECK(fseek(fp, skip, SEEK_SET) == 0);
    n = (long)fread(buf, 1, (size_t)(cap - 1), fp);
    CHECK(n >= 0);
    buf[(n > 0) ? n : 0] = '\0';
    (void)fclose(fp);
}

static void test_format_and_flush(void)
{
    char buf[1024];

    files_reset();
    rlog_init(mock_clock);
    rlog_set_level(RLOG_INFO);

    s_now_ms = 12345;
    rlog_printf(RLOG_INFO, "player", "hello %s", "world");
    s_now_ms = 13000;
    rlog_printf(RLOG_ERROR, "dsp", "err %d", 5);
    CHECK(rlog_pending() > 0);

    CHECK(rlog_flush() == RLOG_OK);
    CHECK(rlog_pending() == 0);
    CHECK(rlog_dropped() == 0);

    read_text(".work/RECHORD.LOG", buf, sizeof(buf));
    CHECK(strcmp(buf,
        "[12345][INFO][player] hello world\n"
        "[13000][ERROR][dsp] err 5\n") == 0);
}

static void test_level_filter(void)
{
    files_reset();
    rlog_init(mock_clock);
    rlog_set_level(RLOG_INFO);            /* default minimum */
    s_now_ms = 0;

    rlog_printf(RLOG_DEBUG, "x", "hidden");
    rlog_printf(RLOG_INFO, "x", "shown");
    CHECK(rlog_pending() ==
          (unsigned long)strlen("[0][INFO][x] shown\n"));

    rlog_set_level(RLOG_DEBUG);           /* lower the bar */
    rlog_printf(RLOG_DEBUG, "x", "now shown");
    CHECK(rlog_pending() >
          (unsigned long)strlen("[0][INFO][x] shown\n"));
    rlog_set_level(RLOG_INFO);
}

static void test_watermark_and_drain(void)
{
    char buf[64];
    int i;

    files_reset();
    rlog_init(mock_clock);
    s_now_ms = 0;

    CHECK(!rlog_should_flush());          /* empty ring */
    for (i = 0; i < 100000 && !rlog_should_flush(); i++) {
        rlog_printf(RLOG_INFO, "t", "filler line %d", i);
    }
    CHECK(rlog_should_flush());           /* >= 75% of the ring */
    CHECK(rlog_flush() == RLOG_OK);
    CHECK(rlog_pending() == 0);
    CHECK(!rlog_should_flush());          /* drained */
    CHECK(file_size(".work/RECHORD.LOG") > (int)RLOG_BUF_SIZE / 2);

    /* Second flush appends, does not truncate. */
    rlog_printf(RLOG_INFO, "t", "after drain");
    CHECK(rlog_flush() == RLOG_OK);
    read_text_tail(".work/RECHORD.LOG", buf, sizeof(buf));
    CHECK(strstr(buf, "[0][INFO][t] after drain\n") != NULL);
}

static void test_drops_are_whole_lines(void)
{
    char buf[4096];
    int i;

    files_reset();
    rlog_init(mock_clock);
    s_now_ms = 0;

    /* Burst far past capacity: ring overflows, drops counted. */
    for (i = 0; i < 200000; i++) {
        rlog_printf(RLOG_INFO, "t", "burst line %d", i);
    }
    CHECK(rlog_pending() <= (unsigned long)RLOG_BUF_SIZE);
    CHECK(rlog_dropped() > 0);
    CHECK(rlog_flush() == RLOG_OK);
    CHECK(rlog_pending() == 0);
    CHECK(rlog_dropped() == 0);           /* reported and reset */

    read_text_tail(".work/RECHORD.LOG", buf, sizeof(buf));
    CHECK(strstr(buf, "[WARN][log] ") != NULL);
    CHECK(strstr(buf, "lines dropped (ring full)") != NULL);
    /* No torn lines: every line ends with '\n'. */
    CHECK(buf[strlen(buf) - 1] == '\n');
}

static void test_rotation(void)
{
    int size_after_flush1;
    int i;

    files_reset();
    rlog_init(mock_clock);
    rlog_set_file_limit(4096);
    s_now_ms = 0;

    /* Round 1: ~2 KB, under the limit -> stays in place. */
    for (i = 0; i < 100000 && rlog_pending() < 2048u; i++) {
        rlog_printf(RLOG_INFO, "t", "round one line %d", i);
    }
    CHECK(rlog_flush() == RLOG_OK);
    size_after_flush1 = file_size(".work/RECHORD.LOG");
    CHECK(size_after_flush1 > 0);
    CHECK(file_size(".work/RECHORD.OLD") == -1);

    /* Round 2: pushes the file past 4096 -> rotated to .OLD. */
    for (i = 0; i < 100000 && rlog_pending() < 2048u; i++) {
        rlog_printf(RLOG_INFO, "t", "round two line %d", i);
    }
    CHECK(rlog_flush() == RLOG_OK);
    CHECK(file_size(".work/RECHORD.OLD") > size_after_flush1);
    CHECK(file_size(".work/RECHORD.LOG") == -1);  /* renamed away */

    /* Round 3: a fresh log starts again. */
    rlog_printf(RLOG_INFO, "t", "round three");
    CHECK(rlog_flush() == RLOG_OK);
    {
        char buf[512];
        read_text(".work/RECHORD.LOG", buf, sizeof(buf));
        CHECK(strstr(buf, "round three") != NULL);
        CHECK(strstr(buf, "round one") == NULL);
    }
    rlog_set_file_limit(RLOG_FILE_MAX_BYTES);
}

static void test_flush_error_keeps_ring(void)
{
    char good_root[512];
    unsigned long pending;

    files_reset();
    rlog_init(mock_clock);
    rlog_set_level(RLOG_INFO);
    s_now_ms = 77000;
    rlog_printf(RLOG_INFO, "t", "keepme");

    /* Copy: fs_host_set_root() rewrites the static buffer in place. */
    snprintf(good_root, sizeof(good_root), "%s", fs_host_get_root());
    fs_host_set_root(".work/nonexistent-subdir");
    CHECK(rlog_flush() == RLOG_ERR_OPEN);
    pending = rlog_pending();
    CHECK(pending > 0);                  /* nothing lost */

    fs_host_set_root(good_root);
    CHECK(rlog_flush() == RLOG_OK);
    CHECK(rlog_pending() == 0);
    {
        char buf[256];
        read_text(".work/RECHORD.LOG", buf, sizeof(buf));
        CHECK(strstr(buf, "[77000][INFO][t] keepme\n") != NULL);
    }
}

int main(void)
{
    (void)mkdir(".work", 0777);   /* fine if it already exists */
    fs_host_set_root(".work");

    test_format_and_flush();
    test_level_filter();
    test_watermark_and_drain();
    test_drops_are_whole_lines();
    test_rotation();
    test_flush_error_keeps_ring();

    printf("test_log: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
