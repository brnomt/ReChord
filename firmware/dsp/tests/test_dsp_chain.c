/*
 * test_dsp_chain.c — host test for the dsp_chain effect chain.
 *
 * Verifies:
 *   1. ORDERING matters: the same two volume modules in different order give
 *      different output (saturation makes gain stages non-commutative);
 *   2. BYPASS is a byte-exact passthrough and matches a chain built without
 *      the bypassed module;
 *   3. set-by-name / get-by-name work case-insensitively and reject unknown
 *      names (config-file driven user chains);
 *   4. add-time validation: chain capacity, state size, alignment, module
 *      init failure — all fail cleanly without corrupting the chain.
 *
 * Compile (host):
 *   cc -Wall -Werror -O2 -o test_dsp_chain \
 *      firmware/dsp/tests/test_dsp_chain.c firmware/dsp/dsp_chain.c \
 *      firmware/dsp/mod_eq.c firmware/dsp/mod_bass.c \
 *      firmware/dsp/mod_volume.c firmware/rechord_dsp_core.c -lm
 */
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "test_util.h"
#include "../dsp_chain.h"

static int16_t input[T_SAMPLES];
static int16_t out_a[T_SAMPLES];
static int16_t out_b[T_SAMPLES];

static void fill_fullscale(void)
{
    gen_dc16(input, T_FRAMES, 32767);
}

/* A module whose init always fails — for add-time error coverage. */
static int fail_init(void *state, int sample_rate)
{
    (void)state;
    (void)sample_rate;
    return -1;
}
static int stub_process(void *state, int16_t *samples, int n)
{
    (void)state; (void)samples; (void)n;
    return 0;
}
static int stub_set_param(void *state, int param, int value)
{
    (void)state; (void)param; (void)value;
    return -1;
}
static int stub_get_param(void *state, int param)
{
    (void)state; (void)param;
    return INT_MIN;
}
static const dsp_module_t fail_module = {
    "fail", fail_init, stub_process, stub_set_param, stub_get_param, 16
};

