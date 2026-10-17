/*
 * rknano_i2s.c — RKnanoD SoC-side audio (clock tree + I2S1 + TX feeder).
 *
 * VERBATIM relocation from dac_cs43131.c (commit c5d571f) — register code,
 * masks, shifts and ordering are 1:1 unchanged (plan hard constraint 2).
 * Only file boundaries moved and the public dac_i2s_* feeder API renamed to
 * rknano_i2s_* (the vector symbol is now rknano_i2s_tx_isr).
 *
 * Provenance of the hardware facts (do not "fix" — see DFIU 75271e5f and
 * docs/rewrite/dac.md §6/§9): CRU @ 0x40180000 (active-low gates, upper-
 * halfword write-enable), ACODEC block @ 0x40090000 (PLL mux +0x200..+0x214),
 * I2S1 @ 0x40030000 (TXDR +0x24). I2S registers are stock-verbatim:
 * TXCR/RXCR=0x401F, CKR=((mclk/(rate<<6))-1)<<16 | 0x3F3F, INTCR=0x01F00102.
 * Wire format: ONE 32-bit FIFO word = ONE channel sample, left-justified;
 * a stereo frame is TWO words, L slot then R slot. The streaming path
 * carries the internal int32 sample as-is (already left-justified); the
 * polled tone path lifts its int16 source with `(uint16_t)sample << 16`.
 */
#include <stdint.h>

#include "cs43131.h"        /* DAC_OK / DAC_ERR_* / DAC_STEP_FEED / s_step  */
#include "ao_regs.h"
#include "rknano_i2s.h"

/* ---- delay --------------------------------------------------------------- */

void delay_loops(volatile unsigned n)
{
    while (n--)
        ;
}

void dac_delay_ms(unsigned ms)
{
    /* ~200 MHz AP clock after ap_clock_set_200mhz(): ~200k loops is ~1 ms
     * (over-shoot is safe). */
    while (ms--)
        delay_loops(200000);
}

/* ---- CRU gates (active-low, upper-halfword write-enable, SDK style) ------ */

void cru_gate(int gate_id, int enable)
{
    unsigned group = (unsigned)gate_id >> 4;
    unsigned bit = 1u << ((unsigned)gate_id & 0xFu);
    uint32_t v = CRU_GATE(group);

    if (enable)
        v &= ~bit;
    CRU_GATE(group) = (v & 0x0000FFFFu) | 0xFFFF0000u;
}

void cru_rst(int rst_id, int assert)
{
    unsigned bit = 1u << ((unsigned)rst_id & 0xFu);
    /* SDK ScuSoftResetCtr: bit+16 = write-enable, bit = reset value. */
    CRU_RST(rst_id >> 4) = (bit << 16) | (assert ? bit : 0u);
    delay_loops(40000);                     /* SDK: DelayUs(5) at 200 MHz */
}

/* ---- GPIO primitives ----------------------------------------------------- */

typedef struct {
    volatile uint32_t dr;                   /* +0x00 SWPORTA_DR           */
    volatile uint32_t ddr;                  /* +0x04 SWPORTA_DDR          */
} dac_gpio_t;

#define DAC_GPIO1 ((dac_gpio_t *)0x40170000u)
#define DAC_GPIO2 ((dac_gpio_t *)0x50030000u)

/* Release a pad to GPIO input (ref FUN_030514cc semantics): mux func 0 +
 * DDR in. The close path Hi-Z's the I2S1 pads this way so the DAC's ASP
 * inputs are not driven by a stopped serializer between sessions. */
void dac_gpio_input(int port, int pin)
{
    dac_gpio_t *g = (port == 1) ? DAC_GPIO1 : DAC_GPIO2;
    uint32_t bit = 1u << (unsigned)(pin & 31);

    grf_iomux(port, pin, 0);
    g->ddr &= ~bit;
}

/* Ref FUN_03051478(port, pin, level): mux the pin to GPIO (func 0),
 * drive it, and set the direction to output. */
void dac_gpio_out(int port, int pin, int level)
{
    dac_gpio_t *g = (port == 1) ? DAC_GPIO1 : DAC_GPIO2;
    uint32_t bit = 1u << (unsigned)(pin & 31);

    grf_iomux(port, pin, 0);
    if (level)
        g->dr |= bit;
    else
        g->dr &= ~bit;
    g->ddr |= bit;
}

/* ---- GRF pin mux --------------------------------------------------------- */

