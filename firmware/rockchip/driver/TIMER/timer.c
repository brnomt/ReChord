/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   Timer.c
* 
* Description:  C program template
*
* History:      <author>          <time>        <version>       
*               anzhiguo        2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_TIMER_

#include "DriverInclude.h"

uint32 TimerClrInt(eTIMER_NUM timerNum)
{
    uint32 intstatus = 0;
    pTIMER_REG timerReg;
    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            timerReg->TimerIntStatus = 0x1;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);          
            timerReg->TimerIntStatus = 0x1;
            break;
        default:
            break;
    }
    return (intstatus);
}
/*----------------------------------------------------------------------
Name	: TimerSetCount
Desc	: 根据APB频率设置TIMER计数初值等参数
Params  : timerNum:TIMER编号
          usTick:TIMER定时微秒数
Return  : 0:成功
Notes   :
----------------------------------------------------------------------*/
uint32 TimerSetCount(eTIMER_NUM timerNum,uint64 usTick)
{
    pTIMER_REG timerReg;
    uint64 loadCount = 0;
    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }  
    
   // printf("\nTimer %d SetCount\n", timerNum);
    //loadCount = 24*usTick;//24M clk 
    loadCount = usTick;
    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;           
        default:
            break;
    }
    
    timerReg->TimerLoadCount0 = (loadCount & 0xffffffff );      //load the init count value low 32 bits
    timerReg->TimerLoadCount1 = ((loadCount>>32) & 0xffffffff );//load the init count value high 32 bits
    #if 1
    //DEBUG("timerReg->TimerLoadCount0 = %d",timerReg->TimerLoadCount0);
	//DEBUG("timerReg->TimerLoadCount1 = %d",timerReg->TimerLoadCount1);

	//DEBUG("timerReg->TimerControlReg = %d",timerReg->TimerControlReg);
	//DEBUG("timerReg->TimerIntStatus = %d",timerReg->TimerIntStatus);
	#endif
    	
    return(0);
}

/*----------------------------------------------------------------------
Name	: TimerGetCount
Desc	: 获取TIMER的计数值
Params  : timerNum:TIMER编号
Return  : 0:成功
Notes   :
----------------------------------------------------------------------*/
uint64 TimerGetCount(eTIMER_NUM timerNum)
{
    uint32 timeCount;
    pTIMER_REG timerReg;

    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }
   // printf("\nTimer %d GetCount\n", timerNum);

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;            
        default:
            break;
    }

    return (((timerReg->TimerCurrentValue1)<<32)|timerReg->TimerCurrentValue0);
}

/*----------------------------------------------------------------------
Name	: TimerMask
Desc	: 屏蔽TIMER中断输出
Params  : timerNum:TIMER编号
Return  : 0:成功
Notes   :
----------------------------------------------------------------------*/
uint32 TimerIntMask(eTIMER_NUM timerNum)
{
    pTIMER_REG timerReg;
    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;            
        default:
            break;
    }
    timerReg->TimerControlReg &= ~TIMR_INOMASK;//timerReg->TimerControlReg |= 0x04;

    return(0);

}

/*----------------------------------------------------------------------
Name	: TimerUnmask
Desc	: 开启TIMER中断输出
Params  : timerNum:TIMER编号
Return  : 0:成功
Notes   :
----------------------------------------------------------------------*/
uint32 TimerIntUnmask(eTIMER_NUM timerNum)
{
    pTIMER_REG timerReg;
    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }
  //  printf("\nTimer %d Unmask\n", timerNum );

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;            
        default:
            break;
    }
    timerReg->TimerControlReg |= TIMR_INOMASK;//timerReg->TimerControlReg |= 0x04;
    //printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);

    return(0);

}

