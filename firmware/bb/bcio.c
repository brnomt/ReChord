/*
 * bcio.c — Core B codec_io backend over the A-served file ring
 * (BRNOMT-68 phase 3c).
 *
 * The codecs on B call the standard codec_io API; this backend replaces the
 * filesystem with the shared file ring that the AP fills (firmware/player/
 * bfs.c). cio_open() does not open a path: the AP has already opened the
 * track with bfs_open() and published IPC_F_OPEN/SIZE. Reads copy from the
 * ring (spinning while A refills); seeks are requests to A.
 *
 * This is the only codec_io backend linked into the B image (the AP keeps
 * codec_io_fs for its own use).
 */
#include <stdint.h>

#include "codec_io.h"
#include "ipc_shared.h"

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)

#define BCIO_WAIT 20000000u     /* bounded spin (~tens of ms at 200 MHz) */

static void dmb(void)
{
    __asm__ volatile("dmb" ::: "memory");
}

static int bcio_wait_open(void)
{
    uint32_t i;

    for (i = 0; i < BCIO_WAIT; i++) {
        if (CTRL[IPC_F_OPEN])
            return 0;
        dmb();
    }
    return -1;
}

static long bcio_read(void *ctx, void *buf, long len)
{
    uint8_t *d = (uint8_t *)buf;
    long got = 0;

    (void)ctx;
    if (buf == 0 || len <= 0)
        return (len == 0) ? 0 : -1;

    while (got < len) {
        uint32_t rd = CTRL[IPC_F_RD];
        uint32_t wr = CTRL[IPC_F_WR];
        uint32_t avail = wr - rd;
        uint32_t n, pos;

        if (avail == 0u) {
            /* End of file, or A stopped serving. */
            if (CTRL[IPC_F_EOF] || !CTRL[IPC_F_OPEN])
                break;
            dmb();
            continue;                   /* wait for the read-ahead */
        }

        n = avail;
        if ((uint32_t)(len - got) < n)
            n = (uint32_t)(len - got);
        pos = rd & IPC_FILE_MASK;
        {
            uint32_t first = IPC_FILE_BYTES - pos;
            uint32_t m = (n < first) ? n : first;
            const volatile uint8_t *src =
                (const volatile uint8_t *)(IPC_FILE_BASE + pos);
            uint32_t i;

            for (i = 0; i < m; i++)
                d[got + i] = src[i];
            if (n > m) {
                const volatile uint8_t *wrap =
                    (const volatile uint8_t *)IPC_FILE_BASE;
                for (i = 0; i < n - m; i++)
                    d[got + m + i] = wrap[i];
            }
        }
        rd += n;
        got += (long)n;
        dmb();
        CTRL[IPC_F_RD] = rd;             /* publish the consumed cursor */
        dmb();
    }
    return got;
}

static long bcio_seek(void *ctx, long offset)
{
    uint32_t seq, i;

    (void)ctx;
    if (offset < 0)
        return -1;

    CTRL[IPC_F_SEEK_TO] = (uint32_t)offset;
    dmb();
    seq = CTRL[IPC_F_SEEK_SEQ] + 1u;
    CTRL[IPC_F_SEEK_SEQ] = seq;          /* request A to reposition */
    dmb();

    for (i = 0; i < BCIO_WAIT; i++) {
        if (CTRL[IPC_F_SEEK_ACK] == seq)
            return (long)(CTRL[IPC_F_BASE] + CTRL[IPC_F_RD]);
        dmb();
    }
    return -1;
}

static long bcio_tell(void *ctx)
{
    (void)ctx;
    return (long)(CTRL[IPC_F_BASE] + CTRL[IPC_F_RD]);
}

static long bcio_size(void *ctx)
{
    uint32_t s;

    (void)ctx;
    s = CTRL[IPC_F_SIZE];
    return (s == 0xFFFFFFFFu) ? -1 : (long)s;
}

static void bcio_close(void *ctx)
{
    (void)ctx;                           /* A owns the file handle */
}

static codec_io_t s_io;

codec_io_t *cio_open(const char *path)
{
    (void)path;                          /* A opened it (bfs_open) */
    if (bcio_wait_open() != 0)
        return 0;
    s_io.ctx = 0;
    s_io.read = bcio_read;
    s_io.seek = bcio_seek;
    s_io.tell = bcio_tell;
    s_io.size = bcio_size;
    s_io.close = bcio_close;
    return &s_io;
}
