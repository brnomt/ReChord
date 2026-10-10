/*
********************************************************************************
*
*				  Copyright (c): 2015 - 2015 + 5, mailboxTest.c
*							   All rights reserved.
*
* FileName: ..\Common\Driver\WatchDog\Hw_WathcDog.h
* Owner: watchdog.c
* Date: 2015.3.2
* Time: 11:30:40
* Desc: 
* History:
*	 <author>	 <date> 	  <time>	 <version>	   <Desc>
*	 wangping    2015-03-02
********************************************************************************
*/

#ifndef __WATCHDOG_HW_WATCHDOG_H__
#define __WATCHDOG_HW_WATCHDOG_H__

/*
*-------------------------------------------------------------------------------
*
*							   #include define									 
*
*-------------------------------------------------------------------------------
*/



/*
*-------------------------------------------------------------------------------
*
*							   #define / #typedef define						 
*
*-------------------------------------------------------------------------------
*/

typedef __packed struct _WATCHDOG_REG
{
    UINT32 WDT_CR;
    UINT32 WDT_TORR; /*This field is used to select the timeout period from
                       which the watchdog counter restarts. A change of the
                        timeout period takes effect only after the next
                        counter restart (kick).
                        The range of values available for a 32-bit watchdog*/ 
    UINT32 WDT_CCVR; /*Current counter value
                        This register, when read, is the current value of the
                        internal counter. This value is read coherently when
                        ever it is read*/
    UINT32 WDT_CRR;  /*This register is used to restart the WDT counter. As a
                        safety feature to prevent accidental restarts, the
                        value 0x76 must be written. A restart also clears the
                        WDT interrupt. Reading this register returns zero.*/
                       
    UINT32 WDT_STAT; /*This register shows the interrupt status of the WDT.
                        1¡¯b1: Interrupt is active regardless of polarity.
                        1¡¯b0: Interrupt is inactive.*/
    UINT32 WDT_EOI;
  
}WDTReg_t, *pWDTReg_t;


#define Wdt  ((WDTReg_t *) WDT_BASE)




typedef enum WdtRstPluseLenth
{
/*This is used to select the number of pclk cycles
for which the system reset stays asserted.*/    
    PCLK_CYCLES_2,
    PCLK_CYCLES_4,
    PCLK_CYCLES_8,
    PCLK_CYCLES_16,
    PCLK_CYCLES_32,
    PCLK_CYCLES_64,
    PCLK_CYCLES_128,
    PCLK_CYCLES_256,
   
}WdtRstPluseLenth_t;

#define WDT_RST_PLUSE_LENGT_OFFSET   2

typedef enum WdtRespMode
{
    RESP_MODE_RESET,/*Generate a system reset */
   
    RESP_MODE_INT_RESET,/*First generate an interrupt and if it is not
                        cleared by the time a second timeout occurs then
                       generate a system reset*/
  
}WdtRespMode_t;

#define WDT_RESP_MODE_OFFSET         1

typedef enum WdtTimeoutPeriod
{
/*This field is used to select the timeout period from
which the watchdog counter restarts. A change of the
timeout period takes effect only after the next
counter restart (kick).
The range of values available for a 32-bit watchdog*/ 

   PERIOD_RANGE_0X0000FFFF, 
   PERIOD_RANGE_0X0001FFFF, 
   PERIOD_RANGE_0X0003FFFF, 
   PERIOD_RANGE_0X0007FFFF, 
   
   PERIOD_RANGE_0X000FFFFF, 
   PERIOD_RANGE_0X001FFFFF, 
   PERIOD_RANGE_0X003FFFFF, 
   PERIOD_RANGE_0X007FFFFF,
   
   PERIOD_RANGE_0X00FFFFFF, 
   PERIOD_RANGE_0X01FFFFFF, 
   PERIOD_RANGE_0X03FFFFFF, 
   PERIOD_RANGE_0X07FFFFFF, 
   
   PERIOD_RANGE_0X0FFFFFFF, 
   PERIOD_RANGE_0X1FFFFFFF, 
   PERIOD_RANGE_0X3FFFFFFF,    
   PERIOD_RANGE_0X7FFFFFFF,  


}WdtTimeoutPeriod_t;
/*
*-------------------------------------------------------------------------------
*
*							   global variable declare							 
*
*-------------------------------------------------------------------------------
*/



/*
*-------------------------------------------------------------------------------
*
*							   API Declare          							 
*
*-------------------------------------------------------------------------------
*/



#endif