/* RK IOMUX (ref FUN_03051458, hardware-verified in the panel driver too):
 * register = GRF + port*0x20 + (pin>>3)*4 (GPIO_IOMUX{0,1,2}{A..D}),
 * 2 bits/pin at ((pin&7)*2), bits 16/17 = write-enable for that field.
 * The ref writes (func | 0x30000) << sh as a plain word; the upper half
 * shifts with the field so only this pin's write-enable is active. */
void grf_iomux(int port, int pin, int func)
{
    volatile uint32_t *r =
        (volatile uint32_t *)(uintptr_t)(GRF_BASE +
                                         (unsigned)port * 0x20u +
                                         (unsigned)(pin >> 3) * 4u);
    unsigned sh = (unsigned)(pin & 7) * 2u;

    *r = ((uint32_t)(func & 3) | 0x30000u) << sh;
}

/* ---- I2S1 master setup --------------------------------------------------- */

void i2s1_setup(uint32_t rate, uint32_t mclk)
{
    /* Ref (FUN_03056550 tail): enable PCLK_GPIO1, mux GPIO1A0..A3 to
     * I2S1A (CLK/LRCK/SCLK/SDO, func 1), select I2S1A in GRF_IOMUX_CON
     * (bit 13, WE bit 29), then the CRU CLKSEL_CON[2] clock routing. */
    cru_gate(G_PCLK_GPIO1, 1);
    /* Reset the I2S1 block BEFORE programming it: stock asserts the soft
     * reset in its close (FUN_030564d8) and only releases it here, so a
     * re-run without a close inherits stale serializer/FIFO state -> the
     * tone plays with noise on top (DFIU 60609d7f). Asserting first makes
     * every entry start from a cold block. */
    cru_rst(R_I2S1_SRST, 1);
    dac_delay_ms(1);
    cru_rst(R_I2S1_SRST, 0);               /* ref: release I2S1 reset    */
    grf_iomux(1, 0, 1);
    grf_iomux(1, 1, 1);
    grf_iomux(1, 2, 1);
    grf_iomux(1, 3, 1);
    GRF_IOMUX = 0x20000000u;               /* i2s1_sel = I2S1A (WE b29)  */

    /* CRU_CLKSEL_CON[2] verbatim literals from the ref: 0x80008000,
     * 0x007F0004, 0x03000000, 0x10000000 (ACODEC/I2S1 clock routing +
     * dividers, WE-masked fields). */
    REG32(CRU_BASE + 0x20u) = 0x80008000u;
    REG32(CRU_BASE + 0x20u) = 0x007F0004u;
    REG32(CRU_BASE + 0x20u) = 0x03000000u;
    REG32(CRU_BASE + 0x20u) = 0x10000000u;

    cru_rst(R_I2S1_SRST, 0);               /* ref: release I2S1 reset    */

    I2S_XFER = 0;
    I2S_CLR = 1;
    delay_loops(400000);
    while (I2S_CLR)
        delay_loops(833);

    /* TXCR/RXCR = 0x401F stock-verbatim (ref FUN_03056550 "i2s regs",
     * REA-confirmed 2026-10-16): CSR = Channel_0 pair (stereo slots),
     * HWT = 1 (32-bit FIFO access), VDW = 0x1F (32-bit valid data).
     * Wire format is set by the stock's DMA payload (Service.c
     * Bit_Convertor_DEC/shift): ONE FIFO word = ONE channel sample,
     * left-justified (sample << 16); a stereo frame is TWO words —
     * L in the first 32-BCLK slot, R in the second. Do not pack L|R
     * into one word: each slot is its own TXDR write (see the feed and
     * ISR below). */
    I2S_TXCR = 0x401Fu;
    I2S_RXCR = 0x401Fu;
    /* CKR stock-verbatim (ref: ((mclk/(rate<<6))-1)<<16 | 0x3F3F): the whole
     * mclk/rate ratio goes into MDIV and TSD/RSD stay 64 (64 BCLK per LRCK =
     * 2x32-bit slots, exactly the CS43131 ASP 5050 32-bit frame).  This
     * makes SCLK = 64*Fs, which is the frame rate the CS43131 paces on.
     * The previous home-grown split (TSD=256) clocked 256 BCLK per LRCK:
     * the DAC's 64-BCLK frame counter then ran at SCLK/64 = 4x the file
     * rate — heard as "stuttering at a high speed and a high pitch" (the
     * FIFO also drained 4x faster than the player fed it).
     * mclk is an exact ratio multiple (table comment above), so the integer
     * division lands on MDIV 8/4/2 for every entry, matching the SDK's
     * CKR_MDIV_Value_8/_4/_2 encodings. */
    {
        uint32_t tmd = (rate > 0u) ? (mclk / (rate << 6)) : 4u;

        if (tmd < 2u)
            tmd = 2u;
        if (tmd > 64u)
            tmd = 64u;
        I2S_CKR = ((tmd - 1u) << 16) | 0x3F3Fu;
    }
    /* INTCR verbatim (ref FUN_03056550 "i2s regs"): 0x01F00102 — TX
     * underrun INT enable + FIFO threshold fields. The polled path does
     * not need the interrupts but the threshold bits gate the FIFO. */
    I2S_INTCR = 0x01F00102u;
    I2S_XFER = 1;
}

