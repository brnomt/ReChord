/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  SetMenu.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_SETMENU_

#include "SysInclude.h"

#ifdef _SYSSET_

#include "SetCommon.h"
#include "SetMenu.h"
#include "SetMenuInterface.h"
#include "SystemSet.h"
#include "AudioControl.h"
#include "mainmenu.h"
#include "DialogBox.h"
#include "MessageBox.h"
#include "Hold.h"
#include "SetBluetooth.h"

_ATTR_SYS_SET_SERVICE_CODE_
UINT16 DisplayMenuRadioButtonPaint(UINT16 SelfID, UINT16 curItemID, UINT16 y_position, BOOL focus, BOOL selected)
{
    UINT16 offset;

    switch (SelfID)
    {
        case SID_MUSIC_SHUFFLE:
        case SID_MUSIC_REPEAT_MODE:
        case SID_EQ_SELECT:
        case SID_RADIO_SCAN_SENSITIVITY:
        case SID_COMMON_SCREEN_OFF_TIMER:
        case SID_LANGUAGE:
        case SID_COMMON_BRIGHTNESS:
            offset = 14;
            break;

        default:
            offset = 0;
            break;
    }

    if (offset == 0)
        return offset;

    if (curItemID == SID_EQ_CUSTOM_EDIT) // not display radio icon
        return offset;

    if (focus)
    {
        if (selected)
        {
            // DispPictureWithIDNumAndXY(IMG_ID_ICON_RADIO_BTN_SELECTED_ON_FOCUS, 0, (CH_CHAR_YSIZE_12 * y_position));
        }
        else
        {
            // DispPictureWithIDNumAndXY(IMG_ID_ICON_RADIO_BTN_ON_FOCUS, 0, (CH_CHAR_YSIZE_12 * y_position));
        }
    }
    else
    {
        if (selected)
        {
            //  DispPictureWithIDNumAndXY(IMG_ID_ICON_RADIO_BTN_SELECTED, 0, (CH_CHAR_YSIZE_12 * y_position));
        }
        else
        {
            //    DispPictureWithIDNumAndXY(IMG_ID_ICON_RADIO_BTN, 0, (CH_CHAR_YSIZE_12 * y_position));
        }
    }

    return offset;
}