int main(void)
{
    dsp_chain_t chain_a, chain_b, chain_eq_only;
    DSP_STATE_STORAGE(vol1, 256);
    DSP_STATE_STORAGE(vol2, 256);
    DSP_STATE_STORAGE(eqst, 512);
    DSP_STATE_STORAGE(eqst2, 512);

    /* ---- 1. ordering: [+] then [-] vs [-] then [+] ----------------- */
    fill_fullscale();

    dsp_chain_init(&chain_a, T_FS);
    CHECK(dsp_chain_add(&chain_a, &dsp_mod_volume, vol1.bytes, sizeof vol1.bytes) == 0,
          "chain A: add volume #1");
    CHECK(dsp_chain_add(&chain_a, &dsp_mod_volume, vol2.bytes, sizeof vol2.bytes) == 1,
          "chain A: add volume #2");
    CHECK(dsp_chain_set_param_at(&chain_a, 0, MOD_VOLUME_RAMP_MS, 0) == 0 &&
          dsp_chain_set_param_at(&chain_a, 1, MOD_VOLUME_RAMP_MS, 0) == 0,
          "chain A: instant ramps");
    CHECK(dsp_chain_set_param_at(&chain_a, 0, MOD_VOLUME_GAIN, 60) == 0 &&
          dsp_chain_set_param_at(&chain_a, 1, MOD_VOLUME_GAIN, -60) == 0,
          "chain A: +6 dB then -6 dB");
    memcpy(out_a, input, sizeof input);
    CHECK(dsp_chain_process(&chain_a, out_a, T_SAMPLES) == 0,
          "chain A processes");

    dsp_chain_init(&chain_b, T_FS);
    CHECK(dsp_chain_add(&chain_b, &dsp_mod_volume, vol1.bytes, sizeof vol1.bytes) == 0,
          "chain B: add volume #1");
    CHECK(dsp_chain_add(&chain_b, &dsp_mod_volume, vol2.bytes, sizeof vol2.bytes) == 1,
          "chain B: add volume #2");
    dsp_chain_set_param_at(&chain_b, 0, MOD_VOLUME_RAMP_MS, 0);
    dsp_chain_set_param_at(&chain_b, 1, MOD_VOLUME_RAMP_MS, 0);
    CHECK(dsp_chain_set_param_at(&chain_b, 0, MOD_VOLUME_GAIN, -60) == 0 &&
          dsp_chain_set_param_at(&chain_b, 1, MOD_VOLUME_GAIN, 60) == 0,
          "chain B: -6 dB then +6 dB");
    memcpy(out_b, input, sizeof input);
    CHECK(dsp_chain_process(&chain_b, out_b, T_SAMPLES) == 0,
          "chain B processes");

    printf("  info: A[+,-] final = %d, B[-,+] final = %d\n",
           (int)out_a[T_SAMPLES - 1], (int)out_b[T_SAMPLES - 1]);
    CHECK(out_a[T_SAMPLES - 1] < 20000,
          "chain A (+6 then -6) lands at half scale (clipped in between)");
    CHECK(out_b[T_SAMPLES - 1] > 32000,
          "chain B (-6 then +6) lands at full scale");
    CHECK(out_a[T_SAMPLES - 1] != out_b[T_SAMPLES - 1],
          "module order changes the result");

    /* ---- 2. bypass is byte-exact and matches a shorter chain ------- */
    {
        dsp_chain_t chain;
        DSP_STATE_STORAGE(bv1, 256);
        static int16_t eq_ref[T_SAMPLES];

        dsp_chain_init(&chain, T_FS);
        dsp_chain_add(&chain, &dsp_mod_eq, eqst.bytes, sizeof eqst.bytes);
        dsp_chain_add(&chain, &dsp_mod_volume, bv1.bytes, sizeof bv1.bytes);
        dsp_chain_set_param(&chain, "eq", MOD_EQ_MID, 60);
        dsp_chain_set_param(&chain, "volume", MOD_VOLUME_RAMP_MS, 0);
        dsp_chain_set_param(&chain, "volume", MOD_VOLUME_GAIN, 60);

        dsp_chain_init(&chain_eq_only, T_FS);
        dsp_chain_add(&chain_eq_only, &dsp_mod_eq, eqst2.bytes, sizeof eqst2.bytes);
        dsp_chain_set_param(&chain_eq_only, "eq", MOD_EQ_MID, 60);

        /* Compare from IDENTICAL fresh filter history: both chains run the
         * same input through the same eq instance state now. */
        CHECK(dsp_chain_set_bypass(&chain, "volume", 1) == 0,
              "bypass enabled by name");
        CHECK(dsp_chain_get_bypass_at(&chain, 1) == 1,
              "bypass flag is readable");

        memcpy(out_a, input, sizeof input);
        dsp_chain_process(&chain, out_a, T_SAMPLES);
        memcpy(eq_ref, input, sizeof input);
        dsp_chain_process(&chain_eq_only, eq_ref, T_SAMPLES);
        CHECK(memcmp(out_a, eq_ref, sizeof out_a) == 0,
              "bypassed chain == chain without the module (byte-exact)");

        /* Re-enabling the module changes the output again. */
        CHECK(dsp_chain_set_bypass(&chain, "volume", 0) == 0,
              "bypass disabled again");
        memcpy(out_b, input, sizeof input);
        dsp_chain_process(&chain, out_b, T_SAMPLES);
        CHECK(memcmp(out_b, out_a, sizeof out_b) != 0,
              "active module changes the output");

        /* Everything bypassed == input untouched. */
        dsp_chain_set_bypass(&chain, "eq", 1);
        dsp_chain_set_bypass(&chain, "volume", 1);
        memcpy(out_b, input, sizeof input);
        dsp_chain_process(&chain, out_b, T_SAMPLES);
        CHECK(memcmp(out_b, input, sizeof input) == 0,
              "fully bypassed chain is byte-exact passthrough");
    }

    /* ---- 3. set-by-name contract ----------------------------------- */
    {
        dsp_chain_t chain;
        DSP_STATE_STORAGE(es, 512);
        DSP_STATE_STORAGE(vs, 256);
        DSP_STATE_STORAGE(bs, 512);

        dsp_chain_init(&chain, T_FS);
        dsp_chain_add(&chain, &dsp_mod_eq, es.bytes, sizeof es.bytes);
        dsp_chain_add(&chain, &dsp_mod_bass, bs.bytes, sizeof bs.bytes);
        dsp_chain_add(&chain, &dsp_mod_volume, vs.bytes, sizeof vs.bytes);

        CHECK(dsp_chain_count(&chain) == 3, "chain holds 3 modules");
        CHECK(strcmp(dsp_chain_name_at(&chain, 0), "eq") == 0 &&
              strcmp(dsp_chain_name_at(&chain, 2), "volume") == 0,
              "name_at() reports insertion order");

        CHECK(dsp_chain_set_param(&chain, "EQ", MOD_EQ_MID, 120) == 0 &&
              dsp_chain_get_param(&chain, "eq", MOD_EQ_MID) == 120,
              "set/get by name is case-insensitive");
        CHECK(dsp_chain_set_param(&chain, "bass", MOD_BASS_BOOST, 55) == 0 &&
              dsp_chain_get_param(&chain, "bass", MOD_BASS_BOOST) == 55,
              "bass params flow through the chain");
        CHECK(dsp_chain_set_param(&chain, "volume", MOD_VOLUME_GAIN, -30) == 0 &&
              dsp_chain_get_param(&chain, "volume", MOD_VOLUME_GAIN) == -30,
              "volume params flow through the chain");
        CHECK(dsp_chain_set_param(&chain, "reverb", 0, 0) == -1,
              "unknown module name is rejected");
        CHECK(dsp_chain_get_param(&chain, "reverb", 0) == INT_MIN,
              "get_param by unknown name returns INT_MIN");
        CHECK(dsp_chain_set_param(&chain, "eq", 77, 0) == -1,
              "unknown param propagates the module's error");
    }

    /* ---- 4. add-time validation ------------------------------------ */
    {
        dsp_chain_t chain;
        static uint8_t tiny[8];
        /* Union forces 8-byte alignment so +1 is guaranteed misaligned. */
        union { double d; uint8_t b[521]; } pad;
        uint8_t *off_by_one = pad.b + 1;
        int i, rc;

        dsp_chain_init(&chain, T_FS);

        for (i = 0; i < DSP_CHAIN_MAX_MODULES; i++) {
            rc = dsp_chain_add(&chain, &dsp_mod_volume, vol1.bytes,
                               sizeof vol1.bytes);
            CHECK(rc == i, "fill chain to capacity");
        }
        CHECK(dsp_chain_add(&chain, &dsp_mod_volume, vol1.bytes,
                            sizeof vol1.bytes) == -1,
              "module beyond capacity is rejected");

        dsp_chain_init(&chain, T_FS);
        CHECK(dsp_chain_add(&chain, &dsp_mod_eq, tiny, (int)sizeof tiny) == -2,
              "state buffer smaller than state_size is rejected");
        CHECK(dsp_chain_add(&chain, &dsp_mod_eq, off_by_one, 512) == -3,
              "misaligned state buffer is rejected");
        CHECK(dsp_chain_add(&chain, &fail_module, vol1.bytes,
                            sizeof vol1.bytes) == -4,
              "failing module init is reported");
        CHECK(dsp_chain_count(&chain) == 0,
              "failed adds leave the chain empty");
    }

    printf("\n%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
