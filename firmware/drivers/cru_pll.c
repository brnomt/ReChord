/*
 * cru_pll.c — AP core clock switch to the APLL at 200 MHz.
 *
 * Ported from the RKnanoD SDK `SetPllFreq()` (driver/CRU/cru.c, >24 MHz
 * branch) with the field values the reference firmware actually ends up with
 * (device ECHOLOG.TXT: con0=0x1064 con1=0x9446 con2=0x08000000 mode=1
 * sel0=0x1001 sel1=0x200, pclk 50 MHz):
 *
 *   FBDIV=100, REFDIV=6, POSTDIV1=1, POSTDIV2=1, DSMPD=1  -> VCO 400 MHz
 *   sys_core_div=2, cal_core_div=2                        -> both cores 200 MHz
 *   pclk_logic_div=2 (code = /4)                          -> pclk 50 MHz
 *
 * The SDK also raises VDD_CORE (GPIO2A3 = PWM1 feedback select, high = 1.20 V)
 * and enables the GRF RAM "high response" mode at 200 MHz; both are done here.
 * Missing either destabilises the part.
 *
 * REGISTER OFFSETS (REA of the reference fw1 switch at 0x030516B8):
 *   APLL_CON0/1/2 = +0x00/04/08, MODE_CON = +0x10,
 *   CLKSEL core dividers = **+0x14** (0xFFFF1001),
 *   CLKSEL pclk/timer    = **+0x18** (0x03000200).
 * The first build used +0x18/+0x1C, leaving +0x14 unprogrammed so the cores
 * ran at VCO/postdiv = 400 MHz; it froze under load (DFIU incident
 * 505c0c1f). GRF_INTER_CON0 = GRF_BASE + 0xC4 (hw_grf.h struct: ...
 * GRF_IOMUX_CON 0xBC, PAD 0xC0, INTER_CON0 0xC4).
 */
#include <stdint.h>

#include "ao_regs.h"        /* CRU_BASE / GRF_BASE / REG32                */
#include "cru_pll.h"
#include "rknano_i2s.h"     /* dac_gpio_out(), delay_loops()              */

#define CRU_APLL_CON0 REG32(CRU_BASE + 0x00u)
#define CRU_APLL_CON1 REG32(CRU_BASE + 0x04u)
#define CRU_APLL_CON2 REG32(CRU_BASE + 0x08u)
#define CRU_MODE_CON  REG32(CRU_BASE + 0x10u)
#define CRU_CLKSEL0   REG32(CRU_BASE + 0x14u)   /* sys/cal core dividers */
#define CRU_CLKSEL1   REG32(CRU_BASE + 0x18u)   /* pclk / timer          */
#define GRF_INTER_CON0 REG32(GRF_BASE + 0xC4u)

/* APLL field macros, byte-identical to the SDK (Hw_cru.h). Each carries its
 * field's write-enable bits in the upper halfword. */
#define POSTDIV1(n)  (((0x07u << 12) << 16) | ((n) << 12))
#define FBDIV(n)     (((0xFFFu << 0) << 16) | ((n) << 0))
#define DSMPD(n)     (((0x01u << 12) << 16) | ((n) << 12))
#define POSTDIV2(n)  (((0x07u << 6) << 16) | ((n) << 6))
#define REFDIV(n)    (((0x3Fu << 0) << 16) | ((n) << 0))

#define APLL_NO_BYPASS (((0x01u << 16) | (uint32_t)~1u) << 15)

#define APLL_SW_RESET (((0x01u << 16) | 1u) << 15)   /* CON1 bit 15 */
#define APLL_PD       (((0x01u << 16) | 1u) << 14)   /* CON1 bit 14 */
#define APLL_PU       (((0x01u << 16) | 0u) << 14)
#define APLL_LOCK     (1u << 10)                     /* CON1 bit 10 (RO) */

#define APLL_MODE_SLOW   ((0x01u << 16) | 0u)        /* WORK_MODE bit 0 */
#define APLL_MODE_NORMAL ((0x01u << 16) | 1u)
#define CLK_SEL_MASK_FFFF (0xFFFFu << 16)
#define CLK_SEL_MASK_3    ((0x03u << 16) << 8)

void ap_clock_set_200mhz(void)
{
    uint32_t delay;

    /* 200 MHz needs VDD_CORE = 1.20 V first: GPIO2A3 is the DCDC feedback
     * select (SDK VDD_VoltageAdjust_Set(SCU_DCOUT_120)). */
    dac_gpio_out(2, 3, 1);
    delay_loops(4000);

    __asm__ volatile("cpsid i" ::: "memory");

    /* If the cores already run from the PLL, drop to the 24 MHz oscillator
     * and clear the dividers before touching the PLL. */
    if (CRU_MODE_CON & 1u) {
        CRU_MODE_CON = APLL_MODE_SLOW;
        CRU_CLKSEL0 = CLK_SEL_MASK_FFFF | 0x0000u;
        CRU_CLKSEL1 = CLK_SEL_MASK_3 | 0x0000u;
    }

    /* PLL reset -> power down -> configure -> power up. */
    CRU_APLL_CON1 = APLL_SW_RESET;
    CRU_APLL_CON1 = APLL_PD;
    CRU_APLL_CON2 = 0x08u << 24;

    CRU_APLL_CON0 = POSTDIV1(1) | FBDIV(100) | APLL_NO_BYPASS;
    CRU_APLL_CON1 = DSMPD(1) | POSTDIV2(1) | REFDIV(6);
    CRU_APLL_CON2 = (CRU_APLL_CON2 & (0x0Fu << 24)) | 0u;   /* Frac = 0 */

    CRU_APLL_CON1 = APLL_PU;

    /* 200 MHz RAM timing: GRF_INTER_CON0 = (PMU_RAM|LOW_IRAM|LOW_DRAM) twice
     * (0x3040 value, 0x3040 write-enable). */
    GRF_INTER_CON0 = 0x30403040u;

    /* Wait for the APLL lock, bounded (the SDK loops 240000 * Delay10cyc). */
    for (delay = 240000u; delay > 0u; delay--) {
        delay_loops(10);
        if (CRU_APLL_CON1 & APLL_LOCK)
            break;
    }

    /* Dividers, then switch the cores onto the PLL. */
    CRU_CLKSEL0 = CLK_SEL_MASK_FFFF |
                  ((2u - 1u) << 12) |   /* cal_core_div   = 2 */
                  ((1u - 1u) << 8)  |   /* cal_stclk_div  = 1 */
                  ((1u - 1u) << 4)  |   /* sys_stclk_div  = 1 */
                  ((2u - 1u) << 0);     /* sys_core_div   = 2 -> 200 MHz */
    delay_loops(10);
    CRU_CLKSEL1 = CLK_SEL_MASK_3 | (2u << 8);   /* pclk_logic /4 = 50 MHz */
    delay_loops(10);
    CRU_MODE_CON = APLL_MODE_NORMAL;

    __asm__ volatile("cpsie i" ::: "memory");
}
