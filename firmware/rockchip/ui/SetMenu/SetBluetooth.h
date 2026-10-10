/*
********************************************************************************
*                   Copyright (C),2004-2016, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  SetBluetooth.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             chad.ma
*    desc:    ORG.
********************************************************************************
*/
#ifndef _SETBLUETOOTH_H_
#define _SETBLUETOOTH_H_

#undef  EXT
#ifdef  _SETBT_IN_
#define EXT
#else
#define EXT extern
#endif
/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                      */
/*                                                                            */
/******************************************************************************/

// bt permanent code.
#define _ATTR_SYS_SET_BT_CODE_         
#define _ATTR_SYS_SET_BT_DATA_         
#define _ATTR_SYS_SET_BT_BSS_          

/*
--------------------------------------------------------------------------------

                ENUMERATIONS AND STRUCTURES

--------------------------------------------------------------------------------
*/
typedef struct _BTSTATIONITEM_STRUCT
{
    struct _BTSTATIONITEM_STRUCT *pPrev;
    struct _BTSTATIONITEM_STRUCT *pNext;
    UINT16 LongFileName[33];
} BTSTATIONITEM;

typedef struct _BTSTATIONLIST_TREE_STRUCT
{
    UINT16  CurId;          //current force item index in all items.
    UINT16  KeyCounter;     //current force item index in current screen.
    UINT16  PreCounter;     //last force item index in current screen.
    UINT16  DispTotalItem;  //total item
    BTSTATIONITEM * pBTBro;
} BTSTATIONLIST_TREE;

typedef enum
{
    //system message
    BT_WIN_STATUS_IDLE = (UINT8)0x00,
    BT_WIN_STATUS_SCAN,
    BT_WIN_STATUS_CONNECTING,
    BT_WIN_STATUS_DISCONNECT_CUR_DEV, //断开当前连接
    BT_WIN_STATUS_CONNECT_NEW_DEV, //  连接一个新设备
    BT_WIN_STATUS_DELETE_DEV,       //删除一个设备
    BT_WIN_STATUS_BT_OFF_ING,       //正在关闭
    BT_WIN_STATUS_BT_GO_TO_OFF,
    BT_WIN_STATUS_ABORT_CONNECTING,
    BT_WIN_STATUS_HCI_CONNECT_CANNELING,
}BT_WIN_STATUS;

typedef enum
{
    //system message
    BT_WIN_SUB_STATUS_IDLE = (UINT8)0x00,
    BT_WIN_SUB_STATUS_PIN_REQ,
    BT_WIN_SUB_STATUS_CONFIRMATION_REQ,
}BT_WIN_CONNETING_SUB_STATUS;

typedef enum
{
    BT_ON   = 0,
    BT_OFF,

}BT_SETTING_STATUS;

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
EXT void BluetoothConnectResult(int result);
EXT void BluetoothDisConnectResult(int result);
EXT UINT32 BTSetMenuFunSwitch (UINT32 iSelectedItem);
EXT void BTFunSwitchOff();


EXT void BluetoothWinSvcStart(void *pArg);

EXT void BluetoothWinIntInit(void *pArg);

EXT void BluetoothMsgIntInit(void *pArg);

EXT void BluetoothWinMsgDeInit(void *pArg);

EXT void BTControlSubWinInit(void);
EXT void BTControlSubWinDeInit(void);

EXT UINT32 BTControlSubWinService(void);
EXT UINT32 BTControlSubWinKeyProc(void);
EXT void   BTControlSubWinDisplay(void);


#ifdef _SETBT_IN_
_ATTR_SYS_SET_BT_DATA_ WIN BTControlSubWin = {

    NULL,
    NULL,

    BTControlSubWinService,
    BTControlSubWinKeyProc,
    BTControlSubWinDisplay,

    BTControlSubWinInit,
    BTControlSubWinDeInit
};
#else
_ATTR_SYS_SET_BT_DATA_ EXT WIN BTControlSubWin;
#endif
/*
********************************************************************************
*
*                         End of SetBluetooth.h
*
********************************************************************************
*/
#endif