/*----------------------------------------------------------------------
Name	: TimerInit
Desc	: TIMER初始化
Params  : timerNum:TIMER编号
Return  : 0:成功
Notes   : 
----------------------------------------------------------------------*/
uint32 TimerInit(eTIMER_NUM timerNum,eTIMER_MODE mode)
{
    pTIMER_REG timerReg;

    
    if((timerNum >= TIMER_MAX) || (mode >= TIMER_MODE_MAX))
    {
        return (-1);
    }
  //  printf("\nTimer %d initial\n", timerNum );
//    g_timerIRQ[timerNum] = 0;

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;            
        default:
            break;
    }

	timerReg->TimerControlReg = TIMR_IMASK | (mode << 1);     //mask interrupt ;free running
	//printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);
    return(0);
}

/*----------------------------------------------------------------------
Name	: TimerStart
Desc	: 启动一个TIMER
Params  : timerNum:TIMER编号
Return  : 0:成功;-1:失败
Notes   : 直接调用该函数就可以启动一个计数器，需要在INTC初始化之后
----------------------------------------------------------------------*/
uint32 TimerStart(eTIMER_NUM timerNum)
{
    pTIMER_REG timerReg;
    
    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }
    
   // printf("\nTimer %d Start\n", timerNum );

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;          
        default:
            break;
    }
    //printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);

    timerReg->TimerControlReg |= TIMR_ENABLE;
    //printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);

    //printf("timerReg->TimerCurrentValue0 = %d\n",timerReg->TimerCurrentValue0);
	//printf("timerReg->TimerCurrentValue1 = %d\n",timerReg->TimerCurrentValue1);
    
    return(0);
}

/*----------------------------------------------------------------------
Name	: TimerStop
Desc	: 停止一个TIMER
Params  : timerNum:TIMER编号
Return  : 0:成功
Notes   :
----------------------------------------------------------------------*/
uint32 TimerStop(eTIMER_NUM timerNum)
{
    pTIMER_REG timerReg;
    
    if(timerNum >= TIMER_MAX)
    {
        return (-1);
    }
   // printf("\nTimer %d Stop\n", timerNum );

    switch (timerNum)
    {
        case TIMER0:
            timerReg = (pTIMER_REG)TIMER_BASE;
            break;
        case TIMER1:
            timerReg = (pTIMER_REG)(TIMER_BASE + 0x20);
            break;          
        default:
            break;
    }
    timerReg->TimerControlReg = TIMR_DISABLE;
	//printf("\nTimerControlReg = %d\n",timerReg->TimerControlReg);

    return(0);
}
_ATTR_DRIVER_DATA_ uint64 TimerusTickBak = 0;
_ATTR_DRIVER_DATA_ uint64 TimernsTickBak = 0;
/*
--------------------------------------------------------------------------------
  Function name : TimerEnable(void)
  Author        : anzhiguo
  Description   : enable timer
                  
  Input         : null
                  
  Return        : null

  History:     <author>         <time>         <version>       
               anzhiguo     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
void TimerEnable(eTIMER_NUM timerNum)
{
    TimerStart(timerNum);
}
/*
--------------------------------------------------------------------------------
  Function name : TimerDisable(void)
  Author        : anzhiguo
  Description   : disable timer
                  
  Input         : null
                  
  Return        : null

  History:     <author>         <time>         <version>       
                anzhiguo     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
void TimerDisable(eTIMER_NUM timerNum)
{
    TimerStop(timerNum);      //disable timer
}
//#endif

/*
--------------------------------------------------------------------------------
  Function name : TimerPeriodSet()
  Author        : anzhiguo
  Description   : set timer timing time,
                  
  Input         : usTick -- timer timing，unit:us
                  PCLK   -- current apb bus frequency unit:MHz
                  
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009-1-14         Ver1.0
  desc:        usTick * PCLK <= 0x00ffffff
--------------------------------------------------------------------------------
*/