/* ---- ACODEC internal mux (verbatim ref, incl. the 0x214 load dance) -------- */

void acodec_mux(uint8_t m4, uint8_t m5, uint8_t m6, uint8_t m7)
{
    volatile uint32_t *r200 = (volatile uint32_t *)(uintptr_t)(ACODEC_BASE + 0x200u);
    volatile uint32_t *r204 = (volatile uint32_t *)(uintptr_t)(ACODEC_BASE + 0x204u);
    volatile uint32_t *r208 = (volatile uint32_t *)(uintptr_t)(ACODEC_BASE + 0x208u);
    volatile uint32_t *r20C = (volatile uint32_t *)(uintptr_t)(ACODEC_BASE + 0x20Cu);
    volatile uint32_t *r210 = (volatile uint32_t *)(uintptr_t)(ACODEC_BASE + 0x210u);
    volatile uint32_t *r214 = (volatile uint32_t *)(uintptr_t)(ACODEC_BASE + 0x214u);
    volatile uint32_t *mode_con = (volatile uint32_t *)(uintptr_t)(CRU_BASE + 0x10u);
    uint32_t v;

    /* CRU_MODE_CON = 0x200020 (WE bits 5/21, both set) — verbatim. */
    *mode_con = 0x200020u;

    *r200 &= ~4u;
    *r214 |= 4u;
    *r200 = (*r200 & ~0x18u) | 0x10u;
    *r204 = m4;                            /* ref: entry[4] -> +0x204     */
    *r208 = m5;                            /* ref: entry[5] -> +0x208     */
    *r20C = m6;                            /* ref: entry[6] -> +0x20c     */
    v = *r210;
    v &= ~0xF0u;
    v |= (uint32_t)m7 << 4;                /* ref: entry[7] -> +0x210 hi  */
    *r210 = v;
    *r214 &= ~1u;
    *r214 &= ~4u;
    dac_delay_ms(5);
    *r200 |= 4u;
}

/* ---- polled I2S feed -------------------------------------------------------- */

/* Diag: TX FIFO level extremes seen since rknano_i2s_start(). If the TXDR
 * offset were still wrong the level would stay 00-00 (writes into the
 * void); with a live serializer it oscillates 00..1d. */
static unsigned s_fifo_lo = 0x3Fu, s_fifo_hi = 0;

void rknano_i2s_fifo_range(unsigned *lo, unsigned *hi)
{
    *lo = s_fifo_lo;
    *hi = s_fifo_hi;
}

int rknano_i2s_feed(const int16_t *pcm, int frames)
{
    int i;
    unsigned stall;

    if (pcm == 0 || frames <= 0)
        return DAC_ERR_ARG;

    s_step = DAC_STEP_FEED;
    for (i = 0; i < frames; i++) {
        unsigned fl;

        /* Wait for TX FIFO room: FIFOLR[5:0] = TX level, write when below
         * 0x1e (SDK I2s.c I2S_PIO_Write). The old "== 0" waited while the
         * FIFO was EMPTY (inverted), so an empty FIFO spun to timeout ->
         * DAC_ERR_TIMEOUT, frames 0. */
        for (stall = 0; (I2S_FIFOLR & 0x3Fu) >= 0x1Eu; stall++) {
            if (stall > 8000000u)
                return DAC_ERR_TIMEOUT;     /* FIFO never drains: no clock */
        }
        fl = I2S_FIFOLR & 0x3Fu;
        if (fl < s_fifo_lo)
            s_fifo_lo = fl;
        if (fl > s_fifo_hi)
            s_fifo_hi = fl;
        /* Wire format (stock Bit_Convertor_DEC/shift + DMA, Service.c):
         * ONE 32-bit FIFO word = ONE channel sample, left-justified
         * (sample << 16); a stereo frame is TWO words, L then R, each in
         * one 32-BCLK slot of the 64-BCLK frame. The level wait above
         * covers both writes (threshold 0x1e leaves room for two). */
        I2S_TXDR = (uint32_t)(uint16_t)pcm[2 * i] << 16;
        I2S_TXDR = (uint32_t)(uint16_t)pcm[2 * i + 1] << 16;
    }
    return DAC_OK;
}

