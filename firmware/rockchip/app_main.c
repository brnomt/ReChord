/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   main.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_MAIN_

#include "SysInclude.h"
#include "fsinclude.h"
#include "mainmenu.h"
#include "PowerOn_Off.h"
#include "Hold.h"
#include "UsbAdapterProbe.h"

#ifdef _RADIO_
#include "FMControl.h"
#endif

#ifdef _MEDIA_MODULE_
extern  void FavoBlockInit(void);
#endif

extern  int32  FwCheck(void);
extern  uint32 BbSystemStartOK;

/*
--------------------------------------------------------------------------------
  Function name : __error__(char *pcFilename, unsigned long ulLine)
  Author        : ZHengYongzhi
  Description   : test program.

  Input         :
  Return        : TRUE

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef DEBUG
_ATTR_SYS_CODE_
void __error__(char *pcFilename, unsigned long ulLine)
{
    while (1);
}
#endif

_ATTR_SYS_INIT_DATA_ uint16 MediaInitStr[]   = {'M','e','d','i','a',' ','I','n','i','t','.','.','.',0};
_ATTR_SYS_INIT_DATA_ uint16 LowFormatStr[]   = {'L','o','w','F','o','r','m','a','.','.','.',0};
_ATTR_SYS_INIT_DATA_ uint16 SysAreaCopyStr[] = {'S','y','s','A','r','e','a','C','o','p','y','.','.','.',0};

//lable must be 8.3 format.
_ATTR_SYS_INIT_DATA_ uint8  VolumeName[] =
{
    'R',
    'o',
    'c',
    'k',
    'C',
    'h',
    'i',
    'p',
    '\0'
};

#ifdef DEEP_SLEEP
_ATTR_SYS_CODE_
void GpioIntForKey(void)
{
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : PowerOnCheckKey
  Author        : ZHengYongzhi
  Description   :  ��⿪ʼʱ�Ƿ���Play�����£����û�У��ػ�

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void PowerOnCheckKey(void)
{
    if ((0 == CheckVbus()) && (Get_PlayKey_State()))
    {
        //power key detect
        // if Reboot powerup, don't check Power Key
        // N   a  n   o   C  R   e  b    o  o  t   F    l    a   g
        // 4e 61 6e 6f, 43 52 65 62, 6f 6f 74 46, 6c 61 67
        if ((RebootTag[0] != 0x6f6e614e)||
                (RebootTag[1] != 0x62655243)||
                (RebootTag[2] != 0x46746f6f)||
                (RebootTag[3] != 0x0067616c))
        {
            System_Power_On(0);
            while (1);
        }
    }
    RebootTag[0] = 0;
    RebootTag[1] = 0;
    RebootTag[2] = 0;
    RebootTag[3] = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void CpuClockGateInit(void)
  Author        : ZHengYongzhi
  Description   : clock gating init.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2015/03/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void CpuClockGateInit(void)
{
    CRU->CRU_CLKGATE_CON[0] = (0x00010000 << (CLK_SYS_CORE_GATE%16))  |
                              (0x00010000 << (HCLK_SYS_CORE_GATE%16)) |
                              (0x00010001 << (HCLK_CAL_CORE_GATE%16)) |
                              (0x00010000 << (HCLK_LOGIC_GATE%16))    |
                              (0x00010000 << (PCLK_LOGIC_GATE%16))    |
                              (0x00010001 << (CLK_RESERVED0%16))      |
                              (0x00010000 << (HCLK_PMU_GATE%16))      |
                              (0x00010001 << (CLK_USBPHY_GATE%16))    |
                              (0x00010001 << (CLK_SFC_GATE%16))       |
                              (0x00010001 << (CLK_RESERVED1%16))      |
                              (0x00010001 << (CLK_SARADC_GATE%16))    |
                              (0x00010001 << (CLK_TIMER0_GATE%16))    |
                              (0x00010001 << (CLK_TIMER1_GATE%16))    |
                              (0x00010000 << (PCLK_PMU_GATE%16))      |
                              (0x00010000 << (CLK_ACODEC_GATE%16))    ;

    CRU->CRU_CLKGATE_CON[1] = (0x00010001 << (CLK_I2S1_SRC_GATE%16))      |
                              (0x00010001 << (CLK_I2S1_FRAC_SRC_GATE%16)) |
                              (0x00010001 << (CLK_I2S1_OUT_GATE%16))      |
                              (0x00010001 << (CLK_I2S1_GATE%16))          |
                              (0x00010001 << (CLK_I2S0_SRC_GATE%16))      |
                              (0x00010000 << (CLK_I2S_FRAC_SRC_GATE%16))  |
                              (0x00010000 << (CLK_I2S_GATE%16))           |
                              (0x00010001 << (DCLK_EBC_SRC_GATE%16))      |
                              (0x00010001 << (CLK_SDMMC_GATE%16))         |
                              (0x00010001 << (CLK_EMMC_GATE%16))          |
                              (0x00010001 << (CLK_SPI0_GATE%16))          |
                              (0x00010001 << (CLK_SPI1_GATE%16))          |
                              (0x00010000 << (CLK_UART0_GATE%16))         |
                              (0x00010000 << (CLK_UART1_GATE%16))         |
                              (0x00010000 << (CLK_UART2_GATE%16))         |
                              (0x00010001 << (CLK_TEST_GATE%16))          ;

    CRU->CRU_CLKGATE_CON[2] = (0x00010001 << (CLK_UART3_GATE%16)) |
                              (0x00010001 << (CLK_UART4_GATE%16)) |
                              (0x00010001 << (CLK_UART5_GATE%16)) ;

    CRU->CRU_CLKGATE_CON[3] = (0x00010001 << (HDRAM0_GATE%16))          |
                              (0x00010001 << (HDRAM1_GATE%16))          |
                              (0x00010001 << (HDRAM2_GATE%16))          |
                              (0x00010001 << (HDRAM3_GATE%16))          |
                              (0x00010001 << (HDRAM4_GATE%16))          |
                              (0x00010001 << (HDRAM5_GATE%16))          |
                              (0x00010001 << (HDRAM6_GATE%16))          |
                              (0x00010001 << (HDRAM7_GATE%16))          |
                              (0x00010001 << (HDRAM_INTERFACE_GATE%16)) |
                              (0x00010001 << (HCLK_HIFI_GATE%16))       ;

    CRU->CRU_CLKGATE_CON[4] = (0x00010001 << (HIRAM0_GATE%16))          |
                              (0x00010001 << (HIRAM1_GATE%16))          |
                              (0x00010001 << (HIRAM2_GATE%16))          |
                              (0x00010001 << (HIRAM3_GATE%16))          |
                              (0x00010001 << (Reserved4_GATE%16))       |
                              (0x00010001 << (Reserved5_GATE%16))       |
                              (0x00010001 << (Reserved6_GATE%16))       |
                              (0x00010001 << (Reserved7_GATE%16))       |
                              (0x00010001 << (HIRAM_INTERFACE_GATE%16)) |
                              (0x00010001 << (HCLK_HIGH_GATE%16))       |
                              (0x00010001 << (HCLK_HIGH_MATRIX_GATE%16))|
                              (0x00010001 << (HCLK_HIGH_DMA_GATE%16))   |
                              (0x00010001 << (HCLK_ASYNC_BRG_GATE%16))  ;


    CRU->CRU_CLKGATE_CON[5] = (0x00010000 << (PCLK_PMU_BUS_GATE%16))      |
                              (0x00010001 << (CLK_RESERVED2%16))          |
                              (0x00010001 << (CLK_RESERVED3%16))          |
                              (0x00010000 << (PCLK_GRF_GATE%16))          |
                              (0x00010000 << (CLK24M_GATE%16))            |
                              (0x00010000 << (PMU_RAM_INTERFACE_GATE%16)) |
                              (0x00010000 << (PMU_RAM0_GATE%16))          |
                              (0x00010000 << (PMU_RAM1_GATE%16))          |
                              (0x00010001 << (CLK_RESERVED4%16))          |
                              (0x00010001 << (PVTM_CLK_GATE%16))          ;

    CRU->CRU_CLKGATE_CON[6] = (0x00010000 << (DRAM0_GATE%16))          |
                              (0x00010000 << (DRAM1_GATE%16))          |
                              (0x00010000 << (DRAM2_GATE%16))          |
                              (0x00010000 << (DRAM3_GATE%16))          |
                              (0x00010000 << (DRAM4_GATE%16))          |
                              (0x00010000 << (DRAM5_GATE%16))          |
                              (0x00010000 << (DRAM6_GATE%16))          |
                              (0x00010000 << (DRAM7_GATE%16))          |
                              (0x00010000 << (DRAM_INTERFACE_GATE%16)) |
                              (0x00010000 << (HCLK_APB_BRG_GATE%16))   |
                              (0x00010001 << (HCLK_USBC_GATE%16))      |
                              (0x00010001 << (HCLK_SDMMC_GATE%16))     |
                              (0x00010000 << (HCLK_EMMC_GATE%16))      |
                              (0x00010001 << (HCLK_EBC_GATE%16))       |
                              (0x00010001 << (HCLK_SYNTH_GATE%16))     |
                              (0x00010001 << (HCLK_SFC_GATE%16))       ;

    CRU->CRU_CLKGATE_CON[7] = (0x00010000 << (IRAM0_GATE%16))          |
                              (0x00010000 << (IRAM1_GATE%16))          |
                              (0x00010000 << (IRAM2_GATE%16))          |
                              (0x00010000 << (IRAM3_GATE%16))          |
                              (0x00010000 << (IRAM4_GATE%16))          |
                              (0x00010000 << (IRAM5_GATE%16))          |
                              (0x00010000 << (IRAM6_GATE%16))          |
                              (0x00010000 << (IRAM7_GATE%16))          |
                              (0x00010000 << (IRAM8_GATE%16))|
                              (0x00010000 << (IRAM9_GATE%16))   |
                              (0x00010000 << (IRAM_INTERFACE_GATE%16)) |
                              (0x00010000 << (HCLK_BOOTROM_GATE%16))   |
                              (0x00010000 << (HCLK_1TOM_BRG_GATE%16))  |
                              (0x00010000 << (HCLK_DMA_GATE%16))       |
                              (0x00010001 << (HCLK_IMDCT_GATE%16))     |
                              (0x00010000 << (HCLK_LCDC_GATE%16))       ;

    CRU->CRU_CLKGATE_CON[8] = (0x00010000 << (HCLK_PMU_APB_BRG_GATE%16))|
                              (0x00010001 << (PCLK_I2S0_GATE%16))       |
                              (0x00010001 << (PCLK_I2S1_GATE%16))       |
                              (0x00010001 << (PCLK_I2C0_GATE%16))       |
                              (0x00010001 << (PCLK_I2C1_GATE%16))       |
                              (0x00010001 << (PCLK_I2C2_GATE%16))       |
                              (0x00010001 << (PCLK_SPI0_GATE%16))       |
                              (0x00010001 << (PCLK_SPI1_GATE%16))       |
                              (0x00010000 << (PCLK_UART0_GATE%16))      |
                              (0x00010000 << (PCLK_UART1_GATE%16))      |
                              (0x00010000 << (PCLK_UART2_GATE%16))      |
                              (0x00010001 << (PCLK_UART3_GATE%16))      |
                              (0x00010001 << (PCLK_UART4_GATE%16))      |
                              (0x00010001 << (PCLK_UART5_GATE%16))      |
                              (0x00010001 << (PCLK_TIMER_GATE%16))      |
                              (0x00010001 << (PCLK_WDT_GATE%16))        ;

    CRU->CRU_CLKGATE_CON[9] = (0x00010000 << (PCLK_PWM0_GATE%16))      |
                              (0x00010000 << (PCLK_PWM1_GATE%16))      |
                              (0x00010001 << (PCLK_MAILBOX_GATE%16))   |
                              (0x00010000 << (PCLK_SARADC_GATE%16))    |
                              (0x00010000 << (PCLK_GPIO0_GATE%16))     |
                              (0x00010000 << (PCLK_GPIO1_GATE%16))     |
                              (0x00010000 << (PCLK_ACODEC_GATE%16))    |
                              (0x00010000 << (PCLK_ASYNC_BRG_GATE%16)) |
                              (0x00010000 << (PCLK_LGC_MATRIX_GATE%16));
}

_ATTR_SYS_INIT_CODE_
void CpuSoftResetInit(void)
{
    CRU->CRU_SOFTRST[0] = (0x00010001 << (USBOTG_SRST%16))  |
                          (0x00010001 << (USBPHY_SRST%16))  |
                          (0x00010001 << (USBGLB_SRST%16))  |
                          (0x00010001 << (I2S0_SRST%16))    |
                          (0x00010001 << (I2S1_SRST%16))    |
                          (0x00010001 << (SPI0_SRST%16))    |
                          (0x00010001 << (SPI1_SRST%16))    |
                          (0x00010000 << (UART0_SRST%16))   |
                          (0x00010000 << (UART1_SRST%16))   |
                          (0x00010000 << (UART2_SRST%16))   |
                          (0x00010001 << (BOOTROM_SRST%16)) |
                          (0x00010000 << (SYSRAM1_SRST%16)) |
                          (0x00010000 << (SYSRAM0_SRST%16)) |
                          (0x00010001 << (LCDC_SRST%16))    |
                          (0x00010001 << (IMDCT_SRST%16))   |
                          (0x00010001 << (SYNTH_SRST%16))   ;

    CRU->CRU_SOFTRST[1] = (0x00010000 << (SYSDMA_SRST%16))   |
                          (0x00010001 << (SDMMC_SRST%16))    |
                          (0x00010000 << (SDIO_SRST%16))     |
                          (0x00010000 << (SYSMATRIX_SRST%16))|
                          (0x00010001 << (TIMER0_SRST%16))   |
                          (0x00010001 << (TIMER1_SRST%16))   |
                          (0x00010000 << (SARADC_SRST%16))   |
                          (0x00010000 << (PWM0_SRST%16))     |
                          (0x00010000 << (PWM1_SRST%16))     |
                          (0x00010001 << (SFC_SRST%16))      |
                          (0x00010001 << (I2C0_SRST%16))     |
                          (0x00010001 << (I2C1_SRST%16))     |
                          (0x00010001 << (I2C2_SRST%16))     |
                          (0x00010001 << (EBC_SRST%16))      |
                          (0x00010001 << (MAILBOX_SRST%16))  |
                          (0x00010001 << (WDT_SRST%16))      ;

    CRU->CRU_SOFTRST[2] = (0x00010001 << (HIGH_MATRIX_SRST%16)) |
                          (0x00010001 << (HIGHRAM1_SRST%16))    |
                          (0x00010001 << (HIGHRAM0_SRST%16))    |
                          (0x00010001 << (HIFI_SRST%16))        |
                          (0x00010001 << (RESERVED0%16))        |
                          (0x00010001 << (UART3_SRST%16))       |
                          (0x00010001 << (UART4_SRST%16))       |
                          (0x00010001 << (UART5_SRST%16))       |
                          (0x00010000 << (GPIO0_SRST%16))       |
                          (0x00010000 << (GPIO1_SRST%16))       ;

    CRU->CRU_SOFTRST[3] = (0x00010000 << (ACODEC_SRST0%16))    |
                          (0x00010000 << (SYS_CORE_SRST%16))  |
                          (0x00010001 << (CAL_CORE_SRST%16))  |
                          (0x00010000 << (BRG_TO_PMU_SRST%16))|
                          (0x00010001 << (RESERVED1%16))      |
                          (0x00010000 << (PMU_SRST%16))       |
                          (0x00010001 << (ASYNC_BRG_SRST%16)) |
                          (0x00010001 << (DMA2_SRST%16))      ;

}

/*
--------------------------------------------------------------------------------
  Function name : void IntrruptInit(void)
  Author        : ZHengYongzhi
  Description   : system breakpoint initialization
  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void SysIntrruptInit(void)
{
    //set priority.
    IrqPriorityInit();
    //......
    //register interrupt functions.
    IntRegister(FAULT_ID15_SYSTICK, (void*)SysTickHandler);
    IntRegister(INT_ID_DMA,         (void*)DmaInt);
    IntRegister(INT_ID_SRADC,       (void*)AdcIsr);
    IntRegister(INT_ID_GPIO2 ,      (void*)GpioInt2);

    //interrupt enable.
    IntEnable(FAULT_ID4_MPU);
    IntEnable(FAULT_ID5_BUS);
    IntEnable(FAULT_ID6_USAGE);
    IntEnable(FAULT_ID15_SYSTICK);
    IntEnable(INT_ID_DMA);
    IntEnable(INT_ID_SRADC);
    IntEnable(INT_ID_GPIO2);
    IntEnable(0);

    IntMasterEnable();
    //DEBUG("system INT initial: DMA & Systick INT enable");
}

_ATTR_SYS_INIT_CODE_
void SysCpuInit(void)
{
    VDD_VoltageAdjust_Init();
    VDD_VoltageAdjust_Enable();
    VDD_VoltageAdjust_Set(SCU_DCOUT_120);

    FREQ_Control_Init();
    FREQ_Enable();

    //USB Init:disable
    UsbIntControlInit();
#if(DEBUG_UART_PORT == 1)
    Grf_otgphy_suspend(1);
#elif(DEBUG_UART_PORT == 0)
    Grf_otgphy_suspend(0);
#elif(DEBUG_UART_PORT == 2)

#endif

    //ADC Vref powerOff
    Grf_Set_ADC_Vref_PowerOn(GRF_VREF_PWOFF);

    //softreset init
    CpuSoftResetInit();

    //clockgate init
    CpuClockGateInit();

#if (FRAME_SUB_BUFFER_NUM > 1)
    //Use B core memory for framebuffer
    PmuPdLogicPowerDown(0);

    ScuClockGateCtr(HCLK_ASYNC_BRG_GATE,1);
    ScuClockGateCtr(HCLK_HIGH_MATRIX_GATE,1);
    ScuClockGateCtr(HCLK_CAL_CORE_GATE,1);
    ScuClockGateCtr(HCLK_HIGH_GATE,1);
    ScuClockGateCtr(HDRAM_INTERFACE_GATE,1);
    ScuClockGateCtr(HDRAM6_GATE,1);
    ScuClockGateCtr(HDRAM7_GATE,1);

    ScuSoftResetCtr(ASYNC_BRG_SRST, FALSE);
    ScuSoftResetCtr(HIGH_MATRIX_SRST, FALSE);
    ScuSoftResetCtr(HIGHRAM1_SRST, FALSE);
#else
    //High Core init:disable
    PmuPdLogicPowerDown(1);
#endif

    //interruption initialization.
    IntMasterControlInit();
    SysIntrruptInit();
}

/*
--------------------------------------------------------------------------------
  Function name : void SysVariableInit(void)
  Author        : ZHengYongzhi
  Description   : system global variable initialization.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void SysVariableInit(void)
{
    //auto power off time initalization.
    HoldState                      = HOLD_STATE_OFF;
    SysTickCounter                 = 0;
    AutoPowerOffDisableCounter     = 0;
    BbSystemStartOK                = 0;

    gBattery.IsBattChange          = TRUE;
    chip_freq.armclk = 0;

    IsBackLightOn                  = FALSE;
    Acodec_over_cur_value = 0;
    Acodec_over_cur_count = 0;
    LCD_TEXTFORT   = 0xffff;
    LCD_IMAGEINDEX = 0xffff;

    DCDCLastLevel  = 0;//(Grf->MPMU_PWR_CON & MPMU_PWR_VID_BUCK_MASK) >> 6;
    LDOLastLevel   = 0;//(Grf->MPMU_PWR_CON & MPMU_PWR_VID_LDO_MASK ) >> 0;

#ifdef _FILE_DEBUG_
    pDebugHook = NULL;
    SysDebugHookInitMem();
#endif
}

/*
--------------------------------------------------------------------------------
  Function name : Gpio_Init
  Author        : ZHengYongzhi
  Description   : hardware initializtion
                  GPIO and AD initializtion��low power level check.
                  LCD and Codec initializtion.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_SYS_INIT_CODE_
void Gpio_Init(void)
{
    ////////////////////////////////////////////
    //for Power control
    //Power On
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin6,IOMUX_GPIO2A6_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin6,GPIO_OUT);  Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin6,GPIO_HIGH);
    //Play On Key
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin5,IOMUX_GPIO2A5_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin5,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin5,ENABLE);

    // pmu idle
    #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin7,IOMUX_GPIO2A7_PMU_IDEL);
    #endif

    //PWM1 (VDD_CORE FB)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3,GPIO_OUT); Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_HIGH);

    //PWM3 (CHG OK)
    #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin1,GPIO_IN);  Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin1,ENABLE);
    #endif

    //CHG_CURRENT
    #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin4,IOMUX_GPIO0B4_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin4,GPIO_OUT); Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin4,GPIO_HIGH);
    #endif

    ///////////////////////////////////////////////////////
    //for eMMC
    //eMMC
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin0,IOMUX_GPIO0A0_EMMC_PWREN);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_EMMC_CLK);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_EMMC_CMD);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_EMMC_D0);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_EMMC_D1);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_EMMC_D2);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_EMMC_D3);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7,IOMUX_GPIO0A7_EMMC_D4);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0,IOMUX_GPIO0B0_EMMC_D5);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1,IOMUX_GPIO0B1_EMMC_D6);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2,IOMUX_GPIO0B2_EMMC_D7);
    //PWM4 (eMMC SDCard Det or PWM)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin0,GPIO_IN);  Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin0,ENABLE);

    ////////////////////////////////////////////////
    //for LCD
    //lcd iomux
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin0,IOMUX_GPIO0C0_LCD_D0);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin1,IOMUX_GPIO0C1_LCD_D1);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin2,IOMUX_GPIO0C2_LCD_D2);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin3,IOMUX_GPIO0C3_LCD_D3);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin4,IOMUX_GPIO0C4_LCD_D4);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin5,IOMUX_GPIO0C5_LCD_D5);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin6,IOMUX_GPIO0C6_LCD_D6);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin7,IOMUX_GPIO0C7_LCD_D7);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_LCD_WRN);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin1,IOMUX_GPIO0D1_LCD_RS);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin7,IOMUX_GPIO0B7_LCD_CSN);
    //PWM0 (LCD BL)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin4,GPIO_OUT); Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin4,GPIO_LOW);

    //////////////////////////////////////////////
    //for WiFi or SDCard
    //WL_POWER:DNP
    #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin3,IOMUX_GPIO1B3_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin3,GPIO_IN);  Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin3,DISABLE);
    #endif

    //WL_REG_ON
    #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin5,IOMUX_GPIO0B5_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin5,GPIO_OUT); Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin5,GPIO_LOW);
    #endif

    //WL_HOST_WAKE
    #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin6,IOMUX_GPIO0B6_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin6,GPIO_IN);  Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin6,DISABLE);
    #endif

    //SDMMC (SDCard & WIFI)
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin5,IOMUX_GPIO1A5_SDMMC_CMD);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin6,IOMUX_GPIO1A6_SDMMC_CLK);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin7,IOMUX_GPIO1A7_SDMMC_D0);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin0,IOMUX_GPIO1B0_SDMMC_D1);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_SDMMC_D2);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin2,IOMUX_GPIO1B2_SDMMC_D3);

    //SDMMC_DET
#if(RKNANOD_CHIP_TYPE == RKNANOD_N)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin6,IOMUX_GPIO2B6_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin6,GPIO_IN);  Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin6,ENABLE);
#elif(RKNANOD_CHIP_TYPE == RKNANOD_G || RKNANOD_CHIP_TYPE == RKNANOD_L)
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin3,IOMUX_GPIO0B3_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin3,GPIO_IN);  Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin3,ENABLE);
#endif
    /////////////////////////////////////////////
    //for BT
    //UART1A  RTS&CTS (JTAG0)

    Grf_Force_Jtag_Set(0);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin6,IOMUX_GPIO2B6_UART1A_RTS);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin7,IOMUX_GPIO2B7_UART1A_CTS);

    //UART1A for BT
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_UART1A_TX);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_UART1A_RX);
    //I2S1A
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin0,IOMUX_GPIO1A0_I2S1A_CLK);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin1,IOMUX_GPIO1A1_I2S1A_LRCK);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin2,IOMUX_GPIO1A2_I2S1A_SCLK);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin3,IOMUX_GPIO1A3_I2S1A_SDO);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin4,IOMUX_GPIO1A4_I2S1A_SDI);

    ////////////////////////////////////////////
    //for I2C
    //I2C1A for FM & ALC5633
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin0,IOMUX_GPIO2B0_I2C1A_SDA);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin1,IOMUX_GPIO2B1_I2C1A_SCL);

    //I2C0A (HP_DET)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin2,IOMUX_GPIO2B2_IO);   Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin2,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin2,ENABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin3,IOMUX_GPIO2B3_IO);   Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin3,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin3,ENABLE);

    ///////////////////////////////////////////
    //Misc
    //UART0A (JTAG1)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_UART0A_RX);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_UART0A_TX);
    //PWM2 (USB Det)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin2,IOMUX_GPIO2A2_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin2,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin2,ENABLE);

}

/*
--------------------------------------------------------------------------------
  Function name : Gpio_Init
  Author        :
  Description   : for low power in IDLE state
  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
void GPIO_Suspend(void)
{
    #if 0
    ////////////////////////////////////////////
    //for Power control
    //PWM1 (VDD_CORE FB)
    //Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin3,DISABLE);
    //PWM3 (CHG OK)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin1,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin1,DISABLE);
    //CHG_CURRENT
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin4,IOMUX_GPIO0B4_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin4,GPIO_IN);  Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin4,DISABLE);

    ///////////////////////////////////////////////////////
    //for eMMC
    //eMMC
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin3,IOMUX_GPIO1B3_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin3,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin3,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin0,IOMUX_GPIO0A0_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin1,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin2,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin2,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin3,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin3,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin4,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin4,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin5,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin5,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin6,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin6,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7,IOMUX_GPIO0A7_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin7,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin7,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0,IOMUX_GPIO0B0_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1,IOMUX_GPIO0B1_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin1,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2,IOMUX_GPIO0B2_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin2,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin2,DISABLE);
    //PWM4 (eMMC SDCard Det or PWM)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin0,DISABLE);

    ////////////////////////////////////////////////
    //for LCD
    //lcd iomux
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin0,IOMUX_GPIO0C0_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin0,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin1,IOMUX_GPIO0C1_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin1,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin1,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin2,IOMUX_GPIO0C2_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin2,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin2,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin3,IOMUX_GPIO0C3_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin3,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin3,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin4,IOMUX_GPIO0C4_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin4,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin4,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin5,IOMUX_GPIO0C5_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin5,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin5,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin6,IOMUX_GPIO0C6_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin5,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin5,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin7,IOMUX_GPIO0C7_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin6,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin6,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin0,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin1,IOMUX_GPIO0D1_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin1,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin1,ENABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin7,IOMUX_GPIO0B7_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin7,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin7,ENABLE);
    //PWM0 (LCD BL)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin4,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin4,ENABLE);

    //////////////////////////////////////////////
    //for WiFi or SDCard
    //WL_HOST_WAKE
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin6,IOMUX_GPIO0B6_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin6,GPIO_IN);  Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin6,DISABLE);
    //WL_REG_ON
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin5,IOMUX_GPIO0B5_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin5,GPIO_IN);  Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin5,DISABLE);
    //SDMMC (SDCard & WIFI)
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin5,IOMUX_GPIO1A5_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin5,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin5,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin6,IOMUX_GPIO1A6_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin6,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin6,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin7,IOMUX_GPIO1A7_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin7,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin7,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin0,IOMUX_GPIO1B0_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin1,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin2,IOMUX_GPIO1B2_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin2,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin2,DISABLE);
    //SDMMC_DET
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin3,IOMUX_GPIO0B3_IO);  Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin3,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin3,DISABLE);

    /////////////////////////////////////////////
    //for BT
    //UART1A  RTS&CTS (JTAG0)
    Grf_Force_Jtag_Set(0);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin6,IOMUX_GPIO2B6_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin6,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin6,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin7,IOMUX_GPIO2B7_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin7,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin7,DISABLE);

    //UART1A for BT
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin0,DISABLE);
    //I2S1A
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin0,IOMUX_GPIO1A0_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin1,IOMUX_GPIO1A1_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin1,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin2,IOMUX_GPIO1A2_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin1,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin3,IOMUX_GPIO1A3_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin3,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin3,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin4,IOMUX_GPIO1A4_IO);  Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin4,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin4,DISABLE);

    ////////////////////////////////////////////
    //for I2C
    //I2C1A for FM & ALC5633
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin0,IOMUX_GPIO2B0_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin0,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin0,ENABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin1,IOMUX_GPIO2B1_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin1,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin1,ENABLE);

    //I2C0A for RTC HYM8563
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin2,IOMUX_GPIO2B2_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin2,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin2,ENABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin3,IOMUX_GPIO2B3_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin3,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin3,ENABLE);

    ///////////////////////////////////////////
    //Misc
    //UART0A (JTAG1)
    #ifndef BB_SYS_JTAG
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin4,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin4,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin5,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin5,DISABLE);
    #endif

    //PWM2 (USB Det)
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin2,IOMUX_GPIO2A2_IO);  Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin2,GPIO_IN);   Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin2,ENABLE);
    #endif
}

/*
--------------------------------------------------------------------------------
  Function name : void SysCpuInit(void)
  Author        : ZHengYongzhi
  Description   : PLL seting. disable interruption,PWM Etc.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void PowerOnDisplayInit(void)
{
    LCD_SetCharSizeInit();
    LCD_SetCharSize(FONT_12x12);
    LCD_SetImageIndex((uint16)(gSysConfig.ImageIndex));
    LCD_SetColor(COLOR_WHITE);

    PowerOnLcdinit();
    FREQ_EnterModule(FREQ_BLON);
    BL_On();

    if (CheckMsg(MSG_SYS_FW_RECOVERY))
    {
        //Firmware Recovery
    }
    else
    {
        //Display Power On Flash
        DisplayPowerOnFlash();
    }
}

/*
--------------------------------------------------------------------------------
  Function name : BoardInit
  Author        : ZHengYongzhi
  Description   : hardware initialization.
                  GPIO and AD initialization. low power level check.
                  lcd and Codec initial.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void BoardInit(void)
{
    //Gpio initialization.
    Gpio_Init();

    DebugInit();

    DEBUG("board initial...\n");

    MemDevInit();

#ifdef FW_UPDATE
    if (0 != FwCheck())
    {
        SendMsg(MSG_SYS_FW_RECOVERY);
    }
#endif

    LoadSysInformation();


    PowerOnDisplayInit();

    //ADC initialization.
    AdcInit();

    //power on level check.
    BatteryInit();

    //Codec initialization.
    Codec_PowerOnInitial();

    //key initialization.
    KeyInit();
}

/*
--------------------------------------------------------------------------------
  Function name : BoardInit
  Author        : ZHengYongzhi
  Description   : Flash and file system Etc initialization.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void MediaInit(void)
{
    HANDLE  hMainFile;
    uint32  retval;

    DEBUG("MediaInit Enter");

    SetDataDiskSize(SYS_DATA_DISK_SIZE);
#ifdef _MULT_DISK_
    SetUserDisk2Size(SYS_USER_DISK2_SIZE);
#endif

#ifdef _MEDIA_MODULE_
    MediaInfoAddr = FtlGetMediaBaseSecAddr();
    FavoBlockInit();  //get favorite information saved address.
#endif

    gSysConfig.Memory = FLASH0;
    if (GetMsg(MSG_FORMAT_DISK))
    {
        if (OK == MemDevProb(gSysConfig.Memory))
        {
            DEBUG("Format....");
            Format(FLASH0, FAT32);
            #ifdef _MEDIA_MODULE_
            gSysConfig.MedialibPara.MediaUpdataFlag = 1;
            #endif
            #ifdef AUDIOHOLDONPLAY
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0 ;
            #endif
        }
        else
        {
            retval = FileSysSetup(gSysConfig.Memory);
            if (retval == ERROR)
            {
                Format(FLASH0, FAT32);
                #ifdef _MEDIA_MODULE_
                gSysConfig.MedialibPara.MediaUpdataFlag = 1;
                #endif
                #ifdef AUDIOHOLDONPLAY
                gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0 ;
                #endif
            }
        }
    }
    else
    {
        retval = FileSysSetup(gSysConfig.Memory);
        if (retval == ERROR)
        {
            Format(FLASH0, FAT32);
            #ifdef _MEDIA_MODULE_
            gSysConfig.MedialibPara.MediaUpdataFlag = 1;
            #endif
            #ifdef AUDIOHOLDONPLAY
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0 ;
            #endif
        }
    }

#ifdef _MULT_DISK_
    //if there is file _$ROCK$.USB in FLASH_DISK0,then mount FLASH_DISK1 when connect USB.
#ifdef _CDROM_
    bShowFlash1 = 1;
    bShowCdrom = gSysConfig.bShowCdrom;
    if (0 == bShowCdrom)
    {
        gSysConfig.bShowCdrom = 1;
        ModuleOverlay(MODULE_ID_SYSRESERVED_OP, MODULE_OVERLAY_ALL);
        SaveSysInformation(1);
    }

#else
    bShowFlash1 = 0;
    if ((hMainFile = FileOpenA("\\","ROCKCHIPBIN", "R")) != NOT_OPEN_FILE)//after open file ok,mount disk.
    {
        FileClose(hMainFile);
        bShowFlash1 = 1;
    }
#endif
#endif

#ifdef  DISK_VOLUME
    VolumeCreate(VolumeName);
#endif

    //system poweron, need get memory info
    SendMsg(MSG_FLASH_MEM0_UPDATE);

    FLASH_PROTECT_ON();

    DEBUG("MediaInit Exit");

}

/*
--------------------------------------------------------------------------------
  Function name : void ModuleInit(void)
  Author        : ZHengYongzhi
  Description   : power on module initialization, breakpoint resume.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void ModuleInit(void)
{
    TASK_ARG TaskArg;

    if (CheckMsg(MSG_SYS_FW_RECOVERY))
    {
        TaskArg.Mdb.TaskID = TASK_ID_MAINMENU;
        TaskSwitch(TASK_ID_MDB, &TaskArg);
    }
    else if (CheckVbus())
    {
        #ifdef _USB_HOST_
            printf ("switch mainmenu\n");
            TaskArg.MainMenu.MenuID = 0;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
        #else
            printf ("\nswitch charge\n");
            TaskSwitch(TASK_ID_CHARGE, NULL);
        #endif
    }
    else
    {
#if 0
        TaskArg.Mdb.TaskID = TASK_ID_MAINMENU;

        if (gSysConfig.RadioConfig.HoldOnPlaySaveFlag > 0)
        {
            TaskArg.Mdb.TaskID = TASK_ID_RADIO;
        }
        else if (gSysConfig.MusicConfig.HoldOnPlaySaveFlag == 1)
        {
            TaskArg.Mdb.TaskID = TASK_ID_MEDIALIB;
        }
        TaskSwitch(TASK_ID_MDB, &TaskArg);
#endif

        TaskArg.MainMenu.MenuID = 0;
        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
    }
    pMainThread = (THREAD*)NULL;
}

/*
--------------------------------------------------------------------------------
  Function name : void SysTickInit(void)
  Author        : ZHengYongzhi
  Description   : system tick initialization.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_SYS_INIT_CODE_
void SysTickInit(void)
{
    SysTickDisable();

    SysTickClkSourceSet(NVIC_SYSTICKCTRL_CLKIN);
    SysTickPeriodSet(10);

    SysTickEnable();

    DEBUG("Systick initial: periodset 10ms");
}


/*
--------------------------------------------------------------------------------
  Function name : INT Main(void)
  Author        : ZHengYongzhi
  Description   : this the main funtion for tasks manage,and call task main window callback function.

  Input         :
  Return        : TRUE

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
UINT32 Main(void)
{
    UINT32 KeyTemp;

    //Grf_NOC_Remap_Sel(NOC_PMU_SRAM);

    IntMasterDisable(); /*disable int for sysinit*/

    //initialization for global variable.
    SysVariableInit();

    //PLL setting,disable interrupt and PWM.
    SysCpuInit();

    //system tick initialization.
    SysTickInit();

    FREQ_EnterModule(FREQ_INIT);

    //ADC initializtion, low power check,LCD initialization, Codec initialization.
    BoardInit();

    //Flash initial,get system parameter, file system initial
    MediaInit();

    //�ļ�ϵͳ��ʼ��֮����ܳ�ʼ�������ļ�
#ifdef _FILE_DEBUG_
    SysDebugHookInit();
#endif

    //module initialization.
    ModuleInit();

    SysServiceInit();

    {
        ModuleOverlay(MODULE_ID_FS_MEM_GET, MODULE_OVERLAY_ALL);
        GetFreeMem(FLASH0, &sysTotalMemeryFlash0, &sysFreeMemeryFlash0);
    }

    FREQ_ExitModule(FREQ_INIT); //init finished

#ifdef _RADIO_
    FmPowerOnInit(); /*fm is auto power up when sys reset*/
#endif

#ifdef _WATCH_DOG_
    WatchDogInitial();
#endif

    DEBUG("Os Start!");
    OSStart(&pMainWin, &pMainThread);

    return (TRUE);
}


UINT32 Main1(void)
{
    //Just for link
}

/*
********************************************************************************
*
*                         End of main.c
*
********************************************************************************
*/

