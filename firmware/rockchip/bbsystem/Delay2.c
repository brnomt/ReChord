/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Delay.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_DELAY_  

#include "SysInclude.h"
#include "driver/CRU/cru.h"   /* chip_freq_t */
extern chip_freq_t chip_freq2;
/*
--------------------------------------------------------------------------------
  Function name : void Delay100cyc(UINT16 count)
  Author        : ZHengYongzhi
  Description   : delay 10 cycle.
                  one while period has x time of cycles.
                  
  Input         : 
  Return        : TRUE

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void Delay10cyc2(UINT32 count)
{
    uint32 cycle;
    
    cycle = count * 2;
    
    while(--cycle);
}

/*
--------------------------------------------------------------------------------
  Function name : void Delay100cyc(UINT16 count)
  Author        : ZHengYongzhi
  Description   : delay 100 time cycle.
                  
  Input         : 
  Return        : TRUE

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void Delay100cyc2(UINT32 count)
{
    uint32 cycle;
    
    cycle = count * 20;
    
    while(--cycle);
}
//#endif

/*
--------------------------------------------------------------------------------
  Function name : void DelayMs_nops(UINT32 msec)
  Author        : yangwenjie
  Description   : how many us that software will to delay 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void ASMDelay2(uint32 i)
{
    __asm__ volatile("subs r0, r0, #1; bhi .-4; bx lr");
}
/*
--------------------------------------------------------------------------------
  Function name : GetTimeHMS(UINT32 TempSec, UINT16 *pHour, UINT16 *pMin, UINT16 *pSec)
  Author        : ZhengYongzhi
  Description   : get the hour,minute,second according to input second.
                  
  Input         : 
  Return        : TRUE

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void GetTimeHMS2(uint32 TempSec, uint16 *pHour, uint8 *pMin, uint8 *pSec)
{
    *pHour = (uint16)(TempSec / 3600);

    *pMin  = (uint8)((TempSec % 3600) / 60);

    *pSec  = (uint8)(TempSec % 60);
}
//#endif
/*
********************************************************************************
*
*                         End of Delay.c
*
********************************************************************************
*/

