/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name: BrowserUI.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             chenfen          2008-3-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_BROSWER_UI_

#include "SysInclude.h"

#ifdef  _BROWSER_

#include "fsinclude.h"

#include "BrowserUI.h"
#include "BroCore.h"

#include "DialogBox.h"
#include "MessageBox.h"
#include "Hold.h"

#ifdef _MUSIC_
#include "audio_globals.h"
#include "audio_file_access.h"
#include "effect.h"
#include "AudioControl.h"
#endif

#include "SysFindFile.h"

#include "mainmenu.h"
#include "FMControl.h"

#include "MediaBroWin.h"
#include "medialibwin.h"

#ifdef _FRAME_BUFFER_
#include "LcdInterface.h"
#endif

#include "image_main.h"
#include "ImageControl.h"
#ifdef THUMB_DEC_INCLUDE
#include "thumbnail_parse.h"
#endif

#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#endif

int16 BroGetFileNumInFloder(BrowserFileStruct *pBro, UINT16 Flag);

/*
--------------------------------------------------------------------------------
  Function name : void BrowserWinInit(void)
  Author        : chenfen
  Description   : browser window initialization func

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef _FRAME_BUFFER_
static int gBrowserData_CurStartIndex = 0;
#endif

#ifdef THUMB_DEC_INCLUDE
static BOOL gIsImproveFreq;
static UINT32 gTickCounter;
#endif

_ATTR_BRO_UI_CODE_
void BrowserWinInit(void *pArg)
{
#ifdef THUMB_DEC_INCLUDE
    ModuleOverlay(MODULE_ID_PICTURE_CONTROL, MODULE_OVERLAY_ALL);
    gIsImproveFreq = FALSE;
    gTickCounter = SysTickCounter;
#endif
    gBrowserFindFileType = ((BROWSER_WIN_ARG*)pArg)->FileType;           //task parameter transfer
    gBrowserFildFileNum  = ((BROWSER_WIN_ARG*)pArg)->FileNum;            //file index

    if ((gBrowserFindFileType == FileTypeALL))//to enter main interface directly.
    {
        SendMsg(MSG_BROW_FROM_MAINMENU);
    }
    else
    {
        ClearMsg(MSG_BROW_FROM_MAINMENU);
    }

    if (((BROWSER_WIN_ARG*)pArg)->FromWhere)
    {
        if (MemorySelect == FLASH0)
        {
            gBrowSerFsType = MUSIC_DB;
        }
        else
        {
            gBrowSerFsType = FS_FAT; //FS_FAT
        }
    }
    else
    {
        gBrowSerFsType = FS_FAT;
    }

    if (gBrowserFildFileNum) //locate file basic index is 1
    {
        BrowserValueInit(1);
        BroGotoCurFile(gBrowserFildFileNum);//file that focus index is gBrowserFildFileNum.
    }
    else
    {
        ClearMsg(MSG_BROW_TASK_HAS_SWITCH);
        BrowserValueInit(1);
    }

    FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep],
                     gBrowSerFsType);
    FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile,
                      &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep], gBrowSerFsType);

    BrowserMemSel         = MemorySelect;
#if 0
    if (gBrowserFindFileType == FileTypeALL) //audio file,disk selet don't switch
    {
        BrowserMemSel         = FLASH0;//MemorySelect;
    }
    else
    {
        BrowserMemSel         = MemorySelect;
    }
#endif
    BrowserMemSelEn       = 0;
    BrowserData.TotleDisk = 1;

#if(defined (_SDCARD_) || defined (_MULT_DISK_)|| defined (_USB_HOST_))
    if (BroswerFlag == TRUE)//it do not enter browser from media.
    {
        if (gBrowserFildFileNum == 0)
        {
            BrowserMemSelEn = 1;
            BroLoadMemSelStr();
        }
    }
    else
    {
        BroswerFlag = TRUE;
    }

#endif

    if ((BrowserMemSelEn != 1) && (BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems == 0))
    {
        SendMsg(MSG_BROW_CHECK_FILE);
    }

    SendMsg(MSG_BROW_DIS_ALL);
    SendMsg(MSG_BROW_DIS_SCLLO_ITEM);//send scroll message
    KeyReset();             //key initial
}

/*
--------------------------------------------------------------------------------
  Function name : void BrowserWinInit(void)
  Author        : chenfen
  Description   : browser window auti-initial function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BrowserWinDeInit(void)
{
#ifdef THUMB_DEC_INCLUDE

    if (gIsImproveFreq)
    {
        gIsImproveFreq = FALSE;
        FREQ_ExitModule(FREQ_JPG);
    }

#endif

    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BrowserWinService(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
uint32 BrowserWinService(void)
{
    UINT32   i;
    UINT32   Retval = 0;
    TASK_ARG TaskArg;

    if (GetMsg(MSG_MESSAGEBOX_DESTROY))//message bos destory message.
    {
        BroKeyExitPro();

        switch (gBrowserFindFileType)
        {
            case FileTypeText:
#ifdef _EBOOK_
                TaskArg.MainMenu.MenuID = MAINMENU_ID_EBOOK;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                break;

            case FileTypeVideo:
#ifdef _VIDEO_
                TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                break;

            case FileTypePicture:
#ifdef _PICTURE_
                TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                break;

            case FileTypeALL:
#ifdef _BROWSER_
                TaskArg.MainMenu.MenuID = MAINMENU_ID_BROWSER;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                break;

            default:
                break;
        }
    }

    if (GetMsg(MSG_BROW_CHECK_FILE))
    {
        if (BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems == 0)
        {
#if 0
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
            TaskArg.Message.HoldTime  = 3;
            WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
#else
            UINT16 TextMode;
            TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
            ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
            DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);  //û����Ӧ��ʽ���ļ�, Ҳû��Ŀ¼, ֻ��ʾ����ͼ
            DisplayMenuStrWithIDNum(BROWSER_TITLE_TXT_X, BROWSER_TITLE_TXT_Y,
                                    BROWSER_TITLE_TXT_XSIZE, BROWSER_TITLE_TXT_YSIZE,
                                    LCD_TEXTALIGN_CENTER, BrowserData.BrowserTitleId);
            LCD_SetTextMode(TextMode);
#endif
        }
    }

#if 0

    if (GetMsg(MSG_BROW_DELETEING_FILE))                        //executive file be deleted
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            ThreadDelete(&pMainThread, &MusicThread);
        }

        BroDeleteFile();
        WinDestroy(&MessageBoxWin);
#ifdef _MEDIA_MODULE_
        gSysConfig.MedialibPara.MediaUpdataFlag = 1;//it should notice media update when file be deleted
#endif
        SendMsg(MSG_BROW_DIS_ALL);
        Retval = 0;
    }

    if (CheckMsg(MSG_BROW_DELETE_FILE)
        && ((CheckMsg(MSG_DIALOG_KEY_OK) || CheckMsg(MSG_DIALOG_KEY_CANCEL))))
    {
        if (GetMsg(MSG_DIALOG_KEY_OK))
        {
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_DELETING_FILE;
            TaskArg.Message.HoldTime  = 3;
            WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
            SendMsg(MSG_BROW_DELETEING_FILE);
            ClearMsg(MSG_BROW_DELETE_FILE);                     //he clear message in here should not turn two to one with below.
        }

        if (GetMsg(MSG_DIALOG_KEY_CANCEL))                      //to refresh screen when dialog return cancel message
        {
            SendMsg(MSG_BROW_DIS_ALL);
            ClearMsg(MSG_BROW_DELETE_FILE);
        }
    }

#endif

    if (GetMsg(MSG_SDCARD_UPDATE))
    {
#ifdef _SDCARD_
        BroSDCardCheck();
#endif
    }

#ifdef _USB_HOST_
    if (GetMsg(MSG_USBHOT_UPDATE))
    {
        BroUSBHostCheck();
    }
#endif


//add for breakpoint play
#ifdef AUDIOHOLDONPLAY
    if (TRUE == GetMsg(MSG_BROW_TO_BREAKPOINT))
    {
        //SendMsg(MSG_MEDIALIB_TO_BREAKPOINT);

        DEBUG("HoldOnplay : %d", gSysConfig.MusicConfig.HoldOnPlaySaveFlag);

        if (gSysConfig.MusicConfig.HoldOnPlaySaveFlag) //have play breakpoint
        {
            AudioFileInfo.ucSelPlayType = gSysConfig.MusicConfig.HoldMusicPlayType;
            gMusicTypeSelID = gSysConfig.MusicConfig.HoldMusicPlayType;

            for (i = 0; i < (SYS_SUPPROT_STRING_MAX_LEN + 1); i++)
            {
                gMusicTypeSelName[i] = gSysConfig.MusicConfig.HoldMusicTypeSelName[i];
            }

            AudioFileInfo.pExtStr = AudioFileExtString;
            AudioFileInfo.PlayedFileNum = gSysConfig.MusicConfig.FileNum;
            gwSaveDirClus = gSysConfig.MusicConfig.HoldClusSave;
            HoldMusicStart();
            TaskArg.Music.FileNum       = gSysConfig.MusicConfig.FileNum;
            TaskArg.Music.MediaTitleAdd = SID_DIR_LIST;
            TaskSwitch(TASK_ID_MUSIC, &TaskArg);
            Retval = 1;
        }
    }
#endif

    return (Retval);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 BrowserWinKey(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
UINT32 BrowserWinKey(void)
{
    INT32 KeyVal;
    INT16 RetVal = MSG_MODULE_OK;
    BrowserFileStruct *pBro;
    TASK_ARG TaskArg;
    KeyVal = GetKeyVal();

    //5 Key function modify
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (KeyVal)
        {
            case KEY_VAL_FFD_DOWN:
                KeyVal = KEY_VAL_DOWN_DOWN;
                break;

            case KEY_VAL_FFD_PRESS:
                KeyVal = KEY_VAL_DOWN_PRESS;
                break;

            case KEY_VAL_FFW_DOWN:
                KeyVal = KEY_VAL_UP_DOWN;
                break;

            case KEY_VAL_FFW_PRESS:
                KeyVal = KEY_VAL_UP_PRESS;
                break;

            default:
                break;
        }
    }

    //6 Key modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
        switch (KeyVal)
        {
            case KEY_VAL_FFW_SHORT_UP:
                KeyVal = KEY_VAL_ESC_SHORT_UP;
                break;

            default:
                break;
        }
    }

    //7 Key function
    switch (KeyVal)
    {
        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (BrowserMemSelEn)
            {
                BroMemSelKeyDown();
            }
            else
#endif
            {
                BroKeyDownProc();
            }

            break;

        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_SHORT_UP:
#if(defined (_SDCARD_)|| defined (_MULT_DISK_))
            if (BrowserMemSelEn)
            {
                BroMemSelKeyUp();
            }
            else
#endif
            {
                BroKeyUpProc();
            }

            break;

        case KEY_VAL_PLAY_SHORT_UP:
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (!BrowserMemSelEn)
#endif
            {
                //BroDeleteFilePro();
            }

            break;

        case KEY_VAL_MENU_SHORT_UP:
#if(defined (_SDCARD_) || defined (_MULT_DISK_) || defined (_USB_HOST_))
            if (BrowserMemSelEn)
            {
                BroMemSelKeyMenu();
            }
            else
#endif
            {
                RetVal = BroKeyEnterPro();
            }

            break;

        case KEY_VAL_ESC_SHORT_UP:
#if(defined (_SDCARD_) || defined (_MULT_DISK_))
            if (BrowserMemSelEn)
            {
                RetVal = BroMemSelKeyExit();
            }
            else
#endif
            {
                RetVal = BroKeyExitPro();
            }

            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&BrowserWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&BrowserWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return RetVal;
}

/*
--------------------------------------------------------------------------------
  Function name : BroUIDisplay
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BrowserWinPaint(void)
{
    UINT16  i, CurNum;
    UINT16  TempColor, TempBkColor, TempCharSize, TempTxtMode, TextMode;
    UINT16  TotalItem;
    UINT16  *printfbuf;
    UINT32   x, y;
    LCD_RECT             r;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;
    BrowserFileStruct   *pBro = BrowserData.pBrowserFile;
#ifdef _FRAME_BUFFER_
    int CurStartIndex = BrowserData.CurStartIndex[BrowserData.CurDeep];
#endif

#ifdef THUMB_DEC_INCLUDE
    FILE* FileHandle;
    FileType    CurFileType;
    ImageMaxHeight = 12;
    ImageMaxWidth = 12;
    IsDisplayBackground(0);
#endif

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_BROW_DIS_ALL)))
    {
        //display backgroud main window.
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);
        DisplayMenuStrWithIDNum(BROWSER_TITLE_TXT_X, BROWSER_TITLE_TXT_Y,
                                BROWSER_TITLE_TXT_XSIZE, BROWSER_TITLE_TXT_YSIZE,
                                LCD_TEXTALIGN_CENTER, BrowserData.BrowserTitleId);
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + BrowserBatteryLevel, 105, 146);
        //send other display message.
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
        ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
    }

    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + BrowserBatteryLevel, 105, 146);
    }

    if (GetMsg(MSG_BROW_DIS_SELE_ITEM))
    {
        if (0/*page to next or prevous*/)
        {
            SendMsg(MSG_BROW_DIS_ALL_ITEM);
        }
        else
        {
            ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
            GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);
            GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
            DisplayMenuStrWithIDNum(BROWSER_TITLE_TXT_X, BROWSER_TITLE_TXT_Y,
                                    BROWSER_TITLE_TXT_XSIZE, BROWSER_TITLE_TXT_YSIZE,
                                    LCD_TEXTALIGN_CENTER, BrowserData.BrowserTitleId);
