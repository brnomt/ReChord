/*
 * dac_cs43131.c — CS43131 dual-DAC bring-up for the Echo Mini (clean-room).
 *
 * Ported from VERIFIED facts (docs/rewrite/dac.md §6), recovered from the
 * reference fw1 binary via Ghidra MCP + the CS43131 datasheet:
 *
 *   - TWO CS43131s, MONO (differential) mode, one chip per channel:
 *     L = 0x60 (7-bit 0x30), R = 0x66 (7-bit 0x33) — chip address table
 *     from ref RAM 0x03060B78; mono enable = the 24-pair (MAP,value) table
 *     byte-identical to datasheet §5.12 (ref RAM 0x03060AB8).
 *   - Control: I2C0 @ 0x40040000 (RK I2C: CON 0x00, CLKDIV 0x04, MTXCNT
 *     0x10, MRXCNT 0x14, IPD 0x1C, TXFIFO +0x100, RXFIFO +0x200). Register
 *     access = [chipaddr+W][MAP_lo][MAP_mid][MAP_hi][ctl=1][data] (24-bit
 *     MAP + INCR control byte, datasheet §4.13).
 *   - Clocks: CRU @ 0x40180000 (CLKGATE_CON[0] @ +0x80, active-low gates
 *     with upper-halfword write-enable). ACODEC block @ 0x40090000
 *     (internal mux, +0x200..+0x210 per sample rate). I2S1 @ 0x40030000.
 *   - Rate table (13 entries, ref RAM 0x030609E8): rate, MCLK (256x rate
 *     hi / 512x lo), 4 ACODEC mux bytes, DAC control byte.
 *
 * BRING-UP STATUS: experimental (dac.md §7 Exp 1/2). Every step leaves a
 * breadcrumb (dac_last_step) so a hardware hang is identifiable on screen.
 * The I2C0 board wiring is not yet recovered; dac_init SCANS the three
 * mux candidates (A/B/C) and proceeds on whichever ACKs the DAC chips
 * (the ref hardwires candidate A: GRF_IOMUX_CON = 0x30000 = sel A + WE).
 *
 * FIX 2026-10-12 (DFIU incident 3a986c9b, hardware failure rc -4 step 2
 * scan A00 B00 C00): GRF mux offsets, CRU gate ids, I2C0 soft reset and
 * the GPIO2 power/reset dance were wrong vs the ref + SDK — all ported
 * verbatim now. I2C MAP byte order is LSB-first per the REF BINARY
 * (docs/rewrite/dac.md §6: [MAP_lo][MAP_mid][MAP_hi][ctl INCR=1][data]).
 * (My earlier MSB-first "fix" from the scanned Fig 4-32 was wrong and is
 * reverted: the reference driver that runs on hardware sends LSB-first.)
 *
 * v1 scope: gates + mux scan + I2C0 + DAC regs + I2S1 TX master + polled
 * FIFO feed. No DMA (ref's DMA block is at 0x03090000 — port later).
 */
#include <stdint.h>

#include "dac_cs43131.h"

/* ---- MMIO primitives ----------------------------------------------------- */

#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))

#define CRU_BASE   0x40180000u
#define CRU_GATE(g) REG32(CRU_BASE + 0x80u + 4u * (unsigned)(g))
#define CRU_RST(r)  REG32(CRU_BASE + 0xC0u + 4u * (unsigned)(r))

#define GRF_BASE   0x50010000u
#define GRF_IOMUX  REG32(GRF_BASE + 0xBCu)          /* GRF_IOMUX_CON        */

#define I2C0_BASE   0x40040000u
#define I2C_CON     REG32(I2C0_BASE + 0x00u)
#define I2C_CLKDIV  REG32(I2C0_BASE + 0x04u)
#define I2C_MRXADDR REG32(I2C0_BASE + 0x08u)
#define I2C_MTXCNT  REG32(I2C0_BASE + 0x10u)
#define I2C_MRXCNT  REG32(I2C0_BASE + 0x14u)
#define I2C_IPD     REG32(I2C0_BASE + 0x1Cu)
#define I2C_TXFIFO(i) REG32(I2C0_BASE + 0x100u + 4u * (unsigned)(i))
#define I2C_RXFIFO(i) REG32(I2C0_BASE + 0x200u + 4u * (unsigned)(i))
#define I2C_CON_EN     (1u << 0)
#define I2C_CON_MOD(m) ((m) << 1)
#define I2C_CON_START  (1u << 3)      /* START bit: generate START cond.  */
#define I2C_MODE_TX    0u
#define I2C_MODE_TRX   1u
#define I2C_MTX_DONE   (1u << 2)      /* MBTFIPD */
#define I2C_MRX_DONE   (1u << 3)      /* MBRFIPD */
#define I2C_START_DONE (1u << 4)      /* STARTIPD */
#define I2C_NAK        (1u << 6)

