/*
 * audio_output.c — centralized audio output controller (audio HAL facade).
 *
 * The bring-up sequence in ao_configure_rate() is the VERBATIM dac_init()
 * body from dac_cs43131.c (commit c5d571f), including its step ordering
 * (plan hard constraint 2): power/reset GPIOs -> CRU gates -> I2C0 reset ->
 * mux scan -> rate lookup -> ACODEC PLL mux -> I2S1 -> CS43131 register
 * blocks -> HP check -> mono table -> unmute. Register writes live in the
 * extracted drivers (cs43131.c / rknano_i2s.c) with names kept 1:1.
 *
 * Feeder: playback streams from a caller-owned SPSC ring through the
 * ao_feeder_ring_irq backend (TX-empty IRQ; rknano_i2s.c). A DMA
 * ping-pong backend can replace the binding (audio_hal.h) without
 * touching player/codecs/ipc_b.
 */
#include <stdint.h>

#include "audio_output.h"
#include "audio_hal.h"
#include "ao_rate_table.h"
#include "ao_regs.h"
#include "cs43131.h"
#include "rknano_i2s.h"

/* ---- feeder binding (seam: swap for DMA later) --------------------------- */

const ao_feeder_ops ao_feeder_ring_irq = {
    rknano_i2s_attach,
    rknano_i2s_detach,
    rknano_i2s_kick,
    rknano_i2s_hold,
};

static const ao_feeder_ops *s_feeder = &ao_feeder_ring_irq;

/* Frames drained to the I2S FIFO (progress accounting; incremented once
 * per stereo frame pulled from the ring). */
static volatile long s_frames_played;

/* Current user-volume attenuation code (0x090001/2; 0.5 dB/step, 0x00 =
 * 0 dB). Kept across configure() so a track relock lands on the same
 * level (the stock ramps 0xFF -> its user level for the same reason). */
static uint8_t s_volume_atten;

/* Ring pull for the IRQ feeder: one packed int16 pair per stereo frame. */
static int ao_ring_pull(uint32_t *word, void *user)
{
    ringbuf_t *rb = (ringbuf_t *)user;

    if (rb_read(rb, word, sizeof *word) != sizeof *word)
        return 0;
    s_frames_played++;
    return 1;
}

/* ---- controller API ------------------------------------------------------ */

int audio_output_init(void)
{
    /* State-only today: hardware comes up in configure() with the verified
     * full bring-up sequence (see ao_configure_rate). */
    s_frames_played = 0;
    return 0;
}

/* ---- bring-up (VERBATIM dac_init body, commit c5d571f) ------------------- */

