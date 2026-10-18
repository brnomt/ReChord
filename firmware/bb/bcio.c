/*
 * bcio.c — Core B codec_io backend over the A-served file rings
 * (BRNOMT-68 phase 3c, two rings BRNOMT-73).
 *
 * The codecs on B call the standard codec_io API; this backend replaces the
 * filesystem with the shared file ring that the AP fills (firmware/player/
 * bfs.c). The AP PUBLISHES the ring to use in IPC_C_RING before every OPEN /
 * NEXT command: 0 = IPC_F_* / IPC_FILE_BASE (current), 1 = IPC_F2_* /
 * IPC_FILE2_BASE (the pre-opened next track, already filled — which is why the
 * boundary does not wait on the SD).
 *
 * cio_open() does not open a path: the AP has already opened the track.
 * Reads copy from the selected ring (spinning while A refills); seeks are
 * requests to A.
 *
 * This is the only codec_io backend linked into the B image (the AP keeps
 * codec_io_fs for its own use).
 */
#include <stdint.h>

#include "codec_io.h"
#include "ipc_shared.h"

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)

#define BCIO_WAIT 20000000u     /* bounded spin (~tens of ms at 200 MHz) */

/* Word slots inside each ring's control set. */
enum { W_OPEN, W_SIZE, W_BASE, W_RD, W_WR, W_EOF, W_SEEK_TO, W_SEEK_SEQ,
       W_SEEK_ACK };

static const uint16_t k_w[2][9] = {
    { IPC_F_OPEN, IPC_F_SIZE, IPC_F_BASE, IPC_F_RD, IPC_F_WR, IPC_F_EOF,
      IPC_F_SEEK_TO, IPC_F_SEEK_SEQ, IPC_F_SEEK_ACK },
    { IPC_F2_OPEN, IPC_F2_SIZE, IPC_F2_BASE, IPC_F2_RD, IPC_F2_WR, IPC_F2_EOF,
      IPC_F2_SEEK_TO, IPC_F2_SEEK_SEQ, IPC_F2_SEEK_ACK },
};
static const uint32_t k_base[2]  = { IPC_FILE_BASE,  IPC_FILE2_BASE };
static const uint32_t k_bytes[2] = { IPC_FILE_BYTES, IPC_FILE2_BYTES };

/* The ring selected for the codec currently being opened. */
static const uint16_t *s_w = k_w[0];
static uint32_t s_base = IPC_FILE_BASE;
static uint32_t s_bytes = IPC_FILE_BYTES;

static void dmb(void)
{
    __asm__ volatile("dmb" ::: "memory");
}

static int bcio_wait_open(void)
{
    uint32_t i;

    for (i = 0; i < BCIO_WAIT; i++) {
        if (CTRL[s_w[W_OPEN]])
            return 0;
        dmb();
    }
    return -1;
}

static long bcio_read(void *ctx, void *buf, long len)
{
    uint8_t *d = (uint8_t *)buf;
    long got = 0;
    uint32_t spins = 0;

    (void)ctx;
    if (buf == 0 || len <= 0)
        return (len == 0) ? 0 : -1;

    while (got < len) {
        uint32_t rd = CTRL[s_w[W_RD]];
        uint32_t wr = CTRL[s_w[W_WR]];
        uint32_t avail = wr - rd;
        uint32_t pos, n;

        if (avail == 0u) {
            /* End of file, or A stopped serving. */
            if (CTRL[s_w[W_EOF]] || !CTRL[s_w[W_OPEN]])
                break;
            /* BOUNDED wait: a read that never returns would wedge the command
             * channel (A blocks in b_cmd_wait for this core's ACK) — the
             * field symptom was "cmd 1 no ack after ~1500 ms" plus silence.
             * Report the stall and fail the read instead. */
            if (++spins >= BCIO_WAIT) {
                CTRL[IPC_C_BSTALL]++;
                dmb();
                break;
            }
            dmb();
            continue;                   /* wait for the read-ahead */
        }
        spins = 0;

        n = avail;
        if ((uint32_t)(len - got) < n)
            n = (uint32_t)(len - got);
        pos = rd & (s_bytes - 1u);
        {
            uint32_t first = s_bytes - pos;
            uint32_t m = (n < first) ? n : first;
            const volatile uint8_t *src =
                (const volatile uint8_t *)(s_base + pos);
            uint32_t i;

            for (i = 0; i < m; i++)
                d[got + i] = src[i];
            if (n > m) {
                const volatile uint8_t *wrap =
                    (const volatile uint8_t *)s_base;
                for (i = 0; i < n - m; i++)
                    d[got + m + i] = wrap[i];
            }
        }
        rd += n;
        got += (long)n;
        dmb();
        CTRL[s_w[W_RD]] = rd;            /* publish the consumed cursor */
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

    CTRL[s_w[W_SEEK_TO]] = (uint32_t)offset;
    dmb();
    seq = CTRL[s_w[W_SEEK_SEQ]] + 1u;
    CTRL[s_w[W_SEEK_SEQ]] = seq;         /* request A to reposition */
    dmb();

    for (i = 0; i < BCIO_WAIT; i++) {
        if (CTRL[s_w[W_SEEK_ACK]] == seq)
            return (long)(CTRL[s_w[W_BASE]] + CTRL[s_w[W_RD]]);
        dmb();
    }
    return -1;
}

static long bcio_tell(void *ctx)
{
    (void)ctx;
    return (long)(CTRL[s_w[W_BASE]] + CTRL[s_w[W_RD]]);
}

static long bcio_size(void *ctx)
{
    uint32_t s;

    (void)ctx;
    s = CTRL[s_w[W_SIZE]];
    return (s == 0xFFFFFFFFu) ? -1 : (long)s;
}

static void bcio_close(void *ctx)
{
    (void)ctx;                           /* A owns the file handle */
}

static codec_io_t s_io;

codec_io_t *cio_open(const char *path)
{
    int ring;

    (void)path;                          /* A opened it (bfs_open/preopen) */

    /* The AP publishes the ring before the OPEN/NEXT command. */
    ring = (int)(CTRL[IPC_C_RING] & 1u);
    s_w = k_w[ring];
    s_base = k_base[ring];
    s_bytes = k_bytes[ring];

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