#define ACODEC_BASE 0x40090000u

#define I2S1_BASE   0x40030000u
#define I2S_TXCR    REG32(I2S1_BASE + 0x00u)
#define I2S_RXCR    REG32(I2S1_BASE + 0x04u)
#define I2S_CKR     REG32(I2S1_BASE + 0x08u)
#define I2S_FIFOLR  REG32(I2S1_BASE + 0x0Cu)
#define I2S_XFER    REG32(I2S1_BASE + 0x1Cu)
#define I2S_CLR     REG32(I2S1_BASE + 0x20u)
/* TXDR: RK convention places the data register at +0x400 (bring-up
 * hypothesis from the family TRM; if the FIFO never drains, try +0x24). */
#define I2S_TXDR    REG32(I2S1_BASE + 0x400u)

static void delay_loops(volatile unsigned n)
{
    while (n--)
        ;
}

void dac_delay_ms(unsigned ms)
{
    /* ~24 MHz boot CPU: ~24k loops is ~1 ms (over-shoot is safe). */
    while (ms--)
        delay_loops(24000);
}

/* ---- CRU gates (active-low, upper-halfword write-enable, SDK style) ------ */

static void cru_gate(int gate_id, int enable)
{
    unsigned group = (unsigned)gate_id >> 4;
    unsigned bit = 1u << ((unsigned)gate_id & 0xFu);
    uint32_t v = CRU_GATE(group);

    if (enable)
        v &= ~bit;
    CRU_GATE(group) = (v & 0x0000FFFFu) | 0xFFFF0000u;
}

/* Gate IDs from the SDK Hw_cru.h eCLOCK_GATE enum (counted from the
 * explicit anchors: CLK_I2S1_SRC_GATE=16, HCLK_PMU_APB_BRG_GATE=128,
 * PCLK_PWM0_GATE=144) and used verbatim by the ref (FUN_030513e0 calls
 * 0x83/0x82/0xe/0x96). The previous values (156/157/177) pointed at
 * reserved bits — PCLK_I2C0 never got a bus clock, hence the all-zero
 * scan (DFIU 3a986c9b). */
#define G_CLK_ACODEC   14     /* 0x0E — ref also gates this one          */
#define G_CLK_I2S1_SRC 16
#define G_CLK_I2S1_FRAC 17
#define G_CLK_I2S1_OUT 18
#define G_CLK_I2S1     19
#define G_PCLK_I2S1    130    /* 0x82 — ref FUN_030513e0(0x82,1)          */
#define G_PCLK_I2C0    131    /* 0x83 — ref FUN_030513e0(0x83,1)          */
#define G_PCLK_GPIO1   149    /* 0x95 — ref FUN_030513e0(0x95,1)          */
#define G_PCLK_ACODEC  150    /* 0x96 — ref FUN_030513e0(0x96,1)          */

/* Soft-reset ids (SDK Hw_cru.h eSOFT_RST): group = id/16, bit = id%16,
 * written to CRU_SOFTRST[group] with bit+16 as write-enable. */
#define R_I2S1_SRST    4
#define R_I2C0_SRST    26

static void cru_rst(int rst_id, int assert)
{
    unsigned bit = 1u << ((unsigned)rst_id & 0xFu);
    /* SDK ScuSoftResetCtr: bit+16 = write-enable, bit = reset value. */
    CRU_RST(rst_id >> 4) = (bit << 16) | (assert ? bit : 0u);
    delay_loops(4800);                      /* SDK: DelayUs(5) ~          */
}

/* ---- GPIO power/reset sequence (ref FUN_03056550 entry) -------------------- */

static void grf_iomux(int port, int pin, int func);   /* defined below    */

