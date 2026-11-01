/*
 * usb_dac_rknanoc.c -- polled USB Audio Class 1.0 playback device on the
 * RKNanoC DWC2 OTG core at 0x60080000.
 *
 * The MSC session's bring-up/enumeration facts (PHY/GRF, core init, FIFO
 * allocation, EP0 poller) are reused here; the differences are the UAC1
 * descriptor set, the class-specific control requests and EP1 as an
 * isochronous OUT endpoint fed straight into the I2S PCM ring.
 *
 * Full speed only (the AHB clock at boot cannot run the HS PHY): 48 kHz,
 * 16-bit stereo = 192 bytes per 1 ms frame, well inside a 196-byte max
 * packet. No interrupts; the main loop polls GRXSTSP.
 */
#include <stdint.h>
#include <string.h>

#include "rknanoc.h"
#include "bt_hw.h"                /* wired/BT audio route               */
#include "usb_dac.h"
#include "usb_msc.h"              /* USB_EXIT_*                          */
#include "../services/scratch.h"
#include "../services/usb_dac_sink.h"
#include "../audio/audio_output.h"

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
#define EP_EPDIS    0x40000000u
#define EP_ISO      0x00040000u   /* EPTYPE = 01 (isochronous)            */
#define INT_RXFLVL  0x00000010u
#define INT_SUSP    0x00000800u
#define INT_RESET   0x00001000u
#define INT_ENUM    0x00002000u

#define USB_DAC_RATE 48000u
#define USB_DAC_MPS  192u         /* 48 frames x 2 ch x 16-bit            */

/* Shared GPIO helper (firmware/drivers/rknano_i2s.c). Declared here instead
 * of including rknano_i2s.h, whose cru_gate() prototype conflicts with the
 * static inline in rknanoc.h used by this driver. */
void dac_gpio_out(int port, int pin, int level);

static struct {
    uint32_t setup[2];
    uint8_t ctrl_out[64];
    uint32_t ctrl_got;
    uint8_t iso_pkt[200] __attribute__((aligned(4)));
    uint32_t mps, max_xfer;
    int setup_pending, configured, exit, alt, streaming, dedicated;
    int16_t volume;
    uint8_t mute;
    int (*user_exit)(void);
    ringbuf_t ring;
    usb_dac_sink_t sink;
} s;

/* AudioControl (interface 0) + AudioStreaming (interface 1, alt 1 =
 * 48 kHz/16-bit stereo, EP1 isochronous OUT). Two interfaces, 110 bytes. */
static const uint8_t k_cfg_dac[110] = {
    9, 2, 110, 0, 2, 1, 0, 0x80, 50,                  /* configuration */
    9, 4, 0, 0, 0, 1, 1, 0, 0,                        /* AC interface */
    9, 0x24, 1, 0x00, 0x01, 40, 0, 1, 1,              /* AC header */
    12, 0x24, 2, 1, 0x01, 0x01, 0, 2, 0x03, 0x00, 0, 0,/* input terminal */
    9, 0x24, 3, 3, 0x02, 0x03, 0, 2, 0,                /* output terminal */
    10, 0x24, 6, 2, 1, 1, 0x03, 0x00, 0x00, 0,        /* feature unit */
    9, 4, 1, 0, 0, 1, 2, 0, 0,                        /* AS alt 0 */
    9, 4, 1, 1, 1, 1, 2, 0, 0,                        /* AS alt 1 */
    7, 0x24, 1, 1, 1, 0x01, 0x00,                     /* AS general */
    11, 0x24, 2, 1, 2, 2, 16, 1, 0x80, 0xBB, 0x00,    /* type I 48 kHz */
    9, 5, 0x01, 0x09, 0xC0, 0x00, 1, 0, 0,            /* EP1 ISO OUT 192 B */
    7, 0x25, 1, 1, 0, 0, 0,                           /* CS endpoint */
};

_Static_assert(sizeof k_cfg_dac == 110, "UAC configuration layout");

