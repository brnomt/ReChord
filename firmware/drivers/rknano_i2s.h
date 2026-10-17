/*
 * rknano_i2s.h — RKnanoD SoC-side audio: board primitives, CRU clock tree,
 * ACODEC PLL mux, I2S1 serializer and the TX feeder (polled + IRQ).
 *
 * Extracted VERBATIM from dac_cs43131.c (commit c5d571f) per the modular
 * pipeline plan. Register code, masks, shifts and init ordering are 1:1
 * unchanged (plan hard constraint 2, DFIU 75271e5f); only file boundaries
 * moved. The shared helper names (cru_gate, grf_iomux, dac_gpio_out, ...)
 * are kept exactly so the moved call sites stay line-identical — they are
 * SoC/board primitives, not chip-driver logic.
 *
 * Public surface here uses the rknano_i2s_* prefix (the ISR symbol
 * rknano_i2s_tx_isr replaces dac_i2s_tx_isr in the vector table).
 */
#ifndef RECHORD_RKNANO_I2S_H
#define RECHORD_RKNANO_I2S_H

#include <stdint.h>

/* ---- SoC/board primitives (moved verbatim; names kept) ------------------ */

void delay_loops(volatile unsigned n);
void dac_delay_ms(unsigned ms);
void cru_gate(int gate_id, int enable);
void cru_rst(int rst_id, int assert);
void grf_iomux(int port, int pin, int func);
void dac_gpio_out(int port, int pin, int level);
void dac_gpio_input(int port, int pin);

/* ---- clock tree + I2S1 setup (moved verbatim; names kept) --------------- */

void acodec_mux(uint8_t m4, uint8_t m5, uint8_t m6, uint8_t m7);
void i2s1_setup(uint32_t rate, uint32_t mclk);

/* ---- TX feeder ---------------------------------------------------------- */

/* Polled feed (DAC test tone path): interleaved int16 STEREO frames into
 * the I2S1 TX FIFO. Returns DAC_OK / DAC_ERR_* (cs43131.h). */
int rknano_i2s_feed(const int16_t *pcm, int frames);

void rknano_i2s_start(void);
void rknano_i2s_stop(void);

/* ISR-driven TX feed: rknano_i2s_tx_isr() (I2S1 TX IRQ, vector slot 48 =
 * INT_ID_I2S1) drains a caller-owned PCM ring into the TX FIFO so decode
 * overlaps playback. `pull` returns 1 and fills *word with one stereo
 * frame as a packed int16 pair (L=[15:0], R=[31:16]); the driver expands
 * it to the two wire words (L<<16 then R<<16 — one 32-bit channel sample
 * per FIFO word). Returns 0 when no data is available (the IRQ source is
 * masked until rknano_i2s_kick()). */
typedef int (*rknano_i2s_pull_fn)(uint32_t *word, void *user);
void rknano_i2s_attach(rknano_i2s_pull_fn pull, void *user);
void rknano_i2s_detach(void);
void rknano_i2s_kick(void);        /* re-arm after producer push / resume */
void rknano_i2s_hold(int held);    /* 1 = pause feeding, 0 = resume        */
void rknano_i2s_tx_isr(void);      /* the IRQ handler (INT_ID_I2S1)        */

/* TX FIFO level range observed since rknano_i2s_start() (diag: 00-00 = the
 * TXDR writes vanish, 00-1x = the FIFO drains = SCLK is running). */
void rknano_i2s_fifo_range(unsigned *lo, unsigned *hi);

#endif /* RECHORD_RKNANO_I2S_H */
