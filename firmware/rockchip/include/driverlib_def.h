/*
 * driverlib_def.h — RKnanoC SoC register definitions (minimal build set).
 *
 * Derived from: docs/memory-map.md (SoC base 0x20000000), bootloader analysis
 * (CRU @ 0x20000000), SDK usage (cru2.c, interrupt2.c, dma2.c) and the
 * RKnano datasheet layout (Rockchip clock/peripheral block standard).
 *
 * This is the MINIMAL set needed to compile the SDK. Register fields are
 * refined from Ghidra decompilation as each driver is integrated.
 */
#ifndef DRIVERLIB_DEF_H
#define DRIVERLIB_DEF_H

#ifdef RECHORD_AP_BUILD
/* AP (fw1/UI) build: the complete RKnanoD SDK driver tree is on the include
 * path and its headers (driver/DriverInclude.h -> Hw_cru.h, cru.h, Hw_dma.h,
 * gpio.h, Adc.h, grf.h, BBSystem.h, codec.h, rockcodec.h, Dma.h, ...) already
 * define every SoC type and constant that the BB-only approximations below
 * would otherwise re-define.  Use the SDK's own (empty) shim instead. */
#include "../driver/driverlib_def.h"
#else
/* BB (section_3) build: the BB object graph does not compile the full driver
 * tree, so it still needs these RKnanoC register/constant approximations. */

#include <stdint.h>
#include "typedef.h"
#include "freq_enums.h"
#include "armcc_compat.h"

/* ================= SoC peripheral bases ================= */
#define RKNANO_CRU_BASE      0x40180000UL   /* Clock & Reset Unit (real SDK hw_memap.h) */
#define RKNANO_INTC_BASE     0x400B0000UL   /* Interrupt controller (TODO: verify) */

/* ================= CRU (Clock & Reset Unit) — layout matches real SDK
 *      Hw_cru.h CRUReg_t @ CRU_BASE 0x40180000 ================= */
typedef struct {
    volatile uint32_t CRU_APLL_CON0;        /* +0x00 */
    volatile uint32_t CRU_APLL_CON1;        /* +0x04 */
    volatile uint32_t CRU_APLL_CON2;        /* +0x08 */
    volatile uint32_t reserved1;            /* +0x0C */
    volatile uint32_t CRU_MODE_CON;         /* +0x10 */
    volatile uint32_t CRU_CLKSEL_CON[13];   /* +0x14 */
    volatile uint32_t reserved2[2];         /* +0x48 */
    volatile uint32_t CRU_CLK_FRACDIV_CON0; /* +0x50 */
    volatile uint32_t CRU_CLK_FRACDIV_CON1; /* +0x54 */
    volatile uint32_t reserved3[10];        /* +0x58 */
    volatile uint32_t CRU_CLKGATE_CON[10];  /* +0x80 */
    volatile uint32_t reserved4[6];         /* +0xA8 */
    volatile uint32_t CRU_SOFTRST[4];       /* +0xC0 */
    volatile uint32_t reserved5[4];         /* +0xD0 */
    volatile uint32_t CRU_STCLK_CON0;       /* +0xE0 */
    volatile uint32_t CRU_STCLK_CON1;       /* +0xE4 */
    volatile uint32_t reserved6[3];         /* +0xE8 */
    volatile uint32_t CRU_GLB_SRST_FST_VALUE; /* +0xF4 */
    volatile uint32_t CRU_GLB_CNT_TH;       /* +0xF8 */
} RKNANO_CRU;

#define CRU ((volatile RKNANO_CRU *)RKNANO_CRU_BASE)

/* APLL bit fields (cru2.c macros) */
#define APLL_REFDIV_SHIFT        0
#define APLL_REFDIV_MASK         0x0000003F
#define APLL_FBDIV_SHIFT         6
#define APLL_FBDIV_MASK          0x00000FFF
#define APLL_POSTDIV1_SHIFT      18
#define APLL_POSTDIV1_MASK       0x00000007
#define APLL_POSTDIV2_SHIFT      21
#define APLL_POSTDIV2_MASK       0x00000007
#define APLL_DSMPD_SHIFT         24
#define APLL_DSMPD_MASK          0x00000001
#define APLL_LOCK_SHIFT          31
#define APLL_LOCK_MASK           0x80000000

