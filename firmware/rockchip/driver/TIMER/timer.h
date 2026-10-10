/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   timer.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#undef  EXT
#ifdef  _IN_TIMER_
#define EXT
#else
#define EXT extern
#endif


extern _ATTR_DRIVER_DATA_ uint64 TimerusTickBak;
extern _ATTR_DRIVER_DATA_ uint64 TimernsTickBak;

/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/

extern void   TimerEnable(eTIMER_NUM timerNum);
extern void   TimerDisable(eTIMER_NUM timerNum);
extern uint64 TimerPeriodGet(eTIMER_NUM timerNum);
extern UINT64 Timer_GetCount(eTIMER_NUM timerNum);
extern void   TimerIntEnable(eTIMER_NUM timerNum);
extern void   TimerIntDisable(eTIMER_NUM timerNum);
extern uint32 TimerClearIntFlag(eTIMER_NUM timerNum);
extern UINT32 TimerIntStateGet(eTIMER_NUM timerNum);



BOOL TimerPeriodSet(eTIMER_NUM timerNum ,UINT64 usTick, uint32 nsTick);

/*
********************************************************************************
*
*                         End of timer.h
*
********************************************************************************
*/
#endif
