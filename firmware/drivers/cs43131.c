/*
 * cs43131.c — CS43131 dual-DAC chip driver (I2C0 master + chip sequences).
 *
 * VERBATIM relocation from dac_cs43131.c (commit c5d571f) — plan hard
 * constraint 2: the I2C transaction flow, MAP byte order and every chip
 * register write below are 1:1 unchanged and hardware-verified (DFIU
 * 158f67d3 I2C START, 5d8b7b3c MAP/read flow, 60609d7f teardown).
 *
 * FIX 2026-10-13 (DFIU 5d8b7b3c, every register read 0xFF): the MAP byte
 * order debate is SETTLED — it is MSB-first ([MAP_hi][MAP_mid][MAP_lo]),
 * byte-exact from ref FUN_03055e78 (strb map>>16, >>8, &0xFF) AND
 * datasheet §4.13 Fig 4-32/4-33/4-34 ("MAP BYTE High (23:16), Mid (15:8),
 * Low (7:0)"). The read follows ref FUN_03055fe8 byte-for-byte:
 * aborted-write preamble + STOP (Fig 4-34), then TRX-mode receive
 * (MRXADDR=vld|(addr|R), MRXRADDR=0, CON=0x0B, LASTACK, MRXCNT, MBRFIPD).
 * This is the datasheet-blessed CS43131 transaction flow; a true
 * repeated-START variant exists in the datasheet but the stock uses this
 * one — kept exactly (plan: document, do not churn).
 */
#include <stdint.h>

#include "cs43131.h"
#include "ao_regs.h"
#include "rknano_i2s.h"     /* dac_delay_ms, dac_gpio_out/input (names kept) */

/* ---- bring-up diag state (shared; names kept from c5d571f) --------------- */

int s_step = DAC_STEP_NONE;
uint8_t s_hp_status[2];
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

int dac_read_reg(int chan, uint32_t map24, uint8_t *out)
{
    return dac_map_read(chan, map24, out);
}

int dac_write_reg(int chan, uint32_t map24, uint8_t val)
{
    return dac_map_write_chan(chan, map24, val);
}

int dac_scan_result(int cand, int chip)
{
    if (cand < 0 || cand > 2 || chip < 0 || chip > 1)
        return -1;
    return s_scan[cand][chip];
}

/* ---- GPIO power/reset sequence (ref FUN_03056550 entry) -------------------- */

/* Verbatim from the ref bring-up entry (before any clock gate): three
 * enables high, then a reset pulse on GPIO2.1 (low 1 ms, high 8 ms,
 * low 2 ms). Without this the CS43131s stay in hardware power-down and
 * ACK nothing on I2C — the observed A00 B00 C00. */
void dac_power_sequence(void)
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

/* Transaction tail, verbatim ref FUN_03055da8 (it is the STOP helper, not a
 * read!): CON = 0x11 (EN|STOP), wait STOPIPD, clear IPD, CON = 0. The
 * CS43131 keeps its MAP across STOP (datasheet Fig 4-34 blesses an
 * "aborted write" preamble that ends in STOP), so this is also the legal
 * separator between the MAP write and the data read. */
static void i2c_stop(void)
{
    I2C_IPD = 0x7Fu;
    I2C_CON = I2C_CON_EN | I2C_CON_STOP;    /* 0x11                       */
    (void)i2c_wait(I2C_STOP_DONE, 400000);
    I2C_IPD = 0x7Fu;
    I2C_CON = 0;                            /* ref parks the module here  */
}

/* One write: FIFO-packed [addr_w|data...], MTXCNT starts the transfer.
 * The CS43131 write frame = [chipaddr+W][MAP hi/mid/lo][ctl=1][data]
 * (MSB-first MAP — datasheet Fig 4-32). */
static int i2c_write(uint8_t addr_w, const uint8_t *buf, unsigned n)
{
    unsigned i, w = 0, bi = 1;
    uint32_t word = addr_w;
    int rc;

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
    rc = i2c_wait(I2C_MTX_DONE, 400000);
    i2c_stop();                        /* ref FUN_03055dcc tail: STOP + CON=0 */
    return rc;
}

/* Read `n` bytes at a 24-bit MAP address. Stock-exact two-transaction flow
 * (ref FUN_03055fe8 + datasheet Fig 4-34):
 *   1. "aborted write" preamble: [addr+W][MAP hi][MAP mid][MAP lo][ctl=1]
 *      + STOP (i2c_write() now ends in the FUN_03055da8 STOP tail).
 *   2. master-receive: MRXADDR = valid | (addr|R), MRXRADDR = 0 (MAP is
 *      already latched, no register phase), CON = EN|MOD(TRX)|START (0x0B),
 *      wait STARTIPD, clear START, CON |= LASTACK (NAK the last byte),
 *      MRXCNT = n, wait MBRFIPD (0x08), read RXFIFO, STOP tail.
 *
 * The old code wrote CON = 0x11 — which is EN|STOP (bit 4 = STOP; the SDK
 * comment "0x11 b 10001" is I2C_send_stop_bit) — and waited IPD 0x20 =
 * STOPIPD. It only emitted the preamble's STOP and NEVER ran a receive, so
 * RXFIFO (empty reads as 0xFFFFFFFF) yielded 0xFF for every register
 * (DFIU 5d8b7b3c). FUN_03055da8 is the STOP helper, not a read. */
