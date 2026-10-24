/*
 * test_eq_bank.c — host tests for the runtime EQ bank (firmware/dsp/eq_bank).
 *
 * Signal is the real pipeline format: interleaved int32 left-justified
 * 24-bit, so full scale is 1 << 23.
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../eq_bank.h"
#include "test_util.h"

#define PI32 3.14159265358979323846
#define FS24 (1 << 23)
#define NFRAMES 24000
#define NSAMP (NFRAMES * 2)

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

static void gen_sine32(int32_t *b, int frames, double freq, double amp)
{
    int i;

    for (i = 0; i < frames; i++) {
        double s = amp * (double)FS24 *
                   sin(2.0 * PI32 * freq * (double)i / 48000.0);

        b[2 * i] = b[2 * i + 1] = (int32_t)s;
    }
}

static void gen_dc32(int32_t *b, int frames, int32_t level)
{
    int i;

    for (i = 0; i < frames; i++)
        b[2 * i] = b[2 * i + 1] = level;
}

static double rms32_steady(const int32_t *b, int n)
{
    double s = 0.0;
    int i, start = n / 2;

    for (i = start; i < n; i++) {
        double v = (double)b[i] / (double)FS24;

        s += v * v;
    }
    return (n > start) ? sqrt(s / (double)(n - start)) : 0.0;
}

static int buffers_equal(const int32_t *a, const int32_t *b, int n)
{
    int i;

    for (i = 0; i < n; i++)
        if (a[i] != b[i])
            return 0;
    return 1;
}

static void test_validation(void)
{
    eq_curve_t c;

    printf("eq_curve_valid\n");
    curve_flat10(&c);
    CHECK(eq_curve_valid(&c) == 1, "flat 10-band curve is valid");

    c.count = 5;
    CHECK(eq_curve_valid(&c) == 1, "5 bands accepted");

    c.count = 4;
    CHECK(eq_curve_valid(&c) == 0, "4 bands rejected");
    c.count = 11;
    CHECK(eq_curve_valid(&c) == 0, "11 bands rejected");

    curve_flat10(&c);
    c.enabled = 2;
    CHECK(eq_curve_valid(&c) == 0, "enabled must be 0/1");

    curve_flat10(&c);
    c.band[0].freq_hz = 19;
    CHECK(eq_curve_valid(&c) == 0, "freq below 20 Hz rejected");
    c.band[0].freq_hz = 20001;
    CHECK(eq_curve_valid(&c) == 0, "freq above 20 kHz rejected");

    curve_flat10(&c);
    c.band[0].q_x100 = 9;
    CHECK(eq_curve_valid(&c) == 0, "Q below 0.10 rejected");
    c.band[0].q_x100 = 401;
    CHECK(eq_curve_valid(&c) == 0, "Q above 4.00 rejected");

    curve_flat10(&c);
    c.band[0].gain_tdb = 121;
    CHECK(eq_curve_valid(&c) == 0, "gain above +12 dB rejected");
    c.band[0].gain_tdb = -121;
    CHECK(eq_curve_valid(&c) == 0, "gain below -12 dB rejected");

    curve_flat10(&c);
    c.band[1].freq_hz = c.band[0].freq_hz;
    CHECK(eq_curve_valid(&c) == 0, "non-increasing frequencies rejected");
}

static void test_flat_and_bypass(eq_bank_state_t *st)
{
    eq_curve_t c;
    int32_t in[NSAMP], out[NSAMP];

    printf("flat / bypass\n");
    gen_sine32(in, NFRAMES, 1000.0, 0.5);

    curve_flat10(&c);
    CHECK(eq_bank_apply(st, &c) == 0, "flat curve applies");
    eq_bank_reset(st);
    memcpy(out, in, sizeof in);
    eq_bank_process(st, out, NSAMP);
    CHECK(buffers_equal(in, out, NSAMP), "flat curve is a bit-exact pass-through");

    curve_flat10(&c);
    c.band[5].gain_tdb = 60;
    c.enabled = 0;
    CHECK(eq_bank_apply(st, &c) == 0, "bypassed curve applies");
    memcpy(out, in, sizeof in);
    eq_bank_process(st, out, NSAMP);
    CHECK(buffers_equal(in, out, NSAMP), "disabled EQ is a bit-exact pass-through");
}

static void test_gain(eq_bank_state_t *st)
{
    eq_curve_t c;
    int32_t in[NSAMP], out[NSAMP];
    double ratio;

    printf("peaking gain\n");

    /* -6 dB at 1 kHz: no headroom scaling (the curve never boosts). */
    curve_flat10(&c);
    c.band[5].gain_tdb = -60;
    CHECK(eq_bank_apply(st, &c) == 0, "-6 dB curve applies");
    eq_bank_reset(st);
    gen_sine32(in, NFRAMES, 1000.0, 0.25);
    memcpy(out, in, sizeof in);
    eq_bank_process(st, out, NSAMP);
    ratio = rms32_steady(out, NSAMP) / rms32_steady(in, NSAMP);
    CHECK(fabs(ratio - 0.5012) < 0.02, "-6 dB at 1 kHz measures ~0.50x");

    /* +6 dB at 1 kHz: the 0.9/max-gain headroom makes the net 0.9x. */
    curve_flat10(&c);
    c.band[5].gain_tdb = 60;
    CHECK(eq_bank_apply(st, &c) == 0, "+6 dB curve applies");
    eq_bank_reset(st);
    gen_sine32(in, NFRAMES, 1000.0, 0.25);
    memcpy(out, in, sizeof in);
    eq_bank_process(st, out, NSAMP);
    ratio = rms32_steady(out, NSAMP) / rms32_steady(in, NSAMP);
    CHECK(fabs(ratio - 0.9) < 0.03, "+6 dB with headroom measures ~0.9x");
}

