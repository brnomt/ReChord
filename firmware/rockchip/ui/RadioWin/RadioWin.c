/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                      All rights reserved.
*
* File Name:   RadioWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*                 WangBo      2009-4-10       1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_RADIOWIN_

#include "SysInclude.h"

#define FM_GLOBALS

#ifdef _RADIO_

#include "RadioWinInterface.h"  //extern interface used radio module
#include "RadioWin.h"
#include "FMControl.h"

#include "mainmenu.h"
#include "Hold.h"

#ifdef _RECORD_
#include  "fsinclude.h"
#include  "File.h"
#include  "FDT.h"
#include  "pcm.h"
#include  "pmu.h"
#include  "audio_main.h"
#include  "recordcontrol.h"

#ifdef _MEDIA_MODULE_
#include "MediaBroWin.h"
#include "medialibwin.h"
#else
#include "BrowserUI.h"
#endif

#endif

#include "MessageBox.h"
#include "AudioControl.h"


extern FM_GLOBAL* gpRadioplayerRegKey;

extern UINT16  FmFreqMaxVal;   // max freq(inlcue japan tune)
extern UINT16  FmFreqMinVal;
extern  THREAD FMThread;
//extern UINT16  RadioOutputVol;


_ATTR_RADIOWIN_INIT_BSS_ INT8U  FmSeekStopFlag ;
extern                   UINT8  FMDeInitDisable;

