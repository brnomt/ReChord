/*
 * test_settings_store.c — host test for the TARGET settings backend
 * (settings_store.c) with a stubbed read path and rewrite hook.
 *
 * The FAT-level in-place write itself is verified against real FAT16/FAT32
 * images (with fsck) by tools/test_fs_target.py; this test covers the glue:
 * defaults on a missing file, parsing what the read path returns, the
 * serialize → fs_target_file_rewrite() hand-off, and the error mapping.
 */
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "fs_target.h"
#include "settings.h"

static int s_checks;
static int s_failures;

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

/* ---- stub file + rewrite hook ------------------------------------------- */

struct fs_file {
    long pos;
};

static struct fs_file s_f;
static char s_content[2048];
static long s_size;
static int  s_present;
static char s_written[2048];
static long s_written_len = -1;
static int  s_rewrite_rc;

fs_file_t *fs_open_read(const char *path)
{
    if (!s_present || path == 0 || strcmp(path, SETTINGS_PATH) != 0)
        return 0;
    s_f.pos = 0;
    return &s_f;
}

long fs_read(fs_file_t *f, void *buf, unsigned long cap)
{
    long n = s_size - f->pos;

    if (n > (long)cap)
        n = (long)cap;
    if (n <= 0)
        return 0;
    memcpy(buf, s_content + f->pos, (size_t)n);
    f->pos += n;
    return n;
}

void fs_close(fs_file_t *f)
{
    (void)f;
}

int fs_target_file_rewrite(const char *path, const void *data, uint32_t len)
{
    (void)path;
    if (s_rewrite_rc != 0)
        return s_rewrite_rc;
    memcpy(s_written, data, len);
    s_written_len = (long)len;
    return FS_OK;
}

/* ---- tests -------------------------------------------------------------- */

int main(void)
{
    settings_t s, want;
    char expect[768];
    int n, custom_volume;

    printf("== settings: target store backend ==\n");

    /* 1. missing file -> defaults, OPEN error */
    s_present = 0;
    memset(&s, 0, sizeof s);
    CHECK(settings_load(&s) == SETTINGS_ERR_OPEN);
    settings_defaults(&want);
    CHECK(memcmp(&s, &want, sizeof s) == 0);

    /* 2. present file -> parsed values */
    n = snprintf(s_content, sizeof s_content,
                 "# RECHORD.CFG\n"
                 "brightness=3\n"
                 "volume=-42\n"
                 "play=shuffle\n"
                 "fade=150\n"
                 "replaygain=album\n"
                 "unknown=ignored\n");
    s_size = n;
    s_present = 1;
    CHECK(settings_load(&s) == SETTINGS_OK);
    CHECK(s.brightness == 3);
    CHECK(s.volume == -42);
    CHECK(strcmp(s.play, "shuffle") == 0);
    CHECK(strcmp(s.fade, "150") == 0);
    CHECK(strcmp(s.replaygain, "album") == 0);

    /* 3. save -> exactly the serialized text reaches the rewrite hook */
    custom_volume = s.volume;
    s_written_len = -1;
    s_rewrite_rc = 0;
    CHECK(settings_save(&s) == SETTINGS_OK);
    n = settings_serialize_text(&s, expect, sizeof expect);
    CHECK(n > 0);
    CHECK(s_written_len == (long)n);
    CHECK(memcmp(s_written, expect, (size_t)n) == 0);
    CHECK(custom_volume == -42);

    /* 4. rewrite failure (missing/read-only/exFAT/too small) -> IO error */
    s_written_len = -1;
    s_rewrite_rc = FS_ERR_UNSUPPORTED;
    CHECK(settings_save(&s) == SETTINGS_ERR_IO);
    CHECK(s_written_len == -1);
    s_rewrite_rc = 0;

    /* 5. arguments */
    CHECK(settings_load(NULL) == SETTINGS_ERR_ARG);
    CHECK(settings_save(NULL) == SETTINGS_ERR_ARG);

    if (s_failures == 0)
        printf("test_settings_store: %d checks, OK\n", s_checks);
    else
        printf("test_settings_store: %d checks, %d FAILURES\n", s_checks,
               s_failures);
    return (s_failures == 0) ? 0 : 1;
}
