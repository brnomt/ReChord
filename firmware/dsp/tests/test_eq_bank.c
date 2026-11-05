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
        c->band[i].type = EQ_TYPE_AUTO;
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

    /* +6 dB at 1 kHz: the preamp backs off max(mean boost, 40 % of the peak),
     * here ~2.4 dB, so the tone nets ~+3.6 dB instead of being cancelled. */
    curve_flat10(&c);
    c.band[5].gain_tdb = 60;
    CHECK(eq_bank_apply(st, &c) == 0, "+6 dB curve applies");
    eq_bank_reset(st);
    gen_sine32(in, NFRAMES, 1000.0, 0.25);
    memcpy(out, in, sizeof in);
    eq_bank_process(st, out, NSAMP);
    ratio = rms32_steady(out, NSAMP) / rms32_steady(in, NSAMP);
    CHECK(ratio > 1.40 && ratio < 1.65, "+6 dB nets about +3.6 dB after the preamp");
}

static void test_no_clip(eq_bank_state_t *st)
{
    /* Core B hands the engine LEFT-justified int32 (24-bit << 8), so true full
     * scale is 2^31. A boosted peak must clip softly, never wrap to the other
     * sign (that is a loud digital crack). */
    eq_curve_t c;
    static int32_t in[NSAMP], out[NSAMP];
    int i, wrapped = 0, loud = 0;
    long long peak = 0;

    printf("headroom\n");
    curve_flat10(&c);
    c.band[5].gain_tdb = 120;         /* +12 dB at 1 kHz */
    CHECK(eq_bank_apply(st, &c) == 0, "+12 dB curve applies");
    eq_bank_reset(st);
    for (i = 0; i < NFRAMES; i++)
        in[2 * i] = in[2 * i + 1] =
            (int32_t)(0.999 * 2147483647.0 *
                      sin(2.0 * PI32 * 1000.0 * (double)i / 48000.0));
    memcpy(out, in, sizeof in);
    eq_bank_process(st, out, NSAMP);

    for (i = 4; i < NSAMP; i++) {
        long long v = out[i] < 0 ? -(long long)out[i] : out[i];
        long long step = (long long)out[i] - out[i - 2];

        if (v > peak)
            peak = v;
        if (step < 0)
            step = -step;
        if (step > 1500000000LL)        /* a 1 kHz sine never jumps this far */
            wrapped = 1;
        if (v > 1717986918LL)
            loud++;
    }
    CHECK(wrapped == 0, "boosted full-scale signal does not wrap");
    CHECK(peak <= 2147483647LL, "boosted full-scale signal stays in range");
    CHECK(loud > 0, "the soft knee still lets peaks reach the top");
}

