/*
********************************************************************************************
*
*        Copyright (c): 2015 - 2015 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Common\Driver\BB\BBSystem.c
* Owner: aaron.sun
* Date: 2015.6.5
* Time: 10:18:19
* Version: 1.0
* Desc: BB system driver
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.5     10:18:19   1.0
********************************************************************************************
*/

#define __COMMON_DRIVER_BB_BBSYSTEM_C__
#include "SysInclude.h"
#include "DriverInclude.h"
#ifdef __COMMON_DRIVER_BB_BBSYSTEM_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#ifdef _LOG_DEBUG_
extern void BBReqDebug(uint8 * buf);
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
uint32 BbSystemStartOK;
uint32 BBsystemHoldState;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
__irq void BBSystemAIsr(void);
__irq void BBDebugIsr(void);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ShutOffBBSystem
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.5
** Time: 14:23:51
*******************************************************************************/
_COMMON_DRIVER_BB_BBSYSTEM_COMMON_
COMMON API void ShutOffBBSystem(void)
{
   BBDebug();
   BbSystemStartOK = 0;

   ScuSoftResetCtr(CAL_CORE_SRST, TRUE);
   //ScuClockGateCtr(HCLK_CAL_CORE_GATE,0);
}

/*******************************************************************************
** Name: StartBBSystem
** Input:uint32 ModuleID
** Return: int32
** Owner:aaron.sun
** Date: 2015.6.5
** Time: 10:20:58
*******************************************************************************/
_COMMON_DRIVER_BB_BBSYSTEM_COMMON_
COMMON API int32 StartBBSystem(uint32 ModuleId)
{
    uint32 timeout = 200000;

    //ScuClockGateCtr(HCLK_CAL_CORE_GATE,1);

    ScuSoftResetCtr(CAL_CORE_SRST, TRUE);

    ModuleOverlay(ModuleId, MODULE_OVERLAY_ALL);
    DelayMs(5);                                 //here must delay 5ms for Code and data transfer to B Core

    if (/*(ModuleId == MODULE_ID_WMA_DECODE_BIN) ||*/(ModuleId == MODULE_ID_AAC_DECODE_BIN))
    {
        memcpy((void *)0x01020000, &chip_freq, sizeof(chip_freq_t));
    }
    else
    {
        memcpy((void *)0x01010000, &chip_freq, sizeof(chip_freq_t));
    }

    BbSystemStartOK = 0;
    ScuSoftResetCtr(CAL_CORE_SRST, FALSE);

    while(!BbSystemStartOK)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif

        BBDebug();
        //__WFI();
        DelayUs(1);
        if (--timeout == 0)
        {
            DEBUG("StartBBSystem: timeout!!!");
            break;
        }
    }
}

/*******************************************************************************
** Name: BBSystemIsr
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.5
** Time: 10:24:03
*******************************************************************************/
_COMMON_DRIVER_BB_BBSYSTEM_COMMON_
__irq COMMON FUN void BBSystemAIsr(void)
{
    uint32 Cmd;
    uint32 Data;

    MailBoxClearB2AInt(MAILBOX_ID_0,  MAILBOX_INT_0);
    Cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_0);

    switch(Cmd)
    {
        case MSGBOX_CMD_SYSTEM_START_OK:
            BbSystemStartOK = 1;
            break;
        case MSGBOX_CMD_BB_HOLD_ACK:
            BBsystemHoldState = 1;
            break;
    }

}

/*******************************************************************************
** Name: BBDebugIsr
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.5
** Time: 10:24:03
*******************************************************************************/
_COMMON_DRIVER_BB_BBSYSTEM_COMMON_
__irq COMMON FUN void BBDebugIsr(void)
{
    uint32 Cmd;
    uint32 Data;

    MailBoxClearB2AInt(MAILBOX_ID_0,  MAILBOX_INT_3);
    Cmd = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_3);

    switch(Cmd)
    {
        case MSGBOX_CMD_SYSTEM_PRINT_LOG:
            BBReqDebug((uint8 *)MailBoxReadB2AData(MAILBOX_ID_0,MAILBOX_CHANNEL_3));
            break;
    }
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: BBSystemDeInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.5
** Time: 14:25:05
*******************************************************************************/
_COMMON_DRIVER_BB_BBSYSTEM_INIT_
INIT API void BBSystemDeInit(void)
{
    //for BB system control
    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_0);
    IntUnregister(INT_ID_MAILBOX0);
    IntDisable(INT_ID_MAILBOX0);

    //for BB system Debug
    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_3);
    IntUnregister(INT_ID_MAILBOX3);
    IntDisable(INT_ID_MAILBOX3);

    ScuSoftResetCtr(MAILBOX_SRST, TRUE);
    ScuSoftResetCtr(HIGHRAM0_SRST, TRUE);

