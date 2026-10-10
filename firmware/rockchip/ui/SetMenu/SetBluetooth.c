/*
********************************************************************************
*                   Copyright (C),2004-2016, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  SetBluetooth.c
*
* Description:  set menu execution function configuration of recording
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-3-3          1.0
*    desc:      the functions below will be excuted when press sure key and it has not child item,
                the itemNum be force set to CurItemInFatherID,that is the index of current item,so it can
                be used to do porper contorl in inside of function.
********************************************************************************
*/
#define  _SETBT_IN_

#include "SysInclude.h"

#ifdef _SYSSET_
#ifdef _BLUETOOTH_
#include "SetCommon.h"
#include "SetBluetooth.h"
#include "SetMenuInterface.h"


#ifdef _A2DP_SOUCRE_
//#include "BlueToothA2doSourceWin.h"
#include "BlueToothControl.h"

#include "mainmenu.h"
#include "Hold.h"
#include "BluetoothScanWin.h"
#include "BluetoothDevInfoWin.h"

#include "RadioWinInterface.h"
#include "AudioControl.h"
#include "bd_addr.h"
#include "rk_bt_Api.h"
#include "MessageBox.h"
#include "BlueToothSysParam.h"

extern uint8   BTA2DPStatus;
extern uint32  BTState;
extern uint8   BTCallStatus;
extern uint8   BTAvcrpStatus;

#define SID_OFFSET 0


static void StationUpProc(void);


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

_ATTR_SYS_SET_BT_DATA_  BTSTATIONITEM BtStationItem[MAX_STATION_NUM_LCD_DISP];//552

_ATTR_SYS_SET_BT_DATA_  /*static*/ BTSTATIONLIST_TREE BTStationTreeInf;
_ATTR_SYS_SET_BT_DATA_  static UINT16 * BtStationPrintBuf;

_ATTR_SYS_SET_BT_DATA_  static uint8 BtCurrFileNum;
_ATTR_SYS_SET_BT_DATA_  struct hci_inq_res * BtRes;

uint8  BtWinStatus;
uint8  BtWinSubStatus;
_ATTR_SYS_SET_BT_DATA_ uint   BtWinBtOffTick;
_ATTR_SYS_SET_BT_DATA_ uint   BtWinBtA2dpDisconnectTick;
_ATTR_SYS_SET_BT_DATA_ uint8  BtWinBtA2dpDisconnectRetry;
_ATTR_SYS_SET_BT_DATA_  uint8  BtWinBtA2dpDisconnectTickEnable;

_ATTR_SYS_SET_BT_BSS_  uint   BtWinBtConncetTick;
_ATTR_SYS_SET_BT_BSS_  uint   BtWinBtConnectId;
_ATTR_SYS_SET_BT_BSS_  uint8  BtScanWinOpened;
_ATTR_SYS_SET_BT_BSS_  uint8  BtWinBtScanConnctMac[6];
//_ATTR_BLUETOOTHWIN_CODE_
//BlueToothScanResult(struct hci_inq_res *ires, uint16 devCnt, uint8 isComplete)
//{
//    BtRes = ires;
//
//    BTStationTreeInf.DispTotalItem = devCnt;
//
//    StationUpProc();
//    SendMsg(MSG_BROW_DIS_ALL_ITEM);
//}

/*public funciton,must be in system code*/
void BluetoothThreadDelete(bool isConnect)
{
    if (TRUE == ThreadCheck(pMainThread, &BlueToothThread))
    {
        //DEBUG("### Bluetooth thread exit ###");

        if (isConnect)  //bt connected
        {
            //disconnect BT
            bt_a2dp_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, NULL);
            gbBTConnected = 0;
            BtWinStatus = BT_WIN_STATUS_BT_GO_TO_OFF;
            DelayMs(500);
        }

        //delete bt thread
        ThreadDelete(&pMainThread, &BlueToothThread);
        //DEBUG("### Bluetooth thread deleted...###");
        BtWinStatus = BT_WIN_STATUS_IDLE;
        SendMsg(MSG_BLUETOOTH_RECONNECT);
    }
    else
    {
        //DEBUG("bt thread not exit");
    }
}

_ATTR_SYS_SET_BT_CODE_
int BluetoothIsCurConnectDev(uint16 * name)
{
    uint8 devName[33];
    uint8 *curConnectName;
    int i, j;
    curConnectName = NULL;

    if (!gSysConfig.BtConfig.btConnected)
    {
        return 0;
    }

    for (i = 0; i < 33; i++)
    {
        devName[i] = name[i] & 0x00FF;
    }

    DEBUG("PairedDevCnt is : %d", gSysConfig.BtConfig.PairedDevCnt);
    {
        DEBUG("LastConnectMac:");
        BtMacAddr2String(gSysConfig.BtConfig.LastConnectMac);
    }

    for (j = 0; j < gSysConfig.BtConfig.PairedDevCnt; j++)
    {
        {
            DEBUG("link MAC %d:", j);
            BtMacAddr2String(gSysConfig.BtConfig.BtLinkKey[j].BdAddr);
        }

        if (memcmp(gSysConfig.BtConfig.BtLinkKey[j].BdAddr, gSysConfig.BtConfig.LastConnectMac, 6) == 0)
        {
            curConnectName = gSysConfig.BtConfig.BtLinkKey[j].name;
            DEBUG("curConnectName is : %s", curConnectName);
            break;
        }
    }

    if (curConnectName == NULL)
    {
        return 0;
    }

    if (strncmp(curConnectName, devName,sizeof(devName)) == 0)
    {
        return 1;
    }

    return 0;
}

