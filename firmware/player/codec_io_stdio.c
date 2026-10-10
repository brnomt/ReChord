/*
 * codec_io_stdio.c — codec_io backend on stdio (host tests + newlib target).
 *
 * Host: works against the real filesystem out of the box — the WAV golden
 * test synthesizes a .wav and decodes it through this backend.
 *
 * Target TODO (register-level bring-up): either
 *   (a) newlib syscalls in firmware/stubs.c route fopen onto the storage
 *       driver (then this file is reused unchanged), or
 *   (b) a services/fs-backed backend replaces this TU behind codec_io.h.
 * Decision is deferred to the storage workstream; codecs do not care.
 */
#include <stdio.h>
#include <stdlib.h>

#include "codec_io.h"

typedef struct
{
    FILE *f;
} stdio_ctx_t;

static long s_read(void *ctx, void *buf, long len)
{
    stdio_ctx_t *c = (stdio_ctx_t *)ctx;
    size_t n;

    if (c == NULL || c->f == NULL || buf == NULL || len <= 0)
        return -1;
    n = fread(buf, 1, (size_t)len, c->f);
    return (long)n;   /* 0 == EOF, which is what decoders expect */
}

static long s_seek(void *ctx, long offset)
{
    stdio_ctx_t *c = (stdio_ctx_t *)ctx;

    if (c == NULL || c->f == NULL || offset < 0)
        return -1;
    if (fseek(c->f, offset, SEEK_SET) != 0)
        return -1;
    return offset;
}

static long s_tell(void *ctx)
{
    stdio_ctx_t *c = (stdio_ctx_t *)ctx;
    long pos;

    if (c == NULL || c->f == NULL)
        return -1;
    pos = ftell(c->f);
    return pos;
}

static long s_size(void *ctx)
{
    stdio_ctx_t *c = (stdio_ctx_t *)ctx;
    long pos, end;

    if (c == NULL || c->f == NULL)
        return -1;
    pos = ftell(c->f);
    if (pos < 0)
        return -1;
    if (fseek(c->f, 0, SEEK_END) != 0)
        return -1;
    end = ftell(c->f);
    if (fseek(c->f, pos, SEEK_SET) != 0)
        return -1;
    return end;
}

static void s_close(void *ctx)
{
    stdio_ctx_t *c = (stdio_ctx_t *)ctx;

    if (c == NULL)
        return;
    if (c->f != NULL)
        fclose(c->f);
    free(c);
}

codec_io_t *cio_open(const char *path)
{
    stdio_ctx_t *c;
    codec_io_t *io;

    if (path == NULL)
        return NULL;

    c = (stdio_ctx_t *)malloc(sizeof(*c));
    io = (codec_io_t *)malloc(sizeof(*io));
    if (c == NULL || io == NULL) {
        free(c);
        free(io);
        return NULL;
    }

    c->f = fopen(path, "rb");
    if (c->f == NULL) {
        free(c);
        free(io);
        return NULL;
    }

    io->ctx  = c;
    io->read = s_read;
    io->seek = s_seek;
    io->tell = s_tell;
    io->size = s_size;
    io->close = s_close;
    return io;
}

void cio_close(codec_io_t *io)
{
    if (io == NULL)
        return;
    if (io->close != NULL)
        io->close(io->ctx);
    free(io);
}

long cio_read(codec_io_t *io, void *buf, long len)
{
    if (io == NULL || io->read == NULL)
        return -1;
    return io->read(io->ctx, buf, len);
}

long cio_seek(codec_io_t *io, long offset)
{
    if (io == NULL || io->seek == NULL)
        return -1;
    return io->seek(io->ctx, offset);
}

long cio_tell(codec_io_t *io)
{
    if (io == NULL || io->tell == NULL)
        return -1;
    return io->tell(io->ctx);
}

long cio_size(codec_io_t *io)
{
    if (io == NULL || io->size == NULL)
        return -1;
    return io->size(io->ctx);
}
