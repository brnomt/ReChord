/*
 * ao_rate_table.{c,h} — pure per-rate audio configuration table.
 *
 * VERBATIM data relocation from dac_cs43131.c (commit c5d571f): the 13
 * entries are byte-exact from ref RAM 0x030609E8 (REA read-back
 * 2026-10-16). Pure data + lookup — no MMIO — so it compiles for host
 * tests (which pin the constants that caused the 2x-speed bug class).
 *
 * Each entry couples three per-rate facts used by the coordinator
 * (audio_output.c):
 *   rate : supported sample rate (Hz)
 *   mux  : ACODEC PLL config bytes (POSDIV_L3/POSDIV_H8/PREDIV/OUTDIV),
 *          written to +0x204/+0x208/+0x20c/+0x210 hi nibble
 *   mclk : Hz at the DAC MCLK pin = ACODEC_PLL/5, always an EXACT
 *          256x/128x/512x multiple of rate
 *   dac  : per-rate CS43131 DAC control byte (MAP 0x01000B)
 */
#ifndef RECHORD_AO_RATE_TABLE_H
#define RECHORD_AO_RATE_TABLE_H

#include <stdint.h>

typedef struct
{
    uint32_t rate;
    uint32_t mclk;        /* Hz at the DAC MCLK pin = ACODEC_PLL/5, always
                           * an EXACT 256x/128x/512x multiple of rate (512x
                           * for 8000/11025/12000, 256x for the mid rates,
                           * 128x for 88200/176400/192000). Byte-verified
                           * from ref RAM 0x030609E8 via REA 2026-10-16 —
                           * the earlier "just under 256x" values (44100 ->
                           * 11288960, 176400 -> 22577664) were transcription
                           * errors (true bytes: 11289600, 22579200). */
    uint8_t  mux[4];      /* ACODEC PLL cfg: POSDIV_L3/POSDIV_H8/PREDIV/OUTDIV
                           * (ref table [4..7] == ACodecpllTable_* rows)   */
    uint8_t  dac;         /* per-rate DAC control byte (ref table [12])  */
} dac_rate_t;

/* Ref RAM 0x030609E8, 13 entries (byte-exact; REA read-back 2026-10-16). */
extern const dac_rate_t dac_rates[13];

/* Pure lookup: exact-rate match or NULL. */
const dac_rate_t *ao_rate_lookup(uint32_t rate_hz);

#endif /* RECHORD_AO_RATE_TABLE_H */