static int i2c_read_map(uint8_t addr_w, uint32_t map24, uint8_t *out,
                        unsigned n)
{
    uint8_t pre[4];
    unsigned k;

    if (n == 0 || n > 32u)
        return DAC_ERR_ARG;

    /* MSB-first MAP order ([23:16][15:8][7:0]) — byte-exact from ref
     * FUN_03055e78 (strb >>16, >>8, &0xFF) and datasheet Fig 4-32/4-34. */
    pre[0] = (uint8_t)(map24 >> 16);         /* MAP high (23:16) first     */
    pre[1] = (uint8_t)(map24 >> 8);
    pre[2] = (uint8_t)(map24);
    pre[3] = 1;                              /* control byte: INCR         */
    if (i2c_write(addr_w, pre, 4) != DAC_OK) /* ends in STOP (Fig 4-34)    */
        return DAC_ERR_NAK;

    I2C_IPD = 0x7Fu;
    I2C_IEN = 0;                             /* polled, like the ref       */
    I2C_MRXADDR  = 0x01000000u | (uint32_t)(addr_w | 1u); /* vld + addr|R */
    I2C_MRXRADDR = 0;                        /* no register-address phase  */
    I2C_CON = I2C_CON_EN | I2C_CON_MOD(I2C_MODE_TRX) | I2C_CON_START; /* 0x0B */
    if (i2c_wait(I2C_START_DONE, 400000) != DAC_OK) {
        i2c_stop();
        return DAC_ERR_TIMEOUT;
    }
    I2C_IPD = I2C_START_DONE;
    I2C_CON &= ~I2C_CON_START;
    I2C_IPD = 0x7Fu;
    I2C_CON |= I2C_CON_LASTACK;              /* NAK the final byte         */
    I2C_MRXCNT = n;                          /* arms the receive           */
    if (i2c_wait(I2C_MRX_DONE, 400000) != DAC_OK) {
        i2c_stop();
        return DAC_ERR_TIMEOUT;
    }
    for (k = 0; k < n; k += 4u) {            /* RXFIFO words, byte j<<8*j  */
        uint32_t word = I2C_RXFIFO(k / 4u);
        unsigned j;
        for (j = 0; j < 4u && k + j < n; j++)
            out[k + j] = (uint8_t)(word >> (8u * j));
    }
    i2c_stop();
    return DAC_OK;
}

/* ---- MAP primitives ------------------------------------------------------ */

/* Per-chip MAP write (ref FUN_03055ecc writes different L/R values). */
int dac_map_write_chan(int chan, uint32_t map24, uint8_t val)
{
    uint8_t frame[5];

    frame[0] = (uint8_t)(map24 >> 16);      /* MAP high (MSB first)       */
    frame[1] = (uint8_t)(map24 >> 8);
    frame[2] = (uint8_t)(map24);
    frame[3] = 1;
    frame[4] = val;
    return i2c_write(dac_chips[chan & 1], frame, 5);
}

/* Broadcast a MAP write to BOTH chips (ref semantics: FUN_03055f08).
 * Frame per the ref binary (FUN_03055e78): [chipaddr+W][MAP hi][MAP mid]
 * [MAP lo][ctl INCR=1][data] — the MAP goes MOST-significant byte first. */
int dac_map_write(uint32_t map24, uint8_t val)
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

int dac_map_read(int chan, uint32_t map24, uint8_t *out)
{
    return i2c_read_map(dac_chips[chan], map24, out, 1);
}

/* Power-up RMW — verbatim ref FUN_03056c68(mask): clear the PDN bits in
 * 0x020000 on BOTH chips (chan 0 then chan 1, ~100 us apart). The bring-up
 * deliberately parks the chip powered DOWN (0x020000 = 0xF6) while it is
 * configured, then releases blocks here: 0x40 = PDN_ASP (serial port),
 * 0x10 = PDN_HP (headphone driver). Without these two calls the chip
 * configures perfectly, reads back sensibly — and outputs silence
 * forever. (The ref tail called this where an earlier port saw
 * "movs r0,#64/#16" and misread the calls as delays!) */
void dac_pdn_up(uint8_t mask)
{
    int chan;

    for (chan = 0; chan < 2; chan++) {
        uint8_t v = 0xFF;

        dac_delay_ms(1);                    /* ref: DelayUs(100)           */
        if (dac_map_read(chan, 0x00020000, &v) == DAC_OK)
            dac_map_write_chan(chan, 0x00020000, (uint8_t)(v & ~(uint8_t)mask));
        dac_delay_ms(1);
    }
}

/* Datasheet §5.12 / ref RAM 0x03060AB8: enable mono mode — 24 pairs,
 * byte-identical to the reference. */
