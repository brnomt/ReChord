/* usb_cdc_rknanoc.c — USB CDC-ACM (virtual serial) console for the Echo Mini.
 *
 * Clean-room, polled DWC2 OTG device at 0x60080000 (same core + register
 * model as usb_msc_rknanoc.c). This enumerates as a CDC-ACM class device
 * (/dev/ttyACM0 on Linux, COMx on Windows) so we can run an interactive
 * debug console: read/write CS43131 registers over I2C, scan the bus, and
 * read back live state without reflashing for every probe.
 *
 * Standalone (not composite with MSC): enter via the "USB console" menu
 * entry. Reboot returns to MSC (flash) mode. Interrupts stay off; everything
 * is polled from usb_cdc_run().
 *
 * Register model (RKNanoC DWC2), mirrored from usb_msc_rknanoc.c.
 */

#include <stdint.h>
#include "usb_cdc.h"
#include "usb_msc.h"                 /* usb_dwc2_init() shared bring-up     */

/* ---- DWC2 OTG registers ------------------------------------------------ */
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
    GOTGINT = 0x04, GAHBCFG = 0x08, GUSBCFG = 0x0C, GRSTCTL = 0x10, GINTSTS = 0x14,
    GINTMSK = 0x18, GRXSTSR = 0x1C, GRXSTSP = 0x20, GRXFSIZ = 0x24,
    GNPTXFSIZ = 0x28, GNPTXSTS = 0x2C, GHWCFG3 = 0x4C, GHWCFG4 = 0x50,
    DIEPTXF1 = 0x104, DIEPTXF2 = 0x108, DIEPTXF3 = 0x10C,
    DCFG = 0x800, DCTL = 0x804, DSTS = 0x808, DIEPMSK = 0x810,
    DOEPMSK = 0x814, DAINT = 0x818, DAINTMSK = 0x81C, PCGCCTL = 0xE00,
};
#define EP_ENA_CNAK 0x84000000u
#define EP_STALL    0x00200000u
#define EP_D0PID    0x10000000u
#define INT_RXFLVL  (1u << 4)
#define INT_RESET   (1u << 12)

/* CDC endpoints: EP1 IN/OUT = data (bulk), EP2 IN = notification (interrupt). */
#define EP_DATA_IN  1
#define EP_DATA_OUT 1
#define EP_NOTIF    2

static struct {
    uint32_t setup[2];
    uint8_t  in_buf[64], out_buf[64];
    unsigned in_len, in_off, out_len, out_off;
    int      setup_pending, configured, exit;
    uint8_t  line[7];                 /* SET_LINE_CODING payload          */
} s;

/* On-screen enumeration diagnostics: what the host actually sent us. */
static struct usb_cdc_diag_t s_diag;

const struct usb_cdc_diag_t *usb_cdc_diag(void) { return &s_diag; }

/* ---- small helpers ----------------------------------------------------- */
static void rk_delay_us(unsigned us)
{
    volatile unsigned n = us * 8u;
    while (n--) __asm__ volatile("nop");
}

/* usb_cable() comes from usb_msc.h (real VBUS detect via GRF). */

static int wait_clear(unsigned off, uint32_t mask)
{
    for (int i = 10001; i && (OTG(off) & mask); i--)
        ;
    return (OTG(off) & mask) ? -1 : 0;
}

/* ---- FIFO helpers ------------------------------------------------------ */
static void fifo_read(uint8_t *dst, unsigned cnt, unsigned total)
{
    unsigned i;
    (void)total;
    for (i = 0; i < (cnt + 3u) / 4u; i++) {
        uint32_t v = EPFIFO(0);
        unsigned k;
        for (k = 0; k < 4u && i * 4u + k < cnt; k++)
            dst[i * 4u + k] = (uint8_t)(v >> (8u * k));
    }
}

static void fifo_write(unsigned ep, const uint8_t *src, unsigned cnt)
{
    unsigned i;
    for (i = 0; i < cnt; i += 4u) {
        uint32_t v = 0;
        unsigned k;
        for (k = 0; k < 4u && i + k < cnt; k++)
            v |= (uint32_t)src[i + k] << (8u * k);
        EPFIFO(ep) = v;
    }
}

static void arm_ep0_out(void)
{
    DOEPTSIZ(0) = 0x60080040u;        /* 3 SETUP, 1 packet, 64 bytes       */
    DOEPCTL(0) |= EP_ENA_CNAK;
}

static void rx_drain(void);           /* forward decl (used by cdc_ep_in)  */

/* Synchronous IN transfer (EP0 data/status + bulk IN), byte-clone of the
 * MSC's ep_in(): clear EPINT, arm DIEPTSIZ, wait for TX FIFO space, push
 * FIFO, wait for completion. The earlier fire-and-forget ep0_send never
 * completed the data stage -> the device never enumerated. */