#define GPLL_REFDIV_SHIFT        0
#define GPLL_REFDIV_MASK         0x0000003F
#define GPLL_FBDIV_SHIFT         6
#define GPLL_FBDIV_MASK          0x00000FFF
#define GPLL_POSTDIV1_SHIFT      18
#define GPLL_POSTDIV1_MASK       0x00000007
#define GPLL_POSTDIV2_SHIFT      21
#define GPLL_POSTDIV2_MASK       0x00000007
#define GPLL_DSMPD_SHIFT         24
#define GPLL_DSMPD_MASK          0x00000001
#define GPLL_LOCK_SHIFT          31
#define GPLL_LOCK_MASK           0x80000000

/* ================= Interrupt controller ================= */
#define INTC_ISR                 (*(volatile uint32_t *)(RKNANO_INTC_BASE + 0x00))
#define INTC_IPR                 (*(volatile uint32_t *)(RKNANO_INTC_BASE + 0x04))
#define INTC_IER                 (*(volatile uint32_t *)(RKNANO_INTC_BASE + 0x08))
#define INTC_ISR_SET             (*(volatile uint32_t *)(RKNANO_INTC_BASE + 0x0C))
#define INTC_IPR_SET             (*(volatile uint32_t *)(RKNANO_INTC_BASE + 0x10))
#define INTC_IER_SET             (*(volatile uint32_t *)(RKNANO_INTC_BASE + 0x14))
#define INTC_ICR                 (*(volatile uint32_t *)(RKNANO_INTC_BASE + 0x18))
#define INTC_IPR_CLEAR           (*(volatile uint32_t *)(RKNANO_INTC_BASE + 0x1C))
#define INTC_IER_CLEAR           (*(volatile uint32_t *)(RKNANO_INTC_BASE + 0x20))

/* ================= System registers (misc) ================= */
#define SYS_CRU_BASE             RKNANO_CRU_BASE
#define SYS_INTC_BASE            RKNANO_INTC_BASE


/* ---- CRU clock gate / soft-reset enums (cru2.c) ---- */
#ifndef DRIVERLIB_CRU_ENUMS
#define DRIVERLIB_CRU_ENUMS
typedef enum {
    GATE_ACLK_CPU = 0, GATE_HCLK_CPU, GATE_PCLK_CPU,
    GATE_ACLK_PERI, GATE_HCLK_PERI, GATE_PCLK_PERI,
    GATE_ACLK_DSP, GATE_HCLK_DSP, GATE_PCLK_DSP,
    GATE_ACLK_SDMMC, GATE_HCLK_SDMMC, GATE_ACLK_USB, GATE_HCLK_USB,
    GATE_ACLK_I2S, GATE_HCLK_I2S, GATE_PCLK_I2C, GATE_PCLK_UART,
    GATE_ACLK_SPI, GATE_PCLK_SPI, GATE_PCLK_PWM, GATE_PCLK_SARADC,
    GATE_ACLK_LCDC, GATE_DCLK_LCDC, GATE_ACLK_GPU, GATE_ACLK_VEPU,
    GATE_ACLK_VDPU, GATE_HCLK_VIO, GATE_PCLK_VIO, GATE_ACLK_CIF, GATE_HCLK_CIF,
    GATE_ACLK_HDMI, GATE_PCLK_HDMI, GATE_ACLK_EMMC, GATE_HCLK_EMMC,
    GATE_MAX
} eCLOCK_GATE;

