/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��o  MainMenu.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_MAINMENU_

#include "SysInclude.h"
#include "fsinclude.h"
#include "MainMenu.h"
#include "MainMenuInterface.h"
#include "BrowserUI.h"
#include "Hold.h"
#include "AudioControl.h"
#include "medialibwin.h"

#ifdef _RECORD_
#include  "File.h"
#include  "FDT.h"

#include  "pcm.h"
#include  "pmu.h"

#include  "audio_main.h"

#include  "recordcontrol.h"
#endif

#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#include "SetBluetooth.h"
#endif

extern uint8  BtWinStatus;

extern THREAD MusicThread;
extern THREAD RecordThread;
extern THREAD FMThread;
_ATTR_SYS_DATA_  UINT16      MM_RecordQuality;   //quality of recording.
_ATTR_SYS_DATA_  UINT16      MM_RecordFmt;   //quality of recording.


_ATTR_SYS_DATA_ UINT8 MainIdFlag = 0;
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
_ATTR_MAIN_MENU_INIT_CODE_
void MainMenuIntInit(void)
{
    //MainMenu  interrupt initialization
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
_ATTR_MAIN_MENU_DEINIT_CODE_
void MainMenuIntDeInit(void)
{
    //MainMenu
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuMsgInit(void)
  Author        : ZHengYongzhi
  Description   : main menu message initial function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_INIT_CODE_
void MainMenuMsgInit(void)
{
    SendMsg(MSG_MENU_DISPLAY_ALL);

}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuMsgDeInit(void)
  Author        : ZHengYongzhi
  Description   : main menu message auti-initialization

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_DEINIT_CODE_
void MainMenuMsgDeInit(void)
{
    // ClearMsg(MSG_MENU_DISPLAY_BACK);
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuInit(void)
  Author        : ZHengYongzhi
  Description   : main menu initialization

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_CODE_
void MainMenuInit(void *pArg)
{
    DEBUG("MainMenu Enter");
    //For set backlight
    //BL_SetLevel(gSysConfig.BLevel);
    //Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_PWM0_OUT);
    //system content switch initial code
    gSysConfig.FindFileType = FileTypeNull;

    MenuId = ((MAINMENU_WIN_ARG*)pArg)->MenuID;

#ifdef _BROWSER_
    if (MainIdFlag == MAINMENU_ID_BROWSER)
    {
        MenuId     = MAINMENU_ID_BROWSER;
        MainIdFlag = 0;
    }
#endif

    //service initial
    MainMenuMsgInit();      //message initial
    MainMenuIntInit();      //interrupt initial
    KeyReset();             //key initial
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuDeInit(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_CODE_
void MainMenuDeInit(void)
{
    MainMenuIntDeInit();
    DEBUG("MainMenu Exit");
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
_ATTR_MAIN_MENU_SERVICE_CODE_
UINT32 MainMenuService(void)
{
    int32 reslt;

    //FREQ_EnterModule(FREQ_MAX);

    if (GetMsg(MSG_SYS_FW_UPGRADE))
    {
        //zyz:Backup SYSCONFIG
        BackupSysInformation();
        DelayMs(200);

        ModuleOverlay(MODULE_ID_FW_UPGRADE, MODULE_OVERLAY_ALL);

        reslt = FwUpdate();
        #ifdef _FILE_DEBUG_
        SysDebugHookDeInit();
        #endif

        //FREQ_ExitModule(FREQ_MAX);

        if(reslt == 0)
        {
            DEBUG(" ### FwUpdate SUCCESS,REBOOT... ####");
            SysReboot(0x0000, 0);
        }
        else
        {
            DEBUG(" ### FwUpdate Fail,Not find FW or Other resion####");
        }
    }
    if (GetMsg(MSG_SYS_FW_RECOVERY))
    {
        FWRecoverDisplay();
        ModuleOverlay(MODULE_ID_FW_UPGRADE, MODULE_OVERLAY_ALL);
        FwUpgradeRecovery();

        #ifdef _FILE_DEBUG_
        SysDebugHookDeInit();
        #endif

        DEBUG("MSG_SYS_FW_RECOVERY Reboot...");
        SysReboot(0x0000, 0);
    }

    return(RETURN_OK);
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
_ATTR_MAIN_MENU_SERVICE_CODE_
UINT32 MainMenuKey(void)
{
    uint32 RetVal;
    UINT32 MainMenuKeyVal;
    WIN    *pWin;
    TASK_ARG TaskArg;

    RetVal = RETURN_OK;

    MainMenuKeyVal =  GetKeyVal();
    //7 key function
    switch (MainMenuKeyVal)
    {
        case KEY_VAL_MENU_SHORT_UP:
            RetVal = MainMenuModeKey();
            break;

        case KEY_VAL_FFD_SHORT_UP:
        case KEY_VAL_FFD_PRESS:
        case KEY_VAL_UP_SHORT_UP:
        case KEY_VAL_UP_PRESS:
            if (++MenuId > (MAINMENU_ID_MAXNUM - 1))
            {
                MenuId = 0;
            }
            SendMsg(MSG_MENU_DISPLAY_ITEM);
            break;

        case KEY_VAL_FFW_SHORT_UP:
        case KEY_VAL_FFW_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
        case KEY_VAL_DOWN_PRESS:
            if (MenuId == 0)
            {
                MenuId = (MAINMENU_ID_MAXNUM - 1);
            }
            else
            {
                MenuId -- ;
            }
            SendMsg(MSG_MENU_DISPLAY_ITEM);
            break;

        case KEY_VAL_ESC_SHORT_UP:
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&MainMenuWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&MainMenuWin, &HoldWin, &TaskArg);
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
_ATTR_MAIN_MENU_SERVICE_CODE_
void MainMenuDisplay(void)
{
    uint32 i;
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_MENU_DISPLAY_ALL)))
    {
        SendMsg(MSG_MENU_DISPLAY_ITEM);
        SendMsg(MSG_BATTERY_UPDATE);
    }

    if (TRUE == GetMsg(MSG_MENU_DISPLAY_ITEM))
    {
        for (i=0; i<(sizeof(MainConfig)/sizeof(Main_Config)); i++)
        {
            if (MainConfig[i].DispFlag && (MainConfig[i].TaskId == MenuId))
            {
                DispPictureWithIDNum(MainConfig[i].ImageId);
                DisplayMenuStrWithIDNum(0,128,127,144,LCD_TEXTALIGN_CENTER,MainConfig[i].TextId);
                break;
            }
        }
    }
    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        //DispPictureWithIDNum(IMG_ID_MAINMENU_BATTERYLEVEL0 + MainMenuBatteryLevel);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuModeKey(void)
  Author        : ZHengYongzhi
  Description   : main menu modules switch

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MAIN_MENU_SERVICE_CODE_
INT16 MainMenuModeKey(void)
{
    INT16  RetVal = 0;
    TASK_ARG TaskArg;

    switch (MenuId)
    {
#ifdef _MUSIC_
        case MAINMENU_ID_MUSIC:
#ifdef _MEDIA_MODULE_
            gSysConfig.FindFileType = FileTypeAudio;
            if (gSysConfig.MedialibPara.MediaUpdataFlag == 1)
            {//delete back thread when medialib not updated
#ifdef _MUSIC_
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    ThreadDelete(&pMainThread, &MusicThread);
                }
                gSysConfig.MusicConfig.HoldOnPlaySaveFlag =0;
#endif

#ifdef _RADIO_
                if (TRUE == ThreadCheck(pMainThread, &FMThread))
                {
                    ThreadDelete(&pMainThread, &FMThread);
                }
#endif

                TaskArg.Mdb.TaskID = TASK_ID_MEDIALIB;
                TaskSwitch(TASK_ID_MDB, &TaskArg);
                RetVal = 1;
            }
            else
            {
                TaskArg.Medialib.CurId= 0;
                TaskSwitch(TASK_ID_MEDIALIB, &TaskArg);
                RetVal = 1;
            }
#else   //3 spi nor flash,directly switch to browser
            TaskArg.Browser.FileType = FileTypeAudio;
            TaskArg.Browser.FileNum   = 0;
            TaskArg.Browser.FromWhere = 0;
            BroswerFlag = TRUE;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            RetVal = 1;
#endif
            break;
#endif


#ifdef _RECORD_
        case MAINMENU_ID_RECORD:
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }
#ifdef _RADIO_
            if (TRUE == ThreadCheck(pMainThread, &FMThread))
            {
                ThreadDelete(&pMainThread, &FMThread);
            }
#endif

#ifdef _BLUETOOTH_
            BluetoothThreadDelete(gbBTConnected);
#endif

            MM_RecordQuality = gSysConfig.RecordConfig.RecordQuality;
            MM_RecordFmt = gSysConfig.RecordConfig.RecordFmt;
            if (RECORD_QUALITY_HIGH == MM_RecordQuality)  //quality record.
            {
                TaskArg.Record.RecordExitTaskID = TASK_ID_MAINMENU;
                TaskArg.Record.RecordType = RECORD_TYPE_MIC_STERO;
                TaskArg.Record.RecordCodecConfig = 0;
                TaskArg.Record.RecordSampleRate = ENC_WAV_H_FS;
                TaskArg.Record.RecordChannel = RECORD_CHANNEL_STERO;
                TaskArg.Record.RecordDataWidth = RECORD_DATAWIDTH_24BIT;
                TaskArg.Record.RecordEncodeType = RECORD_ENCODE_TYPE_PCM;
            }
            else
            {
                TaskArg.Record.RecordExitTaskID = TASK_ID_MAINMENU;
                TaskArg.Record.RecordType = RECORD_TYPE_MIC_STERO;
                TaskArg.Record.RecordCodecConfig = 0;
                TaskArg.Record.RecordSampleRate = ENC_WAV_N_FS;
                TaskArg.Record.RecordChannel = RECORD_CHANNEL_STERO;
                TaskArg.Record.RecordDataWidth = RECORD_DATAWIDTH_16BIT;
                TaskArg.Record.RecordEncodeType = RECORD_ENCODE_TYPE_WAV;
            }

            TaskSwitch(TASK_ID_RECORD, &TaskArg);
            RetVal = 1;
            break;
#endif

#ifdef _EBOOK_
    case MAINMENU_ID_EBOOK:
        TaskArg.Browser.FileType = FileTypeText;
        TaskArg.Browser.FileNum  = 0;
        TaskArg.Browser.FromWhere = 0;
        BroswerFlag = TRUE;
        TaskSwitch(TASK_ID_BROWSER, &TaskArg);
        RetVal = 1;
        break;
#endif

#ifdef _VIDEO_
        case MAINMENU_ID_VIDEO: //video module
            TaskArg.Browser.FileType = FileTypeVideo;
            TaskArg.Browser.FileNum  = 0;
            TaskArg.Browser.FromWhere = 0;
            BroswerFlag = TRUE;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            RetVal = 1;
            break;
#endif

#ifdef _PICTURE_
        case MAINMENU_ID_PICTURE: //picture module
            TaskArg.Browser.FileType = FileTypePicture;
            TaskArg.Browser.FileNum  = 0;
            TaskArg.Browser.FromWhere = 0;
            BroswerFlag = TRUE;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            RetVal = 1;
            break;
#endif

#ifdef _RADIO_
        case MAINMENU_ID_RADIO:
            if (gSysConfig.FMEnable)
            {
                TaskSwitch(TASK_ID_RADIO, NULL);
                RetVal = 1;
            }
            break;
#endif

#ifdef _BROWSER_
        case MAINMENU_ID_BROWSER:
            MainIdFlag = MAINMENU_ID_BROWSER;
            TaskArg.Browser.FileType = FileTypeALL;
            TaskArg.Browser.FileNum  = 0;
            TaskArg.Browser.FromWhere = 0;
            BroswerFlag = TRUE;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            RetVal = 1;
            break;
#endif

#ifdef  _SYSSET_
        case MAINMENU_ID_SETMENU:
            TaskSwitch(TASK_ID_SYSSET, NULL);
            RetVal = 1;
            break;
#endif

        default:
            break;
    }

    return (RetVal);
}
/*
********************************************************************************
*
*                         End of MainMenu.c
*
********************************************************************************
*/
