/*
********************************************************************************
*                   Copyright (c) 2012,SunChuanHu
*                         All rights reserved.
*
* File Name?��o   MDBBuildWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               SunChuanHu      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#include "SysConfig.h"

#define _IN_MDBBUILDWIN_

#include "SysInclude.h"
#include "fsinclude.h"
#include "MDBBuildWin.h"
#include "mainmenu.h"
#include "MediaBroWin.h"
#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#include "SetBluetooth.h"
#endif
#include "medialibwin.h"

_ATTR_MDBBUILDWIN_BSS_ UINT32 LastTaskId;
_ATTR_MDBBUILDWIN_BSS_ uint32 LastDisplayUpdateCounter;
_ATTR_MDBBUILDWIN_BSS_ UINT16 MedialibUpdateImageFrameIndex;

extern uint8  BtWinStatus;

#ifdef _MEDIA_MODULE_
#ifdef MEDIA_UPDATE

/*
--------------------------------------------------------------------------------
  Function name : void MedialibUpdataDisplay(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
void MedialibUpdataDisplay(UINT16 PicId)
{
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode;
    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    DispPictureWithIDNumAndXYoffset(IMG_ID_MEIDASORTBAK0 + PicId, 0, 0);

    DisplayMenuStrWithIDNum(MEDIA_UPDATA_TXT_X, MEDIA_UPDATA_TXT_Y,
                            MEDIA_UPDATA_TXT_XSIZE, MEDIA_UPDATA_TXT_YSIZE,
                            LCD_TEXTALIGN_CENTER, SID_UPDATEING_MEDIALIB);

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);

#ifdef _FRAME_BUFFER_
    Lcd_BuferTranfer();
#endif
}


_ATTR_SYS_CODE_ //PAGE hj
void MedialibUpdateDisplayHook(void)
{
    MedialibUpdataDisplay(0);
}

/*
--------------------------------------------------------------------------------
  Function name : void SysCpuInit(void)
  Author        : ZHengYongzhi
  Description   : PLL setting??��Disable int??��PWM Disable,etc

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_MDBBUILDWIN_CODE_
void UpdateMediaLibray(void)
{
    LastDisplayUpdateCounter = 0;
    MedialibUpdateImageFrameIndex = 0;
    DEBUG("UpdateMediaLibray enter");
    MedialibUpdataDisplay(0);
    FREQ_EnterModule(FREQ_MEDIAUPDATA);

    #ifdef _SDCARD_
    if(MemorySelect!=FLASH0)
    {
        MemorySelect = FLASH0;

        FileSysSetup(FLASH0);
    }
    #endif

    MedialibUpdataDisplay(0);
    ModuleOverlay(MODULE_ID_FILEINFO_SAVE, MODULE_OVERLAY_ALL);
    SearchAndSaveMusicInfo();

    #ifdef _RECORD_
    SearchAndSaveRecordFmInfo();
    #endif

    MedialibUpdataDisplay(1);
    ModuleOverlay(MODULE_ID_FILEINFO_SORT, MODULE_OVERLAY_ALL);
    SortUpdateFun(&(gSysConfig.MedialibPara), MediaInfoAddr);

    MedialibUpdataDisplay(2);
    ModuleOverlay(MODULE_ID_FAVO_RESET, MODULE_OVERLAY_ALL);
    FavoReset();

    MedialibUpdataDisplay(3);
    SaveSysInformation(0);

    FREQ_ExitModule(FREQ_MEDIAUPDATA);
    MedialibUpdataDisplay(3);

    DEBUG("UpdateMediaLibray Success Over");
}
#endif
#endif

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
/*
_ATTR_MDBBUILDWIN_CODE_
void FWUpgradeDisplay(void)
{
    UINT16 TextMode ;

    TextMode = LCD_SetTextMode(LCD_DRAWMODE_NORMAL);
    LCD_ClrSrc();

    DisplayMenuStrWithIDNum(MEDIA_UPDATA_TXT_X, MEDIA_UPDATA_TXT_Y,
                            MEDIA_UPDATA_TXT_XSIZE, MEDIA_UPDATA_TXT_YSIZE,
                            LCD_TEXTALIGN_CENTER, SID_UPDATEING_FIRMWARE);
    Lcd_BuferTranfer(0,0,LCD_MAX_XSIZE,LCD_MAX_YSIZE, NULL);
    BL_On();
    LCD_SetTextMode(TextMode);
}
*/
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
_ATTR_MDBBUILDWIN_CODE_
void FWRecoverDisplay(void)
{
    #if 0
    UINT16 TextMode ;

    TextMode = LCD_SetTextMode(LCD_DRAWMODE_NORMAL);
    LCD_ClrSrc();

    DisplayMenuStrWithIDNum(MEDIA_UPDATA_TXT_X, MEDIA_UPDATA_TXT_Y,
                            MEDIA_UPDATA_TXT_XSIZE, MEDIA_UPDATA_TXT_YSIZE,
                            LCD_TEXTALIGN_CENTER, SID_RECOVER_FIRMWARE);
    Lcd_BuferTranfer(0,0,LCD_MAX_XSIZE,LCD_MAX_YSIZE, NULL);
    BL_On();
    LCD_SetTextMode(TextMode);
    #endif
}