#if(defined (_SDCARD_) || defined (_MULT_DISK_) || defined (_USB_HOST_))

            if (BrowserMemSelEn)
            {
#ifdef _FRAME_BUFFER_
                CurStartIndex = 0;
#endif
            }

#endif

#ifdef _FRAME_BUFFER_

            for (i = CurStartIndex; i < BROWSER_SCREEN_PER_LINE + CurStartIndex; i++)
#else
            for (i = 0; i < BROWSER_SCREEN_PER_LINE; i++)
#endif
            {
                if ((i == BrowserData.PrePointer) || (i == BrowserData.CurPointer))
                {
#ifdef _FRAME_BUFFER_
                    DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, 3 + (i - CurStartIndex) * 17, 17);
#else

                    if (i == BrowserData.PrePointer)
                    {
                        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, 3 + i * 17, 17);
                    }

#endif
                    r.x0 = 20;
#ifdef _FRAME_BUFFER_
                    r.y0 = (i - CurStartIndex) * 17 + 3;
#else
                    r.y0 = i * 17 + 3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
#endif
                    r.x1 = r.x0 + PicInfo.xSize;
                    r.y1 = r.y0 + 17; //r.y0 + CH_CHAR_XSIZE_12 - 1;

                    if (i == BrowserData.CurPointer)
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON, 19, r.y0);
                    }

#ifdef _FRAME_BUFFER_
#ifdef THUMB_DEC_INCLUDE
                    CurFileType = pBro->FileType;

                    if (/*CurFileType == FileTypeAudio || */CurFileType == FileTypePicture)
                    {
                        gTickCounter = SysTickCounter;

                        if (!gIsImproveFreq)
                        {
                            gIsImproveFreq = TRUE;
                            FREQ_EnterModule(FREQ_JPG); //��������ͼ���뷴����Ƶ����Ƶ
                        }

                        FileHandle = (FILE*)FileOpen(NULL, pBro->FileLocInfo.Clus, pBro->FileLocInfo.Index, gBrowSerFsType, FileOpenStringR);

                        if ((int)FileHandle == NOT_OPEN_FILE)
                        {
                            DEBUG("=====FileOpen error=====");
                            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x,
                                                      PicInfo.y + 5 + 17 * (i - CurStartIndex));
                        }
                        else
                        {
                            ImageLeft = PicInfo1.x;
                            ImageTop = 5 + 17 * (i - CurStartIndex);

                            if (CurFileType == FileTypePicture)
                            {
                                SetPicFileType(IMAGE_PIC);
                            }

                            if (!ThumbParse(FileHandle))
                            {
                                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x,
                                                          PicInfo.y + 5 + 17 * (i - CurStartIndex));
                            }

                            FileClose((HANDLE)FileHandle);
                        }
                    }
                    else
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x,
                                                  PicInfo.y + 5 + 17 * (i - CurStartIndex));
                    }

#else
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x,
                                              PicInfo.y + 5 + 17 * (i - CurStartIndex));
#endif
#else
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y + 5 + 17 * i);
#endif

                    if (i == BrowserData.CurPointer)
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON, 19, r.y0);

                        if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                        {
                            PrintfBuf = pBro->LongFileName;
                            BrowserWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 30);
                            SendMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                        }
                        else
                        {
                            ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                        }
                    }

                    r.y0 += 2;
                    r.y1 =  r.y0 + CH_CHAR_XSIZE_12 - 1;
                    TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
                    LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);
                    LCD_SetTextMode(TextMode);
#if 0

                    if (i == BrowserData.CurPointer)//an 4.21
                    {
                        LCD_SetTextMode(TextMode);
                    }

#endif
                }

                pBro = pBro->pNext;
            }
        }

        SendMsg(MSG_BROW_DIS_SCLLO_ITEM);
    }

    if (GetMsg(MSG_BROW_DIS_ALL_ITEM))
    {
        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, 3, 143);
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);
        ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
#ifdef _FRAME_BUFFER_
        TotalItem = BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems;
#else
        TotalItem = BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems - BrowserData.CurStartIndex[BrowserData.CurDeep];
#endif
#if(defined (_SDCARD_) || defined (_MULT_DISK_))

        if (BrowserMemSelEn)
        {
            TotalItem = BrowserData.TotleDisk;

#ifdef _FRAME_BUFFER_
            CurStartIndex = 0;//BrowserData.CurPointer;
#endif

            DEBUG(" TotalItem = %d CurPoint = %d", TotalItem, BrowserData.CurPointer);
        }

#endif

#ifdef _FRAME_BUFFER_

        for (i = CurStartIndex; ((i < BROWSER_SCREEN_PER_LINE + CurStartIndex) && (i < TotalItem )); i++)
#else
        for (i = 0; ((i < BROWSER_SCREEN_PER_LINE) && (i < TotalItem )); i++)
