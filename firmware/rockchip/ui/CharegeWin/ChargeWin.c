/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  ChargeWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_CHARGE_

#include "SysInclude.h"
#include "ChargeWin.h"
#include "mainmenu.h"

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuIntInit(void)
  Author        : ZHengYongzhi
  Description   : main menu interrupt initialization

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_CHARGE_WIN_CODE_
void CharegeWinInit(void *pArg)
{
    KeyReset();

    if (CheckMsg(MSG_CHARGE_START))
    {
        SendMsg(MSG_USB_DISPLAY_CHARGE);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuIntDeInit(void)
  Author        : ZHengYongzhi
  Description   : main menu interrupt auti-initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_CHARGE_WIN_CODE_
void ChargeWinDeInit(void)
{
    ClearMsg(MSG_CHARGE_START);
    UsbAdpterProbeStop();
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MainMenuService(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_CHARGE_WIN_CODE_
UINT32 ChargeWinService(void)
{
    uint32  RetVal;
    TASK_ARG TaskArg;

    RetVal = RETURN_OK;

    if (CheckMsg(MSG_CHARGE_START))
    {
        if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
        {
            SendMsg(MSG_USB_DISPLAY_CHARGE);
        }
    }

    if (CheckVbus() == 0)
    {
        TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);//enter main menu
        RetVal = 1;
        ClearMsg(MSG_USB_DISPLAY_CHARGE);
    }

    return(RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MainMenuKey(void)
  Author        : ZHengYongzhi
  Description   : main menu key handle function.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_CHARGE_WIN_CODE_
UINT32 ChargeWinKey(void)
{
    uint32 RetVal = RETURN_OK;
    UINT32 ChargeWinKeyVal;
    WIN    *pWin;
	TASK_ARG TaskArg;

    ChargeWinKeyVal =  GetKeyVal();

    //7 key function
    switch (ChargeWinKeyVal)
    {
    case KEY_VAL_MENU_DOWN:
        case KEY_VAL_PLAY_DOWN:
        case KEY_VAL_FFD_DOWN:
        case KEY_VAL_FFW_DOWN:
        case KEY_VAL_UP_DOWN:
        case KEY_VAL_DOWN_DOWN:
        case KEY_VAL_VOL_DOWN:
            TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);//enter main menu
            RetVal = 1;
            ClearMsg(MSG_USB_DISPLAY_CHARGE);
            break;

        default:
            break;
    }
    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuDisplay(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_CHARGE_WIN_CODE_
void CharegeWinDisplay(void)
{
    if (CheckMsg(MSG_NEED_PAINT_ALL))
    {
    }

    if (TRUE == GetMsg(MSG_USB_DISPLAY_CHARGE))
    {
        DispPictureWithIDNum(IMG_ID_CHARGELEVEL0 + gBattery.Batt_Level);
    }
}

/*
********************************************************************************
*
*                         End of ChargeWin.c
*
********************************************************************************
*/
