/*
********************************************************************************
*                   Copyright (C),2004-2016, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  BluetoothDevInfoWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             chad.ma
*    desc:    ORG.
********************************************************************************
*/
#define _IN_BLUETOOTH_DEV_INFO_WIN_

#include "SysInclude.h"

#ifdef _BLUETOOTH_
#ifdef _A2DP_SOUCRE_

#include "SetBluetooth.h"
#include "BluetoothScanWin.h"
#include "BlueToothControl.h"

#include "mainmenu.h"
#include "Hold.h"

#include "SetCommon.h"
#include "SystemSet.h"

#include "MessageBox.h"
#include "bd_addr.h"
#include "BluetoothDevInfoWin.h"

/*
********************************************************************************
*
*                         Structure Define
*
********************************************************************************
*/
extern uint16 g_remote_a2dp_version;
extern uint16 g_remote_avrcp_version;
extern BTSTATIONLIST_TREE BTStationTreeInf;
/*
--------------------------------------------------------------------------------
  Function name : void BTLocalDevInfoWinInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTLocalDevInfoWinInit(void *pArg)
{
    //DEBUG("[BT] BluetoothDevInfoWinInit In");
    KeyReset();
    SendMsg(MSG_BT_SHOW_LOCAL_DEV_INFO);
    ClearMsg(MSG_BT_DEV_MAC_SCROLL);
    //DEBUG("[BT] BluetoothDevInfoWinInit Out");
}

/*
--------------------------------------------------------------------------------
  Function name : void BTLocalDevInfoWinDeInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTLocalDevInfoWinDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BTLocalDevInfoWinService(void)
  Author        :
  Description   : it is radio service handle that is used to handle window message,window service task etc.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG

--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTLocalDevInfoWinService(void)
{
    TASK_ARG TaskArg;
    return (RETURN_OK);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BTLocalDevInfoWinKeyProc(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTLocalDevInfoWinKeyProc(void)
{
    UINT16 RetVal;
    UINT32 userEqSetKeyVal;
    TASK_ARG TaskArg;
    userEqSetKeyVal = GetKeyVal();

    switch (userEqSetKeyVal)
    {
        case KEY_VAL_MENU_SHORT_UP: //return up level
        case KEY_VAL_ESC_SHORT_UP: //return up level
        case KEY_VAL_PLAY_SHORT_UP:
            WinDestroy(&BTLocalDevInfoWin);
            //DEBUG("[BT] destroy BT devInfoWin.");
            //SendMsg(MSG_BT_DEVINFO_WIN_DESTROY);
            SendMsg(MSG_SYS_SET_DISPLAY);
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&BTLocalDevInfoWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&BTLocalDevInfoWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return (RetVal);
}

_ATTR_SYS_SET_BT_CODE_
UINT32 mb2wcBuf(char* strbuf, uint16* wcBuf)
{
    UINT32 i= 0,inx = 0;
    UINT32 length = 0;

    if(strbuf == NULL || wcBuf == NULL)
        return 0;

    while(*strbuf != 0)
    {
        *wcBuf++ = *strbuf++;
        length++;
    }

    //DEBUG("strbuf length = %d",length);

    return length;
}

_ATTR_SYS_SET_BT_CODE_
void BtMacScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
    PicturePartInfo     PicPartInfo;
    PicPartInfo.x       = pRect->x0;
    PicPartInfo.y       = pRect->y0;
    PicPartInfo.yoffset = 0;
    PicPartInfo.ysize   = CH_CHAR_XSIZE_12;
    PicPartInfo.pictureIDNump = ImageID;
    SetScrollStringInfo(pRect, PicPartInfo, pstr, Speed);
}

//_ATTR_SYS_SET_BT_CODE_
_ATTR_BLUETOOTHCONTROL_CODE_
BOOL IsValidBtMacAddr(uint8* macAddr)
{
    uint8 *ptr;

    ptr = macAddr;

    if(ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 0 &&
        ptr[3] == 0 && ptr[4] == 0 && ptr[5] == 0)
        return FALSE;
    else if(ptr[0] == 0xff && ptr[1] == 0xff && ptr[2] == 0xff &&
        ptr[3] == 0xff && ptr[4] == 0xff && ptr[5] == 0xff)
        return FALSE;
    else
        return TRUE;
}

//_ATTR_SYS_SET_BT_CODE_
_ATTR_BLUETOOTHCONTROL_CODE_
char * BtMacAddr2String(uint8* macAddr)
{
    char mac_buff[20] = {0};
    uint8 i,j = 0;

    for(i = 0;i < 5;i++)
    {
        j += sprintf(mac_buff + j,"%02X:",macAddr[i]);
    }

    sprintf(mac_buff + j,"%02X",macAddr[5]);
    DEBUG("### LOCAL Mac is : %s ,j = %d ###",mac_buff,j);

    return mac_buff;
}

/*
--------------------------------------------------------------------------------
  Function name : void BluetoothDevInfoWinPaint(void)
  Author        : mlc
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
//Note: All below defined buffer can use STRING ID instead.
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevTitleBuf[30];

_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevNameBuf[20];
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevNameValueBuf[40];

_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevMacBuf[20];
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevMacValueBuf[40];

_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevProfileBuf[20];
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevA2DPValueBuf[40];
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevAVRCPValueBuf[40];

uint32 devInfo_Yoffset;
_ATTR_SYS_SET_BT_CODE_
void BTLocalDevInfoWinPaint(void)
{
    UINT16  i, CurNum;
    UINT16  TempColor, TempBkColor, TempCharSize, TempTxtMode;
    UINT16  TotalItem;
    UINT16  *printfbuf;
    UINT32   x, y;
    LCD_RECT r;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (GetMsg(MSG_BT_SHOW_LOCAL_DEV_INFO) || (CheckMsg(MSG_NEED_PAINT_ALL)))
    {
        DispPictureWithIDNum(IMG_ID_MDB_BACKGROUND);
        //LCD_ClrSrc();

        //show info win title
        {
            uint32 len;
            char * title = "Device Info";
            memset(BTDevTitleBuf,0,sizeof(BTDevTitleBuf)*2);
            len = mb2wcBuf(title,BTDevTitleBuf);
            if(len > 30)
            {
                BTDevTitleBuf[29] = 0;
                DEBUG("!!!ERROR!!! title string exceed the  BTDevTitleBuf limit");
            }

            r.x0 = 2;
            r.x1 = 127;
            r.y0 = 146;
            r.y1 = 160;
            //devInfo_Yoffset = r.y1;

            LCD_SetColor(COLOR_RED);
            LCD_DispStringInRect(&r, &r, BTDevTitleBuf, LCD_TEXTALIGN_CENTER);
            LCD_SetColor(COLOR_BLACK);
        }

        //show device name
        {
            //device name item display
            //char * nameval;
            uint32 namelen,nameVal_len;
            memset(BTDevNameBuf,0,sizeof(BTDevNameBuf)*2);
            namelen = mb2wcBuf("Device Name:\n",BTDevNameBuf);
            if(namelen > 20)
            {
                BTDevNameBuf[19] = 0;
                DEBUG("!!!ERROR!!! title string exceed the BTDevNameBuf limit");
            }

            r.x0 = 2;
            r.x1 = 127;
            r.y0 = 10;
            r.y1 = 24;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevNameBuf, LCD_TEXTALIGN_LEFT);

            //device name display
            //nameval = "AABBCC-DDEEFF_XX";//this value is fake
            memset(BTDevNameValueBuf,0,sizeof(BTDevNameValueBuf)*2);

            //nameVal_len = mb2wcBuf(nameval,BTDevNameValueBuf);
            nameVal_len = mb2wcBuf(&(gSysConfig.BtConfig.LocalBTName[0]),BTDevNameValueBuf);
            if(nameVal_len > 40)
            {
                DEBUG("!!!ERROR!!! title string exceed the BTDevNameValueBuf limit");
            }

            r.x0 = 8;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 16;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevNameValueBuf, LCD_TEXTALIGN_LEFT);
        }

        //show device mac addr
        {
            //mac item display
            char * mac_addr;
            uint32 macstrlen,macstrVal_len;
            memset(BTDevMacBuf,0,sizeof(BTDevMacBuf)*2);
            macstrlen = mb2wcBuf("Device MAC:\n",BTDevMacBuf);
            if(macstrlen > 20)
            {
                BTDevMacBuf[19] = 0;
                DEBUG("!!!ERROR!!! title string exceed the BTDevMacBuf limit");
            }

            r.x0 = 2;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 14;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevMacBuf, LCD_TEXTALIGN_LEFT);


            //mac addr string display

            memset(BTDevMacValueBuf,0,sizeof(BTDevMacValueBuf)*2);

            //get Mac address
            if(IsValidBtMacAddr(gSysConfig.BtConfig.LocalBTMacAddr))
            {
                char *mac_buff;
                mac_buff= BtMacAddr2String(gSysConfig.BtConfig.LocalBTMacAddr);
                macstrVal_len = mb2wcBuf(mac_buff,BTDevMacValueBuf);
            }
            else
            {
                //show the fake data
                mac_addr = "11:22:33:aa:bb:cc"; //this value is fake
                macstrVal_len = mb2wcBuf(mac_addr,BTDevMacValueBuf);
            }
            //macstrVal_len = mb2wcBuf(mac_addr,BTDevMacValueBuf);
            if(macstrVal_len > 40)
            {
                BTDevMacValueBuf[39] = 0;
                DEBUG("!!!ERROR!!! title string exceed the BTDevMacValueBuf limit");
            }

            r.x0 = 8;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 16;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevMacValueBuf, LCD_TEXTALIGN_LEFT);
            if (LCD_GetStringSize(BTDevMacValueBuf) > ((r.x1 - r.x0) * (r.y1 - r.y0) / 12))
            {
                BtMacScrollInit(&r, NULL, BTDevMacValueBuf, 50);
                SendMsg(MSG_BT_DEV_MAC_SCROLL);
            }
        }

        //show support profile
        {
            //profile item diaplay
            uint32 itemlen,a2dp_len,avrcp_len;
            uint32 index;
            uint32 a2dp_ver;
            uint32 avrcp_ver;

            char str[8]         = {0};
            char a2dp_str[32]   = {0};
            char avrcp_str[32]  = {0};
            memset(BTDevProfileBuf,0,sizeof(BTDevProfileBuf)*2);
            itemlen = mb2wcBuf("Device Profile:\n",BTDevProfileBuf);
            if(itemlen > 20)
            {
                BTDevProfileBuf[19] = 0;
                DEBUG("!!!ERROR!!! title string exceed the  BTDevProfileBuf limit");
            }

            r.x0 = 2;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 14;;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevProfileBuf, LCD_TEXTALIGN_LEFT);

            ModuleOverlay(MODULE_ID_LWBT, MODULE_OVERLAY_CODE);

            //A2DP version info diaplay
            //a2dp_str = "A2DP Ver 1.2";     //this value is fake
            index = 0;
            index = sprintf(a2dp_str,"A2DP Ver:");

            a2dp_ver = a2dp_get_profile_versions();
            sprintf(str,"%x.%02x",a2dp_ver >> 8,a2dp_ver & 0xff);

            sprintf(a2dp_str + index,"%s",str);

            memset(BTDevA2DPValueBuf,0,sizeof(BTDevA2DPValueBuf)*2);
            a2dp_len = mb2wcBuf(a2dp_str,BTDevA2DPValueBuf);
            if(a2dp_len > 40)
            {
                BTDevA2DPValueBuf[39] = 0;
                DEBUG("!!!ERROR!!! title string exceed the  BTDevA2DPValueBuf limit");
            }

            r.x0 = 8;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 14;;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevA2DPValueBuf, LCD_TEXTALIGN_LEFT);

            //AVCTP version info diaplay
            //avrcp_str = "AVRCP Ver 1.4";     //this value is fake

            index = 0;
            index = sprintf(avrcp_str,"AVRCP Ver:");

            avrcp_ver = avrcp_get_profile_versions();
            memset(str,0,sizeof(str));
            sprintf(str,"%x.%02x",avrcp_ver >> 8,avrcp_ver & 0xff);

            sprintf(avrcp_str + index,"%s",str);

            memset(BTDevAVRCPValueBuf,0,sizeof(BTDevAVRCPValueBuf)*2);
            avrcp_len = mb2wcBuf(avrcp_str,BTDevAVRCPValueBuf);
            if(avrcp_len > 40)
            {
                BTDevAVRCPValueBuf[39] = 0;
                DEBUG("##ERROE## title string exceed the  BTDevAVRCPValueBuf limit");
            }

            r.x0 = 8;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 14;;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevAVRCPValueBuf, LCD_TEXTALIGN_LEFT);
        }
    }

    if (CheckMsg(MSG_BT_DEV_MAC_SCROLL))
    {
        ScrollStringCommon(BTDevMacValueBuf);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

////////////////////////////////////////////////////////////////////////////////

/*
--------------------------------------------------------------------------------
  Function name : void BTPariedDevInfoWinInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTPariedDevInfoWinInit(void *pArg)
{
    //DEBUG("[BT] BTPariedDevInfoWinInit In");
    KeyReset();
    SendMsg(MSG_BT_SHOW_PARIED_DEV_INFO);
    ClearMsg(MSG_BT_DEV_MAC_SCROLL);
    //DEBUG("[BT] BTPariedDevInfoWinInit Out");
}

/*
--------------------------------------------------------------------------------
  Function name : void BTLocalDevInfoWinDeInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTPariedDevInfoWinDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BTLocalDevInfoWinService(void)
  Author        :
  Description   : it is radio service handle that is used to handle window message,window service task etc.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG

--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTPariedDevInfoWinService(void)
{
    TASK_ARG TaskArg;
    return (RETURN_OK);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BTLocalDevInfoWinKeyProc(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTPariedDevInfoWinKeyProc(void)
{
    UINT16 RetVal;
    UINT32 userEqSetKeyVal;
    TASK_ARG TaskArg;
    userEqSetKeyVal = GetKeyVal();

    switch (userEqSetKeyVal)
    {
        case KEY_VAL_MENU_SHORT_UP: //return up level
        case KEY_VAL_ESC_SHORT_UP: //return up level
        case KEY_VAL_PLAY_SHORT_UP:
            WinDestroy(&BTPariedDevInfoWin);
            //DEBUG("[BT] destroy BT devInfoWin.");
            //SendMsg(MSG_BT_DEVINFO_WIN_DESTROY);
            SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&BTPariedDevInfoWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&BTPariedDevInfoWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void BluetoothDevInfoWinPaint(void)
  Author        : mlc
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
//Note: All below defined buffer can use STRING ID instead.
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevTitleBuf[30];

_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevNameBuf[20];
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevNameValueBuf[40];

_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevMacBuf[20];
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevMacValueBuf[40];

_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevProfileBuf[20];
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevA2DPValueBuf[40];
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTDevAVRCPValueBuf[40];

uint32 devInfo_Yoffset;
_ATTR_SYS_SET_BT_CODE_
void BTPariedDevInfoWinPaint(void)
{
    UINT16  i, CurNum;
    UINT16  TempColor, TempBkColor, TempCharSize, TempTxtMode;
    UINT16  TotalItem;
    UINT16  *printfbuf;
    UINT32   x, y;
    LCD_RECT r;
    uint32  ItemTemp;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    ItemTemp = BTStationTreeInf.CurId;

    if (GetMsg(MSG_BT_SHOW_PARIED_DEV_INFO) || (CheckMsg(MSG_NEED_PAINT_ALL)))
    {
        DispPictureWithIDNum(IMG_ID_MDB_BACKGROUND);
        //LCD_ClrSrc();

        //show info win title
        {
            uint32 len;
            char * title = "Paired Dev Info";
            memset(BTDevTitleBuf,0,sizeof(BTDevTitleBuf)*2);
            len = mb2wcBuf(title,BTDevTitleBuf);
            if(len > 30)
            {
                BTDevTitleBuf[29] = 0;
                DEBUG("!!!ERROR!!! title string exceed the  BTDevTitleBuf limit");
            }

            r.x0 = 2;
            r.x1 = 127;
            r.y0 = 146;
            r.y1 = 160;
            //devInfo_Yoffset = r.y1;

            LCD_SetColor(COLOR_RED);
            LCD_DispStringInRect(&r, &r, BTDevTitleBuf, LCD_TEXTALIGN_CENTER);
            LCD_SetColor(COLOR_BLACK);
        }

        //show device name
        {
            //device name item display
            //char * nameval;
            uint32 namelen,nameVal_len;
            memset(BTDevNameBuf,0,sizeof(BTDevNameBuf)*2);
            namelen = mb2wcBuf("Device Name:\n",BTDevNameBuf);
            if(namelen > 20)
            {
                BTDevNameBuf[19] = 0;
                DEBUG("!!!ERROR!!! title string exceed the BTDevNameBuf limit");
            }

            r.x0 = 2;
            r.x1 = 127;
            r.y0 = 10;
            r.y1 = 24;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevNameBuf, LCD_TEXTALIGN_LEFT);

            //device name display
            //nameval = "AABBCC-DDEEFF_XX";//this value is fake
            memset(BTDevNameValueBuf,0,sizeof(BTDevNameValueBuf)*2);

            //nameVal_len = mb2wcBuf(nameval,BTDevNameValueBuf);
            nameVal_len = mb2wcBuf(&(gSysConfig.BtConfig.BtLinkKey[ItemTemp].name[0]),BTDevNameValueBuf);
            if(nameVal_len > 40)
            {
                DEBUG("!!!ERROR!!! title string exceed the BTDevNameValueBuf limit");
            }

            r.x0 = 8;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 16;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevNameValueBuf, LCD_TEXTALIGN_LEFT);
        }

        //show device mac addr
        {
            //mac item display
            char * mac_addr;
            uint32 macstrlen,macstrVal_len;
            memset(BTDevMacBuf,0,sizeof(BTDevMacBuf)*2);
            macstrlen = mb2wcBuf("Device MAC:\n",BTDevMacBuf);
            if(macstrlen > 20)
            {
                BTDevMacBuf[19] = 0;
                DEBUG("!!!ERROR!!! title string exceed the BTDevMacBuf limit");
            }

            r.x0 = 2;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 14;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevMacBuf, LCD_TEXTALIGN_LEFT);


            //mac addr string display

            memset(BTDevMacValueBuf,0,sizeof(BTDevMacValueBuf)*2);

            //show Mac address
            if(IsValidBtMacAddr(gSysConfig.BtConfig.BtLinkKey[ItemTemp].BdAddr))  //remote device mac is valid
            {
                char *mac_buff;

                mac_buff= BtMacAddr2String(gSysConfig.BtConfig.BtLinkKey[ItemTemp].BdAddr);
                macstrVal_len = mb2wcBuf(mac_buff,BTDevMacValueBuf);
            }
            else
            {
                //show the fake data
                mac_addr = "11:22:33:aa:bb:cc"; //this value is fake
                macstrVal_len = mb2wcBuf(mac_addr,BTDevMacValueBuf);
            }
            //macstrVal_len = mb2wcBuf(mac_addr,BTDevMacValueBuf);
            if(macstrVal_len > 40)
            {
                BTDevMacValueBuf[39] = 0;
                DEBUG("!!!ERROR!!! title string exceed the BTDevMacValueBuf limit");
            }

            r.x0 = 8;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 16;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevMacValueBuf, LCD_TEXTALIGN_LEFT);
            if (LCD_GetStringSize(BTDevMacValueBuf) > ((r.x1 - r.x0) * (r.y1 - r.y0) / 12))
            {
                BtMacScrollInit(&r, NULL, BTDevMacValueBuf, 50);
                SendMsg(MSG_BT_DEV_MAC_SCROLL);
            }
        }

        //show support profile
        {
            //profile item diaplay
            uint32 itemlen,a2dp_len,avrcp_len;
            uint32 index;
            uint32 a2dp_ver;
            uint32 avrcp_ver;

            char str[8]         = {0};
            char a2dp_str[32]   = {0};
            char avrcp_str[32]  = {0};
            memset(BTDevProfileBuf,0,sizeof(BTDevProfileBuf)*2);
            itemlen = mb2wcBuf("Device Profile:\n",BTDevProfileBuf);
            if(itemlen > 20)
            {
                BTDevProfileBuf[19] = 0;
                DEBUG("!!!ERROR!!! title string exceed the  BTDevProfileBuf limit");
            }

            r.x0 = 2;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 14;;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevProfileBuf, LCD_TEXTALIGN_LEFT);

            ModuleOverlay(MODULE_ID_LWBT, MODULE_OVERLAY_BSS|MODULE_OVERLAY_DATA);

            //A2DP version info diaplay
            //a2dp_str = "A2DP Ver 1.2";     //this value is fake
            index = 0;
            index = sprintf(a2dp_str,"A2DP Ver:");

            a2dp_ver = g_remote_a2dp_version;
            sprintf(str,"%x.%02x",a2dp_ver >> 8,a2dp_ver & 0xff);

            sprintf(a2dp_str + index,"%s",str);

            memset(BTDevA2DPValueBuf,0,sizeof(BTDevA2DPValueBuf)*2);
            a2dp_len = mb2wcBuf(a2dp_str,BTDevA2DPValueBuf);
            if(a2dp_len > 40)
            {
                BTDevA2DPValueBuf[39] = 0;
                DEBUG("!!!ERROR!!! title string exceed the  BTDevA2DPValueBuf limit");
            }

            r.x0 = 8;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 14;;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevA2DPValueBuf, LCD_TEXTALIGN_LEFT);

            //AVCTP version info diaplay
            //avrcp_str = "AVRCP Ver 1.4";     //this value is fake

            index = 0;
            index = sprintf(avrcp_str,"AVRCP Ver:");

            avrcp_ver = g_remote_avrcp_version;
            memset(str,0,sizeof(str));
            sprintf(str,"%x.%02x",avrcp_ver >> 8,avrcp_ver & 0xff);

            sprintf(avrcp_str + index,"%s",str);

            memset(BTDevAVRCPValueBuf,0,sizeof(BTDevAVRCPValueBuf)*2);
            avrcp_len = mb2wcBuf(avrcp_str,BTDevAVRCPValueBuf);
            if(avrcp_len > 40)
            {
                BTDevAVRCPValueBuf[39] = 0;
                DEBUG("!!!ERROR!!! title string exceed the  BTDevAVRCPValueBuf limit");
            }

            r.x0 = 8;
            r.x1 = 127;
            r.y0 = devInfo_Yoffset;
            r.y1 = devInfo_Yoffset + 14;;
            devInfo_Yoffset = r.y1;
            LCD_DispStringInRect(&r, &r, (UINT16*)BTDevAVRCPValueBuf, LCD_TEXTALIGN_LEFT);
        }
    }

    if (CheckMsg(MSG_BT_DEV_MAC_SCROLL))
    {
        ScrollStringCommon(BTDevMacValueBuf);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
********************************************************************************
*
*                         End of BluetoothDevInfoWin.c
*
********************************************************************************
*/
#endif
#endif

