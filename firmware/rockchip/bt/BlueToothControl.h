/*
********************************************************************************
*                   Copyright (c) 2009,Rockchip
*                         All rights reserved.
*
* File Name：   BlueTooth.h
*
* Description:
*
* History:      <author>          <time>        <version>
*    desc:    ORG.
********************************************************************************
*/
//#include "Thread.h"
//#include "typedef.h"

#include "audio_globals.h"
#include "SysConfig.h"
#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#undef  EXT
#ifdef _IN_BLUETOOTH_
#define EXT
#else
#define EXT extern
#endif

/******************************************************************************/
/*                                                                                                                                         */
/*                          Macro Define                                                                                              */
/*                                                                                                                                         */
/******************************************************************************/
//section define
#define _ATTR_BLUETOOTHCONTROL_CODE_     
#define _ATTR_BLUETOOTHCONTROL_DATA_     
#define _ATTR_BLUETOOTHCONTROL_BSS_      

#define _ATTR_BLUETOOTHAUDIO_CODE_     
#define _ATTR_BLUETOOTHAUDIO_DATA_     
#define _ATTR_BLUETOOTHAUDIO_BSS_      

#define _ATTR_BLUETOOTHPHONE_CODE_     
#define _ATTR_BLUETOOTHPHONE_DATA_     
#define _ATTR_BLUETOOTHPHONE_BSS_      


#define _ATTR_BLUETOOTHVOICENOTIFY_CODE_     
#define _ATTR_BLUETOOTHVOICENOTIFY_DATA_     
#define _ATTR_BLUETOOTHVOICENOTIFY_BSS_      


#define BT_A2DP_PLAY        ((uint32)0)
#define BT_A2DP_PLAY_PAUSE  ((uint32)1)
#define BT_A2DP_VOL_INC     ((uint32)2)
#define BT_A2DP_VOL_DEC     ((uint32)3)
#define BT_A2DP_NEXT        ((uint32)4)
#define BT_A2DP_PREV        ((uint32)5)

#define BT_HFP_PLAY         ((uint32)6)
#define BT_HFP_STOP         ((uint32)7)
#define BT_HFP_REFUSE       ((uint32)8)
#define BT_HFP_LAST_RECALL  ((uint32)9)
#define BT_HFP_SWITCH       ((uint32)10)
#define BT_RESET_CONNECT_TABLE  ((uint32)11)
_ATTR_BLUETOOTHCONTROL_BSS_    EXT UINT32              OPP_CurrentLen;
_ATTR_BLUETOOTHCONTROL_BSS_    EXT UINT32              OPP_TotalLen;
/******************************************************************************/
/*                                                                                                                                         */
/*                         Struct define                                                                                            */
/*                                                                                                                                         */
/******************************************************************************/
//define the play status.
typedef enum
{
    BTAUDIO_STATE_STOP,
    BTAUDIO_STATE_PLAY,
    BTAUDIO_STATE_PAUSE

} BTAUDIOSTATE;

typedef enum
{
    BT_AD_STATE_STOP,
    BT_AD_STATE_WORK,

} BT_AD_STATE;


//define the play status.
typedef enum
{
    AVCRP_STATE_PAUSE,
    AVCRP_STATE_PLAY,
    AVCRP_STATE_STOP

} AVCRPCTRLSTATE;


typedef enum
{
    BT_CONNECTING,
    BT_CONNECTED,

    BT_PAIRING,
    BT_PAIRED,

    BT_A2DP,
    BT_SCO_CONNECTED,
    BT_ACL_CONNECTED,
    BT_CALL,
    BT_CALL_SETUP,
    BT_CALL_PLAYING,
    BT_A2DP_CONNECTED,
    BT_A2DP_PLAYING,
    BT_A2DP_PLAYING_HOLD,
    BT_A2DP_RINGING,  //测试通过A2DP响铃的情况
    BT_A2DP_STREAMING,
    BT_SCO_MUSIC_PLAY,
    BT_SCO_MUSIC_PLAY_HOLD,
    BT_HFP_CONNECTED,
    BT_IN_SLEEP,
    BT_WAIT_PALY_NOTIFY,
    BT_WAIT_A2DP_AVRCP_CONNECTED,
    BT_AVRCP_CONNECTED,
    BT_CC2564_INIT_COMPLETE,
    BT_WAIT_PROC_CONNECT_REQ,
    BT_CONNECTE_ERR,
    BT_AUTO_CONNCET_A2DP,
    BT_AUTO_CONNCET_HFP,
    BT_WAIT_HFP_CONNECT,
    BT_NFC_INT_LOCK,
    BT_GET_POWER_EVENT,
    BT_NFC_WAIT_CONNECTING,

} BTSTATE;

#define CHECK_BTSTATE(x, State)  ((x >> State) & 0x00000001)
#define SET_BTSTATE(x, State)    ( x |=(1<<State) )
#define CLEAR_BTSTATE(x, State)  ( x &= ~(1<<State))


/******************************************************************************/
/*                                                                                                                                         */
/*                          Variable Define                                                                                           */
/*                                                                                                                                         */
/******************************************************************************/


/******************************************************************************/
/*                                                                                                                                         */
/*                          Function Define                                                                                          */
/*                                                                                                                                         */
/******************************************************************************/
extern void BlueToothControlStart(uint32 controlID);
extern void BlueToothControlTimeOut(void);
extern void BlueToothControlInit(void *pArg);
extern UINT32 BlueToothControlService(void);
extern void BlueToothControlDeInit(void);
extern void BlueToothGetStatus(void);




/******************************************************************************/
/*                                                                                                                                         */
/*                    Description:  Thread sturcture definition                                                                */
/*                                                                                                                                         */
/******************************************************************************/
#ifdef _IN_BLUETOOTH_

_ATTR_BLUETOOTHCONTROL_DATA_ EXT THREAD BlueToothThread =
{

    NULL,
    NULL,

    BlueToothControlInit,
    BlueToothControlService,
    BlueToothControlDeInit,

    NULL
};
#else
_ATTR_BLUETOOTHCONTROL_DATA_ EXT THREAD BlueToothThread;

#endif
/*
********************************************************************************
*
*                         End of BlueTooth.h
*
********************************************************************************
*/
#endif

