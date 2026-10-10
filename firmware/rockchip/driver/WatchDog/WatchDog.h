/*
********************************************************************************
*
*				  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*							   All rights reserved.
*
* FileName: ..\Common\Driver\WatchDog\WathcDog.h
* Owner: mailboxTest.c
* Date: 2015.3.2
* Time: 11:34:15
* Desc: 
* History:
*	 <author>	 <date> 	  <time>	 <version>	   <Desc>
*	 
********************************************************************************
*/

#ifndef __WATCHDOG_WATCHDOG_H__
#define __WATCHDOG_WATCHDOG_H__

#include "Hw_WatchDog.h"
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
extern rk_err_t WatchDogSetPeriod(WdtTimeoutPeriod_t period);
extern rk_size_t WatchDogGetCurrentValue(void);
extern rk_err_t WatchDogClearIntFlag(void);
extern rk_err_t WatchDogReload(void);
extern rk_err_t WatchDogStart(void);
extern rk_err_t WatchDogInit(WdtRespMode_t mode , WdtRstPluseLenth_t RstPluse, WdtTimeoutPeriod_t period);
extern rk_err_t WatchDogDeInit(void);
#endif