/*
--------------------------------------------------------------------------------
  Function name : void SetMenuIntInit(void)
  Author        : anzhiguo
  Description   : ,

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_INIT_CODE_
void SetMenuIntInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void SetMenuIntDeInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo        2009-3-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_INIT_CODE_
void SetMenuIntDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : SetMenuMsgInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               anzhiguo        2009-3-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_INIT_CODE_
void SetMenuValueInit(void)
{
    UINT16 i;
    uint32 menuId;
    SETMENU_INFO_STRUCT * menuitem;
    ReqMenuTxtInfoStructFlag = 0;
    menuitem = SetMenuItem;
    menuitem->CurDispItem = 0;
    menuitem->CurDispItemFirst = 0;
    menuitem->PrevItem = 0;
    menuitem->FatherItemID = 0xffff;
    menuitem->SelItemInID = 0xff;
    menuitem->CurItemInID = 0;
    menuitem->SetMenuTxtInfo = (MENU_TEXT_INFO_STRUCT *)(&SetMenuTxtInfo[ReqMenuTxtInfoStructFlag %
                               SYS_SET_GLOBE_TXT_INFO_STRUCT_NUM]);
    menuId = SID_LANGUAGE;
#ifdef _RADIO_

    if (!gSysConfig.FMEnable)
    {
//        if(menuId > SID_REC_NORMAL_QUALITY)
        menuId -= 9;
    }

#else
//    if(menuId > SID_REC_NORMAL_QUALITY)
    menuId -= 9;
#endif

    for (i = 0; i < LanguageNum; i++) //HJ 9 languages included in the 11 languages
    {
        SetLanguageItem[i] = SetMenuLanguageInfo[i] + menuId + 1;
    }

    GetMenuTextInfoWithIDNum(SID_SETTINGS, menuitem->SetMenuTxtInfo);

    if (menuitem->SetMenuTxtInfo->ChildNum > SYS_SET_MAX_ITEM_PER_SCREEN)
    {
        menuitem->TotalDispItem = SYS_SET_MAX_ITEM_PER_SCREEN;
    }
    else
    {
        menuitem->TotalDispItem = menuitem->SetMenuTxtInfo->ChildNum;
    }

    SendMsg(MSG_SYS_SET_DISPLAY_SCR);//send scroll message
}
/*
--------------------------------------------------------------------------------
 Function name : SetMenuMsgInit(void)
 Author        : anzhiguo
 Description   :

 Input         :
 Return        :

 History:     <author>         <time>         <version>
              anzhiguo        2009-3-3         Ver1.0
 desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_INIT_CODE_
void SetMenuMsgInit(void)
{
    SendMsg(MSG_SYS_SET_DISPLAY);
}

/*
--------------------------------------------------------------------------------
  Function name : void SetMenuMsgDeInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo        2009-3-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_DEINIT_CODE_
void SetMenuMsgDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void SetMenuInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo        2009-3-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_CODE_
void SetMenuInit(void *pArg)
{
    SetMenuValueInit();
    SetMenuMsgInit();
    KeyReset();
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : void SetMenuDeInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo        2009-3-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_CODE_
void SetMenuDeInit(void)
{
    SetMenuIntDeInit();
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SetMenuService(void)
  Author        : anzhiguo
  Description   : it is used to window message,window service etc handle

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo        2009-3-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_SERVICE_CODE_
UINT32 SetMenuService(void)
{
    UINT16 retVal = 0;
    TASK_ARG TaskArg;

    if (GetMsg( MSG_DIALOG_KEY_OK))
    {
        if (GetMsg(MSG_FIRMWAREUPGRADE))//firemware update dialog
        {
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_PLEASE_INSERT_USB;
            TaskArg.Message.HoldTime  = 10;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            WinCreat(&SetMenuWin, &MessageBoxWin, &TaskArg);
            SendMsg(MSG_MES_FIRMWAREUPGRADE);//excute this message handle,when check out this message in system backgroud.
        }

        if (GetMsg(MSG_SYSDEFAULTVALSET))//system response the default value
        {
            retVal = 0;
            //restore system default handle
            SysSetDefaultValue();
            SetLightOffTimerReset();
            SetLcdBackLightOn();
            SetPowerOffTimerReset();
            //send refresh screen message when exit dilag by press dialog sure key
            SendMsg(MSG_SYS_SET_DISPLAY);
        }

        if (CheckMsg(MSG_FORMAT_FLASH))
        {
            //clear Dialogbox
            SendMsg(MSG_SYS_SET_DISPLAY);
            SetMenuPaint();

            //Display MessageBox
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_FORMATTING;
            TaskArg.Message.HoldTime = 1;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            MessageBoxHook = SysFormatFlash;
            WinCreat(&SetMenuWin, &MessageBoxWin, &TaskArg);
        }

        #ifdef _SDCARD_
        if (GetMsg(MSG_FORMAT_SDCARD))
        {
            //clear Dialogbox
            SendMsg(MSG_SYS_SET_DISPLAY);
            SetMenuPaint();

            //Display MessageBox
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_FORMATTING;
            TaskArg.Message.HoldTime = 1;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            MessageBoxHook = SysFormatSDcard;
            WinCreat(&SetMenuWin, &MessageBoxWin, &TaskArg);
        }
        #endif
    }

    if (GetMsg( MSG_DIALOG_KEY_CANCEL))
    {
        retVal = 0;
        GetMsg(MSG_FIRMWAREUPGRADE);//clear the firmware uptate message when key cancel.
        GetMsg(MSG_SYSDEFAULTVALSET);//clear the restore system default message when key cancel.
        SendMsg(MSG_SYS_SET_DISPLAY);
    }

    if (GetMsg(MSG_MESSAGEBOX_DESTROY))//clear the firmware uptate message when receive the message of message box close
    {
        GetMsg(MSG_MES_FIRMWAREUPGRADE);
        SendMsg(MSG_SYS_SET_DISPLAY);

        if (GetMsg(MSG_FORMAT_FLASH))
        {
            #ifdef _MEDIA_MODULE_
            gSysConfig.MedialibPara.MediaUpdataFlag = 1;
            #endif
            #ifdef AUDIOHOLDONPLAY
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0 ;
            #endif

            TaskArg.Mdb.TaskID = TASK_ID_MAINMENU;
            TaskSwitch(TASK_ID_MDB, &TaskArg);
            retVal = 1;
        }
    }

    return (retVal);
}


/*
--------------------------------------------------------------------------------
  Function name : UINT32 SetMenuKey(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo        2009-3-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_SERVICE_CODE_
UINT32 SetMenuKey(void)
{
    UINT32 setMenuKeyVal;
    UINT32 menuId;
    UINT16 FatherItemID;
    SETMENU_INFO_STRUCT * menuitem;
    TASK_ARG TaskArg;
    UINT32  retVal = 0;
    menuitem = (SETMENU_INFO_STRUCT *)(&SetMenuItem[ReqMenuTxtInfoStructFlag]);
    setMenuKeyVal = GetKeyVal();

    switch (setMenuKeyVal)
    {
        case KEY_VAL_UP_SHORT_UP:
            if (menuitem->SetMenuTxtInfo->ChildNum == 1)//the up/down key has no effect,when only one item.
                break;

            if (menuitem->CurItemInID == 0)
            {
                menuitem->CurItemInID = menuitem->SetMenuTxtInfo->ChildNum - 1; //set cursor item index to max

                if (menuitem->SetMenuTxtInfo->ChildNum > SYS_SET_MAX_ITEM_PER_SCREEN)
                {
                    menuitem->TotalDispItem    = (menuitem->CurItemInID % SYS_SET_MAX_ITEM_PER_SCREEN) + 1;
                    menuitem->CurDispItemFirst = menuitem->CurItemInID - menuitem->TotalDispItem + 1;
                    SendMsg(MSG_SYS_SET_DISPLAY_ALL);//full screen refresh
                }
                else
                {
                    menuitem->PrevItem = 0;
                    SendMsg(MSG_SYS_SET_DISP_REFRESH_CUR_PREV_ITEM);
                }

                menuitem->CurDispItem = menuitem->TotalDispItem - 1; //cursor point to the last item that can display
            }
            else
            {
                if (menuitem->CurDispItem == 0)
                {
                    menuitem->CurDispItem = SYS_SET_MAX_ITEM_PER_SCREEN - 1;
                    menuitem->CurDispItemFirst -= SYS_SET_MAX_ITEM_PER_SCREEN;
                    menuitem->TotalDispItem = SYS_SET_MAX_ITEM_PER_SCREEN;
                    SendMsg(MSG_SYS_SET_DISPLAY_ALL);
                }
                else
                {
                    menuitem->PrevItem = menuitem->CurDispItem --;
                    SendMsg(MSG_SYS_SET_DISP_REFRESH_CUR_PREV_ITEM);
                }

                menuitem->CurItemInID --;
            }

            menuId = menuitem->SetMenuTxtInfo->SelfID;
#ifdef _RADIO_

            if (!gSysConfig.FMEnable)
            {
                //if(menuId > SID_REC_NORMAL_QUALITY)
                menuId += 9;
            }

#else
            //if(menuId > SID_REC_NORMAL_QUALITY)
            menuId += 9;
#endif
#ifdef _MUSIC_

            if (SID_EQ_SELECT == menuId)
            {
                SetEqType = menuitem->CurItemInID;
                SendMsg(MSG_AUDIO_EQSET_UPDATA);
            }

#endif

            if (SID_COMMON_BRIGHTNESS == menuId)
            {
                SetBackLightLevel = menuitem->CurItemInID;
                BL_SetLevel(SetBackLightLevel);
            }

            //if(menuitem->SetMenuTxtInfo->ChildNum >menuitem->TotalDispItem)//an 4.22
            SendMsg(MSG_SYS_SET_DISPLAY_SCR);
            break;

        case KEY_VAL_DOWN_SHORT_UP:
            if (menuitem->SetMenuTxtInfo->ChildNum == 1)
                break;

            if (menuitem->CurItemInID == (menuitem->SetMenuTxtInfo->ChildNum - 1))
            {
                menuitem->CurDispItem = 0;
                menuitem->CurItemInID = 0;
                menuitem->CurDispItemFirst = 0;

                if (menuitem->SetMenuTxtInfo->ChildNum > SYS_SET_MAX_ITEM_PER_SCREEN)
                {
                    menuitem->TotalDispItem = SYS_SET_MAX_ITEM_PER_SCREEN;
                    SendMsg(MSG_SYS_SET_DISPLAY_ALL);
                }
                else
                {
                    menuitem->PrevItem = menuitem->SetMenuTxtInfo->ChildNum - 1;
                    SendMsg(MSG_SYS_SET_DISP_REFRESH_CUR_PREV_ITEM);
                }
            }
            else
            {
                if (menuitem->CurDispItem == (menuitem->TotalDispItem - 1))
                {
                    menuitem->CurDispItem = 0;
                    menuitem->CurDispItemFirst += SYS_SET_MAX_ITEM_PER_SCREEN ;

                    if ((menuitem->CurDispItemFirst + SYS_SET_MAX_ITEM_PER_SCREEN - 1) < menuitem->SetMenuTxtInfo->ChildNum)
                    {
                        menuitem->TotalDispItem = SYS_SET_MAX_ITEM_PER_SCREEN;
                    }
                    else
                    {
                        menuitem->TotalDispItem = menuitem->SetMenuTxtInfo->ChildNum - menuitem->CurDispItemFirst;
                    }

                    SendMsg(MSG_SYS_SET_DISPLAY_ALL);
                }
                else
                {
                    menuitem->PrevItem = menuitem->CurDispItem ++;
                    SendMsg(MSG_SYS_SET_DISP_REFRESH_CUR_PREV_ITEM);
                }

                menuitem->CurItemInID ++;
            }

            //if(menuitem->SetMenuTxtInfo->ChildNum >menuitem->TotalDispItem)//an 4.22
            SendMsg(MSG_SYS_SET_DISPLAY_SCR);
            menuId = menuitem->SetMenuTxtInfo->SelfID;
#ifdef _RADIO_

            if (!gSysConfig.FMEnable)
            {
                //if(menuId > SID_REC_NORMAL_QUALITY)
                menuId += 9;
            }

#else   //no radio
            //if(menuId > SID_REC_NORMAL_QUALITY)
            menuId += 9;
#endif
#ifdef _MUSIC_

            if (SID_EQ_SELECT == menuId)
            {
                SetEqType = menuitem->CurItemInID;
                SendMsg(MSG_AUDIO_EQSET_UPDATA);
            }

#endif//_MUSIC_

            if (SID_COMMON_BRIGHTNESS == menuId)
            {
                SetBackLightLevel = menuitem->CurItemInID;
                //PWM_Stop(BL_PWM_CH);
                BL_SetLevel(SetBackLightLevel);
            }

            break;

        case KEY_VAL_ESC_SHORT_UP: //return up level
            menuId = menuitem->SetMenuTxtInfo->SelfID;
#ifdef _RADIO_

            if (!gSysConfig.FMEnable)
            {
                //if(menuId > SID_REC_NORMAL_QUALITY)
                menuId += 9;
            }

#else
            //if(menuId > SID_REC_NORMAL_QUALITY)
            menuId += 9;
#endif
#ifdef _MUSIC_

            if (SID_EQ_SELECT == menuId)
            {
                SetEqType = SysSetEqBackup;
                SendMsg(MSG_AUDIO_EQSET_UPDATA);
            }

#endif//_MUSIC_

            if (SID_COMMON_BRIGHTNESS == menuId)
            {
                SetBackLightLevel = BacklightBackup;
                //PWM_Stop(BL_PWM_CH);
                BL_SetLevel(SetBackLightLevel);
            }

            if (ReqMenuTxtInfoStructFlag == 0)
            {
                TaskArg.MainMenu.MenuID = MAINMENU_ID_SETMENU;
                TaskSwitch(TASK_ID_MAINMENU,&TaskArg);
                retVal = 1;// exit to main interface
            }
            else
            {
                FatherItemID = menuitem->FatherItemID;
                ReqMenuTxtInfoStructFlag--;
                menuitem = (SETMENU_INFO_STRUCT *)(&SetMenuItem[ReqMenuTxtInfoStructFlag]);
                menuitem->SetMenuTxtInfo = (MENU_TEXT_INFO_STRUCT *)(&SetMenuTxtInfo[ReqMenuTxtInfoStructFlag %
                                           SYS_SET_GLOBE_TXT_INFO_STRUCT_NUM]);
                GetMenuTextInfoWithIDNum(FatherItemID, menuitem->SetMenuTxtInfo);
                SendMsg(MSG_SYS_SET_DISPLAY);
                SendMsg(MSG_SYS_SET_DISPLAY_TITLE);
            }

            break;

        case KEY_VAL_MENU_SHORT_UP: //sure
        {
            retVal = MenuKeyPro(menuitem->SetMenuTxtInfo->ChildIDArray[menuitem->CurItemInID]);
            SendMsg(MSG_SYS_SET_DISPLAY_TITLE);
        }
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&SetMenuWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&SetMenuWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return (retVal);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 MenuKeyPro(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SERVICE_CODE_
UINT32 MenuKeyPro(UINT16 itemNum)
{
    UINT32  retval;
    UINT32  menuId;
    UINT32  i;
    SETMENU_INFO_STRUCT *MenuItem;
    SETMENU_INFO_STRUCT *MenuItem1;
    TASK_ARG TaskArg;
    retval = 0;
    MenuItem = (SETMENU_INFO_STRUCT *)(&SetMenuItem[ReqMenuTxtInfoStructFlag++]);//menu item message of current level
    MenuItem1 = (SETMENU_INFO_STRUCT *)(&SetMenuItem[ReqMenuTxtInfoStructFlag]);//next levle menu message
    MenuItem1->SetMenuTxtInfo = (MENU_TEXT_INFO_STRUCT *)(&SetMenuTxtInfo[ReqMenuTxtInfoStructFlag %
                                SYS_SET_GLOBE_TXT_INFO_STRUCT_NUM]);
    GetMenuTextInfoWithIDNum(MenuItem->SetMenuTxtInfo->ChildIDArray[MenuItem->CurItemInID],
                             MenuItem1->SetMenuTxtInfo);
    MenuItem1->FatherItemID =  MenuItem1->SetMenuTxtInfo->FatherID;
    MenuItem1->SelItemInID = 0Xff;
#ifdef _RADIO_
    if (!gSysConfig.FMEnable)
    {
//        if(itemNum > SID_REC_NORMAL_QUALITY)
        itemNum += 9;
    }
#else
    //if(itemNum > SID_REC_NORMAL_QUALITY)
    itemNum += 9;
#endif

    switch (itemNum)
    {
#ifdef _MUSIC_
        case SID_MUSIC_REPEAT_MODE :
            MenuItem1->SelItemInID = SetMusicPlayMode;
            break;

        case SID_MUSIC_SHUFFLE :
            MenuItem1->SelItemInID = SetMusicPlayOrder;
            break;

        case SID_EQ_SELECT :
            MenuItem1->SelItemInID = SetEqType;
            break;

#endif

#ifdef _RECORD_
        case SID_REC_QUALITY  :
            MenuItem1->SelItemInID = SetRecordQuality;
            break;
#endif
#ifdef _BLUETOOTH_
        case SID_SETTING_BT:
        {
            if(IsBTOpened)
                gbBTSwitchOn = BT_ON;
            else
                gbBTSwitchOn = BT_OFF;
            DEBUG("@@@ Select itemid = %d",gbBTSwitchOn);
            MenuItem1->SelItemInID = gbBTSwitchOn;
        }
            break;
#endif

#ifdef _RADIO_
        case SID_RADIO_STEREO_SWITCH:
            MenuItem1->SelItemInID = SetRadioStereo;
            break;

        case SID_FM_REGION:
            MenuItem1->SelItemInID = SetRadioArea;
            break;

        case SID_RADIO_SCAN_SENSITIVITY:
            MenuItem1->SelItemInID = SetScanSensitivity;
            break;
#endif

#ifdef _EBOOK_

        case SID_TEXT_AUTO_PLAY_TIME :
            MenuItem1->SelItemInID = SetTextAutoBrowseTime;
            break;
#endif

#ifdef _PICTURE_
        case SID_PIC_AUTO_PLAY_TIME:
            MenuItem1->SelItemInID = SetPicBroTimer;
            break;
#endif
        case SID_COMMON_SCREEN_OFF_TIMER:
            MenuItem1->SelItemInID = SetLightOffTime;
            break;

        case SID_COMMON_BRIGHTNESS:
            MenuItem1->SelItemInID = SetBackLightLevel;
            break;

        case SID_LANGUAGE  :                          //hj
            MenuItem1->SelItemInID = (UINT8)DefaultLanguage;
            break;

//        case SID_BLIGHT_TIME :
//            MenuItem1->SelItemInID = SetLightOffTime;
//            break;

//        case SID_SETTING_LUM:                                 //brightness
//            MenuItem1->SelItemInID = SetBackLightLevel;
//            break;

//        case SID_BLIGHT_MODE:                                //
//            MenuItem1->SelItemInID = SetBackLightMode;
//            break;

//        case SID_POWER_OFF :
//            MenuItem1->SelItemInID = SetPowerOffTime;
//            break;

        default:
            break;

    }
//------------------------------------------------------------------------------
    // real time audio effect hook
    menuId = MenuItem1->SetMenuTxtInfo->SelfID;
#ifdef _RADIO_
    if (!gSysConfig.FMEnable)
    {
//        if(menuId > SID_REC_NORMAL_QUALITY)
        menuId += 9;
    }
#else
//    if(menuId > SID_REC_NORMAL_QUALITY)
    menuId += 9;
#endif

#ifdef _MUSIC_
    if (SID_EQ_SELECT == menuId)
    {
        //after sure Eq,assign backup variables to EQ.restore EQ when FFW return,
        //only can change g_SYSSET_EqBackup when kick sure key.
        SysSetEqBackup = SetEqType;
    }

#endif

    if (SID_COMMON_BRIGHTNESS == menuId)
    {
        BacklightBackup = SetBackLightLevel;     //save the backlight setting after sure.
    }

//------------------------------------------------------------------------------
//no child item
    if (0 == MenuItem1->SetMenuTxtInfo->ChildNum)
    {
        //excute the menu setting
        retval = ExcuteSetMenuFunc(MenuItem->SetMenuTxtInfo->ChildIDArray[MenuItem->CurItemInID],
                                   MenuItem->CurItemInID );//MenuItem->SetMenuTxtInfo->CurItemInFatherID);

        if (GetMsg(MSG_SETDIALOG_FLAG))
        {
            //popup dialog,exit direct
            ReqMenuTxtInfoStructFlag --;
            return (retval);
        }
        else
        {
            menuId = MenuItem->SetMenuTxtInfo->SelfID;
#ifdef _RADIO_
            if (!gSysConfig.FMEnable)
            {
//                if(menuId > SID_REC_NORMAL_QUALITY)
                menuId += 9;
            }
#else
//            if(menuId > SID_REC_NORMAL_QUALITY)
            menuId += 9;
#endif

#ifdef _MUSIC_

            if (SID_EQ_SELECT == menuId)
            {
                SetEqType = SysSetEqBackup ;     //restore EQ
                //SendMsg(MSG_AUDIO_EQSET_UPDATA);
            }
#endif

            if (SID_COMMON_BRIGHTNESS == menuId)
            {
                SetBackLightLevel = BacklightBackup;
                //PWM_Stop(BL_PWM_CH);
                BL_SetLevel(SetBackLightLevel);
            }

            if (ReqMenuTxtInfoStructFlag == 0)//if it is frist level,then return to main interface.
            {
                retval = 1;
            }
            else//return to last level menu
            {
                ReqMenuTxtInfoStructFlag -= 2;
                SendMsg(MSG_SYS_SET_DISPLAY);
            }
        }
    }
    //go in child menu,when enter child menu.
    else
    {
        ClearMsg(MSG_SETDIALOG_FLAG);

        if (MenuItem1->SelItemInID == 0xff)//there is focus item in display menu,
        {
            MenuItem1->CurItemInID = 0;
            MenuItem1->CurDispItemFirst = 0;
            MenuItem1->CurDispItem = 0;

            if (MenuItem1->SetMenuTxtInfo->ChildNum > SYS_SET_MAX_ITEM_PER_SCREEN)
                MenuItem1->TotalDispItem = SYS_SET_MAX_ITEM_PER_SCREEN;
            else
                MenuItem1->TotalDispItem = MenuItem1->SetMenuTxtInfo->ChildNum ;
        }
        else
        {
            if (itemNum == SID_LANGUAGE)
            {
                MenuItem1->SetMenuTxtInfo->ChildNum = LanguageNum;

                for (i = 0; i < LanguageNum; i++)
                {
                    MenuItem1->SetMenuTxtInfo->ChildIDArray[i] = SetLanguageItem[i];
                }
            }

            MenuItem1->CurDispItem  = MenuItem1->SelItemInID % SYS_SET_MAX_ITEM_PER_SCREEN;
            MenuItem1->CurDispItemFirst = MenuItem1->SelItemInID - MenuItem1->CurDispItem;
            MenuItem1->CurItemInID = MenuItem1->SelItemInID;

            if (MenuItem1->CurDispItemFirst + SYS_SET_MAX_ITEM_PER_SCREEN < MenuItem1->SetMenuTxtInfo->ChildNum)
                MenuItem1->TotalDispItem = SYS_SET_MAX_ITEM_PER_SCREEN;
            else
                MenuItem1->TotalDispItem = MenuItem1->SetMenuTxtInfo->ChildNum - MenuItem1->CurDispItemFirst ;
        }

        SendMsg(MSG_SYS_SET_DISPLAY_SCR);
        SendMsg(MSG_SYS_SET_DISPLAY);
    }

    return (retval);
}

/*
--------------------------------------------------------------------------------
  Function name : void SetMenuDisplay(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo        2009-3-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SERVICE_CODE_
void SetMenuPaint(void)
{
    UINT16  i, color_tmp;
    UINT16  TempColor, TempBkColor, TempCharSize, TempTxtMode;
    UINT16  radioButtonOffset;
    SETMENU_INFO_STRUCT * menuitem;
    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);
    menuitem = (SETMENU_INFO_STRUCT *)(&SetMenuItem[ReqMenuTxtInfoStructFlag]);

    if ((CheckMsg(MSG_NEED_PAINT_ALL)) && (CheckMsg(MSG_SYS_SET_MUSIC_HOLD))) //the lcd flicker,if not has this
    {
        return;
    }

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_SYS_SET_DISPLAY)))
    {
        //to display backgroud picture,battery,hold on icon.
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + SetBatteryLevel, 105, 146);
        SendMsg(MSG_SYS_SET_DISPLAY_TITLE);//refresh title
        SendMsg(MSG_SYS_SET_DISPLAY_ALL);  //refresh all item
    }

    if (GetMsg(MSG_SYS_SET_DISPLAY_TITLE))
    {
        //setting display title
        DisplayMenuStrWithIDNum(SETMENU_TITLE_TXT_X, SETMENU_TITLE_TXT_Y,
                                SETMENU_TITLE_TXT_XSIZE, SETMENU_TITLE_TXT_YSIZE,
                                LCD_TEXTALIGN_CENTER, menuitem->SetMenuTxtInfo->SelfID);
    }

    if (GetMsg(MSG_SYS_SET_DISPLAY_ALL))
    {
        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, 3, 143);

        for (i = 0; i < menuitem->TotalDispItem; i++)
        {
            if (menuitem->SelItemInID == 0xff)
            {
                if (i == menuitem->CurDispItem)
                {
                    DispPictureWithIDNumAndXY(IMG_ID_ICON_SEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * i);
                }
                else
                {
                    DispPictureWithIDNumAndXY(IMG_ID_PONIT_NOSEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * i);
                }
            }
            else
            {
                if (menuitem->SelItemInID == menuitem->CurDispItemFirst + i)
                {
                    DispPictureWithIDNumAndXY(IMG_ID_ICON_SEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * i);
                }
                else
                {
                    DispPictureWithIDNumAndXY(IMG_ID_PONIT_NOSEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * i);
                }
            }

            //refresh character.
            if (i == menuitem->CurDispItem)
            {
                DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON, SETMENU_ITEM_TXT_X - 1, SETMENU_ITEM_TXT_Y + 17 * i);
            }

            DisplayMenuStrWithIDNum(SETMENU_ITEM_TXT_X, (SETMENU_ITEM_TXT_Y + (17 * i)),
                                    SETMENU_ITEM_TXT_XSIZE, SETMENU_ITEM_TXT_YSIZE,
                                    LCD_TEXTALIGN_LEFT, menuitem->SetMenuTxtInfo->ChildIDArray[menuitem->CurDispItemFirst + i]);
        }
    }

    if (GetMsg(MSG_SYS_SET_DISP_REFRESH_CUR_PREV_ITEM))
    {
        if (menuitem->SelItemInID == 0xff)//û��ѡ����Ŀ
        {
            DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, SETMENU_PIC_Y + 17 * menuitem->CurDispItem, 17);
            DispPictureWithIDNumAndXY(IMG_ID_ICON_SEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * menuitem->CurDispItem); //���ָʾͼ��
            DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, SETMENU_PIC_Y + 17 * menuitem->PrevItem, 17);
            DispPictureWithIDNumAndXY(IMG_ID_PONIT_NOSEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * menuitem->PrevItem);
        }
        else
        {
            DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, SETMENU_PIC_Y + 17 * menuitem->CurDispItem, 17);
            DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, SETMENU_PIC_Y + 17 * menuitem->PrevItem, 17);

            if (menuitem->SelItemInID == menuitem->CurDispItemFirst + menuitem->CurDispItem)
            {
                DispPictureWithIDNumAndXY(IMG_ID_ICON_SEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * menuitem->CurDispItem); //���ָʾͼ��
                DispPictureWithIDNumAndXY(IMG_ID_PONIT_NOSEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * menuitem->PrevItem);
            }
            else if (menuitem->SelItemInID == menuitem->CurDispItemFirst + menuitem->PrevItem)
            {
                DispPictureWithIDNumAndXY(IMG_ID_PONIT_NOSEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * menuitem->CurDispItem);
                DispPictureWithIDNumAndXY(IMG_ID_ICON_SEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * menuitem->PrevItem);
            }
            else
            {
                DispPictureWithIDNumAndXY(IMG_ID_PONIT_NOSEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * menuitem->CurDispItem);
                DispPictureWithIDNumAndXY(IMG_ID_PONIT_NOSEL, SETMENU_PIC_X, SETMENU_PIC_Y + 17 * menuitem->PrevItem);
            }
        }

        //display cursor item text
        DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON, SETMENU_ITEM_TXT_X - 1, SETMENU_ITEM_TXT_Y + 17 * menuitem->CurDispItem);
        DisplayMenuStrWithIDNum(SETMENU_ITEM_TXT_X, (SETMENU_ITEM_TXT_Y + (17 * menuitem->CurDispItem)),
                                SETMENU_ITEM_TXT_XSIZE, SETMENU_ITEM_TXT_YSIZE,
                                LCD_TEXTALIGN_LEFT, menuitem->SetMenuTxtInfo->ChildIDArray[menuitem->CurDispItemFirst + menuitem->CurDispItem]);
        DisplayMenuStrWithIDNum(SETMENU_ITEM_TXT_X, (SETMENU_ITEM_TXT_Y + (17 * menuitem->PrevItem)),
                                SETMENU_ITEM_TXT_XSIZE, SETMENU_ITEM_TXT_YSIZE,
                                LCD_TEXTALIGN_LEFT, menuitem->SetMenuTxtInfo->ChildIDArray[menuitem->CurDispItemFirst + menuitem->PrevItem]);
    }

    if (GetMsg(MSG_SYS_SET_DISPLAY_SCR))
    {
    }

    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + SetBatteryLevel, 105, 146);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
********************************************************************************
*
*                         End of SetMenu.c
*
********************************************************************************
*/
#endif