typedef enum {
    RST_ACLK_CPU = 0, RST_HCLK_CPU, RST_PCLK_CPU,
    RST_ACLK_PERI, RST_HCLK_PERI, RST_PCLK_PERI,
    RST_ACLK_DSP, RST_HCLK_DSP, RST_PCLK_DSP,
    RST_ACLK_SDMMC, RST_HCLK_SDMMC, RST_ACLK_USB, RST_HCLK_USB,
    RST_ACLK_I2S, RST_HCLK_I2S, RST_PCLK_I2C, RST_PCLK_UART,
    RST_ACLK_SPI, RST_PCLK_SPI, RST_PCLK_PWM, RST_PCLK_SARADC,
    RST_ACLK_LCDC, RST_DCLK_LCDC, RST_ACLK_GPU, RST_ACLK_VEPU,
    RST_ACLK_VDPU, RST_HCLK_VIO, RST_PCLK_VIO, RST_ACLK_CIF, RST_HCLK_CIF,
    RST_ACLK_HDMI, RST_PCLK_HDMI, RST_ACLK_EMMC, RST_HCLK_EMMC,
    RST_MAX
} eSOFT_RST;
#endif

/* ---- CPU frequency calibration struct (Delay2/systick2) ---- */
#ifndef DRIVERLIB_FREQ
#define DRIVERLIB_FREQ
typedef struct tagCHIP_FREQ {
    uint32 pll;              /* PLL frequency (Hz) */
    uint32 armFreq;          /* ARM core freq */
    uint32 armclk;           /* ARM clock */
    uint32 armclk2;          /* ARM clock alt (PowerManager) */
    uint32 hclk_sys_core;    /* HCLK sys core freq */
    uint32 hclk_cal_core;    /* HCLK calibration count */
    uint32 fclk_sys_core;    /* FCLK sys core (PowerManager) */
    uint32 fclk_cal_core;
    uint32 pclk_logic_pre;   /* PCLK logic pre-div */
    uint32 stclk_sys_core;   /* SysTick sys core */
    uint32 stclk_cal_core;   /* SysTick calibration count */
    uint32 sdramFreq;
} chip_freq_t;
extern chip_freq_t chip_freq2;
extern chip_freq_t chip_freq;   /* alias used by Delay.c */
#endif

/* ================= DMA (DesignWare DW_ahb_dma) ================= */
#define RKNANO_DMAC_BASE    0x40010000UL

typedef struct tagDMA_LLP {
    volatile uint32_t SAR;      /* source address */
    volatile uint32_t DAR;      /* dest address */
    volatile uint32_t LLP;      /* linked list pointer (next LLP) */
    volatile uint32_t CTLL;     /* control low */
    volatile uint32_t CTLH;     /* control high */
    volatile uint32_t SSTAT;    /* source status */
    volatile uint32_t DSTAT;    /* dest status */
    volatile uint32_t SSTAR;    /* source status addr */
    volatile uint32_t DSTAR;    /* dest status addr */
    volatile uint32_t RESERVED[3];
    volatile uint32_t CFGL;     /* config low */
    volatile uint32_t CFGH;     /* config high */
    volatile uint32_t SIZE;     /* transfer size (used by dma2.c) */
} DMA_LLP;

typedef DMA_LLP *pDMA_LLP;  /* pointer to LLP entry (SDK indexes pllplist[i]) */

typedef struct {
    volatile uint32_t SAR;      /* +0x00 source address */
    volatile uint32_t DAR;      /* +0x04 dest address */
    volatile uint32_t LLP;      /* +0x08 linked list */
    volatile uint32_t CTL_L;    /* +0x0C control low */
    volatile uint32_t CTL_H;    /* +0x10 control high */
    volatile uint32_t SSTAT;    /* +0x14 */
    volatile uint32_t DSTAT;    /* +0x18 */
    volatile uint32_t SSTAR;    /* +0x1C */
    volatile uint32_t DSTAR;    /* +0x20 */
    volatile uint32_t RESERVED[3]; /* +0x24..0x2C */
    volatile uint32_t CFG_L;    /* +0x30 config low */
    volatile uint32_t CFG_H;    /* +0x34 config high */
    volatile uint32_t SIZE;     /* +0x38 transfer size */
} DMA_CHANNEL;