static void test_overlap_no_wrap(eq_bank_state_t *st)
{
    /* Neighbouring boosts add up (here ~+20 dB between them). The old preamp
     * only looked at one band, so this folded over (digital crack). */
    eq_curve_t c;
    static int32_t out[NSAMP];
    int i, wrapped = 0;

    printf("overlapping boosts\n");
    curve_flat10(&c);
    c.band[3].gain_tdb = 100;
    c.band[4].gain_tdb = 120;
    c.band[5].gain_tdb = 100;
    CHECK(eq_bank_apply(st, &c) == 0, "overlapping boosts apply");
    eq_bank_reset(st);
    for (i = 0; i < NFRAMES; i++)
        out[2 * i] = out[2 * i + 1] =
            (int32_t)(0.9 * 2147483647.0 *
                      sin(2.0 * PI32 * 500.0 * (double)i / 48000.0));
    eq_bank_process(st, out, NSAMP);
    for (i = 4; i < NSAMP; i++) {
        long long step = (long long)out[i] - out[i - 2];

        if (step < 0)
            step = -step;
        if (step > 1500000000LL)
            wrapped = 1;
    }
    CHECK(wrapped == 0, "overlapping boosts do not wrap");
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

/* BRNOMT-77: an explicit type wins over the band's position. Cut-only curves,
 * so the preamp stays at unity and the tone ratios are the filter alone. */
static double tone_ratio(eq_bank_state_t *st, const eq_curve_t *c, double hz)
{
    static int32_t in[NSAMP], out[NSAMP];

    if (eq_bank_apply(st, c) != 0)
        return -1.0;
    eq_bank_reset(st);
    gen_sine32(in, NFRAMES, hz, 0.25);
    memcpy(out, in, sizeof in);
    eq_bank_process(st, out, NSAMP);
    return rms32_steady(out, NSAMP) / rms32_steady(in, NSAMP);
}

static void test_band_types(eq_bank_state_t *st)
{
    eq_curve_t c;
    double r;

    printf("explicit band types (AutoEQ)\n");

    /* A middle band is a peak by position; LSHELF turns it into a shelf. */
    curve_flat10(&c);
    c.band[4].gain_tdb = -60;                       /* 500 Hz */
    r = tone_ratio(st, &c, 40.0);
    CHECK(r > 0.95 && r < 1.05, "auto middle band: 40 Hz untouched (peak)");
    c.band[4].type = EQ_TYPE_LSHELF;
    r = tone_ratio(st, &c, 40.0);
    CHECK(r > 0.47 && r < 0.55, "LSHELF middle band: 40 Hz cut ~6 dB");
    r = tone_ratio(st, &c, 8000.0);
    CHECK(r > 0.97 && r < 1.03, "LSHELF middle band: 8 kHz untouched");

    /* Band 0 is a low shelf by position; HSHELF there cuts the highs. */
    curve_flat10(&c);
    c.band[0].gain_tdb = -60;                       /* 31 Hz */
    c.band[0].type = EQ_TYPE_HSHELF;
    r = tone_ratio(st, &c, 1000.0);
    CHECK(r > 0.47 && r < 0.55, "HSHELF at band 0: 1 kHz cut ~6 dB");

    /* PEAK on the last band (a high shelf by position). */
    curve_flat10(&c);
    c.band[9].gain_tdb = -60;                       /* 16 kHz */
    c.band[9].freq_hz = 12000;
    c.band[9].type = EQ_TYPE_PEAK;
    c.band[9].q_x100 = 575;                         /* narrow AutoEQ peak */
    r = tone_ratio(st, &c, 3000.0);
    CHECK(r > 0.97 && r < 1.03, "PEAK at the last band: 3 kHz untouched");
    r = tone_ratio(st, &c, 12000.0);
    CHECK(r > 0.47 && r < 0.55, "PEAK at the last band: 12 kHz cut ~6 dB");

    /* Q above 4.0 is only legal with an explicit type. */
    c.band[9].type = EQ_TYPE_AUTO;
    CHECK(!eq_curve_valid(&c), "auto band rejects Q 5.75");
    c.band[9].type = 4;
    CHECK(!eq_curve_valid(&c), "unknown type rejected");
}

/* The block path (band-major) must equal the frame-at-a-time path (one frame
 * per call = sample-major). Loud, boosted, 96 kHz so the soft knee and the
 * saturation both run. */
static void test_block_equals_frames(void)
{
    static int32_t a[NSAMP], b[NSAMP];
    eq_bank_state_t sa, sb;
    eq_curve_t c;
    static const int16_t g[10] = { 90, 30, 0, 0, -10, -10, 0, 20, 40, 40 };
    unsigned seed = 12345u;
    int i, diff = 0;

    curve_flat10(&c);
    for (i = 0; i < 10; i++)
        c.band[i].gain_tdb = g[i];
    for (i = 0; i < NSAMP; i++) {
        seed = seed * 1664525u + 1013904223u;
        a[i] = b[i] = (int32_t)(seed & 0xFFFFFF00u) >> 1;   /* loud noise */
    }

    eq_bank_init(&sa, 96000);
    eq_bank_init(&sb, 96000);
    eq_bank_apply(&sa, &c);
    eq_bank_apply(&sb, &c);

    eq_bank_process(&sa, a, NSAMP);
    for (i = 0; i < NFRAMES; i++)
        eq_bank_process(&sb, b + 2 * i, 2);

    for (i = 0; i < NSAMP; i++)
        diff += (a[i] != b[i]);
    CHECK(diff == 0, "block == frame-at-a-time (bit-exact, 96k boosted)");
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
    test_overlap_no_wrap(&st);
    test_nyquist_skip(&st);
    test_atomic_reject(&st);
    test_retune_continuity(&st);
    test_block_equals_frames();
    test_band_types(&st);

    printf("\n%s (%d failure%s)\n",
           failures == 0 ? "ALL EQ BANK TESTS PASSED" : "FAILURES",
           failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