_ATTR_SYS_SET_BT_CODE_
void BluetoothConnectResult(int result)
{
    //DEBUG("Enter BluetoothConnectResult\n");
    BtWinStatus = BT_WIN_STATUS_IDLE;

    if (result == 0)
    {
        if(BtScanWinOpened)
        {
            memcpy(gSysConfig.BtConfig.LastConnectMac,BtWinBtScanConnctMac,6);
        }
        SendMsg(MSG_BLUETOOTH_CONNECT_SUCCEED);
        //���ӳɹ�
    }
    else
    {
        SendMsg(MSG_BLUETOOTH_CONNECT_FAIL);
        //����ʧ��
    }

    //DEBUG("Leaving BluetoothConnectResult\n");
}

_ATTR_SYS_SET_BT_CODE_
void BluetoothDisConnectResult(int result)
{
    if (BtWinStatus == BT_WIN_STATUS_BT_OFF_ING)
    {
        BtWinStatus = BT_WIN_STATUS_BT_GO_TO_OFF;
        return;
    }

    if (BtWinStatus != BT_WIN_STATUS_CONNECT_NEW_DEV)
    {
        BtWinStatus = BT_WIN_STATUS_IDLE;
    }

    SendMsg(MSG_BLUETOOTH_DISCONNECT_SUCCEED);
}

#if 0
_ATTR_BLUETOOTHWIN_CODE_
void BlueToothDeletePairedDve(struct bd_addr *bdaddr)
{
    int i;

    if (gSysConfig.BtConfig.PairedDevCnt == 0)
        return;

    for (i = 0; i < BT_LINK_KEY_MAX_NUM; i++)
    {
        if (bd_addr_cmp((struct bd_addr *)(&gSysConfig.BtConfig.BtLinkKey[i].BdAddr), bdaddr))
        {
            break;
        }
    }

    if (i == BT_LINK_KEY_MAX_NUM)
    {
        return;
    }

    if (i == (gSysConfig.BtConfig.PairedDevCnt - 1))
    {
        memset(&gSysConfig.BtConfig.BtLinkKey[i], 0x00, sizeof(BT_LINK_KEY));
    }
    else
    {
        for (; i < (gSysConfig.BtConfig.PairedDevCnt - 1) ; i++)
        {
            memcpy(&gSysConfig.BtConfig.BtLinkKey[i], &gSysConfig.BtConfig.BtLinkKey[i + 1], sizeof(BT_LINK_KEY) ) ;
        }
    }

    gSysConfig.BtConfig.PairedDevCnt--;
    gSysConfig.BtConfig.KeyIndex--;
}

#endif



_ATTR_SYS_SET_BT_CODE_
static void StationItemInfoInit(void)
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
    BTStationTreeInf.CurId = BtCurrFileNum;
    BTStationTreeInf.PreCounter = 0;
    BTStationTreeInf.KeyCounter = BtCurrFileNum % MAX_STATION_NUM_LCD_DISP;
    BTStationTreeInf.pBTBro = &BtStationItem[0];
    //BTStationTreeInf.DispTotalItem = gSysConfig.URLCnt;
    BTStationTreeInf.DispTotalItem = gSysConfig.BtConfig.PairedDevCnt;
}


_ATTR_SYS_SET_BT_CODE_
static void StationUpProc(void)
{
    BTSTATIONITEM *pBro;
    UINT16  StartItem = 0;
    UINT16  i, j;
    int outlen;
    StartItem = BTStationTreeInf.CurId - BTStationTreeInf.KeyCounter;

    for (i = 0; i < MAX_STATION_NUM_LCD_DISP; i++) //clear the items that is diaplay
    {
        BtStationItem[i].LongFileName[0] = 0;
    }

    BTStationTreeInf.pBTBro = &BtStationItem[0];
    pBro = BTStationTreeInf.pBTBro;
    BTStationTreeInf.DispTotalItem = gSysConfig.BtConfig.PairedDevCnt;

    for (i = 0; (i < MAX_STATION_NUM_LCD_DISP) && ((i + StartItem) < BTStationTreeInf.DispTotalItem); i++)
    {
        for (j = 0; j < 32; j++)
        {
            pBro->LongFileName[j] = gSysConfig.BtConfig.BtLinkKey[i + StartItem].name[j];
        }

        pBro->LongFileName[31] = 0;
        //utf8_to_unicode(res->name, pBro->LongFileName, &outlen);
        pBro = pBro->pNext;
    }

    //chad.ma add
    if (BTStationTreeInf.DispTotalItem == 0) //means no paired devices,we need to scan neighborhood
    {
        if (gBTFisrtScan == TRUE)
        {
            //DEBUG("### [BT] We need to request scan neighberhood ###");
            SendMsg(MSG_BT_SCAN_REQUEST);
        }
    }

    return;
}



