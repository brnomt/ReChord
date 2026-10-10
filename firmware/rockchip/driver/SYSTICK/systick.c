/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   systick.c
* 
* Description: 
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-7-21          1.0
*    desc:    
********************************************************************************
*/
#define _IN_SYSTICK_

#include "DriverInclude.h"

#if 0
uint32 time,cnt,timestart,timeend;

void time_cnt_start()
{
    timestart = nvic->SysTick.value;
    cnt = SysTickCounter;    
}

void time_cnt_end()
{
    timeend =  nvic->SysTick.value;
    time = (SysTickCounter - cnt) * (nvic->SysTick.Reload + 1) + timestart + (nvic->SysTick.Reload - timeend + 1);
    printf("time = %d\n", time);
}
#endif


/*
--------------------------------------------------------------------------------
  Function name : void SysTickClkSourceSet(UINT32 Source)
  Author        : ZHengYongzhi
  Description   : Select The SysTick clock source.
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:      select the source of systick 
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVER_CODE_
_ATTR_DRIVERLIB_CODE_
void SysTickClkSourceSet(UINT32 Source)
{
    // Select SysTick Clock Source
   
    if(Source) 
	{
        nvic->SysTick.Ctrl |= NVIC_SYSTICKCTRL_CLKIN;
    } 
	else
	{
        nvic->SysTick.Ctrl &= ~NVIC_SYSTICKCTRL_CLKIN;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : BOOL SysTickClkSourceGet(void)
  Author        : ZHengYongzhi
  Description:    Get the SysTick clock source.
                  
  Input         : 
  Return        :true:inter clock, false:external clock. 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
BOOL SysTickClkSourceGet(void)
{    
    // Disable SysTick.
    return((BOOL)(nvic->SysTick.Ctrl & NVIC_SYSTICKCTRL_CLKIN));
}

/*
--------------------------------------------------------------------------------
  Function name : void SysTickEnable(void)
  Author        : ZHengYongzhi
  Description   : Enables the SysTick counter.This will start the SysTick counter.
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:       enable systick and generate abnormal interrupt.  
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void SysTickEnable(void)
{
    // Enable SysTick, Select Internal Clock.
    //
    nvic->SysTick.Ctrl |= (NVIC_SYSTICKCTRL_ENABLE | NVIC_SYSTICKCTRL_TICKINT);
}


/*
--------------------------------------------------------------------------------
  Function name : void SysTickDisable(void)
  Author        : ZHengYongzhi
  Description:    Disables the SysTick counter.This will stop the SysTick counter.
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void SysTickDisable(void)
{
    // Disable SysTick.
    //
    nvic->SysTick.Ctrl &= ~NVIC_SYSTICKCTRL_ENABLE;
}


/*
--------------------------------------------------------------------------------
  Function name : void SysTickPeriodSet(UINT32 ulPeriod)
  Author        : ZHengYongzhi
  Description   : Sets the period of the SysTick counter.
                   
  Input         : mstick -- systick's cycle number,unit:ms
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void SysTickPeriodSet(UINT32 mstick )
{
    UINT32 ulPeriod, pclk;

    if(SysTickClkSourceGet())//internal clock,that is fclk
    {
        pclk = GetHclkSysCoreFreq();

        ulPeriod = mstick * pclk / 1000;
        nvic->SysTick.Reload = ulPeriod - 1;
    }
    else
    {
        pclk = GetStclkSysCoreFreq();

        ulPeriod = mstick * pclk / 1000;
        nvic->SysTick.Reload = ulPeriod - 1;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysTickPeriodGet(void)
  Author        : ZHengYongzhi
  Description   : Gets the period of the SysTick counter.
                   
  Input         :           
  Return        : Returns the period of the SysTick counter.

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 SysTickPeriodGet(void)
{
    // Return the period of the SysTick counter.
    //
    return(nvic->SysTick.Reload + 1);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysTickValueGet(void)
  Author        : ZHengYongzhi
  Description   : Gets the current value of the SysTick counter.
                   
  Input         : 
  Return        : Returns the current value of the SysTick counter.

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 SysTickValueGet(void)
{
    //
    // Return the current value of the SysTick counter.
    return(nvic->SysTick.Value);
}

/*
--------------------------------------------------------------------------------
  Function name : BOOL SysTickRefClockCheck(void)
  Author        : ZHengYongzhi
  Description   : Check the reference clock is provided or not.
                   
  Input         : 
  Return        : TRUE -- provided; FALSE -- not provided;

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
BOOL SysTickRefClockCheck(void)
{
    // Return the current value of the SysTick counter.
    //
    return((BOOL)!(nvic->SysTick.Calibration & NVIC_SYSTICKCALIB_SKEW));
}

/*
--------------------------------------------------------------------------------
  Function name : BOOL SysTickCalibrationCheck(void)
  Author        : ZHengYongzhi
  Description   : the calibration value is exactly 10ms or not.
                   
  Input         : 
  Return        : TRUE -- exactly 10ms; FALSE -- not exactly 10ms;

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
BOOL SysTickCalibrationCheck(void)
{
    // Return the current value of the SysTick counter.
    //
    return((BOOL)!(nvic->SysTick.Calibration & NVIC_SYSTICKCALIB_SKEW));
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysTickCalibrationTenMsGet(void)
  Author        : ZHengYongzhi
  Description   : This value is the Reload value to use for 10ms timing.Depend-
                  ing on the value of SKEW, this might be exactly 10ms or might
                  be the closest value.
  Input         : 
  Return        : If Return 0, then the calibration value is not known.

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 SysTickCalibrationTenMsGet(void)
{
    
    // Return the current value of the SysTick counter.
   
    return(nvic->SysTick.Calibration & NVIC_SYSTICKCALIB_TEMMS_MASK);
}


/*
********************************************************************************
*
*                         End of systick.c
*
********************************************************************************
*/

