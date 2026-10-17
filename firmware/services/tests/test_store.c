/*
 * test_store.c — host tests for the crash-safe record store (store.c).
 *
 * A RAM-backed fake block device stands in for the eMMC: sector-granular,
 * 4-byte aligned buffers, with a write budget so a save can be cut at any
 * sector (the power-cut simulation). Every case asserts the invariant that
 * matters on the device: after ANY interrupted save, the previous record is
 * still the one returned (or EMPTY for a fresh region) — never garbage.
 */
#include "store.h"

#include <stdio.h>
#include <string.h>

static int s_checks;
static int s_failures;

#define TEST_START(name) printf("== %s ==\n", name)

#define CHECK(cond) do {                                                   \
    s_checks++;                                                            \
    if (!(cond)) {                                                         \
        printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);             \
        s_failures++;                                                      \
    }                                                                      \
} while (0)

/* ---- fake block device -------------------------------------------------- */

#define DEV_SECTORS 64
#define SLOT_LBA    8
#define SLOT_SECS   8

static uint8_t  s_dev[DEV_SECTORS * 512];
static int      s_writes_left;      /* -1 = unlimited; N = allow N writes   */
static uint32_t s_lo_lba, s_hi_lba; /* touched window (safety check)        */

static int fake_read(void *user, uint32_t lba, uint32_t count, void *buf)
{
    (void)user;
    if (count == 0 || lba + count > DEV_SECTORS)
        return -1;
    if (lba < s_lo_lba)
        s_lo_lba = lba;
    if (lba + count - 1 > s_hi_lba)
        s_hi_lba = lba + count - 1;
    memcpy(buf, s_dev + (size_t)lba * 512, (size_t)count * 512);
    return 0;
}

static int fake_write(void *user, uint32_t lba, uint32_t count, const void *buf)
{
    (void)user;
    if (count == 0 || lba + count > DEV_SECTORS)
        return -1;
    if (s_writes_left == 0)
        return -1;                  /* the "power" is gone: nothing lands  */
    if (s_writes_left > 0)
        s_writes_left--;
    if (lba < s_lo_lba)
        s_lo_lba = lba;
    if (lba + count - 1 > s_hi_lba)
        s_hi_lba = lba + count - 1;
    memcpy(s_dev + (size_t)lba * 512, buf, (size_t)count * 512);
    return 0;
}

static const store_io_t s_io = { fake_read, fake_write, 0 };

static void dev_reset(void)
{
    memset(s_dev, 0, sizeof s_dev);
    s_writes_left = -1;
    s_lo_lba = 0xFFFFFFFFu;
    s_hi_lba = 0;
}

static void make_record(uint8_t *buf, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++)
        buf[i] = (uint8_t)(i * 7u + 3u);
}

/* ---- tests -------------------------------------------------------------- */

