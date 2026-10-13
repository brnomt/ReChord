/*
 * usb_msc_rknanoc.c -- polled USB mass storage (Bulk-Only Transport,
 * SCSI transparent) on the RKNanoC DWC2 OTG core at 0x60080000.
 *
 * Clean-room implementation of FACTS extracted from the reference fw1:
 * session 0x03055368 (PHY/GRF bring-up, core init, BOT/SCSI loop),
 * poller 0x03054e9c (bus reset / enum done / suspend / EP0 requests),
 * RX FIFO drain 0x03054a60, IN transfer 0x03054c30, OUT arm 0x03054b68,
 * teardown 0x03054bcc, VBUS 0x03051b1c. No interrupts: GAHBCFG global
 * interrupt stays off and everything is polled from usb_msc_run().
 */
#include <stdint.h>
#include <string.h>

#include "emmc.h"
#include "rknanoc.h"
#include "usb_msc.h"

#define OTG(off)    (*(volatile uint32_t *)(0x60080000u + (off)))
#define DIEPCTL(n)  OTG(0x900u + (n) * 0x20u)
#define DIEPINT(n)  OTG(0x908u + (n) * 0x20u)
#define DIEPTSIZ(n) OTG(0x910u + (n) * 0x20u)
#define DTXFSTS(n)  OTG(0x918u + (n) * 0x20u)
#define DOEPCTL(n)  OTG(0xB00u + (n) * 0x20u)
#define DOEPINT(n)  OTG(0xB08u + (n) * 0x20u)
#define DOEPTSIZ(n) OTG(0xB10u + (n) * 0x20u)
#define EPFIFO(n)   (*(volatile uint32_t *)(0x60081000u + (n) * 0x1000u))

enum {
    GOTGINT = 0x04, GAHBCFG = 0x08, GRSTCTL = 0x10, GINTSTS = 0x14,
    GINTMSK = 0x18, GRXSTSP = 0x20, GRXFSIZ = 0x24, GNPTXFSIZ = 0x28,
    GNPTXSTS = 0x2C, GHWCFG3 = 0x4C, GHWCFG4 = 0x50, DIEPTXF1 = 0x104,
    DCFG = 0x800, DCTL = 0x804, DSTS = 0x808, DIEPMSK = 0x810,
    DOEPMSK = 0x814, DAINT = 0x818, DAINTMSK = 0x81C, PCGCCTL = 0xE00,
};

#define EP_ENA_CNAK 0x84000000u
#define EP_STALL    0x00200000u
#define EP_D0PID    0x10000000u
#define INT_RXFLVL  0x00000010u
#define INT_SUSP    0x00000800u
#define INT_RESET   0x00001000u
#define INT_ENUM    0x00002000u

#define CHUNK_SECTORS 64u            /* 32 KiB staging buffer */

static struct {
    uint32_t setup[2];
    uint8_t *out_buf;
    uint32_t out_len, out_got, mps, max_xfer;
    int setup_pending, out_done, in_halt, out_halt, configured, exit, dedicated;
    uint8_t sense_key[USB_MAX_LUNS], sense_asc[USB_MAX_LUNS], lun;
    unsigned nluns;
    int (*user_exit)(void);
} s;

uint32_t usb_msc_diag;

static const uint8_t k_dev[18] = {
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,
    0x1B, 0x07, 0x03, 0x32, 0x00, 0x02, 0x01, 0x02, 0x03, 0x01,
};
static const uint8_t k_qual[10] = { 10, 6, 0x00, 0x02, 0, 0, 0, 0x40, 1, 0 };
static const uint8_t k_str0[4] = { 4, 3, 0x09, 0x04 };
static const char *const k_str[4] = { 0, "ReChord", "ReChord", "000000000001" };
static uint8_t s_cfg[32] = {
    9, 2, 32, 0, 1, 1, 0, 0x80, 0xC8,          /* config, bus powered 400 mA */
    9, 4, 0, 0, 2, 0x08, 0x06, 0x50, 0,        /* MSC, SCSI, bulk-only */
    7, 5, 0x81, 2, 0x40, 0, 0,                 /* EP1 IN bulk */
    7, 5, 0x01, 2, 0x40, 0, 0,                 /* EP1 OUT bulk */
};
static const uint8_t k_inquiry[36] = {
    0x00, 0x80, 0x02, 0x02, 31, 0, 0, 0,
    'R', 'e', 'C', 'h', 'o', 'r', 'd', ' ',
    'E', 'C', 'H', 'O', ' ', 'M', 'I', 'N', 'I', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    '0', '.', '1', ' ',
};
static const uint8_t k_mode6[4] = { 3, 0, 0, 0 };
static const uint8_t k_mode10[8] = { 0, 6, 0, 0, 0, 0, 0, 0 };