typedef struct {
    volatile uint32_t ChEnReg;      /* +0x00 channel enable */
    volatile uint32_t ChEnWeReg;    /* +0x04 channel enable write */
    volatile uint32_t ClearTfr;     /* +0x08 clear transfer */
    volatile uint32_t StatusTfr;    /* +0x0C transfer status */
    volatile uint32_t ClearBlock;   /* +0x10 */
    volatile uint32_t StatusBlock;  /* +0x14 */
    volatile uint32_t ClearSrcTran; /* +0x18 */
    volatile uint32_t StatusSrcTran;/* +0x1C */
    volatile uint32_t ClearDstTran; /* +0x20 */
    volatile uint32_t StatusDstTran;/* +0x24 */
    volatile uint32_t ClearErr;     /* +0x28 */
    volatile uint32_t StatusErr;    /* +0x2C */
    volatile uint32_t StatusInt;    /* +0x30 */
    volatile uint32_t MaskTfr;      /* +0x34 transfer mask */
    volatile uint32_t MaskBlock;    /* +0x38 */
    volatile uint32_t MaskSrcTran;  /* +0x3C */
    volatile uint32_t MaskDstTran;  /* +0x40 */
    volatile uint32_t MaskErr;      /* +0x44 */
    volatile uint32_t MaskInt;      /* +0x48 */
    volatile uint32_t ClearInt;     /* +0x4C */
    volatile uint32_t DmaCfgReg;    /* +0x50 DMA config */
    volatile uint32_t DmaCfgWeReg;  /* +0x54 */
    DMA_CHANNEL CHANNEL[8];         /* +0x58 channels */
} RKNANO_DMA;

#define DmaReg2  ((volatile RKNANO_DMA *)RKNANO_DMAC_BASE)

/* DMA states */
#define DMA_IDLE    0
#define DMA_BUSY    1

/* CFGL bits */
#define B_CFGL_FIFO_EMPTY   0x00000008
#define B_CFGL_CH_SUSP      0x00000004
#define B_CTLL_LLP_SRC_EN   0x00000008
#define B_CTLL_LLP_DST_EN   0x00000004
#define B_CTLL_SRC_TR_WIDTH_MASK 0x00000070
#define B_CTLL_DST_TR_WIDTH_MASK 0x00000700

/* ---- DMA config parameter (pDMA_CFGX) ---- */
typedef struct {
    volatile uint32_t CFGL;
    volatile uint32_t CFGH;
    volatile uint32_t CTLL;
    volatile uint32_t CTLH;
    volatile uint32_t dma_mode;
} DMA_CFGX;

typedef DMA_CFGX *pDMA_CFGX;

/* ---- generic return codes ---- */
#ifndef RETURN_OK
#define RETURN_OK   0
#define RETURN_ERROR (-1)
#endif

/* ================= NVIC / SysTick (Cortex-M3 SCB) ================= */
#define RKNANO_SCB_BASE     0xE000E000UL
#define RKNANO_SYSTICK_BASE 0xE000E010UL

typedef struct {
    volatile uint32_t Ctrl;        /* +0x00 */
    volatile uint32_t Reload;      /* +0x04 */
    volatile uint32_t Value;       /* +0x08 */
    volatile uint32_t Calibration; /* +0x0C */
} RKNANO_SYSTICK;

typedef struct {
    volatile uint32_t IrqEnable;        /* ISER */
    volatile uint32_t IrqDisable;       /* ICER */
    volatile uint32_t IrqPending;       /* ISPR */
    volatile uint32_t IrqUnpend;        /* ICPR */
    volatile uint32_t IrqActive;        /* IABR */
    struct {
        volatile uint32_t Enable[8];
        volatile uint32_t Disable[8];
        volatile uint32_t SetPend[8];
        volatile uint32_t ClearPend[8];
        volatile uint32_t Pending[8];
        volatile uint32_t Priority[60];
    } Irq;
    volatile uint32_t INTcontrolState;  /* ICSR */
    volatile uint32_t VectorTableOffset;/* VTOR */
    volatile uint32_t APIntRst;         /* AIRCR */
    volatile uint32_t SystemHandlerCtrlAndState; /* SHCSR */
    volatile uint32_t SystemPriority[3];/* SHPR */
    RKNANO_SYSTICK SysTick;             /* 0xE000E010 */
} RKNANO_NVIC;

