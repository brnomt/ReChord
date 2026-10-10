/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  SysSetConfig.c
*
* Description:  execute the setting menu configuration function
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-3-3          1.0
*    desc:      the functions below will be excuted when press sure key and it has not child item,
                the itemNum be force set to CurItemInFatherID,that is the index of current item,so it can
                be used to do porper contorl in inside of function.
********************************************************************************
*/
#define _SETCOMMON_IN_

#include "SysInclude.h"

#ifdef _SYSSET_

#include "SetMenu.h"

#include "SetCommon.h"

#include "SetMenuInterface.h"

#include "SetBright.h"
#include "SetMusic.h"
#include "SetRecord.h"
#include "SetText.h"
#include "SystemSet.h"
#include "mainmenu.h"

#include "fsinclude.h"

#include "AudioControl.h"
#include "pcm.h"
#include "recordcontrol.h"

#ifdef _BLUETOOTH_
#include "SetBluetooth.h"
#include "BluetoothDevInfoWin.h"
#endif
/*
--------------------------------------------------------------------------------
  Function name : UINT16 ExcuteSetMenuFunc( UINT16 itemNum )
  Author        : anzhiguo
  Description   : execute the setting menu function it is in the last item,that is it be called when there is not
                  have child item.
  Input         : menu text id
                  the item index of setting item,base 0,part ITEM number.
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_COMMON_CODE_
UINT32 ExcuteSetMenuFunc(UINT32 menuId, UINT32 itemNum)
{
    UINT32 retVal = 0;
    TASK_ARG TaskArg;

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
    DEBUG("menuId = %d",menuId);
    switch (menuId)
    {
#ifdef _MUSIC_
            // case  SID_MUSIC_ONCE:
        case  SID_MUSIC_REPEAT_ONE:
        case  SID_MUSIC_FOLDER_REPEAT :
        case  SID_MUSIC_FOLDER_ONCE:
            // case  SID_MUSIC_ALL_ONCE:
            // case  SID_MUSIC_ALL_REPEAT :
            // case  SID_MUSIC_INTRO :
            DEBUG("itemNum = %d",itemNum);
            MusicPlayModeSet(itemNum);
            break;

        case SID_MUSIC_SHUFFLE_ON:
        case SID_MUSIC_SHUFFLE_OFF:
            DEBUG("itemNum = %d",itemNum);
            MusicPlayOrderSet(itemNum);
            break;

        case SID_EQ_BASS:

        case SID_EQ_NONE:
        case SID_EQ_HEAVY:
//        case SID_EQ_ROCK :
        case SID_EQ_POP :
//        case SID_EQ_CLASSIC :
        case SID_EQ_JAZZ :
        case SID_EQ_UNIQUE:
        case SID_EQ_CUSTOM:
            MusicPlayEqSet( itemNum);
            break;


            // case SID_USER_EQ_SET:
        case SID_EQ_CUSTOM_EDIT:
            //MusicPlayEqSet( itemNum);
            WinCreat(&SetMenuWin, &SetMusicUserEqWin, NULL);
            SendMsg(MSG_SETDIALOG_FLAG);
            break;
#endif

#ifdef _RADIO_
        case SID_SCAN_SENSITIVITY_HIGH:
        case SID_SCAN_SENSITIVITY_LOW:
            RadioSetMenuFmScanSensitivity(itemNum);
            break;

        case  SID_STEREO_SWITCH_OFF:
        case  SID_STEREO_SWITCH_ON:
            RadioSetMenuFmStereo(itemNum);
            break;

        case  SID_FM_REGION_CHINA:
        case  SID_FM_REGION_EUROPE:
        case  SID_FM_REGION_JAPAN:
        case  SID_FM_REGION_USA:
            RadioSetMenuFmArea(itemNum);
            break;
#endif

#ifdef _RECORD_
        case SID_REC_HIGH_QUALITY  :
        case SID_REC_NORMAL_QUALITY:
            RecordSetMenuQuality(itemNum);
            break;
#endif

#ifdef _BLUETOOTH_
        case SID_BT_ON:
            DEBUG("[BT] Select = %d BT_ON",itemNum);
            //BTSetMenuFunSwitch(itemNum);

            WinCreat(&SetMenuWin, &BTControlSubWin , NULL);
            //SendMsg(MSG_SETDIALOG_FLAG);

            break;

        case SID_BT_OFF:
            DEBUG("[BT] Select = %d BT_OFF",itemNum);
            //BTSetMenuFunSwitch(itemNum);
            BTFunSwitchOff();

            break;

        case SID_BT_INFO:
            DEBUG("[BT] Select = %d BT infomation",itemNum);
            WinCreat(&SetMenuWin, &BTLocalDevInfoWin , NULL);
            break;
#endif

#ifdef _EBOOK_
        case  SID_TEXT_AUTO_PLAY_1S :
        case  SID_TEXT_AUTO_PLAY_2S :
        case  SID_TEXT_AUTO_PLAY_3S :
        case  SID_TEXT_AUTO_PLAY_4S :
        case  SID_TEXT_AUTO_PLAY_5S :
            TextSetAutoBrowseTime(itemNum);
            break;
#endif

#ifdef _PICTURE_
        case  SID_PIC_AUTO_PLAY_1S :
        case  SID_PIC_AUTO_PLAY_2S :
        case  SID_PIC_AUTO_PLAY_3S :
        case  SID_PIC_AUTO_PLAY_4S :
        case  SID_PIC_AUTO_PLAY_5S :
            PicSetAutoBrowseTime (itemNum);
            break;
#endif

        case SID_SCREEN_OFF_15_SEC:
        case SID_SCREEN_OFF_30_SEC:
        case SID_SCREEN_OFF_1_MIN:
        case SID_SCREEN_OFF_3_MIN:
        case SID_SCREEN_OFF_5_MIN:
        case SID_SCREEN_OFF_30_MIN:
            BackLightOffTimeSet(itemNum);
            break;

        case SID_COMMON_BRIGHTNESS:
            WinCreat(&SetMenuWin, &SetBrightLevelWin, NULL);
            SendMsg(MSG_SETDIALOG_FLAG);
            break;

        case SID_LUM_1:
        case SID_LUM_2:
        case SID_LUM_3:
        case SID_LUM_4:
        case SID_LUM_5:
            BackLightLevelSet(itemNum);
            break;

        case  SID_CHINESE_S :
        case  SID_CHINESE_T :
        case  SID_ENGLISH   :
        case  SID_KOREAN    :
        case  SID_JAPANESE  :
        case  SID_FRENCH    :
        case  SID_GERMAN    :
        case  SID_PORTUGUESE:
        case  SID_RUSSIAN   :
        case  SID_SWEDISH   :
        case  SID_THAI      :
        case  SID_POLISH    :
        case  SID_DENISH    :
        case  SID_HOLLANDS  :
        case  SID_GREEK     :
        case  SID_CZECHIC   :
        case  SID_TURKISH   :
        case  SID_HEBREW    :
        case  SID_ARABIC    :
            LanguegeSelect( itemNum);
            break;

        case SID_FORMAT_DISK:
            SysSetFormat(FLASH0);
            break;

        case SID_FORMAT_SDCARD:     //chad.ma add
            SysSetFormat(CARD);
            break;

        case SID_SYS_ABOUT :
            WinCreat(&SetMenuWin, &ProductInfoWin, NULL);
            SendMsg(MSG_SETDIALOG_FLAG);
            break;

//        case SID_SYS_UPGRADE :
//            FirmwareUpgradeSet( itemNum);
//            break;

        case SID_SYS_DEFAULT_SET:
            SysDefaultValSet(itemNum);
            ResetFMPesetData();//HJ
            break;

//        case SID_EXIT_SYS_SETTING :
//            TaskArg.MainMenu.MenuID = MAINMENU_ID_SETMENU;
//            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
//            retVal = 1;
//            break;

        default:
            break;
    }
//    ModuleOverlay(MODULE_ID_SYSRESERVED_OP, MODULE_OVERLAY_ALL);
    SaveSysInformation(1); // Save USER settings once one setting changed.
    return retVal;
}

/*
********************************************************************************
*
*                         End of SetCommon.c
*
********************************************************************************
*/
#endif

