/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:  MusicWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               zhangshuai      2009-02-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_MUSICWIN_

#include "SysInclude.h"

#ifdef _MUSIC_

#include "MusicWinInterface.h"
#include "MusicLrc.h"
#include "MusicWin.h"
#include "mainmenu.h"
#include "MessageBox.h"
#include "Hold.h"
#include "ID3.h"
#include "AddrSaveMacro.h"
#include "MediaBroWin.h"
#include "medialibwin.h"
#include "rockcodec.h"
#include "LcdInterface.h"

#include "image_main.h"
#include "ImageControl.h"
#ifdef THUMB_DEC_INCLUDE
#include "thumbnail_parse.h"
#endif

#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#include "SetBluetooth.h"
#endif

extern INT32 MusicNextFile ;
extern uint32 oldtick;

extern uint32 audio_have_album_pic;
extern uint32 audio_dec_album_done;

_ATTR_MUSIC_BSS_  static bool IsCheckedAlbum;

/*
--------------------------------------------------------------------------------
  Function name : void MusicWinIntInit(void)
  Author        : ZHengYongzhi
  Description   : main menu interrupt initial,register the interrupt functions that main menu will use,

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_INIT_CODE_
void MusicWinIntInit(void)
{

}

/*
--------------------------------------------------------------------------------
  Function name : void MusicWinIntDeInit(void)
  Author        : ZHengYongzhi
  Description   : main menu interrupt auti-initial,unload the interrupt functions

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_DEINIT_CODE_
void MusicWinIntDeInit(void)
{

}

/*
--------------------------------------------------------------------------------
  Function name : void MusicWinMsgInit(void)
  Author        : ZHengYongzhi
  Description   : main menu message initial��initial the message parameters that main menu will use

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_INIT_CODE_
void MusicWinMsgInit(void *pArg)
{
    //MusicWin MsgInit
    MediaTitleAdd = ((MUSIC_WIN_ARG*)pArg)->MediaTitleAdd;
    SendMsg(MSG_MUSIC_DISPLAY_ALL);
    ClearMsg(MSG_AUDIO_CARD_CHECK);
    ClearMsg(MSG_AUDIO_FILE_ERROR);
}

/*
--------------------------------------------------------------------------------
  Function name : void MusicWinMsgDeInit(void)
  Author        : ZHengYongzhi
  Description   : main menu message auti-initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_DEINIT_CODE_
void MusicWinMsgDeInit(void)
{
    ClearMsg(MSG_MEDIA_BREAKPOINT_PLAY);
}

/*
--------------------------------------------------------------------------------
  Function name : void MusicWinInit(void)
  Author        : ZHengYongzhi
  Description   : main menu message initial function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_CODE_
void MusicWinInit(void *pArg)
{
    DEBUG("MusicWin Enter");

    MusicDispType = MUSIC_DISPLAY_NOR;
    CurrentTimeSecBk = 0xffffffff;
    gIsMusicWinFlg = TRUE;
    audio_dec_album_save = 0;
    audio_dec_album_done = 0;
    audio_have_album_pic = 0;
    AudioAlbumHandle = (FILE*) - 1;

    IsCheckedAlbum = FALSE;

    KeyReset();
    MusicWinMsgInit(pArg);
    MusicWinIntInit();

    MusicWinSvcStart(pArg);

//    if ((TRUE == CheckMsg(MSG_AUDIO_FILE_ERROR)) || (TRUE == CheckMsg(MSG_AUDIO_DECODE_ERROR)))
//    {
//        DEBUG("XXXXXXXXXX DELETE MusicThread!!! XXXXXXXXX");
//        ThreadDelete(&pMainThread, &MusicThread);//hoshi
//    }

    SetMusicUIState(1);//2009-5-20 zs

    GetPictureInfoForBuffer(IMG_ID_FM_FREGUAGE, &MusicSchedPicInfo);
    GetPictureResourceToBuffer(IMG_ID_FM_FREGUAGE, MusicSchedBuffer, MUSIC_SCHED_BUF_SIZE);
    GetPictureResourceToBuffer(IMG_ID_FM_FREGUAGE + 1, (uint8*)(&MusicSchedBuffer[0] + MUSIC_SCHED_BUF_SIZE), MUSIC_SCHED_BUF_SIZE);
}

/*
--------------------------------------------------------------------------------
  Function name : void MusicWinDeInit(void)
  Author        : ZHengYongzhi
  Description   :  main menu message auti-initial function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_CODE_
void MusicWinDeInit(void)
{
    if (MusicDispType == MUSIC_DISPLAY_LRC)
    {
        MusicLrcDeInit();
    }

#ifdef THUMB_DEC_INCLUDE

    if (AudioAlbumHandle != (FILE*) - 1)
    {
        FileClose((HANDLE)AudioAlbumHandle);
        AudioAlbumHandle = (FILE*) - 1;
    }

#endif

    MusicWinSvcStop();
    MusicWinIntDeInit();
    MusicWinMsgDeInit();
    SetMusicUIState(0);//2009-5-20 zs
    gIsMusicWinFlg = FALSE;

    IsCheckedAlbum = FALSE;
    DEBUG("MusicWin Exit");
}
/*
--------------------------------------------------------------------------------
  Function name : void MusicWinScrollFileNameInit(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_SERVICE_CODE_
void MusicWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
    PicturePartInfo     PicPartInfo;

    PicPartInfo.x       = pRect->x0;
    PicPartInfo.y       = pRect->y0;
    PicPartInfo.yoffset = 0;
    PicPartInfo.ysize   = CH_CHAR_XSIZE_12;

    PicPartInfo.pictureIDNump = ImageID;
    SetScrollStringInfo(pRect, PicPartInfo, pstr, Speed);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MusicWinService(void)
  Author        : ZHengYongzhi
  Description   :  main menu message service function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_SERVICE_CODE_
UINT32 MusicWinService(void)
{
    UINT32 Retval = 0;
    TASK_ARG TaskArg;
    FS_TYPE fsType;
    UINT8   *fileEx;

//-----------------------------------------------------------------------------
//message of backgroud dialog
    if (TRUE == GetMsg(MSG_AUDIO_NOFILE))
    {
        ThreadDelete(&pMainThread, &MusicThread);

        MusicWinDialogType = MUSICWIN_DIALOG_NOFILE;
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&MusicWin, &MessageBoxWin, &TaskArg);
    }

    if (TRUE == GetMsg(MSG_AUDIO_CARD_CHECK))
    {
        ThreadDelete(&pMainThread, &MusicThread);

        TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
        Retval = 1;
    }

    if ((TRUE == GetMsg(MSG_AUDIO_FILE_ERROR)) || (TRUE == GetMsg(MSG_AUDIO_DECODE_ERROR)))// by zs 06.09 .chad.ma 20151015
    {
#ifndef FILE_ERROR_DIALOG
        MusicWinDialogType = MUSICWIN_DIALOG_FORMAT_ERROR;
        SendMsg(MSG_MESSAGEBOX_DESTROY);
#else
        //LCD_ClrSrc();
        MusicWinDialogType = MUSICWIN_DIALOG_FORMAT_ERROR;
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_FILE_FORMAT_ERROR;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&MusicWin, &MessageBoxWin, &TaskArg);
#endif

        MusicPlayStateFF = 0;
    }

    if (TRUE == GetMsg(MSG_MESSAGEBOX_DESTROY))
    {
        if (MUSICWIN_DIALOG_FORMAT_ERROR == MusicWinDialogType)
        {
            uint32 Playednumber = AudioFileInfo.CurrentFileNum;

            if (pAudioRegKey->PlayOrder == AUDIO_RAND)
            {
                Playednumber = AudioPlayFileNum;
            }

            AudioErrorFileCount++;

            if ((AudioFileInfo.TotalFiles <= AudioErrorFileCount)
                || (AUDIO_REPEAT == pAudioRegKey->RepeatMode)
                || (((AUDIO_FOLDER_ONCE == pAudioRegKey->RepeatMode)
                     || (AUIDO_FOLDER_REPEAT == pAudioRegKey->RepeatMode)
                     || (AUDIO_ALLONCE == pAudioRegKey->RepeatMode)
                     || (AUDIO_ALLREPEAT == pAudioRegKey->RepeatMode)
                     || (AUDIO_REPEAT1 == pAudioRegKey->RepeatMode))
                    && (Playednumber >= AudioFileInfo.TotalFiles)))
            {
                AudioErrorFileCount = 0;
                ThreadDelete(&pMainThread, &MusicThread);

                if ((AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_BROWSER)
                    && (AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_FOLDER))
                {
                    gMusicTypeSelID = AudioFileInfo.ucSelPlayType;
                    TaskArg.MediaBro.CurId = 1;
                    TaskArg.MediaBro.TitleAdd = MediaTitleAdd;
                    memcpy((UINT8*) & (TaskArg.MediaBro.MediaDirTreeInfo),
                           (UINT8*) & (gSysConfig.MediaDirTreeInfo),
                           sizeof(SYS_MEDIABRO_DIR_TREE_STRUCT));
                    TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
                }
                else//the playing musi is from brower,so return to brower
                {
                    if ((AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_BROWSER))
                    {
                        TaskArg.Browser.FileType = FileTypeALL;
                        TaskArg.Browser.FileNum  = 0;
                        TaskArg.Browser.FromWhere = 0; // from browser
                    }
                    else
                    {
                        TaskArg.Browser.FileType = FileTypeAudio;

                        if (pAudioRegKey->RepeatMode >= AUDIO_ALLONCE)
                        {
                            TaskArg.Browser.FileNum  = (AudioPlayFileNumSave + AudioFileInfo.CurrentFileNum);
                        }
                        else
                        {
                            TaskArg.Browser.FileNum  = AudioFileInfo.CurrentFileNum;
                        }

                        TaskArg.Browser.FromWhere = 1; // from folder
#ifndef AUDIOHOLDONPLAY

                        if ((AUDIO_FOLDER_ONCE == pAudioRegKey->RepeatMode)
                            || (AUIDO_FOLDER_REPEAT == pAudioRegKey->RepeatMode)
                            || (AUDIO_REPEAT == pAudioRegKey->RepeatMode))
                        {
                            TaskArg.Browser.FileNum = GetGlobeFileNum(AudioFileInfo.CurrentFileNum,
                                                      AudioFileInfo.FindData.Clus,
                                                      AudioFileExtString, MUSIC_DB);
                        }

#endif
                    }

                    TaskSwitch(TASK_ID_BROWSER, &TaskArg);
                }

#ifdef AUDIOHOLDONPLAY
                gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif
                Retval = 1;
            }
            else
            {
                AudioDecodeProc(MSG_AUDIO_STOP, (void*)AUDIO_STOP_NORMAL);

                if (AudioCodecOpenErr != 1)
                    SendMsg(MSG_MUSIC_DISPLAY_ALL);
            }
        }
        else
        {
            #if 0
            ThreadDelete(&pMainThread, &MusicThread);
            TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            #endif
            BroswerFlag = FALSE;
            TaskArg.Browser.FileType = FileTypeAudio;
            TaskArg.Browser.FromWhere = 1;
            fsType = MUSIC_DB;
            fileEx = (UINT8*)AudioFileExtString;
            TaskArg.Browser.FileNum = GetGlobeFileNum(AudioFileInfo.CurrentFileNum,
                                                  AudioFileInfo.FindData.Clus,
                                                  fileEx,
                                                  fsType);
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            Retval = 1;
        }
    }

//-----------------------------------------------------------------------------
//backgroud handle message
    if (TRUE == GetMsg(MSG_AUDIO_NEXTFILE))
    {
        if (MusicDispType == MUSIC_DISPLAY_LRC)
        {
            MusicLrcDeInit();
        }
    }

    if (TRUE == GetMsg(MSG_AUDIO_INIT_SUCESS))
    {
#ifdef THUMB_DEC_INCLUDE
        DEBUG("MSG_AUDIO_INIT_SUCESS  IsCheckedAlbum = %d", IsCheckedAlbum);

        if (IsCheckedAlbum)
            IsCheckedAlbum = FALSE;
#endif
        SendMsg(MSG_MUSIC_DISPLAY_ALL);
    }

    if (TRUE == GetMsg(MSG_AUDIO_EXIT))
    {
        ThreadDelete(&pMainThread, &MusicThread);
        TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
        Retval = 1;
    }

//-----------------------------------------------------------------------------
//UI display message
    if (MusicPlayState == AUDIO_STATE_PLAY ||  MusicPlayState == AUDIO_STATE_PAUSE)
    {
        uint32 newtick;
        newtick = GetSysTick();

        if (newtick - oldtick > 20)
        {
            oldtick = newtick;
            SendMsg(MSG_MUSIC_DISPFLAG_CURRENT_TIME);
        }
    }

    return (Retval);
}

_ATTR_MUSIC_SERVICE_CODE_
void MusicDisplayFileNums(UINT16 CutternFileNum, UINT16 TotalPageFileNum)
{
    UINT16 temp;
    UINT16 TxtDrawMode;
    UINT16 DisplayBuf[10];

    DisplayBuf[0] = CutternFileNum / 1000 + '0';
    temp = CutternFileNum % 1000;
    DisplayBuf[1] = temp / 100 + '0' ;
    temp = temp % 100;
    DisplayBuf[2] = temp / 10 + '0' ;
    DisplayBuf[3] = temp % 10 + '0';
    DisplayBuf[4] = '/';
    DisplayBuf[5] = TotalPageFileNum / 1000 + '0' ;
    temp = TotalPageFileNum % 1000;
    DisplayBuf[6] = temp / 100 + '0' ;
    temp = temp % 100;
    DisplayBuf[7] = temp / 10 + '0' ;
    DisplayBuf[8] = temp % 10 + '0';
    DisplayBuf[9] = 0;

    TxtDrawMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    LCD_NFDispStringAt(1, 2, DisplayBuf);
    LCD_SetTextMode(TxtDrawMode);
}

_ATTR_MUSIC_SERVICE_CODE_
void MusicDisplayBitrate(UINT32 Bitrate)
{
    UINT16 temp;
    UINT16 TxtDrawMode;
    UINT16 DisplayBuf[10];

    //DEBUG("Bitrate = %d",Bitrate);
    temp = Bitrate / 1000 / 1000;
    if(temp % 10)   //bitrate is 5bit kbps
    {
        DisplayBuf[0] = Bitrate / 1000 / 10000 + '0';
        temp = (Bitrate / 1000) % 10000;
        DisplayBuf[1] = temp / 1000 + '0' ;
        temp = temp % 1000;
        DisplayBuf[2] = temp / 100 + '0' ;
        temp = temp % 100;
        DisplayBuf[3] = temp / 10 + '0';
        DisplayBuf[4] = temp % 10 + '0';
        DisplayBuf[5] = 'k';
        DisplayBuf[6] = 'b';
        DisplayBuf[7] = 'p';
        DisplayBuf[8] = 's';
        DisplayBuf[9] = 0;
    }
    else    //bitrate is 4bit kbps
    {
        DisplayBuf[0] = Bitrate / 1000 / 1000 + '0';
        temp = (Bitrate / 1000) % 1000;
        DisplayBuf[1] = temp / 100 + '0' ;
        temp = temp % 100;
        DisplayBuf[2] = temp / 10 + '0' ;
        DisplayBuf[3] = temp % 10 + '0';
        DisplayBuf[4] = 'k';
        DisplayBuf[5] = 'b';
        DisplayBuf[6] = 'p';
        DisplayBuf[7] = 's';
        DisplayBuf[8] = 0;
    }

    DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_KBPSBK, 4, 125);
    TxtDrawMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    LCD_NFDispStringAt(4, 125, DisplayBuf);
    LCD_SetTextMode(TxtDrawMode);
}

_ATTR_MUSIC_SERVICE_CODE_
void MusicDisplayTime(UINT16 Hour, UINT16 Min, UINT16 Sec, UINT16 mode)
{
    UINT16 TxtDrawMode;
    UINT16 DisplayBuf[9];

    DisplayBuf[0] = Hour / 10 + '0';
    DisplayBuf[1] = Hour % 10 + '0';
    DisplayBuf[2] = ':';
    DisplayBuf[3] = Min / 10 + '0';
    DisplayBuf[4] = Min % 10 + '0';
    DisplayBuf[5] = ':';
    DisplayBuf[6] = Sec / 10 + '0';
    DisplayBuf[7] = Sec % 10 + '0';
    DisplayBuf[8] = 0;

    TxtDrawMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);

    if (mode == 0)  //display current time
    {
        if (!audio_have_album_pic)  //no album picture
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_CURRTIME_BACK, 2, 87);
            LCD_NFDispStringAt(2, 87, DisplayBuf);
        }
        else    //have album cover picture
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_CURRTIME_BACK, 2, 90);
            LCD_NFDispStringAt(2, 90, DisplayBuf);
        }
    }
    else        //display total time
    {
        if (!audio_have_album_pic)  //no album picture
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_CURRTIME_BACK, 66, 87);
            LCD_NFDispStringAt(83, 87, DisplayBuf);
        }
        else    //have album cover picture
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_CURRTIME_BACK, 66, 90);
            LCD_NFDispStringAt(83, 90, DisplayBuf);
        }
    }

    LCD_SetTextMode(TxtDrawMode);
}

/*
--------------------------------------------------------------------------------
  Function name : void MusicWinPaint(void)
  Author        : Chenwei
  Description   : main menu display program

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               Chenwei        2009/03/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef THUMB_DEC_INCLUDE
extern  int16    Jpg_gImageXSize;
extern  int16    Jpg_gImageYSize;
extern int AudioGetPicInfo(FILE* hFile, int* picType);
extern int ThumbInit(FILE* hFile, int picType);
extern int ThumbDecode(int picType);
extern UINT16 frame_buffer[BUFFER_MAX_NUM][LCD_HEIGHTA][LCD_WIDTH];
#if (FRAME_SUB_BUFFER_NUM > 1)
extern UINT16 frame_bufferB[BUFFER_MAX_NUM][LCD_HEIGHTB][LCD_WIDTH];
#endif
extern UINT16 CurrentFrameIndex;

//_ATTR_MUSIC_BSS_ int AudioAlbumpicType;
_ATTR_MUSIC_SERVICE_CODE_
void MusicWinCheckAlbum(void)
{
    FS_TYPE FsType;
    int     ret = 0;

    int AudioAlbumpicType = -1;

    if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER)
    {
        FsType = MUSIC_DB;
    }

#ifdef _RECORD_
    else if (AudioFileInfo.ucSelPlayType == MUSIC_TYPE_SEL_RECORDFILE)
    {
        FsType = RECORD_DB;
    }
#endif
    else
    {
        FsType = FS_FAT;
    }

    ImageMaxHeight = 128;
    ImageMaxWidth = 128;
    ImageLeft = 0;
    ImageTop = 0;

    SetPicFileType(AUDIO_PIC);
    IsDisplayBackground(1);

    IsCheckedAlbum = TRUE;

    AudioAlbumHandle = (FILE*)FileOpen(AudioFileInfo.Fdt.Name,
                                       AudioFileInfo.FindData.Clus,
                                       AudioFileInfo.FindData.Index - 1,
                                       FsType,
                                       FileOpenStringR);

    if ((int)AudioAlbumHandle == NOT_OPEN_FILE)
    {
        audio_have_album_pic = 0;
        return;
    }
    else
    {
        ret = AudioGetPicInfo(AudioAlbumHandle, &AudioAlbumpicType);

        if (ret)
        {
            ret = ThumbInit(AudioAlbumHandle, AudioAlbumpicType);

            if (ret == 0)
            {
                audio_have_album_pic = 0;
                FileClose((HANDLE)AudioAlbumHandle);
                AudioAlbumHandle = (FILE*) - 1;
                return;
            }
            else
            {
                audio_have_album_pic = 1;

                if ((Jpg_gImageXSize <= 1000) && (Jpg_gImageYSize <= 1000))
                {
                    FREQ_EnterModule(FREQ_JPG);
                    ThumbDecode(AudioAlbumpicType);
                    FREQ_ExitModule(FREQ_JPG);
                    FileClose((HANDLE)AudioAlbumHandle);
                    AudioAlbumHandle = (FILE*) - 1;
                    audio_dec_album_done = 1;
                    return;
                }
                else
                {
                    if (BUFFER_MAX_NUM >= 1)
                    {
                        audio_dec_album_done = 0;
                    }
                    else    // if there is no framebuffer for store album data, skip it
                    {
                        FileClose((HANDLE)AudioAlbumHandle);
                        AudioAlbumHandle = (FILE*) - 1;
                        audio_dec_album_done = 0;
                        audio_have_album_pic = 0;
                    }

                    return;
                }
            }
        }
        else
        {
            audio_have_album_pic = 0;
            FileClose((HANDLE)AudioAlbumHandle);
            AudioAlbumHandle = (FILE*) - 1;
            return;
        }
    }
}
#endif

_ATTR_MUSIC_SERVICE_CODE_
void MusicWinPaint(void)
{
    UINT8                   j, k, l;
    UINT16                  i, VolTemp, TempColor, TempBkColor, TempCharSize, TempTxtMode;
    UINT16                  y, y1;
    UINT16                  *p2Unicode;
    UINT32                  CurrentTimeSec, TotalTimeSec;
    LCD_RECT                r;
    PICTURE_INFO_STRUCT     PicInfo;
    PicturePartInfo         PicPartInfo;
    UINT16                  TextDrawMode;
    char                    SpectrumOutTemp[12];//the size of buffer is defined by real situation.
    UINT16                  DisplayBuf[10];
    uint32                  ret;
    UINT32                  tickCnt;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if(AudioCodecOpenErr)
    {
        //file format error or decodec error, don't paint the baackground
        //to save time to play next song.
        AudioCodecOpenErr = 0;
        return;
    }

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_MUSIC_DISPLAY_ALL)))
    {
        //the other diaplay element message display in here,
        SendMsg(MSG_MUSIC_DISPLAY_BACKGROUND);
        SendMsg(MSG_MUSIC_DISPFLAG_SPECTRUM);
        SendMsg(MSG_MUSIC_DISPFLAG_REPEATMODE);
        SendMsg(MSG_MUSIC_DISPFLAG_ORDER);
        SendMsg(MSG_MUSIC_DISPFLAG_EQ);
        SendMsg(MSG_MUSIC_DISPFLAG_VOL);
        SendMsg(MSG_MUSIC_DISPLAY_UPDATE);
        SendMsg(MSG_BATTERY_UPDATE);
    }

#ifdef THUMB_DEC_INCLUDE
    if (GetMsg(MSG_MUSIC_DISPLAY_BACKGROUND))
    {
        if (!IsCheckedAlbum)
        {
            audio_have_album_pic = 0;
            audio_dec_album_done = 0;

            ModuleOverlay(MODULE_ID_PICTURE_CONTROL, MODULE_OVERLAY_ALL);
            MusicWinCheckAlbum();
        }

        if ((audio_have_album_pic == 1) && (audio_dec_album_done == 1))
        {
            DispPictureWithIDNum(IMG_ID_MUSIC_BOTTOM);
        }
        else if ((audio_have_album_pic == 1) && (audio_dec_album_done == 0))
        {
            if (audio_dec_album_save == 0) // if albumm saved, not used to decoder
            {
                DispPictureWithIDNum(IMG_ID_MUSIC_BACKGROUND);
                FREQ_EnterModule(FREQ_JPG);
            }
        }
        else
        {
            DispPictureWithIDNum(IMG_ID_MUSIC_BACKGROUND);
        }
    }

    if (BUFFER_MAX_NUM >= 1)    // if there is no framebuffer for store album data, skip it
    {
        if ((audio_have_album_pic == 1) && (audio_dec_album_done == 0))
        {
            if (audio_dec_album_save == 0)
            {
                ret = ThumbJpgDecSub();
            }
            else
            {
                ret = 0;    // albumm saved, not used to decoder
            }

            if (ret == 0)
            {
                FREQ_ExitModule(FREQ_JPG);
                FileClose((HANDLE)AudioAlbumHandle);
                AudioAlbumHandle = (FILE*) - 1;
                MemCpy((uint8*)frame_buffer[CurrentFrameIndex], (uint8*)frame_buffer[BUFFER_MAX_NUM - 1], LCD_WIDTH * LCD_HEIGHTA * 2);
#if (FRAME_SUB_BUFFER_NUM > 1)
                MemCpy((uint8*)frame_bufferB[CurrentFrameIndex], (uint8*)frame_bufferB[BUFFER_MAX_NUM - 1], LCD_WIDTH * LCD_HEIGHTB * 2);
#endif
                audio_dec_album_done = 1;
                audio_dec_album_save = 1; //album saved int buffer
                DispPictureWithIDNum(IMG_ID_MUSIC_BOTTOM);
                SendMsg(MSG_MUSIC_DISPFLAG_SPECTRUM);
                SendMsg(MSG_MUSIC_DISPFLAG_REPEATMODE);
                SendMsg(MSG_MUSIC_DISPFLAG_ORDER);
                //SendMsg(MSG_MUSIC_DISPFLAG_EQ);
                SendMsg(MSG_MUSIC_DISPFLAG_VOL);
                SendMsg(MSG_MUSIC_DISPLAY_UPDATE);
            }
        }
    }

#else

    if (GetMsg(MSG_MUSIC_DISPLAY_BACKGROUND))
    {
        DispPictureWithIDNum(IMG_ID_MUSIC_BACKGROUND);
    }

#endif

    if (GetMsg(MSG_MUSIC_DISPLAY_UPDATE))           //refresh icon when switch song
    {
        SendMsg(MSG_MUSIC_DISPFLAG_STATUS);
        SendMsg(MSG_MUSIC_DISPFLAG_AUDIOTYPE);
        SendMsg(MSG_MUSIC_DISPFLAG_BITRATE);
        SendMsg(MSG_MUSIC_DISPFLAG_SCHED);
        SendMsg(MSG_MUSIC_DISPFLAG_CURRENT_TIME);
        SendMsg(MSG_MUSIC_DISPFLAG_TOTAL_TIME);
        SendMsg(MSG_MUSIC_DISPFLAG_FILENUM);
        //SendMsg(MSG_MUSIC_DISPFLAG_SPECTRUM);
        //SendMsg(MUSIC_UPDATESPECTRUM);
        SendMsg(MSG_MUSIC_DISPFLAG_FILENAME);
        SendMsg(MSG_MUSIC_DISPFLAG_LY);
        ClearMsg(MSG_MUSIC_DISPFLAG_SCROLL_FILENAME);

        CurrentTimeSecBk = 0xffffffff;
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_LY))          //LY icon,if interface display type is changing,it need to initial different message,
    {
        if (RETURN_OK == MusicLrcCheck(MusicLongFileName))
        {
            if (!audio_have_album_pic)
            {
                //find music LRC file and no have album picture
                printf("\n -------------MUSIC_DISPLAY_LRC\n");
                MusicLrcInit();
                MusicDispType = MUSIC_DISPLAY_LRC;
                SendMsg(MSG_MUSIC_DISPFLAG_LRC);
            }
            else
            {
                //have album picture
                MusicDispType = MUSIC_DISPLAY_NOR;
            }

            SendMsg(MSG_MUSIC_DISPFLAG_AB);
        }
        else
        {
            //no find music LRC
            MusicDispType = MUSIC_DISPLAY_NOR;
            SendMsg(MSG_MUSIC_DISPFLAG_AB);
            SendMsg(MSG_MUSIC_DISPFLAG_FILENUM);
            SendMsg(MSG_MUSIC_DISPFLAG_CURRENT_TIME);
        }
    }

    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNum(IMG_ID_MUSIC_BATTERY01 + MusicBatteryLevel);
    }

    if ( TRUE == GetMsg(MSG_MUSIC_DISPFLAG_AB))
    {
        if (!audio_have_album_pic)
            DispPictureWithIDNum(IMG_ID_MUSIC_A_B_MODE_BACK + MusicABState);
    }

    if ( TRUE == GetMsg(MSG_MUSIC_DISPFLAG_REPEATMODE))
    {
        if (MusicABState == 0)
        {
            switch (MusicRepeatMode)
            {
                case AUDIO_ONCE:
                case AUDIO_FOLDER_ONCE:
                    DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_LOOP_MODE01, 3, 141);
                    break;

                case AUDIO_REPEAT:
                case AUDIO_REPEAT1:
                    DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_LOOP_MODE01 + 1, 3, 141);
                    break;

                case AUIDO_FOLDER_REPEAT:
                case AUDIO_ALLONCE:
                case AUDIO_ALLREPEAT:
                    DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_LOOP_MODE01 + 2, 3, 141);
                    break;

                default:
                    break;
            }
        }

        //DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_LOOP_MODE01+MusicRepeatMode,3,141);
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_ORDER))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_ORDER_MODE01 + MusicPlayOrder, 45, 141);
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_FILENAME))
    {
        r.x0 = 26;
        r.y0 = 103;
        r.x1 = 124;
        r.y1 = r.y0 + CH_CHAR_XSIZE_12 - 1;

        GetPictureInfoWithIDNum(IMG_ID_MUSIC_NAMEBK, &PicInfo);//get picture struct information
        DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_NAMEBK, r.x0, r.y0 - 2);

        if (LCD_GetStringSize(MusicLongFileName) >= PicInfo.xSize - 3)//'-3' to wordaround some filename last charactor don't display
        {
            MusicWinScrollInit(&r, IMG_ID_MUSIC_NAMEBK, MusicLongFileName, 30);///IMG_ID_MUSIC_ID3MANE
            SendMsg(MSG_MUSIC_DISPFLAG_SCROLL_FILENAME);
        }
        else
        {
            ClearMsg(MSG_MUSIC_DISPFLAG_SCROLL_FILENAME);
        }

        LCD_DispStringInRect(&r, &r, MusicLongFileName, LCD_TEXTALIGN_LEFT);
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_CURRENT_TIME)) // current play time
    {
        CodecGetTime(&CurrentTimeSec);

        if (CurrentTimeSec != MusicCurrentTime)
        {
            CurrentTimeSec = MusicCurrentTime;
        }

        CurrentTimeSec = CurrentTimeSec / 1000;// turn ms to s

        if (CurrentTimeSec != CurrentTimeSecBk)
        {
//            if ( CurrentTimeSec < gAudioPlayTime / 1000 )
//            {
//                //add this judge for some audio format SEEK not accurate or Seek FAIL.
//                {
//                    CurrentTimeSecBk = CurrentTimeSec;
//                    GetTimeHMS(CurrentTimeSec,&i,&j,&k);
//                    MusicDisplayTime(i,j,k,0);
//                }
//            }
//            else
            {
                SendMsg(MSG_MUSIC_DISPFLAG_SCHED);

                if (CurrentTimeSec > MusicTotalTime)
                    CurrentTimeSec = MusicTotalTime;

#ifdef _RK_CUE_
                if (AudioFileInfo.FindData.IsCue == 1)
                {
                    if (CurrentTimeSec < (AudioFileInfo.FindData.CueStartTime / 1000))
                    {
                        CurrentTimeSec = AudioFileInfo.FindData.CueStartTime / 1000;
                    }

                    CurrentTimeSec -= (AudioFileInfo.FindData.CueStartTime / 1000);
                }
#endif
                CurrentTimeSecBk = CurrentTimeSec;
                GetTimeHMS(CurrentTimeSec, &i, &j, &k);
                MusicDisplayTime(i, j, k, 0);
            }
        }
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_FILENUM))
    {
        if (!audio_have_album_pic)
            DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_NUMBER, 0, 2);

        MusicDisplayFileNums(MusicCurrFileNum, MusicTotalFileNum);
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_AUDIOTYPE))
    {
        //DispPictureWithIDNum(IMG_ID_MUSIC_FILETYPE_MP3 + CurrentCodecType);
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_STATUS))
    {
        if (AUDIO_STATE_PLAY == MusicPlayState)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_PLAY_STATE, 0, 0);
        }
        else
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_PAUSE_STATE, 0, 0);
        }
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_EQ))
    {
        //TextDrawMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_CURRTIME_BACK, 65, 125);
        {
            eEQMode eqMode;
            eqMode = MusicEQType;
            DEBUG(" === MusicEQType = %d === ", MusicEQType);

            switch (eqMode)
            {
                case EQ_BASS:
                    DisplayMenuStrWithIDNum(66, 125, 60, 125 + CH_CHAR_YSIZE_12,
                                            LCD_TEXTALIGN_CENTER,
                                            SID_BASS);
                    break;

                case EQ_HEAVY:
                    DisplayMenuStrWithIDNum(66, 125, 60, 125 + CH_CHAR_YSIZE_12,
                                            LCD_TEXTALIGN_CENTER,
                                            SID_EQ_HEAVY);
                    break;

                case EQ_POP:
                    DisplayMenuStrWithIDNum(66, 125, 60, 125 + CH_CHAR_YSIZE_12,
                                            LCD_TEXTALIGN_CENTER,
                                            SID_EQ_POP);
                    break;

                case EQ_JAZZ:
                    DisplayMenuStrWithIDNum(66, 125, 60, 125 + CH_CHAR_YSIZE_12,
                                            LCD_TEXTALIGN_CENTER,
                                            SID_EQ_JAZZ);
                    break;

                case EQ_UNIQUE:
                    DisplayMenuStrWithIDNum(66, 125, 60, 125 + CH_CHAR_YSIZE_12,
                                            LCD_TEXTALIGN_CENTER,
                                            SID_EQ_UNIQUE);
                    break;

                case EQ_USER:
                    DisplayMenuStrWithIDNum(66, 125, 60, 125 + CH_CHAR_YSIZE_12,
                                            LCD_TEXTALIGN_CENTER,
                                            SID_EQ_CUSTOM);
                    break;

                case EQ_NOR:
                    DisplayMenuStrWithIDNum(66, 125, 60, 125 + CH_CHAR_YSIZE_12,
                                            LCD_TEXTALIGN_CENTER,
                                            SID_EQ_NONE);
                    break;

                default:
                    break;
            }
        }
        //LCD_SetTextMode(TextDrawMode);
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_BITRATE))
    {
        MusicDisplayBitrate((UINT32)MusicBitrate);
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_VOL))
    {
        VolTemp = (UINT16)((UINT32)(40) * (MusicWinVol) / (32));

        for (i = 0; i < VolTemp; i++)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_FM_VOLUME_GUAGE, 70 + i, 147);
        }

        for (; i < 40; i++)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_FM_VOLUME_GUAGE_BK, 70 + i, 147);
        }

        DisplayBuf[0] = (MusicWinVol - 0) / 10 + '0';
        DisplayBuf[1] = (MusicWinVol - 0) % 10 + '0';
        DisplayBuf[2] = 0;
        DispPictureWithIDNumAndXYoffset(IMG_ID_FM_VOLUME_BK, 0, 0);

        //i=LCD_GetColor();
        //LCD_SetColor(COLOR_BLACK);
        LCD_NFDispStringAt(113, 143, DisplayBuf);
        //LCD_SetColor(i);
    }

    if (!audio_have_album_pic)
    {
        if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_SCHED)) //scroll bar
        {
#ifdef _RK_CUE_

            if (AudioFileInfo.FindData.IsCue == 1)
            {
                if (MusicCurrentTime < AudioFileInfo.FindData.CueStartTime)
                {
                    MusicCurrentTime = AudioFileInfo.FindData.CueStartTime;
                }

                MusicTimeGuageVal = (MusicCurrentTime - AudioFileInfo.FindData.CueStartTime) * (MUSIC_TIME_GUAGE_CONST - 1) / (AudioFileInfo.FindData.CueEndTime - AudioFileInfo.FindData.CueStartTime);
            }
            else
#endif
            {
                MusicTimeGuageVal = (UINT32)(((UINT64)MusicCurrentTime) * (MUSIC_TIME_GUAGE_CONST - 1) / MusicTotalTime);
            }

            if (MusicTimeGuageVal <= MUSIC_TIME_GUAGE_CONST)
            {
                for (i = 0; i < MusicTimeGuageVal; i++)
                {
                    DisplayPictureFromBuffer(MusicSchedPicInfo.x + 7 + i,
                                             MusicSchedPicInfo.y + 78,
                                             MusicSchedPicInfo.xSize,
                                             MusicSchedPicInfo.ySize,
                                             (UINT16 *)MusicSchedBuffer);
                }

                for (; i < MUSIC_TIME_GUAGE_CONST; i++)
                {
                    DisplayPictureFromBuffer(MusicSchedPicInfo.x + 7 + i,
                                             MusicSchedPicInfo.y + 78,
                                             MusicSchedPicInfo.xSize,
                                             MusicSchedPicInfo.ySize,
                                             (UINT16 *)(&MusicSchedBuffer[0] + MUSIC_SCHED_BUF_SIZE));
                }
            }
            else//ylz++
            {
                for (i = 0; i < MUSIC_TIME_GUAGE_CONST; i++)
                {
                    DisplayPictureFromBuffer(MusicSchedPicInfo.x + 7 + i,
                                             MusicSchedPicInfo.y + 78,
                                             MusicSchedPicInfo.xSize,
                                             MusicSchedPicInfo.ySize,
                                             (UINT16 *)MusicSchedBuffer);
                }
            }
        }
    }
    else //have ablum cover picture
    {
        uint16 time_guage_const = 128 - 4;
        MusicTimeGuageVal = (UINT32)(((UINT64)MusicCurrentTime) * (time_guage_const - 1) / MusicTotalTime);

        if (MusicTimeGuageVal <= time_guage_const)
        {
            for (i = 0; i < MusicTimeGuageVal; i++)
            {
                DisplayPictureFromBuffer(MusicSchedPicInfo.x + 2 + i,
                                         MusicSchedPicInfo.y + 85,
                                         MusicSchedPicInfo.xSize,
                                         MusicSchedPicInfo.ySize,
                                         (UINT16 *)MusicSchedBuffer);
            }

            for (; i < time_guage_const; i++)
            {
                DisplayPictureFromBuffer(MusicSchedPicInfo.x + 2 + i,
                                         MusicSchedPicInfo.y + 85,
                                         MusicSchedPicInfo.xSize,
                                         MusicSchedPicInfo.ySize,
                                         (UINT16 *)(&MusicSchedBuffer[0] + MUSIC_SCHED_BUF_SIZE));
            }
        }
        else//ylz++
        {
            for (i = 0; i < MUSIC_TIME_GUAGE_CONST; i++)
            {
                DisplayPictureFromBuffer(MusicSchedPicInfo.x + 2 + i,
                                         MusicSchedPicInfo.y + 85,
                                         MusicSchedPicInfo.xSize,
                                         MusicSchedPicInfo.ySize,
                                         (UINT16 *)MusicSchedBuffer);
            }
        }
    }

    if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_TOTAL_TIME))
    {
#ifdef _RK_CUE_

        if (AudioFileInfo.FindData.IsCue == 1)
        {
            CurrentTimeSec = AudioFileInfo.FindData.CueEndTime - AudioFileInfo.FindData.CueStartTime;
        }
        else
#endif
        {
            CurrentTimeSec = MusicTotalTime;
        }

        CurrentTimeSec = CurrentTimeSec / 1000;//turn ms to s,
        GetTimeHMS(CurrentTimeSec, &i, &j, &k);
        MusicDisplayTime(i, j, k, 1);
    }

    if (!audio_have_album_pic)
    {
        if (MusicDispType == MUSIC_DISPLAY_LRC)
        {
            MusicLrcPlay(0);

            if (TRUE == GetMsg(MSG_MUSIC_DISPFLAG_LRC))
            {
                r.x0 = 23;
                r.y0 = 27;
                r.x1 = 108;
                r.y1 = r.y0 + 35;
                DisplayPicture_part(IMG_ID_MUSIC_BACKGROUND1, 0, 0, 20, 50);
                LCD_DispStringInRect(&r, &r, UnicodeLrcBuffer, LCD_TEXTALIGN_CENTER);
            }
        }
        else
        {
            if (TRUE == GetMsg(MUSIC_UPDATESPECTRUM))
            {
#ifdef _RK_SPECTRUM_
                CodecGetTime(&CurrentTimeSec);
                CurrentTimeSec = CurrentTimeSec / 200;    //refresh spectrum every 200ms

                if (CurrentTimeSec != CurrentTime200MsBk)
                {
                    CurrentTime200MsBk = CurrentTimeSec;
                    DisplayPicture_part(IMG_ID_MUSIC_BACKGROUND, 0, 0, 26, 38);
                    GetPictureInfoWithIDNum(IMG_ID_MUSIC_SPECTRUM20, &PicInfo);

                    //do insert value in here
                    for (i = 0; i < 10 ; i++)
                    {
                        SpectrumOutTemp[i] = SpectrumOut[i];
                    }

                    if (2 > SpectrumOutTemp[9] )
                    {
                        SpectrumOutTemp[10] = SpectrumOutTemp[9] ;
                        SpectrumOutTemp[11] = SpectrumOutTemp[9] ;
                    }
                    else
                    {
                        SpectrumOutTemp[10] = SpectrumOutTemp[9] - 1;
                        SpectrumOutTemp[11] = SpectrumOutTemp[9] - 2;
                    }

                    for (i = 0; i < 12; i++)
                    {
                        y1 = (SpectrumOutTemp[i]);// real pectrum height.

                        if (y1 > 35)
                        {
                            y1 = 35;
                        }

                        y = 35 - y1;
                        DisplayPicture_part(IMG_ID_MUSIC_SPECTRUM20, PicInfo.x + i * 7, PicInfo.y, y, y1);
                    }
                }

#endif
            }
        }
    }

#ifdef _BLUETOOTH_
    /*add for showing BT icon*/
    if (gbBTConnected && (gbBTSwitchOn == BT_ON) && IsBTOpened)
    {
        //DEBUG("###### SHOW BT ICON #####");
        DispPictureWithIDNumAndXYoffset(IMG_ID_BLUETOOTH_ICONSMALL, 92, 2);
    }