/*
--------------------------------------------------------------------------------
 void BlueToothWinSvcStart(void *pArg)
  Author        : zhuzhe
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothWinSvcStart(void *pArg)
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
        //DEBUG("@@@ create bluetooth thread Success @@@");
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
 void BlueToothWinSvcStart(void *pArg)
  Author        : zhuzhe
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothWinSvcStop(void)
{
    ThreadDeleteAll(&pMainThread);
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothWinIntInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothWinIntInit(void *pArg)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothWinDeIntInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothWinIntDeInit(void *pArg)
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
void BluetoothMsgInit(void)
{
    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    ClearMsg(MSG_DIALOG_KEY_OK);
    ClearMsg(MSG_DIALOG_KEY_CANCEL);
    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
    ClearMsg(MSG_BLUETOOTH_WIN_DISCONNECTED); //�ڴ�����Ķ�����Ϣ��֪ͨ
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothWinMsgDeinit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BluetoothWinMsgDeInit(void *pArg)
{
    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothWinInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTControlSubWinInit()
{
    BtWinStatus = BT_WIN_STATUS_IDLE;
    BtWinBtA2dpDisconnectTickEnable = 0;
    gBTFisrtScan = TRUE;
    //DEBUG("Enter BTControlSubWinInit\n");
    BluetoothMsgInit();
    KeyReset();
    StationItemInfoInit();
    StationUpProc();
    BluetoothWinSvcStart(NULL);
    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    //DEBUG("Leaving BTControlSubWinInit\n");
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothWinDeInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTControlSubWinDeInit(void)
{
    //BlueToothWinSvcStop();
    //ModuleOverlay(MODULE_ID_SYSRESERVED_OP, MODULE_OVERLAY_ALL);
    //SaveSysInformation();
    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
}

//uint32 systimer = 0;
/*
--------------------------------------------------------------------------------
  Function name : UINT32 BlueToothWinService(void)
  Author        :
  Description   : it is radio service handle that is used to handle window message,window service task etc.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG

--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTControlSubWinService(void)
{
    UINT32 RetVal = 0;
    TASK_ARG TaskArg;

    /*
        uint32 curtick = GetSysTick();
        if (curtick - systimer > 200)
        {
            DEBUG("system tick=%d, bit pool=%d\n", curtick, avdtp_get_sbc_bitpool());
            systimer = curtick;
        }
    */
    //DEBUG("enter BlueToothWinService\n");
    if(BtScanWinOpened == 0)
    {
        if(GetMsg(MSG_BT_HCI_CANNEL_CONNECT_OK))
        {
            BtWinStatus = BT_WIN_STATUS_IDLE;
            TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
            TaskArg.Message.ContentID = SID_BT_CONNECT_CANCEL_COMPLETE + SID_OFFSET;
            TaskArg.Message.HoldTime  = 1;
            WinCreat(&BTControlSubWin, &MessageBoxWin, &TaskArg); //show cannel connect ok
        }

        if(GetMsg(MSG_BT_HCI_CANNEL_CONNECT_FAIL_CONNECED_EXIT))
        {
            BtWinStatus = BT_WIN_STATUS_HCI_CONNECT_CANNELING;
            bt_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac);
        }

        if(GetMsg(MSG_BT_SCAN_REQUEST) == TRUE)
        {
            //DEBUG("## [BT] Create scan win to scan...##");
            WinCreat(&BTControlSubWin, &BlueToothScanWin, NULL);
        }

        if (GetMsg(MSG_BLUETOOTH_CONNECT_SUCCEED))
        {
            TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
            TaskArg.Message.ContentID = SID_BT_CONNECTING_OK + SID_OFFSET;
            TaskArg.Message.HoldTime  = 1;
            WinCreat(&BTControlSubWin, &MessageBoxWin, &TaskArg);
            //BT_DEBUG("MSG_BLUETOOTH_CONNECT_SUCCEED\n");
        }

        if (GetMsg(MSG_BLUETOOTH_CONNECT_FAIL))
        {
            TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
            TaskArg.Message.ContentID = SID_BT_CONNECTING_FAIL + SID_OFFSET;
            TaskArg.Message.HoldTime  = 1;
            WinCreat(&BTControlSubWin, &MessageBoxWin, &TaskArg);
            //BT_DEBUG("MSG_BLUETOOTH_CONNECT_FAIL\n");
        }

        if (GetMsg(MSG_BLUETOOTH_WIN_DISCONNECTED))
        {
            BtWinBtA2dpDisconnectTickEnable = 0;
            if(BtWinStatus == BT_WIN_STATUS_ABORT_CONNECTING)
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_CONNECT_CANCEL_COMPLETE + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BTControlSubWin, &MessageBoxWin, &TaskArg); //show cannel connect is ok
            }
            else
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_DISCONNECT + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BTControlSubWin, &MessageBoxWin, &TaskArg); //show disconnect window
            }
            if (BtWinStatus == BT_WIN_STATUS_CONNECT_NEW_DEV)
            {
                BtWinStatus = BT_WIN_STATUS_CONNECTING;
                BtWinBtConncetTick = GetSysTick();
                SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                memcpy(gSysConfig.BtConfig.LastConnectMac, gSysConfig.BtConfig.BtLinkKey[BTStationTreeInf.CurId].BdAddr, 6);
                bt_a2dp_connect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, BluetoothConnectResult);
            }
            else
            {
                if ((BtWinStatus != BT_WIN_STATUS_BT_OFF_ING ) && (BtWinStatus != BT_WIN_STATUS_BT_GO_TO_OFF))
                {
                    BtWinStatus = BT_WIN_STATUS_IDLE;
                }
                else
                {
                    BtWinStatus = BT_WIN_STATUS_BT_GO_TO_OFF;
                }

                SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
            }
        }

        if (GetMsg(MSG_DIALOG_KEY_OK))
        {
            /*disconnect current device*/
            if (BtWinStatus == BT_WIN_STATUS_DISCONNECT_CUR_DEV)
            {
                if (gSysConfig.BtConfig.btConnected)
                {
                    gSysConfig.BtConfig.btConnected = 0;
                    //bt_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac);
                    BtWinBtA2dpDisconnectTick = GetSysTick();
                    BtWinBtA2dpDisconnectRetry = 2;
                    BtWinBtA2dpDisconnectTickEnable = 1;
                    bt_a2dp_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, BluetoothDisConnectResult);
                }
            }

            /*disconnect device and swtich connect a new device*/
            if (BtWinStatus == BT_WIN_STATUS_CONNECT_NEW_DEV)
            {
                if (gSysConfig.BtConfig.btConnected)
                {
                    gSysConfig.BtConfig.btConnected = 0;
                    //bt_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac);
                    BtWinBtA2dpDisconnectTick = GetSysTick();
                    BtWinBtA2dpDisconnectRetry = 2;
                    BtWinBtA2dpDisconnectTickEnable = 1;
                    bt_a2dp_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, BluetoothDisConnectResult);
                }
            }

            /*delete a paired device*/
            if (BtWinStatus == BT_WIN_STATUS_DELETE_DEV)
            {
                uint ItemTemp = BTStationTreeInf.CurId;

                if (gSysConfig.BtConfig.btConnected)
                {
                    if (memcmp(gSysConfig.BtConfig.LastConnectMac , gSysConfig.BtConfig.BtLinkKey[BTStationTreeInf.CurId].BdAddr, 6) == 0)
                    {
                        gSysConfig.BtConfig.btConnected = 0;
                        bt_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac);
                    }
                }

                BlueToothDeletePairedDve((struct bd_addr *)gSysConfig.BtConfig.BtLinkKey[BTStationTreeInf.CurId].BdAddr);
                StationUpProc();
                BtWinStatus = BT_WIN_STATUS_IDLE;
            }


            SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
        }

        if (GetMsg(MSG_DIALOG_KEY_CANCEL))
        {
             BtWinStatus = BT_WIN_STATUS_IDLE;
             SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
        }

