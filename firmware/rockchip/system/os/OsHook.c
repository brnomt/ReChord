/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  OsHook.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:
********************************************************************************
*/
#define _IN_OSHOOK_

//#include "OsInclude.h"

#include "freq_enums.h"             /* FREQ_BLON / FREQ_IDLE */
#include "driver/GPIO/gpio.h"      /* GPIO_CH2, GPIOPortA_Pin5 */
#include "driver/driverlib_def.h"  /* IntrTypeRisingEdge enum */
#include "driver/CRU/Hw_cru.h"     /* CRU gate IDs */
#include "driver/pmc/pmc.h"         /* SCU_DCOUT_* levels */
#include "SysInclude.h"

/*
--------------------------------------------------------------------------------
  Function name : UINT32 WinPaintCheckPaintAllHook(void)
  Author        : ZHengYongzhi
  Description   : check whethe is full screen refresh.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
UINT32 WinPaintCheckPaintAllHook(void)
{
#ifdef _FRAME_BUFFER_
    return 0;
#else
    return (CheckMsg(MSG_NEED_PAINT_ALL));
#endif
}

/*
--------------------------------------------------------------------------------
  Function name : void WinPaintDmaFillImage2Lcd(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void WinPaintDmaFillImage2Lcd(void)
{
    #ifdef _FRAME_BUFFER_
        Lcd_BuferTranfer();
    #endif
}

/*
--------------------------------------------------------------------------------
  Function name : void WinPaintBackLightOnHook(void)
  Author        : ZHengYongzhi
  Description   : open backlight

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void WinPaintBackLightOnHook(void)
{
    ClearMsg(MSG_NEED_PAINT_ALL);
    if(TRUE == GetMsg(MSG_NEED_BACKLIGHT_ON))
    {
        FREQ_EnterModule(FREQ_BLON);
        BL_On();
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void WinPaintBackLightOnHook(void)
  Author        : ZHengYongzhi
  Description   : open backlight

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
uint32 WinPaintBLStatusCheckHook(void)
{
    if((IsBackLightOn == TRUE)/* && (gSysSetBLModeArray[gSysConfig.BLmode] == BL_DARK)*/)
    {
        return 1;
    }
    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void Os_idle(void)
  Author        : ZHengYongzhi
  Description   : system enter idle

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#if 0
__asm void OS_WFI(void)
{
    WFI
    BX      R14
}
#endif

void Os_idle_playkey_wakeup(void)
{
    DEBUG();
}

void Os_idle(void)
{
    uint32 LDOBack,DCDCBack;
    uint32 SysClkBack;
    uint32 hpcon_bk, hpdac_bk;
    uint32 scu_clkgate0, scu_clkgate1;

    DEBUG("Enter");

    //wait for DMAs transfer finished
    //...
#ifdef _WATCH_DOG_
    //system idle ,restart watchdog
    WatchDogDisableInt();
    WatchDogDeInit();
#endif

    SysTickDisable();
    Codec_Suspend();

    //GPIO2A5: PlayKey int
    GPIO_Suspend();
    GpioIsrRegister(GPIO_CH2,GPIOPortA_Pin5, Os_idle_playkey_wakeup);
    Gpio_SetIntMode(GPIO_CH2,GPIOPortA_Pin5, IntrTypeRisingEdge);
    Gpio_EnableInt(GPIO_CH2,GPIOPortA_Pin5);

    //clockgate off

    FREQ_EnterModule(FREQ_IDLE);

    VDD_VoltageAdjust_Set(SCU_DCOUT_100);
    Delay100cyc(500);

    #if 0
    //OSC off, PVTM On
    Grf_PVTM_Set_Cal_Cnt(1000);
    Grf_PVTM_OSC_Enable(1);
    Grf_PVTM_Fun_Start(1);
    Grf_PVTM_Set_Divider(127, 1);

    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin7,IOMUX_GPIO2A7_PMU_IDEL);
    PmuSelLowFreq(PMU_LOW_FREQ_CLK_SEL_PVTM);
    PmuSetPowerMode(PMU_POWER_MODE_POWER_DOWN_VD_LOGIC |
                    PMU_POWER_MODE_USE_LOW_FREQ_EN   |
                    PMU_POWER_MODE_OSC_POWER_DOWN_EN |
                    PMU_POWER_MODE_PLL_POWER_DOWN_EN |
                    PMU_POWER_MODE_EN);

    DelayMs(1000);
    __WFI();
    while(1);
    #endif

    __WFI();

    VDD_VoltageAdjust_Set(SCU_DCOUT_120);
    Delay100cyc(500);

    FREQ_ExitModule(FREQ_IDLE);
    FREQ_EnterModule(FREQ_BLON);        //加快唤醒速度
    SendMsg(MSG_SYS_RESUME);

    //clockgate on
    //...

    //Gpio Resume
    Gpio_DisableInt(GPIO_CH2,GPIOPortA_Pin5);
    GPIOIsrUnRegister(GPIO_CH2,GPIOPortA_Pin5);
    Gpio_Init();

    Codec_Resume();
    SysTickEnable();

#ifdef _WATCH_DOG_
    WatchDogInitial();
#endif
    DEBUG("Exit");
}

/*
********************************************************************************
*
*                         End of OsHook.c
*
********************************************************************************
*/

