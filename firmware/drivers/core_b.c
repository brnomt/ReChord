/*
 * core_b.c — start the second Cortex-M3 with our own heartbeat image.
 *
 * Sequence taken from the reference fw1 core-start (REA 0x0305B93C) and the
 * SDK BBSystemInit: power the "high" domain, clock the core/RAM, de-assert
 * HIGHRAM0_SRST, assert CAL_CORE_SRST, copy the image to HIGHRAM0
 * (0x01000000), release the core, then poll the shared block at 0x01020000
 * for the readiness magic. Non-blocking: a timeout just returns -1.
 *
 * The B blob is linked into the AP image as rodata (objcopy -I binary).
 */
#include <stdint.h>

#include "ao_regs.h"
#include "core_b.h"
#include "rknano_i2s.h"      /* cru_gate(), cru_rst(), dac_delay_ms() */

#define PMU_PWRDN_CN   REG32(0x50020000u + 0x0Cu)
#define PMU_PD_HIGH    (1u << 1)

#define SHARED         ((volatile uint32_t *)0x01020000u)
#define BB_MAGIC       0x42524459u          /* 'BRDY' */

#define HIGHRAM0_BASE  0x01000000u
#define BB_MAX_BYTES   (128u * 1024u)

/* Gate IDs (Hw_cru.h eCLOCK_GATE). */
#define G_HCLK_CAL_CORE     2
#define G_HDRAM0            48          /* 48..55 HDRAM0..7, 56 IFACE */
#define G_HIRAM0            64          /* 64..67 HIRAM0..3, 72 IFACE */
#define G_HCLK_HIGH         73
#define G_HCLK_HIGH_MATRIX  74
#define G_HCLK_ASYNC_BRG    76
#define G_PCLK_MAILBOX      146

/* Soft-reset IDs (Hw_cru.h eSOFT_RST). */
#define R_MAILBOX_SRST      30
#define R_HIGHRAM0_SRST     34
#define R_CAL_CORE_SRST     50

extern const unsigned char _binary_bb_core_bin_start[];
extern const unsigned char _binary_bb_core_bin_end[];

static uint32_t s_magic;

int ap_core_b_start(void)
{
    const unsigned char *src = _binary_bb_core_bin_start;
    unsigned len = (unsigned)(_binary_bb_core_bin_end -
                              _binary_bb_core_bin_start);
    volatile uint32_t *dst = (volatile uint32_t *)HIGHRAM0_BASE;
    unsigned i;
    int tries;

    s_magic = 0;
    if (len == 0 || len > BB_MAX_BYTES || (len & 3u) != 0u)
        return -1;

    /* 1. Power the high domain and clock the core / high RAM (SDK). */
    PMU_PWRDN_CN &= ~PMU_PD_HIGH;
    cru_gate(G_HCLK_ASYNC_BRG, 1);
    cru_gate(G_HCLK_HIGH_MATRIX, 1);
    cru_gate(G_HCLK_CAL_CORE, 1);
    cru_gate(G_HCLK_HIGH, 1);
    for (i = G_HDRAM0; i <= G_HDRAM0 + 8u; i++)   /* incl. HDRAM_INTERFACE */
        cru_gate((int)i, 1);
    for (i = G_HIRAM0; i <= G_HIRAM0 + 3u; i++)
        cru_gate((int)i, 1);
    cru_gate(G_HIRAM0 + 8u, 1);                   /* HIRAM_INTERFACE = 72 */
    cru_gate(G_PCLK_MAILBOX, 1);

    cru_rst(R_HIGHRAM0_SRST, 0);
    cru_rst(R_MAILBOX_SRST, 0);

    /* 2. Assert the core reset, copy the image, then release it. */
    cru_rst(R_CAL_CORE_SRST, 1);
    for (i = 0; i < len / 4u; i++)
        dst[i] = ((const uint32_t *)src)[i];
    __asm__ volatile("dsb" ::: "memory");
    dac_delay_ms(5);

    SHARED[0] = 0;
    cru_rst(R_CAL_CORE_SRST, 0);

    /* 3. Poll the heartbeat (bounded, non-blocking). */
    dac_delay_ms(5);
    for (tries = 0; tries < 200; tries++) {       /* ~200 ms */
        s_magic = SHARED[0];
        if (s_magic == BB_MAGIC)
            return 0;
        dac_delay_ms(1);
    }
    return -1;
}

uint32_t ap_core_b_magic(void)
{
    return s_magic;
}
