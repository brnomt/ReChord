/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��o  SystemSet.c
*
* Description:  set menu execution function configuration
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-3-3          1.0
*    desc:      the functions below will be excuted when press sure key and it has not child item,
                the itemNum be force set to CurItemInFatherID,that is the index of current item,so it can
                be used to do porper contorl in inside of function.
********************************************************************************
*/
#define _SYSTEMSET_IN_

#include "SysInclude.h"

#ifdef _SYSSET_


#include "DialogBox.h"
#include "MessageBox.h"
#include "SetCommon.h"
#include "SystemSet.h"
#include "SetMenuInterface.h"
#include "SetMenu.h"
#include "Hold.h"

#include "fsinclude.h"
#ifdef _MUSIC_
#include "AudioControl.h"
#endif

#ifdef _RECORD_
#include "recordcontrol.h"
#endif

#ifdef _RADIO_
#include "FMControl.h"
#endif

extern SYSTICK_LIST SetPowerDownTimer;

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LanguegeSelect(UINT16 itemNum)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_//_ATTR_SYS_SET_SERVICE_CODE_
UINT32 LanguegeSelect(UINT32 itemNum)
{
    UINT32 retVal = 0;

    SetLanguage = SetMenuLanguageInfo[itemNum];
    gLangSel = gSysConfig.SysLanguage;
    DefaultLanguage = itemNum;
    return retVal;
}