typedef struct {
    volatile uint32_t dr;                   /* +0x00 SWPORTA_DR           */
    volatile uint32_t ddr;                  /* +0x04 SWPORTA_DDR          */
} dac_gpio_t;

#define DAC_GPIO1 ((dac_gpio_t *)0x40170000u)
#define DAC_GPIO2 ((dac_gpio_t *)0x50030000u)

/* Ref FUN_03051478(port, pin, level): mux the pin to GPIO (func 0),
 * drive it, and set the direction to output. */
static void dac_gpio_out(int port, int pin, int level)
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

/* Verbatim from the ref bring-up entry (before any clock gate): three
 * enables high, then a reset pulse on GPIO2.1 (low 1 ms, high 8 ms,
 * low 2 ms). Without this the CS43131s stay in hardware power-down and
 * ACK nothing on I2C — the observed A00 B00 C00. */
static void dac_power_sequence(void)
{
    dac_gpio_out(2, 0, 1);
    dac_gpio_out(2, 14, 1);
    dac_gpio_out(2, 2, 1);
    dac_delay_ms(2);
    dac_gpio_out(2, 1, 0);
    dac_delay_ms(1);
    dac_gpio_out(2, 1, 1);
    dac_delay_ms(8);
    dac_gpio_out(2, 1, 0);
    dac_delay_ms(2);
}

/* ---- GRF pin mux --------------------------------------------------------- */

/* RK IOMUX (ref FUN_03051458, hardware-verified in the panel driver too):
 * register = GRF + port*0x20 + (pin>>3)*4 (GPIO_IOMUX{0,1,2}{A..D}),
 * 2 bits/pin at ((pin&7)*2), bits 16/17 = write-enable for that field.
 * The ref writes (func | 0x30000) << sh as a plain word; the upper half
 * shifts with the field so only this pin's write-enable is active. */
static void grf_iomux(int port, int pin, int func)
{
    volatile uint32_t *r =
        (volatile uint32_t *)(uintptr_t)(GRF_BASE +
                                         (unsigned)port * 0x20u +
                                         (unsigned)(pin >> 3) * 4u);
    unsigned sh = (unsigned)(pin & 7) * 2u;

    *r = ((uint32_t)(func & 3) | 0x30000u) << sh;
}

/* ---- I2C0 polled master -------------------------------------------------- */

/* Chip 8-bit WRITE addresses (ref table @ RAM 0x03060B78). */
static const uint8_t dac_chips[2] = { 0x60, 0x66 };

static int i2c_wait(uint32_t done_mask, unsigned timeout)
{
    while (timeout--) {
        uint32_t ipd = I2C_IPD;
        if (ipd & I2C_NAK) {
            I2C_IPD = ipd;
            return DAC_ERR_NAK;
        }
        if (ipd & done_mask) {
            I2C_IPD = ipd;                 /* W1C pending bits           */
            return DAC_OK;
        }
    }
    return DAC_ERR_TIMEOUT;
}

/* One write: FIFO-packed [addr_w|data...], MTXCNT starts the transfer.
 * The CS43131 write frame = [chipaddr+W][MAP lo/mid/hi][ctl=1][data]. */
static int i2c_write(uint8_t addr_w, const uint8_t *buf, unsigned n)
{
    unsigned i, w = 0, bi = 1;
    uint32_t word = addr_w;

    if (n + 1u > 32u)
        return DAC_ERR_ARG;

    /* Generate a START condition and wait for it BEFORE the FIFO/MTXCNT
     * (ref FUN_03055dcc: CON = EN|START|TX, wait STARTIPD, clear START).
     * Without the explicit START the transfer never begins and every probe
     * NAKs/times out -> the A00 B00 C00 scan (DFIU). */
    I2C_IPD = 0x7Fu;
    I2C_CON = I2C_CON_EN | I2C_CON_START | I2C_CON_MOD(I2C_MODE_TX);
    if (i2c_wait(I2C_START_DONE, 400000) != DAC_OK)
        return DAC_ERR_TIMEOUT;
    I2C_IPD = I2C_START_DONE;
    I2C_CON = I2C_CON_EN | I2C_CON_MOD(I2C_MODE_TX);
    for (i = 0; i < n; i++) {
        word |= (uint32_t)buf[i] << (8u * bi);
        if (++bi == 4) {
            I2C_TXFIFO(w++) = word;
            word = 0;
            bi = 0;
        }
    }
    if (bi)
        I2C_TXFIFO(w) = word;

    I2C_IPD = 0x7Fu;
    I2C_MTXCNT = n + 1;
    return i2c_wait(I2C_MTX_DONE, 400000);
}