int usb_cable(void)
{
    return (int)((RK_GRF[0xE8 / 4] >> 1) & 1u);
}

static void wait_clear(unsigned off, uint32_t mask)
{
    for (int i = 10001; i && (OTG(off) & mask); i--)
        rk_delay_us(1);
}

/* Pops cnt bytes from the RX FIFO, keeping the first keep in dst. */
static void fifo_read(uint8_t *dst, uint32_t keep, uint32_t cnt)
{
    uint32_t i = 0;

    if (dst && keep == cnt && !((uintptr_t)dst & 3u))
        for (; i + 4u <= cnt; i += 4u)
            *(uint32_t *)(dst + i) = EPFIFO(0);
    for (; i < cnt; i += 4u) {
        uint32_t v = EPFIFO(0);
        for (uint32_t b = 0; b < 4u && i + b < keep; b++)
            dst[i + b] = (uint8_t)(v >> (8u * b));
    }
}

static void fifo_write(unsigned ep, const uint8_t *src, uint32_t n)
{
    uint32_t i = 0;

    if (!((uintptr_t)src & 3u))
        for (; i + 4u <= n; i += 4u)
            EPFIFO(ep) = *(const uint32_t *)(src + i);
    for (; i < n; i += 4u) {
        uint32_t v = 0;
        for (uint32_t b = 0; b < 4u && i + b < n; b++)
            v |= (uint32_t)src[i + b] << (8u * b);
        EPFIFO(ep) = v;
    }
}

static void arm_ep0_out(void)
{
    DOEPTSIZ(0) = 0x60080040u;       /* 3 SETUP packets, 1 packet, 64 bytes */
    DOEPCTL(0) |= EP_ENA_CNAK;
}

static void rx_drain(void)
{
    while (OTG(GINTSTS) & INT_RXFLVL) {
        uint32_t st = OTG(GRXSTSP), sts = (st >> 17) & 15u, cnt = (st >> 4) & 0x7FFu;

        if (sts == 6u) {                             /* SETUP packet */
            fifo_read((uint8_t *)s.setup, cnt < 8u ? cnt : 8u, cnt);
        } else if (sts == 2u) {                      /* OUT data */
            if ((st & 15u) == 1u && s.out_buf) {
                uint32_t take = s.out_len - s.out_got;

                if (take > cnt)
                    take = cnt;
                fifo_read(s.out_buf + s.out_got, take, cnt);
                s.out_got += take;
            } else {
                fifo_read(0, 0, cnt);
            }
        } else if (sts == 4u) {                      /* SETUP stage done */
            s.setup_pending = 1;
        }
    }
    if (DOEPINT(1) & 1u) {
        DOEPINT(1) = 1u;
        s.out_done = 1;
    }
    if (DOEPINT(0) & 9u) {
        if (DOEPINT(0) & 8u)
            s.setup_pending = 1;
        DOEPINT(0) = 9u;
        arm_ep0_out();
    }
}

/* Synchronous IN transfer (also the EP0 data/status stage). 0 = done. */
static int ep_in(unsigned ep, const void *buf, uint32_t len)
{
    const uint8_t *p = buf;
    uint32_t mps = ep ? s.mps : 64u, tries = 2000000u;

    DIEPINT(ep) = 0xFFu;
    DIEPTSIZ(ep) = len | (len ? (len + mps - 1u) / mps : 1u) << 19;
    DIEPCTL(ep) |= EP_ENA_CNAK;
    for (;;) {
        if (len) {
            uint32_t n = len < mps ? len : mps, space;

            if (s.dedicated)
                space = DTXFSTS(ep) & 0xFFFFu;
            else
                space = (OTG(GNPTXSTS) & 0xFF0000u) ? OTG(GNPTXSTS) & 0xFFFFu : 0u;
            if (space >= (n + 3u) >> 2) {
                fifo_write(ep, p, n);
                p += n;
                len -= n;
                continue;
            }
        } else if (DIEPINT(ep) & 1u) {
            DIEPINT(ep) = 1u;
            return 0;
        }
        rx_drain();
        if ((OTG(GINTSTS) & INT_RESET) || !usb_cable() || --tries == 0u)
            return -1;
        rk_delay_us(1);
    }
}

