/*
 * bfs.c — A-side file service for Core B (see bfs.h).
 *
 * Ring protocol (firmware/player/ipc_shared.h):
 *   - A owns IPC_F_BASE/WR/EOF/SIZE/OPEN; B owns RD and the seek request.
 *   - logical write offset = BASE + WR; B's logical read offset = BASE + RD.
 *   - A writes file bytes into the ring, then DMB, then publishes WR.
 *   - B reads WR, copies, updates RD; it DMBs before consuming again.
 *   - seek: B puts the target in SEEK_TO, increments SEEK_SEQ; A repositions,
 *     resets BASE/RD/WR/EOF and copies SEEK_SEQ into SEEK_ACK.
 *
 * All data goes through the ring; the FS itself never runs on B.
 */
#include <stdint.h>

#include "bfs.h"
#include "codec_io.h"
#include "ipc_shared.h"

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)

#define REFILL_CHUNK 4096u

static codec_io_t *s_io;

static void dmb(void)
{
    __asm__ volatile("dmb" ::: "memory");
}

int bfs_open(const char *path)
{
    long size;

    if (path == 0)
        return -1;
    bfs_close();

    s_io = cio_open(path);
    if (s_io == 0)
        return -1;
    size = cio_size(s_io);
    (void)cio_seek(s_io, 0);

    CTRL[IPC_F_BASE] = 0;
    CTRL[IPC_F_RD] = 0;
    CTRL[IPC_F_WR] = 0;
    CTRL[IPC_F_EOF] = 0;
    CTRL[IPC_F_SIZE] = (size < 0) ? 0xFFFFFFFFu : (uint32_t)size;
    CTRL[IPC_F_SEEK_TO] = 0;
    CTRL[IPC_F_SEEK_ACK] = CTRL[IPC_F_SEEK_SEQ];
    dmb();
    CTRL[IPC_F_OPEN] = 1;
    dmb();
    return 0;
}

void bfs_close(void)
{
    if (s_io != 0) {
        cio_close(s_io);
        s_io = 0;
    }
    CTRL[IPC_F_OPEN] = 0;
    dmb();
}

int bfs_is_open(void)
{
    return s_io != 0;
}

void bfs_poll(void)
{
    uint32_t rd, wr, freeb;

    if (s_io == 0)
        return;

    /* 1. Apply a pending seek. B is quiesced until SEEK_ACK catches up. */
    if (CTRL[IPC_F_SEEK_SEQ] != CTRL[IPC_F_SEEK_ACK]) {
        uint32_t to = CTRL[IPC_F_SEEK_TO];

        if (cio_seek(s_io, (long)to) == (long)to) {
            CTRL[IPC_F_BASE] = to;
            CTRL[IPC_F_RD] = 0;
            CTRL[IPC_F_WR] = 0;
            CTRL[IPC_F_EOF] = 0;
        }
        dmb();
        CTRL[IPC_F_SEEK_ACK] = CTRL[IPC_F_SEEK_SEQ];
        dmb();
    }

    /* 2. Read ahead: fill the ring while it has room and the file is not in. */
    if (CTRL[IPC_F_EOF])
        return;

    rd = CTRL[IPC_F_RD];
    wr = CTRL[IPC_F_WR];
    freeb = IPC_FILE_BYTES - (wr - rd);

    while (freeb > 0u) {
        uint32_t pos = wr & IPC_FILE_MASK;
        uint32_t first = IPC_FILE_BYTES - pos;
        uint32_t n = (freeb < first) ? freeb : first;
        long got;

        if (n > REFILL_CHUNK)
            n = REFILL_CHUNK;

        got = cio_read(s_io, (void *)(IPC_FILE_BASE + pos), (long)n);
        if (got < 0) {
            CTRL[IPC_F_OPEN] = 0;       /* surface the error to B */
            dmb();
            return;
        }
        if (got == 0) {
            CTRL[IPC_F_EOF] = 1;        /* clean EOF */
            dmb();
            return;
        }

        wr += (uint32_t)got;
        freeb -= (uint32_t)got;
        dmb();
        CTRL[IPC_F_WR] = wr;            /* publish after the data */
        dmb();

        if ((uint32_t)got < n) {
            CTRL[IPC_F_EOF] = 1;        /* short read == end of file */
            dmb();
            return;
        }
    }
}
