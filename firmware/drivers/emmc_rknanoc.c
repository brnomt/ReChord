/*
 * emmc_rknanoc.c -- eMMC on the RKNanoC DW-MMC controller (0x60030000),
 * polled.
 *
 * Clean-room implementation of FACTS extracted from the reference fw1
 * (init 0x030527ac, command 0x0305257c, data 0x030523d4, clock 0x0305235c,
 * block read 0x03052694 / write 0x0305271c, layout 0x03052a98; CRU gate
 * 0x030513e0 / reset 0x03051430).
 */
#include <stdint.h>
#include <string.h>

#include "emmc.h"
#include "rknanoc.h"

#define MMC  ((volatile uint32_t *)0x60030000u)
#define CRU  RK_CRU
#define GRF  RK_GRF
#define delay_us  rk_delay_us
#define clk_gate  cru_gate
#define clk_reset cru_reset

enum {                       /* DW-MMC word offsets */
    CTRL = 0, PWREN = 1, CLKDIV = 2, CLKSRC = 3, CLKENA = 4, TMOUT = 5,
    CTYPE = 6, BLKSIZ = 7, BYTCNT = 8, INTMASK = 9, CMDARG = 10, CMD = 11,
    RESP0 = 12, RINTSTS = 17, STATUS = 18, FIFOTH = 19, FIFO = 0x80,
};

#define CMD_START    0x80000000u
#define STATUS_BUSY  0x200u          /* card data busy */
#define STATUS_EMPTY 0x4u            /* FIFO empty */
#define F_RESP       0x40u
#define F_LONG       0x80u
#define F_CRC        0x100u
#define F_STOP       0x4000u
#define F_INIT       0x8000u
#define RCA_ARG      0x20000u        /* RCA 2 */

static uint32_t s_sectors, s_fifo_words = 16u;
static uint32_t s_write_floor = 0xFFFFFFFFu;   /* set by emmc_layout() */

static int wait_clear(unsigned reg, uint32_t mask, uint32_t tries)
{
    while (MMC[reg] & mask) {
        if (tries-- == 0)
            return -1;
        delay_us(1);
    }
    return 0;
}

static int update_clock(void)
{
    MMC[CMD] = 0x80202000u;          /* start | update clock registers only */
    return wait_clear(CMD, CMD_START, 50000);
}

/* Card clock from the 24 MHz source: CRU pre-divider, then CLKDIV. */
static int set_clock(uint32_t khz)
{
    uint32_t div = (khz + 23999u) / khz, cd;

    if (div != 1u) {
        if (div & 1u)
            div++;
        if (div > 31u)
            div = 32u;
    }
    cd = (24000u / div + khz - 1u) / khz;
    if (cd > 1u && (cd & 1u))
        cd++;
    MMC[CLKENA] = 0;
    if (update_clock())
        return -1;
    CRU[0x30 / 4] = 0x9F008000u | (div - 1u) << 8;
    MMC[CLKDIV] = cd >> 1;
    if (update_clock())
        return -1;
    MMC[CLKENA] = 0x10001u;
    return update_clock();
}

static int send_cmd(uint32_t idx, uint32_t arg, uint32_t flags, int wait_busy)
{
    uint32_t tries = 1000001u, st;

    if (wait_clear(STATUS, STATUS_BUSY, 500000))
        return -2;
    MMC[RINTSTS] = 0xFFFFFFFFu;
    MMC[CMDARG] = arg;
    MMC[CMD] = idx | flags | 0xA0002000u;
    if (wait_clear(CMD, CMD_START, 50000))
        return -3;
    while (!(MMC[RINTSTS] & 0x4u)) {        /* command done */
        if (--tries == 0)
            return -4;
        delay_us(1);
    }
    st = MMC[RINTSTS];
    if (st & 0x1102u)                       /* resp error / timeout / HLE */
        return -5;
    if ((flags & F_CRC) && (st & 0x40u))    /* resp CRC */
        return -6;
    if (wait_busy && wait_clear(STATUS, STATUS_BUSY, 1000000))
        return -7;
    return 0;
}