/*
--------------------------------------------------------------------------------
  Function name : void RadioWinIntInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                  WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_INIT_CODE_
void RadioWinIntInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioWinIntDeInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                  WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_DEINIT_CODE_
void RadioWinIntDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioWinMsgInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                  WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_INIT_CODE_
void RadioWinMsgInit(void *pArg)
{
    SendMsg(MSG_RADIOSUBWIN_DISPLAY_FM_MENU);
    ClearMsg(MSG_DIALOG_KEY_OK);
    ClearMsg(MSG_DIALOG_KEY_CANCEL);

    if (pArg != NULL)
    {
        if (((RADIO_WIN_ARG*)pArg)->AutoPreset == 1)
        {
            SendMsg(MSG_RADIOSUBWIN_SETTING_AUTO_PRESET);
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioWinMsgDeInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_DEINIT_CODE_
void RadioWinMsgDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioWinInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_CODE_
void RadioWinInit(void *pArg)
{
    RadioWinMsgInit(pArg);

    KeyReset();

    if (TRUE == ThreadCheck(pMainThread, &FMThread))
    {
        FmInitFlag = TRUE;
    }
    else
    {
        FmInitFlag = FALSE;
    }

    if(!GetMsg(MSG_RADIOSUBWIN_REC_OR_DELETE_REC))
    {
        if (FmFunctionSele != 0)
            FmFunctionSele = 0;

        PreFmKeyCounter  = 0;
        FmKeyCounter = 0;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioWinDeInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                  WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_CODE_
void RadioWinDeInit(void)
{
    RadioWinIntDeInit();
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 RadioWinService(void)
  Author        : WangBo
  Description   : it is radio service handle that is used to handle window message,window service task etc.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                  WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_SERVICE_CODE_
UINT32 RadioWinService(void)
{
    UINT32 RetVal = 0;
    TASK_ARG TaskArg;
    UINT32 RecordDirClus;
    UINT16 RecordQuality;   //quality of recording.
    UINT16 RecordFmt;   //format of recording.

    if (TRUE == GetMsg(MSG_RADIOSUBWIN_DIALOG))
    {
        TaskArg.Dialog.Button  = DIALOG_BUTTON_YES;  //init Dialog parameter
        TaskArg.Dialog.TitleID = SID_WARNING;
        if (FmListSaveAndDel == 1)  //save
        {
            TaskArg.Dialog.ContentID  =  SID_SAVE_CHANNEL;
        }
        else                        //delete
        {
            TaskArg.Dialog.ContentID  =  SID_DELETE_CHANNEL;
        }

        WinCreat(&RadioWin, &DialogWin, &TaskArg);
    }

    if (TRUE == GetMsg(MSG_RADIOSUBWIN_DISPLAY_FLAG))
    {
        WinCreat(&RadioWin, &RadioSubFreqWin, NULL);
    }

    //when get dialog message,create freq list sub window
    if (FmSaveAndDel() == 1)
    {
        WinCreat(&RadioWin, &RadioSubFreqWin, NULL);
    }

    //To avoid screen controled by fm control early then unable display all radio screen
    if ((TRUE == CheckMsg(MSG_FM_AUTOSEARCH)) && (FALSE == CheckMsg(MSG_RADIOWIN_DISPLAY_ALL)))
    {
        if (TRUE == GetMsg(MSG_FM_AUTOSEARCH))
        {
            FREQ_EnterModule(FREQ_FMAUTOSEARCH);
            FM_Process(MSG_FM_AUTOSEARCH,(void *)1);

            SendMsg(MSG_RADIOWIN_DISPLAY_FREQ);
            SendMsg(MSG_RADIOWIN_DISPLAY_CH);
            RadioGetGuaVal();
            SendMsg(MSG_RADIOWIN_DISPLAY_GUAGE);
            FREQ_ExitModule(FREQ_FMAUTOSEARCH);
        }
    }

    if (TRUE == GetMsg(MSG_RADIOSUBWIN_DISPLAY_FM_MENU))
    {
        FmFunctionSele_bit=0 ;
        WinCreat(&RadioWin, &RadioSubFuncWin, NULL);
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_EXIT_FM_MENU))
    {
        //FMDeInitDisable = 0;
        //ThreadDelete(&pMainThread, &FMThread);
        TaskArg.MainMenu.MenuID = MAINMENU_ID_RADIO;
        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
        RetVal = 1;
    }

    if (TRUE == GetMsg(MSG_RADIOSUBWIN_DISPLAY_REC))
    {
#ifdef _FM_RECORD_
        //FMDeInitDisable = 1;
        RecordQuality = gSysConfig.RecordConfig.RecordQuality;
        RecordFmt = gSysConfig.RecordConfig.RecordFmt;
        if((RecordFmt != FM_RecordFmt)
            || (RecordQuality != FM_RecordQuality))
        {
            FM_Stop(1);
            DelayMs(10);
            FM_RecordFmt = RecordFmt;
            FM_RecordQuality = RecordQuality;
            FM_Codec_Config(FM_RecordQuality,FM_RecordFmt);
            FM_Start(1);
        }
        if (RECORD_QUALITY_HIGH == FM_RecordQuality)  //high quality record.
        {
            TaskArg.Record.RecordExitTaskID = TASK_ID_RADIO;
            TaskArg.Record.RecordType = RECORD_TYPE_LINEIN1;
            TaskArg.Record.RecordCodecConfig = 1;
            TaskArg.Record.RecordSampleRate = ENC_WAV_H_FS;
            TaskArg.Record.RecordChannel = RECORD_CHANNEL_STERO;
            TaskArg.Record.RecordDataWidth = RECORD_DATAWIDTH_24BIT;
            TaskArg.Record.RecordEncodeType = RECORD_ENCODE_TYPE_PCM;
        }
        else    // low quality
        {
            TaskArg.Record.RecordExitTaskID = TASK_ID_RADIO;
            TaskArg.Record.RecordType = RECORD_TYPE_LINEIN1;
            TaskArg.Record.RecordCodecConfig = 1;
            TaskArg.Record.RecordSampleRate = ENC_WAV_N_FS;
            TaskArg.Record.RecordChannel = RECORD_CHANNEL_STERO;
            TaskArg.Record.RecordDataWidth = RECORD_DATAWIDTH_16BIT;
            TaskArg.Record.RecordEncodeType = RECORD_ENCODE_TYPE_WAV;
        }
        TaskSwitch(TASK_ID_RECORD,&TaskArg);
        RetVal = 1;
#else
        SendMsg(MSG_RADIOSUBWIN_DISPLAY_FM_MENU);
#endif
    }

#ifdef _FM_RECORD_
    if (TRUE == GetMsg(MSG_RADIOSUBWIN_DISPLAY_PLAY_RECDATA))
    {
        #ifdef _MEDIA_MODULE_
        gMusicTypeSelID = MUSIC_TYPE_SEL_RECORDFILE;
        TaskArg.MediaBro.TitleAdd = SID_RECORE_FILE;
        #endif
//        ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);

#if 0
        if (GetTotalFiles(GetDirClusIndex("U:\\RECORD     \\FM         "),AudioFileExtString, FS_FAT))
        {
            TaskArg.MediaBro.CurId= 0;
            TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
            RetVal = 1;
        }
        else
        {
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
            TaskArg.Message.HoldTime  = 2;
            WinCreat(&RadioWin, &MessageBoxWin, &TaskArg);
        }
#else
        if (gSysConfig.MedialibPara.gRecordFmFileNum)
        {
            #ifdef _MEDIA_MODULE_
            TaskArg.MediaBro.CurId= 0;
            TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
            RetVal = 1;
            #else   //_SPINOR_
            SendMsg(MSG_RADIOSUBWIN_DISPLAY_CANNOT_CONTINUE);
            #endif
        }
        else
        {
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
            TaskArg.Message.HoldTime  = 2;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            WinCreat(&RadioWin, &MessageBoxWin, &TaskArg);
        }
#endif
    }

    if (TRUE == GetMsg(MSG_RADIOSUBWIN_DISPLAY_DELETE_RECDATA))
    {
        #ifdef _MEDIA_MODULE_
        gMusicTypeSelID = MUSIC_TYPE_SEL_RECORDFILE_DEL;
        TaskArg.MediaBro.TitleAdd = SID_RECORE_FILE;
        #endif
//        ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
#if 0
        if (GetTotalFiles(GetDirClusIndex("U:\\RECORD     \\FM         "),AudioFileExtString, FS_FAT))
        {
            TaskArg.MediaBro.CurId= 0;
            TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
            RetVal = 1;
        }
        else
        {
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
            TaskArg.Message.HoldTime  = 2;
            WinCreat(&RadioWin, &MessageBoxWin, &TaskArg);
        }
#else
        if (gSysConfig.MedialibPara.gRecordFmFileNum)
        {
            #ifdef _MEDIA_MODULE_
            TaskArg.MediaBro.CurId= 0;
            TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
            RetVal = 1;
            #else   //_SPINOR_
            SendMsg(MSG_RADIOSUBWIN_DISPLAY_CANNOT_CONTINUE);
            #endif
        }
        else
        {
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
            TaskArg.Message.HoldTime  = 2;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            WinCreat(&RadioWin, &MessageBoxWin, &TaskArg);
        }
#endif
    }
#endif

    if (TRUE == GetMsg(MSG_RADIOSUBWIN_DISPLAY_CANNOT_CONTINUE))
    {
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_COULD_NOT_CONTINUE;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&RadioWin, &MessageBoxWin, &TaskArg);
    }

    if (TRUE == GetMsg(MSG_MESSAGEBOX_DESTROY))
    {
        SendMsg(MSG_RADIOSUBWIN_DISPLAY_FM_MENU);
    }

    if ((SeekingStateTracker == SEEKING_STATE_FMCONTROL_SEARCHING))
    {
        if ((gpRadioplayerRegKey->FmState != FM_State_HandSearch) || (gpRadioplayerRegKey->FmState != FM_State_AutoSearch))
        {
            SeekingStateTracker = SEEKING_STATE_RADIOWIN_NONE;
            LCD_ClrRect(0, 27, 29, 36);
            SendMsg(MSG_RADIOWIN_DISPLAY_UP_DOWN);  //show up/down icon when auto search end then change to FM_State_StepStation mode
            //FmKeyCounter = 1; //Avoid back to list screen incorrectlly when focuse on keyCounter 2 to select auto search
        }
    }

    return RetVal;
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioWinPaint(void)
  Author        : WangBo
  Description

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                  WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_SERVICE_CODE_
UINT16 FmSaveAndDel(void)
{
    INT8U i;
    int resetvolumet = 0,j;
    if (FmListSaveAndDel == 1)     //save
    {
        if ((TRUE == GetMsg(MSG_DIALOG_KEY_OK)))
        {
            gpRadioplayerRegKey->FmFreqArray[CurrentChannel] = gbFmFreq;

            if (gpRadioplayerRegKey->FmSaveNum < FREQMAXNUMBLE - 1)
            {
                gpRadioplayerRegKey->FmSaveNum = gpRadioplayerRegKey->FmSaveNum + 1;
            }

            return 1;
        }
        if ((TRUE == GetMsg(MSG_DIALOG_KEY_CANCEL)))
        {
            //gpRadioplayerRegKey->FmFreqArray[CurrentChannel] = 0;  //ylz++
            return 1;
        }
    }

    if (FmListSaveAndDel == 0)   //delete
    {
        if ((TRUE == GetMsg(MSG_DIALOG_KEY_OK)))
        {
            resetvolumet = 1;
            if (CurrentChannel == gpRadioplayerRegKey->FmSaveNum)
            {
                for (j = 0;j < FREQMAXNUMBLE;j++)
                {
                    if (j != CurrentChannel)
                    {
                        if (gpRadioplayerRegKey->FmFreqArray[j] != 0)
                        {
                            gpRadioplayerRegKey->FmSaveNum = j;
                            gbFmFreq = gpRadioplayerRegKey->FmFreqArray[j];
                            break;
                        }
                        gbFmFreq = 8750; // if no FM
                    }
                }

                FM_Process(MSG_FM_START, (void *)resetvolumet);
            }
            gpRadioplayerRegKey->FmFreqArray[CurrentChannel] = 0;
            return 1;
        }
        if ((TRUE == GetMsg(MSG_DIALOG_KEY_CANCEL)))  //back to preset list screen too
        {
            return 1;
        }
    }

    return 0;
}

/**************************************************************************
* Description:  Display CH��Freq��schedule small triangle
* Input      :  NULL
* Output     :  NULL
* return:
***************************************************************************/
_ATTR_RADIOWIN_SERVICE_CODE_
void PaintCHAndFreq(void)
{
    UINT16      temp,temp1,i,j;
    UINT16      TempColor,TempBkColor,TempTxtMode,TempCharSize;
    PICTURE_INFO_STRUCT  PicInfo;
    UINT16 DisplayBuf[10];

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);


    if (gpRadioplayerRegKey->FmState != FM_State_HandSearch)
    {
        //��ʾ CH
        DispPictureWithIDNumAndXYoffset(IMG_ID_FM_CH_BACK , 98, 128);

        DisplayBuf[0] = 'C';    //
        DisplayBuf[1] = 'H';
        DisplayBuf[2] = ':';
        DisplayBuf[3] = (gbFmFreqIndex + 1)/ 10  + '0';
        DisplayBuf[4] = (gbFmFreqIndex + 1) % 10 + '0';
        DisplayBuf[5] = 0;

        LCD_NFDispStringAt(98,128,DisplayBuf);// //CHxx

    }
    //display Freq
    DisplayBuf[0] = gbFmFreq / 10000;  //freq 8700~10800

    if (DisplayBuf[0]==1)
    {
        DisplayBuf[0] = gbFmFreq/10000+'0';
        temp = gbFmFreq%10000;
        DisplayBuf[1] = temp/1000+'0' ;
        temp = temp % 1000;
        DisplayBuf[2] = temp/100+'0' ;
        temp = temp % 100;
        DisplayBuf[3] = '.' ;
        DisplayBuf[4] = temp/10+'0' ;
        DisplayBuf[5] = 'M';
        DisplayBuf[6] = 'H';
        DisplayBuf[7] = 'z';
        DisplayBuf[8] = 0;
    }
    else
    {
        temp = gbFmFreq%10000;
        DisplayBuf[0] = temp/1000+'0' ;
        temp = temp % 1000;
        DisplayBuf[1] = temp/100+'0' ;
        temp = temp % 100;
        DisplayBuf[2] = '.' ;
        DisplayBuf[3] = temp/10+'0' ;
        DisplayBuf[4] = 'M';
        DisplayBuf[5] = 'H';
        DisplayBuf[6] = 'z';
        DisplayBuf[7] = 0;
    }
    DispPictureWithIDNumAndXYoffset(IMG_ID_FM_BACKGROUND01,0,0);
    LCD_NFDispStringAt(42,46,DisplayBuf);

    RadioGetGuaVal();

    if (RadioGuageVal > FM_GUAGE_CONST)
    {
        RadioGuageVal = 0;
    }

    for (i = 0; i < RadioGuageVal; i++)
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_FM_FREGUAGE, 7+i, 118);          //move schedule small triangle
    }

    for (;i<FM_GUAGE_CONST;i++)
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_FM_FREGUAGE_BK, 7+i, 118);
    }

