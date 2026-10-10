/*
********************************************************************************
*                   Copyright (C),2004-2016, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  BluetoothScanWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             chad.ma
*    desc:    ORG.
********************************************************************************
*/
#define _IN_BLUETOOTH_SCAN_WIN_

#include "SysInclude.h"

#ifdef _BLUETOOTH_
#ifdef _A2DP_SOUCRE_
//#include "BlueToothA2doSourceWin.h"

#include "SetBluetooth.h"
#include "BluetoothScanWin.h"
#include "BlueToothControl.h"
#include "BluetoothDevInfoWin.h"

#ifdef PIN_CODE_WIN
#include "BluetoothPinCodeWin.h"
#endif

#include "mainmenu.h"
#include "Hold.h"

#include "RadioWinInterface.h"
#include "AudioControl.h"

#include "MessageBox.h"
#include "bd_addr.h"
#include "rk_bt_Api.h"

extern uint8   BTA2DPStatus;
extern uint32  BTState;
extern uint8   BTCallStatus;
extern uint8   BTAvcrpStatus;

#define SID_OFFSET 0

static void ScanStationUpProc(void);

#define     MAX_STATION_NUM_LCD_DISP     6

#define     ALL_BROITEM_UPDATE_STATION   0
#define     UP_UPDATE_STATION           1
#define     DOWN_UPDATE_STATION         2

/*
********************************************************************************
*
*                         Structure Define
*
********************************************************************************
*/
//typedef struct _BTSTATIONITEM_STRUCT
//{
//    struct _BTSTATIONITEM_STRUCT *pPrev;
//    struct _BTSTATIONITEM_STRUCT *pNext;
//    UINT16 LongFileName[33];
//} BTSTATIONITEM;
//
//typedef struct _BTSTATIONLIST_TREE_STRUCT
//{
//    UINT16  CurId;          //current force item index in all items.
//    UINT16  KeyCounter;     //current force item index in current screen.
//    UINT16  PreCounter;     //last force item index in current screen.
//    UINT16  DispTotalItem;  //total item
//    BTSTATIONITEM * pBTBro;
//} BTSTATIONLIST_TREE;


typedef enum
{
    //system message
    TITLE_SCAN_LIST = (UINT8)0x00,
    TITLE_SCANING,
    TITLE_CONNECT,

} SCAN_TITLE_TYPE;


extern BTSTATIONITEM BtStationItem[MAX_STATION_NUM_LCD_DISP];//552
_ATTR_SYS_SET_BT_DATA_  static BTSTATIONLIST_TREE BTScanStationTreeInf;
_ATTR_SYS_SET_BT_DATA_  static UINT16 * BtStationPrintBuf;

_ATTR_SYS_SET_BT_DATA_  static uint8 BtScanCurrFileNum;
_ATTR_SYS_SET_BT_DATA_  struct hci_inq_res * BtScanRes;
_ATTR_SYS_SET_BT_BSS_  uint8 BtPinCodeReqcnt;
//_ATTR_BLUETOOTHWIN_DATA_  struct hci_inq_res * BtWaitConnectRes;
extern struct hci_inq_res BtWaitConnectRes;

extern  uint    BtWinBtA2dpDisconnectTick;
extern  uint8   BtWinBtA2dpDisconnectRetry;
extern  uint8   BtWinBtA2dpDisconnectTickEnable;

extern  uint    BtWinBtConncetTick;

extern uint8 BtWinStatus;
extern uint8  BtWinSubStatus;
extern uint32  bt_connect_handle;
extern uint8  BtScanWinOpened;
_ATTR_SYS_SET_BT_BSS_  uint   BtWinBtScantimetoutTick;
_ATTR_SYS_SET_BT_BSS_  uint8  BtWinBtScantimetoutTickEnable;

extern uint8  BtWinBtScanConnctMac[6];
_ATTR_SYS_SET_BT_CODE_
BlueToothScanResult(struct hci_inq_res *ires, uint16 devCnt, int isComplete)
{
    BtScanRes = ires;
    BTScanStationTreeInf.DispTotalItem = devCnt;
    ScanStationUpProc();

    if (isComplete)
    {
        BtWinBtScantimetoutTickEnable = 0;

        if (gSysConfig.BtConfig.btConnected)
        {
            SendMsg(MSG_BLUETOOTH_AVRCP_PLAY);
        }

        BtWinStatus = BT_WIN_STATUS_IDLE;
    }

    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
}