static void flush_fifos(void)
{
    OTG(GRSTCTL) = 0x420u;           /* flush all TX FIFOs */
    wait_clear(GRSTCTL, 0x20u);
    OTG(GRSTCTL) = 0x10u;            /* flush RX FIFO */
    wait_clear(GRSTCTL, 0x10u);
}

static void bus_reset(void)
{
    OTG(DCFG) &= ~0x7F0u;            /* address 0 */
    DIEPINT(0) = 0xFFu;
    DOEPINT(0) = 0xFFu;
    for (unsigned n = 1; n < 4u; n++) {
        DIEPINT(n) = 0xFFu;
        DOEPINT(n) = 0xFFu;
        DIEPCTL(n) = (DIEPCTL(n) & 0x80000000u) ? 0x48000000u : 0u;
        DOEPCTL(n) = (DOEPCTL(n) & 0x80000000u) ? 0x48000000u : 0u;
    }
    flush_fifos();
    OTG(DAINTMSK) = 0x10001u;
    arm_ep0_out();
    s.configured = 0;
    s.out_buf = 0;
    s.in_halt = s.out_halt = 0;
}

static void handle_setup(void)
{
    static uint32_t ep0w[16];
    const uint8_t *r = (const uint8_t *)s.setup;
    uint8_t *b = (uint8_t *)ep0w, type = r[0], req = r[1];
    uint32_t val = r[2] | r[3] << 8, idx = r[4] | r[5] << 8, wlen = r[6] | r[7] << 8;
    const void *data = b;
    uint32_t n = 0;
    int in = (idx & 0x80u) != 0;

    if ((type & 0x60u) == 0x20u) {                   /* class: Bulk-Only */
        if (req == 0xFE && (type & 0x80u)) {         /* GET MAX LUN */
            b[0] = (uint8_t)(s.nluns - 1u);
            n = 1;
            goto send;
        }
        if (req == 0xFF && !(type & 0x80u) && wlen == 0u)   /* BOT reset */
            goto status;
        goto stall;
    }
    if (type & 0x60u)
        goto stall;
    switch (req) {
    case 0:                                          /* GET_STATUS */
        b[0] = b[1] = 0;
        if ((type & 0x1Fu) == 2u && (idx & 0x7Fu) == 1u)
            b[0] = ((in ? DIEPCTL(1) : DOEPCTL(1)) & EP_STALL) ? 1 : 0;
        n = 2;
        goto send;
    case 1:                                          /* CLEAR_FEATURE */
    case 3:                                          /* SET_FEATURE */
        if ((type & 0x1Fu) == 2u && val == 0u && (idx & 0x7Fu) == 1u) {
            volatile uint32_t *ctl = in ? &DIEPCTL(1) : &DOEPCTL(1);
            int *halt = in ? &s.in_halt : &s.out_halt;

            if (req == 1) {
                *ctl = (*ctl & ~(EP_STALL | EP_D0PID)) | EP_D0PID;
                *halt = 0;
            } else {
                *ctl |= EP_STALL;
                *halt = 1;
            }
        }
        goto status;
    case 5:                                          /* SET_ADDRESS */
        OTG(DCFG) = (OTG(DCFG) & ~0x7F0u) | (val & 0x7Fu) << 4;
        goto status;
    case 6:                                          /* GET_DESCRIPTOR */
        switch (val >> 8) {
        case 1:
            data = k_dev;
            n = sizeof k_dev;
            goto send;
        case 2:
        case 7: {                                    /* config / other speed */
            uint32_t mps = (val >> 8) == 2u ? s.mps : (s.mps == 64u ? 512u : 64u);

            s_cfg[1] = (uint8_t)(val >> 8);
            s_cfg[22] = s_cfg[29] = (uint8_t)mps;
            s_cfg[23] = s_cfg[30] = (uint8_t)(mps >> 8);
            ep_in(0, s_cfg, wlen < sizeof s_cfg ? wlen : sizeof s_cfg);
            s_cfg[1] = 2;
            return;
        }
        case 3:
            if ((val & 0xFFu) == 0u) {
                data = k_str0;
                n = sizeof k_str0;
            } else if ((val & 0xFFu) < 4u) {
                const char *str = k_str[val & 0xFFu];

                while (str[n] && n < 30u) {
                    b[2 + 2 * n] = (uint8_t)str[n];
                    b[3 + 2 * n] = 0;
                    n++;
                }
                n = 2 * n + 2;
                b[0] = (uint8_t)n;
                b[1] = 3;
            } else {
                goto stall;
            }
            goto send;
        case 6:
            data = k_qual;
            n = sizeof k_qual;
            goto send;
        default:
            goto stall;
        }
    case 8:                                          /* GET_CONFIGURATION */
        b[0] = (uint8_t)s.configured;
        n = 1;
        goto send;
    case 9:                                          /* SET_CONFIGURATION */
        if (val == 1u) {
            OTG(DAINTMSK) |= 0x20002u;
            DIEPCTL(1) = s.mps | (uint32_t)s.dedicated << 22 | 0x18088000u;
            DOEPCTL(1) = s.mps | 0x18088000u;        /* D0PID, SNAK, bulk, active */
            OTG(DCTL) |= 0x100u;                     /* clear global IN NAK */
            s.configured = 1;
            s.out_buf = 0;
        } else {
            s.configured = 0;
        }
        goto status;
    case 10:                                         /* GET_INTERFACE */
        b[0] = 0;
        n = 1;
        goto send;
    case 11:                                         /* SET_INTERFACE */
        if (val == 0u)
            goto status;
        goto stall;
    default:
        goto stall;
    }
send:
    ep_in(0, data, n < wlen ? n : wlen);
    return;
status:
    ep_in(0, 0, 0);
    return;
stall:
    DIEPCTL(0) |= EP_STALL;
    DOEPCTL(0) |= EP_STALL;
}

