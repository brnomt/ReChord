/*
 * fs_target.c — TODO stub for the SDK FAT backend (device firmware).
 *
 * WHY this file exists: the service layer links exactly one fs backend. On the
 * target this stub is linked until the real backend lands, so a missing
 * integration fails loudly (every call returns FS_ERR_UNSUPPORTED / NULL)
 * instead of corrupting anything.
 *
 * TODO(services): implement on top of the RKnanoD SDK file API
 * (firmware/rockchip/system/fileseek + the FAT stack the SDK provides, or the
 * in-tree FAT driver once it exists). Contract to satisfy:
 *   - volume-rooted paths like "\\RECHORD.CFG" resolve on the user volume
 *     (the volume the IDB parser reports as user_base, see
 *     docs/re/route-b-minimum.md "Storage" open contract),
 *   - fs_open_append must append to ECHOLOG.TXT without truncating,
 *   - fs_list_dir reports names only (no "." / "..").
 * This stub is written from the fs.h contract alone — no third-party code.
 */
#include "fs.h"

int fs_list_dir_each(const char *path, fs_dirent_cb cb)
{
    (void)path;
    (void)cb;
    return FS_ERR_UNSUPPORTED;
}

fs_file_t *fs_open_read(const char *path)
{
    (void)path;
    return (fs_file_t *)0;
}

fs_file_t *fs_open_write(const char *path)
{
    (void)path;
    return (fs_file_t *)0;
}

fs_file_t *fs_open_append(const char *path)
{
    (void)path;
    return (fs_file_t *)0;
}

long fs_read(fs_file_t *f, void *buf, unsigned long cap)
{
    (void)f;
    (void)buf;
    (void)cap;
    return FS_ERR_UNSUPPORTED;
}

long fs_write(fs_file_t *f, const void *buf, unsigned long len)
{
    (void)f;
    (void)buf;
    (void)len;
    return FS_ERR_UNSUPPORTED;
}

long fs_file_size(fs_file_t *f)
{
    (void)f;
    return FS_ERR_UNSUPPORTED;
}

void fs_close(fs_file_t *f)
{
    (void)f;
}


/* TODO(SDK FAT): pagination listing on target; same contract as fs.h. */
int fs_list_dir(const char *path, uint32_t index, fs_dirent_t *out,
                uint32_t max, uint32_t *out_count, uint32_t *out_total)
{
    (void)path; (void)index; (void)out; (void)max;
    if (out_count) *out_count = 0;
    if (out_total) *out_total = 0;
    return -1;   /* not implemented until the FAT backend lands */
}
