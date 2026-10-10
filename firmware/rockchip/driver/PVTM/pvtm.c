/*
********************************************************************************
*                       Copyright (c) 2015 RockChips
*                         All rights reserved.
*
* File Name£º   pvtm.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                                           
*    desc:    ORG.
********************************************************************************
*/
#include "DriverInclude.h"


void PVTM_Init()
{
    ScuClockGateCtr(PVTM_CLK_GATE, 1);
} 

void PVTM_Deinit()
{
    ScuClockGateCtr(PVTM_CLK_GATE, 0);
} 

void PVTM_Start()
{
    PVTM_Init();
    Grf_PVTM_OSC_Enable(TRUE);
    
    Grf_PVTM_Fun_Start(FALSE);
    DelayUs(1);
    Grf_PVTM_Fun_Start(TRUE);
    DelayUs(1);
}

uint32 PVTM_Get_Cnt()
{
    bool cal_done;
    
    cal_done = Grf_PVTM_Is_FreqCal_Done();

    if( cal_done ) 
        Grf_PVTM_Get_Cal_Cnt();
}

void  PVTM_Set_CalCnt(uint32 counter)
{ 
    Grf_PVTM_Set_Cal_Cnt(counter);
}

/*
********************************************************************************
*
*                         End of Scu.c
*
********************************************************************************
*/ 


