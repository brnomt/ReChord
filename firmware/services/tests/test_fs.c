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

    printf("test_fs: %d checks, %d failures\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