static void poll(void)
{
    uint32_t gi = OTG(GINTSTS);

    rx_drain();
    if (gi & INT_RESET) {
        OTG(GINTSTS) = INT_RESET;
        bus_reset();
    }
    if (gi & INT_ENUM) {
        uint32_t h3 = OTG(GHWCFG3), pk, xf, m;

        OTG(GINTSTS) = INT_ENUM;
        s.mps = (OTG(DSTS) & 6u) == 0u ? 512u : 64u;
        pk = (1u << (((h3 & 0x7Fu) >> 4) + 4u)) - 1u;
        xf = (1u << ((h3 & 15u) + 11u)) - 1u;
        m = s.mps * pk;
        if (m > xf)
            m = xf;
        s.max_xfer = s.mps * (m / s.mps);
        DIEPCTL(0) &= ~3u;                           /* EP0 MPS 64 */
        arm_ep0_out();
    }
    usb_msc_diag |= (gi >> 10 & 15u) << 16;
    if (gi & INT_SUSP) {
        OTG(GINTSTS) = INT_SUSP;
        if (s.configured)
            s.exit = USB_EXIT_SUSPEND;
    }
    if (s.setup_pending) {
        s.setup_pending = 0;
        handle_setup();
    }
    if (!usb_cable())
        s.exit = USB_EXIT_UNPLUG;
    if (!s.exit && s.user_exit && s.user_exit())
        s.exit = USB_EXIT_USER;
}

/* OUT transfer into buf; returns bytes received or -1. */
static int ep_out(void *buf, uint32_t len)
{
    uint32_t pk = len ? (len + s.mps - 1u) / s.mps : 1u;

    s.out_buf = buf;
    s.out_len = len;
    s.out_got = 0;
    s.out_done = 0;
    DOEPINT(1) = 0xFFu;
    DOEPTSIZ(1) = pk * s.mps | pk << 19;
    DOEPCTL(1) |= EP_ENA_CNAK;
    while (!s.out_done) {
        if (s.out_halt || s.exit || !s.configured)
            return -1;
        poll();
    }
    s.out_buf = 0;
    return (int)s.out_got;
}