static const uint8_t k_dev[18] = {
    0x12, 0x01, 0x10, 0x01, 0x00, 0x00, 0x00, 0x40,
    0x1B, 0x07, 0x03, 0x32, 0x00, 0x02, 0x01, 0x02, 0x03, 0x01,
};
static const uint8_t k_qual[10] = { 10, 6, 0x00, 0x02, 0, 0, 0, 0x40, 1, 0 };
static const uint8_t k_str0[4] = { 4, 3, 0x09, 0x04 };
static const char *const k_str[4] = { 0, "ReChord", "ReChord USB DAC",
                                      "000000000001" };

static void wait_clear(unsigned off, uint32_t mask)
{
    for (int i = 10001; i && (OTG(off) & mask); i--)
        rk_delay_us(1);
}

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
    DOEPTSIZ(0) = 0x60080040u;
    DOEPCTL(0) |= EP_ENA_CNAK;
}

static void arm_iso_out(void)
{
    DOEPINT(1) = 0xFFu;
    DOEPTSIZ(1) = USB_DAC_MPS | (1u << 19);
    DOEPCTL(1) = USB_DAC_MPS | EP_ISO | EP_ENA_CNAK;
    OTG(DAINTMSK) |= 0x20000u;
}

static void disarm_iso_out(void)
{
    if (DOEPCTL(1) & 0x80000000u) {
        DOEPCTL(1) |= EP_EPDIS;
        for (int i = 1000; i && (DOEPCTL(1) & 0x80000000u); i--)
            rk_delay_us(1);
        DOEPCTL(1) &= ~EP_EPDIS;
    }
    OTG(DAINTMSK) &= ~0x20000u;
}

static void rx_drain(void);

/* SET_CUR carries its value in the control OUT data stage; the setup packet
 * is handled as soon as it lands, so give the data a few microseconds to
 * arrive in the RX FIFO before reading it. */
static void wait_ctrl(uint32_t bytes)
{
    for (int i = 10000; i && s.ctrl_got < bytes; i--) {
        rx_drain();
        rk_delay_us(1);
    }
}

static void uac_apply_volume(void)
{
    int db = s.mute ? -127 : (int)s.volume / 256;

    if (db < -127)
        db = -127;
    if (db > 0)
        db = 0;
    audio_output_set_volume_db(db);
}

static void rx_drain(void)
{
    while (OTG(GINTSTS) & INT_RXFLVL) {
        uint32_t st = OTG(GRXSTSP);
        uint32_t sts = (st >> 17) & 15u, ep = st & 15u, cnt = (st >> 4) & 0x7FFu;

        if (sts == 6u) {                             /* SETUP packet */
            s.ctrl_got = 0;
            fifo_read((uint8_t *)s.setup, cnt < 8u ? cnt : 8u, cnt);
        } else if (sts == 2u || sts == 3u) {         /* OUT data/complete */
            if (ep == 0u) {
                uint32_t room = sizeof s.ctrl_out - s.ctrl_got;
                uint32_t take = cnt < room ? cnt : room;

                fifo_read(s.ctrl_out + s.ctrl_got, take, cnt);
                s.ctrl_got += take;
            } else if (ep == 1u) {
                if (sts == 2u) {              /* data packet, not the
                                               * non-ISO "complete" status */
                    uint32_t keep = cnt < sizeof s.iso_pkt ? cnt : sizeof s.iso_pkt;

                    fifo_read(s.iso_pkt, keep, cnt);
                    if (s.streaming && keep)
                        (void)usb_dac_sink_write(&s.sink, s.iso_pkt, keep);
                    if (s.streaming)
                        arm_iso_out();
                } else {
                    fifo_read(0, 0, cnt);
                }
            } else {
                fifo_read(0, 0, cnt);
            }
        } else if (sts == 4u) {
            s.setup_pending = 1;
        }
    }
    if (DOEPINT(1) & 1u)
        DOEPINT(1) = 1u;
    if (DOEPINT(0) & 9u) {
        if (DOEPINT(0) & 8u)
            s.setup_pending = 1;
        DOEPINT(0) = 9u;
        arm_ep0_out();
    }
}

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
    OTG(GRSTCTL) = 0x420u;
    wait_clear(GRSTCTL, 0x20u);
    OTG(GRSTCTL) = 0x10u;
    wait_clear(GRSTCTL, 0x10u);
}

