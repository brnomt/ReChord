/*
 * codec_io.c — the codec_io convenience wrappers (the dispatch layer).
 *
 * Split out from the backend TUs so exactly one backend (codec_io_stdio.c
 * on the host/newlib, codec_io_fs.c on the target) supplies cio_open(), and
 * this file supplies the single, backend-agnostic cio_read/seek/tell/size/
 * close dispatch. Only this file + one backend is linked per image
 * (player.mk: PLAYER_CODEC_IO_IMPL).
 */
#include "codec_io.h"

#include <stddef.h>

void cio_close(codec_io_t *io)
{
    if (io == NULL)
        return;
    if (io->close != NULL)
        io->close(io->ctx);
    /* The backend owns freeing its own io handle; nothing here. */
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
