/*
 * test_fs.c — host smoke tests for the fs abstraction (fs_host.c backend).
 *
 * Covers: device-style path mapping, open/read/write/append/size/close and
 * fs_list_dir (skips "." / "..", reports is_dir).
 */
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

static int s_files;
static int s_dirs;

static int count_cb(const char *name, int is_dir)
{
    (void)name;
    if (is_dir) {
        s_dirs++;
    } else {
        s_files++;
    }
    return 0;                   /* keep iterating */
}

static int stop_early_cb(const char *name, int is_dir)
{
    (void)name;
    (void)is_dir;
    return 1;                   /* stop after the first entry */
}

/* Whole-file writes by path (the index builder's primitives): append grows
 * the file, patch rewrites inside it without changing its size. */
static void test_append_patch(void)
{
    static char buf[64];
    fs_file_t *f;
    long n;

    (void)remove(".work/APT.TXT");
    f = fs_open_write("\\APT.TXT");
    CHECK(f != NULL);
    if (f == NULL) {
        return;
    }
    CHECK(fs_write(f, "abc", 3) == 3);
    fs_close(f);

    CHECK(fs_append("\\APT.TXT", "def", 3) == FS_OK);
    CHECK(fs_append("\\APT.TXT", "", 0) == FS_OK);      /* no-op */
    CHECK(fs_patch("\\APT.TXT", 2, "XY", 2) == FS_OK);
    CHECK(fs_patch("\\APT.TXT", 0, "", 0) == FS_OK);

    f = fs_open_read("\\APT.TXT");
    CHECK(f != NULL);
    if (f != NULL) {
        CHECK(fs_file_size(f) == 6);
        n = fs_read(f, buf, sizeof buf);
        CHECK(n == 6 && memcmp(buf, "abXYef", 6) == 0);
        fs_close(f);
    }
    /* A missing file is refused, and the host backend does not invent one. */
    CHECK(fs_patch("\\NOPE.TXT", 0, "x", 1) == FS_ERR_OPEN);
}

int main(void)
{
    fs_file_t *f;
    char buf[64];
    long n;

    if (mkdir(".work", 0777) != 0 && errno != EEXIST) {
        printf("FAIL cannot create .work\n");
        return 1;
    }
    fs_host_set_root(".work");

    /* write + read back through a DOS-style device path */
    f = fs_open_write("\\HELLO.TXT");
    CHECK(f != NULL);
    CHECK(fs_write(f, "abc123", 6) == 6);
    fs_close(f);

    f = fs_open_read("\\HELLO.TXT");
    CHECK(f != NULL);
    CHECK(fs_file_size(f) == 6);
    n = fs_read(f, buf, sizeof(buf));
    CHECK(n == 6);
    buf[n] = '\0';
    CHECK(strcmp(buf, "abc123") == 0);
    fs_close(f);

    /* append grows, does not truncate */
    f = fs_open_append("/HELLO.TXT");   /* slash form maps to the same file */
    CHECK(f != NULL);
    CHECK(fs_write(f, "789", 3) == 3);
    fs_close(f);
    f = fs_open_read("\\HELLO.TXT");
    CHECK(f != NULL);
    CHECK(fs_file_size(f) == 9);
    fs_close(f);

    /* errors */
    CHECK(fs_open_read("\\MISSING.TXT") == NULL);
    CHECK(fs_read(NULL, buf, 4) == FS_ERR_ARG);
    CHECK(fs_write(NULL, buf, 4) == FS_ERR_ARG);
    CHECK(fs_file_size(NULL) == FS_ERR_ARG);
    fs_close(NULL);                      /* must not crash */
    CHECK(fs_list_dir_each("\\", NULL) == FS_ERR_ARG);
    CHECK(fs_list_dir_each("\\MISSINGDIR", count_cb) == FS_ERR_OPEN);

    /* list: a file and a subdirectory */
    if (mkdir(".work/sub", 0777) != 0 && errno != EEXIST) {
        printf("FAIL cannot create .work/sub\n");
        return 1;
    }
    s_files = 0;
    s_dirs = 0;
    CHECK(fs_list_dir_each("\\", count_cb) >= 2);
    CHECK(s_files >= 1);
    CHECK(s_dirs >= 1);
    CHECK(fs_list_dir_each("\\", stop_early_cb) == 1);

    test_append_patch();

    printf("test_fs: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
