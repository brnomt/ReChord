/*
 * file_io.c — whole-file helpers shared by every small text file.
 *
 * Only the two calls that can be written against the generic read/write
 * primitives live here: fs_load_file() and fs_store_file(). fs_size_file()
 * and fs_ensure_file() need a backend primitive (a path size without a handle,
 * and the create path), so each backend implements them (fs_target.c /
 * fs_host.c) behind the same fs.h declarations.
 *
 * This TU is what keeps settings, resume, Bluetooth and the log from each
 * re-implementing open/read/close and truncate/append.
 */
#include <string.h>

#include "fs.h"
#include "fs_target.h"

int fs_load_file(const char *path, void *buf, unsigned long cap)
{
    fs_file_t *f;
    long got;

    if (path == 0 || buf == 0 || cap < 2)
        return FS_ERR_ARG;
    f = fs_open_read(path);
    if (f == 0)
        return FS_ERR_OPEN;
    got = fs_read(f, buf, cap - 1u);
    fs_close(f);
    if (got < 0)
        return FS_ERR_IO;
    ((char *)buf)[got] = '\0';
    return (int)got;
}

int fs_store_file(const char *path, const void *data, uint32_t len)
{
    if (path == 0 || (data == 0 && len > 0))
        return FS_ERR_ARG;
    return fs_target_file_rewrite(path, data, len);
}
