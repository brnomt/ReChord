/*
 * fs_host.c — POSIX host backend of the fs abstraction (tests, tools).
 *
 * WHY: the service layer is developed and tested on the host against a real
 * filesystem. Device-style paths ("\\RECHORD.CFG", "/MUSIC/x") are mapped
 * under a configurable root directory so tests never touch real data.
 */
#include "fs.h"
#include "fs_host.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Directory iteration needs POSIX dirent. Bare-metal toolchains (the
 * arm-none-eabi newlib used for the cross-compile check) do not provide it,
 * so fs_list_dir_each() degrades to FS_ERR_UNSUPPORTED there. This file is only
 * ever LINKED into host binaries; the check just keeps the tree uniformly
 * cross-compilable. */
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define FS_HOST_HAVE_DIRENT 1
#include <dirent.h>
#include <sys/stat.h>
#endif

#define FS_HOST_PATH_MAX 512

struct fs_file {
    FILE *fp;
    long size;
};

/* Root directory that device paths resolve under. "." by default. */
static char s_root[FS_HOST_PATH_MAX] = ".";

void fs_host_set_root(const char *dir)
{
    unsigned long n;

    if (dir == NULL || dir[0] == '\0') {
        strcpy(s_root, ".");
        return;
    }
    n = (unsigned long)strlen(dir);
    if (n >= (unsigned long)sizeof(s_root)) {
        n = (unsigned long)sizeof(s_root) - 1;
    }
    memcpy(s_root, dir, n);
    s_root[n] = '\0';
    /* Strip trailing separators so joins stay clean. */
    while (n > 1 && (s_root[n - 1] == '/' || s_root[n - 1] == '\\')) {
        s_root[--n] = '\0';
    }
}

const char *fs_host_get_root(void)
{
    return s_root;
}

/* Map a device-style path to a host path under the root directory.
 * Returns 0 on success, FS_ERR_ARG if the input is unusable. */
static int map_path(const char *dev_path, char *out, unsigned long cap)
{
    unsigned long n = 0;
    unsigned long root_len;
    const char *p = dev_path;

    if (dev_path == NULL || out == NULL || cap < 2) {
        return FS_ERR_ARG;
    }

    root_len = (unsigned long)strlen(s_root);
    if (root_len >= cap - 1) {
        return FS_ERR_ARG;
    }
    memcpy(out, s_root, root_len);
    n = root_len;
    out[n++] = '/';

    /* Skip volume-root markers: device paths start at the volume root. */
    while (*p == '/' || *p == '\\') {
        p++;
    }
    while (*p != '\0') {
        char ch = *p++;
        if (ch == '\\') {
            ch = '/';          /* accept DOS-style separators */
        }
        if (n >= cap - 1) {
            return FS_ERR_ARG;
        }
        out[n++] = ch;
    }
    out[n] = '\0';
    return FS_OK;
}

int fs_list_dir_each(const char *path, fs_dirent_cb cb)
{
#if defined(FS_HOST_HAVE_DIRENT)
    char host[FS_HOST_PATH_MAX];
    DIR *d;
    struct dirent *ent;
    int count = 0;

    if (cb == NULL || map_path(path, host, (unsigned long)sizeof(host)) != FS_OK) {
        return FS_ERR_ARG;
    }
    d = opendir(host);
    if (d == NULL) {
        return FS_ERR_OPEN;
    }
    while ((ent = readdir(d)) != NULL) {
        struct stat st;
        char full[FS_HOST_PATH_MAX];
        int is_dir = 0;

        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        if (snprintf(full, sizeof(full), "%s/%s", host, ent->d_name)
                < (int)sizeof(full)) {
            if (stat(full, &st) == 0 && S_ISDIR(st.st_mode)) {
                is_dir = 1;
            }
        }
        count++;
        if (cb(ent->d_name, is_dir) != 0) {
            break;
        }
    }
    closedir(d);
    return count;
#else
    (void)path;
    (void)cb;
    return FS_ERR_UNSUPPORTED;
#endif
}

/* Shared open helper; read/write/append differ only in fopen mode. */
static fs_file_t *open_with_mode(const char *path, const char *mode)
{
    char host[FS_HOST_PATH_MAX];
    fs_file_t *f;

    if (map_path(path, host, (unsigned long)sizeof(host)) != FS_OK) {
        return NULL;
    }
    f = (fs_file_t *)malloc(sizeof(*f));
    if (f == NULL) {
        return NULL;
    }
    f->fp = fopen(host, mode);
    if (f->fp == NULL) {
        free(f);
        return NULL;
    }
    f->size = -1;
    if (fseek(f->fp, 0, SEEK_END) == 0) {
        f->size = ftell(f->fp);
        (void)fseek(f->fp, 0, SEEK_SET);
    }
    return f;
}

fs_file_t *fs_open_read(const char *path)
{
    return open_with_mode(path, "rb");
}

fs_file_t *fs_open_write(const char *path)
{
    return open_with_mode(path, "wb");
}

fs_file_t *fs_open_append(const char *path)
{
    return open_with_mode(path, "ab");
}

long fs_read(fs_file_t *f, void *buf, unsigned long cap)
{
    size_t n;

    if (f == NULL || f->fp == NULL || (buf == NULL && cap != 0)) {
        return FS_ERR_ARG;
    }
    if (cap == 0) {
        return 0;
    }
    n = fread(buf, 1, (size_t)cap, f->fp);
    if (n == 0 && ferror(f->fp)) {
        return FS_ERR_IO;
    }
    return (long)n;
}

long fs_write(fs_file_t *f, const void *buf, unsigned long len)
{
    size_t n;

    if (f == NULL || f->fp == NULL || (buf == NULL && len != 0)) {
        return FS_ERR_ARG;
    }
    if (len == 0) {
        return 0;
    }
    n = fwrite(buf, 1, (size_t)len, f->fp);
    if (n != (size_t)len) {
        return FS_ERR_IO;
    }
    return (long)n;
}

long fs_file_size(fs_file_t *f)
{
    if (f == NULL || f->fp == NULL) {
        return FS_ERR_ARG;
    }
    return f->size;
}

void fs_close(fs_file_t *f)
{
    if (f == NULL) {
        return;
    }
    if (f->fp != NULL) {
        (void)fclose(f->fp);
    }
    free(f);
}


/* Pagination listing over the host FS (see fs.h contract). */
int fs_list_dir(const char *path, uint32_t index, fs_dirent_t *out,
                uint32_t max, uint32_t *out_count, uint32_t *out_total)
{
    DIR *d = opendir(path);
    struct dirent *e;
    uint32_t i = 0, written = 0;

    if (d == NULL)
        return -1;
    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;                    /* callers never see dot entries */
        if (i >= index && written < max) {
            snprintf(out[written].name, FS_NAME_MAX, "%s", e->d_name);
            out[written].is_dir = (e->d_type == DT_DIR);
            written++;
        }
        i++;
    }
    closedir(d);
    if (out_count) *out_count = written;
    if (out_total) *out_total = i;
    return 0;
}