void rknano_i2s_start(void)
{
    s_fifo_lo = 0x3Fu;
    s_fifo_hi = 0;
    I2S_XFER = 1;
}

void rknano_i2s_stop(void)
{
    I2S_XFER = 0;
}

/* ---- ISR-driven TX feed (streaming; decode overlaps playback) ------------- */

/* The polled feed above is single-threaded: decode -> blocking feed, so any
 * decode burst longer than the ~32-frame TX FIFO gaps the stream ("sample,
 * silence, sample").  The stock streams instead: a ring drained into TXDR by
 * hardware (its DMA channel; route B in docs/HANDOVER.md).  Route A here is
 * the lighter equivalent: the I2S1 TX-empty/threshold IRQ (INT_ID_I2S1 =
 * enum 48 = NVIC IRQn 32, vector slot 48) pulls frames from a caller ring
 * while the main loop decodes into it — the 32 KiB ring absorbs decode/FS
 * latency.  INTCR/INTSR semantics per the SDK (Hw_i2s.h + I2sIntIsr):
 * INTCR bit0 = TX-empty IRQ enable, bit1 = TX-underrun IRQ enable, bit2 =
 * underrun clear (W1C-style), and INTSR TX-empty/underrun go inactive once
 * data lands in the FIFO. */

static rknano_i2s_pull_fn s_pull;
static void *s_pull_user;
static volatile int s_feed_hold;

void rknano_i2s_attach(rknano_i2s_pull_fn pull, void *user)
{
    s_pull = pull;
    s_pull_user = user;
    s_feed_hold = 0;
    /* Lowest NVIC priority (0xE0 = top 3 priority bits set; works for any
     * implemented priority width). The TX feeder must never preempt or
     * starve anything else (input/timers are polled today, but future IRQs
     * must win). The feeder is short (~a dozen FIFO writes) and the 32 KiB
     * ring means a delayed ISR cannot cause dropouts. */
    NVIC_IPR_BYTE_IRQ32 = 0xE0u;
    NVIC_ICPR1 = NVIC_IRQ32;             /* unpend before enabling          */
    NVIC_ISER1 = NVIC_IRQ32;
    /* TX-empty only. The stock base value 0x01F00102 also enables the
     * TX-underrun IRQ (bit 1) for its DMA handler; with our ring dry an
     * underrun would re-fire every frame (IRQ storm), and the threshold-
     * based TX-empty already wakes us for every refill. */
    I2S_INTCR = (I2S_INTCR | INTCR_TX_EMPTY) & ~INTCR_TX_UNDERR;
    __asm__ volatile("cpsie i" ::: "memory");
}

void rknano_i2s_detach(void)
{
    I2S_INTCR &= ~INTCR_TX_EMPTY;
    NVIC_ICER1 = NVIC_IRQ32;
    s_pull = 0;
}

void rknano_i2s_kick(void)
{
    if (s_pull != 0 && !s_feed_hold)
        I2S_INTCR |= INTCR_TX_EMPTY;     /* re-arm after ring dry / resume  */
}

void rknano_i2s_hold(int held)
{
    s_feed_hold = held ? 1 : 0;
    if (held)
        I2S_INTCR &= ~INTCR_TX_EMPTY;    /* mask source (no IRQ storm)      */
    else
        rknano_i2s_kick();
}

void rknano_i2s_tx_isr(void)
{
    if (I2S_INTSR & INTSR_TX_UR_ACT)
        I2S_INTCR |= INTCR_TX_UR_CLR;    /* clear underrun (SDK I2sIntIsr)  */

    if (s_pull == 0 || s_feed_hold) {
        I2S_INTCR &= ~INTCR_TX_EMPTY;
        return;
    }
    /* Top the FIFO up past the IRQ threshold (SDK I2S_PIO_Write level).
     * Each ring frame is one int32 stereo frame = TWO 32-bit slot words
     * (L then R); the sample is already left-justified, so each goes out
     * as-is — require room for two before pulling. */
    while ((I2S_FIFOLR & 0x3Fu) < 0x1Du) {
        uint32_t l, r;

        if (!s_pull(&l, &r, s_pull_user)) {
            /* Ring dry: mask the source so it cannot re-fire until the
             * producer pushes data and calls rknano_i2s_kick(). */
            I2S_INTCR &= ~INTCR_TX_EMPTY;
            break;
        }
        I2S_TXDR = l;                            /* L channel sample      */
        I2S_TXDR = r;                            /* R channel sample      */
    }
}