#endif
        {
            //draw file type icon. add 2 in here is because the basic pictrue is smaller 4 than file icon in y coordinate,so get middle value.
#ifdef _FRAME_BUFFER_
            DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, 3 + (i - CurStartIndex) * 17, 17);
#ifdef THUMB_DEC_INCLUDE
            CurFileType = pBro->FileType;

            if (/*CurFileType == FileTypeAudio || */CurFileType == FileTypePicture)
            {
                gTickCounter = SysTickCounter;

                if (!gIsImproveFreq)
                {
                    gIsImproveFreq = TRUE;
                    FREQ_EnterModule(FREQ_JPG); //��������ͼ���뷴����Ƶ����Ƶ
                }

                FileHandle = (FILE*)FileOpen(NULL, pBro->FileLocInfo.Clus, pBro->FileLocInfo.Index, gBrowSerFsType, FileOpenStringR);

                if ((int)FileHandle == NOT_OPEN_FILE)
                {
                    DEBUG("=====FileOpen error=====");
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x,
                                              PicInfo.y + 5 + 17 * (i - CurStartIndex));
                }
                else
                {
                    ImageLeft = PicInfo1.x;
                    ImageTop = 5 + 17 * (i - CurStartIndex);

                    if (CurFileType == FileTypePicture)
                    {
                        SetPicFileType(IMAGE_PIC);
                    }

                    if (!ThumbParse(FileHandle))
                    {
                        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x,
                                                  PicInfo.y + 5 + 17 * (i - CurStartIndex));
                    }

                    FileClose((HANDLE)FileHandle);
                }
            }
            else
            {
                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x,
                                          PicInfo.y + 5 + 17 * (i - CurStartIndex));
            }

#else
            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x,
                                      PicInfo.y + 5 + 17 * (i - CurStartIndex));
#endif
            r.x0 = 20;
            r.y0 = (i - CurStartIndex) * 17 + 3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0 + 17; //r.y0 + CH_CHAR_XSIZE_12 - 1;
#else
            DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, 3 + i * 17, 17);
            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y + 5 + 17 * i);
            r.x0 = 20;
            r.y0 = i * 17 + 3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0 + 17; //r.y0 + CH_CHAR_XSIZE_12 - 1;
#endif

            if (i == BrowserData.CurPointer)
            {
                DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON, 19, r.y0);

                if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                {
                    PrintfBuf = pBro->LongFileName;
                    BrowserWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 30);
                    SendMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                }
                else
                {
                    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
                }
            }

            r.y0 += 2;
            r.y1 =  r.y0 + CH_CHAR_XSIZE_12 - 1;
            TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
            LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);
            LCD_SetTextMode(TextMode);
            pBro = pBro->pNext;
        }

        SendMsg(MSG_BROW_DIS_SCLLO_ITEM);
    }

    if (GetMsg(MSG_BROW_DIS_SCLLO_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL_BLOCK, &PicInfo1);
#ifdef _FRAME_BUFFER_
        CurNum = BrowserData.CurPointer;
#else
        CurNum = BrowserData.CurStartIndex[BrowserData.CurDeep] + BrowserData.CurPointer;
#endif

        if (CurNum > 0)
            y = (PicInfo.ySize - PicInfo1.ySize) * CurNum  / ( BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems - 1); //+picInfo1.ySize/2;
        else
            y = 0;

        if ((y + PicInfo1.ySize) > PicInfo.ySize)
            y = PicInfo.ySize - PicInfo1.ySize;

        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_SCOLL, 122, 0);
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_SCOLL_BLOCK, 0 + 123, y); //scroll smal icon
    }

    //if(CheckMsg(MSG_BROW_DELETE_FILE))//not scroll display when delete file
    //{
    //    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
    //}

    if (CheckMsg(BROWSER_DISPFLAG_SCROLL_FILENAME))//an 4.21
    {
        TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        ScrollStringCommon(PrintfBuf);
        LCD_SetTextMode(TextMode);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);

#ifdef THUMB_DEC_INCLUDE
    if (SysTickCounter - gTickCounter > 500) //����5sû������ͼ�����������Ƶ
    {
        if (gIsImproveFreq)
        {
            gIsImproveFreq = FALSE;
            FREQ_ExitModule(FREQ_JPG);
        }
    }

#endif
}

