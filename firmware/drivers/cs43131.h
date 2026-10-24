/*
 * cs43131.h — CS43131 dual-DAC chip driver (I2C + chip sequences).
 *
 * Extracted VERBATIM from dac_cs43131.c (commit c5d571f) per the modular
 * pipeline plan. The I2C transaction flow is the verified datasheet
 * Fig 4-32/4-34 sequence (MSB-first MAP, aborted-write preamble + STOP,
 * TRX receive) — plan hard constraint 2: move 1:1, never "clean up".
 *
 * NAMING: the moved helpers keep their c5d571f names (dac_map_write,
 * dac_pdn_up, ...) so every call site in the bring-up sequence stays
 * line-identical; only the I2S feeder API moved to the rknano_i2s_* prefix.
 *
 * Facts source: docs/rewrite/dac.md §6 (reference fw1 binary + datasheet).
 * Two CS43131s, MONO (differential) mode, one chip per channel:
 * L = 0x60 (7-bit 0x30), R = 0x66 (7-bit 0x33) — ref RAM 0x03060B78;
 * mono enable = the 24-pair (MAP,value) table at ref RAM 0x03060AB8.
 * Control: I2C0 @ 0x40040000; register access =
 * [chipaddr+W][MAP_hi][MAP_mid][MAP_lo][ctl=1][data].
 */
#ifndef RECHORD_CS43131_H
#define RECHORD_CS43131_H

#include <stdint.h>

/* Return codes (negative = failure). */
enum {
    DAC_OK         = 0,
    DAC_ERR_ARG    = -1,   /* bad rate / argument                        */
    DAC_ERR_NAK    = -2,   /* I2C NAK (chip absent on that candidate)    */
    DAC_ERR_TIMEOUT = -3,  /* I2C/I2S FIFO never completed               */
    DAC_ERR_NOCHIP = -4,   /* no I2C mux candidate ACKs either DAC chip  */
    DAC_ERR_HP     = -5,   /* HP check failed on both chips              */
};

/* Breadcrumb: last reached step of the bring-up / the feed path. */
enum {
    DAC_STEP_NONE = 0,
    DAC_STEP_POWER,        /* GPIO2 power/reset sequence (ref entry)     */
    DAC_STEP_GATES,        /* CRU gates enabled                          */
    DAC_STEP_I2C_SCAN,     /* I2C0 mux candidate scan                    */
    DAC_STEP_RATE,         /* rate table lookup + ACODEC mux             */
    DAC_STEP_I2S,          /* I2S1 master configured                     */
    DAC_STEP_DAC_REGS,     /* first DAC register block                   */
    DAC_STEP_HP_CHECK,     /* MAP 0x0F0000 read on both chips            */
    DAC_STEP_DAC_REGS2,    /* second DAC register block                  */
    DAC_STEP_MONO,         /* 24-pair mono-mode table                    */
    DAC_STEP_DAC_REGS3,    /* mute/unmute dance block                    */
    DAC_STEP_VOLUME,       /* per-rate DAC control byte                  */
    DAC_STEP_UNMUTE,       /* final unmute                               */
    DAC_STEP_DONE,         /* bring-up complete                          */
    DAC_STEP_FEED,         /* first feed entered (test path)             */
};

/* Bring-up diag state, shared across the audio modules. Kept as single
 * globals (names unchanged) so the moved sequences stay line-identical to
 * c5d571f. Written by the bring-up sequence / feed; read via the getters. */
extern int s_step;
extern uint8_t s_hp_status[2];

/* Mono-mode table (datasheet §5.12 / ref RAM 0x03060AB8), used by the
 * bring-up sequence (audio_output.c). */
typedef struct { uint32_t map; uint8_t val; } dac_mono_pair_t;
extern const dac_mono_pair_t dac_mono_seq[24];

/* ---- MAP access primitives (I2C) ---------------------------------------- */

/* Per-chip MAP write (ref FUN_03055ecc writes different L/R values). */
int dac_map_write_chan(int chan, uint32_t map24, uint8_t val);
/* Broadcast a MAP write to BOTH chips (ref semantics: FUN_03055f08). */
int dac_map_write(uint32_t map24, uint8_t val);
int dac_map_read(int chan, uint32_t map24, uint8_t *out);

/* Power-up RMW — verbatim ref FUN_03056c68(mask): clear PDN bits in
 * 0x020000 on BOTH chips (0x40 = PDN_ASP, 0x10 = PDN_HP). */
void dac_pdn_up(uint8_t mask);

/* ---- volume (PCM_VOLUME_B/A = MAP 0x090001/0x090002, datasheet §7) -----
 * 0.5 dB/step ATTENUATION: 0x00 = 0 dB ... 0xFF = -127.5 dB. The chip
 * RESETS these to 0x78 = -60.0 dB — they MUST be programmed or playback
 * is extremely quiet (DFIU-class fix 2026-10-16: an earlier port
 * misattributed the stock's volume writer FUN_03055f28 to MAP 0x010009,
 * which is RESERVED (datasheet 0x01 0007-000A), and never touched
 * PCM_VOLUME_A/B at all).
 * With 0x090003 = 0xEC (PCM_VOL_BEQA=1) channel A's value drives both
 * channels; the stock writes both registers anyway (FUN_03055f28). */
void dac_volume_set(uint8_t atten);              /* = ref FUN_03055f28    */
/* Soft ramp between two attenuation codes: ref FUN_03055f48 semantics
 * (step 6 codes = 3 dB, 2 ms apart, click-free fade). */
void dac_volume_ramp(uint8_t from, uint8_t to);  /* = ref FUN_03055f48    */

/* Same 6-code step, but spread over `ms` milliseconds (BRNOMT-67). Used by
 * the play/pause fade so its duration is the user setting, not the fixed
 * 2 ms/step of the stock volume ramp. ms == 0 jumps to the target. */
void dac_volume_ramp_ms(uint8_t from, uint8_t to, unsigned ms);

/* GPIO power/reset sequence (ref FUN_03056550 entry). */
void dac_power_sequence(void);

/* Boot pclk for I2C0 (24 MHz XIN) and the I2C0 mux-candidate scan.
 * i2c0_scan() fills the scan matrix and returns the first ACKing mux
 * candidate (0..2) or -1. */
#define DAC_PCLK_HZ 50000000u   /* pclk after ap_clock_set_200mhz() (APLL/4) */
int i2c0_scan(void);

/* Output teardown + close — ref FUN_03055F80 + FUN_030564d8 I2S half
 * (hardware mute, DAC mute, amp off, I2S1 reset/pad Hi-Z). */
void dac_output_off(void);

/* ---- diagnostics (DAC test console) ------------------------------------- */

int dac_last_step(void);
void dac_get_hp_status(uint8_t out[2]);
int dac_scan_result(int cand, int chip);
int dac_read_reg(int chan, uint32_t map24, uint8_t *out);
int dac_write_reg(int chan, uint32_t map24, uint8_t val);

#endif /* RECHORD_CS43131_H */