static void csw(uint32_t tag, uint32_t residue, uint8_t status)
{
    uint32_t w[4] = { 0x53425355u, tag, residue, status };  /* "USBS" */

    while (s.in_halt && !s.exit)
        poll();
    ep_in(1, w, 13);
}

/* Command failed: latch sense, stall the data endpoint, CSW status 1. */
static void fail(uint32_t tag, uint32_t dtl, uint8_t flags, uint8_t key, uint8_t asc)
{
    s.sense_key[s.lun] = key;
    s.sense_asc[s.lun] = asc;
    if (dtl) {
        if (flags & 0x80u) {
            DIEPCTL(1) |= EP_STALL;
            s.in_halt = 1;
        } else {
            DOEPCTL(1) |= EP_STALL;
            s.out_halt = 1;
        }
    }
    csw(tag, dtl, 1);
}

static void put_be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static void scsi(const uint32_t *cbw, const usb_lun_t *u, uint32_t *rd, uint32_t *wr)
{
    static uint32_t buf[CHUNK_SECTORS * 128u];
    static uint32_t resp[9];
    const uint8_t *cb = (const uint8_t *)cbw + 15;
    uint8_t *r = (uint8_t *)resp, flags = ((const uint8_t *)cbw)[12];
    uint32_t tag = cbw[1], dtl = cbw[2], n, count = u->count;
    const void *data = r;

    usb_msc_diag = (usb_msc_diag & ~0xFFu) | cb[0];
    if (!count && cb[0] != 0x03 && cb[0] != 0x12 && cb[0] != 0x1B && cb[0] != 0x1E) {
        fail(tag, dtl, flags, 2, 0x3A);              /* medium not present */
        return;
    }
    switch (cb[0]) {
    case 0x00:                                       /* TEST UNIT READY */
    case 0x1E:                                       /* PREVENT ALLOW REMOVAL */
    case 0x2F:                                       /* VERIFY */
    case 0x35:                                       /* SYNCHRONIZE CACHE */
        csw(tag, 0, 0);
        return;
    case 0x03:                                       /* REQUEST SENSE */
        memset(r, 0, 18);
        r[0] = 0x70;
        r[2] = s.sense_key[s.lun];
        r[7] = 10;
        r[12] = s.sense_asc[s.lun];
        s.sense_key[s.lun] = s.sense_asc[s.lun] = 0;
        n = 18;
        break;
    case 0x12:                                       /* INQUIRY */
        if (cb[1] & 1u) {
            fail(tag, dtl, flags, 5, 0x24);
            return;
        }
        data = k_inquiry;
        n = sizeof k_inquiry;
        break;
    case 0x1A:                                       /* MODE SENSE(6) */
        memcpy(r, k_mode6, sizeof k_mode6);
        r[2] = u->write ? 0 : 0x80;                  /* WP */
        n = sizeof k_mode6;
        break;
    case 0x5A:                                       /* MODE SENSE(10) */
        memcpy(r, k_mode10, sizeof k_mode10);
        r[3] = u->write ? 0 : 0x80;
        n = sizeof k_mode10;
        break;
    case 0x1B:                                       /* START STOP UNIT */
        usb_msc_diag = (usb_msc_diag & ~0xFF00u) | (uint32_t)cb[4] << 8;
        csw(tag, 0, 0);
        if (cb[4] & 2u)
            s.exit = USB_EXIT_EJECT;
        return;
    case 0x23:                                       /* READ FORMAT CAPACITIES */
        memset(r, 0, 12);
        r[3] = 8;
        put_be32(r + 4, count);
        r[8] = 2;
        r[10] = 2;
        n = 12;
        break;
    case 0x25:                                       /* READ CAPACITY(10) */
        put_be32(r, count - 1u);
        put_be32(r + 4, 512);
        n = 8;
        break;
    case 0x28:                                       /* READ(10) */
    case 0x2A: {                                     /* WRITE(10) */
        int write = cb[0] == 0x2A;
        uint32_t lba = (uint32_t)cb[2] << 24 | cb[3] << 16 | cb[4] << 8 | cb[5];
        uint32_t cnt = (uint32_t)cb[7] << 8 | cb[8];

        if (lba + cnt < lba || lba + cnt > count || dtl != cnt * 512u ||
            ((flags & 0x80u) != 0) == write) {
            fail(tag, dtl, flags, 5, 0x21);
            return;
        }
        if (write && !u->write) {
            fail(tag, dtl, flags, 7, 0x27);          /* write protected */
            return;
        }
        while (cnt) {
            uint32_t k = cnt < CHUNK_SECTORS ? cnt : CHUNK_SECTORS, bytes = k << 9;
            uint8_t *p = (uint8_t *)buf;

            if (!write && u->read(u->base + lba, k, buf)) {
                fail(tag, dtl, flags, 3, 0x11);
                return;
            }
            for (uint32_t off = 0; off < bytes; ) {
                uint32_t piece = bytes - off < s.max_xfer ? bytes - off : s.max_xfer;

                if (write ? ep_out(p + off, piece) != (int)piece : ep_in(1, p + off, piece) != 0) {
                    if (!write)
                        s.exit = USB_EXIT_IO;
                    return;
                }
                off += piece;
            }
            if (write && u->write(u->base + lba, k, buf)) {
                fail(tag, dtl, flags, 3, 0x03);
                return;
            }
            *(write ? wr : rd) += k;
            lba += k;
            cnt -= k;
        }
        csw(tag, 0, 0);
        return;
    }
    default:
        fail(tag, dtl, flags, 5, 0x20);              /* invalid opcode */
        return;
    }
    if (n > dtl)
        n = dtl;
    if (n)
        ep_in(1, data, n);
    csw(tag, dtl - n, 0);
}