static int xfer_data(uint32_t idx, uint32_t arg, uint32_t *buf, uint32_t len, int write)
{
    uint32_t words = len >> 2, fifo_tries = 5000000u, tries = 5000001u;
    int ret = 0;

    if (wait_clear(STATUS, STATUS_BUSY, 1000000))
        return -2;
    if (!(MMC[STATUS] & STATUS_EMPTY)) {
        MMC[CTRL] |= 2u;                    /* FIFO reset */
        wait_clear(CTRL, 2u, 5000);
    }
    MMC[BLKSIZ] = 512u;
    MMC[BYTCNT] = len;
    MMC[RINTSTS] = 0xFFFFFFFFu;
    MMC[CMDARG] = arg;
    MMC[CMD] = idx | (write ? 0x400u : 0u) | 0xA0002340u;  /* data, CRC, resp */
    if (wait_clear(CMD, CMD_START, 50000))
        return -3;
    while (words) {
        uint32_t n;

        if (MMC[RINTSTS] & 0xAF82u)
            return -12;
        n = (MMC[STATUS] & 0x3FFFFFFFu) >> 17;  /* words in FIFO */
        if (write)
            n = n < s_fifo_words ? s_fifo_words - n : 0u;
        if (n == 0u) {
            if (--fifo_tries == 0u)
                return -13;
            delay_us(1);
            continue;
        }
        if (n > words)
            n = words;
        words -= n;
        if (write)
            while (n--)
                MMC[FIFO] = *buf++;
        else
            while (n--)
                *buf++ = MMC[FIFO];
    }
    while (!(MMC[RINTSTS] & 0xAE88u)) {     /* data over or error */
        if (--tries == 0u) {
            ret = -10;
            break;
        }
        delay_us(1);
    }
    if (!ret && (MMC[RINTSTS] & 0xAE80u))
        ret = -11;
    MMC[RINTSTS] = 0xFFFFFFFFu;
    return ret;
}

static int wait_tran(void)
{
    for (uint32_t i = 0; i < 100000u; i++) {
        uint32_t r;

        if (send_cmd(13, RCA_ARG, F_RESP | F_CRC, 0))
            return -1;
        r = MMC[RESP0];
        if ((r & 0x100u) && ((r & 0x1FFFu) >> 9) == 4u)   /* ready, tran */
            return 0;
        delay_us(10);
    }
    return -2;
}

int emmc_init(void)
{
    static uint32_t ext_csd[128];
    uint32_t i;

    clk_gate(0x19, 1);
    clk_gate(0x6C, 1);
    clk_reset(0x11, 0);
    CRU[0x30 / 4] = 0x9F009F00u;
    GRF[0] = 0xFFFF5555u;                   /* eMMC pinmux */
    GRF[1] = 0x003F0015u;
    GRF[9] = 0x00C00040u;
    delay_us(10);

    MMC[CTRL] = 7u;                         /* controller/FIFO/DMA reset */
    if (wait_clear(CTRL, 7u, 1000))
        return -1;
    i = (MMC[FIFOTH] & 0x0FFFFFFFu) >> 16;  /* reset RX watermark = depth-1 */
    s_fifo_words = (i - 7u < 0x3F9u) ? i + 1u : 16u;
    MMC[FIFOTH] = 0x300F0010u;
    MMC[CTYPE] = 0;
    MMC[CLKSRC] = 0;
    MMC[TMOUT] = 0xFFFFFF40u;
    MMC[RINTSTS] = 0xFFFFFFFFu;
    MMC[INTMASK] = 0;
    MMC[PWREN] = 1;
    if (set_clock(400))
        return -2;
    MMC[PWREN] = 0;
    delay_us(10);
    MMC[PWREN] = 1;
    delay_us(600);

    send_cmd(0, 0, F_INIT, 0);
    delay_us(100);
    for (i = 0; ; i++) {
        int r;

        if (i == 8000u)
            return -3;
        r = send_cmd(1, 0x40FF8000u, F_RESP, 0);
        if (r == 0 && (MMC[RESP0] & 0x80000000u))
            break;
        if (r == -3 || r == -4)             /* controller, not card, stuck */
            return -15;
        delay_us(1000);
    }
    if (!(MMC[RESP0] & 0x40000000u))        /* sector addressing required */
        return -4;
    if (send_cmd(2, 0, F_RESP | F_LONG | F_CRC, 0))
        return -5;
    if (send_cmd(3, RCA_ARG, F_RESP | F_CRC, 0))
        return -6;
    if (send_cmd(9, RCA_ARG, F_RESP | F_LONG | F_CRC, 0))
        return -7;
    if (set_clock(24000))
        return -8;
    if (send_cmd(7, RCA_ARG, F_RESP | F_CRC, 1))
        return -9;
    if (send_cmd(16, 512, F_RESP | F_CRC, 0))
        return -10;
    if (xfer_data(8, 0, ext_csd, 512, 0))   /* EXT_CSD */
        return -11;
    s_sectors = ext_csd[212 / 4];           /* SEC_COUNT */
    if (s_sectors < 0x100000u)
        return -12;
    if (send_cmd(6, 0x03B70200u, F_RESP | F_CRC, 1))   /* BUS_WIDTH = 8 */
        return -13;
    if (wait_tran())
        return -14;
    MMC[CTYPE] = 0x10000u;
    clk_gate(0x6C, 0);
    return 0;
}

