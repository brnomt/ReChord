/*
 * test_ringbuf.c — host tests for the SPSC ring buffer: basic traffic,
 * wrap-around continuity, overflow/underflow clamping, skip, and
 * free-running counter wrap-around safety.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../ringbuf.h"
#include "test_util.h"

#define CAP 64   /* deliberately small so traffic wraps constantly */

static uint8_t storage[CAP];

static void test_init(void)
{
    ringbuf_t rb;
    uint8_t buf[CAP];

    TEST("init validation");
    CHECK_EQI(rb_init(&rb, storage, CAP), 0, "pow2 capacity accepted");
    CHECK_EQI(rb_init(&rb, storage, CAP - 1), -1, "non-pow2 rejected");
    CHECK_EQI(rb_init(&rb, storage, 0), -1, "zero capacity rejected");
    CHECK_EQI(rb_init(0, storage, CAP), -1, "NULL ring rejected");
    CHECK_EQI(rb_init(&rb, 0, CAP), -1, "NULL storage rejected");
    CHECK_EQI(rb_init(&rb, buf, CAP), 0, "plain array storage ok");
}

static void test_basic(void)
{
    ringbuf_t rb;
    uint8_t out[CAP];
    const char *msg = "hello-ring";

    TEST("basic write/read");
    rb_init(&rb, storage, CAP);
    CHECK_EQI(rb_used(&rb), 0, "starts empty");
    CHECK_EQI(rb_free(&rb), CAP, "starts full-free");
    CHECK_EQI(rb_write(&rb, msg, 10), 10, "write 10");
    CHECK_EQI(rb_used(&rb), 10, "used 10");
    CHECK_EQI(rb_free(&rb), CAP - 10, "free reduced");
    CHECK_EQI(rb_read(&rb, out, 10), 10, "read 10");
    CHECK(memcmp(out, msg, 10) == 0, "payload intact");
    CHECK_EQI(rb_used(&rb), 0, "empty again");
}

static void test_underflow(void)
{
    ringbuf_t rb;
    uint8_t out[CAP];

    TEST("underflow clamps");
    rb_init(&rb, storage, CAP);
    CHECK_EQI(rb_read(&rb, out, 10), 0, "read on empty returns 0");
    rb_write(&rb, "abc", 3);
    CHECK_EQI(rb_read(&rb, out, 10), 3, "short read clamps to used");
    CHECK(memcmp(out, "abc", 3) == 0, "short read payload");
    CHECK_EQI(rb_read(&rb, out, 1), 0, "empty again");
}

static void test_overflow(void)
{
    ringbuf_t rb;
    uint8_t out[CAP];
    int i;

    TEST("overflow clamps");
    rb_init(&rb, storage, CAP);
    for (i = 0; i < CAP; i++)
        storage[i] = 0;    /* sentinel: distinguish unwritten slots */
    {
        uint8_t big[CAP * 2];
        for (i = 0; i < CAP * 2; i++)
            big[i] = (uint8_t)(i + 1);
        CHECK_EQI(rb_write(&rb, big, CAP * 2), CAP,
                  "over-size write clamps to capacity");
        CHECK_EQI(rb_free(&rb), 0, "ring is exactly full");
        CHECK_EQI(rb_write(&rb, big, 1), 0, "no write while full");
        CHECK_EQI(rb_read(&rb, out, CAP), CAP, "read all back");
        for (i = 0; i < CAP; i++) {
            if (out[i] != (uint8_t)(i + 1))
                break;
        }
        CHECK(i == CAP, "overflow kept the first CAP bytes in order");
    }
}