/*
--------------------------------------------------------------------------------
  Function name : void MdbBuildWinInit(void)
  Author        : SunChuanHu
  Description   :MDB Build Win Initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
void MdbBuildWinInit(void * pArg)
{
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    LastTaskId = ((MDB_WIN_ARG *)pArg)->TaskID;

    if( gSysConfig.MedialibPara.MediaUpdataFlag )
    {
        DEBUG(" ***Media Need Update*** ");
        DispPictureWithIDNum(IMG_ID_MDB_BACKGROUND);
      // display title
        DisplayMenuStrWithIDNum(MDB_TITLE_TXT_X, MDB_TITLE_TXT_Y, MDB_TITLE_TXT_XSIZE,
                            MDB_TITLE_TXT_YSIZE, LCD_TEXTALIGN_CENTER, SID_MEDIALIB );
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01+gBattery.Batt_Level,105,146);
    }

#ifdef _FRAME_BUFFER_
    Lcd_BuferTranfer();
#endif

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}


/*
--------------------------------------------------------------------------------
  Function name : void MainMenuIntDeInit(void)
  Author        : ZHengYongzhi
  Description   : ???2?|�����?D???��??3?��o???��

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
void MdbBuildWinDeInit(void)
{
    //MainMenu?D???��??3?��o???��
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MainMenuService(void)
  Author        : ZHengYongzhi
  Description   : ???2?|�����?��t??3��?D����,��??��?��2??|���衧a?????��2????��??��?????��2?��t??|����������???

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
UINT32 MdbBuildWinService(void)
{
    TASK_ARG TaskArg;
    uint32 LDOBack;
    int32  reslt;

    FREQ_EnterModule(FREQ_MAX);

    /*
    We don't put FM update here considerd spi flash FW updating not enter MDB task.
    So we put it in mainmenu task,you can put it anywhere as you wanted.But must
    note the code segment DO NOT overlay.They must be the co-existence.
    */
    /*
    if (GetMsg(MSG_SYS_FW_UPGRADE))
    {
        //FWUpgradeDisplay();

        //zyz:Backup SYSCONFIG
        BackupSysInformation();
        DelayMs(200);

        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
//        ModuleOverlay(MODULE_ID_FILE_ENCODE, MODULE_OVERLAY_ALL);
        ModuleOverlay(MODULE_ID_FW_UPGRADE, MODULE_OVERLAY_ALL);
        reslt = FwUpdate();

        #ifdef _FILE_DEBUG_
        SysDebugHookDeInit();
        #endif

        FREQ_ExitModule(FREQ_MAX);

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
        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
//        ModuleOverlay(MODULE_ID_FILE_ENCODE, MODULE_OVERLAY_ALL);
        ModuleOverlay(MODULE_ID_FW_UPGRADE, MODULE_OVERLAY_ALL);
        FwUpgradeRecovery();

        #ifdef _FILE_DEBUG_
        SysDebugHookDeInit();
        #endif

        FREQ_ExitModule(FREQ_MAX);

        DEBUG("MSG_SYS_FW_RECOVERY Reboot...");
        SysReboot(0x0000, 0);
    }
    */

    if (gSysConfig.MedialibPara.MediaUpdataFlag)
    {
        gSysConfig.MedialibPara.MediaUpdataFlag   = 0;
#ifdef _MUSIC_
        gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;  //clear break point
#endif

        #ifdef _RADIO_
        gSysConfig.RadioConfig.HoldOnPlaySaveFlag = 0;
        #endif

        /*before update meida libray,we must disconnect BT if BT is connect,
        becouse the code is overlay.
        */
        #ifdef _BLUETOOTH_
        BluetoothThreadDelete(gbBTConnected);
        #endif

        #ifdef MEDIA_UPDATE
        UpdateMediaLibray();
        #endif

        SendMsg(MSG_FLASH_MEM0_UPDATE);
    }

    if(CheckMsg(MSG_FLASH_MEM0_UPDATE)
       ||CheckMsg(MSG_FLASH_MEM1_UPDATE)
       ||CheckMsg(MSG_SDCARD_MEM_UPDATE))
    {
       ModuleOverlay(MODULE_ID_FS_MEM_GET, MODULE_OVERLAY_ALL);
    }

    if(GetMsg(MSG_FLASH_MEM0_UPDATE))
    {
        GetFreeMem(FLASH0,&sysTotalMemeryFlash0, &sysFreeMemeryFlash0);
    }

    if(GetMsg(MSG_FLASH_MEM1_UPDATE))
    {
        GetFreeMem(FLASH1,&sysTotalMemeryFlash1, &sysFreeMemeryFlash1);
    }

    if(GetMsg(MSG_SDCARD_MEM_UPDATE))
    {
        if (CheckCard())    //sd card is or not insert?
        {
            GetFreeMem(CARD,&sysTotalMemeryCard, &sysFreeMemeryCard);
            DEBUG("sysTotalMemeryCard = %d ,sysFreeMemeryCard = %d",sysTotalMemeryCard,sysFreeMemeryCard);
        }
        else
        {
            DEBUG("No SD Card inserted");
        }
    }

    FREQ_ExitModule(FREQ_MAX);

    if(LastTaskId == TASK_ID_MAINMENU)
    {
        TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
    }