uint32_t emmc_sectors(void)
{
    return s_sectors;
}

int emmc_read(uint32_t lba, uint32_t count, void *buf)
{
    int ret, stop;

    if (count == 0u)
        return 0;
    if (lba + count < lba || lba + count > s_sectors)
        return -30;
    clk_gate(0x6C, 1);
    if (count == 1u) {
        ret = xfer_data(17, lba, buf, 512, 0);
    } else {
        ret = xfer_data(18, lba, buf, count << 9, 0);
        stop = send_cmd(12, RCA_ARG, F_RESP | F_CRC | F_STOP, 1);
        if (!ret)
            ret = stop;
    }
    clk_gate(0x6C, 0);
    return ret;
}

/* Refuses anything below the user volume: IDB and firmware slots are
 * never writable from here (also not before emmc_layout() succeeded). */
int emmc_write(uint32_t lba, uint32_t count, const void *buf)
{
    int ret, r;

    if (count == 0u)
        return 0;
    if (lba < s_write_floor || lba + count < lba || lba + count > s_sectors)
        return -30;
    clk_gate(0x6C, 1);
    if (count == 1u) {
        ret = xfer_data(24, lba, (uint32_t *)buf, 512, 1);
    } else {
        ret = xfer_data(25, lba, (uint32_t *)buf, count << 9, 1);
        r = send_cmd(12, RCA_ARG, F_RESP | F_CRC | F_STOP, 1);
        if (!ret)
            ret = r;
    }
    r = wait_tran();                        /* programming done */
    if (!ret && r)
        ret = -31;
    clk_gate(0x6C, 0);
    return ret;
}

static uint32_t s_buf[256];

/* exFAT/FAT boot sector or an MBR with a first partition (reference
 * 0x03052a1c). */
static int is_volume_start(uint32_t lba)
{
    const uint8_t *b = (const uint8_t *)s_buf;

    if (lba >= s_sectors || emmc_read(lba, 1, s_buf) ||
        b[0x1FE] != 0x55 || b[0x1FF] != 0xAA)
        return 0;
    if (memcmp(b + 3, "EXFAT   ", 8) == 0 ||
        ((b[0] & 0xFD) == 0xE9 && (b[0x0B] | b[0x0C] << 8) == 0x200))
        return 1;
    return (b[0x1BE] & 0x7F) == 0 && b[0x1C2] != 0 &&
           (b[0x1C6] | b[0x1C7] | b[0x1C8] | b[0x1C9]) != 0;
}

/* IDB copies at LBA 0x40 + n*0x400: sector 0 magic (RC4'd 0x0FF0AA55),
 * "NANO" chip tag at +0x20A, system size in MiB at +0x202. The volume
 * normally starts right after the firmware area; otherwise the reference
 * scans forward in 1 MiB steps (this unit: 0x24000 holds a "SAVE" block,
 * the volume is at 0x88000). */
int emmc_layout(uint32_t *user_start)
{
    const uint8_t *b = (const uint8_t *)s_buf;

    for (uint32_t n = 0; n < 5u; n++) {
        uint32_t fw;

        if (emmc_read(0x40u + n * 0x400u, 2, s_buf) || s_buf[0] != 0xFCDC8C3Bu ||
            memcmp(b + 0x20A, "NANO", 4) != 0)
            continue;
        fw = (uint32_t)(b[0x202] | b[0x203] << 8) * 0x800u;
        if (fw == 0u || fw > s_sectors - 0x2000u)
            continue;
        for (uint32_t off = fw; off < 0x100000u; off += 0x800u) {
            if (is_volume_start(0x2000u + off)) {
                *user_start = s_write_floor = 0x2000u + off;
                return 0;
            }
        }
        return -2;
    }
    return -1;
}