static int ao_configure_rate(uint32_t rate_hz)
{
    const dac_rate_t *rt = 0;
    int i, cand;

    /* Ref order (FUN_03056550): power/reset GPIOs FIRST, then gates,
     * then the I2C0 soft reset, and only then touch I2C0. */
    s_step = DAC_STEP_POWER;
    /* Clock the GPIO/PCLK domain BEFORE the power writes (ref gates 0x95 =
     * PCLK_GPIO1, dac.md §6): if the GPIO bus clock is off the power/reset
     * writes are dropped and the CS43131s stay in power-down, ACKing nothing. */
    cru_gate(G_PCLK_GPIO1, 1);
    dac_power_sequence();

    s_step = DAC_STEP_GATES;
    cru_gate(G_CLK_ACODEC, 1);
    cru_gate(G_CLK_I2S1_SRC, 1);
    cru_gate(G_CLK_I2S1_FRAC, 1);
    cru_gate(G_CLK_I2S1_OUT, 1);
    cru_gate(G_CLK_I2S1, 1);
    cru_gate(G_PCLK_I2S1, 1);
    cru_gate(G_PCLK_I2C0, 1);
    cru_gate(G_PCLK_ACODEC, 1);

    cru_rst(R_I2C0_SRST, 1);               /* ref: rst assert, ~100 us,  */
    dac_delay_ms(1);                        /* release (FUN_03051430)     */
    cru_rst(R_I2C0_SRST, 0);

    s_step = DAC_STEP_I2C_SCAN;
    I2C_CON = 0;                            /* ref: idle before CLKDIV    */
    cand = i2c0_scan();
    if (cand < 0)
        return DAC_ERR_NOCHIP;

    s_step = DAC_STEP_RATE;
    for (i = 0; i < 13; i++) {
        if (dac_rates[i].rate == rate_hz) {
            rt = &dac_rates[i];
            break;
        }
    }
    if (!rt)
        return DAC_ERR_ARG;

    /* ACODEC mux for this rate (verbatim from ref). */
    acodec_mux(rt->mux[0], rt->mux[1], rt->mux[2], rt->mux[3]);

    s_step = DAC_STEP_I2S;
    i2s1_setup(rt->rate, rt->mclk);

    s_step = DAC_STEP_DAC_REGS;
    dac_map_write(0x000B0000, 0x1F);
    dac_delay_ms(80);
    dac_gpio_out(1, 4, 1);            /* amp/HP enable (ref FUN_03051478)  */
    dac_map_write(0x00020052, 0x02);

    s_step = DAC_STEP_HP_CHECK;
    {
        uint8_t st0 = 0, st1 = 0;

        dac_map_read(0, 0x0F0000, &st0);
        dac_map_read(1, 0x0F0000, &st1);
        s_hp_status[0] = st0;
        s_hp_status[1] = st1;
        /* Ref aborts only if BOTH chips fail the check. */
        if ((st0 & 0x03u) == 0x03u && (st1 & 0x03u) == 0x03u)
            return DAC_ERR_HP;
    }

    s_step = DAC_STEP_DAC_REGS2;
    dac_map_write(0x000F0010, 0xE7);
    dac_map_write(0x00020000, 0xF6);
    dac_map_write(0x0001000C, 0x04);
    dac_map_write(0x00040010, 0x01);
    dac_map_write(0x00040011, 0x00);
    dac_map_write(0x00040012, 0x08);
    dac_map_write(0x00040013, 0x00);
    dac_map_write(0x00040014, 0x1F);
    dac_map_write(0x00040015, 0x00);
    dac_map_write(0x00040016, 0x3F);
    dac_map_write(0x00040017, 0x00);
    dac_map_write(0x00040018, 0x0C);
    dac_map_write(0x00040019, 0x0A);
    dac_map_write(0x00050000, 0x00);
    dac_map_write(0x00050001, 0x00);
    /* Per-chip output channel/gain (ref FUN_03055ecc): L=7, R=15. */
    dac_map_write_chan(0, 0x0005000A, 0x07);
    dac_map_write_chan(0, 0x0005000B, 0x07);
    dac_map_write_chan(1, 0x0005000A, 0x0F);
    dac_map_write_chan(1, 0x0005000B, 0x0F);
    dac_map_write(0x00090000, 0x02);
    dac_volume_set(0xFF);            /* ref FUN_03055f28(255): volume mute   */
    dac_map_write(0x00090003, 0xEC);
    dac_map_write(0x00090004, 0x00);

    s_step = DAC_STEP_MONO;
    for (i = 0; i < 24; i++)
        dac_map_write(dac_mono_seq[i].map, dac_mono_seq[i].val);

    s_step = DAC_STEP_DAC_REGS3;
    dac_map_write(0x000B0000, 0x1F);
    dac_delay_ms(80);
    dac_map_write(0x00080000, 0x10);
    dac_map_write(0x000D0000, 0x04);
    dac_map_write(0x000D0001, 0x00);
    dac_map_write(0x000F0010, 0x87);
    dac_map_write(0x000F0011, 0x07);
    dac_map_write(0x00010006, 0x04);
    dac_map_write(0x0001000D, 0x02);
    dac_map_write(0x00010010, 0x99);
    dac_map_write(0x00180000, 0x45);
    dac_map_write(0x00010010, 0x00);
    dac_pdn_up(0x40);                    /* PDN_ASP up — ref FUN_03056c68(0x40) */
    dac_map_write(0x00010010, 0x99);
    dac_map_write(0x00080032, 0x20);
    dac_pdn_up(0x10);                    /* PDN_HP up — ref FUN_03056c68(0x10)  */
    dac_delay_ms(10);
    dac_map_write(0x00080032, 0x00);
    dac_map_write(0x00010010, 0x00);

    s_step = DAC_STEP_VOLUME;
    /* Per-rate DAC control byte (ref: MAP 0x01000B). */
    dac_map_write(0x0001000B, rt->dac);
    dac_delay_ms(10);

    s_step = DAC_STEP_UNMUTE;
    /* Stock FUN_030563b0 output-enable (jack-plugged branch), REA/Ghidra
     * decompiled from ref fw1: GPIO2.14 = 1 then GPIO2.0 = 0 — GPIO2.0 HIGH
     * is a HARDWARE MUTE (the power-on trio drives it high on purpose), so
     * leaving it asserted silences a perfectly configured DAC. Then the
     * volume regs PCM_VOLUME_B/A (0x090001/0x090002, FUN_03055f28) fade
     * 0xFF -> the user level via FUN_03055f48. The stock's level variable
     * is 0 at boot (= 0 dB), and dac_volume_ramp is that fade — do NOT
     * replace it with a bare write to 0x010009: that MAP is RESERVED and
     * the real volume regs then sit at their -60 dB reset default
     * (the "extremely quiet" bug). */
    dac_gpio_out(2, 14, 1);
    dac_gpio_out(2, 0, 0);
    dac_gpio_out(1, 4, 1);            /* amp enable again (ref)           */
    dac_volume_ramp(0xFF, s_volume_atten); /* FUN_03055f48(0xff, user level) */
    /* Ref FUN_03055ecc again right before the feed. */
    dac_map_write_chan(0, 0x0005000A, 0x07);
    dac_map_write_chan(0, 0x0005000B, 0x07);
    dac_map_write_chan(1, 0x0005000A, 0x0F);
    dac_map_write_chan(1, 0x0005000B, 0x0F);
    dac_delay_ms(10);

    s_step = DAC_STEP_DONE;
    return DAC_OK;
}

