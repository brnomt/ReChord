/*
 * test_eq_ipc.c — host tests for the AP <-> Core B EQ seqlock (eq_ipc.h).
 */
#include <stdint.h>
#include <string.h>

#include "eq_ipc.h"
#include "test_util.h"

static const uint16_t iso_freq[10] = {
    31, 62, 125, 250, 500, 1000, 2000, 4000, 8000, 16000
};

static void curve_flat10(eq_curve_t *c)
{
    int i;

    c->count = 10;
    c->enabled = 1;
    for (i = 0; i < 10; i++) {
        c->band[i].freq_hz = iso_freq[i];
        c->band[i].q_x100 = 70;
        c->band[i].gain_tdb = 0;
    }
}

static int curves_equal(const eq_curve_t *a, const eq_curve_t *b)
{
    int i;

    if (a->count != b->count || a->enabled != b->enabled)
        return 0;
    for (i = 0; i < a->count; i++) {
        if (a->band[i].freq_hz != b->band[i].freq_hz ||
            a->band[i].q_x100 != b->band[i].q_x100 ||
            a->band[i].gain_tdb != b->band[i].gain_tdb)
            return 0;
    }
    return 1;
}

static void test_pack(void)
{
    eq_band_t in, out;
    uint32_t w;

    TEST("band packing");
    in.freq_hz = 20000;
    in.q_x100 = 400;
    in.gain_tdb = 120;
    w = eq_ipc_pack_band(&in);
    eq_ipc_unpack_band(w, &out);
    CHECK(in.freq_hz == out.freq_hz && in.q_x100 == out.q_x100 &&
          in.gain_tdb == out.gain_tdb, "max values round-trip");

    in.freq_hz = 31;
    in.q_x100 = 10;
    in.gain_tdb = -120;
    w = eq_ipc_pack_band(&in);
    eq_ipc_unpack_band(w, &out);
    CHECK(in.freq_hz == out.freq_hz && in.q_x100 == out.q_x100 &&
          in.gain_tdb == out.gain_tdb, "min values round-trip");

    CHECK((eq_ipc_pack_band(&(eq_band_t){ 1000, 70, 0 }) & 0x7FFFu) == 1000,
          "frequency occupies the low 15 bits");
}

static void test_publish_read(void)
{
    volatile uint32_t sh[IPC_SH_COUNT];
    eq_curve_t c, got;
    uint32_t gen = 0;

    TEST("publish / read");
    memset((void *)sh, 0, sizeof sh);

    CHECK(eq_ipc_read(sh, &gen, &got) == 0, "untouched block reads nothing");

    curve_flat10(&c);
    c.band[5].gain_tdb = -45;
    c.band[9].gain_tdb = 60;
    eq_ipc_publish(sh, &c);

    CHECK((sh[IPC_C_EQ_GEN] & 1u) == 0, "published generation is even");
    CHECK(eq_ipc_read(sh, &gen, &got) == 1, "new curve is read");
    CHECK(curves_equal(&c, &got), "curve round-trips through the block");
    CHECK(eq_ipc_read(sh, &gen, &got) == 0, "same generation is not re-read");

    c.enabled = 0;
    eq_ipc_publish(sh, &c);
    CHECK(eq_ipc_read(sh, &gen, &got) == 1, "bypass update is read");
    CHECK(got.enabled == 0, "bypass flag survives");

    c.count = 5;
    c.enabled = 1;
    eq_ipc_publish(sh, &c);
    CHECK(eq_ipc_read(sh, &gen, &got) == 1, "5-band curve is read");
    CHECK(got.count == 5, "band count survives");
}

static void test_odd_generation(void)
{
    volatile uint32_t sh[IPC_SH_COUNT];
    eq_curve_t c, got;
    uint32_t gen = 0;

    TEST("torn update");
    memset((void *)sh, 0, sizeof sh);
    curve_flat10(&c);
    eq_ipc_publish(sh, &c);
    sh[IPC_C_EQ_GEN] |= 1u;              /* writer mid-update */
    CHECK(eq_ipc_read(sh, &gen, &got) == 0, "odd generation is ignored");

    sh[IPC_C_EQ_GEN] &= ~1u;             /* stable again, same value */
    CHECK(eq_ipc_read(sh, &gen, &got) == 1, "stable generation is read");
}

static void test_bad_count(void)
{
    volatile uint32_t sh[IPC_SH_COUNT];
    eq_curve_t got;
    uint32_t gen = 0;

    TEST("malformed curve");
    memset((void *)sh, 0, sizeof sh);
    sh[IPC_C_EQ_GEN] = 2;
    sh[IPC_C_EQ_EN] = 1;
    sh[IPC_C_EQ_N] = EQ_BANDS_MAX + 1;
    CHECK(eq_ipc_read(sh, &gen, &got) == -1, "count above the cap is rejected");
    CHECK(eq_ipc_read(sh, &gen, &got) == 0, "rejected generation is marked seen");
}

int main(void)
{
    printf("=== eq_ipc tests ===\n");
    test_pack();
    test_publish_read();
    test_odd_generation();
    test_bad_count();

    printf("\n%s (%d failure%s)\n",
           failures == 0 ? "ALL EQ IPC TESTS PASSED" : "FAILURES",
           failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