_ATTR_SYS_SET_BT_CODE_
static void ScanStationItemInfoInit(void)
{
    int i;

    //MusicBroItem clear zero to long file name and file number of MusicBroItem
    for (i = 0; i < MAX_STATION_NUM_LCD_DISP; i++)
    {
        BtStationItem[i].LongFileName[0] = 0;
    }

    //the items in MusicBroItem build one double drection list
    for (i = 0; i < MAX_STATION_NUM_LCD_DISP - 1; i++)
    {
        BtStationItem[i].pNext = &BtStationItem[i + 1];
    }

    for (i = 1; i < MAX_STATION_NUM_LCD_DISP; i++)
    {
        BtStationItem[i].pPrev = &BtStationItem[i - 1];
    }

    BtStationItem[0].pPrev = &BtStationItem[MAX_STATION_NUM_LCD_DISP - 1];
    BtStationItem[MAX_STATION_NUM_LCD_DISP - 1].pNext = &BtStationItem[0];
    BTScanStationTreeInf.CurId = BtScanCurrFileNum;
    BTScanStationTreeInf.PreCounter = 0;
    BTScanStationTreeInf.KeyCounter = BtScanCurrFileNum % MAX_STATION_NUM_LCD_DISP;
    BTScanStationTreeInf.pBTBro = &BtStationItem[0];
    //BTScanStationTreeInf.DispTotalItem = gSysConfig.URLCnt;
    BTScanStationTreeInf.DispTotalItem = 0;
}


_ATTR_SYS_SET_BT_CODE_
static void ScanStationUpProc(void)
{
    BTSTATIONITEM *pBro;
    UINT16  StartItem = 0;
    UINT16  i, j;
    int outlen;
    struct hci_inq_res  *res = NULL;
    StartItem = BTScanStationTreeInf.CurId - BTScanStationTreeInf.KeyCounter;

    for (i = 0; i < MAX_STATION_NUM_LCD_DISP; i++) //clear the items that is diaplay
    {
        BtStationItem[i].LongFileName[0] = 0;
    }

    BTScanStationTreeInf.pBTBro = &BtStationItem[0];
    pBro = BTScanStationTreeInf.pBTBro;
    i = 0;

    for (res = BtScanRes; res != NULL; res = res->next)
    {
        if (i == StartItem)
        {
            break;
        }

        i++;
    }

    for (i = 0; (i < MAX_STATION_NUM_LCD_DISP) && ((i + StartItem) < BTScanStationTreeInf.DispTotalItem); i++)
    {
        //GetBtStationInfo(pBro->LongFileName, RADIONAME, i + StartItem);
        //memcpy(pBro->LongFileName, res->name , 32);
        res->name[31] = 0;

        for (j = 0; j < 32; j++)
        {
            pBro->LongFileName[j] = res->name[j];
        }

        //utf8_to_unicode(res->name, pBro->LongFileName, &outlen);
        res = res->next;
        pBro = pBro->pNext;
    }

    return;
}