/*
--------------------------------------------------------------------------------
  Function name : UINT16 BackLightOffTimeSet(UINT16 itemNum)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
UINT32 BackLightOffTimeSet(UINT32 itemNum)
{
    UINT32 retVal = 0;

    SetLightOffTime = itemNum;
    BacklightSystickCounterBack = SysTickCounter;
    return retVal;
}

/*
--------------------------------------------------------------------------------
 Function name : UINT16 BackLightLevelSet(UINT16 itemNum)
 Author        : anzhiguo
 Description   :

 Input         :
 Return        :

 History:     <author>         <time>         <version>
               anzhiguo     2009-3-3       Ver1.0
 desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
UINT32 BackLightLevelSet(UINT32 itemNum)
{
    UINT32 retVal = 0;
    BacklightBackup = itemNum;
    return retVal;
}

/*
--------------------------------------------------------------------------------
 Function name : UINT16 BackLightDispModeSet(UINT16 itemNum)
 Author        : anzhiguo
 Description   :

 Input         :
 Return        :

 History:     <author>         <time>         <version>
               anzhiguo     2009-3-3       Ver1.0
 desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
UINT32 BackLightDispModeSet(UINT32 itemNum)
{
    UINT32 retVal = 0;
    SetBackLightMode = itemNum;
    return retVal;
}

/*
--------------------------------------------------------------------------------
 Function name : UINT16 PowerOffTimeSet(UINT16 itemNum)
 Author        : anzhiguo
 Description   :

 Input         :
 Return        :

 History:     <author>         <time>         <version>
               anzhiguo     2009-3-3       Ver1.0
 desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
UINT32 PowerOffTimeSet(UINT32 itemNum)
{
    UINT32 retVal = 0;
    uint32 PowerOffBuff[6] = {0, 10 * 60 * 100, 15 * 60 * 100, 30 * 60 * 100, 60 * 60 * 100, 120 * 60 * 100};
    SetPowerDownTimer.Counter = 0;

    if (SetPowerOffTime == itemNum)
        return 0;

    SetPowerOffTime = itemNum;

    if (itemNum)
    {
        SetPowerDownTimer.Reload = PowerOffBuff[itemNum];

        if (SetPowerOffFlag == 0)
        {
            SetPowerOffFlag = 1;
            SystickTimerStart(&SetPowerDownTimer);
        }
    }
    else
    {
        if (SetPowerOffFlag == 1)
        {
            SystickTimerStop(&SetPowerDownTimer);
            SetPowerOffFlag = 0;
        }
    }

    return retVal;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 ConvertMemMB(UINT16 *pBuff, UINT32 *Mem)
  Author        : anzhiguo
  Description   : proudct information

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_SYSTEM_CODE_
UINT32 ConvertMemMB(UINT16 *pBuff, UINT32 *Mem)
{
    unsigned long Frememery;
    unsigned int i;
    UINT16 retVal = 0;
    Frememery = (*Mem) >> 10;
    *(pBuff + 4) = Frememery % 10 + '0';
    Frememery = Frememery / 10 ;
    *(pBuff + 3) = (Frememery % 10) + '0';
    Frememery = Frememery / 10 ;
    *(pBuff + 2) = (Frememery % 10) + '0';
    Frememery = Frememery / 10 ;
    *(pBuff + 1) = (Frememery % 10) + '0';
    *(pBuff + 0) = (Frememery / 10) + '0';

    for (i = 0; i < 4; i++)
    {
        if (*(pBuff + i) == '0')
        {
            *(pBuff + i) = ' ';
        }
        else
        {
            break;
        }
    }

    return retVal;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 FirmwareUpgradeSet(UINT16 itemNum)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_SYSTEM_CODE_
UINT32 FirmwareUpgradeSet(UINT32 itemNum)
{
    UINT32 retVal = 0;
    TASK_ARG TaskArg;

    TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
    TaskArg.Dialog.TitleID = SID_WARNING;
    TaskArg.Dialog.ContentID = SID_FIRMWARE_UPGRADE;
    WinCreat(&SetMenuWin, &DialogWin, &TaskArg);

    SendMsg(MSG_FIRMWAREUPGRADE);
    SendMsg(MSG_SETDIALOG_FLAG);

    return retVal;
}

/*
--------------------------------------------------------------------------------
 Function name : void SysSetDefaultValue(void)
 Author        : anzhiguo
 Description   : restore system default value.

 Input         :
 Return        :

 History:     <author>         <time>         <version>
               anzhiguo     2009-3-3       Ver1.0
 desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
void SysSetDefaultValue(void)
{
    uint32 i;
    uint32 Len;
    uint32 SystemDefaultParaAddr;
    uint8  FlashBuf[512];

    FIRMWARE_INFO_T       *pFirmwareModuleInfo;
    SYSTEM_DEFAULT_PARA_T *pSystemDefaultPara;
    ////////////////////////////////////////////////////////////////////////////
    MDReadData(SysDiskID, CodeLogicAddress, 512, FlashBuf);
    pFirmwareModuleInfo = (FIRMWARE_INFO_T *)FlashBuf;
    SystemDefaultParaAddr  = CodeLogicAddress + sizeof(pFirmwareModuleInfo -> LoadStartBase);
    SystemDefaultParaAddr += sizeof(pFirmwareModuleInfo -> ModuleInfo.ModuleNum);
    SystemDefaultParaAddr += pFirmwareModuleInfo -> ModuleInfo.ModuleNum * sizeof(CODE_INFO_T);
    MDReadData(SysDiskID, SystemDefaultParaAddr, sizeof(SYSTEM_DEFAULT_PARA_T), FlashBuf);
    pSystemDefaultPara = (SYSTEM_DEFAULT_PARA_T *)FlashBuf;
    ///////////////////////////////////////////////////////////////////////
    DefaultLanguage = (uint8)pSystemDefaultPara->gLanguage;//HJ
    gSysConfig.SysLanguage = SetMenuLanguageInfo[DefaultLanguage];
    gSysConfig.SysLanguage = (uint8)pSystemDefaultPara->gLanguage;
    gSysConfig.OutputVolume = (uint8)pSystemDefaultPara->Volume;
    gSysConfig.BLmode = (uint8)pSystemDefaultPara->BLMode;
    gSysConfig.BLevel = (uint8)pSystemDefaultPara->BLevel;
    BL_SetLevel(SetBackLightLevel);
    gSysConfig.BLtime = (uint8)pSystemDefaultPara->BLtime;
    gSysConfig.ShutTime = (uint8)pSystemDefaultPara->SettingPowerOffTime;
#ifdef _MUSIC_
    gSysConfig.MusicConfig.RepeatMode = (uint8)pSystemDefaultPara->MusicRepMode;
    gSysConfig.MusicConfig.PlayOrder = (uint8)pSystemDefaultPara->MusicPlayOrder;
    gSysConfig.MusicConfig.Eq.Mode = pSystemDefaultPara->MusicEqSel;
#ifdef _RK_EQ_

    for (i = 0; i < 5; i++)
    {
        gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i] = 3;
    }

#endif
    SendMsg(MSG_SERVICE_MUSIC_MODE_UPDATE);
    SendMsg(MSG_SERVICE_MUSIC_ORDER_UPDATE);
    SendMsg(MSG_AUDIO_EQSET_UPDATA);
#endif
#ifdef _RADIO_
    gSysConfig.RadioConfig.FmArea = (uint8)pSystemDefaultPara->FMArea;
    gSysConfig.RadioConfig.FmStereo = (uint8)pSystemDefaultPara->FMStereo;
    gSysConfig.RadioConfig.ScanSensitivity = (uint8)pSystemDefaultPara->FMScanSensitivity;
#endif
#ifdef _RECORD_
    gSysConfig.RecordConfig.RecordQuality = 1;
    gSysConfig.RecordConfig.RecordFmt = 1;  //wav
    gSysConfig.RecordConfig.RecordVol = 2;
#endif
#ifdef _EBOOK_
    gSysConfig.TextConfig.AutoPlayTime = (uint8)pSystemDefaultPara->TextAutoPlayTime;
#endif
#ifdef _PICTURE_
    gSysConfig.PicConfig.AutoPlayTime = (uint8)pSystemDefaultPara->PicAutoPlayTime;
#endif
}
void SysSetDefaultValueClone(void)
{
    uint32 i;
    uint32 Len;
    uint32 SystemDefaultParaAddr;
    uint8  FlashBuf[512];

    FIRMWARE_INFO_T       *pFirmwareModuleInfo;
    SYSTEM_DEFAULT_PARA_T *pSystemDefaultPara;
    //printf("set default value\n");
    ////////////////////////////////////////////////////////////////////////////
    MDReadData(SysDiskID, CodeLogicAddress, 512, FlashBuf);
    pFirmwareModuleInfo = (FIRMWARE_INFO_T *)FlashBuf;

    SystemDefaultParaAddr  = CodeLogicAddress + sizeof(pFirmwareModuleInfo -> LoadStartBase);
    SystemDefaultParaAddr += sizeof(pFirmwareModuleInfo -> ModuleInfo.ModuleNum);
    SystemDefaultParaAddr += pFirmwareModuleInfo -> ModuleInfo.ModuleNum * sizeof(CODE_INFO_T);
    MDReadData(SysDiskID, SystemDefaultParaAddr, sizeof(SYSTEM_DEFAULT_PARA_T), FlashBuf);
    pSystemDefaultPara = (SYSTEM_DEFAULT_PARA_T *)FlashBuf;
    ///////////////////////////////////////////////////////////////////////
    DefaultLanguage = DefaultLanguageID;
    gSysConfig.SysLanguage = SetMenuLanguageInfo[DefaultLanguage];
    gSysConfig.SysLanguage = (uint8)pSystemDefaultPara->gLanguage;
    gSysConfig.OutputVolume = (uint8)pSystemDefaultPara->Volume;
    gSysConfig.BLmode = (uint8)pSystemDefaultPara->BLMode;
    gSysConfig.BLevel = (uint8)pSystemDefaultPara->BLevel;
    BL_SetLevel(SetBackLightLevel);
    gSysConfig.BLtime = (uint8)pSystemDefaultPara->BLtime;
    gSysConfig.ShutTime = (uint8)pSystemDefaultPara->SettingPowerOffTime;
#ifdef _MUSIC_
    gSysConfig.MusicConfig.RepeatMode = (uint8)pSystemDefaultPara->MusicRepMode;
    gSysConfig.MusicConfig.PlayOrder = (uint8)pSystemDefaultPara->MusicPlayOrder;
    gSysConfig.MusicConfig.Eq.Mode = pSystemDefaultPara->MusicEqSel;
#ifdef _RK_EQ_

    for (i = 0; i < 5; i++)
    {
        gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i] = 3;
    }

#endif
    SendMsg(MSG_SERVICE_MUSIC_MODE_UPDATE);
    SendMsg(MSG_SERVICE_MUSIC_ORDER_UPDATE);
    SendMsg(MSG_AUDIO_EQSET_UPDATA);
#endif
#ifdef _RADIO_
    gSysConfig.RadioConfig.FmArea = (uint8)pSystemDefaultPara->FMArea;
    gSysConfig.RadioConfig.FmStereo = (uint8)pSystemDefaultPara->FMStereo;
    gSysConfig.RadioConfig.ScanSensitivity = (uint8)pSystemDefaultPara->FMScanSensitivity;
#endif
#ifdef _RECORD_
    gSysConfig.RecordConfig.RecordQuality = 1;
    gSysConfig.RecordConfig.RecordFmt = 1;
    gSysConfig.RecordConfig.RecordVol = 2;
#endif
#ifdef _EBOOK_
    gSysConfig.TextConfig.AutoPlayTime = (uint8)pSystemDefaultPara->TextAutoPlayTime;
#endif
#ifdef _PICTURE_
    gSysConfig.PicConfig.AutoPlayTime = (uint8)pSystemDefaultPara->PicAutoPlayTime;
#endif
}
/*
--------------------------------------------------------------------------------
  Function name : UINT16 SysDefaultValSet(UINT16 itemNum)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
UINT32 SysDefaultValSet(UINT32 itemNum)
{
    UINT32 retVal = 0;
    TASK_ARG TaskArg;

    TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
    TaskArg.Dialog.TitleID = SID_WARNING;
    TaskArg.Dialog.ContentID = SID_RESTORE_DEFAULT_VAL;
    WinCreat(&SetMenuWin, &DialogWin, &TaskArg);
    SendMsg(MSG_SYSDEFAULTVALSET);
    // ClearMsg(MSG_FIRMWAREUPGRADE);
    SendMsg(MSG_SETDIALOG_FLAG);
    return retVal;
}


/*
--------------------------------------------------------------------------------
  Function name : void ProductInfoWinInit(void)
  Author        : Anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               Anzhiguo       2009/03/01         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
void ProductInfoWinInit(void)
{
    KeyReset();
    SendMsg(MSG_PRODUCTINFO);
}

/*
--------------------------------------------------------------------------------
  Function name : void ProductInfoWinDeInit(void)
  Author        : Anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               Anzhiguo     2009/03/01         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
void ProductInfoWinDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SetMusicEqService(void)
  Author        : Anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                Anzhiguo     2009/03/01         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
UINT32 ProductInfoWinService(void)
{
    TASK_ARG TaskArg;

    return(RETURN_OK);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 ProductInfoWinKey(void)
  Author        : Anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               Anzhiguo     2009/03/01         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_SYSTEM_CODE_
UINT32 ProductInfoWinKey(void)
{
    UINT16 RetVal;
    UINT32 userEqSetKeyVal;
    TASK_ARG TaskArg;

    userEqSetKeyVal = GetKeyVal();

    switch (userEqSetKeyVal)
    {
        case KEY_VAL_ESC_SHORT_UP: //return up level
            WinDestroy(&ProductInfoWin);
            SendMsg(MSG_SYS_SET_DISPLAY);
            break;

        case KEY_VAL_PLAY_SHORT_UP:
            WinDestroy(&ProductInfoWin);
            SendMsg(MSG_SYS_SET_DISPLAY);
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&ProductInfoWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&ProductInfoWin, &HoldWin, &TaskArg);

            break;

        default:
            break;
    }

    return (RetVal);
}

_ATTR_SYS_SET_SYSTEM_CODE_
UINT16 CheckhuanhangAtnumber(UINT16 *pstr)
{
    UINT16 *point, number;
    point = pstr;
    number = 0;

    while (*point)
    {
        if ((*point) == '\n')
            break;

        point++;
        number++;
    }

    if (!(*point))
        return 0;

    return number;
}

_ATTR_SYS_SET_SYSTEM_CODE_
void SyssetScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
    PicturePartInfo     PicPartInfo;

    PicPartInfo.x       = pRect->x0;
    PicPartInfo.y       = pRect->y0;
    PicPartInfo.yoffset = 0;
    PicPartInfo.ysize   = CH_CHAR_XSIZE_12;
    PicPartInfo.pictureIDNump = ImageID;
    SetScrollStringInfo(pRect, PicPartInfo, pstr, Speed);
}

_ATTR_SYS_SET_SYSTEM_CODE_
void ScrollStringForInFo(UINT16 *pstr )
{
    UINT16    *pCurternStr;
    UINT16     TextMode;
    UINT16     Count;
    Count = SysTickCounter - ScrollString.SystickCounterBack;

    if (Count > ScrollString.ScrollSpeed)
    {
        Count = CheckhuanhangAtnumber(pstr + ScrollString.CharNumber);

        if (Count)
        {
            ScrollString.CharNumber += Count;
            ScrollString.CharNumber++;
            pCurternStr = pstr + ScrollString.CharNumber;
        }
        else
        {
            ScrollString.ScrollNumber = 1;
            ScrollString.DiplayRect.x0 = ScrollString.StartX ;
            pCurternStr = pstr;
            ScrollString.CharNumber = 0;
        }

#if (LCD_PIXEL == LCD_PIXEL_1)

        if (ScrollString.pictureID == NULL)
        {
            LCD_ClrRect(ScrollString.DiplayRect.x0, ScrollString.DiplayRect.y0, ScrollString.DiplayRect.x1, ScrollString.DiplayRect.y1);
        }
        else
#endif
        {
            DisplayPicture_part(ScrollString.pictureID, ScrollString.pictureInfo.x, ScrollString.pictureInfo.y, ScrollString.pictureInfo.yoffset, ScrollString.pictureInfo.ysize);
        }

        LCD_DispStringInRect(&(ScrollString.DiplayRect), &(ScrollString.DiplayRect), pCurternStr, LCD_TEXTALIGN_LEFT);
        ScrollString.ScrollNumber++;
        ScrollString.SystickCounterBack = SysTickCounter;
    }
}
/*********************************************************************/
_ATTR_SYS_SET_SYSTEM_BSS_ UINT16  fillDialogBuf[128];
_ATTR_SYS_SET_SYSTEM_CODE_
void FillMemeryBuf(int memType,uint16* fillBuf,uint32* index)
{
    UINT32 i = 0;
    UINT32 usedSpace;

    i = *index;

    if(*index > 128)
        return;

    if(memType == FLASH0) //emmc
    {
        fillBuf[i++] = '\n';

        fillBuf[i++] = 'C';
        fillBuf[i++] = ':';

        usedSpace = sysTotalMemeryFlash0 - sysFreeMemeryFlash0;
        ConvertMemMB(&fillBuf[i], &usedSpace);
        i += 5;

        fillBuf[i++] = '/';
        ConvertMemMB(&fillBuf[i], &sysTotalMemeryFlash0);
        i += 5;
        fillBuf[i++] = 'M';
        fillBuf[i++] = 'B';
        fillBuf[i]   = 0;
    }
    else if(memType == CARD) //sd card
    {
        fillBuf[i++] = '\n';

        fillBuf[i++] = 'C';
        fillBuf[i++] = 'A';
        fillBuf[i++] = 'R';
        fillBuf[i++] = 'D';
        fillBuf[i++] = ':';

        usedSpace = sysTotalMemeryCard - sysFreeMemeryCard;
        ConvertMemMB(&fillBuf[i], &usedSpace);
        i += 5;

        fillBuf[i++] = '/';
        ConvertMemMB(&fillBuf[i], &sysTotalMemeryCard);
        i += 5;
        fillBuf[i++] = 'M';
        fillBuf[i++] = 'B';
        fillBuf[i]   = 0;
    }
    else
    {
        //......
    }

    *index = i;
}