int audio_output_configure(const audio_format_t *fmt)
{
    if (fmt == 0 || fmt->sample_rate == 0)
        return DAC_ERR_ARG;
    if (fmt->bits != 16)                 /* 24/32-bit plumbed, not built   */
        return DAC_ERR_ARG;
    if (fmt->channels < 1 || fmt->channels > 2)
        return DAC_ERR_ARG;
    return ao_configure_rate(fmt->sample_rate);
}

int audio_output_start(ringbuf_t *pcm_ring)
{
    rknano_i2s_start();
    if (pcm_ring != 0)
        s_feeder->attach(ao_ring_pull, pcm_ring);
    return 0;
}

void audio_output_stop(void)
{
    rknano_i2s_stop();
}

void audio_output_detach(void)
{
    s_feeder->detach();
}

void audio_output_hold(int held)
{
    s_feeder->hold(held);
}

void audio_output_kick(void)
{
    s_feeder->kick();
}

long audio_output_frames_played(void)
{
    return s_frames_played;
}

void audio_output_clear_played(void)
{
    s_frames_played = 0;
}

void audio_output_power_off(void)
{
    dac_output_off();
}

void audio_output_set_volume_db(int db)
{
    /* dB -> PCM_VOLUME codes via the pure integer mapping
     * (audio_hal.h ao_volume_atten_code: 2 codes per dB, 0x00 = 0 dB,
     * 0xFF = -127.5 dB — datasheet volume decode; positive dB clamps at
     * 0). Ramp, don't jump: PCM_SZC soft ramp is enabled (0x09 0003 =
     * 0xEC) and the stock volume path (FUN_03055f48) is a soft ramp to
     * the user level — this is that setter. */
    dac_volume_ramp(s_volume_atten, ao_volume_atten_code(db));
    s_volume_atten = ao_volume_atten_code(db);
}

int audio_output_feed_blocking(const int16_t *pcm, int frames)
{
    return rknano_i2s_feed(pcm, frames);
}