static void test_no_clip(eq_bank_state_t *st)
{
    eq_curve_t c;
    int32_t in[NSAMP], out[NSAMP];
    int i, peak = 0, wrapped = 0;

    printf("headroom\n");
    curve_flat10(&c);
    c.band[5].gain_tdb = 120;         /* +12 dB */
    CHECK(eq_bank_apply(st, &c) == 0, "+12 dB curve applies");
    eq_bank_reset(st);
    gen_sine32(in, NFRAMES, 1000.0, 1.0);   /* full scale */
    memcpy(out, in, sizeof in);
    eq_bank_process(st, out, NSAMP);

    for (i = 0; i < NSAMP; i++) {
        int v = out[i] < 0 ? -out[i] : out[i];

        if (v > peak)
            peak = v;
        if ((in[i] > 0 && out[i] < -FS24 / 2) ||
            (in[i] < 0 && out[i] > FS24 / 2))
            wrapped = 1;
    }
    CHECK(peak <= FS24, "boosted full-scale signal stays inside 24-bit range");
    CHECK(wrapped == 0, "boosted signal does not wrap polarity");
}

static void test_nyquist_skip(eq_bank_state_t *st)
{
    eq_curve_t c;
    int32_t in[4096], out[4096];

    printf("Nyquist handling\n");
    gen_sine32(in, 2048, 1000.0, 0.25);
    curve_flat10(&c);
    c.band[9].gain_tdb = 60;          /* 16 kHz band */
    CHECK(eq_bank_init(st, 8000) == 0, "re-init at 8 kHz");
    CHECK(eq_bank_apply(st, &c) == 0, "curve still validates at 8 kHz");
    memcpy(out, in, sizeof in);
    eq_bank_process(st, out, 4096);
    CHECK(buffers_equal(in, out, 4096), "bands at/above Nyquist are ignored");
    (void)eq_bank_init(st, 48000);
}

static void test_atomic_reject(eq_bank_state_t *st)
{
    eq_curve_t good, bad;
    int32_t in[4096], a[4096], b[4096];

    printf("atomic update\n");
    curve_flat10(&good);
    good.band[5].gain_tdb = 60;
    CHECK(eq_bank_apply(st, &good) == 0, "good curve applies");

    gen_sine32(in, 2048, 1000.0, 0.25);
    memcpy(a, in, sizeof a);
    eq_bank_reset(st);
    eq_bank_process(st, a, 4096);

    bad = good;
    bad.band[2].freq_hz = 1;          /* invalid, below the 20 Hz floor */
    CHECK(eq_bank_apply(st, &bad) < 0, "invalid curve is rejected");
    CHECK(eq_bank_curve(st)->band[5].gain_tdb == 60,
          "rejected curve leaves the active curve untouched");

    memcpy(b, in, sizeof b);
    eq_bank_reset(st);
    eq_bank_process(st, b, 4096);
    CHECK(buffers_equal(a, b, 4096), "rejected curve leaves the output untouched");
}

static void test_retune_continuity(eq_bank_state_t *st)
{
    eq_curve_t c;
    int32_t b1[2048], b2[2048];
    int32_t last, first;
    long delta;

    printf("retune continuity\n");
    curve_flat10(&c);
    c.band[5].gain_tdb = 60;
    CHECK(eq_bank_apply(st, &c) == 0, "initial curve applies");
    eq_bank_reset(st);

    gen_dc32(b1, 1024, FS24 / 10);
    eq_bank_process(st, b1, 2048);
    last = b1[2046];

    c.band[5].gain_tdb = 30;          /* gain-only change, same geometry */
    CHECK(eq_bank_apply(st, &c) == 0, "gain-only retune applies");
    gen_dc32(b2, 1024, FS24 / 10);
    eq_bank_process(st, b2, 2048);
    first = b2[0];

    delta = (long)first - (long)last;
    if (delta < 0)
        delta = -delta;
    CHECK(delta < (long)(FS24 / 2),
          "gain-only retune keeps the filter history (no jump)");
}

int main(void)
{
    eq_bank_state_t st;

    printf("=== eq_bank tests ===\n");
    if (eq_bank_init(&st, 48000) != 0) {
        printf("  FAIL: init\n");
        return 1;
    }

    test_validation();
    test_flat_and_bypass(&st);
    test_gain(&st);
    test_no_clip(&st);
    test_nyquist_skip(&st);
    test_atomic_reject(&st);
    test_retune_continuity(&st);

    printf("\n%s (%d failure%s)\n",
           failures == 0 ? "ALL EQ BANK TESTS PASSED" : "FAILURES",
           failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