static void gpio_out(unsigned port, unsigned pin, int on)
{
    static const uint32_t base[3] = { 0x40160000u, 0x40170000u, 0x50030000u };
    volatile uint32_t *g = (volatile uint32_t *)base[port];

    RK_GRF[port * 8u + (pin >> 3)] = 0x30000u << ((pin & 7u) * 2u);   /* GPIO */
    if (on)
        g[0] |= 1u << pin;
    else
        g[0] &= ~(1u << pin);
    g[1] |= 1u << pin;
}

static void hw_on(void)
{
    cru_gate(7, 1);
    cru_gate(0x6A, 1);
    cru_reset(1, 0);
    cru_reset(2, 0);
    cru_reset(0, 0);
    rk_delay_us(5);
    RK_GRF[0xB8 / 4] = 0x00010000u;                  /* USB PHY bring-up */
    RK_GRF[0xB4 / 4] = 0x000E0002u;
    RK_GRF[0x98 / 4] = 0x00040004u;
    RK_GRF[0xB4 / 4] = 0x00010000u;
    rk_delay_us(10200);
    RK_GRF[0x88 / 4] = 0x0C000C00u;
    RK_GRF[0x84 / 4] = 0x1FE00003u;
    RK_GRF[0x8C / 4] = 0x001C0014u;
    RK_GRF[0x80 / 4] = 0x00070004u;
    gpio_out(1, 4, 0);
    gpio_out(2, 0, 1);
    gpio_out(2, 2, 0);
}

static void hw_off(void)
{
    OTG(DCTL) |= 2u;                                 /* soft disconnect */
    RK_GRF[0xB4 / 4] = 0x00010001u;
    RK_GRF[0xB4 / 4] = 0x000E0004u;
    RK_GRF[0xB8 / 4] = 0x00010001u;
    RK_GRF[0x98 / 4] = 0x00040000u;
    cru_reset(1, 1);
    cru_reset(2, 1);
    cru_reset(0, 1);
    cru_gate(7, 0);
    cru_gate(0x6A, 0);
}