static void bus_reset(void)
{
    OTG(DCFG) &= ~0x7F0u;
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
    s.alt = 0;
    s.streaming = 0;
}

static void handle_setup(void)
{
    static uint32_t ep0w[16];
    const uint8_t *r = (const uint8_t *)s.setup;
    uint8_t *b = (uint8_t *)ep0w;
    uint8_t type = r[0], req = r[1];
    uint16_t val = (uint16_t)(r[2] | r[3] << 8);
    uint16_t idx = (uint16_t)(r[4] | r[5] << 8);
    uint16_t wlen = (uint16_t)(r[6] | r[7] << 8);
    const void *data = b;
    uint32_t n = 0;

    if ((type & 0x60u) == 0x20u) {                   /* UAC class requests */
        uint8_t rec = type & 0x1Fu, sel = (uint8_t)(val >> 8);

        if (rec == 2u && (idx & 0x7Fu) == 1u && sel == 1u) {
            if (req == 0x81u || req == 0x82u || req == 0x83u) {
                b[0] = 0x80; b[1] = 0xBB; b[2] = 0x00;   /* 48000 Hz */
                n = 3;
                goto send;
            }
            if (req == 0x84u) {
                b[0] = 1; b[1] = b[2] = 0;               /* 1 Hz */
                n = 3;
                goto send;
            }
            if (req == 0x01u) {                          /* SET_CUR */
                wait_ctrl(3);
                goto status;
            }
        }
        if (rec == 1u && idx == 0x0200u && (val & 0xFFu) == 0u) {
            if (sel == 1u) {                             /* mute */
                if (req == 0x81u) {
                    b[0] = s.mute;
                    n = 1;
                    goto send;
                }
                if (req == 0x82u || req == 0x83u) {
                    b[0] = 0;
                    n = 1;
                    goto send;
                }
                if (req == 0x84u) {
                    b[0] = 1;
                    n = 1;
                    goto send;
                }
                if (req == 0x01u) {
                    wait_ctrl(1);
                    if (s.ctrl_got)
                        s.mute = s.ctrl_out[0] & 1u;
                    uac_apply_volume();
                    goto status;
                }
            }
            if (sel == 2u) {                             /* volume 1/256 dB */
                if (req == 0x81u) {
                    b[0] = (uint8_t)s.volume;
                    b[1] = (uint8_t)(s.volume >> 8);
                    n = 2;
                    goto send;
                }
                if (req == 0x82u) {                      /* -16384 = 0xC000 */
                    b[0] = 0x00; b[1] = 0xC0;
                    n = 2;
                    goto send;
                }
                if (req == 0x83u) {
                    b[0] = b[1] = 0;
                    n = 2;
                    goto send;
                }
                if (req == 0x84u) {
                    b[0] = 0x80; b[1] = 0x00;            /* 0.5 dB */
                    n = 2;
                    goto send;
                }
                if (req == 0x01u) {
                    wait_ctrl(2);
                    if (s.ctrl_got >= 2u)
                        s.volume = (int16_t)(s.ctrl_out[0] | s.ctrl_out[1] << 8);
                    uac_apply_volume();
                    goto status;
                }
            }
        }
        goto stall;
    }
    if (type & 0x60u)
        goto stall;
    switch (req) {
    case 0:                                          /* GET_STATUS */
        b[0] = b[1] = 0;
        n = 2;
        goto send;
    case 1:                                          /* CLEAR_FEATURE */
    case 3:                                          /* SET_FEATURE */
        goto status;                                 /* no halts on EP0/EP1 ISO */
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
        case 7:
            ep_in(0, k_cfg_dac, wlen < sizeof k_cfg_dac ? wlen : sizeof k_cfg_dac);
            return;
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
        s.configured = val == 1u;
        s.alt = 0;
        s.streaming = 0;
        if (s.configured)
            OTG(DAINTMSK) |= 0x20000u;
        else
            disarm_iso_out();
        goto status;
    case 10:                                         /* GET_INTERFACE */
        b[0] = (uint8_t)((idx & 0xFFu) == 1u ? s.alt : 0);
        n = 1;
        goto send;
    case 11:                                         /* SET_INTERFACE */
        if ((idx & 0xFFu) == 1u) {
            if (val == 1u) {
                s.alt = 1;
                s.streaming = 1;
                arm_iso_out();
            } else if (val == 0u) {
                s.alt = 0;
                s.streaming = 0;
                disarm_iso_out();
            } else {
                goto stall;
            }
            goto status;
        }
        if ((idx & 0xFFu) == 0u && val == 0u)
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
        DIEPCTL(0) &= ~3u;
        arm_ep0_out();
    }
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

static void gpio_out(unsigned port, unsigned pin, int on)
{
    static const uint32_t base[3] = { 0x40160000u, 0x40170000u, 0x50030000u };
    volatile uint32_t *g = (volatile uint32_t *)base[port];

    RK_GRF[port * 8u + (pin >> 3)] = 0x30000u << ((pin & 7u) * 2u);
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
    RK_GRF[0xB8 / 4] = 0x00010000u;
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
    OTG(DCTL) |= 2u;
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

    OTG(DCTL) |= 2u;
    for (int i = 10000; i && !(OTG(GRSTCTL) & 0x80000000u); i--)
        rk_delay_us(1);
    OTG(GRSTCTL) |= 1u;
    wait_clear(GRSTCTL, 1u);
    OTG(PCGCCTL) = 0xFu;
    rk_delay_us(10000);
    OTG(PCGCCTL) = 0;
    OTG(GAHBCFG) |= 0x80u;
    OTG(DCFG) = (OTG(DCFG) & ~0x7F3u) | 1u;      /* full speed */

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
    OTG(DCTL) |= 2u;
    rk_delay_us(150000);
    OTG(DCTL) &= ~2u;
    bus_reset();
}

int usb_dac_run(int (*user_exit)(void), usb_dac_diag_t *diag)
{
    audio_format_t fmt = { USB_DAC_RATE, 16, 2 };
    void *mem;
    int ret;

    if (diag)
        memset(diag, 0, sizeof *diag);
    mem = scratch_claim(SCRATCH_OWNER_USB);
    if (mem == 0)
        return USB_EXIT_BUSY;

    memset(&s, 0, sizeof s);
    s.mps = s.max_xfer = 64;
    s.volume = 0;
    s.user_exit = user_exit;

    if (rb_init(&s.ring, mem, SCRATCH_BYTES) != 0) {
        scratch_release(SCRATCH_OWNER_USB);
        return USB_EXIT_BUSY;
    }
    usb_dac_sink_init(&s.sink, &s.ring);
    usb_dac_sink_start(&s.sink, USB_DAC_RATE, 2);

    hw_on();
    core_init();
    /* MSC bring-up mutes the wired output (GPIO2.0 high) and disables the
     * amp; a DAC must undo that and drop any Bluetooth route so the local
     * DAC is audible. Do it even when configure() short-circuits on an
     * already-open 48 kHz stream (the paused player). */
    bt_hw_route(0);
    dac_gpio_out(2, 14, 1);            /* wired output selected */
    dac_gpio_out(2, 0, 0);             /* hardware mute off     */
    dac_gpio_out(1, 4, 1);             /* amp enable            */
    if (audio_output_configure(&fmt) != DAC_OK) {
        ret = USB_EXIT_IO;
        goto out;
    }
    audio_output_start(&s.ring);
    usb_dac_sink_prefill(&s.sink);
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
    while (!s.exit)
        poll();
    ret = s.exit;

out:
    audio_output_stop();
    audio_output_power_off();
    OTG(DCTL) |= 2u;
    rk_delay_us(150000);
    flush_fifos();
    hw_off();
    scratch_release(SCRATCH_OWNER_USB);

    if (diag) {
        diag->packets = s.sink.packets;
        diag->frames = s.sink.frames;
        diag->dropped = s.sink.dropped;
        diag->repeated = s.sink.repeated;
        diag->alt = (uint8_t)s.alt;
        diag->configured = (uint8_t)s.configured;
        diag->exit = ret;
    }
    return ret;
}