static void test_wrap(void)
{
    ringbuf_t rb;
    uint8_t out[CAP];
    int i, ok = 1;

    TEST("wrap-around continuity");
    rb_init(&rb, storage, CAP);
    /* Repeatedly write/read odd sizes so copies straddle the wrap point. */
    for (i = 0; i < 200; i++) {
        uint8_t chunk[17];
        int n = (i % 17) + 1;
        int j, got;

        for (j = 0; j < n; j++)
            chunk[j] = (uint8_t)(i * 3 + j);
        if (rb_write(&rb, chunk, (size_t)n) != (size_t)n) {
            ok = 0;
            break;
        }
        got = (int)rb_read(&rb, out, (size_t)n);
        if (got != n || memcmp(out, chunk, (size_t)n) != 0) {
            ok = 0;
            break;
        }
    }
    CHECK(ok, "200 write/read cycles across the wrap point");

    /* One big spanning transfer: writes and reads that straddle the wrap
     * point (index 63 -> 0) go through both split-copy branches. */
    {
        uint8_t big[52];
        for (i = 0; i < 52; i++)
            big[i] = (uint8_t)(0xA0 + i);
        rb_init(&rb, storage, CAP);
        CHECK_EQI(rb_write(&rb, big, 52), 52, "fill 52/64 (wr=52)");
        CHECK_EQI(rb_read(&rb, out, 20), 20, "drain 20 (rd=20)");
        for (i = 0; i < 20; i++)
            if (out[i] != (uint8_t)(0xA0 + i))
                break;
        CHECK(i == 20, "pre-wrap bytes in order");
        {
            uint8_t more[24];
            for (i = 0; i < 24; i++)
                more[i] = (uint8_t)(0xB0 + i);
            /* wr=52: 12 bytes land at 52..63, 12 wrap to 0..11. */
            CHECK_EQI(rb_write(&rb, more, 24), 24,
                      "write 24 spans the wrap point");
        }
        CHECK_EQI(rb_read(&rb, out, 56), 56, "drain 56 spans the wrap");
        for (i = 0; i < 32; i++)
            if (out[i] != (uint8_t)(0xA0 + 20 + i))
                break;
        CHECK(i == 32, "old tail bytes in order (0xA0..)");
        for (i = 0; i < 24; i++)
            if (out[32 + i] != (uint8_t)(0xB0 + i))
                break;
        CHECK(i == 24, "wrapped bytes in order (0xB0..)");
        CHECK_EQI(rb_used(&rb), 0, "fully drained");
    }
}

static void test_skip(void)
{
    ringbuf_t rb;
    uint8_t out[CAP];

    TEST("skip (drop buffered bytes)");
    rb_init(&rb, storage, CAP);
    rb_write(&rb, "0123456789", 10);
    CHECK_EQI(rb_skip(&rb, 4), 4, "skip 4");
    CHECK_EQI(rb_used(&rb), 6, "used reflects skip");
    CHECK_EQI(rb_skip(&rb, 100), 6, "skip clamps to used");
    CHECK_EQI(rb_read(&rb, out, 1), 0, "empty after skip-all");
}

static void test_counter_wrap(void)
{
    ringbuf_t rb;
    uint8_t out[CAP];
    const char *msg = "wrap-safe";

    TEST("free-running counter wrap-around");
    rb_init(&rb, storage, CAP);
    /* Simulate counters about to wrap past SIZE_MAX (32-bit M3 behavior
     * would wrap identically; host size_t may be 64-bit — the arithmetic
     * is width-agnostic). */
    rb.rd = (size_t)-8;
    rb.wr = (size_t)-8;
    CHECK_EQI(rb_used(&rb), 0, "used 0 across the wrap point");
    CHECK_EQI(rb_free(&rb), CAP, "free CAP across the wrap point");
    CHECK_EQI(rb_write(&rb, msg, 9), 9, "write after wrap");
    CHECK_EQI(rb_used(&rb), 9, "used correct after wrap");
    CHECK_EQI(rb_read(&rb, out, 9), 9, "read after wrap");
    CHECK(memcmp(out, msg, 9) == 0, "payload intact after wrap");
}

int main(void)
{
    test_init();
    test_basic();
    test_underflow();
    test_overflow();
    test_wrap();
    test_skip();
    test_counter_wrap();

    if (failures == 0)
        printf("test_ringbuf: ALL PASS\n");
    else
        printf("test_ringbuf: %d FAILURES\n", failures);
    return failures;
}
