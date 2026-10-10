/*
 * fs.h — minimal storage abstraction for the ReChord service layer.
 *
 * WHY: settings and logging must run unchanged on the host (tests) and on the
 * target (device). They talk only to this API; the backend is selected at link
 * time:
 *
 *   - fs_host.c   — POSIX backend over a real filesystem (tests, tools).
 *   - fs_target.c — TODO stub for the SDK FAT backend (device firmware).
 *
 * Exactly ONE backend is linked into any given image.
 *
 * Path convention: device-style volume-rooted paths, e.g. "\\RECHORD.CFG" or
 * "/MUSIC/track.flac". The host backend maps them under a configurable root
 * directory (fs_host_set_root), so tests run in a scratch dir.
 *
 * Return convention: functions that return a count/length return >= 0 on
 * success and a negative FS_ERR_* code on failure. Open calls return NULL on
 * failure.
 */
#include <stdint.h>

#ifndef RECHORD_SERVICES_FS_H
#define RECHORD_SERVICES_FS_H

#define FS_OK              0
#define FS_ERR_ARG        (-1)  /* NULL/invalid argument */
#define FS_ERR_OPEN       (-2)  /* file or directory not found / not openable */
#define FS_ERR_IO         (-3)  /* read/write failure */
#define FS_ERR_UNSUPPORTED (-4) /* operation not available in this backend */

/* Opaque file handle (backed by FILE* on the host, by an SDK file id on the
 * target). */
typedef struct fs_file fs_file_t;

/* Longest entry name we carry, including NUL (matches settings/path style). */
#ifndef FS_NAME_MAX
#define FS_NAME_MAX 256
#endif

/* Directory entry value type. Callers that collect entries (UI browser,
 * tests) use this struct; the callback API below stays pointer-light. */
typedef struct fs_dirent {
    char name[FS_NAME_MAX];
    int  is_dir;
} fs_dirent_t;

/* Directory entry callback for fs_list_dir.
 *   name   — entry name (no path prefix)
 *   is_dir — nonzero if the entry is a directory
 * Return nonzero to stop the iteration early. */
typedef int (*fs_dirent_cb)(const char *name, int is_dir);

/* List a directory. Returns the number of entries reported to cb (>= 0), or a
 * negative FS_ERR_* code. Entries "." and ".." are never reported. */
int fs_list_dir_each(const char *path, fs_dirent_cb cb);

/* Canonical directory listing for the browser: pagination without loading
 * the whole directory. Fills up to `max` entries from `index`, reports how
 * many were written (*out_count) and the total entry count (*out_total).
 * Returns 0 on success. (Canonical shape adopted at integration: the UI
 * browser needs random access pages; fs_list_dir_each stays as the simple
 * iteration form.) */
int fs_list_dir(const char *path, uint32_t index, fs_dirent_t *out,
                uint32_t max, uint32_t *out_count, uint32_t *out_total);

/* Open a file for reading. NULL on failure. */
fs_file_t *fs_open_read(const char *path);

/* Read up to <cap> bytes. Returns bytes read (0 at EOF) or FS_ERR_*. */
long fs_read(fs_file_t *f, void *buf, unsigned long cap);

/* Total size of the open file in bytes, or FS_ERR_*. */
long fs_file_size(fs_file_t *f);

/* Close and release a handle opened by any fs_open_* call. NULL is ignored. */
void fs_close(fs_file_t *f);

/* Open a file for writing, creating or truncating it. NULL on failure. */
fs_file_t *fs_open_write(const char *path);

/* Open a file for appending, creating it if needed. NULL on failure. */
fs_file_t *fs_open_append(const char *path);

/* Write <len> bytes. Returns bytes written or FS_ERR_*. */
long fs_write(fs_file_t *f, const void *buf, unsigned long len);

#endif /* RECHORD_SERVICES_FS_H */