static void core_init(void)
{
    uint32_t depth;

    OTG(DCTL) |= 2u;                                 /* soft disconnect */
    for (int i = 10000; i && !(OTG(GRSTCTL) & 0x80000000u); i--)   /* AHB idle */
        rk_delay_us(1);
    OTG(GRSTCTL) |= 1u;                              /* core soft reset */
    wait_clear(GRSTCTL, 1u);
    OTG(PCGCCTL) = 0xFu;
    rk_delay_us(10000);
    OTG(PCGCCTL) = 0;
    OTG(GAHBCFG) |= 0x80u;
    /* ponytail: forced full speed (64-byte bulk, ~1 MB/s): the AHB clock
     * is the uncalibrated boot clock, below what high speed needs. Drop
     * the |1 once the PLL is set up (the reference runs high speed). */
    OTG(DCFG) = (OTG(DCFG) & ~0x7F3u) | 1u;

    s.dedicated = (int)((OTG(GHWCFG4) >> 25) & 1u);
    depth = OTG(GHWCFG3) >> 16;
    if (!s.dedicated && depth >= 0x220u) {
        OTG(GRXFSIZ) = 0x120u;
        OTG(GNPTXFSIZ) = 0x01000120u;
    } else if (s.dedicated && depth >= 0x240u) {
        OTG(GRXFSIZ) = 0x120u;
        OTG(GNPTXFSIZ) = 0x00200120u;
        OTG(DIEPTXF1) = 0x01000140u;
    }
    flush_fifos();
    OTG(GRSTCTL) = 8u;
    rk_delay_us(1);
    DIEPCTL(0) = 0x48000000u;
    DOEPCTL(0) = 0x48000000u;
    DIEPTSIZ(0) = 0;
    DOEPTSIZ(0) = 0;
    DIEPINT(0) = 0xFFu;
    DOEPINT(0) = 0xFFu;
    OTG(DIEPMSK) = 0x2Fu;
    OTG(DOEPMSK) = 0xFu;
    OTG(DAINT) = 0xFFFFFFFFu;
    OTG(DAINTMSK) = 0x10001u;
    OTG(GINTSTS) = 0xFFFFFFFFu;
    OTG(GOTGINT) = 0xFFFFFFFFu;
    OTG(GINTMSK) = 0xC00C3C10u;
    OTG(DCTL) &= ~2u;                                /* connect */
    bus_reset();
}

/* Shared DWC2 bring-up (PHY + core + FIFOs + EP0), reused by the CDC-ACM
 * console driver so both devices use the exact init that enumerates. */
void usb_dwc2_init(void)
{
    hw_on();
    core_init();
}

int usb_msc_run(const usb_lun_t *luns, unsigned nluns, int (*user_exit)(void),
                uint32_t *rd_sectors, uint32_t *wr_sectors)
{
    static uint32_t cbw[8];
    int ret;

    memset(&s, 0, sizeof s);
    usb_msc_diag = 0;
    s.mps = s.max_xfer = 64u;
    s.user_exit = user_exit;
    s.nluns = nluns - 1u < USB_MAX_LUNS ? nluns : 1u;
    for (unsigned i = 0; i < USB_MAX_LUNS; i++) {
        s.sense_key[i] = 6;                          /* UNIT ATTENTION, */
        s.sense_asc[i] = 0x28;                       /* medium changed */
    }
    *rd_sectors = *wr_sectors = 0;

    hw_on();
    core_init();
    for (int i = 0; !(OTG(GINTSTS) & INT_RESET); i++) {
        if (!usb_cable()) {
            ret = USB_EXIT_UNPLUG;
            goto out;
        }
        if (i == 3000) {
            ret = USB_EXIT_NO_HOST;
            goto out;
        }
        if (user_exit && user_exit()) {
            ret = USB_EXIT_USER;
            goto out;
        }
        rk_delay_us(1000);
    }
    while (!s.exit) {
        const uint8_t *b = (const uint8_t *)cbw;
        int n;

        if (!s.configured || s.out_halt) {
            poll();
            continue;
        }
        n = ep_out(cbw, 31);
        if (n < 0)
            continue;
        if (n != 31 || cbw[0] != 0x43425355u || b[13] >= s.nluns ||   /* "USBC" */
            (b[12] & 0x7Fu) || b[14] - 1u > 15u) {
            DIEPCTL(1) |= EP_STALL;                  /* invalid CBW: wait for */
            DOEPCTL(1) |= EP_STALL;                  /* reset recovery */
            s.in_halt = s.out_halt = 1;
            continue;
        }
        s.lun = b[13];
        scsi(cbw, &luns[s.lun], rd_sectors, wr_sectors);
    }
    ret = s.exit;
out:
    hw_off();
    return ret;
}