#endif

    if (TRUE == CheckMsg(MSG_MUSIC_DISPFLAG_SCROLL_FILENAME))
    {
        ScrollStringCommon(MusicLongFileName);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MusicWinKeyProc(void)
  Description   : audio key handle program.

  Input         : NULL
  Return        : TRUE\FALSE

  History:     <author>         <time>         <version>
             zhangshuai     2009/02/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_SERVICE_CODE_
UINT32 MusicWinKeyProc(void)
{
    UINT32 MusicKeyVal;
    UINT32 i;
    UINT32 RetVal = RETURN_OK;
    TASK_ARG TaskArg;
    FS_TYPE fsType;
    UINT8   *fileEx;
    PICTURE_INFO_STRUCT     PicInfo;
    LCD_RECT                r;
    MusicKeyVal =  GetKeyVal();

    //5 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (MusicKeyVal)
        {
            case KEY_VAL_ESC_PRESS_START:           //ong press switch VOL function
                if (CheckMsg(MSG_KEY_VOL_STATE))
                {
                    ClearVolMsg();
                    SendMsg(MSG_MUSIC_DISPFLAG_VOL);
                }
                else
                {
                    KeyVolAdjStart();
                    SendMsg(MSG_KEY_VOL_SHOWSTATE);
                    SendMsg(MSG_MUSIC_DISPFLAG_VOL);
                }

                break;

            case KEY_VAL_FFD_DOWN:
            case KEY_VAL_FFD_PRESS:
                if (CheckMsg(MSG_KEY_VOL_STATE))
                {
                    MusicKeyVal = KEY_VAL_UP_DOWN;
                }

                break;

            case KEY_VAL_FFW_DOWN:
            case KEY_VAL_FFW_PRESS:
                if (CheckMsg(MSG_KEY_VOL_STATE))
                {
                    MusicKeyVal = KEY_VAL_DOWN_DOWN;
                }

                break;

            case KEY_VAL_FFD_SHORT_UP:
            case KEY_VAL_FFD_LONG_UP:
            case KEY_VAL_FFW_SHORT_UP:
            case KEY_VAL_FFW_LONG_UP:
                if (CheckMsg(MSG_KEY_VOL_STATE))
                {
                    MusicKeyVal = KEY_VAL_NONE;
                }

                break;

            default:
                break;
        }
    }

    //6 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
        switch (MusicKeyVal)
        {
            case KEY_VAL_UP_PRESS_START:            //long press A-B function
                MusicKeyVal = KEY_VAL_ESC_SHORT_UP;
                break;

            case KEY_VAL_UP_PRESS:                  //long press function cancel
            case KEY_VAL_DOWN_PRESS:
                MusicKeyVal = KEY_VAL_NONE;
                break;

            default:
                break;
        }
    }

    //7 key function
    switch (MusicKeyVal)
    {
        case KEY_VAL_ESC_LONG_UP:                                 // set AB repeat.
            if (AUDIO_STATE_PLAY == MusicPlayState)
            {
                if (AUDIO_AB_NULL == AudioPlayInfo.ABRequire)
                {
                    AudioDecodeProc(MSG_AUDIO_ABSETA, NULL);
                    //   if(MUSIC_DISPLAY_NOR == MusicDispType)
                    {
                        SendMsg(MSG_MUSIC_DISPFLAG_AB);
                    }
                }
                else if (AUDIO_AB_A == AudioPlayInfo.ABRequire)
                {
                    AudioDecodeProc(MSG_AUDIO_ABPLAY, NULL);
                    // if(MUSIC_DISPLAY_NOR == MusicDispType)
                    {
                        SendMsg(MSG_MUSIC_DISPFLAG_AB);
                    }
                }
                else
                {
                    AudioDecodeProc(MSG_AUDIO_ABSTOP, NULL);
                    // if(MUSIC_DISPLAY_NOR == MusicDispType)
                    {
                        SendMsg(MSG_MUSIC_DISPFLAG_AB);
                    }
                }
            }

            break;

        case KEY_VAL_ESC_SHORT_UP:                             //enter brower.
            if (CheckMsg(MSG_MUSIC_NEXTFILE))
            {
                if (GetMsg(MSG_MUSIC_NEXTFILE_OK))
                {
                    ClearMsg(MSG_MUSIC_NEXTFILE)  ;
                    ClearMsg(MSG_MUSIC_NEXTFILE_OK);
                }
                else
                {
                    break;
                }
            }

            if (GetMsg(MSG_MEDIA_NOW_PLAY))
            {
                TaskArg.Medialib.CurId = 0;
                TaskSwitch(TASK_ID_MEDIALIB, &TaskArg);
                RetVal = 1;
                break;
            }

            if (TRUE == CheckMsg(MSG_MEDIA_BREAKPOINT_PLAY))
            {
                BroswerFlag = FALSE;
                TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                RetVal = 1;
            }
            else if ((AudioFileInfo.ucSelPlayType !=  SORT_TYPE_SEL_BROWSER)
                     && (AudioFileInfo.ucSelPlayType !=  SORT_TYPE_SEL_FOLDER))
            {
                gMusicTypeSelID = AudioFileInfo.ucSelPlayType;

                if (SelFromNowPlay)
                {
                    TaskArg.Medialib.CurId = 0;
                    TaskSwitch(TASK_ID_MEDIALIB, &TaskArg);
                }
                else
                {
                    TaskArg.MediaBro.CurId = 1;
                    TaskArg.MediaBro.TitleAdd = MediaTitleAdd;
                    memcpy((UINT8*) & (TaskArg.MediaBro.MediaDirTreeInfo),
                           (UINT8*) & (gSysConfig.MediaDirTreeInfo),
                           sizeof(SYS_MEDIABRO_DIR_TREE_STRUCT));
                    TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
                }

                RetVal = 1;

            }
            else
            {
                if (pAudioRegKey->RepeatMode >= AUDIO_ALLONCE)
                {
                    TaskArg.Browser.FileNum  = (AudioPlayFileNumSave + AudioFileInfo.CurrentFileNum);
                }
                else
                {
                    TaskArg.Browser.FileNum  = AudioFileInfo.CurrentFileNum;
                }

                if (AudioFileInfo.ucSelPlayType ==  SORT_TYPE_SEL_FOLDER)
                {
                    TaskArg.Browser.FileType = FileTypeAudio;
                    TaskArg.Browser.FromWhere = 1;
                    fsType = MUSIC_DB;
                    fileEx = (UINT8*)AudioFileExtString;
                    if ((AUDIO_FOLDER_ONCE == pAudioRegKey->RepeatMode)
                    || (AUIDO_FOLDER_REPEAT == pAudioRegKey->RepeatMode)
                    || (AUDIO_REPEAT == pAudioRegKey->RepeatMode))
                    {
                        TaskArg.Browser.FileNum = GetGlobeFileNum(AudioFileInfo.CurrentFileNum,
                                                  AudioFileInfo.FindData.Clus,
                                                  fileEx,
                                                  fsType);
                    }
                }
                else
                {
#ifdef _SPINOR_
                    //DEBUG();
                    if (BroswerFlag) //means from spi music app.
                    {
                        //DEBUG();
                        TaskArg.Browser.FileType = FileTypeAudio;
                        TaskArg.Browser.FromWhere = 0;
                        fsType = FS_FAT;
                        fileEx = (UINT8*)AudioFileExtString;
                    }
                    else
                    {
                        TaskArg.Browser.FileType = FileTypeALL;
                        TaskArg.Browser.FromWhere = 0;
                        fsType = FS_FAT;
                        fileEx = (UINT8*)ALLFileExtString;
                    }

                    if ((AUDIO_FOLDER_ONCE == pAudioRegKey->RepeatMode)
                    || (AUIDO_FOLDER_REPEAT == pAudioRegKey->RepeatMode)
                    || (AUDIO_REPEAT == pAudioRegKey->RepeatMode))
                    {
                        TaskArg.Browser.FileNum = GetGlobeFileNum(AudioFileInfo.CurrentFileNum,
                                                  AudioFileInfo.FindData.Clus,
                                                  fileEx,
                                                  fsType);
                    }
                    #else
                    //DEBUG();
                    if (gSysConfig.FindFileType == FileTypeAudio)
                    {
                        TaskArg.Browser.FileType = FileTypeAudio;
                        TaskArg.Browser.FromWhere = 0;
                        fsType = FS_FAT;
                        fileEx = (UINT8*)MusicFileExtString;
                        TaskArg.Browser.FileNum = GetGlobeFileNum(AudioFileInfo.CurrentFileNum,
                                              AudioFileInfo.FindData.Clus,
                                              fileEx,
                                              fsType);
                    }
                    else
                    {
                        //��ʾ�������ʾ�����ļ�����;
                        TaskArg.Browser.FileType = FileTypeALL;
                        TaskArg.Browser.FromWhere = 0;
                        fsType = FS_FAT;
                        //��ʾ�����ļ��������ļ����Ͳ���
                        fileEx = (UINT8*)ALLFileExtString;
                        TaskArg.Browser.FileNum = GlobalFilenum;
                    }
                    #endif
                }

                TaskSwitch(TASK_ID_BROWSER, &TaskArg);
                RetVal = 1;
            }

            break;

        case KEY_VAL_ESC_PRESS_START:                          //�˳����ֽ���
            BroswerFlag = FALSE;
            TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            RetVal = 1;
            break;

        case KEY_VAL_MENU_SHORT_UP:                             // play pause
            if (AUDIO_STATE_PLAY == MusicPlayState)
            {
                MusicDecodeProc(MSG_AUDIO_PAUSE, NULL);
            }
            else if (AUDIO_STATE_PAUSE == MusicPlayState)
            {
                MusicDecodeProc(MSG_AUDIO_RESUME, NULL);
            }

            SendMsg(MSG_MUSIC_DISPFLAG_STATUS);
            break;

        case KEY_VAL_UP_DOWN:                                   //volume increse
        case KEY_VAL_UP_PRESS :
            if (MusicWinVol < MAX_VOLUME)
            {
                MusicWinVol++;
                MusicDecodeProc(MSG_AUDIO_VOLUMESET, NULL);
            }

            SendMsg(MSG_MUSIC_DISPFLAG_VOL);
            break;

        case KEY_VAL_DOWN_DOWN:                                 //volume reduce
        case KEY_VAL_DOWN_PRESS:
            if (MusicWinVol)
            {
                MusicWinVol--;
                MusicDecodeProc(MSG_AUDIO_VOLUMESET, NULL);
            }

            SendMsg(MSG_MUSIC_DISPFLAG_VOL);
            break;

        case KEY_VAL_FFD_PRESS_START:
            if (AUDIO_STATE_PAUSE == MusicPlayState)
            {
                MusicPlayStateFF = AUDIO_STATE_PAUSE_FFD;
            }
            else if (AUDIO_STATE_PLAY == MusicPlayState)
            {
                MusicPlayStateFF = AUDIO_STATE_PLAY_FFD;

                SendMsg(MSG_MUSIC_DISPFLAG_SCHED);

                SendMsg(MSG_MUSIC_DISPFLAG_FILENAME);
                SendMsg(MSG_MUSIC_DISPFLAG_TOTAL_TIME);
            }

            SendMsg(MSG_MUSIC_DISPFLAG_STATUS_FF);
            break;

        case KEY_VAL_FFD_PRESS:                                 //���
            if (MusicPlayStateFF == AUDIO_STATE_PAUSE_FFD)
            {
                AudioErrorFileCount = 0;//sen #20090803#1 all audio file is not support
                AudioStopMode = AUDIO_STOP_FORCE;
                MusicNextFile = 1;
                MusicTimeGuageVal = 0;
                AudioStop(AUDIO_STOP_FORCE);
                AudioGetNextMusic((UINT32)MusicNextFile);
                SendMsg(MSG_AUDIO_NEXTFILE);
                SendMsg(MSG_MUSIC_DISPFLAG_STATUS_FF);
                SendMsg(MSG_MUSIC_DISPFLAG_FILENUM);
            }
            else
            {
                MusicDecodeProc(MSG_AUDIO_FFD, (void*)AUDIO_STEPLEN);
                SendMsg(MSG_MUSIC_DISPFLAG_SCHED);

                if (MusicCurrentTime >= MusicTotalTime)
                {
                    if ((MusicRepeatMode != AUDIO_REPEAT) && (MusicRepeatMode != AUDIO_REPEAT1))
                    {
                        AudioErrorFileCount = 0;//sen #20090803#1 all audio file is not support
                        AudioStopMode = AUDIO_STOP_FORCE;
                        MusicNextFile = 1;
                        MusicTimeGuageVal = 0;
                        AudioStop(AudioStopMode);
                        AudioGetNextMusic((UINT32)MusicNextFile);
                        SendMsg(MSG_AUDIO_NEXTFILE);
                    }
                    else
                    {
                        AudioStop(AUDIO_STOP_FORCE);
                        MusicNextFile = 1;
                        AudioGetNextMusic((UINT32)MusicNextFile);
                        SendMsg(MSG_AUDIO_NEXTFILE);
                        MusicStepTime = 0;
                    }

                    pAudioRegKey->CurrentTime = 0;

                    MusicDecodeProc(MSG_AUDIO_FFD, (void*)AUDIO_STEPLEN);

                    SendMsg(MSG_MUSIC_DISPFLAG_SCHED);
                    SendMsg(MSG_MUSIC_DISPFLAG_FILENAME);
                    SendMsg(MSG_MUSIC_DISPFLAG_TOTAL_TIME);
                }
            }

            SendMsg(MSG_MUSIC_DISPFLAG_CURRENT_TIME);

            if (AudioPlayInfo.ABRequire != AUDIO_AB_NULL)
            {
                AudioPlayInfo.ABRequire = AUDIO_AB_NULL;
                AudioDecodeProc(MSG_AUDIO_ABSTOP, NULL);
                SendMsg(MSG_MUSIC_DISPFLAG_AB);
            }

            break;

        case KEY_VAL_FFW_PRESS_START:

            //LCD_ClrRect(44, 27, 128, 36);
            if (AUDIO_STATE_PAUSE == MusicPlayState)
            {
                MusicPlayStateFF = AUDIO_STATE_PAUSE_FFW;
            }
            else if (AUDIO_STATE_PLAY == MusicPlayState)
            {
                MusicPlayStateFF = AUDIO_STATE_PLAY_FFW;
                SendMsg(MSG_MUSIC_DISPFLAG_FILENAME);
                SendMsg(MSG_MUSIC_DISPFLAG_TOTAL_TIME);
            }

            SendMsg(MSG_MUSIC_DISPFLAG_STATUS_FF);
            break;

        case KEY_VAL_FFW_PRESS:                                  //����

            if (MusicPlayStateFF == AUDIO_STATE_PAUSE_FFW)
            {
                AudioErrorFileCount = 0;//sen #20090803#1 all audio file is not support
                AudioStopMode = AUDIO_STOP_FORCE;
                MusicNextFile = -1;
                MusicTimeGuageVal = 0;
                AudioStop(AUDIO_STOP_FORCE);
                AudioGetNextMusic((UINT32)MusicNextFile);
                SendMsg(MSG_AUDIO_NEXTFILE);

                SendMsg(MSG_MUSIC_DISPFLAG_STATUS_FF);
                SendMsg(MSG_MUSIC_DISPFLAG_FILENUM);
            }
            else
            {

                MusicDecodeProc(MSG_AUDIO_FFW, (void*)AUDIO_STEPLEN);
                SendMsg(MSG_MUSIC_DISPFLAG_SCHED);

                if (MusicCurrentTime == 0)
                {
                    if ((MusicRepeatMode != AUDIO_REPEAT) && (MusicRepeatMode != AUDIO_REPEAT1))
                    {
                        AudioErrorFileCount = 0;//sen #20090803#1 all audio file is not support
                        AudioStopMode = AUDIO_STOP_FORCE;
                        MusicNextFile = -1;
                        MusicTimeGuageVal = 0;
                        AudioStop(AUDIO_STOP_FORCE);
                        AudioGetNextMusic((UINT32)MusicNextFile);
                        SendMsg(MSG_AUDIO_NEXTFILE);
                    }

                    pAudioRegKey->CurrentTime = pAudioRegKey->TotalTime;
                    MusicDecodeProc(MSG_AUDIO_FFW, (void*)AUDIO_STEPLEN);
                    SendMsg(MSG_MUSIC_DISPFLAG_SCHED);
                    SendMsg(MSG_MUSIC_DISPFLAG_FILENAME);
                    SendMsg(MSG_MUSIC_DISPFLAG_TOTAL_TIME);
                }
            }

            SendMsg(MSG_MUSIC_DISPFLAG_CURRENT_TIME);

            if (AudioPlayInfo.ABRequire != AUDIO_AB_NULL)
            {
                AudioPlayInfo.ABRequire = AUDIO_AB_NULL;
                AudioDecodeProc(MSG_AUDIO_ABSTOP, NULL);
                SendMsg(MSG_MUSIC_DISPFLAG_AB);
            }

            break;

        case KEY_VAL_FFD_LONG_UP:                              //Resume
            if ((AUDIO_STATE_PAUSE != MusicPlayState))
            {
                AudioFFDStop();
                MusicDecodeProc(MSG_AUDIO_RESUME, NULL);

                for (i = 0; i < 200; i++)
                {
                    BBDebug();
                    CodecGetTime(&pAudioRegKey->CurrentTime);

                    if (pAudioRegKey->CurrentTime > MusicTotalTime)
                    {
                        gAudioPlayTime            = MusicTotalTime;
                        pAudioRegKey->CurrentTime = MusicTotalTime;
                    }

                    if (pAudioRegKey->CurrentTime >= gAudioPlayTime)
                    {
                        gAudioPlayTime = pAudioRegKey->CurrentTime;
                        break;
                    }
                    else    //currenttime < musicStepTime
                    {
                        if (gAudioPlayTime - pAudioRegKey->CurrentTime < 500
                            &&  (MusicTotalTime - pAudioRegKey->CurrentTime) < 500)
                        {
                            pAudioRegKey->CurrentTime = gAudioPlayTime;
                            break;
                        }
                    }

                    DelayMs(1);
                }
            }

            SendMsg(MSG_MUSIC_DISPFLAG_REPEATMODE);
            SendMsg(MSG_MUSIC_DISPFLAG_ORDER);
            SendMsg(MSG_MUSIC_DISPFLAG_EQ);
            SendMsg(MSG_BATTERY_UPDATE);
            SendMsg(MSG_MUSIC_DISPFLAG_STATUS);
            MusicPlayStateFF = 0;
            break;

        case KEY_VAL_FFW_LONG_UP:
            if ((AUDIO_STATE_PAUSE != MusicPlayState))
            {
                AudioFFWStop();
                MusicDecodeProc(MSG_AUDIO_RESUME, NULL);

                for (i = 0; i < 200; i++)
                {
                    BBDebug();
                    CodecGetTime(&pAudioRegKey->CurrentTime);

                    if (pAudioRegKey->CurrentTime < gAudioPlayTime)
                    {
                        // DEBUG();
                        //MusicStepTime = pAudioRegKey->CurrentTime;
                        break;
                    }

                    DelayMs(1);
                }
            }

            SendMsg(MSG_MUSIC_DISPFLAG_REPEATMODE);
            SendMsg(MSG_MUSIC_DISPFLAG_ORDER);
            SendMsg(MSG_MUSIC_DISPFLAG_EQ);
            SendMsg(MSG_BATTERY_UPDATE);
            SendMsg(MSG_MUSIC_DISPFLAG_STATUS);
            MusicPlayStateFF = 0;
            break;

        case KEY_VAL_FFD_SHORT_UP://next file
            GetPictureInfoWithIDNum(IMG_ID_MUSIC_NAMEBK, &PicInfo);//get picture struct information
            DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_NAMEBK, 26, 101);
            LCD_ClrRect(0, 0, 128, 104);
            AudioErrorFileCount = 0;//sen #20090803#1 all audio file is not support
            AudioStopMode = AUDIO_STOP_FORCE;
            MusicNextFile = 1;
            MusicTimeGuageVal = 0;
            MusicDecodeProc(MSG_AUDIO_NEXTFILE, (void*)MusicNextFile);
            ClearMsg(MUSIC_UPDATESPECTRUM);
            ClearMsg(MSG_MUSIC_DISPFLAG_SPECTRUM);
            break;

        case KEY_VAL_FFW_SHORT_UP://preious file
