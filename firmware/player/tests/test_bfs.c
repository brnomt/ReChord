/*
 * test_bfs.c — the A->B file ring (bfs.c) against Core B's reader protocol.
 *
 * The shared block is mapped at its real address (IPC_SHARED_BASE) so bfs.c
 * runs unmodified. The "B side" below mirrors firmware/bb/bcio.c: read what
 * is available from RD, publish RD; a seek posts SEEK_TO/SEEK_SEQ, waits for
 * SEEK_ACK and then reads from BASE + RD. B consumes odd amounts and seeks to
 * odd offsets; every byte must match the file, the ring must be refilled to
 * within one chunk, and the reads must start on file sectors.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "bfs.h"
#include "ipc_shared.h"

#define CTRL ((volatile uint32_t *)IPC_SHARED_BASE)
#define PATH ".work/bfs_src.bin"
#define SIZE (300u * 1024u + 123u)

static int s_failures;
#define CHECK(c, m) do { if (c) printf("  PASS: %s\n", m); \
    else { printf("  FAIL: %s (line %d)\n", m, __LINE__); s_failures++; } } while (0)

static uint8_t pat(uint32_t i)
{
    return (uint8_t)(i * 7u + (i >> 8) * 13u + 1u);
}

/* Core B's view of ring 0 (bcio.c semantics). */
static long b_read(uint8_t *dst, long len)
{
    long got = 0;
    int idle = 0;

    while (got < len) {
        uint32_t rd = CTRL[IPC_F_RD], wr = CTRL[IPC_F_WR], avail = wr - rd;
        uint32_t n, i;

        if (avail == 0u) {
            if (CTRL[IPC_F_EOF] || !CTRL[IPC_F_OPEN] || ++idle > 3)
                break;
            bfs_poll();                  /* the AP serves while B waits */
            continue;
        }
        idle = 0;
        n = avail < (uint32_t)(len - got) ? avail : (uint32_t)(len - got);
        for (i = 0; i < n; i++)
            dst[got + i] = *(volatile uint8_t *)(uintptr_t)
                (IPC_FILE_BASE + ((rd + i) & IPC_FILE_MASK));
        CTRL[IPC_F_RD] = rd + n;
        got += (long)n;
    }
    return got;
}

static long b_seek(uint32_t to)
{
    uint32_t seq = CTRL[IPC_F_SEEK_SEQ] + 1u;

    CTRL[IPC_F_SEEK_TO] = to;
    CTRL[IPC_F_SEEK_SEQ] = seq;
    bfs_poll();
    if (CTRL[IPC_F_SEEK_ACK] != seq)
        return -1;
    return (long)(CTRL[IPC_F_BASE] + CTRL[IPC_F_RD]);
}

/* Consume from `pos` to EOF in odd steps; 1 = every byte right. */
static int b_drain(uint32_t pos, int *refill_ok)
{
    static uint8_t buf[5000];
    static const long steps[] = { 1, 333, 4096, 17, 1500, 4999, 512, 77 };
    unsigned k = 0;

    for (;;) {
        long n = b_read(buf, steps[k++ % 8]), i;

        for (i = 0; i < n; i++)
            if (buf[i] != pat(pos + (uint32_t)i))
                return 0;
        pos += (uint32_t)n;
        bfs_poll();
        /* After a poll the ring is within one chunk of full (or at EOF). */
        if (!CTRL[IPC_F_EOF] &&
            IPC_FILE_BYTES - (CTRL[IPC_F_WR] - CTRL[IPC_F_RD]) >= 4096u)
            *refill_ok = 0;
        if (n == 0)
            break;
    }
    return pos == SIZE;
}

int main(void)
{
    static uint8_t data[SIZE];
    static const uint32_t seeks[] = { 0, 1, 511, 512, 513, 70001, 4095,
                                      SIZE - 700, SIZE - 1, 123457 };
    FILE *f;
    unsigned i;
    int ok, refill = 1, aligned = 1;

    printf("=== bfs file ring ===\n");
    if (mmap((void *)(uintptr_t)IPC_SHARED_BASE, 0x10000u,
             PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS |
             MAP_FIXED_NOREPLACE, -1, 0) != (void *)(uintptr_t)IPC_SHARED_BASE) {
        printf("  SKIP: cannot map the shared block on this host\n");
        return 0;
    }
    for (i = 0; i < SIZE; i++)
        data[i] = pat(i);
    f = fopen(PATH, "wb");
    if (f == 0 || fwrite(data, 1, SIZE, f) != SIZE)
        return 1;
    fclose(f);

    CHECK(bfs_open(PATH) == 0, "open");
    bfs_poll();
    CHECK(CTRL[IPC_F_WR] - CTRL[IPC_F_RD] == IPC_FILE_BYTES,
          "initial fill tops the ring");
    CHECK(b_drain(0, &refill), "sequential read: every byte right");

    ok = 1;
    for (i = 0; i < sizeof seeks / sizeof seeks[0]; i++) {
        long at = b_seek(seeks[i]);

        if (at != (long)seeks[i]) {
            ok = 0;
            break;
        }
        /* The ring is re-based on the target's sector: refills land on file
         * sectors and word-aligned ring offsets (the multi-sector path). */
        if ((CTRL[IPC_F_BASE] & 511u) != 0u ||
            CTRL[IPC_F_RD] != seeks[i] - CTRL[IPC_F_BASE])
            aligned = 0;
        bfs_poll();
        if (CTRL[IPC_F_WR] < IPC_FILE_BYTES &&
            ((CTRL[IPC_F_BASE] + CTRL[IPC_F_WR]) & 511u) != 0u &&
            CTRL[IPC_F_BASE] + CTRL[IPC_F_WR] != SIZE)
            aligned = 0;
        ok &= b_drain(seeks[i], &refill);
    }
    CHECK(ok, "10 seeks (odd offsets, near EOF): position and data right");
    CHECK(aligned, "after a seek the ring is sector-based and refills end on sectors");
    CHECK(refill, "every poll leaves the ring within one chunk of full");
    bfs_close();
    remove(PATH);

    printf("%s (%d failure%s)\n", s_failures ? "FAILURES" : "ALL BFS TESTS PASSED",
           s_failures, s_failures == 1 ? "" : "s");
    return s_failures != 0;
}