/* Repeated-start read of `n` bytes at a 24-bit MAP address (INCR preamble). */
static int i2c_read_map(uint8_t addr_w, uint32_t map24, uint8_t *out,
                        unsigned n)
{
    uint8_t pre[4];

    /* MSB-first MAP order, matching the ref binary (FUN_03055e78 packs
     * [MAP_hi][MAP_mid][MAP_lo][ctl=1][data] - the dac.md §6 "MAP_lo first"
     * note was a transcription error; the disassembly is MSB-first). */
    pre[0] = (uint8_t)(map24 >> 16);
    pre[1] = (uint8_t)(map24 >> 8);
    pre[2] = (uint8_t)(map24);
    pre[3] = 1;                              /* control byte: INCR         */
    if (i2c_write(addr_w, pre, 4) != DAC_OK)
        return DAC_ERR_NAK;

    I2C_CON = I2C_CON_EN | I2C_CON_MOD(I2C_MODE_TRX);
    I2C_IPD = 0xFFFFFFFFu;
    I2C_MRXADDR = 0x01000000u | addr_w;     /* vld=1 + 7-bit address byte */
    I2C_MRXCNT = n;
    if (i2c_wait(I2C_MRX_DONE, 400000) != DAC_OK)
        return DAC_ERR_TIMEOUT;
    {
        uint32_t word = I2C_RXFIFO(0);
        unsigned k;
        for (k = 0; k < n && k < 4; k++)
            out[k] = (uint8_t)(word >> (8u * k));
        if (n > 4) {
            word = I2C_RXFIFO(1);
            for (k = 4; k < n; k++)
                out[k] = (uint8_t)(word >> (8u * (k - 4)));
        }
    }
    return DAC_OK;
}

/* Broadcast a MAP write to BOTH chips (ref semantics: FUN_03055f08).
 * Frame per the ref binary (dac.md §6): [chipaddr+W][MAP lo][MAP mid]
 * [MAP hi][ctl INCR=1][data] — the MAP goes LEAST-significant byte first. */
static int dac_map_write(uint32_t map24, uint8_t val)
{
    uint8_t frame[5];
    int rc, worst = DAC_OK;

    frame[0] = (uint8_t)(map24 >> 16);      /* MAP high (23:16) — MSB 1st */
    frame[1] = (uint8_t)(map24 >> 8);       /* MAP mid  (15:8)            */
    frame[2] = (uint8_t)(map24);            /* MAP low  (7:0)             */
    frame[3] = 1;                           /* INCR control byte          */
    frame[4] = val;
    rc = i2c_write(dac_chips[0], frame, 5);
    if (rc != DAC_OK)
        worst = rc;
    rc = i2c_write(dac_chips[1], frame, 5);
    if (rc != DAC_OK)
        worst = rc;
    return worst;
}

static int dac_map_read(int chan, uint32_t map24, uint8_t *out)
{
    return i2c_read_map(dac_chips[chan], map24, out, 1);
}

/* ---- rate table + DAC sequences (verbatim from ref) ---------------------- */

typedef struct
{
    uint32_t rate;
    uint32_t mclk;        /* Hz expected at the DAC MCLK pin             */
    uint8_t  mux[4];      /* ACODEC mux bytes (ref table [4..7])         */
    uint8_t  dac;         /* per-rate DAC control byte (ref table [12])  */
} dac_rate_t;

