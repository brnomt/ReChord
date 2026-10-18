/*
 * bfs.c — A-side file service for Core B (see bfs.h).
 *
 * TWO rings (BRNOMT-73): the AP serves the CURRENT track into one file ring
 * while B reads it, and PRE-OPENS the next track into the other ring during
 * playback. At the gapless boundary B reopens its codec reading the ring that
 * is already full, so it does not wait on the SD card.
 *
 * Ring protocol (firmware/player/ipc_shared.h), per ring:
 *   - A owns OPEN/SIZE/BASE/WR/EOF; B owns RD and the seek request.
 *   - logical write offset = BASE + WR; B's logical read offset = BASE + RD.
 *   - A writes file bytes into the ring, then DMB, then publishes WR.
 *   - B reads WR, copies, updates RD; it DMBs before consuming again.
 *   - seek: B puts the target in SEEK_TO, increments SEEK_SEQ; A repositions,
 *     resets BASE/RD/WR/EOF and copies SEEK_SEQ into SEEK_ACK.
 *
 * Ring 0 is IPC_F_* / IPC_FILE_BASE, ring 1 is IPC_F2_* / IPC_FILE2_BASE.
 * `s_cur` is the ring B is reading; `s_pre` is the pre-opened next track.
 *
 * All data goes through the rings; the FS itself never runs on B.
 */
#include <stdint.h>

#include "bfs.h"
#include "codec_io.h"
#include "ipc_shared.h"

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)

#define REFILL_CHUNK 4096u

/* Longest path we keep for the pre-open match (device path, like PLAYER's). */
#define BFS_PATH_MAX 208

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

typedef struct {
    codec_io_t *io;
    int         slot;
} bfs_svc_t;

static bfs_svc_t s_cur;                  /* ring B is reading              */
static bfs_svc_t s_pre;                  /* pre-opened next track, or io=0 */
static char      s_pre_path[BFS_PATH_MAX];
static int       s_pre_valid;

static void dmb(void)
{
#if defined(__arm__) || defined(__thumb__)
    __asm__ volatile("dmb" ::: "memory");
#else
    __asm__ volatile("" ::: "memory");   /* host tests: compiler barrier */
#endif
}

