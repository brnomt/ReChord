/*
 * test_mod_eq.c — host test for the mod_eq parametric EQ effect module.
 *
 * Verifies:
 *   1. the 3 bands boost their own region RELATIVE to a far frequency
 *      (low -> 50 Hz vs 1 kHz, mid -> 1 kHz vs 100 Hz, high -> 12 kHz vs
 *      1 kHz), mirroring the relative-assertion style of test_dsp_core.c
 *      (the core's 0.9 headroom pre-scale makes absolute levels quieter);
 *   2. default (0 dB) is ~0.9x passthrough — the documented headroom rule;
 *   3. set_param/get_param contract: round-trip, range and unknown-param
 *      errors;
 *   4. BIT-LEVEL parity with the rechord_dsp_core public API on pseudo-
 *      random input: mod_eq is a per-instance instantiation of the core's
 *      biquad engine, so it must agree with the core to rounding.
 *
 * Compile (host):
 *   cc -Wall -Werror -O2 -o test_mod_eq \
 *      firmware/dsp/tests/test_mod_eq.c firmware/dsp/mod_eq.c \
 *      firmware/rechord_dsp_core.c -lm
 */
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "../dsp_module.h"
#include "../../rechord_dsp_core.h"

#define PARITY_N 8192

static int16_t buf[T_SAMPLES];
static int16_t ref[T_SAMPLES];

/* Run the module on a fresh copy of `in`; return steady-state RMS. */
static double run_sine(const dsp_module_t *m, void *st,
                       double freq, double amp)
{
    gen_sine16(buf, T_FRAMES, freq, amp);
    memcpy(ref, buf, sizeof buf);
    m->process(st, buf, T_SAMPLES);
    return rms16_steady(buf, T_SAMPLES);
}

int main(void)
{
    DSP_STATE_STORAGE(st, 512);
    const dsp_module_t *m = &dsp_mod_eq;
    double low_ratio, mid_ratio, high_ratio, ratio, in_rms;

    printf("mod_eq: state_size = %d bytes\n", m->state_size);
    CHECK(m->state_size > 0 && m->state_size <= 512, "state_size fits 512B");
    CHECK(m->init(st.bytes, T_FS) == 0, "init() accepts 48 kHz");

    /* ---- 1. per-band relative boost ------------------------------- */
    CHECK(m->set_param(st.bytes, MOD_EQ_LOW, 120) == 0, "set LOW = +12 dB");
    low_ratio = run_sine(m, st.bytes, 50.0, 0.2) /
                run_sine(m, st.bytes, 1000.0, 0.2);
    CHECK(low_ratio > 2.0, "low shelf: 50 Hz >> 1 kHz");
    CHECK(m->set_param(st.bytes, MOD_EQ_LOW, 0) == 0, "reset LOW");

    CHECK(m->set_param(st.bytes, MOD_EQ_MID, 120) == 0, "set MID = +12 dB");
    mid_ratio = run_sine(m, st.bytes, 1000.0, 0.2) /
                run_sine(m, st.bytes, 100.0, 0.2);
    CHECK(mid_ratio > 2.0, "mid peaking: 1 kHz >> 100 Hz");
    CHECK(m->set_param(st.bytes, MOD_EQ_MID, 0) == 0, "reset MID");

    CHECK(m->set_param(st.bytes, MOD_EQ_HIGH, 120) == 0, "set HIGH = +12 dB");
    high_ratio = run_sine(m, st.bytes, 12000.0, 0.2) /
                 run_sine(m, st.bytes, 1000.0, 0.2);
    CHECK(high_ratio > 2.0, "high shelf: 12 kHz >> 1 kHz");
    CHECK(m->set_param(st.bytes, MOD_EQ_HIGH, 0) == 0, "reset HIGH");
    CHECK(m->set_param(st.bytes, MOD_EQ_MID, 0) == 0, "reset MID");

    /* ---- 2. default is headroom passthrough (~0.9x) ---------------- */
    gen_sine16(buf, T_FRAMES, 500.0, 0.5);
    memcpy(ref, buf, sizeof buf);
    m->process(st.bytes, buf, T_SAMPLES);
    in_rms = rms16_steady(ref, T_SAMPLES);
    ratio = rms16_steady(buf, T_SAMPLES) / in_rms;
    CHECK(ratio > 0.85 && ratio < 0.95,
          "0 dB settings pass through at ~0.9x (headroom rule)");

    /* ---- 3. parameter contract ------------------------------------- */
    CHECK(m->set_param(st.bytes, MOD_EQ_MID, -75) == 0 &&
          m->get_param(st.bytes, MOD_EQ_MID) == -75,
          "set/get round-trip (-7.5 dB)");
    CHECK(m->set_param(st.bytes, MOD_EQ_MID, 201) == -2,
          "out-of-range gain is rejected");
    CHECK(m->set_param(st.bytes, 99, 0) == -1, "unknown param is rejected");
    CHECK(m->get_param(st.bytes, 99) == INT_MIN,
          "get_param() error is INT_MIN");
    CHECK(m->set_param(st.bytes, MOD_EQ_MID, 0) == 0, "reset MID again");

    /* ---- 4. parity with the core public API ------------------------ */
    {
        static int16_t in16[PARITY_N];
        static int16_t mod_out[PARITY_N];
        static int32_t core_out[PARITY_N];
        rch_dsp_config_t cfg;
        int i, maxdiff = 0;

        gen_pseudo_random16(in16, PARITY_N, 20261009u);

        memcpy(mod_out, in16, sizeof in16);
        m->init(st.bytes, T_FS);
        m->set_param(st.bytes, MOD_EQ_LOW, 30);    /* +3.0 dB */
        m->set_param(st.bytes, MOD_EQ_MID, 60);    /* +6.0 dB */
        m->set_param(st.bytes, MOD_EQ_HIGH, -20);  /* -2.0 dB */
        m->process(st.bytes, mod_out, PARITY_N);

        memset(&cfg, 0, sizeof cfg);
        cfg.mode = RCH_DSP_PARAM_EQ;
        cfg.db_gain[0] = 3.0;
        cfg.db_gain[1] = 0.0;
        cfg.db_gain[2] = 6.0;
        cfg.db_gain[3] = 0.0;
        cfg.db_gain[4] = -2.0;
        cfg.sample_rate = (unsigned long)T_FS;
        cfg.channels = 2;
        rch_dsp_reset();
        CHECK(rch_dsp_configure(&cfg) == 0, "core configured as reference");
        for (i = 0; i < PARITY_N; i++)
            core_out[i] = ((int32_t)in16[i]) << 8;   /* int16 -> 24-bit */
        rch_dsp_process(core_out, PARITY_N);

        for (i = 0; i < PARITY_N; i++) {
            int ref16 = core_out[i] >> 8;          /* truncation reference */
            int diff = (int)mod_out[i] - ref16;
            if (diff < 0) diff = -diff;
            if (diff > maxdiff) maxdiff = diff;
        }
        printf("  info: max |mod_eq - core| = %d LSB\n", maxdiff);
        CHECK(maxdiff <= 1,
              "mod_eq matches rechord_dsp_core bit-for-bit (<=1 LSB rounding)");
    }

    printf("\n%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