#if (FRAME_SUB_BUFFER_NUM <= 1)
    ScuSoftResetCtr(ASYNC_BRG_SRST, TRUE);
    ScuSoftResetCtr(HIGH_MATRIX_SRST, TRUE);
    ScuSoftResetCtr(HIGHRAM1_SRST, TRUE);
#endif

    ScuClockGateCtr(PCLK_MAILBOX_GATE,0);

    ScuClockGateCtr(HIRAM3_GATE,0);
    ScuClockGateCtr(HIRAM2_GATE,0);
    ScuClockGateCtr(HIRAM1_GATE,0);
    ScuClockGateCtr(HIRAM0_GATE,0);
    ScuClockGateCtr(HIRAM_INTERFACE_GATE,0);


    ScuClockGateCtr(HDRAM0_GATE,0);
    ScuClockGateCtr(HDRAM1_GATE,0);
    ScuClockGateCtr(HDRAM2_GATE,0);
    ScuClockGateCtr(HDRAM3_GATE,0);
    ScuClockGateCtr(HDRAM4_GATE,0);
    ScuClockGateCtr(HDRAM5_GATE,0);

#if (FRAME_SUB_BUFFER_NUM <= 1)
    ScuClockGateCtr(HDRAM6_GATE,0);
    ScuClockGateCtr(HDRAM7_GATE,0);
    ScuClockGateCtr(HDRAM_INTERFACE_GATE,0);

    ScuClockGateCtr(HCLK_HIGH_GATE,0);
    ScuClockGateCtr(HCLK_CAL_CORE_GATE,0);
    ScuClockGateCtr(HCLK_HIGH_MATRIX_GATE,0);
    ScuClockGateCtr(HCLK_ASYNC_BRG_GATE,0);

    PmuPdLogicPowerDown(1);
#endif

}

/*******************************************************************************
** Name: BBSystemInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.5
** Time: 10:27:18
*******************************************************************************/
_COMMON_DRIVER_BB_BBSYSTEM_INIT_
INIT API void BBSystemInit(void)
{
#if (FRAME_SUB_BUFFER_NUM <= 1)
    PmuPdLogicPowerDown(0);

    ScuClockGateCtr(HCLK_ASYNC_BRG_GATE,1);
    ScuClockGateCtr(HCLK_HIGH_MATRIX_GATE,1);
    ScuClockGateCtr(HCLK_CAL_CORE_GATE,1);
    ScuClockGateCtr(HCLK_HIGH_GATE,1);

    ScuClockGateCtr(HDRAM_INTERFACE_GATE,1);
    ScuClockGateCtr(HDRAM7_GATE,1);
    ScuClockGateCtr(HDRAM6_GATE,1);
#endif

    ScuClockGateCtr(HDRAM5_GATE,1);
    ScuClockGateCtr(HDRAM4_GATE,1);
    ScuClockGateCtr(HDRAM3_GATE,1);
    ScuClockGateCtr(HDRAM2_GATE,1);
    ScuClockGateCtr(HDRAM1_GATE,1);
    ScuClockGateCtr(HDRAM0_GATE,1);

    ScuClockGateCtr(HIRAM_INTERFACE_GATE,1);
    ScuClockGateCtr(HIRAM3_GATE,1);
    ScuClockGateCtr(HIRAM2_GATE,1);
    ScuClockGateCtr(HIRAM1_GATE,1);
    ScuClockGateCtr(HIRAM0_GATE,1);

    ScuClockGateCtr(PCLK_MAILBOX_GATE,1);

#if (FRAME_SUB_BUFFER_NUM <= 1)
    ScuSoftResetCtr(ASYNC_BRG_SRST, FALSE);
    ScuSoftResetCtr(HIGH_MATRIX_SRST, FALSE);
    ScuSoftResetCtr(HIGHRAM1_SRST, FALSE);
#endif
    ScuSoftResetCtr(HIGHRAM0_SRST, FALSE);
    ScuSoftResetCtr(MAILBOX_SRST, FALSE);

    //for BB system control
    MailBoxClearB2AInt(MAILBOX_ID_0,  MAILBOX_INT_0);
    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_0);
    IntRegister(INT_ID_MAILBOX0, (void*)BBSystemAIsr);
    IntPendingClear(INT_ID_MAILBOX0);
    IntEnable(INT_ID_MAILBOX0);

    //for BB system Debug
    MailBoxClearB2AInt(MAILBOX_ID_0,  MAILBOX_INT_3);
    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_3);
    IntRegister(INT_ID_MAILBOX3, (void*)BBDebugIsr);
    IntPendingClear(INT_ID_MAILBOX3);
    IntEnable(INT_ID_MAILBOX3);

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