static int path_eq(const char *a, const char *b)
{
    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

static void path_set(char *dst, const char *src)
{
    unsigned i = 0;

    while (src[i] != '\0' && i + 1u < BFS_PATH_MAX) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/* Publish a fresh ring for `s`: header fields first, OPEN last. */
static void ring_publish(bfs_svc_t *s, long size)
{
    const uint16_t *w = k_w[s->slot];

    CTRL[w[W_BASE]] = 0;
    CTRL[w[W_RD]] = 0;
    CTRL[w[W_WR]] = 0;
    CTRL[w[W_EOF]] = 0;
    CTRL[w[W_SIZE]] = (size < 0) ? 0xFFFFFFFFu : (uint32_t)size;
    CTRL[w[W_SEEK_TO]] = 0;
    CTRL[w[W_SEEK_ACK]] = CTRL[w[W_SEEK_SEQ]];
    dmb();
    CTRL[w[W_OPEN]] = 1;
    dmb();
}

/* Close `s`'s handle and stop serving its ring. */
static void ring_close(bfs_svc_t *s)
{
    if (s->io != 0) {
        cio_close(s->io);
        s->io = 0;
    }
    CTRL[k_w[s->slot][W_OPEN]] = 0;
    dmb();
}

/* Service a pending seek on `s` and refill its ring ahead of B. */
static void poll_svc(bfs_svc_t *s)
{
    const uint16_t *w = k_w[s->slot];
    uint32_t ring_base = k_base[s->slot];
    uint32_t ring_bytes = k_bytes[s->slot];
    uint32_t ring_mask = ring_bytes - 1u;
    uint32_t rd, wr, freeb;

    if (s->io == 0)
        return;

    /* 1. Apply a pending seek. B is quiesced until SEEK_ACK catches up. */
    if (CTRL[w[W_SEEK_SEQ]] != CTRL[w[W_SEEK_ACK]]) {
        uint32_t to = CTRL[w[W_SEEK_TO]];

        if (cio_seek(s->io, (long)to) == (long)to) {
            CTRL[w[W_BASE]] = to;
            CTRL[w[W_RD]] = 0;
            CTRL[w[W_WR]] = 0;
            CTRL[w[W_EOF]] = 0;
        }
        dmb();
        CTRL[w[W_SEEK_ACK]] = CTRL[w[W_SEEK_SEQ]];
        dmb();
    }

    /* 2. Read ahead: fill the ring while it has room and the file is not in. */
    if (CTRL[w[W_EOF]])
        return;

    rd = CTRL[w[W_RD]];
    wr = CTRL[w[W_WR]];
    freeb = ring_bytes - (wr - rd);

    while (freeb > 0u) {
        uint32_t pos = wr & ring_mask;
        uint32_t first = ring_bytes - pos;
        uint32_t n = (freeb < first) ? freeb : first;
        long got;

        if (n > REFILL_CHUNK)
            n = REFILL_CHUNK;

        got = cio_read(s->io, (void *)(uintptr_t)(ring_base + pos), (long)n);
        if (got < 0) {
            CTRL[w[W_OPEN]] = 0;        /* surface the error to B */
            dmb();
            return;
        }
        if (got == 0) {
            CTRL[w[W_EOF]] = 1;         /* clean EOF */
            dmb();
            return;
        }

        wr += (uint32_t)got;
        freeb -= (uint32_t)got;
        dmb();
        CTRL[w[W_WR]] = wr;             /* publish after the data */
        dmb();

        if ((uint32_t)got < n) {
            CTRL[w[W_EOF]] = 1;         /* short read == end of file */
            dmb();
            return;
        }
    }
}

int bfs_open(const char *path)
{
    long size;

    if (path == 0)
        return -1;

    /* A hard start always resets to ring 0 and discards any pre-open. */
    bfs_close();

    s_cur.slot = 0;
    s_cur.io = cio_open(path);
    if (s_cur.io == 0)
        return -1;
    size = cio_size(s_cur.io);
    (void)cio_seek(s_cur.io, 0);
    ring_publish(&s_cur, size);
    return 0;
}

int bfs_preopen(const char *path)
{
    long size;
    int slot;

    if (path == 0)
        return -1;
    if (s_pre_valid && s_pre.io != 0 && path_eq(s_pre_path, path))
        return 0;                       /* already pre-opened for this file */

    if (s_pre.io != 0) {
        cio_close(s_pre.io);
        s_pre.io = 0;
    }
    s_pre_valid = 0;

    slot = 1 - s_cur.slot;
    s_pre.io = cio_open(path);
    if (s_pre.io == 0)
        return -1;
    s_pre.slot = slot;
    size = cio_size(s_pre.io);
    (void)cio_seek(s_pre.io, 0);
    path_set(s_pre_path, path);
    s_pre_valid = 1;
    ring_publish(&s_pre, size);
    return 0;
}

int bfs_preopen_ready(const char *path)
{
    if (!s_pre_valid || s_pre.io == 0 || path == 0)
        return 0;
    return path_eq(s_pre_path, path) ? 1 : 0;
}

int bfs_promote(void)
{
    codec_io_t *old;
    int slot;

    if (!s_pre_valid || s_pre.io == 0)
        return -1;

    slot = s_pre.slot;
    old = s_cur.io;
    s_cur.io = s_pre.io;
    s_cur.slot = slot;

    s_pre.io = 0;                       /* the other ring is now free */
    s_pre.slot = 1 - slot;
    s_pre_valid = 0;
    s_pre_path[0] = '\0';

    /* Retire the old ring explicitly. Left flagged open with no file behind
     * it, Core B would spin forever in bcio_read waiting for a refill that can
     * never come while the AP waits for its ACK (BRNOMT-73: observed as
     * "cmd 1 no ack after 1517 ms" plus silence). */
    CTRL[k_w[s_pre.slot][W_OPEN]] = 0;
    dmb();

    if (old != 0)
        cio_close(old);                 /* previous track fully consumed */
    return slot;
}

void bfs_close(void)
{
    ring_close(&s_cur);
    ring_close(&s_pre);
    s_pre_valid = 0;
    s_pre_path[0] = '\0';
}

int bfs_is_open(void)
{
    return s_cur.io != 0;
}

void bfs_poll(void)
{
    poll_svc(&s_cur);
    poll_svc(&s_pre);
}