//_ATTR_DRIVER_CODE_
BOOL TimerPeriodSet(eTIMER_NUM timerNum,UINT64 usTick, uint32 nsTick)
{
    UINT64 count,Timeclk,count_check;
    BOOL   Retval = TRUE;
    
    TimerusTickBak = usTick;
    TimernsTickBak = nsTick;

    ScuClockGateCtr(CLK_TIMER0_GATE, 1);
    ScuClockGateCtr(CLK_TIMER1_GATE, 1);
    ScuClockGateCtr(PCLK_TIMER_GATE, 1);
    ScuSoftResetCtr(TIMER0_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(TIMER0_SRST, 0);
    ScuSoftResetCtr(TIMER1_SRST, 1);
    DelayMs(1);    
    ScuSoftResetCtr(TIMER1_SRST, 0);

    SetTimerFreq(timerNum,XIN24M,24*1000*1000);

    Timeclk  = GetTimerFreq(timerNum);
    
    count =  (Timeclk * usTick + Timeclk * nsTick / 1000)/1000/1000;
    //count_check = ((count >> 32) & 0xffffffff)
    //if(count_check > 0xffffffff) 
	//{
        //count = 0xffffffff;
        //Retval = FALSE; 
	//}

   /* Disable the timer ,usermode,mask*/
    TimerDisable(timerNum);
    TimerInit(timerNum,TIMER_FREE_RUNNING);
 
    TimerSetCount(timerNum,count);
  
    TimerIntEnable(timerNum);
   /* free mode and enable the timer  ，not mask*/
    TimerEnable(timerNum);
   
    return Retval;
}

/*
--------------------------------------------------------------------------------
  Function name : TimerPeriodSet()
  Author        : anzhiguo
  Description   : get timer timing time
                  
  Input         : PCLK   -- current APB bus frequency.unit is MHZ
  Return        :        -- timer timing time unit:us

  History:     <author>         <time>         <version>       
               anzhiguo     2009-1-14         Ver1.0
  desc:        mstick max value= 0x00ffffff/clk; 
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
uint64 TimerPeriodGet(eTIMER_NUM timerNum)
{
    UINT64 count,Timeclk,count_check;
    count = TimerGetCount(timerNum);
    Timeclk  = GetTimerFreq(timerNum);

    return(count / Timeclk);
}
/*
--------------------------------------------------------------------------------
  Function name : Timer_GetCount(void)
  Author        : anzhiguo
  Description   : get current timer count value.
                  
  Input         : null
                  null
  Return        : current timer count value.

  History:     <author>         <time>         <version>       
               anzhiguo     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
UINT64 Timer_GetCount(eTIMER_NUM timerNum)
{
    UINT64 count;
    
    count = TimerGetCount(timerNum);;        
    return count;
}

/*
--------------------------------------------------------------------------------
  Function name : TimerIntEnable
  Author        : anzhiguo
  Description   : timer interrupt enable
                  
  History:     <author>         <time>         <version>       
               anzhiguo     2009-1-14         Ver1.0
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
void TimerIntEnable(eTIMER_NUM timerNum)
{
    TimerIntUnmask(timerNum);
}
/*
--------------------------------------------------------------------------------
  Function name : TimerIntDIsable
  Author        : anzhiguo
  Description   : timer disable
                  

  History:     <author>         <time>         <version>       
               anzhiguo     2009-1-14         Ver1.0
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
void TimerIntDisable(eTIMER_NUM timerNum)
{
    TimerIntMask(timerNum);
}

/*
--------------------------------------------------------------------------------
  Function name : Timer_ClearIntrFlag()
  Author        : anzhiguo
  Description   : clear timer interrupt flag
                  
  Input         : null
                  null
  Return        : 

  History:     <author>         <time>         <version>       
              anzhiguo     2009-1-14         Ver1.0
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
uint32 TimerClearIntFlag(eTIMER_NUM timerNum)
{
    UINT32 tmp;

    tmp = TimerClrInt(timerNum);

	return(tmp);
}

/*
--------------------------------------------------------------------------------
  Function name : TimerIntStateGet
  Author        : anzhiguo
  Description   : get timer interrupt status.
                  
  Return        : timerinterrupt status.

  History:     <author>         <time>         <version>       
               anzhiguo     2009-1-14         Ver1.0
  desc:       
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
UINT32 TimerIntStateGet(eTIMER_NUM timerNum)
{
    UINT32 tmp;
    
    //tmp = Timer->TimerIntStatus;
    
    return(tmp);
}


//#endif
/*
********************************************************************************
*
*                         End of timer.c
*
********************************************************************************
*/