#define nvic  ((volatile RKNANO_NVIC *)RKNANO_SCB_BASE)

/* SysTick control bits */
#define NVIC_SYSTICKCTRL_ENABLE     0x00000001
#define NVIC_SYSTICKCTRL_TICKINT    0x00000002
#define NVIC_SYSTICKCTRL_CLKSOURCE  0x00000004
#define NVIC_SYSTICKCTRL_CLKIN      0x00000000
#define NVIC_SYSTICKCTRL_COUNTFLAG  0x00010000
#define NVIC_SYSTICKCALIB_NOREF     0x80000000
#define NVIC_SYSTICKCALIB_SKEW      0x40000000
#define NVIC_SYSTICKCALIB_TEMMS_MASK 0x00FFFFFF

/* DMA block sizes + interrupt enable */
#ifndef DRIVERLIB_DMA_EXTRA
#define DRIVERLIB_DMA_EXTRA
#define DMA_MAX_BLOCK_SIZE  0x1000   /* 4KB per block */
#define LLP_BLOCK_SIZE      0x1000
#define B_CTLL_INT_EN       0x00000001
#endif

/* DMA address increment modes (DW ahb-dma CTLL) */
#ifndef DRIVERLIB_DMA_INC
#define DRIVERLIB_DMA_INC
#define B_CTLL_SINC_MASK    0x00000300
#define B_CTLL_SINC_INC     0x00000000
#define B_CTLL_SINC_DEC     0x00000100
#define B_CTLL_DINC_MASK    0x00000C00
#define B_CTLL_DINC_INC     0x00000000
#define B_CTLL_DINC_DEC     0x00000400
#endif

#ifndef NULL
#define NULL  ((void *)0)
#endif

/* ================= GRF (General Register File) ================= */
#ifndef DRIVERLIB_GRF
#define DRIVERLIB_GRF
#define RKNANO_GRF_BASE     0x400C0000UL

typedef struct {
    volatile uint32_t GRF_GPIO0_DIR;        /* +0x00 */
    volatile uint32_t GRF_GPIO0_DR;         /* +0x04 */
    volatile uint32_t GRF_GPIO0_DDR;        /* +0x08 */
    volatile uint32_t GRF_GPIO0_SR;         /* +0x0C */
    volatile uint32_t GRF_GPIO0_SL;         /* +0x10 */
    volatile uint32_t GRF_GPIO0_SMT;        /* +0x14 */
    volatile uint32_t GRF_GPIO0_IE;         /* +0x18 */
    volatile uint32_t GRF_GPIO0_E;          /* +0x1C */
    volatile uint32_t GRF_SOC_CON[16];      /* +0x20 SoC control */
    volatile uint32_t GRF_SOC_STATUS[16];   /* +0x60 SoC status */
    volatile uint32_t GRF_SOC_USB_STATUS;    /* USB status (Hook.c) */
    volatile uint32_t GRF_IOFUNC_CON[16];   /* +0xA0 pin mux */
    volatile uint32_t GPIO_IO0MUX[4];
    volatile uint32_t GPIO_IO1MUX[4];
    volatile uint32_t GPIO_IO2MUX[4];
    volatile uint32_t GPIO_IO3MUX[4];
    volatile uint32_t GPIO_IO0PULL[4];
    volatile uint32_t GPIO_IO1PULL[4];
    volatile uint32_t GPIO_IO2PULL[4];
    volatile uint32_t GPIO_IO3PULL[4];
    volatile uint32_t GRF_IOFUNC_STATUS[16];/* +0xE0 */
} RKNANO_GRF;

#define Grf  ((volatile RKNANO_GRF *)RKNANO_GRF_BASE)

#endif /* DRIVERLIB_GRF */