_ATTR_SYS_SET_SYSTEM_CODE_
void FillSN2Buf(uint32 snlength,char* sn_buf, uint16* fillBuf,uint32* index)
{
    UINT32 i= 0,inx = 0;
    i = *index;

    if(snlength == 0)
        return;

    if(i > 128)
        return;

    fillBuf[i++] = '\n';
    //dumpMemoryCharA(SN_buf,SN_length,0);
    printf("\n SN length = %d  SN:%s\n", snlength, sn_buf);
    fillBuf[i++] = 'S';
    fillBuf[i++] = 'N';
    fillBuf[i++] = ':';

    for (inx = 0; inx < snlength; inx++)
    {
        if (i > 128) //filldialogBuf have 80 elements
            break;

        fillBuf[i++] = sn_buf[inx];
    }

    if (i > 128)
        fillBuf[127] = 0;

    fillBuf[i] = 0;

    *index = i;
    DEBUG("Index SN = %d",i);
}

_ATTR_SYS_SET_SYSTEM_CODE_
void ProductInfoWinDisplay(void)
{
    UINT16  color_tmp,  cpuFreqTemp;
    UINT16  firmwareYear, firmwareDate, firmwareMonth;
    UINT16  masterVersion, slaveVersion, smallVersion;
    UINT32  temp;
    UINT16  *pFillDialogBuf = fillDialogBuf;
    LCD_RECT  r;
    uint8   SdCardStatus;
    UINT16  TempColor, TempBkColor, TempCharSize, TempTxtMode;

    //chad.ma add for SN infomation showing.Pls using  with FactoryTools and SN tool.
    UINT32 cnt_index;
    uint8  SN_buf[RKDEVICE_SN_LEN];
    uint16 SN_length;
    uint16 index;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_WHITE);
    LCD_SetBkColor(COLOR_BLACK);

    if (GetMsg(MSG_PRODUCTINFO) || (CheckMsg(MSG_NEED_PAINT_ALL)))
    {
        LCD_ClrSrc();
        SetGetFirmwareVersion(&firmwareYear, &firmwareDate, &masterVersion, &slaveVersion, &smallVersion);

        fillDialogBuf[0] = 'V';
        fillDialogBuf[1] = 'e';
        fillDialogBuf[2] = 'r';
        fillDialogBuf[3] = ':';
        fillDialogBuf[4] = ' ';
        fillDialogBuf[5] = (uint8)((masterVersion >> 4) & 0x0f) + '0';
        fillDialogBuf[6] = (uint8)((masterVersion >> 0) & 0x0f) + '0';
        fillDialogBuf[7] = '.';
        fillDialogBuf[8] = (uint8)((slaveVersion >> 4) & 0x0f) + '0';
        fillDialogBuf[9] = (uint8)((slaveVersion >> 0) & 0x0f) + '0';
        fillDialogBuf[10] = '.';
        fillDialogBuf[11] = ((smallVersion >> 12) & 0x0f) + '0';
        fillDialogBuf[12] = ((smallVersion >> 8)  & 0x0f) + '0';
        fillDialogBuf[13] = ((smallVersion >> 4)  & 0x0f) + '0';
        fillDialogBuf[14] = ((smallVersion >> 0)  & 0x0f) + '0';
        fillDialogBuf[15] = '\n';

        fillDialogBuf[16] = 'D';
        fillDialogBuf[17] = 'a';
        fillDialogBuf[18] = 't';
        fillDialogBuf[19] = 'e';
        fillDialogBuf[20] = ':';

        fillDialogBuf[21] = (uint8)((firmwareYear >> 12) & 0x0f) + '0';
        fillDialogBuf[22] = (uint8)((firmwareYear >>  8) & 0x0f) + '0';
        fillDialogBuf[23] = (uint8)((firmwareYear >>  4) & 0x0f) + '0';
        fillDialogBuf[24] = (uint8)((firmwareYear >>  0) & 0x0f) + '0';

        fillDialogBuf[25] = '-';
        fillDialogBuf[26] = (uint8)((firmwareDate >> 12) & 0x0f) + '0';
        fillDialogBuf[27] = (uint8)((firmwareDate >>  8) & 0x0f) + '0';
        fillDialogBuf[28] = '-';
        fillDialogBuf[29] = (uint8)((firmwareDate >> 4) & 0x0f) + '0';
        fillDialogBuf[30] = (uint8)((firmwareDate >> 0) & 0x0f) + '0';

        cnt_index = 31;
        FillMemeryBuf(FLASH0,fillDialogBuf,&cnt_index);

        //<------add by chad.ma 20151127 begin
#ifdef _EMMC_
        //SN_length = EmmcGetProductSn((uint8*)SN_buf);
#endif
        SN_length =  MDGetProductSn(SysDiskID, (uint8*)SN_buf);

        if (SN_length > 0)
        {
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (CheckCard())    //if sd cord is exit show sd card capacity
            {
                if(sysTotalMemeryCard!= 0)   //we have get sd card total memery
                {
                    FillMemeryBuf(CARD,fillDialogBuf,&cnt_index);
                }
                else
                {
                    DEBUG("Don't get the sd card capacity!!!");
                }
            }

            FillSN2Buf(SN_length,SN_buf,fillDialogBuf,&cnt_index);
#else
            FillSN2Buf(SN_length,SN_buf,fillDialogBuf,&cnt_index);
#endif
        }
        else
        {
            DEBUG("# Error! GetProductSN Fail...");

#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (CheckCard())    //if sd cord is exit show sd card capacity
            {
                if(sysTotalMemeryCard!= 0)   //we have get sd card total memery
                {
                    FillMemeryBuf(CARD,fillDialogBuf,&cnt_index);
                }
                else
                {
                    DEBUG("Don't get the sd card capacity!!!");
                }
            }
#endif
        }

        //------>add by chad.ma 20151127 end
        r.x0 = 0;
        r.x1 = 127;
        r.y0 = 0;
        r.y1 = 63;

        if (LCD_GetStringSize(fillDialogBuf) > ((r.x1 - r.x0) * (r.y1 - r.y0) / 12))
        {
            SyssetScrollInit(&r, NULL, fillDialogBuf, 120);
            SendMsg(MSG_SYS_SET_INFO_SCROLL);
        }

        LCD_DispStringInRect(&r, &r, (UINT16*)fillDialogBuf, LCD_TEXTALIGN_LEFT);
    }

    if (CheckMsg(MSG_SYS_SET_INFO_SCROLL))
    {
        ScrollStringForInFo(fillDialogBuf);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}