const dac_mono_pair_t dac_mono_seq[24] = {
    { 0x180009, 0x2F }, { 0x18000A, 0xB4 }, { 0x18000B, 0x25 },
    { 0x18000C, 0x80 }, { 0x18000F, 0x55 }, { 0x180010, 0xFA },
    { 0x180019, 0x2F }, { 0x18001A, 0xB4 }, { 0x18001B, 0x25 },
    { 0x18001C, 0x80 }, { 0x18001F, 0x4E }, { 0x180020, 0xFD },
    { 0x180029, 0x2F }, { 0x18002A, 0xB4 }, { 0x18002B, 0x25 },
    { 0x18002C, 0x80 }, { 0x18002F, 0x7C }, { 0x180030, 0xFD },
    { 0x180039, 0x2F }, { 0x18003A, 0xB4 }, { 0x18003B, 0x25 },
    { 0x18003C, 0x80 }, { 0x18003F, 0x52 }, { 0x180040, 0xFE },
};

/* ---- volume ------------------------------------------------------------- */

/* Volume registers = PCM_VOLUME_B/A (MAP 0x090001/0x090002, datasheet
 * register map p.128): 0.5 dB/step attenuation, 0x00 = 0 dB. = ref
 * FUN_03055f28(v) which writes BOTH registers.
 * NOTE (fix 2026-10-16): an earlier port mapped this writer to MAP
 * 0x010009 — a RESERVED register (datasheet 0x01 0007-000A) — leaving
 * PCM_VOLUME_A/B at their 0x78 (-60 dB) reset default: playback was
 * extremely quiet. Datasheet-recommended init writes 0x00 (0 dB). */
void dac_volume_set(uint8_t atten)
{
    dac_map_write(0x00090001, atten);
    dac_map_write(0x00090002, atten);
}

/* Soft ramp = ref FUN_03055f48(from, to), ported 1:1: +-6 codes per step
 * (3 dB at 0.5 dB/step), 2 ms apart, clamped to the target. */
void dac_volume_ramp(uint8_t from, uint8_t to)
{
    int v = (int)from;
    int target = (int)to;
    int step = (target < v) ? -6 : 6;

    while (v != target) {
        v += step;
        if (step < 0) {
            if (v < target)
                v = target;
        } else if (target <= v) {
            v = target;
        }
        dac_volume_set((uint8_t)v);
        dac_delay_ms(2);
    }
}

/* BRNOMT-67: the same 6-code step, paced so the whole ramp takes `ms`.
 * At 0 dB the range is 43 steps, so 300 ms becomes ~7 ms/step; the DAC's
 * PCM_SZC soft ramp smooths each step on top of this. */
void dac_volume_ramp_ms(uint8_t from, uint8_t to, unsigned ms)
{
    int v = (int)from;
    int target = (int)to;
    int step = (target < v) ? -6 : 6;
    int steps;
    unsigned per_step;

    if (v == target)
        return;
    if (ms == 0) {
        dac_volume_set((uint8_t)target);
        return;
    }

    steps = ((v > target) ? (v - target) : (target - v) + 5) / 6;
    if (steps < 1)
        steps = 1;
    per_step = ms / (unsigned)steps;
    if (per_step < 1)
        per_step = 1;

    while (v != target) {
        v += step;
        if (step < 0) {
            if (v < target)
                v = target;
        } else if (target <= v) {
            v = target;
        }
        dac_volume_set((uint8_t)v);
        if (v != target)
            dac_delay_ms(per_step);
    }
}

/* Output teardown + close — ref FUN_03055F80 (audio-stop) + the I2S half
 * of FUN_030564d8 (audiocodec_close): hardware mute back ON (GPIO2.0=1),
 * DAC mute reg 0x010009 = 0xFF, 0x020000 = 0x10 (PDN_HP), 0x0B0000 = 0x00,
 * amp GPIO1.4 = 0, GPIO2.1/2.2 park, then stop I2S (XFER=0, DMACR=0),
 * ASSERT the I2S1 soft reset and Hi-Z the I2S1 pads. Without the close
 * half, the next run inherits stale serializer state and plays tone with
 * noise on top (DFIU 60609d7f). */
void dac_output_off(void)
{
    dac_gpio_out(2, 0, 1);              /* hardware mute ON                */
    dac_gpio_out(2, 14, 1);
    dac_volume_set(0xFF);               /* ref FUN_03055f28(255): volume mute */
    dac_map_write(0x00020000, 0x10);    /* PDN_HP down (stock stop value)  */
    dac_delay_ms(5);
    dac_map_write(0x000B0000, 0x00);
    dac_delay_ms(10);
    dac_gpio_out(1, 4, 0);              /* amp off                         */
    dac_gpio_out(2, 1, 1);
    dac_gpio_out(2, 2, 0);
    I2S_XFER = 0;                       /* FUN_030564d8 close tail         */
    I2S_DMACR = 0;
    cru_rst(R_I2S1_SRST, 1);            /* reset held asserted while idle  */
    dac_gpio_input(1, 0);
    dac_gpio_input(1, 1);
    dac_gpio_input(1, 2);
    dac_gpio_input(1, 3);
}

/* ---- I2C0 mux candidates + scan --------------------------------------------- */

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

int i2c0_scan(void)
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