int main(void)
{
    store_t st;
    uint8_t rec[4096], out[4096];
    uint32_t len = 0;

    store_init(&st, &s_io, SLOT_LBA, SLOT_SECS);
    CHECK(store_capacity(&st) == SLOT_SECS * 512u - STORE_HDR_BYTES);

    TEST_START("empty region");
    dev_reset();
    CHECK(store_load(&st, out, sizeof out, &len) == STORE_ERR_EMPTY);

    TEST_START("save/load round-trip");
    dev_reset();
    make_record(rec, 600);
    CHECK(store_save(&st, rec, 600) == STORE_OK);
    memset(out, 0, sizeof out);
    CHECK(store_load(&st, out, sizeof out, &len) == STORE_OK);
    CHECK(len == 600);
    CHECK(memcmp(rec, out, 600) == 0);
    CHECK(rec[0] == out[0] && rec[599] == out[599]);

    TEST_START("second save wins (A/B alternation)");
    make_record(rec, 1000);
    rec[0] = 0xAA;
    CHECK(store_save(&st, rec, 1000) == STORE_OK);
    CHECK(store_load(&st, out, sizeof out, &len) == STORE_OK);
    CHECK(len == 1000 && out[0] == 0xAA && memcmp(rec, out, 1000) == 0);
    /* five more rotations stay readable */
    for (int i = 0; i < 5; i++) {
        rec[1] = (uint8_t)i;
        CHECK(store_save(&st, rec, 1000) == STORE_OK);
    }
    CHECK(store_load(&st, out, sizeof out, &len) == STORE_OK);
    CHECK(len == 1000 && out[1] == 4);

    TEST_START("corrupt newest payload -> previous record survives");
    dev_reset();
    make_record(rec, 300);
    rec[0] = 1;
    CHECK(store_save(&st, rec, 300) == STORE_OK);
    rec[0] = 2;
    CHECK(store_save(&st, rec, 300) == STORE_OK);
    /* newer slot is B (seq 2): flip one payload byte there */
    s_dev[SLOT_LBA * 512 + SLOT_SECS * 512 + 512 + 5] ^= 0xFF;
    CHECK(store_load(&st, out, sizeof out, &len) == STORE_OK);
    CHECK(len == 300 && out[0] == 1);

    TEST_START("corrupt newest header -> previous record survives");
    dev_reset();
    rec[0] = 1;
    CHECK(store_save(&st, rec, 300) == STORE_OK);
    rec[0] = 2;
    CHECK(store_save(&st, rec, 300) == STORE_OK);
    s_dev[(SLOT_LBA + SLOT_SECS) * 512 + 1] ^= 0xFF;   /* slot B magic/bytes */
    CHECK(store_load(&st, out, sizeof out, &len) == STORE_OK);
    CHECK(len == 300 && out[0] == 1);

    TEST_START("both slots corrupt -> EMPTY, not garbage");
    s_dev[SLOT_LBA * 512 + 512 + 5] ^= 0xFF;            /* slot A payload    */
    CHECK(store_load(&st, out, sizeof out, &len) == STORE_ERR_EMPTY);

    TEST_START("power cut during the FIRST save -> EMPTY (no garbage)");
    for (int cut = 0; cut <= 2; cut++) {
        dev_reset();
        s_writes_left = cut;        /* cut = 0: first payload write dies    */
        CHECK(store_save(&st, rec, 600) == STORE_ERR);
        CHECK(store_load(&st, out, sizeof out, &len) == STORE_ERR_EMPTY);
    }

    TEST_START("power cut during the SECOND save -> previous record");
    for (int cut = 0; cut <= 2; cut++) {
        dev_reset();
        make_record(rec, 600);
        rec[0] = 0x11;
        CHECK(store_save(&st, rec, 600) == STORE_OK);
        rec[0] = 0x22;
        s_writes_left = cut;       /* cut = writes that land before power  */
        (void)store_save(&st, rec, 600);   /* may fail at any sector       */
        s_writes_left = -1;
        CHECK(store_load(&st, out, sizeof out, &len) == STORE_OK);
        CHECK(len == 600 && out[0] == 0x11);   /* old record still rules   */
    }
    /* All three writes landing means the save completed: the new record
     * wins (this is the boundary of the two cases above). */
    rec[0] = 0x22;
    CHECK(store_save(&st, rec, 600) == STORE_OK);
    CHECK(store_load(&st, out, sizeof out, &len) == STORE_OK);
    CHECK(out[0] == 0x22);

    TEST_START("size and argument guards");
    dev_reset();
    CHECK(store_save(&st, rec, SLOT_SECS * 512u - STORE_HDR_BYTES + 1u) ==
          STORE_ERR_BIG);
    CHECK(store_save(&st, 0, 10) == STORE_ERR_ARG);
    CHECK(store_load(0, out, sizeof out, &len) == STORE_ERR_ARG);
    CHECK(store_load(&st, 0, sizeof out, &len) == STORE_ERR_ARG);
    make_record(rec, 600);
    CHECK(store_save(&st, rec, 600) == STORE_OK);
    CHECK(store_load(&st, out, 100, &len) == STORE_ERR_BIG);

    TEST_START("zero-length record");
    dev_reset();
    CHECK(store_save(&st, out, 0) == STORE_OK);
    CHECK(store_load(&st, out, sizeof out, &len) == STORE_OK);
    CHECK(len == 0);

    TEST_START("touches only its own region");
    CHECK(s_lo_lba >= SLOT_LBA);
    CHECK(s_hi_lba < SLOT_LBA + 2u * SLOT_SECS);

    if (s_failures == 0)
        printf("test_store: %d checks, OK\n", s_checks);
    else
        printf("test_store: %d checks, %d FAILURES\n", s_checks, s_failures);
    return (s_failures == 0) ? 0 : 1;
}
