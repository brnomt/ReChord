/*
********************************************************************************
*
*                Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All rights reserved.
*
* FileName: ..\Common\Driver\WatchDog\WathcDog.c
* Owner: WathcDog.c
* Date: 2015.3.2
* Time: 13:55:12
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
*   wangping    2015-03-02
********************************************************************************
*/

/*
*-------------------------------------------------------------------------------
*
*                             #include define
*
*-------------------------------------------------------------------------------

*/

#include "SysInclude.h"
#include "WatchDog.h"

/*
*-------------------------------------------------------------------------------
*
*                              #define / #typedef define
*
*-------------------------------------------------------------------------------
*/



/*
*-------------------------------------------------------------------------------
*
*                              local variable define
*
*-------------------------------------------------------------------------------
*/



/*
*-------------------------------------------------------------------------------
*
*                              global variable define
*
*-------------------------------------------------------------------------------
*/



/*
*-------------------------------------------------------------------------------
*
*                              local function declare
*
*-------------------------------------------------------------------------------
*/
rk_err_t WatchDogStart(void);
rk_err_t WatchDogStop(void);
/*
*-------------------------------------------------------------------------------
*
*                              local function define
*
*-------------------------------------------------------------------------------
*/




/*
*-------------------------------------------------------------------------------
*
*                              API function define
*
*-------------------------------------------------------------------------------
*/


/******************************************************************************
 * WatchDogInit -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:27:53
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_err_t WatchDogInit(WdtRespMode_t mode , WdtRstPluseLenth_t RstPluse, WdtTimeoutPeriod_t period)
{
    ScuClockGateCtr(PCLK_WDT_GATE, TRUE);

    ScuSoftResetCtr(WDT_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(WDT_SRST, 0);

    //printf("\n ccvr = %d\n",WatchDogGetCurrentValue());
    Wdt->WDT_CR = (mode << WDT_RESP_MODE_OFFSET) | (RstPluse << WDT_RST_PLUSE_LENGT_OFFSET);

    WatchDogSetPeriod(period);
    WatchDogReload();
    //printf("00 CCRV = %d\n",WatchDogGetCurrentValue());
    //WatchDogStart();

    return OK;
}

/******************************************************************************
 * WatchDogDeInit -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:27:45
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_err_t WatchDogDeInit(void)
{
    WatchDogStop();

    ScuClockGateCtr(PCLK_WDT_GATE, FALSE);
    return OK;
}

/******************************************************************************
 * WatchDogStart -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:27:37
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_err_t WatchDogStart(void)
{
    int backup = Wdt->WDT_CR;
    Wdt->WDT_CR = 1 |backup;
    return OK;
}

/******************************************************************************
 * WatchDogStop -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:28:09
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_err_t WatchDogStop(void)
{
    Wdt->WDT_CR &= ~1;
    return OK;
}


/******************************************************************************
 * WatchDogReload -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:28:15
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_err_t WatchDogReload(void)
{
    Wdt->WDT_CRR = 0x76;
    return OK;
}



/******************************************************************************
 * WatchDogIsr -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:29:09
 * --------------------
 ******************************************************************************/
//_ATTR_SYS_CODE_
//void WatchDogIsr(void)
//{
//    return ;
//}


/******************************************************************************
 * WatchDogClearIntFlag -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:29:14
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_err_t WatchDogClearIntFlag(void)
{
    uint32 eoi;
    eoi = Wdt->WDT_EOI;
    return eoi;
}

/******************************************************************************
 * WatchDogSetPeriod -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:29:33
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_err_t WatchDogSetPeriod(WdtTimeoutPeriod_t period)
{
    Wdt->WDT_TORR = period;
    return OK;
}

/******************************************************************************
 * WatchDogGetCurrentValue -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:29:38
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_size_t WatchDogGetCurrentValue(void)
{
    return Wdt->WDT_CCVR;
}

/******************************************************************************
 * WatchDogGetStat -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:29:43
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
UINT32 WatchDogGetStat(void)
{
    return (Wdt->WDT_STAT & 0x01);
}

/******************************************************************************
 * WatchDogEnableInt -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:29:49
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_size_t WatchDogEnableInt(void)
{
    IntEnable(INT_ID_WDT);
}




/******************************************************************************
 * WatchDogDisableInt -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:30:01
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_size_t WatchDogDisableInt(void)
{
    IntDisable(INT_ID_WDT);
}


/******************************************************************************
 * WatchDogIntRegister -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.3.2  written
 * Time : 15:30:07
 * --------------------
 ******************************************************************************/
_ATTR_SYS_CODE_
rk_size_t WatchDogIntRegister(ExecFunPtr(*pfnHandler)(void))
{
    IntRegister(INT_ID_WDT, pfnHandler);
}


_ATTR_SYS_CODE_
void WatchDogIsr(void)
{
    printf(" \n>>>### watch dog isr coming<<<### \n");
    #ifdef _USB_HOST_
    USBHost_Reinit();
    #endif
    Grf_NOC_Remap_Sel(NOC_BOOT_ROM);
    //printf("\n CCRV = %d",WatchDogGetCurrentValue());

    while(1);
    //printf("watch dog isr coming\n");
}

_ATTR_SYS_CODE_
void WatchDogInitial()
{
    IntPendingClear(INT_ID_WDT);
    WatchDogIntRegister(WatchDogIsr);
    WatchDogInit(RESP_MODE_INT_RESET, PCLK_CYCLES_64, PERIOD_RANGE_0X1FFFFFFF);
    WatchDogEnableInt();
    WatchDogStart();
}

