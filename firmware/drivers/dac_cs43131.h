/*
 * dac_cs43131.h — CS43131 dual-DAC bring-up API (Echo Mini, clean-room).
 *
 * Facts source: docs/rewrite/dac.md §6 (reference fw1 binary + datasheet).
 * Experimental bring-up (Exp 1/2): every step leaves a breadcrumb so a
 * hardware hang is identifiable from the menu.
 */
#ifndef RECHORD_DAC_CS43131_H
#define RECHORD_DAC_CS43131_H

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

/* Breadcrumb: last reached step of dac_init / the feed path. */
enum {
    DAC_STEP_NONE = 0,
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
    DAC_STEP_FEED,         /* first dac_i2s_feed entered (test path)     */
};

/* Full bring-up for `rate_hz` (one of the 13 table rates).
 * Returns DAC_OK or the failure code; dac_last_step() gives the breadcrumb. */
int dac_init(uint32_t rate_hz);

/* Queue `frames` interleaved int16 STEREO frames into I2S1 TX (polled
 * FIFO). DAC_ERR_TIMEOUT if the FIFO stops draining (clock not running). */
int dac_i2s_feed(const int16_t *pcm, int frames);

void dac_i2s_start(void);
void dac_i2s_stop(void);

/* Last-reached bring-up step (DAC_STEP_*). */
int dac_last_step(void);

/* I2C0 mux-candidate scan result: which candidate (0=A GPIO2B2/B3,
 * 1=B GPIO2C1/C0, 2=C GPIO0A6/A5) ACKed, per chip (0=L 0x60, 1=R 0x66).
 * Returns 1=ACK / 0=NAK, -1 if candidate out of range. */
int dac_scan_result(int cand, int chip);

/* Raw HP-detect status bytes read from MAP 0x0F0000 (L, R). */
void dac_get_hp_status(uint8_t out[2]);

/* Busy-wait delay (~ms at the 24 MHz boot clock). */
void dac_delay_ms(unsigned ms);

#endif /* RECHORD_DAC_CS43131_H */