static int cdc_ep_in(unsigned ep, const void *buf, uint32_t len)
{
    const uint8_t *p = buf;
    uint32_t mps = 64u, tries = 2000000u;

    DIEPINT(ep) = 0xFFu;
    DIEPTSIZ(ep) = len | (len ? (len + mps - 1u) / mps : 1u) << 19;
    DIEPCTL(ep) |= EP_ENA_CNAK;
    for (;;) {
        if (len) {
            uint32_t n = len < mps ? len : mps;
            uint32_t space = OTG(GNPTXSTS) & 0xFFFFu;
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

static void ep0_send(const void *data, uint32_t n)
{
    cdc_ep_in(0, data, n);
}

static void ep0_stall(void)
{
    DIEPCTL(0) |= EP_STALL;
    DOEPCTL(0) |= EP_STALL;
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
    unsigned n;
    s_diag.resets++;
    OTG(DCFG) &= ~0x7F0u;
    DIEPINT(0) = 0xFFu;
    DOEPINT(0) = 0xFFu;
    for (n = 1; n < 4u; n++) {
        DIEPINT(n) = 0xFFu;
        DOEPINT(n) = 0xFFu;
        DIEPCTL(n) = (DIEPCTL(n) & 0x80000000u) ? 0x48000000u : 0u;
        DOEPCTL(n) = (DOEPCTL(n) & 0x80000000u) ? 0x48000000u : 0u;
    }
    flush_fifos();
    OTG(DAINTMSK) = 0x10001u;
    arm_ep0_out();
    s.configured = 0;
    s.in_len = s.in_off = s.out_len = s.out_off = 0;
}

/* ---- descriptors ------------------------------------------------------- */
static const uint8_t k_dev[18] = {
    0x12, 0x01, 0x00, 0x02, 0x02, 0x00, 0x00, 0x40,   /* class=2 (CDC)     */
    0x1B, 0x07, 0x03, 0x32, 0x00, 0x02, 0x01, 0x02, 0x03, 0x01,
};
static const uint8_t k_qual[10] = { 10, 6, 0x00, 0x02, 0x02, 0x00, 0, 0x40, 1, 0 };
static const uint8_t k_str0[4] = { 4, 3, 0x09, 0x04 };
static const char *const k_str[4] = { 0, "ReChord", "ReChord", "000000000001" };

/* Config: IAD + comm iface (CDC Header/ACM/Union/CallMgmt + notif EP) +
 * data iface (bulk IN + bulk OUT). Total length 75. */
static const uint8_t k_cfg[75] = {
    9, 2, 75, 0, 2, 1, 0, 0x80, 50,          /* config: 2 interfaces        */
    8, 0x0B, 0, 2, 0x02, 0x02, 0x01, 0,      /* IAD: if 0..1, CDC/ACM        */
    /* comm interface (if 0) */
    9, 4, 0, 0, 1, 0x02, 0x02, 0x01, 0,      /* CDC comm, ACM, AT           */
    5, 0x24, 0x00, 0x10, 0x01,               /* CDC Header 1.10             */
    4, 0x24, 0x02, 0x02,                     /* CDC ACM                     */
    5, 0x24, 0x06, 0, 1,                     /* CDC Union: if0 master if1   */
    5, 0x24, 0x01, 0, 1,                     /* CDC Call Mgmt: if0          */
    7, 5, 0x82, 3, 8, 0, 9,                  /* EP2 IN interrupt (notif)    */
    /* data interface (if 1) */
    9, 4, 1, 0, 2, 0x0A, 0, 0, 0,            /* CDC data, 2 bulk EPs        */
    7, 5, 0x81, 2, 0x40, 0, 0,               /* EP1 IN bulk                 */
    7, 5, 0x01, 2, 0x40, 0, 0,               /* EP1 OUT bulk                */
};

static uint32_t build_str(uint8_t *dst, const char *s)
{
    unsigned i, n = 0;
    dst[1] = 3;
    for (i = 0; s[i] && n < 60u; i++) {
        dst[2 + n] = (uint8_t)s[i];
        dst[3 + n] = 0;
        n += 2;
    }
    dst[0] = (uint8_t)(2u + n);
    return 2u + n;
}

/* ---- EP0 setup handler ------------------------------------------------- */
static void handle_setup(void)
{
    static uint8_t buf[64];
    const uint8_t *r = (const uint8_t *)s.setup;
    uint8_t type = r[0], req = r[1];
    uint32_t val = r[2] | r[3] << 8, idx = r[4] | r[5] << 8, wlen = r[6] | r[7] << 8;
    const void *data = buf;
    uint32_t n = 0;
    int i;

    for (i = 0; i < 8; i++)
        s_diag.last_setup[i] = r[i];
    s_diag.setups++;

    if ((type & 0x60u) == 0x20u) {              /* CDC class requests       */
        s_diag.class_req++;
        switch (req) {
        case 0x20:                              /* SET_LINE_CODING (7 bytes) */
            if (wlen == 7u) {                   /* data stage follows        */
                DOEPCTL(0) |= EP_ENA_CNAK;
                DOEPTSIZ(0) = 0x60080007u;      /* accept 7 bytes            */
                return;
            }
            goto status;
        case 0x21:                              /* GET_LINE_CODING           */
            data = s.line;
            n = 7;
            goto send;
        case 0x22:                              /* SET_CONTROL_LINE_STATE    */
            goto status;
        default:
            goto stall;
        }
    }
    if (type & 0x60u)
        goto stall;
    switch (req) {
    case 0:                                     /* GET_STATUS                */
        buf[0] = buf[1] = 0;
        n = 2;
        goto send;
    case 1:                                     /* CLEAR_FEATURE             */
    case 3:                                     /* SET_FEATURE               */
        goto status;
    case 5:                                     /* SET_ADDRESS               */
        OTG(DCFG) = (OTG(DCFG) & ~0x7F0u) | (val & 0x7Fu) << 4;
        s_diag.set_addr++;
        goto status;
    case 6:                                     /* GET_DESCRIPTOR            */
        switch (val >> 8) {
        case 1: data = k_dev; n = sizeof k_dev; s_diag.desc_dev++; break;
        case 2: data = k_cfg; n = sizeof k_cfg; s_diag.desc_cfg++; break;
        case 3: {
            unsigned si = val & 0xFFu;
            s_diag.desc_str++;
            if (si == 0u) { data = k_str0; n = sizeof k_str0; }
            else if (si < 4u) n = build_str(buf, k_str[si]);
            else goto stall;
            break;
        }
        case 6: data = k_qual; n = sizeof k_qual; break;
        default: goto stall;
        }
        if (n > wlen) n = wlen;
        goto send;
    case 9:                                     /* SET_CONFIGURATION         */
        /* activate CDC data EPs (bulk) + notif EP (interrupt) */
        DIEPCTL(EP_DATA_IN)  = 0x40u | 0x18088000u;  /* EP1 IN bulk, 64B   */
        DOEPCTL(EP_DATA_OUT) = 0x40u | 0x18088000u;  /* EP1 OUT bulk, 64B  */
        DIEPCTL(EP_NOTIF)    = 0x40u | 0x1808C000u;  /* EP2 IN intr, 64B   */
        s.configured = 1;
        goto status;
    default:
        goto stall;
    }

send:
    ep0_send(data, n);
    return;
status:
    ep0_send(0, 0);
    return;
stall:
    ep0_stall();
}

/* ---- RX path (EP0 setup + bulk OUT) ------------------------------------ */
static void rx_drain(void)
{
    while (OTG(GINTSTS) & INT_RXFLVL) {
        uint32_t st = OTG(GRXSTSP), sts = (st >> 17) & 15u, cnt = (st >> 4) & 0x7FFu;
        if (sts == 6u) {                        /* SETUP                     */
            fifo_read((uint8_t *)s.setup, cnt < 8u ? cnt : 8u, cnt);
            s.setup_pending = 1;
        } else if (sts == 2u && cnt) {          /* OUT data                  */
            unsigned take = cnt < sizeof s.out_buf - s.out_len ? cnt : sizeof s.out_buf - s.out_len;
            fifo_read(s.out_buf + s.out_len, take, cnt);
            s.out_len += take;
        }
    }
    if (DOEPINT(0) & 9u) {
        if (DOEPINT(0) & 8u)
            s.setup_pending = 1;
        DOEPINT(0) = 9u;
    }
    if (DOEPINT(EP_DATA_OUT) & 1u) {
        DOEPINT(EP_DATA_OUT) = 1u;
        arm_ep0_out();
    }
}

/* ---- core init: shared usb_dwc2_init() (hw_on + core_init) from usb_msc -- */

/* ---- public: send text to the host (bulk IN) --------------------------- */
int usb_cdc_puts(const char *str)
{
    unsigned n = 0;
    while (str[n] && n < sizeof s.in_buf)
        n++;
    return cdc_ep_in(EP_DATA_IN, str, n);
}

int usb_cdc_getc(void)                          /* -1 if none               */
{
    if (s.out_off >= s.out_len)
        return -1;
    return s.out_buf[s.out_off++];
}

int usb_cdc_available(void)
{
    return (int)(s.out_len - s.out_off);
}

void usb_cdc_request_exit(void)
{
    s.exit = 1;
}

/* ---- main poll loop ---------------------------------------------------- */
void usb_cdc_init(void)
{
    s.exit = 0;
    s.line[0] = 0x80; s.line[1] = 0x25; s.line[2] = 0;   /* 9600 8N1        */
    s.line[3] = 0; s.line[4] = 0; s.line[5] = 0; s.line[6] = 8;
    usb_dwc2_init();                           /* shared DWC2 bring-up       */
}

void usb_cdc_poll(void)
{
    if (OTG(GINTSTS) & INT_RESET) {          /* bus reset from host         */
        OTG(GINTSTS) = INT_RESET;
        bus_reset();
    }
    rx_drain();
    if (s.setup_pending) {
        s.setup_pending = 0;
        handle_setup();
    }
    s_diag.gintsts = OTG(GINTSTS);
    s_diag.dsts = OTG(DSTS);
    s_diag.daint = OTG(DAINT);
    s_diag.configured = (uint32_t)s.configured;
}

int usb_cdc_run(void)
{
    usb_cdc_init();
    while (!s.exit) {
        usb_cdc_poll();
        rk_delay_us(10);
    }
    return 0;
}