//      if(GetMsg(MSG_BLUETOOTH_LINK_KEY_NOTIFY))   //�Ӵ�����ʱ��ͬʱ���и����ڵ�service ���� ,Ӱ���Ӵ���ˢ������ ���ŵ� key��ȥ
//      {
//          StationUpProc();
//          SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
//      }
    }

    if (GetMsg(MSG_MESSAGEBOX_DESTROY))
    {
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    }

    if (GetMsg(MSG_BLUETOOTH_SCAN_WIN_DESTROY))
    {
        StationUpProc();
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    }

    if (TRUE == GetMsg(MSG_BLUETOOTH_CONNECTING)) // �����Զ��������һ���豸
    {
        if (!gbBTConnected && gSysConfig.BtConfig.PairedDevCnt && BtWinStatus == BT_WIN_STATUS_IDLE )
        {
            BtWinStatus = BT_WIN_STATUS_CONNECTING;
            BtWinBtConncetTick = GetSysTick();
            SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
            bt_a2dp_connect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, BluetoothConnectResult);
        }
    }

    if (BT_WIN_STATUS_BT_OFF_ING ==  BtWinStatus)
    {
        uint systick;
        systick = GetSysTick();

        if (systick > BtWinBtOffTick + BT_OFF_TIME_OUT)
        {
            DEBUG("[BT] bt off time out ");
            BtWinStatus = BT_WIN_STATUS_IDLE;

            DEBUG("[BT] bt thread will delete... ");
            ThreadDelete(&pMainThread, &BlueToothThread);

            WinDestroy(&BTControlSubWin);
            SendMsg(MSG_SYS_SET_DISPLAY);

            //TaskArg.MainMenu.MenuID = MAINMENU_ID_BLUETOOTH;
            //TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            //RetVal = 1;
        }
    }

    if (BT_WIN_STATUS_CONNECTING ==  BtWinStatus && BtWinSubStatus != BT_WIN_SUB_STATUS_PIN_REQ
        && BtWinSubStatus != BT_WIN_SUB_STATUS_CONFIRMATION_REQ)
    {
        uint systick;
        systick = GetSysTick();

        if (systick > BtWinBtConncetTick + BT_CONNECT_TIME_OUT)
        {
            BtWinStatus = BT_WIN_STATUS_IDLE;
            //SendMsg(MSG_BLUETOOTH_ACL_DISCONNECTED);//ǿ�ƶϿ�����
            printf("BT_CONNECT_TIME_OUT,bt_disconnect\n");
            bt_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac);
            SendMsg(MSG_BLUETOOTH_CONNECT_FAIL);
        }
    }

    if (BT_WIN_STATUS_BT_GO_TO_OFF == BtWinStatus)
    {
        BtWinStatus = BT_WIN_STATUS_IDLE;
        ThreadDelete(&pMainThread, &BlueToothThread);

        //DEBUG("[BT] bt Subwin should destroyed here.");
        WinDestroy(&BTControlSubWin);
        SendMsg(MSG_SYS_SET_DISPLAY);

        //TaskArg.MainMenu.MenuID = MAI NMENU_ID_BLUETOOTH;
        //TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
        //RetVal = 1;
    }

    if (((BT_WIN_STATUS_DISCONNECT_CUR_DEV  == BtWinStatus) || (BT_WIN_STATUS_CONNECT_NEW_DEV  == BtWinStatus) ) && BtWinBtA2dpDisconnectTickEnable)
    {
        uint systick;
        int ret;
        systick = GetSysTick();

        if (systick > BtWinBtA2dpDisconnectTick + BT_OFF_TIME_OUT)
        {
            ret = bt_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac);
            BtWinBtA2dpDisconnectTick = GetSysTick();
            BtWinBtA2dpDisconnectRetry--;

            if (BtWinBtA2dpDisconnectRetry == 0)
            {
                BtWinBtA2dpDisconnectTickEnable = 0;
                SendMsg(MSG_BLUETOOTH_ACL_DISCONNECTED);//ǿ�ƶϿ�����
            }
        }
    }

    //DEBUG("Leaving BlueToothWinService\n");
    return RetVal;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BlueToothWinKeyProc(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTControlSubWinKeyProc(void)
{
    UINT32  TempKeyVal;
    UINT32  RetVal = 0;
    TASK_ARG TaskArg;
    TempKeyVal =  GetKeyVal();

    if (GetMsg(MSG_BLUETOOTH_LINK_KEY_NOTIFY))  //�Ӵ�����ʱ��ͬʱ���и����ڵ�service ���� ,Ӱ���Ӵ���ˢ������ ���ŵ� key��ȥ
    {
        StationUpProc();
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
    }

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
        case KEY_VAL_PLAY_LONG_UP:
        {
        }
        break;

        case KEY_VAL_MENU_PRESS:
        {
            if (BtWinStatus != BT_WIN_STATUS_CONNECT_NEW_DEV)
            {
                //DEBUG("[BT] menu long press,will destroyed bt sub win");
                WinDestroy(&BTControlSubWin);
                SendMsg(MSG_SYS_SET_DISPLAY);
            }

            break;
        }

        case KEY_VAL_MENU_SHORT_UP: //connect paried device / connect another paried device.
        {
            int i;
            struct hci_inq_res  *res = NULL;
            uint ItemTemp = BTStationTreeInf.CurId;// - BTStationTreeInf.KeyCounter;

            if (BtWinStatus == BT_WIN_STATUS_SCAN)
            {
                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
                TaskArg.Message.ContentID = SID_BT_SCANING + SID_OFFSET;
                TaskArg.Message.HoldTime  = 1;
                WinCreat(&BTControlSubWin, &MessageBoxWin, &TaskArg);
                break;
            }
            else if (BtWinStatus == BT_WIN_STATUS_CONNECTING)
            {
//                TaskArg.Message.TitleID   = SID_BLUETOOTH + SID_OFFSET;
//                TaskArg.Message.ContentID = SID_BT_CONNECTING + SID_OFFSET;
//                TaskArg.Message.HoldTime  = 1;
//                WinCreat(&BTControlSubWin, &MessageBoxWin, &TaskArg);
                if(BtWinStatus == BT_WIN_STATUS_CONNECTING && BtWinSubStatus == BT_WIN_SUB_STATUS_IDLE)
                {

                    if(hci_is_link((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, 0x01))
                    {
                        BtWinStatus = BT_WIN_STATUS_ABORT_CONNECTING;
                        bt_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac);
                    }
                    else
                    {
                        BtWinStatus = BT_WIN_STATUS_HCI_CONNECT_CANNELING;
                        hci_create_connection_cancel_command((struct bd_addr *)gSysConfig.BtConfig.BtLinkKey[BtWinBtConnectId].BdAddr);
                    }
                    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                }
                break;
            }
            else if(BtWinStatus == BT_WIN_STATUS_ABORT_CONNECTING)
            {
                break;
            }
            else if(BtWinStatus == BT_WIN_STATUS_HCI_CONNECT_CANNELING)
            {
                break;
            }
            else if(BtWinStatus != BT_WIN_STATUS_IDLE)
            {
                break;
            }

            if (gSysConfig.BtConfig.PairedDevCnt && BtWinStatus == BT_WIN_STATUS_IDLE)
            {
                if (!gSysConfig.BtConfig.btConnected && gSysConfig.BtConfig.PairedDevCnt)
                {
                    BtWinStatus = BT_WIN_STATUS_CONNECTING;
                    BtWinBtConncetTick = GetSysTick();
                    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                    memcpy(gSysConfig.BtConfig.LastConnectMac, gSysConfig.BtConfig.BtLinkKey[ItemTemp].BdAddr, 6);
                    bt_a2dp_connect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, BluetoothConnectResult);
                }
                else
                {
                    if (bd_addr_cmp((struct bd_addr *)gSysConfig.BtConfig.BtLinkKey[ItemTemp].BdAddr, (struct bd_addr *)gSysConfig.BtConfig.LastConnectMac))
                    {
                        BtWinStatus = BT_WIN_STATUS_DISCONNECT_CUR_DEV;
                    }
                    else
                    {
                        BtWinStatus = BT_WIN_STATUS_CONNECT_NEW_DEV;
                    }
                    BtWinBtConnectId = ItemTemp;
                    TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
                    TaskArg.Dialog.TitleID = SID_WARNING;
                    TaskArg.Dialog.ContentID = SID_BT_DISCONNECT_CUR_DEV;
                    WinCreat(&BTControlSubWin, &DialogWin, &TaskArg);
                }
            }

            break;
        }

        case KEY_VAL_FFD_SHORT_UP:  //show parid device info.
        {
            if (BtWinStatus == BT_WIN_STATUS_IDLE)
            {
                //DEBUG("[BT] we will create paired device win");
                WinCreat(&BTControlSubWin, &BTPariedDevInfoWin, NULL);
            }
        }
        break;

        case KEY_VAL_PLAY_SHORT_UP: //scan bt device
        {
            //Next
            //bt_scan(BlueToothScanResult);
            if (BtWinStatus == BT_WIN_STATUS_IDLE)
            {
                //DEBUG("[BT] we will create scansun win");
                WinCreat(&BTControlSubWin, &BlueToothScanWin, NULL);
            }

            break;
        }

        case KEY_VAL_FFW_SHORT_UP:  //delete parid device
        {
            //Prev
            if(BtWinStatus == BT_WIN_STATUS_IDLE)
            {
                BtWinStatus =  BT_WIN_STATUS_DELETE_DEV;
                TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
                TaskArg.Dialog.TitleID = SID_WARNING;
                TaskArg.Dialog.ContentID = SID_BT_DELETE_DEV;
                WinCreat(&BTControlSubWin, &DialogWin, &TaskArg);
            }
            break;
        }

        case KEY_VAL_UP_SHORT_UP:
            BTStationTreeInf.PreCounter = BTStationTreeInf.KeyCounter;

            if (BTStationTreeInf.CurId > 0)
            {
                BTStationTreeInf.CurId--;

                if (BTStationTreeInf.KeyCounter == 0)
                {
                    BTStationTreeInf.KeyCounter = MAX_STATION_NUM_LCD_DISP - 1;
                    StationUpProc();
                    SendMsg(MSG_BROW_DIS_ALL_ITEM);
                }
                else
                {
                    BTStationTreeInf.KeyCounter--;
                    SendMsg(MSG_BROW_DIS_SELE_ITEM);
                }
            }
            else
            {
                BTStationTreeInf.CurId = BTStationTreeInf.DispTotalItem - 1;

                if (BTStationTreeInf.DispTotalItem > MAX_STATION_NUM_LCD_DISP)
                {
                    if (BTStationTreeInf.DispTotalItem % MAX_STATION_NUM_LCD_DISP)
                    {
                        BTStationTreeInf.KeyCounter = BTStationTreeInf.DispTotalItem % MAX_STATION_NUM_LCD_DISP - 1;
                    }
                    else
                    {
                        BTStationTreeInf.KeyCounter =  MAX_STATION_NUM_LCD_DISP - 1;
                    }

                    StationUpProc();
                    SendMsg(MSG_BROW_DIS_ALL_ITEM);
                }
                else
                {
                    BTStationTreeInf.KeyCounter = BTStationTreeInf.DispTotalItem - 1;
                    SendMsg(MSG_BROW_DIS_SELE_ITEM);
                }
            }

            break;

        case KEY_VAL_DOWN_SHORT_UP:
            BTStationTreeInf.PreCounter = BTStationTreeInf.KeyCounter;

            if (BTStationTreeInf.CurId < (BTStationTreeInf.DispTotalItem - 1))
            {
                BTStationTreeInf.CurId++;

                if (BTStationTreeInf.KeyCounter >= MAX_STATION_NUM_LCD_DISP - 1)
                {
                    BTStationTreeInf.KeyCounter = 0;
                    StationUpProc();
                    SendMsg(MSG_BROW_DIS_ALL_ITEM);//display all screen
                }
                else
                {
                    BTStationTreeInf.KeyCounter++;
                    SendMsg(MSG_BROW_DIS_SELE_ITEM);
                }
            }
            else
            {
                BTStationTreeInf.CurId = 0;
                BTStationTreeInf.KeyCounter = 0;

                if (BTStationTreeInf.DispTotalItem > MAX_STATION_NUM_LCD_DISP)
                {
                    StationUpProc();
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
//            if (!gSysConfig.BtConfig.btConnected)
            {
//                DEBUG("[BT] delete bt thread add tasks witch");
//                ThreadDelete(&pMainThread, &BlueToothThread);

                WinDestroy(&BTControlSubWin);
                SendMsg(MSG_SYS_SET_DISPLAY);
            }
//            else
//            {
//                DEBUG("[BT] disconnect bt");
//                BtWinStatus = BT_WIN_STATUS_BT_OFF_ING;
//                BtWinBtOffTick = GetSysTick();
//                gSysConfig.BtConfig.btConnected = 0;
//                bt_a2dp_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, BluetoothDisConnectResult);
//            }

            break;
        }

        case KEY_VAL_ESC_PRESS:
        {
            if (BtWinStatus != BT_WIN_STATUS_CONNECT_NEW_DEV)
            {
                //DEBUG("[BT] menu long press,will destroyed bt sub win");
                WinDestroy(&BTControlSubWin);
                SendMsg(MSG_SYS_SET_DISPLAY);
            }
        }
            break;

        default:
            break;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void BlueToothWinPaint(void)
  Author        : mlc
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_BT_CODE_
void BtStationScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
    PicturePartInfo     PicPartInfo;
    PicPartInfo.x       = pRect->x0;
    PicPartInfo.y       = pRect->y0;
    PicPartInfo.yoffset = 0;
    PicPartInfo.ysize   = CH_CHAR_YSIZE_12;
    PicPartInfo.pictureIDNump = ImageID;
    SetScrollStringInfo(pRect, PicPartInfo, pstr, Speed);
}

_ATTR_SYS_SET_BT_CODE_
void BTControlSubWinDisplay(void)
{
    UINT16  i, CurNum;
    UINT16  TempColor, TempBkColor, TempCharSize, TempTxtMode;
    UINT16  TotalItem;
    UINT16  *printfbuf;
    UINT32   x, y;
    LCD_RECT             r;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;
    BTSTATIONITEM   *pBro = BTStationTreeInf.pBTBro;
    //DEBUG("bluetooth win paint\n");

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (GetMsg(MSG_BLUETOOTH_DISPLAY_ALL))
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
            LCD_DispStringInRect(&r, &r, L"DEV LIST", LCD_TEXTALIGN_CENTER);
        }
        else if (BtWinStatus == BT_WIN_STATUS_CONNECTING)
        {
            LCD_DispStringInRect(&r, &r, L"CONNECTING", LCD_TEXTALIGN_CENTER);
        }
        else if (BtWinStatus == BT_WIN_STATUS_CONNECT_NEW_DEV || BtWinStatus == BT_WIN_STATUS_DISCONNECT_CUR_DEV)
        {
            LCD_DispStringInRect(&r, &r, L"DISCONNECTING", LCD_TEXTALIGN_CENTER);
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
        GetPictureInfoWithIDNum(IMG_ID_BLUETOOTH_DEV_ICON, &PicInfo1);

        for (i = 0; i < MAX_STATION_NUM_LCD_DISP; i++)
        {
            if (BTStationTreeInf.DispTotalItem == 0)
            {
                break;
            }

            if ((i == BTStationTreeInf.PreCounter) || (i == BTStationTreeInf.KeyCounter))
            {
                //icon
                if (BluetoothIsCurConnectDev(pBro->LongFileName))
                {
                    DEBUG("# IsCurrrent connect Dev,** Select **  i = %d #", i);
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_ITEMBARSEL0, PicInfo.x, 5 + 17 * i);
                }

                //item
                if (i == BTStationTreeInf.PreCounter)
                {
                    DEBUG("@ i = %d No Select@", i);
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_ITEMBARNOSEL0, PicInfo.x, 5 + i * 17);
                }

                r.x0 = PicInfo.x;
                r.y0 = 5 + 17 * i;
                r.x1 = r.x0 + PicInfo.xSize;
                r.y1 = r.y0 + CH_CHAR_YSIZE_12;
                //DEBUG("X0 = %d,y0 = %d x1 = %d y1 = %d",r.x0,r.y0,r.x1,r.y1);
                //TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);

                if (i == BTStationTreeInf.KeyCounter)
                {
                    //TextMode = LCD_SetTextMode(LCD_DRAWMODE_REV);
                    //DEBUG();
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

                LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);
                /*if (i == BTStationTreeInf.KeyCounter)
                {
                    LCD_SetTextMode(TextMode);
                }*/
            }

            pBro = pBro->pNext;
        }

        if (BTStationTreeInf.DispTotalItem > MAX_STATION_NUM_LCD_DISP)
        {
            SendMsg(MSG_BROW_DIS_SCLLO_ITEM);
        }
    }

    if (GetMsg(MSG_BROW_DIS_ALL_ITEM))
    {
        //DispPictureWithIDNum(IMG_ID_BROWSER_CONTENTBACKGROUND);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_ITEMBARSEL0, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BLUETOOTH_DEV_ICON, &PicInfo1);
        TotalItem = BTStationTreeInf.DispTotalItem - (BTStationTreeInf.CurId - BTStationTreeInf.KeyCounter);

        for (i = 0; ((i < MAX_STATION_NUM_LCD_DISP) && (i < TotalItem )); i++)
        {
            {
                dumpMemoryShortA2String(pBro->LongFileName, 32);
            }

            //icon
            if (BluetoothIsCurConnectDev(pBro->LongFileName))
            {
                //DEBUG("# Is Currrent connected Dev #");
                DispPictureWithIDNumAndXY(IMG_ID_BLUETOOTH_PARIED_DEV, PicInfo1.x + 5, 5 + 17 * i);
            }
            else
            {
                //DEBUG("# NOT connected Dev #");
                DispPictureWithIDNumAndXY(IMG_ID_BLUETOOTH_DEV_ICON, PicInfo1.x + 5, 5 + 17 * i);
            }

            r.x0 = PicInfo.x;
            r.y0 = 5 + 17 * i;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0 + CH_CHAR_YSIZE_12;
            //TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);

            if (i == BTStationTreeInf.KeyCounter)
            {
                //DEBUG("&& i = %d &&", i);
                //TextMode = LCD_SetTextMode(LCD_DRAWMODE_REV);
                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_ITEMBARSEL0, PicInfo.x, 5 + i * 17);

                if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                {
                    BtStationPrintBuf = pBro->LongFileName;
                    BtStationScrollInit(&r, IMG_ID_BROWSER_ITEMBARSEL0, pBro->LongFileName, 30);
                    SendMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                    //DEBUG("$$$ SCROLL filename $$$");
                }
                else
                {
                    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                }
            }

            LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);
            /*if (i == BTStationTreeInf.KeyCounter)
            {
                LCD_SetTextMode(TextMode);
            }*/
            pBro = pBro->pNext;
        }

        if (BTStationTreeInf.DispTotalItem > MAX_STATION_NUM_LCD_DISP)
        {
            SendMsg(MSG_BROW_DIS_SCLLO_ITEM);
        }
    }

    //slider block display..
    if (GetMsg(MSG_BROW_DIS_SCLLO_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCROLLBACKGROUND, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_SETMENU_SCROLL1, &PicInfo1);
        CurNum = BTStationTreeInf.CurId;

        if (CurNum > 0)
        {
            y = (PicInfo.ySize - (PicInfo1.ySize << 1)) * CurNum / (BTStationTreeInf.DispTotalItem - 1);
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
        ScrollStringCommon(BtStationPrintBuf);
        //LCD_SetTextMode(TextMode);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

#endif //_A2DP_SOUCRE_



/*
--------------------------------------------------------------------------------
  Function name : UINT16 BTSetMenuFunSwitch (UINT16 iSelectedItem)
  Author        :
  Description   : record file format.

  Input         :
  Return        :

  History:     <author>         <time>         <version>

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTSetMenuFunSwitch (UINT32 iSelectedItem)
{
    UINT32 retVal = 0;
    //SetBTSwitch = iSelectedItem;
    return retVal;
}

_ATTR_SYS_SET_BT_CODE_
void BTFunSwitchOff()
{
    DEBUG("[BT] BTFunSwitchOff IsBTConnected = %d ", gbBTConnected);
    DEBUG("[BT] BTFunSwitchOff BT status = %s ", (gbBTSwitchOn == BT_ON) ? "Open" : "Closed");

    if (gbBTConnected)
    {
        gbBTConnected = 0;
        gbBTSwitchOn = BT_OFF;
        IsBTOpened = FALSE;
        bt_a2dp_disconnect((struct bd_addr *)gSysConfig.BtConfig.LastConnectMac, BluetoothDisConnectResult);
    }
    else
    {
        gbBTSwitchOn = BT_OFF;
        IsBTOpened = FALSE;
    }

    if (TRUE == ThreadCheck(pMainThread, &BlueToothThread))
    {
        ThreadDelete(&pMainThread, &BlueToothThread);
    }

    DEBUG("[BT] BT SWITCH OFF ");
}

#endif
#endif
