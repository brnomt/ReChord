/*
 * codec_io_fs.c — codec_io backend backed by the firmware fs abstraction
 * (fs.h).  One of PLAYER_CODEC_IO_IMPL in player.mk.
 *
 * Why this file exists: the player decodes from files on the device
 * (eMMC "Internal", microSD "SD"). The host tests use codec_io_stdio.c
 * (fopen). On the device codecs must walk the fs_target FAT/exFAT layer,
 * not stdio FILE*. This backend maps the codec_io seam onto fs_open_read /
 * fs_read / fs_file_size / fs_close / fs_seek.
 *
 * No heap: cio_open() hands out a slot from a fixed static pool (same
 * convention as the rest of the module — the AP links -nostdlib, no
 * malloc/free exists there). Slots match fs_target.c's 4 read handles, so
 * the pool can never outlive the FS layer's own capacity.
 *
 * Link contract: this file supplies `cio_open` (+ the static vtable). The
 * shared dispatch wrappers cio_read/seek/tell/size/close live in
 * codec_io.c and are linked once. Ownership: cio_open returns a pointer
 * into the pool; s_close() releases the slot (fs_close + used=0); cio_close
 * (codec_io.c) just dispatches, so nothing is ever freed with free().
 *
 * Freestanding-friendly: no libc dependency, compiles under the same
 * -ffreestanding flags the SDK uses.
 */
#include "codec_io.h"
#include "fs.h"

#include <stddef.h>   /* NULL — freestanding, no libc allocation used */

#define CIO_FS_SLOTS 4   /* == fs_target.c's s_files[4] read handles */

typedef struct
{
    codec_io_t io;       /* vtable; io.ctx points back at this slot      */
    fs_file_t *f;
    long       size;     /* cached fs_file_size(); -1 if unknown         */
    long       pos;      /* last known position for tell()               */
    int        used;
} fs_slot_t;

static fs_slot_t s_slots[CIO_FS_SLOTS];

static long s_read(void *ctx, void *buf, long len)
{
    fs_slot_t *s = (fs_slot_t *)ctx;
    long n;

    if (s == NULL || s->f == NULL || buf == NULL || len <= 0)
        return -1;
    n = fs_read(s->f, buf, (unsigned long)len);
    if (n < 0)
        return -1;   /* fs_read returns FS_ERR_*; normalize to codec_io -1 */
    s->pos += n;
    return n;   /* 0 == EOF, which is what decoders expect */
}

static long s_seek(void *ctx, long offset)
{
    fs_slot_t *s = (fs_slot_t *)ctx;
    long r;

    if (s == NULL || s->f == NULL || offset < 0)
        return -1;
    r = fs_seek(s->f, offset);
    if (r < 0)
        return -1;
    s->pos = r;
    return r;
}

static long s_tell(void *ctx)
{
    fs_slot_t *s = (fs_slot_t *)ctx;

    if (s == NULL || s->f == NULL)
        return -1;
    return s->pos;
}

static long s_size(void *ctx)
{
    fs_slot_t *s = (fs_slot_t *)ctx;

    if (s == NULL || s->f == NULL)
        return -1;
    if (s->size == -1) {
        s->size = fs_file_size(s->f);
        if (s->size < 0)
            return -1;
    }
    return s->size;
}

static void s_close(void *ctx)
{
    fs_slot_t *s = (fs_slot_t *)ctx;

    if (s == NULL)
        return;
    if (s->f != NULL) {
        fs_close(s->f);
        s->f = NULL;
    }
    s->used = 0;   /* slot returns to the pool; never free()d */
}

codec_io_t *cio_open(const char *path)
{
    fs_slot_t *s = NULL;
    int i;

    if (path == NULL || path[0] == '\0')
        return NULL;

    for (i = 0; i < CIO_FS_SLOTS; i++) {
        if (!s_slots[i].used) {
            s = &s_slots[i];
            break;
        }
    }
    if (s == NULL)
        return NULL;   /* pool exhausted: all FS read handles in use */

    s->used = 1;
    s->f = NULL;
    s->size = -1;
    s->pos = 0;

    s->f = fs_open_read(path);
    if (s->f == NULL) {
        s->used = 0;
        return NULL;
    }
    /* Probe the size eagerly so decoders that need it (WAV chunk walking,
     * MP3 Xing/Info scan, FLAC STREAMINFO totals) get a fast answer and so
     * a failed size() surfaces as an open error rather than a mid-decode
     * surprise. */
    s->size = fs_file_size(s->f);
    if (s->size < 0) {
        fs_close(s->f);
        s->f = NULL;
        s->used = 0;
        return NULL;
    }

    s->io.ctx = s;
    s->io.read = s_read;
    s->io.seek = s_seek;
    s->io.tell = s_tell;
    s->io.size = s_size;
    s->io.close = s_close;
    return &s->io;
}