/*
--------------------------------------------------------------------------------
  Function name : BrowserValueInit
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BrowserWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
    PicturePartInfo     PicPartInfo;
    LCD_RECT            pRect1;
    PicPartInfo.x       = pRect->x0;
    PicPartInfo.y       = pRect->y0;
    PicPartInfo.yoffset = 0;
    PicPartInfo.ysize   = 16;
    pRect1.x0 = pRect->x0;
    pRect1.y0 = pRect->y0 + 2;
    pRect1.x1 = pRect->x1;
    pRect1.y1 = pRect->y0 + CH_CHAR_XSIZE_12 - 1;;

    PicPartInfo.pictureIDNump = ImageID;
    SetScrollStringInfo(&pRect1, PicPartInfo, pstr, Speed);
}

/*
--------------------------------------------------------------------------------
  Function name : BrowserValueInit
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BrowserValueInit(UINT16 NeedInitAll)
{
    uint32 i;

    if (NeedInitAll)
    {
#ifdef _FRAME_BUFFER_

        if (!CheckMsg(MSG_BROW_TASK_HAS_SWITCH))
#endif
        {
            memset((UINT8*)&BrowserData, 0, sizeof(BrowserData));
        }

        BrowserData.pSearchFileInfo = GetSearchFileInfo(gBrowserFindFileType);

        switch (gBrowserFindFileType)
        {
            case FileTypeAudio:
                BrowserData.BrowserTitleId = SID_MUSIC_FILE;
                break;

            case FileTypeText:
                BrowserData.BrowserTitleId = SID_TEXT;
                break;

            case FileTypeVideo:
                BrowserData.BrowserTitleId = SID_VIDEO_FILE;
                break;

            case FileTypePicture:
                BrowserData.BrowserTitleId = SID_PICTURE_FILE;
                break;

            default:
                BrowserData.BrowserTitleId = SID_EXPLORER;
                break;
        }

        BrowserData.FolderInfo[BrowserData.CurDeep].DirClus = GetRootDirClus(gBrowSerFsType);
    }

    BrowserData.PrePointer = 0;
    BrowserData.CurPointer = 0;

    memset(BrowserFileItem, 0, sizeof(BrowserFileItem));

    for (i = 0; i < BROWSER_SCREEN_PER_LINE - 1; i++)
    {
        BrowserFileItem[i].pNext = &BrowserFileItem[i + 1];
    }

    for (i = 1; i < BROWSER_SCREEN_PER_LINE; i++)
    {
        BrowserFileItem[i].pPrev = &BrowserFileItem[i - 1];
    }

    BrowserFileItem[0].pPrev = &BrowserFileItem[BROWSER_SCREEN_PER_LINE - 1];
    BrowserFileItem[BROWSER_SCREEN_PER_LINE - 1].pNext = &BrowserFileItem[0];
    BrowserData.pBrowserFile     = &BrowserFileItem[0];
}

/*
--------------------------------------------------------------------------------
  Function name : void BroGotoCurFile(UINT16 GlobeFileNum)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroGotoCurFile(UINT16 GlobeFileNum)
{
    UINT16 CurFileNum;
    UINT16 i;
    BroFileInfo *pSearchFileInfo;
    FIND_DATA FindData;
    pSearchFileInfo = GetSearchFileInfo(gBrowserFindFileType);

    if (gBrowSerFsType == MUSIC_DB)
    {
        gwSaveDirClus = 0;
    }
    else
    {
        gwSaveDirClus = BootSector.BPB_RootClus;
    }

    CurFileNum      = GetCurFileNum(GlobeFileNum, &FindData, pSearchFileInfo->FileExtName, gBrowSerFsType);
    BrowserData.CurDeep = CurDirDeep;

    for (i = 0; i <= CurDirDeep; i++)
    {
        BrowserData.FolderInfo[i].DirClus = SubDirInfo[i].DirClus;
#ifdef _FRAME_BUFFER_

        if (!CheckMsg(MSG_BROW_TASK_HAS_SWITCH))
#endif
        {
            BrowserData.CurPointerBack[i] = (SubDirInfo[i].CurDirNum) % BROWSER_SCREEN_PER_LINE;

            if (gBrowSerFsType == FS_FAT && BrowserData.CurPointerBack[i] > 0)
            {
                BrowserData.CurPointerBack[i]--;
            }

            BrowserData.CurStartIndex[i] = ((SubDirInfo[i].CurDirNum)  / BROWSER_SCREEN_PER_LINE) * BROWSER_SCREEN_PER_LINE;
        }
    }

#ifdef _FRAME_BUFFER_
    ClearMsg(MSG_BROW_TASK_HAS_SWITCH);
#endif
    FolderSearchFile(pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep], gBrowSerFsType);

    if (CurFileNum > BrowserData.FolderInfo[CurDirDeep].TotalFiles)
    {
        CurFileNum = BrowserData.FolderInfo[CurDirDeep].TotalFiles;
    }

    //if( gSysConfig.MusicConfig.RepeatModeBak <= AUDIO_REPEAT)
    {
        CurFileNum += BrowserData.FolderInfo[CurDirDeep].TotalFolders;//add direction to locate file
    }

#ifdef _FRAME_BUFFER_
    BrowserData.CurPointer = CurFileNum - 1;

    if (gBrowserData_CurStartIndex > BrowserData.CurPointer)
        BrowserData.CurStartIndex[BrowserData.CurDeep] = BrowserData.CurPointer;
    else if ((BrowserData.CurPointer + 1 - BROWSER_SCREEN_PER_LINE) > gBrowserData_CurStartIndex)
        BrowserData.CurStartIndex[BrowserData.CurDeep] = BrowserData.CurPointer + 1 - BROWSER_SCREEN_PER_LINE;
    else
        BrowserData.CurStartIndex[BrowserData.CurDeep] = gBrowserData_CurStartIndex;

#else
    BrowserData.CurStartIndex[BrowserData.CurDeep] = ((CurFileNum - 1) / BROWSER_SCREEN_PER_LINE) * BROWSER_SCREEN_PER_LINE;
    BrowserData.CurPointer = CurFileNum - BrowserData.CurStartIndex[BrowserData.CurDeep] - 1;
#endif
    BrowserData.PrePointer = BrowserData.CurPointer;
    FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile,
                      &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep], gBrowSerFsType);
}

/*
--------------------------------------------------------------------------------
  Function name : FolderSearchFile
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
uint16 FolderSearchFile(uint8 * ExtName, FolderInfoStruct *pFolderInfo, FS_TYPE FsType)
{
    uint16 ret = RETURN_OK;
    pFolderInfo->TotalFiles = GetTotalFiles(pFolderInfo->DirClus, ExtName, FsType);
    pFolderInfo->TotalFolders = GetTotalSubDir(pFolderInfo->DirClus, FsType);

    if (FsType == MUSIC_DB)
    {
        if (pFolderInfo->TotalFolders >= 1)
        {
            pFolderInfo->TotalFolders++;
        }
    }

    pFolderInfo->TotalItems = pFolderInfo->TotalFiles + pFolderInfo->TotalFolders;

    if (FsType == MUSIC_DB) //if total file larger than 8192,we just only show 8192.
    {
        if (8192 < pFolderInfo->TotalItems)
        {
            pFolderInfo->TotalItems = 8192;
        }
    }

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : FolderGetItemInfo
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void FolderGetItemInfo(uint8 * ExtName, BrowserFileStruct*pBrowserFile,
                       FolderInfoStruct *pFolderInfo , UINT16 StartItem, FS_TYPE FsType)
{
    uint32 index;
    uint16 ret = RETURN_OK;
    FDT    TempFDT;
    FIND_DATA FindData;
    int16  NeedLoadFileNum;
    int16  NeedLoadFolderNum;
    int16  SkipLoadFileNum;
    int16  SkipLoadFolderNum;
    BrowserFileStruct* pBroFolder;
    BrowserFileStruct* pBroFile;

    if (pFolderInfo->TotalFolders > StartItem) //StartItem base 0
    {
        NeedLoadFolderNum = pFolderInfo->TotalFolders - StartItem;

        if (NeedLoadFolderNum >= BROWSER_SCREEN_PER_LINE)
        {
            NeedLoadFileNum = 0;
            NeedLoadFolderNum = BROWSER_SCREEN_PER_LINE;
        }
        else
        {
            NeedLoadFileNum = BROWSER_SCREEN_PER_LINE - NeedLoadFolderNum;
        }

        SkipLoadFileNum = 0;
        SkipLoadFolderNum = StartItem;
    }
    else
    {
        SkipLoadFolderNum = pFolderInfo->TotalFolders;
        NeedLoadFolderNum = 0;
        NeedLoadFileNum = pFolderInfo->TotalItems - StartItem;

        if (NeedLoadFileNum >= BROWSER_SCREEN_PER_LINE)
        {
            NeedLoadFileNum = BROWSER_SCREEN_PER_LINE;
        }

        SkipLoadFileNum = StartItem - pFolderInfo->TotalFolders;
    }

    pBroFolder = pBrowserFile;
    pBroFile = pBrowserFile + NeedLoadFolderNum;
    FindData.Clus = pFolderInfo->DirClus;
    FindData.Index = 0;
    FindData.TotalItem = pFolderInfo->TotalItems;

    if (FindFirst(&TempFDT, &FindData, ExtName, FsType) != RETURN_OK)
    {
        return;
    }

    if (FsType == MUSIC_DB)
    {
        if (NeedLoadFolderNum)
        {
            if (SkipLoadFolderNum == 0)
            {
                pBroFolder->FileType = FileTypeFolder;
                memcpy(pBroFolder->LongFileName, L"All files\0", 20);
                pBroFolder->FileLocInfo.Clus = FindData.Clus;
                pBroFolder->FileLocInfo.Index = FindData.Index - 1;
                pBroFolder = pBroFolder->pNext;
                NeedLoadFolderNum--;
            }
            else
            {
                SkipLoadFolderNum--;
            }
        }
    }

    do
    {
        if (TempFDT.Attr & ATTR_DIRECTORY)
        {
            if (NeedLoadFolderNum)
            {
                if (SkipLoadFolderNum == 0)
                {
                    //get direction
                    pBroFolder->FileFDT =  TempFDT;
                    pBroFolder->FileType = FileTypeFolder;
                    pBroFolder->FileLocInfo.Clus = FindData.Clus;
                    pBroFolder->FileLocInfo.Index = FindData.Index - 1;
                    GetLongFileName(FindData.Clus, FindData.Index - 1, FsType, pBroFolder->LongFileName);
                    pBroFolder = pBroFolder->pNext;
                    NeedLoadFolderNum--;
                }
                else
                {
                    SkipLoadFolderNum--;
                }
            }
        }
        else if (NeedLoadFileNum)
        {
            if (SkipLoadFileNum == 0)
            {
                BroFileInfo * TempFileInfo = BroGetFileInfo(TempFDT.Name);
                //file get
                pBroFile->FileFDT =  TempFDT;

                if (FsType == MUSIC_DB)
                {
                    pBroFile->FileType = FileTypeAudio;
                }
                else
                {
                    pBroFile->FileType = TempFileInfo->FileType;
                }

                pBroFile->FileLocInfo.Clus = FindData.Clus;
                pBroFile->FileLocInfo.Index = FindData.Index - 1;
                GetLongFileName(FindData.Clus, FindData.Index - 1, FsType , pBroFile->LongFileName);
                FileExtNameRemove(pBroFile->LongFileName, AudioFileExtString);
                pBroFile = pBroFile->pNext;
                NeedLoadFileNum--;
            }
            else
            {
                SkipLoadFileNum--;
            }
        }

        if (NeedLoadFolderNum == 0 && NeedLoadFileNum == 0)
        {
            break;  //file find out.
        }
    }
    while (FindNext(&TempFDT, &FindData, ExtName, FsType) == RETURN_OK);
}

/*
--------------------------------------------------------------------------------
  Function name : BroGetFileNumInFloder
  Author        : ChenFen
  Description   :

  Input         : when Flag =0 ,it go to get file number of all kinds of file.
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
int16 BroGetFileNumInFloder(BrowserFileStruct *pBro, UINT16 Flag)
{
    int16  i;
    int16  ret;
    int16  FileNum = 0;
    int32  index   = 0;
    FDT    TempFDT;
    FileType FileType ;//= pBro->FileType;
    UINT32 DirClus    = pBro->FileLocInfo.Clus;

    if (Flag)
    {
        FileType = pBro->FileType;
    }
    else
    {
        FileType = FileTypeALL;
    }

    while (1)
    {
        ret = GetFDTInfo(&TempFDT,  DirClus, index++);

        if ( RETURN_OK != ret)
        {
            break;
        }

        if (TempFDT.Name[0] == FILE_NOT_EXIST)            //null direction item,there is no file in follow,
        {
            break;
        }

        if (TempFDT.Name[0] != FILE_DELETED)
        {
            while (TempFDT.Attr == ATTR_LFN_ENTRY)       //long file name item should find short file name
            {
                GetFDTInfo(&TempFDT,  DirClus, index++);
            }

            if ( TempFDT.Attr & ATTR_VOLUME_ID )
                continue;

            if ( TempFDT.Attr & ATTR_HIDDEN )
            {
                DEBUG("Dir or File is Hidden attr...");
                continue;
            }

            if (!( TempFDT.Attr & ATTR_DIRECTORY ))//file
            {
                if ( BroCheckFileType( TempFDT.Name, FileType ))
                {
                    FileNum++;

                    for (i = 0; i < 11; i++)
                    {
                        if (pBro->FileFDT.Name[i] != TempFDT.Name[i])
                            break;
                    }

                    if (11 == i)
                        break;
                }
            }
        }
    }

    return (FileNum);
}

/*
--------------------------------------------------------------------------------
  Function name : void BroKeyDownProc(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef _FRAME_BUFFER_
extern void LCD_Shift_Window(UINT16 frame_index,
                             BUFFER_SHIFT_DIRECTION  direction,
                             UINT16 distance,
                             UINT16 x0, UINT16 y0,
                             UINT16 x1, UINT16 y1);
#endif

_ATTR_BRO_UI_CODE_
void BroKeyDownProc(void)
{
    UINT16 TotalItems = BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems;

    if (TotalItems == 0)
        return;

#ifdef _FRAME_BUFFER_

    if (BrowserData.CurPointer < (TotalItems - 1))
#else
    if ((BrowserData.CurPointer + BrowserData.CurStartIndex[BrowserData.CurDeep]) < TotalItems - 1)
#endif
    {
        BrowserData.PrePointer = BrowserData.CurPointer;

        if ((BrowserData.CurPointer ) >= BROWSER_SCREEN_PER_LINE - 1)
        {
#ifdef _FRAME_BUFFER_
            BrowserData.CurPointer++;

            if ((BrowserData.CurPointer - BrowserData.CurStartIndex[BrowserData.CurDeep])
                == BROWSER_SCREEN_PER_LINE)
            {
                LCD_Shift_Window(0, DIRECTION_UP, 17, 0, 3, 122, 139);
                BrowserData.CurStartIndex[BrowserData.CurDeep] += 1;

                FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                                  BrowserData.pBrowserFile,
                                  &BrowserData.FolderInfo[BrowserData.CurDeep],
                                  BrowserData.CurStartIndex[BrowserData.CurDeep],
                                  gBrowSerFsType);
            }

            SendMsg(MSG_BROW_DIS_SELE_ITEM);
#else
            BrowserData.CurStartIndex[BrowserData.CurDeep] += BROWSER_SCREEN_PER_LINE;
            BrowserData.CurPointer = 0;
            FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                              BrowserData.pBrowserFile,
                              &BrowserData.FolderInfo[BrowserData.CurDeep],
                              BrowserData.CurStartIndex[BrowserData.CurDeep],
                              gBrowSerFsType);
            SendMsg(MSG_BROW_DIS_ALL_ITEM);
#endif
        }
        else
        {
            BrowserData.CurPointer++;
            SendMsg(MSG_BROW_DIS_SELE_ITEM);
        }
    }
    else
    {
        BrowserData.CurStartIndex[BrowserData.CurDeep] = 0;
        BrowserData.CurPointer = 0;

        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                          BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep],
                          BrowserData.CurStartIndex[BrowserData.CurDeep],
                          gBrowSerFsType);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void BroKeyUpProc(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroKeyUpProc(void)
{
    UINT16 TotalItems = BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems;

    if (TotalItems == 0)
        return;

    BrowserData.PrePointer = BrowserData.CurPointer;

#ifdef _FRAME_BUFFER_

    if (BrowserData.CurPointer > 0)
    {
        BrowserData.CurPointer--;

        if (BrowserData.CurPointer < BrowserData.CurStartIndex[BrowserData.CurDeep])
        {
            LCD_Shift_Window(0, DIRECTION_DOWN, 17, 0, 3, 122, 139);
            BrowserData.CurStartIndex[BrowserData.CurDeep] -= 1;

            FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile,
                              &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep], gBrowSerFsType);
        }

        SendMsg(MSG_BROW_DIS_SELE_ITEM);
    }
    else
    {
        //return to the last page.
        if (TotalItems >= BROWSER_SCREEN_PER_LINE)
        {
            BrowserData.CurStartIndex[BrowserData.CurDeep] = TotalItems - BROWSER_SCREEN_PER_LINE;
        }
        else
        {
            BrowserData.CurStartIndex[BrowserData.CurDeep] = 0;
        }

        BrowserData.CurPointer = TotalItems - 1;

        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep], gBrowSerFsType);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }

#else

    if (BrowserData.CurPointer > 0)
    {
        BrowserData.CurPointer--;
        SendMsg(MSG_BROW_DIS_SELE_ITEM);
    }
    else
    {
        if (BrowserData.CurStartIndex[BrowserData.CurDeep] > 0)
        {
            //previous
            BrowserData.CurStartIndex[BrowserData.CurDeep] -= BROWSER_SCREEN_PER_LINE;
            BrowserData.CurPointer = BROWSER_SCREEN_PER_LINE - 1;
        }
        else
        {
            //return to the last page.
            BrowserData.CurStartIndex[BrowserData.CurDeep] = ((TotalItems - 1) / BROWSER_SCREEN_PER_LINE) * BROWSER_SCREEN_PER_LINE;
            BrowserData.CurPointer = TotalItems - BrowserData.CurStartIndex[BrowserData.CurDeep] - 1;
        }

        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep], gBrowSerFsType);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }

#endif
}



/*
--------------------------------------------------------------------------------
  Function name : int16 BroKeyEnterPro(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
int16 BroKeyEnterPro(void)
{
    UINT16      i;
    UINT16      temp;
    FileType    CurFileType;
    BrowserFileStruct *pBro;
    TASK_ARG    TaskArg;
    BroFileInfo *FileExtName;
    pBro = BrowserData.pBrowserFile;

#ifdef _FRAME_BUFFER_

    for (i = BrowserData.CurStartIndex[BrowserData.CurDeep]; i < BrowserData.CurPointer; i++)
    {
        pBro = pBro->pNext;
    }

#else

    for (i = 0; i < BrowserData.CurPointer; i++)
    {
        pBro = pBro->pNext;
    }

#endif
    CurFileType = pBro->FileType;

    if (CurFileType == FileTypeFolder)
    {
        if (0 == BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems)
        {
            return 0 ;//it go to do nothing if is null.
        }

        if ((BrowserData.CurStartIndex[BrowserData.CurDeep] == 0)  &&  (BrowserData.CurPointer == 0)
            && (gBrowSerFsType == MUSIC_DB))
        {
            i = 1;
            FileExtName  = GetSearchFileInfo(FileTypeAudio);
#ifdef _MUSIC_
            //printf("From All File Clus = %d, FileExName = %d", pBro->FileLocInfo.Clus, FileExtName->FileExtName);
            TaskArg.Music.FileNum = GetGlobeFileNum(i, pBro->FileLocInfo.Clus, FileExtName->FileExtName, gBrowSerFsType);

            //printf("From All File: AudioFileNum = %d\n", TaskArg.Music.FileNum);

            if (BrowserData.CurDeep == 0)
            {
                GetResourceStr(SID_DIR_LIST, gMusicTypeSelName, MAX_FILENAME_LEN);
            }
            else
            {
                GetLongFileName(SubDirInfo[CurDirDeep - 1].DirClus, SubDirInfo[CurDirDeep - 1].CurDirNum - 1, MUSIC_DB,
                                gMusicTypeSelName);
            }

            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }

#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif

            gSysConfig.MusicConfig.RepeatModeBak =  gSysConfig.MusicConfig.RepeatMode + AUDIO_ALLONCE;
            AudioFileInfo.ucSelPlayType = SORT_TYPE_SEL_FOLDER;
            gwSaveDirClus = BrowserData.FolderInfo[BrowserData.CurDeep].DirClus;
            //ThreadDeleteAll(&pMainThread);
#ifdef AUDIOHOLDONPLAY  //4.debug,it can not switch music when in brower.
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0 ;
#endif
            SendMsg(MSG_BROW_TASK_HAS_SWITCH);
            TaskSwitch(TASK_ID_MUSIC, &TaskArg);
            return 1;
#endif
        }

        if (BrowserData.CurDeep < MAX_DIR_DEPTH - 1)
        {
            BrowserData.CurPointerBack[BrowserData.CurDeep] = BrowserData.CurPointer;
            BrowserData.CurDeep++;
            BrowserData.CurStartIndex[BrowserData.CurDeep] = 0;
            BrowserData.CurPointer = 0;

            if (gBrowSerFsType == MUSIC_DB)
            {
                BrowserData.FolderInfo[BrowserData.CurDeep].DirClus = GetCurDir((pBro->FileLocInfo).Clus, (pBro->FileLocInfo).Index,
                        gBrowSerFsType);
            }
            else
            {
                BrowserData.FolderInfo[BrowserData.CurDeep].DirClus = (((UINT32)pBro->FileFDT.FstClusHI << 16) |
                        pBro->FileFDT.FstClusLO);
            }

            FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep],
                             gBrowSerFsType);
            FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile,
                              &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep], gBrowSerFsType);

            if (0 == BrowserData.FolderInfo[BrowserData.CurDeep].TotalItems)
            {
                //if there is not file,browser interface is null,it do not display dialog.
                SendMsg(MSG_BROW_CHECK_FILE);
            }
            else
            {
                SendMsg(MSG_BROW_DIS_ALL_ITEM);
                ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);//clear scroll message,if not,it still fresh scrolllist when the long file name folder is empty.
            }
        }
    }
    else
    {
        FileExtName = GetSearchFileInfo(pBro->FileType);

        if (gBrowSerFsType == FS_FAT)
        {
            if ((CurFileType != FileTypeAudio) && (CurFileType != FileTypePicture))
            {
                i = BroGetFileNumInFloder(pBro, 0);
                GlobalFilenum = GetGlobeFileNum(i, pBro->FileLocInfo.Clus, "*", gBrowSerFsType);
                i = BroGetFileNumInFloder(pBro, 1);
            }
            else
            {
                i = BroGetFileNumInFloder(pBro, 1);
            }
        }
        else
        {
#ifdef _FRAME_BUFFER_
            i = BrowserData.CurPointer - BrowserData.FolderInfo[BrowserData.CurDeep].TotalFolders + 1;
#else
            i = BrowserData.CurStartIndex[BrowserData.CurDeep] + BrowserData.CurPointer -
                BrowserData.FolderInfo[BrowserData.CurDeep].TotalFolders + 1;
#endif
        }

        if (i == 0) //delete all file.then do not exit to brower,but to key menu the device crash.
        {
            return 0;
        }

#ifdef _FRAME_BUFFER_
        gBrowserData_CurStartIndex = BrowserData.CurStartIndex[BrowserData.CurDeep];
#endif

        if (CurFileType == FileTypeAudio)
        {
#ifdef _MUSIC_
#ifdef _BLUETOOTH_
#ifdef _A2DP_SOUCRE_
#else
            if (TRUE == ThreadCheck(pMainThread, &BlueToothThread))
            {
                ThreadDeleteAll(&pMainThread);
            }
#endif
#endif

            if (gSysConfig.FindFileType != FileTypeAudio)
            {
                i = BroGetFileNumInFloder(pBro, 0);
                GlobalFilenum = GetGlobeFileNum(i, pBro->FileLocInfo.Clus, "*", gBrowSerFsType);
                //DEBUG("GlobalFilenum=%d",GlobalFilenum);
                i = BroGetFileNumInFloder(pBro, 1);
            }
            TaskArg.Music.FileNum = GetGlobeFileNum(i, pBro->FileLocInfo.Clus, FileExtName->FileExtName, gBrowSerFsType);

            if (BrowserData.CurDeep == 0)
            {
                GetResourceStr(SID_DIR_LIST, gMusicTypeSelName, MAX_FILENAME_LEN);
            }
            else
            {
                GetLongFileName(SubDirInfo[CurDirDeep - 1].DirClus, SubDirInfo[CurDirDeep - 1].CurDirNum - 1, MUSIC_DB,
                                gMusicTypeSelName);
            }

            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                if ((AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_BROWSER) && (gBrowSerFsType == FS_FAT))
                {
                    //------------------------------------------------------------------------------
                    //this part has called music backgroud variables,only backgroud music playing,it can process the part code.
                    temp  = AudioFileInfo.CurrentFileNum;

                    if (FIND_FILE_RANGE_DIR == AudioFileInfo.Range)
                    {
                        temp = GetGlobeFileNum(AudioFileInfo.CurrentFileNum, AudioFileInfo.FindData.Clus, FileExtName->FileExtName,
                                               gBrowSerFsType);
                    }

                    if (TaskArg.Music.FileNum == temp)
                    {
                        SendMsg(MSG_BROW_TASK_HAS_SWITCH);
                        TaskSwitch(TASK_ID_MUSIC, NULL);
                        return 1;
                    }
                }
                else if ((AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER) && (gBrowSerFsType == MUSIC_DB))
                {
                    //------------------------------------------------------------------------------
                    //this part has called music backgroud variables,only backgroud music playing,it can process the part code.
                    temp  = AudioFileInfo.CurrentFileNum;

                    if (FIND_FILE_RANGE_DIR == AudioFileInfo.Range)
                    {
                        temp = GetGlobeFileNum(AudioFileInfo.CurrentFileNum, AudioFileInfo.FindData.Clus, FileExtName->FileExtName,
                                               gBrowSerFsType);
                    }

                    if (TaskArg.Music.FileNum == temp)
                    {
                        SendMsg(MSG_BROW_TASK_HAS_SWITCH);
                        TaskSwitch(TASK_ID_MUSIC, NULL);
                        return 1;
                    }
                }

                //------------------------------------------------------------------------------
            }

            ThreadDelete(&pMainThread, &MusicThread);

            if (gBrowSerFsType == FS_FAT)
            {
                AudioFileInfo.ucSelPlayType = SORT_TYPE_SEL_BROWSER; //the macro SORT_TYPE_SEL_BROWSER is consist wich meida.
                //gwSaveDirClus = 0;
                gwSaveDirClus = BootSector.BPB_RootClus;    //chad.ma modified 20160826
            }
            else
            {
                gSysConfig.MusicConfig.RepeatModeBak =  gSysConfig.MusicConfig.RepeatMode;
                gwSaveDirClus = 0;
                AudioFileInfo.ucSelPlayType = SORT_TYPE_SEL_FOLDER;
            }

#ifdef AUDIOHOLDONPLAY  //4.debug,it can not switch music when in brower.

            if (TaskArg.Music.FileNum == gSysConfig.MusicConfig.FileNum
                && gSysConfig.MusicConfig.HoldOnPlaySaveFlag == 1)
            {
                SendMsg(MSG_BROW_TO_BREAKPOINT);
                return 1;
            }

            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif
            SendMsg(MSG_BROW_TASK_HAS_SWITCH);
            TaskSwitch(TASK_ID_MUSIC, &TaskArg);
            return 1;
#endif
        }
        else if (CurFileType == FileTypePicture)
        {
#ifdef _PICTURE_
            TaskArg.Pic.FileNum = GetGlobeFileNum(i, pBro->FileLocInfo.Clus, FileExtName->FileExtName, gBrowSerFsType);
            SendMsg(MSG_BROW_TASK_HAS_SWITCH);
            TaskSwitch(TASK_ID_PICTURE, &TaskArg);
            return 1;
#endif
        }
        else if (CurFileType == FileTypeVideo)
        {
#ifdef _VIDEO_
            ThreadDeleteAll(&pMainThread);
            TaskArg.Video.FileNum = GetGlobeFileNum(i, pBro->FileLocInfo.Clus, FileExtName->FileExtName, gBrowSerFsType);
            SendMsg(MSG_BROW_TASK_HAS_SWITCH);
            TaskSwitch(TASK_ID_VIDEO, &TaskArg);
            return 1;
#endif
        }
        else if (CurFileType == FileTypeText)
        {
#ifdef _EBOOK_
            TaskArg.Text.FileNum = GetGlobeFileNum(i, pBro->FileLocInfo.Clus, FileExtName->FileExtName, gBrowSerFsType);
            SendMsg(MSG_BROW_TASK_HAS_SWITCH);
            TaskSwitch(TASK_ID_EBOOK, &TaskArg);
            return 1;
#endif
        }
        else //other file format not support
        {
            DEBUG(" ## NOT SUPPORT FORMAT ##");
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_FILE_FORMAT_ERROR;
            TaskArg.Message.HoldTime  = 3;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            gBrowserKeep  = 1;
            //HorizontalDisplay();
            WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
        }
    }

    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void BroDeleteFilePro(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroDeleteFilePro(void)
{
    uint32 i;
    BrowserFileStruct *pBro;
    TASK_ARG TaskArg;
    pBro = BrowserData.pBrowserFile;

    for (i = 0; i < BrowserData.CurPointer; i++)
    {
        pBro = pBro->pNext;
    }

    if ((pBro->FileType == FileTypeFolder) || (gBrowserFindFileType != FileTypeALL))
    {
        return ;
    }

    i = BroGetFileNumInFloder(pBro, 1);

    if (i == 0)  //no file return.
    {
        return;
    }

    TaskArg.Dialog.Button = DIALOG_BUTTON_NO;
    TaskArg.Dialog.TitleID = SID_WARNING;
    TaskArg.Dialog.ContentID = SID_DELETE_FILE_ASK;
    WinCreat(&BrowserWin, &DialogWin, &TaskArg);

    SendMsg(MSG_BROW_DELETE_FILE);//file delete message.
    ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
    return;
}

#ifdef _USB_HOST_

uint8  bro_usbcon = 0;
uint8  bro_usbdis = 0;

/*
--------------------------------------------------------------------------------
  Function name : uint32 BroUSBHostDetect(void)
  Author        : wrm
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               wrm             2016/9/22         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
uint32 BroUSBHostDetect()
{
    uint32 retval = 0;
    UHC_CHN_INFO *pUsbHC = NULL;

    pUsbHC = (UHC_CHN_INFO *)HostGetChannelInfo();
    if (pUsbHC->ConnectStatus == 1)
    {
        bro_usbdis = 0;
        if (bro_usbcon < 20)
        {
            bro_usbcon++;
            DelayMs(30);
            //�������ӳɹ�
            return FALSE;
        }
        else
        {
            DEBUG ("usb host detected device");
            retval = USBHostEnumDevice();
            if (retval == HOST_OK)
            {
                gSysConfig.UsbHost = 1;
                bro_usbcon = 0;
                return TRUE;
            }
            else
            {
                DEBUG ("usb host connect failure");
                bro_usbcon = 0;
                return 2;
            }
        }
    }
    else
    {
        bro_usbcon = 0;
        if (bro_usbdis < 150)
        {
            bro_usbdis++;
            DelayMs(10);
        }
        else
        {
            //usb host���ӳ�ʱ��δ����ö��
            DEBUG ("usb host deteted timeout");
            bro_usbdis = 0;
            return 2;
        }

        //��������ʧ��
        return FALSE;
    }
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : int16 BroKeyExitPro(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
int16 BroKeyExitPro(void)
{
    uint32   i;
    FileType CurFileType;
    TASK_ARG TaskArg;
    BrowserFileStruct *pBro;

    if (BrowserData.CurDeep > 0)
    {
        BrowserData.CurDeep--;
        BrowserData.CurPointer = BrowserData.CurPointerBack[BrowserData.CurDeep];
        FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName,
                         &BrowserData.FolderInfo[BrowserData.CurDeep],
                         gBrowSerFsType);
        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                          BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep],
                          BrowserData.CurStartIndex[BrowserData.CurDeep],
                          gBrowSerFsType);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
        return (MSG_MODULE_OK);
    }
    else
    {
#ifdef _MEDIA_MODULE_

        if (gBrowSerFsType == MUSIC_DB)
        {
            if (gBrowserFindFileType == FileTypeAudio)
            {
                //�޴���ѡ������ý���
                if ( BroswerFlag == FALSE)
                {
                    TaskArg.Medialib.CurId = gMusicTypeSelID;
                    TaskSwitch(TASK_ID_MEDIALIB, &TaskArg); //exit to media
                    return (MSG_MODULE_EXIT);
                }
                //�д���ѡ���������ѡ��
                else
                {
                    BrowserMemSelEn = 1;
                    BroLoadMemSelStr();

                    SendMsg(MSG_BROW_DIS_ALL_ITEM);
                    return (MSG_MODULE_OK);
                }
            }
        }
        else
#endif
        {
#if(defined (_SDCARD_) || defined (_MULT_DISK_) || defined (_USB_HOST_))

            if (BrowserMemSelEn == 0 && !gBrowserKeep)
            {
                BrowserMemSelEn = 1;
                BroLoadMemSelStr();

                SendMsg(MSG_BROW_DIS_ALL_ITEM);
                return (MSG_MODULE_OK);
            }
            else
#endif
            {
                gBrowserKeep = 0;

                switch (gBrowserFindFileType)
                {
                    case FileTypeAudio:
#ifdef _MUSIC_
#ifdef _MEDIA_MODULE_
                        if (BroswerFlag == FALSE)
                        {
                            TaskSwitch(TASK_ID_MEDIALIB, NULL); //exit to media
                        }
                        else
#endif
                        {
                            TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
                            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                        }

#endif
                        break;

                    case FileTypeText:
#ifdef _EBOOK_
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_EBOOK;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                        break;

                    case FileTypeVideo:
#ifdef _VIDEO_
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                        break;

                    case FileTypePicture:
#ifdef _PICTURE_
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
                        break;

                    default:
                        TaskArg.MainMenu.MenuID = MAINMENU_ID_BROWSER;
                        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                        break;
                }

                return (MSG_MODULE_EXIT);
            }
        }
    }
}

#if(defined (_SDCARD_) || defined (_MULT_DISK_)|| defined (_USB_HOST_))

_ATTR_BRO_UI_CODE_
void BroUSBHostCheck(void)
{
    if ((BrowserMemSel == USBHOST_FLASH) || (BrowserMemSelEn == 1))
    {
        BrowserValueInit(1);
        FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName,
                         &BrowserData.FolderInfo[BrowserData.CurDeep],
                         gBrowSerFsType);
        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                          BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep],
                          BrowserData.CurStartIndex[BrowserData.CurDeep],
                          gBrowSerFsType);
        BrowserMemSel         = FLASH0;
        BrowserData.TotleDisk = 1;

        BrowserMemSelEn = 1;
        BroLoadMemSelStr();

        ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : BroLoadMemSelStr
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroSDCardCheck(void)
{
    //it is to update display item information only when card select interface
    //and brower display the content of sd card
    if ((BrowserMemSel == CARD) || (BrowserMemSelEn == 1))
    {
        BrowserValueInit(1);
        FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName,
                         &BrowserData.FolderInfo[BrowserData.CurDeep],
                         gBrowSerFsType);
        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName,
                          BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep],
                          BrowserData.CurStartIndex[BrowserData.CurDeep],
                          gBrowSerFsType);
        BrowserMemSel         = FLASH0;
        BrowserData.TotleDisk = 1;
        BrowserMemSelEn = 1;
        BroLoadMemSelStr();
        ClearMsg(BROWSER_DISPFLAG_SCROLL_FILENAME);
        SendMsg(MSG_BROW_DIS_ALL_ITEM);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : BroLoadMemSelStr
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroLoadMemSelStr(void)
{
    BrowserFileStruct *pBro = BrowserData.pBrowserFile;
    BrowserData.CurPointer = 0;

#ifdef _MULT_DISK_
    if (BrowserMemSel == CARD)
    {
        BrowserData.CurPointer = 1;

        if (bShowFlash1)
        {
            BrowserData.CurPointer = 2;
        }
    }
    else if (BrowserMemSel == FLASH1)
    {
        BrowserData.CurPointer = 1;
    }

#else
#endif
//���̹��ѡ��
#if(defined (_SDCARD_) && defined (_USB_HOST_))
    //DEBUG("MemorySelect=%d  BrowserMemSel=%d",MemorySelect,BrowserMemSel);
    if (BrowserMemSel == CARD)
    {
        BrowserData.CurPointer = 2;
    }
    else if (BrowserMemSel == USBHOST_FLASH)
    {
        BrowserData.CurPointer = 1;
    }
    else
        BrowserData.CurPointer = 0;
#else

    #ifdef _USB_HOST_
    if (BrowserMemSel == USBHOST_FLASH)
    {
        BrowserData.CurPointer = 1;
    }
    #endif

    #ifdef _SDCARD_
    if (BrowserMemSel == CARD)
    {
        BrowserData.CurPointer = 1;
    }
    #endif

    if (BrowserMemSel == FLASH0)
    {
        BrowserData.CurPointer = 0;
    }
#endif

    //��һ��MainMenu����������ر���C����
    if (gSysConfig.FindFileType == FileTypeNull)
    {
        BrowserData.CurPointer = 0;
        gSysConfig.FindFileType = gBrowserFindFileType;
    }
    BrowserData.PrePointer = BrowserData.CurPointer;

#ifdef _MULT_DISK_

    if (bShowFlash1)
        BrowserData.TotleDisk = 2;
    else
        BrowserData.TotleDisk = 1;

    if (gSysConfig.SDEnable)
    {
        if (CheckCard())
        {
            BrowserData.TotleDisk += 1;
        }
    }
#else
    BrowserData.TotleDisk = 1;

    if (gSysConfig.SDEnable)
    {
        if (CheckCard())
        {
            BrowserData.TotleDisk += 1;
        }
    }
#endif

#ifdef _USB_HOST_
    BrowserData.TotleDisk += 1;
#endif

    //DEBUG("TotleDisk=%d\n",BrowserData.TotleDisk);

    pBro->FileType   = FileTypeFolder;
    GetResourceStr(SID_C_FLASH, pBro->LongFileName, MAX_FILENAME_LEN);

#ifdef _MULT_DISK_

    if (bShowFlash1)
    {
        pBro = pBro->pNext;
        pBro->FileType   = FileTypeFolder;
        GetResourceStr(SID_D_FLASH, pBro->LongFileName, MAX_FILENAME_LEN);
    }
#endif

#ifdef _USB_HOST_
    pBro = pBro->pNext;
    pBro->FileType   = FileTypeFolder;
    GetResourceStr(SID_D_FLASH, pBro->LongFileName, MAX_FILENAME_LEN);//SID_D_SD_CARD
#endif

#ifdef _SDCARD_

    if (gSysConfig.SDEnable)
    {
        pBro = pBro->pNext;
        pBro->FileType   = FileTypeFolder;

#ifdef _MULT_DISK_

        if (bShowFlash1)
        {
            GetResourceStr(SID_E_SD_CARD, pBro->LongFileName, MAX_FILENAME_LEN);
        }
        else
        {
            GetResourceStr(SID_D_SD_CARD, pBro->LongFileName, MAX_FILENAME_LEN);
        }
#else
        #ifdef _USB_HOST_
            GetResourceStr(SID_E_SD_CARD, pBro->LongFileName, MAX_FILENAME_LEN);//SID_D_SD_CARD
        #else
            GetResourceStr(SID_D_SD_CARD, pBro->LongFileName, MAX_FILENAME_LEN);//SID_D_SD_CARD
        #endif
#endif
    }
#endif
}

/*
--------------------------------------------------------------------------------
  Function name : void BroMemSelKeyDown(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroMemSelKeyDown(void)
{
    BrowserData.PrePointer = BrowserData.CurPointer;

    if (++BrowserData.CurPointer >= BrowserData.TotleDisk)
    {
        BrowserData.CurPointer = 0;
    }

    SendMsg(MSG_BROW_DIS_SELE_ITEM);
}

/*
--------------------------------------------------------------------------------
  Function name : void BroMemSelKeyUp(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroMemSelKeyUp(void)
{
    BrowserData.PrePointer = BrowserData.CurPointer;

    if (BrowserData.CurPointer == 0)
    {
        BrowserData.CurPointer = BrowserData.TotleDisk - 1;
    }
    else
    {
        BrowserData.CurPointer--;
    }

    SendMsg(MSG_BROW_DIS_SELE_ITEM);
}

/*
--------------------------------------------------------------------------------
  Function name : void BroMemSelKeyUp(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
void BroMemSelKeyMenu(void)
{
    uint8 FileSysSetupRet;
    uint32 ret = 0;

    if (MemorySelect != BrowserMem[BrowserData.CurPointer])
    {
        if (ThreadCheck(pMainThread, &MusicThread) == TRUE)
        {
            DEBUG("Change the Devices!!!,delete music thread");
            ThreadDelete(&pMainThread, &MusicThread);
        }
        gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;

#ifdef _BLUETOOTH_  //chad.ma add
        BluetoothThreadDelete(gbBTConnected);
#endif

        //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
        MemorySelect = BrowserMem[BrowserData.CurPointer];

#ifdef _MULT_DISK_

        if ((MemorySelect == FLASH1) && (bShowFlash1 == 0))
            MemorySelect = CARD;

#endif

#ifdef _USB_HOST_
        if ((MemorySelect == USBHOST_FLASH) && (gSysConfig.UsbHost == 0))
        {
            DEBUG("BroMenu: UsbHost == 0");
            //����USBHSOT
            USBHostSetParams(HOST_FORBID_INIT, 0);
            USBHostStart();
            //�ȴ�USB ö�ٳɹ�������ʱ�˳�
            while (1)
            {
                ret = BroUSBHostDetect();
                if (ret == TRUE)
                {
                    gSysConfig.Listen_UsbHost = 1;
                    gBrowSerFsType = FS_FAT;
                    BrowserMemSel = USBHOST_FLASH;
                    break;
                }
                else if (ret == 2) //���ӳ�ʱ
                {
                    USBHost_Reinit();
                    break;
                }
                else
                {

                }
            }
        }
        else if ((MemorySelect == USBHOST_FLASH) && (gSysConfig.UsbHost == 1))
        {
            //DEBUG("BroMenu: UsbHost == 1");
            gBrowSerFsType = FS_FAT;
            BrowserMemSel = USBHOST_FLASH;
        }
#endif

#ifdef _SDCARD_

        if (MemorySelect == CARD)
        {
            if (gSysConfig.SDEnable)
            {
                gBrowSerFsType = FS_FAT;
                BrowserMemSel = CARD;
                SDCardEnable();
                MDScanDev(1);
            }
            else
            {
                MemorySelect = FLASH0;
            }
        }

#endif
        if (MemorySelect == FLASH0)
        {
            BrowserMemSel = FLASH0;
            if (gBrowserFindFileType == FileTypeAudio)
            {
                gBrowSerFsType = MUSIC_DB;
            }
            else
            {
                gBrowSerFsType = FS_FAT;
            }
        }

        FileSysSetupRet =  FileSysSetup(MemorySelect);

#if(defined (_SDCARD_) || defined (_USB_HOST_))
        if (FileSysSetupRet != OK)  //�ļ�ϵͳ����ʧ�ܣ��޷�����
        {
            TASK_ARG TaskArg;
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_COULD_NOT_CONTINUE;
            TaskArg.Message.HoldTime  = 1;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;

            WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
            MemorySelect = FLASH0;
            FileSysSetup(MemorySelect);
        }
#endif
    }
    //���л�����
    else
    {
        #ifdef _USB_HOST_
        if (MemorySelect == USBHOST_FLASH)
        {
            gBrowSerFsType = FS_FAT;
            BrowserMemSel = USBHOST_FLASH;
        }
        #endif

        #ifdef _SDCARD_
        if (MemorySelect == CARD)
        {
            gBrowSerFsType = FS_FAT;
            BrowserMemSel = CARD;
        }
        #endif

        if (MemorySelect == FLASH0)
        {
            BrowserMemSel = FLASH0;
            if (gBrowserFindFileType == FileTypeAudio)
            {
                gBrowSerFsType = MUSIC_DB;
            }
            else
            {
                gBrowSerFsType = FS_FAT;
            }
        }
    }

#if 0//def _SDCARD_

    if (MemorySelect == CARD)
    {
        if (gSysConfig.SDEnable)
        {
            SDCardEnable();
            MDScanDev(1);
        }
        else
        {
            MemorySelect = FLASH0;
        }

        FileSysSetupRet =  FileSysSetup(MemorySelect);

        if (FileSysSetupRet == OK)
        {
            SDCardDisable();
            MemorySelect = FLASH0;
            FileSysSetup(MemorySelect);
        }
        else
        {
            TASK_ARG TaskArg;
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_COULD_NOT_CONTINUE;
            TaskArg.Message.HoldTime  = 3;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            WinCreat(&BrowserWin, &MessageBoxWin, &TaskArg);
        }
    }

#endif

    BrowserMemSel = MemorySelect;
    BrowserMemSelEn = 0;
    BrowserValueInit(1);
    FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep],
                     gBrowSerFsType);
    FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile,
                      &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep], gBrowSerFsType);
    SendMsg(MSG_BROW_DIS_ALL);
    SendMsg(MSG_BROW_CHECK_FILE);
}

/*
--------------------------------------------------------------------------------
  Function name : void BroMemSelKeyUp(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
int16 BroMemSelKeyExit(void)
{
    uint32 i;
    FileType CurFileType;
    TASK_ARG TaskArg;

    if (gSysConfig.FindFileType == FileTypeAudio)
    {
        TaskArg.Medialib.CurId = gMusicTypeSelID;
        TaskSwitch(TASK_ID_MEDIALIB, &TaskArg);
        if(BroswerFlag)
            BroswerFlag = FALSE;

        return (MSG_MODULE_EXIT);
    }

    switch (gBrowserFindFileType)
    {
        case FileTypeAudio:
#ifdef _MUSIC_
            //�˳����������ý���������
            TaskArg.Medialib.CurId = gMusicTypeSelID;
            TaskSwitch(TASK_ID_MEDIALIB, &TaskArg);
            if(BroswerFlag)
                BroswerFlag = FALSE;
            #if 0
            TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            if(BroswerFlag)
                BroswerFlag = FALSE;
            #endif
#endif
            break;

        case FileTypeText:
#ifdef _EBOOK_
#if 0
            TaskArg.Text.FileNum = gSysConfig.TextConfig.FileNum;
            TaskSwitch(TASK_ID_EBOOK, &TaskArg);
#else
            TaskArg.MainMenu.MenuID = MAINMENU_ID_EBOOK;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
#endif
            break;

        case FileTypeVideo:
#ifdef _VIDEO_
            TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
            break;

        case FileTypePicture:
#ifdef _PICTURE_
            TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
#endif
            break;

        default:
#ifdef _SPINOR_
            if (BroswerFlag == TRUE)//it do not enter browser from media.
            {
                //4  from music application,gBrowserFindFileType == FileTypeFolder
                TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                BroswerFlag = FALSE;
            }
            else
#else
            {
                //4 from browser application
                TaskArg.MainMenu.MenuID = MAINMENU_ID_BROWSER;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            }
#endif
            break;
    }

    return (MSG_MODULE_EXIT);
}
#endif


/*
--------------------------------------------------------------------------------
  Function name : void BroDeleteFilePro(void)
  Author        : ChenFen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               ChenFen       2009/03/18         Ver1.0
--------------------------------------------------------------------------------
*/
_ATTR_BRO_UI_CODE_
UINT32 BroDeleteFile(void)
{
    UINT32 i;
    UINT32 Retval = 0;
    BrowserFileStruct *pBro = BrowserData.pBrowserFile;

    for (i = 0; i < BrowserData.CurPointer; i++)
    {
        pBro = pBro->pNext;
    }

    if (pBro->FileType == FileTypeFolder)
    {
        return Retval;
    }

//    ModuleOverlay(MODULE_ID_FILE_ENCODE, MODULE_OVERLAY_ALL);
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
    FREQ_EnterModule(FREQ_MAX);//frist to improve the cpu frequency before to delete file.

    if (RETURN_OK == BroFileDelete(pBro->FileLocInfo.Clus, pBro->FileFDT.Name))
    {
//        ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
        FileSysSetup(MemorySelect);
#ifdef AUDIOHOLDONPLAY  //clear breakpoint
        {
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0 ;//zyz 2009-6-16
        }
#endif
        FolderSearchFile(BrowserData.pSearchFileInfo->FileExtName, &BrowserData.FolderInfo[BrowserData.CurDeep],
                         gBrowSerFsType);

        if (BrowserData.CurPointer > 0)
        {
            BrowserData.CurPointer--;
        }
        else
        {
            if (BrowserData.CurStartIndex[BrowserData.CurDeep] > 0)
            {
                BrowserData.CurStartIndex[BrowserData.CurDeep] -= BROWSER_SCREEN_PER_LINE;
                BrowserData.CurPointer = BROWSER_SCREEN_PER_LINE - 1;
            }
        }

        FolderGetItemInfo(BrowserData.pSearchFileInfo->FileExtName, BrowserData.pBrowserFile,
                          &BrowserData.FolderInfo[BrowserData.CurDeep], BrowserData.CurStartIndex[BrowserData.CurDeep], gBrowSerFsType);
        FREQ_ExitModule(FREQ_MAX);//retore cpu frequncy after delete file
        return (RETURN_OK);
    }

    FREQ_ExitModule(FREQ_MAX);//retore cpu frequncy after delete file
    return (RETURN_FAIL);
}
/*
********************************************************************************
*
*                         End of BrowserUI.c
*
********************************************************************************
*/
#endif