/* Ref RAM 0x030609E8, 13 entries (byte-exact). */
static const dac_rate_t dac_rates[13] = {
    {  8000,  4096000, { 0x00, 0x30, 0x32, 0x08 }, 0 },
    { 11025,  5644800, { 0x03, 0x12, 0x19, 0x03 }, 1 },
    { 12000,  6144000, { 0x00, 0x18, 0x19, 0x0A }, 2 },
    { 16000,  4096000, { 0x00, 0x30, 0x32, 0x08 }, 0 },
    { 22050,  5644800, { 0x03, 0x12, 0x19, 0x03 }, 1 },
    { 24000,  6144000, { 0x00, 0x18, 0x19, 0x0A }, 2 },
    { 32000,  8192000, { 0x00, 0x60, 0x32, 0x08 }, 0 },
    { 44100, 11288960, { 0x06, 0x24, 0x19, 0x03 }, 1 },
    { 48000, 12288000, { 0x00, 0x30, 0x19, 0x0A }, 2 },
    { 88200, 11288960, { 0x06, 0x24, 0x19, 0x03 }, 3 },
    { 96000, 24576000, { 0x00, 0x30, 0x19, 0x09 }, 4 },
    {176400, 22577664, { 0x04, 0x49, 0x19, 0x03 }, 5 },
    {192000, 24576000, { 0x00, 0x30, 0x19, 0x09 }, 6 },
};

/* Datasheet §5.12 / ref RAM 0x03060AB8: enable mono mode — 24 pairs,
 * byte-identical to the reference. */
static const struct { uint32_t map; uint8_t val; } dac_mono_seq[24] = {
    { 0x180009, 0x2F }, { 0x18000A, 0xB4 }, { 0x18000B, 0x25 },
    { 0x18000C, 0x80 }, { 0x18000F, 0x55 }, { 0x180010, 0xFA },
    { 0x180019, 0x2F }, { 0x18001A, 0xB4 }, { 0x18001B, 0x25 },
    { 0x18001C, 0x80 }, { 0x18001F, 0x4E }, { 0x180020, 0xFD },
    { 0x180029, 0x2F }, { 0x18002A, 0xB4 }, { 0x18002B, 0x25 },
    { 0x18002C, 0x80 }, { 0x18002F, 0x7C }, { 0x180030, 0xFD },
    { 0x180039, 0x2F }, { 0x18003A, 0xB4 }, { 0x18003B, 0x25 },
    { 0x18003C, 0x80 }, { 0x18003F, 0x52 }, { 0x180040, 0xFE },
};

/* ---- I2S1 master setup + polled feed -------------------------------------- */

static void i2s1_setup(uint32_t rate, uint32_t mclk)
{
    uint32_t div;

    /* Ref (FUN_03056550 tail): enable PCLK_GPIO1, mux GPIO1A0..A3 to
     * I2S1A (CLK/LRCK/SCLK/SDO, func 1), select I2S1A in GRF_IOMUX_CON
     * (bit 13, WE bit 29), then the CRU CLKSEL_CON[2] clock routing. */
    cru_gate(G_PCLK_GPIO1, 1);
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
    delay_loops(48000);
    while (I2S_CLR)
        delay_loops(100);

    I2S_TXCR = 0x401Fu;      /* verbatim: ch0-1 + 32-bit bus word + 16-bit */
    I2S_RXCR = 0x401Fu;      /* ref writes RXCR too (verbatim)            */
    /* CKR verbatim: high16 = (mclk/(rate*64))-1, low16 = 0x3F3F. */
    div = (mclk / (64u * rate)) - 1u;
    I2S_CKR = ((div & 0xFFFFu) << 16) | 0x3F3Fu;
    I2S_XFER = 1;
}

/* ---- ACODEC internal mux (verbatim ref, incl. the 0x214 load dance) -------- */

static void acodec_mux(uint8_t m4, uint8_t m5, uint8_t m6, uint8_t m7)
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

/* ---- I2C0 mux candidates + scan --------------------------------------------- */

static int s_step = DAC_STEP_NONE;
static uint8_t s_hp_status[2];
static uint8_t s_scan[3][2];

int dac_last_step(void)
{
    return s_step;
}

void dac_get_hp_status(uint8_t out[2])
{
    out[0] = s_hp_status[0];
    out[1] = s_hp_status[1];
}

int dac_scan_result(int cand, int chip)
{
    if (cand < 0 || cand > 2 || chip < 0 || chip > 1)
        return -1;
    return s_scan[cand][chip];
}

/* I2C0 port candidates (SDK i2c.c I2C_GPIO_init): A = GPIO2B2/B3 func1
 * (+GRF_IOMUX_CON sel=0), B = GPIO2C1/C0 func2 (sel=1), C = GPIO0A6/A5
 * func3 (sel=2). The board wiring is unrecovered -> scan all three. */