_ATTR_SYS_CODE_
UINT32 SysSetFormat(UINT32 itemNum)
{
    UINT32 retVal = 0;
    TASK_ARG TaskArg;

    if (CARD == itemNum)
    {
        if (TRUE == SDA_CheckCard(SDM_SD_ID))
        {
            TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
            TaskArg.Dialog.TitleID = SID_WARNING;
            TaskArg.Dialog.ContentID = SID_FORMAT_DISK;
            WinCreat(&SetMenuWin, &DialogWin, &TaskArg);
            SendMsg(MSG_FORMAT_SDCARD);
            SendMsg(MSG_SETDIALOG_FLAG);
        }
        else
        {
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_WARNING;   //SD��δ���룬�����ԡ�
            TaskArg.Message.HoldTime = 1;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            SendMsg(MSG_SYS_SET_DISPLAY);
            SetMenuPaint();
            WinCreat(&SetMenuWin, &MessageBoxWin, &TaskArg);
        }
    }
    else
    {
        TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
        TaskArg.Dialog.TitleID = SID_WARNING;
        TaskArg.Dialog.ContentID = SID_FORMAT_DISK;
        WinCreat(&SetMenuWin, &DialogWin, &TaskArg);
        SendMsg(MSG_FORMAT_FLASH);
        SendMsg(MSG_SETDIALOG_FLAG);
    }

    return retVal;
}

_ATTR_SYS_SET_SYSTEM_CODE_
void SysFormatFlash(void)
{
    ThreadDeleteAll(&pMainThread);

    FREQ_EnterModule(FREQ_MAX);

    gSysConfig.Memory =  FLASH0;
    FileSysSetup(gSysConfig.Memory);
    Format(FLASH0, FAT32);

    FREQ_ExitModule(FREQ_MAX);
}

_ATTR_SYS_SET_SYSTEM_CODE_
void SysFormatSDcard(void)
{
    ThreadDeleteAll(&pMainThread);

    FREQ_EnterModule(FREQ_MAX);

    //format SDCard
    FileSysSetup(CARD);
    Format(CARD, FAT32);

    //Change to Flash
    gSysConfig.Memory =  FLASH0;
    FileSysSetup(gSysConfig.Memory);
    FileInit();

    FREQ_ExitModule(FREQ_MAX);
}

/*
********************************************************************************
*
*                         End of SystemSet.c
*
********************************************************************************
*/
#endif

