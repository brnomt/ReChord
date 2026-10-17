/*
 * ao_regs.h — MMIO register defines for the ReChord audio HAL.
 *
 * VERBATIM relocation of the register block from dac_cs43131.c (commit
 * c5d571f). Every define below is byte-identical to the hardware-verified
 * driver; this header exists only so the split modules (rknano_i2s.c,
 * cs43131.c, audio_output.c) keep their register lines 1:1 unchanged.
 * DO NOT "clean up" values here — see DFIU 75271e5f (2x-speed framing bug
 * history) and docs/HANDOVER.md session F.
 */
#ifndef RECHORD_AO_REGS_H
#define RECHORD_AO_REGS_H

#include <stdint.h>

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
#define I2C_MRXRADDR REG32(I2C0_BASE + 0x0Cu)
#define I2C_MTXCNT  REG32(I2C0_BASE + 0x10u)
#define I2C_MRXCNT  REG32(I2C0_BASE + 0x14u)
#define I2C_IEN     REG32(I2C0_BASE + 0x18u)
#define I2C_IPD     REG32(I2C0_BASE + 0x1Cu)
#define I2C_TXFIFO(i) REG32(I2C0_BASE + 0x100u + 4u * (unsigned)(i))
#define I2C_RXFIFO(i) REG32(I2C0_BASE + 0x200u + 4u * (unsigned)(i))
#define I2C_CON_EN     (1u << 0)
#define I2C_CON_MOD(m) ((m) << 1)
#define I2C_CON_START  (1u << 3)      /* START bit: generate START cond.  */
#define I2C_CON_STOP   (1u << 4)      /* STOP bit: 0x11 = EN|STOP tail    */
#define I2C_CON_LASTACK (1u << 5)     /* NAK the last read byte            */
#define I2C_MODE_TX    0u
#define I2C_MODE_TRX   1u             /* write-phase + repeated-START read */
#define I2C_MTX_DONE   (1u << 2)      /* MBTFIPD */
#define I2C_MRX_DONE   (1u << 3)      /* MBRFIPD */
#define I2C_START_DONE (1u << 4)      /* STARTIPD */
#define I2C_STOP_DONE  (1u << 5)      /* STOPIPD */
#define I2C_NAK        (1u << 6)

#define ACODEC_BASE 0x40090000u

#define I2S1_BASE   0x40030000u
#define I2S_TXCR    REG32(I2S1_BASE + 0x00u)
#define I2S_RXCR    REG32(I2S1_BASE + 0x04u)
#define I2S_CKR     REG32(I2S1_BASE + 0x08u)
#define I2S_FIFOLR  REG32(I2S1_BASE + 0x0Cu)
#define I2S_DMACR   REG32(I2S1_BASE + 0x10u)
#define I2S_INTCR   REG32(I2S1_BASE + 0x14u)
#define I2S_INTSR   REG32(I2S1_BASE + 0x18u)
#define I2S_XFER    REG32(I2S1_BASE + 0x1Cu)
#define I2S_CLR     REG32(I2S1_BASE + 0x20u)
/* TXDR sits at +0x24, right after I2S_CLR — settled 2026-10-13: the SDK
 * I2SReg_t (Hw_i2s.h, where the (0x400-0x24) reserved padding is commented
 * out) and the stock DMA destination literal 0x40030024 (ref fw1 pool at
 * file 0x78a8, next to the 0x03090000 DMA-block base) both say +0x24, and
 * no 0x40030400 exists anywhere in the ref. The old +0x400 wrote every
 * tone sample into a void -> the FIFO never saw data -> no audio. */
#define I2S_TXDR    REG32(I2S1_BASE + 0x24u)

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

/* ---- NVIC / I2S interrupt bits (ISR feeder) ------------------------------ */

#define NVIC_ISER1  REG32(0xE000E104u)   /* IRQn 32..63 set-enable          */
#define NVIC_ICER1  REG32(0xE000E184u)   /* IRQn 32..63 clear-enable        */
#define NVIC_ICPR1  REG32(0xE000E284u)   /* IRQn 32..63 clear-pending       */
#define NVIC_IRQ32  1u                   /* INT_ID_I2S1                     */

#define INTCR_TX_EMPTY   0x1u            /* Hw_i2s.h bit 0                  */
#define INTCR_TX_UNDERR  0x2u            /* bit 1                           */
#define INTCR_TX_UR_CLR  0x4u            /* bit 2 (clear underrun)          */
#define INTSR_TX_UR_ACT  0x2u

#endif /* RECHORD_AO_REGS_H */
