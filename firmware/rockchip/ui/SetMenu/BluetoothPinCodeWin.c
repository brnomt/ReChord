/*
********************************************************************************
*                   Copyright (C),2004-2016, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:  BluetoothPinCodeWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             chad.ma
*    desc:    ORG.
********************************************************************************
*/

#define _IN_BLUETOOTH_PIN_CODE_WIN_

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
#include "hci.h"
#include "BluetoothPinCodeWin.h"

/*
********************************************************************************
*
*                         Structure Define
*
********************************************************************************
*/
_ATTR_SYS_SET_BT_CODE_
uint32 BT_GetValidPinCodeLen(uint8* pincode)
{
    uint32 len = 0;

    if(pincode == NULL)
        return len;

    while(*pincode++ != 0)
        len++;

    return len;
}


/*
--------------------------------------------------------------------------------
  Function name : void BTPinCodeWinInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTPinCodeWinInit(void * pArg)
{
    //DEBUG("[BT] BTPinCodeWinInit In");

    KeyReset();
    memset(gPinCode,0,BT_PIN_CODE_NUM_MAX + 1);
    memset(BTPinCodes,0,sizeof(BT_PINCODESs)*BT_PIN_CODE_NUM_MAX);

    SendMsg(MSG_BT_DISPLAY_PINCODE_INIT);
    SendMsg(MSG_BT_DISPLAY_PINCODE_UPDATE);
    SendMsg(MSG_BT_DISPLAY_PINCODE_TIPS);
    //DEBUG("[BT] BTPinCodeWinInit Out");
}

/*
--------------------------------------------------------------------------------
  Function name : void BTPinCodeWinDeInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTPinCodeWinDeInit()
{
    //DEBUG("BTPinCodeWinDeInit");
}

/*
--------------------------------------------------------------------------------
  Function name : void BTPinCodeWinService(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTPinCodeWinService()
{
    return(RETURN_OK);
}


/*
--------------------------------------------------------------------------------
  Function name : void BTPinCodeWinService(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTPinCodeWinKeyProc()
{
    UINT16 i,RetVal;
    UINT32 pincodeSetKeyVal;
    TASK_ARG TaskArg;

    RetVal = 0;

    pincodeSetKeyVal = GetKeyVal();

    //5 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
    }

    //6 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
    }

    switch (pincodeSetKeyVal)
    {
        case KEY_VAL_PLAY_SHORT_UP:
        {
            uint8 preXcor;
            preXcor = curPincodeXcorVal;

            DEBUG("curPincodeXcorVal = %d",curPincodeXcorVal);

            if(curPincodeXcorVal < BT_PIN_CODE_NUM_MAX)
            {
                BTPinCodes[preXcor].pin_val = curPincodeYcorVal;
                BTPinCodes[preXcor].changed = TRUE;

                if(curPincodeXcorVal < BT_PIN_CODE_NUM_MAX - 1)
                {
                    curPincodeXcorVal++;

                    if(BTPinCodes[preXcor].changed)
                    {
                        gPinCode[preXcor] = BTPinCodes[preXcor].pin_val + '0';
                        SendMsg(MSG_BT_DISPLAY_PINCODE_UPDATE);
                    }
                }
                else if(curPincodeXcorVal == BT_PIN_CODE_NUM_MAX - 1)
                {

                    gPinCode[preXcor] = BTPinCodes[preXcor].pin_val + '0';
                    SendMsg(MSG_BT_DISPLAY_PINCODE_UPDATE);
                    curPincodeXcorVal++;
                }
            }
            else
            {//store pin code and destroy pincode win

                curPincodeXcorVal = 0;
                curPincodeYcorVal = 0;

                dumpMemoryCharA(gPinCode, 8);
                SendMsg(MSG_NEED_PAINT_ALL);
                WinDestroy(&BTPinCodeWin);
                RetVal = 1;
            }

            break;
        }

        case KEY_VAL_UP_SHORT_UP:
        {
            if(curPincodeYcorVal > 0)
                curPincodeYcorVal--;
            else if(curPincodeYcorVal == 0)
                curPincodeYcorVal = 9;

            SendMsg(MSG_NEED_PAINT_ALL);
            break;
        }

        case KEY_VAL_DOWN_SHORT_UP:
        {
            if(curPincodeYcorVal < 9)
                curPincodeYcorVal++;
            else if(curPincodeYcorVal == 9)
                curPincodeYcorVal = 0;

            SendMsg(MSG_NEED_PAINT_ALL);
            break;
        }

        case KEY_VAL_FFW_SHORT_UP: //exit key
        {
            uint8 preXcor;

            preXcor = curPincodeXcorVal;
            if (0 < curPincodeXcorVal)
            {
                BTPinCodes[preXcor].changed = FALSE;

                curPincodeXcorVal--;
                gPinCode[preXcor] = 0; //delete pin code
                SendMsg(MSG_BT_DISPLAY_PINCODE_UPDATE);
            }
            else
            {
                for(i = 0; i < BT_PIN_CODE_NUM_MAX; i++)//backup after exiting
                {
                }

                curPincodeXcorVal = 0;
                gPinCode[preXcor] = 0; //delete pin code
                curPincodeYcorVal = 0;
                SendMsg(MSG_NEED_PAINT_ALL);
                WinDestroy(&BTPinCodeWin);
                RetVal = 1;
            }
            break;
        }

        case KEY_VAL_MENU_SHORT_UP: //confirmed and close pin code win.
            dumpMemoryCharA(gPinCode, 8);

            curPincodeXcorVal = 0;
            curPincodeYcorVal = 0;
            SendMsg(MSG_BT_INPUT_PINCODE_OK);
            SendMsg(MSG_NEED_PAINT_ALL);
            WinDestroy(&BTPinCodeWin);
            RetVal = 1;

            break;

        case KEY_VAL_ESC_SHORT_UP: //confirmed and close pin code win.
            dumpMemoryCharA(gPinCode, 8);

            curPincodeXcorVal = 0;
            curPincodeYcorVal = 0;
            SendMsg(MSG_BT_INPUT_PINCODE_CANCLE);
            SendMsg(MSG_NEED_PAINT_ALL);
            WinDestroy(&BTPinCodeWin);
            RetVal = 1;

            break;

        case KEY_VAL_HOLD_ON:
            break;

        case KEY_VAL_HOLD_OFF:
            break;

        default:
            break;
    }


    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void BTPinCodeWinService(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTPinCodeTipsBuf[50];

_ATTR_SYS_SET_BT_CODE_
void BTPinCodeWinPaint()
{
    UINT16 i,RetVal;
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

    RetVal = 0;
    if (CheckMsg(MSG_NEED_PAINT_ALL) )
    {
        SendMsg(MSG_BT_DISPLAY_PINCODE_INIT);
        SendMsg(MSG_BT_DISPLAY_PINCODE_UPDATE);
        SendMsg(MSG_BT_DISPLAY_PINCODE_TIPS);
    }

    if (GetMsg(MSG_BT_DISPLAY_PINCODE_INIT))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_MDB_BACKGROUND, 0 , 0);

        for(i = 0; i < BT_PIN_CODE_NUM_MAX; i++)
        {
            if (i == curPincodeXcorVal)
            {
                DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER_SELECT, i*15, 28);
            }
            else
            {
                DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER_BACKGROUND, i*15 + 2, 30);
            }
        }

        DispPictureWithIDNumAndXYoffset(IMG_ID_SETMENU_ICON_SEL,38,76);

        for(i = 0;i < 3;i++)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER0 + (curPincodeYcorVal + i + 9)%10, 58, i * 16 + 90);
        }
    }

    if (GetMsg(MSG_BT_DISPLAY_PINCODE_UPDATE))
    {
        for(i = 0; i < BT_PIN_CODE_NUM_MAX; i++)
        {
            if (i == curPincodeXcorVal)
            {
                //select image
                DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER_SELECT, i*15 , 28);
                //background
                DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER_BACKGROUND, i*15+2, 30);
                if(BTPinCodes[i].changed)
                {
                    //number
                    DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER0 + BTPinCodes[i].pin_val, i*15+3, 31);
                }
            }
            else
            {
                //clear select image
                DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER_SELECT_BACKGROUND, i*15 , 28);

                DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER_BACKGROUND, i*15+2 , 30);

                if(BTPinCodes[i].changed)
                {
                    DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER0 + BTPinCodes[i].pin_val, i*15+3, 31);
                }
            }
        }

        DispPictureWithIDNumAndXYoffset(IMG_ID_SETMENU_ICON_SEL,38,76);

        for(i = 0;i < 3;i++)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_NUMBER0 + (curPincodeYcorVal + i + 9)%10, 58, i * 16 + 90);
        }
    }

    if (GetMsg(MSG_BT_DISPLAY_PINCODE_TIPS))
    {
        uint32 len;
        char * tips = "Please input 4 ~ 10 pin code.";

        memset(BTPinCodeTipsBuf,0,sizeof(BTPinCodeTipsBuf)*2);
        len = mb2wcBuf(tips,BTPinCodeTipsBuf);
        if(len > 50)
        {
            BTPinCodeTipsBuf[49] = 0;
            DEBUG("!!!ERROR!!! tips string exceed the  BTPinCodeTipsBuf limit");
        }

        r.x0 = 2;
        r.x1 = 127;
        r.y0 = 60 ;
        r.y1 = 84;

        LCD_SetColor(COLOR_RED);
        LCD_DispStringInRect(&r, &r, BTPinCodeTipsBuf, LCD_TEXTALIGN_CENTER);
        LCD_SetColor(COLOR_BLACK);
    }
}

//------------------------------------------------------------------------------

/*
--------------------------------------------------------------------------------
  Function name : void BTPinKeyCfmWinInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTPinKeyCfmWinInit(void * pArg)
{
    //DEBUG("[BT] BTPinKeyCfmWinInit In");

    pinKeyButtonSel  = ((DIALOG_WIN_ARG*)pArg)->Button;
    pinKeyTitleID    = ((DIALOG_WIN_ARG*)pArg)->TitleID;
    pinKeyContentID  = ((DIALOG_WIN_ARG*)pArg)->ContentID;

    KeyReset();

    SendMsg(MSG_BT_DISPLAY_PINKEY_ALL);
    //DEBUG("[BT] BTPinKeyCfmWinInit Out");
}

/*
--------------------------------------------------------------------------------
  Function name : void BTPinKeyCfmWinDeInit(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
void BTPinKeyCfmWinDeInit()
{
    KeyReset();
    //DEBUG("BTPinKeyCfmWinDeInit");
}

/*
--------------------------------------------------------------------------------
  Function name : void BTPinKeyCfmWinService(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTPinKeyCfmWinService()
{
    INT16 Retval = 0;

    return(Retval);
}

/*
--------------------------------------------------------------------------------
  Function name : void BTPinKeyCfmWinKeyProc(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BT_CODE_
UINT32 BTPinKeyCfmWinKeyProc()
{
    UINT32 RetVal = 0;
    UINT32 KeyTemp;
    TASK_ARG TaskArg;
    KeyTemp =  GetKeyVal();

    switch (KeyTemp)
    {
    case KEY_VAL_PLAY_SHORT_UP:
    case KEY_VAL_MENU_SHORT_UP:
        if (pinKeyButtonSel == BT_PIN_KEY_BUTTON_YES)
        {
            SendMsg(MSG_BT_CONFIRM_PINCODE_OK);
        }
        else
        {
            SendMsg(MSG_BT_CONFIRM_PINCODE_NO_OK);
        }
        SendMsg(MSG_NEED_PAINT_ALL);
		WinDestroy(&BTPinKeyCfmWin);
        RetVal = 1;
        break;

    #if (KEY_NUM == KEY_NUM_5)
    case KEY_VAL_DOWN_SHORT_UP:
    case KEY_VAL_UP_SHORT_UP:
    #else
    case KEY_VAL_FFD_SHORT_UP:
    case KEY_VAL_FFW_SHORT_UP:
    #endif
        if (pinKeyButtonSel == BT_PIN_KEY_BUTTON_YES)
        {
            pinKeyButtonSel = BT_PIN_KEY_BUTTON_NO;
        }
        else
        {
            pinKeyButtonSel = BT_PIN_KEY_BUTTON_YES;
        }
        SendMsg(MSG_BT_DISPLAY_PINKEY_BUTTON);
        break;

    case KEY_VAL_HOLD_ON:
        TaskArg.Hold.HoldAction = HOLD_STATE_ON;
        WinCreat(&BTPinKeyCfmWin, &HoldWin, &TaskArg);
        break;

    case KEY_VAL_HOLD_OFF:
        TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
        WinCreat(&BTPinKeyCfmWin, &HoldWin, &TaskArg);
        break;

    default:
        break;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void BTPinKeyCfmWinPaint(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
extern uint32 g_remote_numeric_value;
extern struct hci_inq_res BtWaitConnectRes;

_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  BTPinKeyTipsBuf[80];
_ATTR_SYS_SET_BT_CODE_
void BTPinKeyCfmWinPaint()
{
    PICTURE_INFO_STRUCT     PicInfo;//it will be use resource location and txt.
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode;
    LCD_RECT r0,r1,r2,r3;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_BT_DISPLAY_PINKEY_ALL)))
    {
        //display backgroud and dialog title and content.

        uint32 len;
        uint8 pairngDevName[33];
        char  pinkeyBuff[8] = {0};
        char* pairingDev    = "Pairing Device:";
        char* pinkeyContent = "Pls make sure the pair Pin Key is:";

        DispPictureWithIDNumAndXYoffset(IMG_ID_MDB_BACKGROUND,0,0);

        //paring device tips string
        memset(BTPinKeyTipsBuf,0,sizeof(BTPinKeyTipsBuf)*2);
        len = mb2wcBuf(pairingDev,BTPinKeyTipsBuf);
        if(len > 80)
        {
            BTPinKeyTipsBuf[79] = 0;
            DEBUG("!!!ERROR!!! tips string exceed the  BTPinCodeTipsBuf limit");
        }
        r0.x0 = 2;r0.x1 = 127;
        r0.y0 = 20 ; r0.y1 = 34;

        LCD_DispStringInRect(&r0, &r0, BTPinKeyTipsBuf, LCD_TEXTALIGN_LEFT);

        //paring device
        r1.x0 = 2; r1.x1 = 127;
        r1.y0 = 36; r1.y1 = 48;
        memset(BTPinKeyTipsBuf,0,sizeof(BTPinKeyTipsBuf)*2);

        memcpy(pairngDevName,BtWaitConnectRes.name,32);

        len = mb2wcBuf(pairngDevName,BTPinKeyTipsBuf);
        if(len > 80)
        {
            BTPinKeyTipsBuf[79] = 0;
            DEBUG("!!!ERROR!!! tips string exceed the  BTPinCodeTipsBuf limit");
        }
        LCD_SetColor(COLOR_RED);
        LCD_DispStringInRect(&r1, &r1, BTPinKeyTipsBuf, LCD_TEXTALIGN_LEFT);
        LCD_SetColor(COLOR_BLACK);

        //pin key tips string
        memset(BTPinKeyTipsBuf,0,sizeof(BTPinKeyTipsBuf)*2);
        len = mb2wcBuf(pinkeyContent,BTPinKeyTipsBuf);
        if(len > 80)
        {
            BTPinKeyTipsBuf[79] = 0;
            DEBUG("!!!ERROR!!! tips string exceed the  BTPinCodeTipsBuf limit");
        }
        r2.x0 = 2; r2.x1 = 127;
        r2.y0 = 50; r2.y1 = 74;
        LCD_DispStringInRect(&r2, &r2, BTPinKeyTipsBuf, LCD_TEXTALIGN_LEFT);

        //pin key number

        memset(BTPinKeyTipsBuf,0,sizeof(BTPinKeyTipsBuf)*2);
        len = sprintf(pinkeyBuff,"%d",g_remote_numeric_value);
        DEBUG("Len = %d,PIN KEY :## %d ##",len,g_remote_numeric_value);
        len = mb2wcBuf(pinkeyBuff,BTPinKeyTipsBuf);
        if(len > 80)
        {
            BTPinKeyTipsBuf[79] = 0;
            DEBUG("!!!ERROR!!! tips string exceed the  BTPinCodeTipsBuf limit");
        }
        r3.x0 = 2; r3.x1 = 127;
        r3.y0 = 80; r3.y1 = 92;

        LCD_SetColor(COLOR_RED);
        LCD_DispStringInRect(&r3, &r3, BTPinKeyTipsBuf, LCD_TEXTALIGN_CENTER);
        LCD_SetColor(COLOR_BLACK);

		//initial the other picture display mssage
		SendMsg(MSG_BT_DISPLAY_PINKEY_BUTTON);
    }

    if (TRUE == GetMsg(MSG_BT_DISPLAY_PINKEY_BUTTON))
    {
        if(pinKeyButtonSel)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_SEL01_ICON ,24,108);
            DispPictureWithIDNumAndXYoffset(IMG_ID_NOSEL00_ICON ,68,108);
        }
        else
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_SEL01_ICON ,68,108);
            DispPictureWithIDNumAndXYoffset(IMG_ID_NOSEL00_ICON ,24,108);
        }
        DisplayMenuStrWithIDNum(24, 108, 30, 12, LCD_TEXTALIGN_CENTER, SID_YES);//ȷ���ַ�
        DisplayMenuStrWithIDNum(68, 108, 30, 12, LCD_TEXTALIGN_CENTER, SID_NO);//ȡ���ַ�
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

#endif
#endif