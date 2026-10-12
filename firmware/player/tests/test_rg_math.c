/*
 * test_rg_math.c — host unit tests for the ReplayGain fixed-point helpers.
 */
#include "rg_math.h"
#include "test_util.h"

#include <stdint.h>

static void test_gain_table(void)
{
    /* 0 dB -> unity gain (1.0x = 32768 in Q16). */
    CHECK_EQI(rg_gain_q16(0), 32768, "0 dB => 1.0x (Q16 32768)");

    /* -20 dB => factor 0.1 (32768 * 0.1 = 3276.8 -> 3276). */
    CHECK(rg_gain_q16(-200) >= 3276 && rg_gain_q16(-200) <= 3277,
          "-20 dB => ~0.1x");

    /* +20 dB => factor 10 (327680). */
    CHECK_EQI(rg_gain_q16(200), 327680, "+20 dB => 10x");

    /* -60 dB => factor 0.001 (-> 32). */
    CHECK(rg_gain_q16(-600) >= 32 && rg_gain_q16(-600) <= 33,
          "-60 dB => ~0.001x");
}

static void test_peak_limit(void)
{
    /* A +60 dB boost (gain 32768000) with a peak of 1.0 must be clamped to
     * unity (32768) so a full-scale track does not clip. */
    CHECK_EQI(rg_effective_gain_q16(600, 32768), 32768,
              "+60 dB + peak 1.0 => clamped to 1.0x (Q1.15 32768)");

    /* No peak info (peak_q15 == 0) => the dB gain is used as-is. */
    CHECK_EQI(rg_effective_gain_q16(-200, 0), rg_gain_q16(-200),
              "no peak => raw dB gain");
}

static void test_scale_roundtrip(void)
{
    int16_t s[] = { 1000, -1000, 30000, -30000 };
    /* unity gain must be a no-op. */
    rg_scale_pcm(s, 4, 32768);
    CHECK_EQI(s[0], 1000, "unity: +1000");
    CHECK_EQI(s[1], -1000, "unity: -1000");
    CHECK_EQI(s[2], 30000, "unity: +30000");
    CHECK_EQI(s[3], -30000, "unity: -30000");

    /* 0 gain => silence. */
    rg_scale_pcm(s, 4, 0);
    CHECK_EQI(s[0], 0, "zero gain => silent");

    /* +20 dB (gain 327680) => 10x, saturating at 32767. */
    s[0] = 1000; s[1] = -1000; s[2] = 30000; s[3] = -30000;
    rg_scale_pcm(s, 4, 327680);
    CHECK_EQI(s[0], 10000, "+20 dB: 1000 * 10");
    CHECK_EQI(s[2], 32767, "+20 dB: 30000 saturates to 32767");
}

int main(void)
{
    TEST("rg_math");
    test_gain_table();
    test_peak_limit();
    test_scale_roundtrip();
    if (failures == 0) {
        printf("test_rg_math: OK\n");
        return 0;
    }
    printf("test_rg_math: %d FAILURE(S)\n", failures);
    return failures;
}