/*
--------------------------------------------------------------------------------
 void BlueToothScanWinSvcStart(void *pArg)
  Author        : zhuzhe
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothScanWinSvcStart(void *pArg)
{
    //BlueTooth backgroud service handle start
    if (TRUE != ThreadCheck(pMainThread, &BlueToothThread))
    {
        if (TRUE != ThreadCheck(pMainThread, &MusicThread))
        {
            ThreadDeleteAll(&pMainThread);
        }

        ModuleOverlay(MODULE_ID_BT, MODULE_OVERLAY_ALL);
        ThreadCreat(&pMainThread, &BlueToothThread, pArg);
    }
    else
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            SendMsg(MSG_BLUETOOTH_A2DP_START);
        }
    }
}

/*
--------------------------------------------------------------------------------
 void BlueToothScanWinSvcStart(void *pArg)
  Author        : zhuzhe
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothScanWinSvcStop(void)
{
    ThreadDeleteAll(&pMainThread);
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothScanWinIntInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothScanWinIntInit(void *pArg)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothScanWinDeIntInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothScanWinIntDeInit(void *pArg)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothMsgIntInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothScanMsgInit(void)
{
    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    ClearMsg(MSG_DIALOG_KEY_OK);
    ClearMsg(MSG_DIALOG_KEY_CANCEL);
    ClearMsg(MSG_BLUETOOTH_CONNECT_RETURN);
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothScanWinMsgDeinit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothScanWinMsgDeInit(void *pArg)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothScanWinInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothScanWinInit(void *pArg)
{
    BtScanWinOpened = 1;
    //DEBUG("[BT] BlueToothScanWinInit In");
    BluetoothMsgInit();
    KeyReset();
    ScanStationItemInfoInit();

    if (gSysConfig.BtConfig.btConnected)
    {
        //DEBUG("[BT] bt Connected :check music thread");

        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            if (AUDIO_STATE_PLAY == AudioPlayState)
            {
                MusicDecodeProc(MSG_AUDIO_PAUSE, NULL);
            }
        }
    }

    //DEBUG("[BT] before bt_Scan");
    bt_scan(BlueToothScanResult);
    //DEBUG("[BT] after bt_Scan");
    BtWinStatus = BT_WIN_STATUS_SCAN;
    BtWinBtA2dpDisconnectTickEnable = 0;
    gBTFisrtScan = FALSE;
    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    //DEBUG("[BT] BlueToothScanWinInit Out");
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothScanWinDeInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothScanWinDeInit(void)
{
    //BlueToothScanWinSvcStop();
    //ModuleOverlay(MODULE_ID_SYSRESERVED_OP, MODULE_OVERLAY_ALL);
    //SaveSysInformation();
    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
    BtScanWinOpened = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BlueToothScanWinService(void)
  Author        :
  Description   : it is radio service handle that is used to handle window message,window service task etc.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG

--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BluetoothScanWinService(void)
{
    UINT32 RetVal = 0;
    TASK_ARG TaskArg;

#ifdef NUMERIC_REQ_WIN
    if (GetMsg(MSG_BT_USER_CONFIRMATION_REQUEST) == TRUE)
    {
        //DEBUG("MSG_BT_USER_CONFIRMATION_REQUEST\n");
        if(BtWinStatus == BT_WIN_STATUS_CONNECTING)
        {
            BtWinSubStatus = BT_WIN_SUB_STATUS_CONFIRMATION_REQ;
            #if 0
            TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
            TaskArg.Dialog.TitleID = SID_WARNING;
            TaskArg.Dialog.ContentID = SID_BT_DISCONNECT_CUR_DEV;
            WinCreat(&BlueToothScanWin, &DialogWin, &TaskArg);
            #else
            TaskArg.Dialog.Button = BT_PIN_KEY_BUTTON_YES;  //default Yes
            TaskArg.Dialog.TitleID = NULL;                  //can modified it
            TaskArg.Dialog.ContentID = NULL;                //can modified it
            WinCreat(&BlueToothScanWin, &BTPinKeyCfmWin,NULL);
            #endif
        }
    }

    if (BtWinSubStatus == BT_WIN_SUB_STATUS_CONFIRMATION_REQ)
    {
        if (GetMsg(MSG_BT_AUTH_OK) == TRUE)
        {
            BtWinBtConncetTick = GetSysTick();
            BtWinSubStatus = BT_WIN_SUB_STATUS_IDLE;
        }

        if (GetMsg(MSG_BT_AUTH_FAIL) == TRUE)
        {
        }
    }
#endif

#ifdef PIN_CODE_WIN
    if (GetMsg(MSG_BT_PIN_CODE_REQ) == TRUE)
    {
        if(BtWinStatus == BT_WIN_STATUS_CONNECTING)
        {
            BtWinSubStatus = BT_WIN_SUB_STATUS_PIN_REQ; //open pin code input window
            if(BtPinCodeReqcnt == 0)
            {
                hci_pin_code_request_reply((struct bd_addr *)BtWinBtScanConnctMac, 4, "0000");
            }
            else
            {
                WinCreat(&BlueToothScanWin, &BTPinCodeWin, NULL);
            }
            BtPinCodeReqcnt++;
        }
    }

    if (BtWinSubStatus == BT_WIN_SUB_STATUS_PIN_REQ)
    {
        if (GetMsg(MSG_BT_AUTH_OK) == TRUE)
        {
            BtWinBtConncetTick = GetSysTick();
            BtWinSubStatus = BT_WIN_SUB_STATUS_IDLE;
        }

        if (GetMsg(MSG_BT_AUTH_FAIL) == TRUE)
        {
            printf("hci_Authentication_Requested@MSG_BT_AUTH_FAIL\n");
            hci_Authentication_Requested(bt_connect_handle);
        }
    }
#endif

    if (GetMsg(MSG_BLUETOOTH_CONNECT_SUCCEED))
    {
        TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
        TaskArg.Message.ContentID = SID_BT_CONNECTING_OK + SID_OFFSET;
        TaskArg.Message.HoldTime  = 2;
        WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
        SendMsg(MSG_BLUETOOTH_CONNECT_RETURN);
    }

    if (GetMsg(MSG_BLUETOOTH_CONNECT_FAIL))
    {
        TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
        TaskArg.Message.ContentID = SID_BT_CONNECTING_FAIL + SID_OFFSET;
        TaskArg.Message.HoldTime  = 2;
        WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
        //SendMsg(MSG_BLUETOOTH_CONNECT_RETURN);
    }

    if(GetMsg(MSG_BT_HCI_CANNEL_CONNECT_OK))
    {
        BtWinStatus = BT_WIN_STATUS_IDLE;
        TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
        TaskArg.Message.ContentID = SID_BT_CONNECT_CANCEL_COMPLETE + SID_OFFSET;
        TaskArg.Message.HoldTime  = 1;
        WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg); //show cannel connect ok
    }

    if(GetMsg(MSG_BT_HCI_CANNEL_CONNECT_FAIL_CONNECED_EXIT))
    {
        BtWinStatus = BT_WIN_STATUS_HCI_CONNECT_CANNELING;
        bt_disconnect(&BtWaitConnectRes.bdaddr);
    }

    if (GetMsg(MSG_BLUETOOTH_WIN_DISCONNECTED))
    {
#ifdef PIN_CODE_WIN
        if (BtWinSubStatus == BT_WIN_SUB_STATUS_PIN_REQ)
        {
            WinDestroy(&BTPinCodeWin);//acl is disconnected,close pin code input window
            BtWinSubStatus = BT_WIN_SUB_STATUS_IDLE;
        }
#endif
        BtWinBtA2dpDisconnectTickEnable = 0;
        if(BtWinStatus == BT_WIN_STATUS_ABORT_CONNECTING)
        {
            TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
            TaskArg.Message.ContentID = SID_BT_CONNECT_CANCEL_COMPLETE + SID_OFFSET;
            TaskArg.Message.HoldTime  = 1;
            WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg); //show cannel connect is ok
        }
        else
        {
            TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
            TaskArg.Message.ContentID = SID_BT_DISCONNECT + SID_OFFSET;
            TaskArg.Message.HoldTime  = 1;
            WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg); //show disconnect window
        }

        if (BtWinStatus == BT_WIN_STATUS_CONNECT_NEW_DEV)
        {
            BtWinStatus = BT_WIN_STATUS_CONNECTING;
            BtWinBtConncetTick = GetSysTick();
            SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
            memcpy(BtWinBtScanConnctMac, BtWaitConnectRes.bdaddr.addr, 6);
            bt_a2dp_connect((struct bd_addr *)BtWinBtScanConnctMac, BluetoothConnectResult);
        }
        else
        {
            BtWinStatus = BT_WIN_STATUS_IDLE;
        }
    }

    if (GetMsg(MSG_MESSAGEBOX_DESTROY))
    {
        if (GetMsg(MSG_BLUETOOTH_CONNECT_RETURN))
        {
            WinDestroy(&BlueToothScanWin);
            SendMsg(MSG_BLUETOOTH_SCAN_WIN_DESTROY);
        }
        else
        {
            SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
        }
    }

    if(GetMsg(MSG_BT_INPUT_PINCODE_OK))
    {
#ifdef PIN_CODE_WIN
        if (BtWinStatus == BT_WIN_STATUS_CONNECTING && BtWinSubStatus == BT_WIN_SUB_STATUS_PIN_REQ) //pin code req
        {
            uint32 pincode_len;
            pincode_len = BT_GetValidPinCodeLen(gPinCode);
            //DEBUG("[BT] pincode valid len = %d",pincode_len);
            hci_pin_code_request_reply((struct bd_addr *)BtWinBtScanConnctMac, pincode_len, gPinCode);
        }
#endif
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    }

    if(GetMsg(MSG_BT_INPUT_PINCODE_CANCLE))
    {
#ifdef PIN_CODE_WIN
        if (BtWinStatus == BT_WIN_STATUS_CONNECTING && BtWinSubStatus == BT_WIN_SUB_STATUS_PIN_REQ) //pin code req
        {
            //hci_pin_code_request_neg_reply((struct bd_addr *)BtWinBtScanConnctMac);
            //BT_DEBUG("bt_disconnect@BluetoothScanWinService\n");
            bt_disconnect((struct bd_addr *)BtWinBtScanConnctMac);//ֱ�ӶϿ�����
        }
        BtWinSubStatus == BT_WIN_SUB_STATUS_IDLE;
#endif
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    }

    if(GetMsg(MSG_BT_CONFIRM_PINCODE_OK))
    {
#ifdef NUMERIC_REQ_WIN
        if (BtWinStatus == BT_WIN_STATUS_CONNECTING && BtWinSubStatus == BT_WIN_SUB_STATUS_CONFIRMATION_REQ)
        {
            hci_user_confirmati_req_reply((struct bd_addr *)BtWinBtScanConnctMac);
        }
#endif
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    }

    if(GetMsg(MSG_BT_CONFIRM_PINCODE_NO_OK))
    {
#ifdef NUMERIC_REQ_WIN
        if (BtWinStatus == BT_WIN_STATUS_CONNECTING && BtWinSubStatus == BT_WIN_SUB_STATUS_CONFIRMATION_REQ)
        {
            hci_user_confirmati_req_negative_reply((struct bd_addr *)BtWinBtScanConnctMac);
        }
#endif
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    }

    if (GetMsg(MSG_DIALOG_KEY_OK))
    {
        if (gSysConfig.BtConfig.btConnected)
        {
            gSysConfig.BtConfig.btConnected = 0;
            //bt_disconnect((struct bd_addr *)BtWinBtScanConnctMac);
            BtWinBtA2dpDisconnectTick = GetSysTick();
            BtWinBtA2dpDisconnectRetry = 2;
            BtWinBtA2dpDisconnectTickEnable = 1;
            bt_a2dp_disconnect((struct bd_addr *)BtWinBtScanConnctMac, NULL);
        }
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    }

    if (GetMsg(MSG_DIALOG_KEY_CANCEL))
    {
        BtWinStatus = BT_WIN_STATUS_IDLE;
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    }

//    if(GetMsg(MSG_BT_DEVINFO_WIN_DESTROY))
//    {
//        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
//    }

    if (BtWinBtScantimetoutTickEnable)
    {
        if (GetSysTick() > BtWinBtScantimetoutTick + 4500)
        {
            BtWinBtScantimetoutTickEnable = 0;
            bt_scan_cancel();
            DelayMs(200);
            BtWinStatus = BT_WIN_STATUS_IDLE;
            SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
        }
    }

    return RetVal;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BlueToothScanWinKeyProc(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BluetoothScanWinKeyProc(void)
{
    UINT32  TempKeyVal;
    UINT32  RetVal = 0;
    TASK_ARG TaskArg;
    LCD_RECT             r;
    TempKeyVal =  GetKeyVal();

    //5 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (TempKeyVal)
        {
            case KEY_VAL_FFD_SHORT_UP:
                TempKeyVal = KEY_VAL_DOWN_SHORT_UP;
                break;

            case KEY_VAL_FFD_PRESS:
                TempKeyVal = KEY_VAL_DOWN_PRESS;
                break;

            case KEY_VAL_FFW_SHORT_UP:
                TempKeyVal = KEY_VAL_UP_SHORT_UP;
                break;

            case KEY_VAL_FFW_PRESS:
                TempKeyVal = KEY_VAL_UP_PRESS;
                break;

            default:
                break;
        }
    }

    //6 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
        switch (TempKeyVal)
        {
            default:
                break;
        }
    }

    //7 key function
    switch (TempKeyVal)
    {
//        case KEY_VAL_PLAY_SHORT_UP:
//        {
//            //here according to current select item to display device info
//            WinCreat(&BlueToothScanWin, &BTLocalDevInfoWin, NULL);
//            break;
//        }

        case KEY_VAL_PLAY_LONG_UP:
        {
        }
        break;

        case KEY_VAL_MENU_PRESS_START:
        {
#if 0

            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }

            ThreadDelete(&pMainThread, &BlueToothThread);
            ModuleOverlay(MODULE_ID_SYSRESERVED_OP, MODULE_OVERLAY_ALL);
            SaveSysInformation();
#endif
            WinDestroy(&BTControlSubWin);
            SendMsg(MSG_SYS_SET_DISPLAY);

            //TaskArg.MainMenu.MenuID = MAINMENU_ID_BLUETOOTH;
            //TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            //RetVal = 1;
            break;
        }

        case KEY_VAL_MENU_SHORT_UP:
        {
            int i;
            struct hci_inq_res  *res = NULL;
            uint ItemTemp = BTScanStationTreeInf.CurId;// - BTScanStationTreeInf.KeyCounter;

            if (BtWinStatus == BT_WIN_STATUS_SCAN)
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_SCANING + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
                break;
                {
                    //bt_clean_scan_result(); //������ϴ�ɨ����
                    //BtScanRes = NULL;
                    //BTScanStationTreeInf.DispTotalItem = 0;
                    //ScanStationUpProc();
                }
                //BtWinStatus = BT_WIN_STATUS_IDLE; ?//��ɨ����������ӿ���ȡ��ɨ����������
            }
            else if (BtWinStatus == BT_WIN_STATUS_CONNECTING)
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_CONNECTING + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
                break;
            }
            else if(BtWinStatus != BT_WIN_STATUS_IDLE)
            {
                break;
            }
            i = 0;

            for (res = BtScanRes; res != NULL; res = res->next)
            {
                if (i == ItemTemp)
                {
                    break;
                }

                i++;
            }
            BtPinCodeReqcnt = 0;
            if (res != NULL && BtWinStatus == BT_WIN_STATUS_IDLE)
            {
                if (!gSysConfig.BtConfig.btConnected)
                {
                    BtWinStatus = BT_WIN_STATUS_CONNECTING;
                    BtWinBtConncetTick = GetSysTick();
                    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                    memcpy(&BtWaitConnectRes, res, sizeof(struct hci_inq_res));
                    memcpy(BtWinBtScanConnctMac, (uint8*)&res->bdaddr, 6);
                    bt_a2dp_connect(&res->bdaddr, BluetoothConnectResult);
                }
                else
                {
                    if (bd_addr_cmp(&res->bdaddr, (struct bd_addr *)BtWinBtScanConnctMac))
                    {
                        //BtWinStatus = BT_WIN_STATUS_DISCONNECT_CUR_DEV;
                    }
                    else
                    {
                        BtWinStatus = BT_WIN_STATUS_CONNECT_NEW_DEV;
                        //BtWaitConnectRes = res;
                        memcpy(&BtWaitConnectRes, res, sizeof(struct hci_inq_res));
                    }

                    TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
                    TaskArg.Dialog.TitleID = SID_WARNING;
                    TaskArg.Dialog.ContentID = SID_BT_DISCONNECT_CUR_DEV;
                    WinCreat(&BlueToothScanWin, &DialogWin, &TaskArg);
                }
            }

            break;
        }

        //case KEY_VAL_FFD_SHORT_UP:
        case KEY_VAL_PLAY_SHORT_UP:
        {
            //play key
            if (BtWinStatus == BT_WIN_STATUS_SCAN)
            {
                //TaskArg.Message.TitleID   = SID_BLUETOOTH+SID_OFFSET;
                //TaskArg.Message.ContentID = SID_BT_SCANING+SID_OFFSET;
                //TaskArg.Message.HoldTime  = 1;
                //WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
                //bt_clean_scan_result();
                bt_scan_cancel();
                //DEBUG("BT scan cancel.......");
                DelayMs(100);
                BtWinStatus = BT_WIN_STATUS_IDLE;

                if (gSysConfig.BtConfig.btConnected)
                {
                    SendMsg(MSG_BLUETOOTH_AVRCP_PLAY);
                }

                SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                break;
            }
            else if (BtWinStatus == BT_WIN_STATUS_CONNECTING)
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_CONNECTING + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
                break;
            }

            {
                bt_clean_scan_result(); //������ϴ�ɨ����
                BtScanRes = NULL;
                BTScanStationTreeInf.DispTotalItem = 0;
                BTScanStationTreeInf.CurId = 0;
                BTScanStationTreeInf.KeyCounter = 0;
                ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                ScanStationUpProc();
            }

            if (gSysConfig.BtConfig.btConnected)
            {
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    if (AUDIO_STATE_PLAY == AudioPlayState)
                    {
                        MusicDecodeProc(MSG_AUDIO_PAUSE, NULL);
                    }
                }
            }

            bt_scan(BlueToothScanResult);
            //DEBUG("BT scan.......");
            BtWinBtScantimetoutTickEnable = 1;
            BtWinBtScantimetoutTick = GetSysTick();
            BtWinStatus = BT_WIN_STATUS_SCAN;
            gBTFisrtScan = FALSE;
            SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
            break;
        }

        case KEY_VAL_FFW_SHORT_UP:
        {
            //Prev
            uint8 devmac[] = {0x60, 0xAB, 0x3A, 0x34, 0x18, 0x00};

            if (BtWinStatus == BT_WIN_STATUS_SCAN)
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_SCANING + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
                break;
            }
            else if (BtWinStatus == BT_WIN_STATUS_CONNECTING)
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_CONNECTING + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
                break;
            }
            else if(BtWinStatus != BT_WIN_STATUS_IDLE)
            {
                break;
            }

            memcpy(BtWinBtScanConnctMac, devmac, 6);
            BtWinStatus = BT_WIN_STATUS_CONNECTING;
            BtWinBtConncetTick = GetSysTick();
            bt_a2dp_connect((struct bd_addr *)BtWinBtScanConnctMac, BluetoothConnectResult);
            SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
            break;
        }
        case KEY_VAL_FFD_SHORT_UP:
        {
            //DEBUG("KEY_VAL_FFD_SHORT_UP\n");
            if(BtWinStatus == BT_WIN_STATUS_CONNECTING && BtWinSubStatus == BT_WIN_SUB_STATUS_IDLE)
            {

                if(hci_is_link((struct bd_addr *)BtWinBtScanConnctMac, 0x01))
                {
                    //DEBUG("bt cannel connecting bt_disconnect\n");
                    BtWinStatus = BT_WIN_STATUS_ABORT_CONNECTING;
                    bt_disconnect((struct bd_addr *)BtWinBtScanConnctMac);
                }
                else
                {
                    //DEBUG("bt cannel connecting hci_create_connection_cancel_command\n");
                    {
                        LCD_RECT             r;
                        r.x0 = 14;
                        r.y0 = 146;
                        r.x1 = r.x0 + 100;
                        r.y1 = r.y0 + CH_CHAR_YSIZE_12;
                        LCD_DispStringInRect(&r, &r, L"CANCELING", LCD_TEXTALIGN_CENTER);

                    }
                    BtWinStatus = BT_WIN_STATUS_HCI_CONNECT_CANNELING;
                    hci_create_connection_cancel_command(&BtWaitConnectRes.bdaddr);
                }
                SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);

            }
        }
        break;
        case KEY_VAL_UP_SHORT_UP:
            BTScanStationTreeInf.PreCounter = BTScanStationTreeInf.KeyCounter;

            if (BTScanStationTreeInf.CurId > 0)
            {
                BTScanStationTreeInf.CurId--;

                if (BTScanStationTreeInf.KeyCounter == 0)
                {
                    BTScanStationTreeInf.KeyCounter = MAX_STATION_NUM_LCD_DISP - 1;
                    ScanStationUpProc();
                    SendMsg(MSG_BROW_DIS_ALL_ITEM);
                }
                else
                {
                    BTScanStationTreeInf.KeyCounter--;
                    SendMsg(MSG_BROW_DIS_SELE_ITEM);
                }
            }
            else
            {
                BTScanStationTreeInf.CurId = BTScanStationTreeInf.DispTotalItem - 1;

                if (BTScanStationTreeInf.DispTotalItem > MAX_STATION_NUM_LCD_DISP)
                {
                    if (BTScanStationTreeInf.DispTotalItem % MAX_STATION_NUM_LCD_DISP)
                    {
                        BTScanStationTreeInf.KeyCounter = BTScanStationTreeInf.DispTotalItem % MAX_STATION_NUM_LCD_DISP - 1;
                    }
                    else
                    {
                        BTScanStationTreeInf.KeyCounter =  MAX_STATION_NUM_LCD_DISP - 1;
                    }

                    ScanStationUpProc();
                    SendMsg(MSG_BROW_DIS_ALL_ITEM);
                }
                else
                {
                    BTScanStationTreeInf.KeyCounter = BTScanStationTreeInf.DispTotalItem - 1;
                    SendMsg(MSG_BROW_DIS_SELE_ITEM);
                }
            }

            break;

        case KEY_VAL_DOWN_SHORT_UP:
            BTScanStationTreeInf.PreCounter = BTScanStationTreeInf.KeyCounter;

            if (BTScanStationTreeInf.CurId < (BTScanStationTreeInf.DispTotalItem - 1))
            {
                BTScanStationTreeInf.CurId++;

                if (BTScanStationTreeInf.KeyCounter >= MAX_STATION_NUM_LCD_DISP - 1)
                {
                    BTScanStationTreeInf.KeyCounter = 0;
                    ScanStationUpProc();
                    SendMsg(MSG_BROW_DIS_ALL_ITEM);//display all screen
                }
                else
                {
                    BTScanStationTreeInf.KeyCounter++;
                    SendMsg(MSG_BROW_DIS_SELE_ITEM);
                }
            }
            else
            {
                BTScanStationTreeInf.CurId = 0;
                BTScanStationTreeInf.KeyCounter = 0;

                if (BTScanStationTreeInf.DispTotalItem > MAX_STATION_NUM_LCD_DISP)
                {
                    ScanStationUpProc();
                    SendMsg(MSG_BROW_DIS_ALL_ITEM);
                }
                else
                {
                    SendMsg(MSG_BROW_DIS_SELE_ITEM);
                }
            }

            break;

        case KEY_VAL_HOLD_ON:
        {
            break;
        }

        case KEY_VAL_HOLD_OFF:
        {
            break;
        }

        case KEY_VAL_ESC_SHORT_UP:
        {
            //TaskArg.MainMenu.MenuID = MAINMENU_ID_BLUETOOTH;
            //TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            if (BtWinStatus == BT_WIN_STATUS_SCAN)
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_SCANING + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
                break;
            }
            else if (BtWinStatus == BT_WIN_STATUS_CONNECTING)
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_CONNECTING + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BlueToothScanWin, &MessageBoxWin, &TaskArg);
                break;
            }

            WinDestroy(&BlueToothScanWin);
            //DEBUG("[BT] destroy BT ScanWin.");
            SendMsg(MSG_BLUETOOTH_SCAN_WIN_DESTROY);
            //RetVal = 1;
            break;
        }

        default:
            break;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothScanWinPaint(void)
  Author        : mlc
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
void BtStationScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed);

_ATTR_SYS_SET_BT_CODE_
void BluetoothScanWinPaint(void)
{
    UINT16  i, CurNum;
    UINT16  TempColor, TempBkColor, TempCharSize, TempTxtMode;
    UINT16  TotalItem;
    UINT16  *printfbuf;
    UINT32   x, y;
    LCD_RECT             r;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;
    BTSTATIONITEM   *pBro = BTScanStationTreeInf.pBTBro;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (GetMsg(MSG_BLUETOOTH_DISPLAY_ALL) || GetMsg(MSG_NEED_PAINT_ALL))
    {
        //Display BackGround
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);
        //Display Title
        r.x0 = 14;
        r.y0 = 146;
        r.x1 = r.x0 + 100;
        r.y1 = r.y0 + CH_CHAR_YSIZE_12;

        if (BtWinStatus == BT_WIN_STATUS_IDLE)
        {
            LCD_DispStringInRect(&r, &r, L"SCAN LIST", LCD_TEXTALIGN_CENTER);
        }
        else if (BtWinStatus == BT_WIN_STATUS_SCAN)
        {
            LCD_DispStringInRect(&r, &r, L"SCANING", LCD_TEXTALIGN_CENTER);
        }
        else if (BtWinStatus == BT_WIN_STATUS_CONNECTING)
        {
            LCD_DispStringInRect(&r, &r, L"CONNECTING", LCD_TEXTALIGN_CENTER);
        }
        else if(BtWinStatus == BT_WIN_STATUS_ABORT_CONNECTING || BtWinStatus == BT_WIN_STATUS_HCI_CONNECT_CANNELING)
        {
            LCD_DispStringInRect(&r, &r, L"CANCELING", LCD_TEXTALIGN_CENTER);
        }

        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }

    if (GetMsg(MSG_BROW_DIS_SELE_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_ITEMBARNOSEL0, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_MUSIC, &PicInfo1);

        for (i = 0; i < MAX_STATION_NUM_LCD_DISP; i++)
        {
            r.x0 = PicInfo.x;
            r.y0 = 5 + 17 * i;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0 + CH_CHAR_YSIZE_12;

            if ((i == BTScanStationTreeInf.PreCounter) || (i == BTScanStationTreeInf.KeyCounter))
            {
                //icon
                //DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_MUSIC, PicInfo1.x, PicInfo.y+PicInfo.ySize*i);

                //item
                if (i == BTScanStationTreeInf.PreCounter)
                {
                    //DEBUG("i = %d", i);
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_ITEMBARNOSEL0, PicInfo.x, 5 + i * 17);
                }

                if (i == BTScanStationTreeInf.KeyCounter)
                {
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_ITEMBARSEL0, PicInfo.x, 5 + i * 17);

                    if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                    {
                        BtStationPrintBuf = pBro->LongFileName;
                        BtStationScrollInit(&r, IMG_ID_BROWSER_ITEMBARSEL0, pBro->LongFileName, 30);
                        SendMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                    }
                    else
                    {
                        ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                    }
                }
            }

            LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);
            pBro = pBro->pNext;
        }

        if (BTScanStationTreeInf.DispTotalItem > MAX_STATION_NUM_LCD_DISP)
        {
            SendMsg(MSG_BROW_DIS_SCLLO_ITEM);
        }
    }

    if (GetMsg(MSG_BROW_DIS_ALL_ITEM))
    {
        //DispPictureWithIDNum(IMG_ID_BROWSER_CONTENTBACKGROUND);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_ITEMBARSEL0, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BLUETOOTH_DEV_ICON, &PicInfo1);
        TotalItem = BTScanStationTreeInf.DispTotalItem - (BTScanStationTreeInf.CurId - BTScanStationTreeInf.KeyCounter);

        for (i = 0; ((i < MAX_STATION_NUM_LCD_DISP) && (i < TotalItem )); i++)
        {
            //icon
            //DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_MUSIC, PicInfo1.x, PicInfo.y+PicInfo.ySize*i);
            DispPictureWithIDNumAndXY(IMG_ID_BLUETOOTH_DEV_ICON, PicInfo1.x + 5, 5 + 17 * i);
            r.x0 = PicInfo.x;
            r.y0 = 5 + 17 * i;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0 + CH_CHAR_YSIZE_12;

            //TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);

            if (i == BTScanStationTreeInf.KeyCounter)
            {
                //GetPictureInfoWithIDNum(IMG_ID_BROWSER_ITEMBARSEL0, &PicInfo1);
                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_ITEMBARSEL0, PicInfo.x, 5 + i * 17);

                //TextMode = LCD_SetTextMode(LCD_DRAWMODE_REV);
                if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                {
                    BtStationPrintBuf = pBro->LongFileName;
                    BtStationScrollInit(&r, IMG_ID_BROWSER_ITEMBARSEL0, pBro->LongFileName, 30);
                    SendMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                }
                else
                {
                    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                }
            }

            LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);

            /*if (i == BTScanStationTreeInf.KeyCounter)
            {
                LCD_SetTextMode(TextMode);
            }*/

            pBro = pBro->pNext;
        }

        if (BTScanStationTreeInf.DispTotalItem > MAX_STATION_NUM_LCD_DISP)
        {
            SendMsg(MSG_BROW_DIS_SCLLO_ITEM);
        }
    }

    //slider block display..
    if (GetMsg(MSG_BROW_DIS_SCLLO_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCROLLBACKGROUND, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_SETMENU_SCROLL1, &PicInfo1);
        CurNum = BTScanStationTreeInf.CurId;

        if (CurNum > 0)
        {
            y = (PicInfo.ySize - (PicInfo1.ySize << 1)) * CurNum / (BTScanStationTreeInf.DispTotalItem - 1);
        }
        else
        {
            y = 0;
        }

        if ((y + PicInfo1.ySize) > PicInfo.ySize)
        {
            y = PicInfo.ySize - PicInfo1.ySize;
        }

        DispPictureWithIDNum(IMG_ID_SETMENU_SCROLLBACKGROUND);
        DispPictureWithIDNumAndXYoffset(IMG_ID_SETMENU_SCROLL1, 0, y);//scroll small icon
    }

    if (CheckMsg(BROWSER_DISPFLAG_SCROLL_FILENAME))//an 4.21
    {
        //TextMode = LCD_SetTextMode(LCD_DRAWMODE_REV);
        //TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        ScrollStringCommon(BtStationPrintBuf);
        //LCD_SetTextMode(TextMode);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
********************************************************************************
*
*                         End of BluetoothScanWin.c
*
********************************************************************************
*/
#endif
#endif