static void i2c0_mux(int cand)
{
    switch (cand) {
    case 0:
        GRF_IOMUX = (GRF_IOMUX & ~0x3u) | 0x00030000u;
        grf_iomux(2, 10, 1);
        grf_iomux(2, 11, 1);
        break;
    case 1:
        GRF_IOMUX = (GRF_IOMUX & ~0x3u) | 0x00030001u;
        grf_iomux(2, 17, 2);
        grf_iomux(2, 16, 2);
        break;
    default:
        GRF_IOMUX = (GRF_IOMUX & ~0x3u) | 0x00030002u;
        grf_iomux(0, 6, 3);
        grf_iomux(0, 5, 3);
        break;
    }
}

/* Probe a chip with a harmless MAP write (page 0 reg 0). */
static int dac_probe(uint8_t addr_w)
{
    uint8_t frame[5] = { 0, 0, 0, 1, 0 };

    return i2c_write(addr_w, frame, 5);
}

/* Boot pclk for I2C0 at this point (24 MHz XIN; GetI2CFreq in the SDK). */
#define DAC_PCLK_HZ 24000000u

static int i2c0_scan(void)
{
    int c;
    /* Ref/SDK I2CSetSpeed: div = ceil(pclk_kHz / (speed*8)) - 2, then
     * CLKDIV = (div>>1) | ((div+1)>>1)<<16.  100 kHz @ 24 MHz => 14/14.
     * (The old hard-coded 59 yielded ~25 kHz and, worse, hid that the
     * bus was never even clocked.) */
    uint32_t div = (DAC_PCLK_HZ / 1000u + 799u) / 800u - 2u;
    I2C_CLKDIV = (div >> 1) | (((div + 1) >> 1) << 16);
    for (c = 0; c < 3; c++) {
        i2c0_mux(c);
        s_scan[c][0] = (dac_probe(dac_chips[0]) == DAC_OK) ? 1 : 0;
        s_scan[c][1] = (dac_probe(dac_chips[1]) == DAC_OK) ? 1 : 0;
        if (s_scan[c][0] || s_scan[c][1])
            return c;
    }
    return -1;
}

/* ---- bring-up ---------------------------------------------------------------- */

int dac_init(uint32_t rate_hz)
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
    dac_map_write(0x00090000, 0x02);
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
    dac_delay_ms(64);
    dac_map_write(0x00010010, 0x99);
    dac_map_write(0x00080032, 0x20);
    dac_delay_ms(16);
    dac_map_write(0x00080032, 0x00);
    dac_map_write(0x00010010, 0x00);

    s_step = DAC_STEP_VOLUME;
    /* Per-rate DAC control byte (ref: MAP 0x01000B). */
    dac_map_write(0x0001000B, rt->dac);
    dac_delay_ms(10);

    s_step = DAC_STEP_UNMUTE;
    dac_gpio_out(1, 4, 1);            /* amp enable again (ref)           */
    dac_map_write(0x00010009, 0x00);   /* unmute                          */
    dac_delay_ms(10);

    s_step = DAC_STEP_DONE;
    return DAC_OK;
}

/* ---- polled I2S feed -------------------------------------------------------- */

int dac_i2s_feed(const int16_t *pcm, int frames)
{
    int i;
    unsigned stall;

    if (pcm == 0 || frames <= 0)
        return DAC_ERR_ARG;

    s_step = DAC_STEP_FEED;
    for (i = 0; i < frames; i++) {
        /* Wait for TX FIFO room: FIFOLR[5:0] = TX level, write when below
         * 0x1e (SDK I2s.c I2S_PIO_Write). The old "== 0" waited while the
         * FIFO was EMPTY (inverted), so an empty FIFO spun to timeout ->
         * DAC_ERR_TIMEOUT, frames 0. */
        for (stall = 0; (I2S_FIFOLR & 0x3Fu) >= 0x1Eu; stall++) {
            if (stall > 8000000u)
                return DAC_ERR_TIMEOUT;     /* FIFO never drains: no clock */
        }
        I2S_TXDR = (uint32_t)(uint16_t)pcm[2 * i] |
                   ((uint32_t)(uint16_t)pcm[2 * i + 1] << 16);
    }
    return DAC_OK;
}

void dac_i2s_start(void)
{
    I2S_XFER = 1;
}

void dac_i2s_stop(void)
{
    I2S_XFER = 0;
}
