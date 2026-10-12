/*
 * codec_io_fs.c — codec_io backend backed by the firmware fs abstraction
 * (fs.h).  One of PLAYER_CODEC_IO_IMPL in player.mk.
 *
 * Why this file exists: the player decodes from files on the device
 * (eMMC "Internal", microSD "SD"). The host tests + the current target
 * path use codec_io_stdio.c (fopen). On the device codecs that want to
 * seek should walk the fs_target FAT/exFAT layer, not stdio FILE*. This
 * backend maps the codec_io seam onto fs_open_read / fs_read / fs_file_size /
 * fs_close / fs_seek.
 *
 * Link contract: this file supplies `cio_open` (+ the static vtable). The
 * shared dispatch wrappers cio_read/seek/tell/size/close live in
 * codec_io.c and are linked once. Ownership: cio_open allocates the io
 * handle (codec_io_t) and the ctx; s_close frees the ctx only; cio_close
 * (codec_io.c) frees the io handle.
 *
 * Freestanding-friendly: small helpers only, no libc string dependency, so
 * it compiles under the same -ffreestanding flags the SDK uses.
 */
#include "codec_io.h"
#include "fs.h"

#include <stdlib.h>

typedef struct
{
    fs_file_t *f;
    long       size;   /* cached fs_file_size(); -1 if unknown            */
    long       pos;    /* last known position for tell()                  */
} fs_ctx_t;

static long s_read(void *ctx, void *buf, long len)
{
    fs_ctx_t *c = (fs_ctx_t *)ctx;
    long n;

    if (c == NULL || c->f == NULL || buf == NULL || len <= 0)
        return -1;
    n = fs_read(c->f, buf, (unsigned long)len);
    if (n < 0)
        return -1;   /* fs_read returns FS_ERR_*; normalize to codec_io -1 */
    c->pos += n;
    return n;   /* 0 == EOF, which is what decoders expect */
}

static long s_seek(void *ctx, long offset)
{
    fs_ctx_t *c = (fs_ctx_t *)ctx;
    long r;

    if (c == NULL || c->f == NULL || offset < 0)
        return -1;
    r = fs_seek(c->f, offset);
    if (r < 0)
        return -1;
    c->pos = r;
    return r;
}

static long s_tell(void *ctx)
{
    fs_ctx_t *c = (fs_ctx_t *)ctx;

    if (c == NULL || c->f == NULL)
        return -1;
    return c->pos;
}

static long s_size(void *ctx)
{
    fs_ctx_t *c = (fs_ctx_t *)ctx;

    if (c == NULL || c->f == NULL)
        return -1;
    if (c->size == -1) {
        c->size = fs_file_size(c->f);
        if (c->size < 0)
            return -1;
    }
    return c->size;
}

static void s_close(void *ctx)
{
    fs_ctx_t *c = (fs_ctx_t *)ctx;

    if (c == NULL)
        return;
    if (c->f != NULL)
        fs_close(c->f);
    free(c);   /* ctx only; cio_close (codec_io.c) frees the io handle   */
}

codec_io_t *cio_open(const char *path)
{
    fs_ctx_t *c;
    codec_io_t *io;

    if (path == NULL || path[0] == '\0')
        return NULL;

    c = (fs_ctx_t *)malloc(sizeof(*c));
    if (c == NULL)
        return NULL;
    c->f = NULL;
    c->size = -1;
    c->pos = 0;

    c->f = fs_open_read(path);
    if (c->f == NULL) {
        free(c);
        return NULL;
    }
    /* Probe the size eagerly so decoders that need it (WAV chunk walking,
     * MP3 Xing/Info scan, FLAC STREAMINFO totals) get a fast answer and so
     * a failed size() surfaces as an open error rather than a mid-decode
     * surprise. */
    c->size = fs_file_size(c->f);
    if (c->size < 0) {
        fs_close(c->f);
        free(c);
        return NULL;
    }

    io = (codec_io_t *)malloc(sizeof(*io));
    if (io == NULL) {
        fs_close(c->f);
        free(c);
        return NULL;
    }
    io->ctx = c;
    io->read = s_read;
    io->seek = s_seek;
    io->tell = s_tell;
    io->size = s_size;
    io->close = s_close;
    return io;
}