#ifdef _FRAME_BUFFER_
    WinPaintDmaFillImage2Lcd();
#endif

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioWinPaint(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                  WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_SERVICE_CODE_
void RadioWinPaint(void)
{
    UINT16      temp,temp1,i,j;
    UINT16      TempColor,TempBkColor,TempTxtMode,TempCharSize;
    PICTURE_INFO_STRUCT  PicInfo;
    UINT16      DisplayBuf[10];

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_RADIOWIN_DISPLAY_ALL)))
    {
        DispPictureWithIDNum(IMG_ID_FM_BACKGROUND);                       //display background
        SendMsg(MSG_BATTERY_UPDATE);
        SendMsg(MSG_RADIOWIN_DISPLAY_STEREO);
        SendMsg(MSG_RADIOWIN_DISPLAY_REGION);
        SendMsg(MSG_RADIOWIN_DISPLAY_FREQ);
        SendMsg(MSG_RADIOWIN_DISPLAY_GUAGE);
        SendMsg(MSG_RADIOWIN_DISPLAY_CH);
        SendMsg(MSG_RADIOWIN_DISPLAY_VOL);
        SendMsg(MSG_RADIOWIN_DISPLAY_SETTING);
        SendMsg(MSG_RADIOWIN_DISPLAY_BASICFREQ);
        SendMsg(MSG_RADIOWIN_DISPLAY_STATUS);
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_SETTING))
    {
        uint16 sigleCharWth;

        if(TempCharSize == FONT_12x12)
        {
            sigleCharWth = AS_CHAR_XSIZE_12;
        }
        else if(TempCharSize == FONT_16x16)
        {
            sigleCharWth = AS_CHAR_XSIZE_16;
        }

        DisplayPicture_part(IMG_ID_FM_BACKGROUND,0,0,0,26);

        //If use font12 to display,one ascii charactor width is 6 pixs.
        if (FmFunctionSele == FM_OPT_MODE_MANUAL)
        {
            int xwidth = StrLenA("manual") * sigleCharWth;
            //DEBUG("manual xwidth = %d",xwidth);
            DisplayMenuStrWithIDNum(4,5,xwidth,15,LCD_TEXTALIGN_LEFT,FMFont[FM_OPT_MODE_MANUAL]);
        }
        else if (FmFunctionSele == FM_OPT_MODE_AUTOSAVE)
        {
            int xwidth = StrLenA("auto preset") * sigleCharWth;
            //DEBUG("auto preset xwidth = %d",xwidth);
            DisplayMenuStrWithIDNum(4,5,xwidth,15,LCD_TEXTALIGN_LEFT,FMFont[FM_OPT_MODE_AUTOSAVE]);
        }
        else if (FmFunctionSele == FM_OPT_MODE_PRESET)
        {
            int xwidth = StrLenA("preset") * sigleCharWth;
            DisplayMenuStrWithIDNum(4,5,xwidth,15,LCD_TEXTALIGN_LEFT,FMFont[FM_OPT_MODE_PRESET]);
        }
        else
        {
            if (FmFunctionSeleBack == FM_OPT_MODE_MANUAL)
            {
                int xwidth = StrLenA("manual") * sigleCharWth;
                DisplayMenuStrWithIDNum(4,5,xwidth,15,LCD_TEXTALIGN_LEFT,FMFont[FM_OPT_MODE_MANUAL]);
            }
            else if (FmFunctionSeleBack == FM_OPT_MODE_AUTOSAVE)
            {
                int xwidth = StrLenA("auto preset") * sigleCharWth;
                DisplayMenuStrWithIDNum(4,5,xwidth,15,LCD_TEXTALIGN_LEFT,FMFont[FM_OPT_MODE_AUTOSAVE]);
            }
            else if (FmFunctionSeleBack == FM_OPT_MODE_PRESET)
            {
                int xwidth = StrLenA("preset") * sigleCharWth;
                DisplayMenuStrWithIDNum(4,5,xwidth,15,LCD_TEXTALIGN_LEFT,FMFont[FM_OPT_MODE_PRESET]);
            }
        }
    }


    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNum(IMG_ID_MUSIC_BATTERY01 + RadioBatteryLevel);
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_FREQ))  //display freq
    {
        temp1 = gbFmFreq / 10000;  //freq 8700~10800
        if (temp1)
        {
            DisplayBuf[0] = gbFmFreq/10000+'0';
            temp = gbFmFreq%10000;
            DisplayBuf[1] = temp/1000+'0' ;
            temp = temp % 1000;
            DisplayBuf[2] = temp/100+'0' ;
            temp = temp % 100;
            DisplayBuf[3] = '.' ;
            DisplayBuf[4] = temp/10+'0' ;
            DisplayBuf[5] = 'M';
            DisplayBuf[6] = 'H';
            DisplayBuf[7] = 'z';
            DisplayBuf[8] = 0;
        }
        else
        {
            temp = gbFmFreq%10000;
            DisplayBuf[0] = temp/1000+'0' ;
            temp = temp % 1000;
            DisplayBuf[1] = temp/100+'0' ;
            temp = temp % 100;
            DisplayBuf[2] = '.' ;
            DisplayBuf[3] = temp/10+'0' ;
            DisplayBuf[4] = 'M';
            DisplayBuf[5] = 'H';
            DisplayBuf[6] = 'z';
            DisplayBuf[7] = 0;
        }
        DispPictureWithIDNumAndXYoffset(IMG_ID_FM_BACKGROUND01,0,0);
        LCD_NFDispStringAt(42,46,DisplayBuf);
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_GUAGE))
    {
        RadioGetGuaVal();
        DEBUG("RadioGuageVal  = %d",RadioGuageVal);
        if (RadioGuageVal > FM_GUAGE_CONST)
        {
            RadioGuageVal=0;
        }

        for (i = 0; i < RadioGuageVal; i++)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_FM_FREGUAGE, 7+i, 118);
        }

        for (; i<FM_GUAGE_CONST; i++)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_FM_FREGUAGE_BK, 7+i, 118);
        }
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_REGION))
    {
        DisplayMenuStrWithIDNum(13,142,30,15,LCD_TEXTALIGN_LEFT,SID_FM_REGION_CHINA+RadioArea);
        //DisplayMenuStrWithIDNum(13,142,30,15,LCD_TEXTALIGN_LEFT,SID_RADIO);
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_BASICFREQ))
    {
        if ((RadioArea==RADIOAREA_CHINA)||(RadioArea==RADIOAREA_EUROPE)||(RadioArea==RADIOAREA_USA))
        {
            DisplayBuf[0] = '8';
            DisplayBuf[1] = '7';
            DisplayBuf[2] = 'M';
            DisplayBuf[3] = 'H';
            DisplayBuf[4] = 'z';
            DisplayBuf[5] = 0;
            LCD_NFDispStringAt(5,104,DisplayBuf);

            DisplayBuf[0] = '1';
            DisplayBuf[1] = '0';
            DisplayBuf[2] = '8';
            DisplayBuf[3] = 'M';
            DisplayBuf[4] = 'H';
            DisplayBuf[5] = 'z';
            DisplayBuf[6] = 0;
            LCD_NFDispStringAt(88,104,DisplayBuf);
        }
        else
        {
            DisplayBuf[0] = '7';
            DisplayBuf[1] = '6';
            DisplayBuf[2] = 'M';
            DisplayBuf[3] = 'H';
            DisplayBuf[4] = 'z';
            DisplayBuf[5] = 0;
            LCD_NFDispStringAt(5,104,DisplayBuf);

            DisplayBuf[0] = '9';
            DisplayBuf[1] = '0';
            DisplayBuf[2] = 'M';
            DisplayBuf[3] = 'H';
            DisplayBuf[4] = 'z';
            DisplayBuf[5] = 0;
            LCD_NFDispStringAt(88,104,DisplayBuf);
        }

    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_CH) )
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_FM_CH_BACK , 98, 128);

        DisplayBuf[0] = 'C';
        DisplayBuf[1] = 'H';
        DisplayBuf[2] = ':';
        DisplayBuf[3] = (gbFmFreqIndex + 1) / 10 + '0';
        DisplayBuf[4] = (gbFmFreqIndex + 1) % 10 + '0';
        DisplayBuf[5] = 0;

        if ((gpRadioplayerRegKey->FmState == FM_State_AutoSearch ) || (gpRadioplayerRegKey->FmState == FM_State_StepStation)) //�Զ�����Ԥ��״̬����ʾ CH
        {
            LCD_NFDispStringAt(98,128,DisplayBuf);// //CHxx
        }
    }
    else
    {
        //DispPictureWithIDNum(IMG_ID_RADIO_CHOFF);                          //CH OFF
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_STEREO))// && (gpRadioplayerRegKey->FmStereo == RADIO_STEREO_OPEN))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_FM_STEREO , 3 , 128);
        if (gpRadioplayerRegKey->FmStereo == RADIO_STEREO_OPEN)
        {          // "������"
            DisplayBuf[0] = 'S';
            DisplayBuf[1] = 't';
            DisplayBuf[2] = 'e';
            DisplayBuf[3] = 'r';
            DisplayBuf[4] = 'e';
            DisplayBuf[5] = 'o';
            DisplayBuf[6] =  0;

            LCD_NFDispStringAt(3,128,DisplayBuf);
        }
        else
        {
            DisplayBuf[0] = 'M';
            DisplayBuf[1] = 'o';
            DisplayBuf[2] = 'n';
            DisplayBuf[3] = 'o';
            DisplayBuf[4] = 0;

            LCD_NFDispStringAt(3,128,DisplayBuf);
        }
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_VOL))
    {
        temp= (UINT16)((UINT32)(FM_VOL_GUAGE_CONST) * RadioOutputVol)/32;

        for (i = 0; i < temp; i++)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_FM_VOLUME_GUAGE, 70+i, 147);          //�ƶ�����С����
        }

        for (;i<FM_VOL_GUAGE_CONST;i++)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_FM_VOLUME_GUAGE_BK, 70+i, 147);
        }

        DisplayBuf[0] = (RadioOutputVol-0)/10 + '0';
        DisplayBuf[1] = (RadioOutputVol-0)%10 + '0';
        DisplayBuf[2] = 0;

        DispPictureWithIDNumAndXYoffset(IMG_ID_FM_VOLUME_BK,0,0);
        LCD_NFDispStringAt(113,143,DisplayBuf);

    }

    if(TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_STATUS))
    {
        if(FmInitFlag)
        {
            if(FmStandbyFlag)
            {
                //DEBUG("fm PAUSE");
                DispPictureWithIDNumAndXY(IMG_ID_MUSIC_PAUSE_STATE,56,124);
            }
            else
            {
                //DEBUG("FM play");
                DispPictureWithIDNumAndXY(IMG_ID_MUSIC_PLAY_STATE,56,124);
            }
        }
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}
/*
--------------------------------------------------------------------------------
  Function name : void RadioGetGuaVal(void)
  Author        : WangBo
  Description

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                  WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_SERVICE_CODE_
void RadioGetGuaVal(void)
{
    RadioGuageVal = (UINT16)((UINT32)(FM_GUAGE_CONST) * (gbFmFreq - FmFreqMin) / (FmFreqMax - FmFreqMin));
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioRefreshFreq(void)
  Author        : WangBo
  Description   :

  Input         : NULL
  Return        : NULL

  History:     <author>         <time>         <version>
                WangBo         2009-5-6        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_SERVICE_CODE_
void RadioRefreshFreq(void)
{
    INT8U i,k=0;
    UINT16    FmFreqArray2[FREQMAXNUMBLE];

    for (i = 0;i < FREQMAXNUMBLE;i++)
    {
        if (gpRadioplayerRegKey->FmFreqArray[i] >= FmFreqMinVal && gpRadioplayerRegKey->FmFreqArray[i] <= FmFreqMaxVal)
        {
            FmFreqArray2[k] = gpRadioplayerRegKey->FmFreqArray[i];
            k = k + 1;
        }
    }

    for (i = 0;i < FREQMAXNUMBLE; i++)
    {
        gpRadioplayerRegKey->FmFreqArray[i] = 0;
    }

    for (i = 0; i < k; i++)
    {
        gpRadioplayerRegKey->FmFreqArray[i] =  FmFreqArray2[i];
    }

}

/**************************************************************************
* Description:  slove the qustion that to pause when in auto searching
* Input      :  NULL
* Output     :  NULL
*
***************************************************************************/
_ATTR_RADIOWIN_SERVICE_CODE_
UINT32 RadioKeyStopAutoSearch(void)
{
    UINT32  RadioKeyVal;
    UINT32  RetVal = 0;
    TASK_ARG TaskArg;

    RadioKeyVal =  GetKeyVal();

    switch (RadioKeyVal)
    {
        case KEY_VAL_FFD_SHORT_UP:
        case KEY_VAL_FFW_SHORT_UP:
            RetVal = TRUE;
            break;

        default:
            break;
    }
    return (RetVal);
}
/*
--------------------------------------------------------------------------------
  Function name : UINT32 RadioWinKeyProc(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                  WangBo      2009-4-10       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_SERVICE_CODE_
UINT32 RadioWinKeyProc(void)
{
    UINT32 RadioKeyVal;
    UINT32  RetVal = 0;
    TASK_ARG TaskArg;

    RadioKeyVal =  GetKeyVal();

    switch (RadioKeyVal)
    {
        case KEY_VAL_ESC_SHORT_UP:
            {
                SendMsg(MSG_RADIOSUBWIN_DISPLAY_FM_MENU);
                break;
            }

        case KEY_VAL_ESC_PRESS_START:
            {
                TaskArg.MainMenu.MenuID = MAINMENU_ID_RADIO;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                RetVal = 1;
                break;
            }

        case KEY_VAL_PLAY_SHORT_UP:
            {
                if (FmInitFlag == TRUE)
                {
                    if (FmStandbyFlag == TRUE)
                    {
                        FmStandbyFlag = FALSE;
                        SendMsg(MSG_RADIOSUBWIN_DISPLAY_OFF); //HJ
                        FmDriverTable.Tuner_MuteControl(0);//HJ
                        SendMsg(MSG_RADIOWIN_DISPLAY_STATUS);
                        #if(FM_OUT_CONFIG == FM_I2S_HP)
                        {
                            FM_Start(0);
                        }
                        #endif
                    }
                    else
                    {
                        FmStandbyFlag = TRUE;
                        SendMsg(MSG_RADIOSUBWIN_DISPLAY_OFF);
                        FmDriverTable.Tuner_MuteControl(1);//HJ
                        SendMsg(MSG_RADIOWIN_DISPLAY_STATUS);
                        #if(FM_OUT_CONFIG == FM_I2S_HP)
                        {
                            FM_Stop(0);
                        }
                        #endif
                    }
                }
                break;
            }

        case KEY_VAL_FFD_DOWN:
        case KEY_VAL_FFW_DOWN:
            if (RadioStopSeeking())
            {
                FmSeekStopFlag = 1;
            }
            break;

        case KEY_VAL_FFD_PRESS_START:
            if (gpRadioplayerRegKey->FmState == FM_State_HandStepFreq)
            {
                SendMsg(MSG_RADIOWIN_DISPLAY_SCAN);
                SeekingStateTracker = SEEKING_STATE_RADIOWIN_TRIGER;
            }
            break;


        case KEY_VAL_FFW_PRESS_START:
            if (gpRadioplayerRegKey->FmState == FM_State_HandStepFreq)
            {
                SendMsg(MSG_RADIOWIN_DISPLAY_SCAN);
                SeekingStateTracker = SEEKING_STATE_RADIOWIN_TRIGER;
            }
            break;

        case KEY_VAL_FFD_SHORT_UP:   // short press FFD,add frequency by step
            {
                if (FmSeekStopFlag)                   //do not response RELEASE message,when stop searching station
                {
                    FmSeekStopFlag = 0;              //it prevent frequency step again
                }
                else
                {
                    RadioSetStepFreq(FM_DIRECT_INC);
                    RadioSetStepStation(FM_DIRECT_INC);

                    RadioGetGuaVal();
                    SendMsg(MSG_RADIOWIN_DISPLAY_GUAGE);
                    SendMsg(MSG_RADIOWIN_DISPLAY_FREQ);
                    SendMsg(MSG_RADIOWIN_DISPLAY_CH);
                    SendMsg(MSG_RADIOWIN_DISPLAY_STEREO);
                }
                break;
            }

        case KEY_VAL_FFW_SHORT_UP: //�̰� FFW ����������Ƶ��
            {
                if (FmSeekStopFlag)                //��ֹͣ��̨ʱ����ӦRELEASE��Ϣ
                {
                    FmSeekStopFlag = 0;           //������ֹƵ���ٲ���
                }
                else
                {
                    RadioSetStepFreq(FM_DIRECT_DEC);
                    RadioSetStepStation(FM_DIRECT_DEC);
                    RadioGetGuaVal();

                    SendMsg(MSG_RADIOWIN_DISPLAY_FREQ);
                    SendMsg(MSG_RADIOWIN_DISPLAY_GUAGE);
                    SendMsg(MSG_RADIOWIN_DISPLAY_CH);
                    SendMsg(MSG_RADIOWIN_DISPLAY_STEREO);
                }
                break;
            }

        case KEY_VAL_FFD_PRESS: //���� FFD,��������Ƶ��
            {
                if (!FmSeekStopFlag)
                {
                    RadioQuickSeekStation(FM_DIRECT_INC);
                }
                break;
            }

        case KEY_VAL_FFW_PRESS: //���� FFW,��������Ƶ��
            {
                if (!FmSeekStopFlag)
                {
                    RadioQuickSeekStation(FM_DIRECT_DEC);
                }
                break;
            }

        case KEY_VAL_FFD_LONG_UP:    // ���� FFD ����Ѱ��һ�����õ�Ƶ��
            {
                if (FmSeekStopFlag)
                {
                    FmSeekStopFlag = 0;
                }
                else
                {
                    gbFreqBack = gpRadioplayerRegKey->FmFreq;
                    RadioStartSeekByHand(FM_DIRECT_INC);
                    SendMsg(MSG_RADIOWIN_DISPLAY_STEREO);
                }
                break;

            }

        case KEY_VAL_FFW_LONG_UP:    // ���� FFW ����Ѱ��һ�����õ�Ƶ��
            {
                if (FmSeekStopFlag)
                {
                    FmSeekStopFlag = 0;
                }
                else
                {
                    gbFreqBack = gpRadioplayerRegKey->FmFreq;
                    RadioStartSeekByHand(FM_DIRECT_DEC);
                    SendMsg(MSG_RADIOWIN_DISPLAY_STEREO);
                }
                break;
            }

        case KEY_VAL_DOWN_PRESS:    //��ʱ�䣬��������
        case KEY_VAL_DOWN_DOWN:
            {
                if ((gpRadioplayerRegKey->FmState == FM_State_HandSearch)||(gpRadioplayerRegKey->FmState == FM_State_AutoSearch))
                {
                    return TRUE;
                }

                if (RadioOutputVol)
                {
                    RadioOutputVol--;
                    FM_Process(MSG_RADIO_VOLUMESET,NULL);
                }
                SendMsg(MSG_RADIOWIN_DISPLAY_VOL);
                break;
            }

        case KEY_VAL_UP_PRESS:        //��ʱ�䣬��������
        case KEY_VAL_UP_DOWN:
            {
                if ((gpRadioplayerRegKey->FmState == FM_State_HandSearch)||(gpRadioplayerRegKey->FmState == FM_State_AutoSearch))
                {
                    return TRUE;
                }
                if (RadioOutputVol < MAX_VOLUME)
                {
                    RadioOutputVol++;
                    FM_Process(MSG_RADIO_VOLUMESET,NULL);
                }
                SendMsg(MSG_RADIOWIN_DISPLAY_VOL);
                break;
            }

        case KEY_VAL_HOLD_ON:
            {
                TaskArg.Hold.HoldAction = HOLD_STATE_ON;
                WinCreat(&RadioWin, &HoldWin, &TaskArg);
                break;
            }

        case KEY_VAL_HOLD_OFF:
            {
                TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
                WinCreat(&RadioWin, &HoldWin, &TaskArg);
                break;
            }

        default:
            break;

    }
    return (RetVal);
}

/*
********************************************************************************
*
*                         End of RadioWin.c
*
********************************************************************************
*/
#endif