/* ================= GPIO (OsHook.c) ================= */
#ifndef DRIVERLIB_GPIO
#define DRIVERLIB_GPIO
#define GPIO_CH0  0
#define GPIO_CH1  1
#define GPIO_CH2  2
#define GPIO_CH3  3
#define GPIO_CH4  4

#define GPIOPortA_Pin0  0
#define GPIOPortA_Pin1  1
#define GPIOPortA_Pin2  2
#define GPIOPortA_Pin3  3
#define GPIOPortA_Pin4  4
#define GPIOPortA_Pin5  5
#define GPIOPortA_Pin6  6
#define GPIOPortA_Pin7  7

typedef enum { IntrTypeLowLevel, IntrTypeHighLevel,
               IntrTypeRisingEdge, IntrTypeFallingEdge } IntrType;

#define IOMUX_GPIO2A7_PMU_IDEL  0

API void GpioIsrRegister(uint32 ch, uint32 pin, void (*isr)(void));
API void Gpio_SetIntMode(uint32 ch, uint32 pin, IntrType type);
API void Gpio_EnableInt(uint32 ch, uint32 pin);
API void Gpio_DisableInt(uint32 ch, uint32 pin);
API void Grf_GpioMuxSet(uint32 ch, uint32 pin, uint32 mode);
#endif /* DRIVERLIB_GPIO */





/* NVIC constants (interrupt2.c) */
#ifndef DRIVERLIB_NVIC_BITS
#define DRIVERLIB_NVIC_BITS
#define NVIC_APINTRST_VECTKEY         0x05FA0000
#define NVIC_APINTRST_PRIGROUP_MASK   0x00000700
#define NVIC_SYSHANDCTRL_MEMFAULTENA  0x00010000
#define NVIC_SYSHANDCTRL_BUSFAULTENA  0x00020000
#define NVIC_SYSHANDCTRL_USGFAULTENA  0x00040000
#define NVIC_INTCTRLSTA_NMIPENDSET    0x80000000
#define NVIC_INTCTRLSTA_PENDSVSET     0x10000000
#define NVIC_INTCTRLSTA_PENDSTSET     0x04000000
#define NVIC_INTCTRLSTA_PENDSVCLR     0x08000000
#define NVIC_INTCTRLSTA_PENDSTCLR     0x02000000
#define NVIC_INTCTRLSTA_ISRPENDING    0x00400000
#define NVIC_INTCTRLSTA_VECTACTIVE_MASK 0x000001FF
#endif

/* ================= SAR-ADC (battery.c) ================= */
#ifndef DRIVERLIB_ADC
#define DRIVERLIB_ADC
#define RKNANO_ADC_BASE     0x400D0000UL

typedef struct {
    volatile uint32_t ADC_CTRL;   /* +0x00 control */
    volatile uint32_t ADC_STAS;   /* +0x04 status */
    volatile uint32_t ADC_DATA;   /* +0x08 data */
} RKNANO_ADC;

#define Adc  ((volatile RKNANO_ADC *)RKNANO_ADC_BASE)

#define ADC_START       0x00000001
#define ADC_POWERUP     0x00000002
#define ADC_INT_ENBALE  0x00000004
#define ADC_CH_MASK     0x000000F0

extern uint32 AdcSamplingCh;
#endif

/* ================= I2S + PMU registers (Service.c) ================= */
#ifndef DRIVERLIB_I2S_PMU
#define DRIVERLIB_I2S_PMU
#define RKNANO_I2S_BASE   0x400C1000UL
#define RKNANO_PMU_BASE   0x400E0000UL

typedef struct {
    volatile uint32_t I2S_TXDR;    /* TX data register */
} RKNANO_I2S;
#define I2s_Reg  ((volatile RKNANO_I2S *)RKNANO_I2S_BASE)

typedef struct {
    volatile uint32_t PMU_SYS_REG3;
} RKNANO_PMU;
#define Pmu_Reg  ((volatile RKNANO_PMU *)RKNANO_PMU_BASE)