#ifdef _MUSIC_
    else if(LastTaskId == TASK_ID_MEDIALIB)
    {
        TaskArg.Medialib.CurId = 0;
        if (gSysConfig.MusicConfig.HoldOnPlaySaveFlag == 1)
        {
            TaskArg.Medialib.CurId = MEDIA_MUSIC_BREAKPOINT;
        }
    }
#endif

    #ifdef _RADIO_
    else if (LastTaskId == TASK_ID_RADIO)
    {
        TaskArg.Radio.Freq = gSysConfig.RadioConfig.FmFreq;
        TaskArg.Radio.AutoPreset = 0;
        SendMsg(MSG_RADIOSUBWIN_HOLDONPLAYSAVE);
    }
    #endif

    TaskSwitch(LastTaskId, &TaskArg);

    return(RETURN_FAIL);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MainMenuKey(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MDBBUILDWIN_CODE_
UINT32 MdbBuildWinKey(void)
{
    return 0;
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
_ATTR_MDBBUILDWIN_CODE_
void MdbBuildWinDisplay(void)
{
    DEBUG();
}

/*
--------------------------------------------------------------------------------
  Function name : void MedialibUpdateDisplayHook(void)
  Author        :
  Description   : Medialib update display hook called by external

  Input         :
  Return        :

  History:     <author>         <time>         <version>

  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_MDBBUILDWIN_CODE_
//void MedialibUpdateDisplayHook(void)
//{
    //MedialibUpdataDisplay();
//}

