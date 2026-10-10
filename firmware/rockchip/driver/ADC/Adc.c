/*
********************************************************************************
*          Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All Rights Reserved
*
*Description: AD sampling,battery voltage check program.
*
$Header: /cvs_database/cvs_server_rk26/mp3_project/RK260x_SDK/System/Driver/Adc/Adc.c,v 1.1.1.5 2007/12/27 07:58:59 ke Exp $
$Author: ke $
$Date: 2007/12/27 07:58:59 $
$Revision: 1.1.1.5 $
********************************************************************************
*/
#define _IN_ADC_

#include "DriverInclude.h"
#include "interrupt.h"

#define ADC_SLEEP_TIME 500

uint8  AdcSleep = 0;
uint8  AdcSleepEn = 0;
uint32 AdcSleepConter = 0;

extern void AdcPowerUp(void);
extern void AdcPowerDown(void);
/*
--------------------------------------------------------------------------------
  Function name : void AdcSleepEnable(void)
  Author        : Zhengyongzhi
  Description   :

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
              anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AdcSleepEnable(void)
{
    AdcSleepEn = 1;
}

_ATTR_SYS_CODE_
void AdcSleepDisable(void)
{
    AdcSleepEn = 0;
}

_ATTR_SYS_CODE_
void AdcSleepEnter(void)
{
    #if 1//def DEEP_SLEEP
    if (AdcSleepEn)
    {
        AdcSleep = 1;
        AdcSamplingChConfig[0] = ADC_CHANEL_BATTERY;
        AdcSamplingChConfig[1] = ADC_CHANEL_BATTERY;
        AdcSamplingChConfig[2] = ADC_CHANEL_BATTERY;
        AdcSamplingChConfig[3] = ADC_CHANEL_BATTERY;
        AdcPowerDown();
    }
    #endif
}

_ATTR_SYS_CODE_
void AdcSleepExit(void)
{
    #if 1//def DEEP_SLEEP
    AdcSleep = 0;
    AdcSamplingChConfig[0] = ADC_CHANEL_KEY;
    AdcSamplingChConfig[1] = ADC_CHANEL_TEMP;
    AdcSamplingChConfig[2] = ADC_CHANEL_KEY;
    AdcSamplingChConfig[3] = ADC_CHANEL_BATTERY;
    AdcPowerUp();
    #endif
}

/*
--------------------------------------------------------------------------------
  Function name : void AdcPowerUp(void)
  Author        : Zhengyongzhi
  Description   :

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
              anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AdcPowerUp(void)
{
    Grf_Set_ADC_Vref_PowerOn(GRF_VREF_PWON);
    ScuClockGateCtr(CLK_SARADC_GATE, 1);
    ScuClockGateCtr(PCLK_SARADC_GATE, 1);
    //ScuSoftResetCtr(SARADC_SRST, 0);

	Adc->ADC_CTRL |= ADC_POWERUP ;
}

/*
--------------------------------------------------------------------------------
  Function name : void AdcPowerDown(void)
  Author        : Zhengyongzhi
  Description   :

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
              anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AdcPowerDown(void)
{
    uint32 timeout = 20;
    while((AdcFlag & ADC_FINISH) == 0)
    {
        DelayUs(10);
        timeout--;
        if (timeout == 0)
        {
            break;
        }
    }
	Adc->ADC_CTRL &= ~ADC_INT_CLEAR;
    Adc->ADC_CTRL &= ~ADC_START;
    Adc->ADC_CTRL &= ~ADC_POWERUP ;

    //ScuSoftResetCtr(SARADC_SRST, 1);
    ScuClockGateCtr(CLK_SARADC_GATE, 0);
    ScuClockGateCtr(PCLK_SARADC_GATE, 0);

    Grf_Set_ADC_Vref_PowerOn(GRF_VREF_PWOFF);
}

/*
--------------------------------------------------------------------------------
  Function name : void AdcPowerDown(void)
  Author        : Zhengyongzhi
  Description   :

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
              anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AdcChConfig(uint8  ch)
{
    uint32 temp;

    Adc->ADC_CTRL &= ~ADC_START;
	temp = Adc->ADC_CTRL & (~ADC_CH_MASK);
    Adc->ADC_CTRL = temp | ch | ADC_START;
}

/*
--------------------------------------------------------------------------------
  Function name : void AdcInit(void)
  Author        : Zhengyongzhi
  Description   : ad sampling initial

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
              anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void AdcInit(void)
{
    AdcSamplingCounter = 0;
    AdcPendCounter     = 0;

    pAdcPendIn  = AdcPendBuf;
    pAdcPendout = AdcPendBuf;
    AdcPendCnt  = 0;

    AdcFlag            = ADC_FINISH;
    AdcSamplingCh      = AdcSamplingChConfig[AdcSamplingCounter];

    SetSarAdcFreq(1000000);

    Grf_Verf_Trim_Sel(GRF_VREF_TRIM_2500);
    Grf_Verf_Vbg_Sel(5);

    AdcSleepEnable();
	AdcPowerUp();
}

/*
--------------------------------------------------------------------------------
  Function name : void AdcStart(uint8 StartCh)
  Author        : Zhengyongzhi
  Description   : start ad sampling.
  				  AD sampling is started in systick and timer interruption,if adc is processing,
  				  then suspend the channel.after finish this ad sampling, start the interrupt in
  				  interrupt service again.
  Input         : null
  Return        : null
  History:     <author>         <time>         <version>
                anzhiguo     2009-3-24         Ver1.0
  desc:          ORG //register system interruption, used for key scan and battery detect.
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void AdcStart(uint8 StartCh)
{
    uint8  ch;
    uint32 temp;
    uint32 IsEnable;

    #if 0
    ///////////////////////////////////////////////////////////////////////////////////////////
    //在非录音模式下，ADC每10ms启动一次
    //在录音模式，不会进入Sleep模式
    //进入Sleep模式之后，每隔5s检测一次，以降低功耗
    if ((++AdcSleepConter) > ADC_SLEEP_TIME)
    {
        AdcSleepConter = 0;
    }

    if (AdcSleep)
    {
        if (AdcSleepConter == 0)
        {
            AdcPowerUp();
            DEBUG("AdcPowerUp");
            return;
        }
        if (AdcSleepConter == 2)
        {
            AdcPowerDown();
            DEBUG("AdcPowerDown");
            return;
        }
        if (AdcSleepConter > 2)
        {
            return;
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////
    #endif
    if (AdcSleep)
    {
        return;
    }
    {
        if (++AdcSamplingCounter >= sizeof(AdcSamplingChConfig))
        {
            AdcSamplingCounter = 0;
        }
        ch  = AdcSamplingChConfig[AdcSamplingCounter];
    }

    DisableIntMaster();
    if ((AdcFlag & ADC_FINISH) || (AdcPendCounter > ADC_CHANEL_MAX))
    {
        AdcPendCounter = 0;

        AdcChConfig(ch);

        AdcSamplingCh  = ch;
        AdcFlag &= ~ADC_FINISH;
    }
    else
    {
        AdcPendCounter ++;

        *pAdcPendIn++ = ch;
        if (pAdcPendIn >= AdcPendBuf + ADC_PENDING_SIZE)
        {
            pAdcPendIn = AdcPendBuf;
        }

        if (++AdcPendCnt > ADC_PENDING_SIZE)
        {
            AdcPendCnt = ADC_PENDING_SIZE;
        }
    }
    EnableIntMaster();
}

/*
--------------------------------------------------------------------------------
  Function name : __irq void AdcIsr(void)
  Author        : Zhengyongzhi
  Description   : interruption of ad sampling.sampling ad data, and detect whether is channel suspended.

  Input         : null
  Return        : null
  History:     <author>         <time>         <version>
                anzhiguo     2009-3-24         Ver1.0
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
__irq void AdcIsr(void)
{
    uint8  ch;
    uint32 temp;

    DisableIntMaster();
    AdcData[AdcSamplingCh] = (UINT16)(Adc->ADC_DATA);
    Adc->ADC_CTRL &= ~ADC_INT_CLEAR;

    AdcFlag |= (0x01 << AdcSamplingCh);
    AdcFlag |= ADC_FINISH;

    if (AdcPendCnt > 0)
    {
        AdcPendCnt--;

        ch = *pAdcPendout++;
        if (pAdcPendout >= AdcPendBuf + ADC_PENDING_SIZE)
        {
            pAdcPendout = AdcPendBuf;
        }
        if (ch < ADC_CHANEL_MAX)
        {
            AdcChConfig(ch);

            AdcSamplingCh  = ch;
            AdcFlag &= ~ADC_FINISH;
        }
    }

    EnableIntMaster();
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 CheckAdcState(uint8 ch)
  Author        : Zhengyongzhi
  Description   :

  Input         : null
  Return        : null
  History:     <author>         <time>         <version>
                anzhiguo     2009-3-24         Ver1.0
  desc:          ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
uint32 CheckAdcState(uint8 ch)
{
    uint32 temp;

    temp = (AdcFlag & (0x01 << ch));

    if (temp != 0)
    {
        AdcFlag &= ~((uint32)0x01 << ch);
    }

    return(temp);
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 GetAdcData(uint8 ch)
  Author        : Zhengyongzhi
  Description   :

  Input         : null
  Return        : null
  History:     <author>         <time>         <version>
                anzhiguo     2009-3-24         Ver1.0
  desc:          ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
uint32 GetAdcData(uint8 ch)
{
    return(AdcData[ch]);
}

/*
********************************************************************************
*
*                         End of Adc.c
*
********************************************************************************
*/