/* DMA I2S0 TX channel config (Service.c) */
#define DMA_CHN0  0
#define DMA_CHN1  1
#define DMA_CHN2  2
#define DMA_CHN3  3
#define DMA_CHN4  4
#define DMA_CHN5  5
#define DMA_CHN6  6
#define DMA_CHN7  7
#define DMA_CHN_MAX      8
#define DMA_CTLL_I2S0_TX  0x00000018   /* 32-bit, inc src, fixed dst */
#define DMA_CFGL_I2S0_TX  0x00000001
#define DMA_CFGH_I2S0_TX  0x00000000
#define DMA_CTLL_I2S0_RX  0x00000018
#define DMA_CFGL_I2S0_RX  0x00000001
#define DMA_CFGH_I2S0_RX  0x00000000

/* Watchdog / misc */
#define PCLK_WDT_GATE    0
#define NOC_BOOT_ROM     0
#endif

/* SCU output clock values (OsHook.c) */
#ifndef DRIVERLIB_SCU
#define DRIVERLIB_SCU
#define SCU_DCOUT_100  100000000
#define SCU_DCOUT_120  120000000
#endif

/* ---- PLL argument struct (PowerManager.c) ---- */
#ifndef DRIVERLIB_PLLARG
#define DRIVERLIB_PLLARG
typedef struct {
    uint32 VCO;
    uint32 div_con_24m;
    uint32 div_con;
    uint32 fbdiv;
    uint32 refdiv;
    uint32 postdiv1;
    uint32 postdiv2;
    uint32 dsm;
    uint32 sys_core_div;
    uint32 sys_stclk_div;
    uint32 cal_core_div;
    uint32 cal_stclk_div;
    uint32 pclk_logic_div;
} PLL_ARG_t;
extern PLL_ARG_t PllArg;
#endif

#define BAD_CLUS  0x0FFFFFF7
extern uint32 RX_FIFO_ADDR;
#define RECORD_DMACHANNEL_IIS  3
#define WAV_AD_PIPO_BUFFER_SIZE 4096
#define RECORD_STA_PCMBUF_EMPTY 0x01

#define FILE_NOT_EXIST  0xFFFFFFFF
#define ATTR_LFN_ENTRY  0x0F

#ifndef _ATTR_FAT_FIND_CODE_
#define _ATTR_FAT_FIND_CODE_
#endif
#define I2S_START_DMA_TX  1
#define Codec_DACoutHP    0
#define ACodec_I2S_DATA_WIDTH24  24

/* ---- audio clock / sample-rate constants (AudioControl.c) ---- */
#ifndef DRIVERLIB_AUDIO_CLK
#define DRIVERLIB_AUDIO_CLK
#define Pll_Target_Freq_40960   40960000
#define Pll_Target_Freq_56448   56448000
#define Pll_Target_Freq_61440   61440000
#define I2S_DATA_WIDTH24        24
#define F_SOURCE_24000KHz       24000000
#define FS_88200Hz              88200
#define FS_96KHz                96000
#define FS_8000Hz               8000
#define FS_16KHz                16000
#define FS_22050Hz              22050
#define FS_11025Hz              11025
#define FS_12000Hz              12000
#define FS_24000Hz              24000
#define FS_32000Hz              32000
#define FS_32KHz                32000
#define FS_128KHz               128000
#define FS_192KHz               192000
#define FS_48KHz                48000
#define FS_64KHz                64000
#define FS_12KHz                12000
#define FS_24KHz                24000
#define FS_1764KHz              176400
#define I2S_CH    0
#define I2S_PORT  0
#endif

/* ---- BT status / fade (AudioControl.c) ---- */
#ifndef DRIVERLIB_BT_FADE
#define DRIVERLIB_BT_FADE
#define BT_WIN_STATUS_IDLE        0
#define BT_WIN_STATUS_CONNECTING  1
#define FADE_OUT                  0
#define FADE_IN                   1
extern uint32 BtWinStatus;
extern uint32 BluetoothReConnectResult;
#endif

#endif /* RECHORD_AP_BUILD */

#endif /* DRIVERLIB_DEF_H */