#ifdef _RK_CUE_
            if ((MusicCurrentTime < 3000 && AudioFileInfo.FindData.IsCue == 0)
                || ((MusicCurrentTime - AudioFileInfo.FindData.CueStartTime) < 3000 && AudioFileInfo.FindData.IsCue == 1))
#else
            if (MusicCurrentTime < 3000)  // 3000 ms
#endif
            {
                //-> Depend on UI
                GetPictureInfoWithIDNum(IMG_ID_MUSIC_NAMEBK, &PicInfo);//get picture struct information
                DispPictureWithIDNumAndXYoffset(IMG_ID_MUSIC_NAMEBK, 26, 101);
                LCD_ClrRect(0, 0, 128, 104);
                //<- Depend on UI
                AudioErrorFileCount = 0;//sen #20090803#1 all audio file is not support
                AudioStopMode = AUDIO_STOP_FORCE;
                MusicNextFile = -1;
                MusicTimeGuageVal = 0;
                MusicDecodeProc(MSG_AUDIO_NEXTFILE, (void*)MusicNextFile);
                ClearMsg(MUSIC_UPDATESPECTRUM);
                ClearMsg(MSG_MUSIC_DISPFLAG_SPECTRUM);
            }
            else
            {
                uint16 MusicPlayStateBk = MusicPlayState;
                MusicDecodeProc(MSG_AUDIO_FFW, (void*)AUDIO_STEPLEN);
#ifdef _RK_CUE_
                if (AudioFileInfo.FindData.IsCue == 1)
                {
                    pAudioRegKey->CurrentTime = AudioFileInfo.FindData.CueStartTime /*+ 1500*/; //tiantian
                }
                else
#endif
                {
                    pAudioRegKey->CurrentTime = 0;
                }

                gAudioPlayTime = 0;

                AudioFFWStop();

                if ((AUDIO_STATE_PAUSE != MusicPlayStateBk))
                {
                    MusicDecodeProc(MSG_AUDIO_RESUME, NULL);
                }
                else    //audio state pause,chad.ma add 20151112
                {
                    CurrentTimeSecBk = 0xffffffff;
                }

                MusicPlayState = MusicPlayStateBk;
                SendMsg(MSG_MUSIC_DISPFLAG_SCHED);
            }

            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&MusicWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&MusicWin, &HoldWin, &TaskArg);
            break;

        default:
            RetVal = FALSE;
            break;
    }

    return RetVal;
}


/*
********************************************************************************
*
*                         End of MusicWin.c
*
********************************************************************************
*/
#endif

