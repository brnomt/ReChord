/*
 * test_ao_slots.c — host tests for the audio HAL's pure pieces:
 *  - ao_slot_word(): the wire packing (one 32-bit left-justified channel
 *    sample per FIFO word) must equal the raw expression used by the
 *    hardware feed/ISR in rknano_i2s.c (verbatim-move constraint).
 *  - dac_rates[]: the 13 per-rate entries must keep the byte-exact ref
 *    constants (ref RAM 0x030609E8) and the CKR geometry the framing
 *    depends on: mclk = rate * {128,256,512} exactly and
 *    MDIV = mclk/(rate<<6) in {2,4,8} (=> 64-BCLK frame at SCLK = 64*Fs).
 *    These are the numbers behind the 2x-speed/2x-pitch bug class (DFIU
 *    75271e5f) — any drift here is that regression.
 */
#include <stdint.h>

#include "test_util.h"
#include "audio/audio_hal.h"
#include "audio/ao_rate_table.h"

int main(void)
{
    int i;

    /* Slot packing: the 16-bit (polled tone) path composes
     * ao_i16_to_s32 -> ao_slot_word, which must equal the raw inline
     * expression kept in rknano_i2s.c; the streaming path passes the
     * internal int32 sample straight through (it is already the slot). */
    CHECK(ao_slot_word(ao_i16_to_s32(0)) == ((uint32_t)(uint16_t)0 << 16),
          "slot: zero");
    CHECK(ao_slot_word(ao_i16_to_s32(0x1234)) == 0x12340000u,
          "slot: positive sample");
    CHECK(ao_slot_word(ao_i16_to_s32(-1)) == 0xFFFF0000u,
          "slot: -1 sign pattern");
    CHECK(ao_slot_word(ao_i16_to_s32(-32768)) == 0x80000000u,
          "slot: INT16_MIN");
    CHECK(ao_slot_word((int32_t)0x12345678) == 0x12345678u,
          "slot: int32 sample is the slot word (identity)");
    CHECK(ao_slot_word((int32_t)-1) == 0xFFFFFFFFu,
          "slot: int32 -1 sign pattern");

    /* Stream-depth gate (audio_hal.h): 16/24/32 accepted, others rejected.
     * A 32-bit ASP word reads every left-justified depth at full scale. */
    CHECK(ao_format_bits_ok(16) && ao_format_bits_ok(24) &&
          ao_format_bits_ok(32), "bits gate: 16/24/32 accepted");
    CHECK(!ao_format_bits_ok(8) && !ao_format_bits_ok(20) &&
          !ao_format_bits_ok(0) && !ao_format_bits_ok(64),
          "bits gate: other widths rejected");

    /* Volume mapping: 2 codes per dB (0.5 dB/step), 0x00 = 0 dB,
     * 0xFF = -127.5 dB (datasheet volume decode). Pure integer. */
    CHECK_EQI(ao_volume_atten_code(0), 0, "0 dB -> 0x00");
    CHECK_EQI(ao_volume_atten_code(-1), 2, "-1 dB -> 2 codes");
    CHECK_EQI(ao_volume_atten_code(-20), 40, "-20 dB -> 0x28");
    CHECK_EQI(ao_volume_atten_code(-60), 120, "-60 dB -> 0x78");
    CHECK_EQI(ao_volume_atten_code(20), 0, "+20 dB clamps at 0 dB");
    CHECK_EQI(ao_volume_atten_code(-128), 255, "-128 dB clamps at mute");
    CHECK_EQI(ao_volume_atten_code(-10000), 255, "floor: 0xFF mute");

    /* Exact ref constants (the three that were once transcribed wrong). */
    for (i = 0; i < 13; i++) {
        if (dac_rates[i].rate == 44100)
            CHECK(dac_rates[i].mclk == 11289600u, "mclk 44100 = 11289600");
        if (dac_rates[i].rate == 88200)
            CHECK(dac_rates[i].mclk == 11289600u, "mclk 88200 = 11289600");
        if (dac_rates[i].rate == 176400)
            CHECK(dac_rates[i].mclk == 22579200u, "mclk 176400 = 22579200");
    }

    /* CKR geometry for every entry: exact ratio multiple + stock MDIV. */
    for (i = 0; i < 13; i++) {
        uint32_t rate = dac_rates[i].rate;
        uint32_t mclk = dac_rates[i].mclk;
        uint32_t ratio = mclk / rate;
        uint32_t tmd;

        CHECK(rate > 0, "entry has a rate");
        CHECK(mclk == rate * ratio && (ratio == 128u || ratio == 256u ||
                                       ratio == 512u),
              "mclk is an exact 128x/256x/512x multiple of rate");
        tmd = mclk / (rate << 6);          /* stock CKR: mclk/(rate<<6)   */
        CHECK(tmd == 2u || tmd == 4u || tmd == 8u,
              "stock MDIV lands on 2/4/8 (64-BCLK frame)");
        CHECK(tmd * 64u == ratio, "MDIV*64 == mclk/rate (Fs exact)");
    }

    /* Lookup: every table rate resolves; unknown rates do not. */
    for (i = 0; i < 13; i++)
        CHECK(ao_rate_lookup(dac_rates[i].rate) == &dac_rates[i],
              "lookup resolves a table rate");
    CHECK(ao_rate_lookup(12345) == 0, "lookup rejects unknown rate");

    printf("test_ao_slots: OK\n");
    return 0;
}
