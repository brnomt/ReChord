/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  medialibWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_MEDIALIBWIN_

#include "SysInclude.h"

#ifdef _MEDIA_MODULE_

#include "fsinclude.h"
#include "medialibwin.h"
#include "mainmenu.h"
#include "SysFindFile.h"
#include "FileInfo.h"
#include "AddrSaveMacro.h"
#include "AudioControl.h"
#include "MediaBroWin.h"
#include "BrowserUI.h"
#include "MessageBox.h"
#include "Hold.h"

/*
--------------------------------------------------------------------------------
  Function name : void MedialibWinMsgInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIALIBWIN_INIT_CODE_
void MedialibWinMsgInit(void)
{
    SendMsg(MSG_MEDIAWIN_DISPLAY_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : void MedialibWinMsgDeInit(void)
  Author        : anzhiguo
  Description   : kill main menu message entry

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             anzhiguo     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIALIBWIN_INIT_CODE_
void MedialibWinMsgDeInit(void)
{
}
/*
--------------------------------------------------------------------------------
  Function name : void MedialibWinValueInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               anzhiguo        2009-3-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIALIBWIN_INIT_CODE_
int MedialibWinGetCurNum(int CurId)
{
    int CurNum = 0;

    //Must be match with MediaItem
    switch (CurId)
    {
        case SORT_TYPE_SEL_NOW_PLAY:
        case SORT_TYPE_SEL_ID3TITLE:
        case SORT_TYPE_SEL_ID3SINGER:
        case SORT_TYPE_SEL_ID3ALBUM:
        case SORT_TYPE_SEL_GENRE:
            CurNum = CurId;
            break;

        case SORT_TYPE_SEL_FOLDER:
            CurNum = 5;
            break;

        case MUSIC_TYPE_SEL_MYFAVORITE:
            CurNum = 6;
            break;

        case SORT_TYPE_PLAY_LIST:
        case MEDIA_SORT_UPDATE_SEL:
        case MEDIA_MUSIC_BREAKPOINT:
#ifdef _RECORD_
        case MUSIC_TYPE_SEL_RECORDFILE:
        case MUSIC_TYPE_SEL_FMFILE:
        case MUSIC_TYPE_SEL_RECORDFILE_DEL:
#endif
            break;
    }

    return CurNum;
}

_ATTR_MEDIALIBWIN_INIT_CODE_
void MedialibWinValueInit(void *pArg)
{
    MedialibWinItemData.TotalItem  =  MAX_TOTLE_ITEM_NUM ;
    gMusicTypeSelID            = MedialibWinGetCurNum(((MEDIAWIN_WIN_ARG*)pArg)->CurId);
    MedialibWinItemData.CurNum = MedialibWinGetCurNum(((MEDIAWIN_WIN_ARG*)pArg)->CurId);
    MedialibWinItemData.CurStartIndex = ((MedialibWinItemData.CurNum) / MAX_ITEM_NUM_MEDIA_DISP) * MAX_ITEM_NUM_MEDIA_DISP ;

    if (MedialibWinItemData.CurStartIndex > MedialibWinItemData.CurNum )
    {
        MedialibWinItemData.CurStartIndex = MedialibWinItemData.CurStartIndex - MAX_ITEM_NUM_MEDIA_DISP;
    }

#ifndef _TEST_NOWPLAYING_

    if ((FALSE == ThreadCheck(pMainThread, &MusicThread)))
    {
        if (((MEDIAWIN_WIN_ARG*)pArg)->CurId != SORT_TYPE_SEL_FOLDER)  //ESC from BrowserUI
        {
            gMusicTypeSelID            = MedialibWinGetCurNum(((MEDIAWIN_WIN_ARG*)pArg)->CurId + 1);
            MedialibWinItemData.CurNum = MedialibWinGetCurNum(((MEDIAWIN_WIN_ARG*)pArg)->CurId + 1);
        }

        MedialibWinItemData.CurStartIndex = ((MedialibWinItemData.CurNum) / MAX_ITEM_NUM_MEDIA_DISP) * MAX_ITEM_NUM_MEDIA_DISP + 1;

        if (MedialibWinItemData.CurStartIndex > MedialibWinItemData.CurNum )
        {
            MedialibWinItemData.CurStartIndex = MedialibWinItemData.CurStartIndex - MAX_ITEM_NUM_MEDIA_DISP;
        }
    }

#endif
    MedialibWinItemData.CurPointer = MedialibWinItemData.CurNum - MedialibWinItemData.CurStartIndex;

    if (MedialibWinItemData.TotalItem - MedialibWinItemData.CurStartIndex > MAX_ITEM_NUM_MEDIA_DISP)
    {
        MedialibWinItemData.TotalDispItem = MAX_ITEM_NUM_MEDIA_DISP;
    }
    else
    {
        MedialibWinItemData.TotalDispItem = MedialibWinItemData.TotalItem - MedialibWinItemData.CurStartIndex;
    }

    MedialibWinItemData.PrePointer = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void MedialibWinInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIALIBWIN_CODE_
void MedialibWinInit(void *pArg)
{
    /*
    if (gSysConfig.MedialibPara.MediaUpdataFlag)//��ý�����±��ʱ������ý���ʱ����ý���
    {
        gSysConfig.MedialibPara.MediaUpdataFlag = 0;
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))        //ý������ʱ�رȺ�̨����
        {
            ThreadDeleteAll(&pMainThread);
        }

        HoldOnPlayInfo.HoldMusicGetSign =0;
        UpdateMediaLibray();
    }
    */

#if 0
#ifdef _SDCARD_
    if (gSysConfig.Memory == CARD)// when playing music from sd card and enter media library window, switch to flash
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            ThreadDelete(&pMainThread, &MusicThread);

#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif
            //HoldOnPlayInfo.HoldMusicGetSign =0;
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
        }

        gSysConfig.Memory = FLASH0;
        FileSysSetup( gSysConfig.Memory);
        SDCardEnable();
    }
#endif

#ifdef _USB_HOST_
    if (gSysConfig.Memory == USBHOST_FLASH)
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            ThreadDelete(&pMainThread, &MusicThread);
#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif
            //HoldOnPlayInfo.HoldMusicGetSign =0;
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
        }

        gSysConfig.Memory = FLASH0;
        FileSysSetup( gSysConfig.Memory);
    }
#endif
#endif

    MedialibWinValueInit(pArg);
    MedialibWinMsgInit();
    KeyReset();

    if (((MEDIAWIN_WIN_ARG*)pArg)->CurId == MEDIA_MUSIC_BREAKPOINT)
    {
        SendMsg(MSG_MEDIALIB_TO_BREAKPOINT);
    }
    else
    {
        SendMsg(MSG_MEDIALIB_SCROLL_PAINT);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void MedialibWinDeInit(void)
  Author        : ZHengYongzhi
  Description

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIALIBWIN_CODE_
void MedialibWinDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void MedialibSwitchFs(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo      2008/07/21        Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIALIBWIN_CODE_
void MedialibSwitchFs(void)
{
#ifdef _SDCARD_
    if (gSysConfig.Memory == CARD)// when playing music from sd card and enter media library window, switch to flash
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            ThreadDelete(&pMainThread, &MusicThread);

#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif
            //HoldOnPlayInfo.HoldMusicGetSign =0;
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
        }

        gSysConfig.Memory = FLASH0;
        FileSysSetup( gSysConfig.Memory);
        SDCardEnable();
    }
#endif

#ifdef _USB_HOST_
    if (gSysConfig.Memory == USBHOST_FLASH)
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            ThreadDelete(&pMainThread, &MusicThread);
#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif
            //HoldOnPlayInfo.HoldMusicGetSign =0;
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
        }

        gSysConfig.Memory = FLASH0;
        FileSysSetup( gSysConfig.Memory);
    }
#endif
}


/*
--------------------------------------------------------------------------------
  Function name : UINT32 MedialibWinKey(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIALIBWIN_SERVICE_CODE_
UINT32 SortedMusicMode(UINT16 MusicTypeSelID)
{
    UINT32 RetVal;
    TASK_ARG TaskArg;
    uint32 FreqBak;
    UINT8  pPathBuffer[51];
    UINT16 i;
    RetVal = 0;

    switch (MusicTypeSelID)
    {
        case SORT_TYPE_PLAY_LIST:
            gMusicTypeSelID = MusicTypeSelID;
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
            TaskArg.Message.HoldTime  = 2;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            WinCreat(&MedialibWin, &MessageBoxWin, &TaskArg);
            break;

        case SORT_TYPE_SEL_NOW_PLAY:
#ifdef _MUSIC_
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                gMusicTypeSelID = AudioFileInfo.ucSelPlayType;

                if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER
                    || AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_BROWSER)
                {
                    BroswerFlag = FALSE;
                }

                TaskArg.Music.FileNum  = AudioFileInfo.CurrentFileNum;
                if( gMusicTypeSelID > MAX_TOTLE_ITEM_NUM)
                {
                    gMusicTypeSelID -= 3;
                }
                TaskArg.Music.MediaTitleAdd = MediaItem[gMusicTypeSelID][1];

                TaskSwitch(TASK_ID_MUSIC, &TaskArg);
                SendMsg(MSG_MEDIA_NOW_PLAY);
                return 1;
            }

#endif
            break;

        case SORT_TYPE_SEL_FILENAME :
        case SORT_TYPE_SEL_ID3TITLE:
        case SORT_TYPE_SEL_ID3SINGER :
        case SORT_TYPE_SEL_ID3ALBUM :
        case SORT_TYPE_SEL_GENRE :
            MedialibSwitchFs();
            gMusicTypeSelID = MusicTypeSelID;

            if (gSysConfig.MedialibPara.gMusicFileNum)
            {
                TaskArg.MediaBro.CurId = 0;
                TaskArg.MediaBro.TitleAdd = MediaItem[gMusicTypeSelID][1];
                TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
                RetVal = 1;
            }
            else//display dialog if no music file
            {
                TaskArg.Message.TitleID   = SID_WARNING;
                TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
                TaskArg.Message.HoldTime  = 2;
                TaskArg.Message.CurDisFrameIndex = 0;
                TaskArg.Message.UnsupportFrameIndex = 1;
                WinCreat(&MedialibWin, &MessageBoxWin, &TaskArg);
            }

            break;

        case MUSIC_TYPE_SEL_MYFAVORITE :
            gMusicTypeSelID = MusicTypeSelID;
            TaskArg.MediaBro.TitleAdd = MediaItem[6][1];

            if (gSysConfig.MedialibPara.gMyFavoriteFileNum)
            {
                TaskArg.MediaBro.CurId = 0;
                TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
                RetVal = 1;
            }
            else
            {
                TaskArg.Message.TitleID   = SID_WARNING;
                TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
                TaskArg.Message.HoldTime  = 2;
                TaskArg.Message.CurDisFrameIndex = 0;
                TaskArg.Message.UnsupportFrameIndex = 1;
                WinCreat(&MedialibWin, &MessageBoxWin, &TaskArg);
            }

            break;
#ifdef _RECORD_

        case MUSIC_TYPE_SEL_RECORDFILE:
            gMusicTypeSelID = MusicTypeSelID;
            TaskArg.MediaBro.TitleAdd = MediaItem[gMusicTypeSelID][1];

//            ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);

            if (GetTotalFiles(GetDirClusIndex("U:\\RECORD     \\VOICE"), RecordFileExtString, FS_FAT))
            {
                TaskArg.MediaBro.CurId = 0;
                TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
                RetVal = 1;
            }
            else
            {
                TaskArg.Message.TitleID   = SID_WARNING;
                TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
                TaskArg.Message.HoldTime  = 2;
                TaskArg.Message.CurDisFrameIndex = 0;
                TaskArg.Message.UnsupportFrameIndex = 1;
                WinCreat(&MedialibWin, &MessageBoxWin, &TaskArg);
            }

            break;

        case MUSIC_TYPE_SEL_FMFILE:
            gMusicTypeSelID = MusicTypeSelID;
            TaskArg.MediaBro.TitleAdd = MediaItem[gMusicTypeSelID][1];

            if (gSysConfig.MedialibPara.gRecordFmFileNum)
            {
                TaskArg.MediaBro.CurId = 0;
                TaskSwitch(TASK_ID_MEDIABRO, &TaskArg);
                RetVal = 1;
            }
            else
            {
                TaskArg.Message.TitleID   = SID_WARNING;
                TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
                TaskArg.Message.HoldTime  = 2;
                TaskArg.Message.CurDisFrameIndex = 0;
                TaskArg.Message.UnsupportFrameIndex = 1;
                WinCreat(&MedialibWin, &MessageBoxWin, &TaskArg);
            }

            break;
#endif

        case MEDIA_SORT_UPDATE_SEL :
#ifdef _MUSIC_
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }

#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif

            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif
#ifdef _MEDIA_MODULE_
            /*
                    UpdateMediaLibray();

                    MedialibWinItemData.CurStartIndex = ((MedialibWinItemData.CurNum) / MAX_ITEM_NUM_MEDIA_DISP) * MAX_ITEM_NUM_MEDIA_DISP +1;
                    if (MedialibWinItemData.CurStartIndex > MedialibWinItemData.CurNum )
                    {
                        MedialibWinItemData.CurStartIndex = MedialibWinItemData.CurStartIndex - MAX_ITEM_NUM_MEDIA_DISP;
                    }

                    MedialibWinItemData.CurPointer = MedialibWinItemData.CurNum - MedialibWinItemData.CurStartIndex;

                    SendMsg(MSG_MEDIAWIN_DISPLAY_ALL);
                */
            gSysConfig.MedialibPara.MediaUpdataFlag = 1;
            TaskArg.Mdb.TaskID = TASK_ID_MEDIALIB;
            TaskSwitch(TASK_ID_MDB, &TaskArg);
#endif
            return 1;

        case SORT_TYPE_SEL_FOLDER:
            gMusicTypeSelID = MusicTypeSelID;
            TaskArg.Browser.FileType = FileTypeAudio;
            TaskArg.Browser.FileNum  = 0;
            TaskArg.Browser.FromWhere = 1;
            BroswerFlag = TRUE;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            RetVal = 1;
            #if 0
            if (gSysConfig.MedialibPara.gMusicFileNum != 0)
            {
                gMusicTypeSelID = MusicTypeSelID;
                TaskArg.Browser.FileType = FileTypeAudio;
                TaskArg.Browser.FileNum  = 0;
                TaskArg.Browser.FromWhere = 1;
                TaskSwitch(TASK_ID_BROWSER, &TaskArg);
                BroswerFlag = FALSE;
                RetVal = 1;
            }
            else
            {
                TaskArg.Message.TitleID   = SID_WARNING;
                TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
                TaskArg.Message.HoldTime  = 2;
                TaskArg.Message.CurDisFrameIndex = 0;
                TaskArg.Message.UnsupportFrameIndex = 1;
                WinCreat(&MedialibWin, &MessageBoxWin, &TaskArg);
            }
            #endif
            break;

        case MEDIA_MUSIC_BREAKPOINT:
#ifdef AUDIOHOLDONPLAY  //4
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
                TaskArg.Music.MediaTitleAdd = MediaItem[gMusicTypeSelID][1];
                TaskSwitch(TASK_ID_MUSIC, &TaskArg);
                SendMsg(MSG_MEDIA_BREAKPOINT_PLAY);
                return 1;
            }

#endif
            break;

        default:
            break;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MedialibWinService(void)
  Author        : anzhiguo
  Description   : it is media service handle that is used to handle window message,window service task etc.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             anzhiguo     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIALIBWIN_SERVICE_CODE_
UINT32 MedialibWinService(void)
{
    TASK_ARG TaskArg;

    if (GetMsg( MSG_MEDIALIB_TO_BREAKPOINT))
    {
        SortedMusicMode((UINT16)MEDIA_MUSIC_BREAKPOINT);
        return 1;
    }

    if (GetMsg( MSG_MESSAGEBOX_DESTROY))
    {
        SendMsg(MSG_NEED_PAINT_ALL);
    }

    return (RETURN_OK);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MedialibWinKey(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIALIBWIN_SERVICE_CODE_
UINT32 MedialibWinKey(void)
{
    uint32 RetVal;
    UINT32 MedialibKeyVal;
    TASK_ARG TaskArg;
    RetVal = RETURN_OK;
    MedialibKeyVal =  GetKeyVal();

    //5 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (MedialibKeyVal)
        {
            case KEY_VAL_FFW_DOWN:
            case KEY_VAL_FFW_PRESS:
                MedialibKeyVal = KEY_VAL_UP_DOWN;
                break;

            case KEY_VAL_FFD_DOWN:
            case KEY_VAL_FFD_PRESS:
                MedialibKeyVal = KEY_VAL_DOWN_DOWN;
                break;

            default:
                break;
        }
    }

    //6 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
        switch (MedialibKeyVal)
        {
            case KEY_VAL_FFW_SHORT_UP:
                MedialibKeyVal = KEY_VAL_ESC_SHORT_UP;
                break;

            default:
                break;
        }
    }

    switch (MedialibKeyVal)
    {
        case KEY_VAL_ESC_PRESS_START://long press exit to main interface.
        case KEY_VAL_ESC_SHORT_UP:    //exit to main interface
            TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            RetVal = 1;
            break;

        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_SHORT_UP:
            MedialibWinItemData.PrePointer = MedialibWinItemData.CurPointer;

#ifdef _TEST_NOWPLAYING_
            if (0)
#else
            if (FALSE == ThreadCheck(pMainThread, &MusicThread))//&&(gSysConfig.MusicConfig.HoldOnPlaySaveFlag==0)&&(HoldOnPlayInfo.HoldMusicGetSign==0))//�޺�̨���ִ���
#endif
            {
                if (MedialibWinItemData.CurNum == 1) //cursor in the frist item
                {
                    MedialibWinItemData.CurNum = MedialibWinItemData.TotalItem - 1; //the index of cursor set the max

                    if (MedialibWinItemData.TotalItem > MAX_ITEM_NUM_MEDIA_DISP)
                    {
                        MedialibWinItemData.CurStartIndex = ((MedialibWinItemData.TotalItem) / MAX_ITEM_NUM_MEDIA_DISP) * MAX_ITEM_NUM_MEDIA_DISP + 1;

                        if (MedialibWinItemData.CurStartIndex > MedialibWinItemData.CurNum)// - 1)
                        {
                            MedialibWinItemData.CurStartIndex = MedialibWinItemData.CurStartIndex - MAX_ITEM_NUM_MEDIA_DISP;
                        }

                        if ((MedialibWinItemData.CurNum - MedialibWinItemData.CurStartIndex ) > MAX_ITEM_NUM_MEDIA_DISP)
                        {
                            MedialibWinItemData.TotalDispItem = MAX_ITEM_NUM_MEDIA_DISP;
                        }
                        else
                        {
                            MedialibWinItemData.TotalDispItem = MedialibWinItemData.TotalItem - MedialibWinItemData.CurStartIndex;
                        }

                        SendMsg(MSG_MEDIAWIN_ALL_ITEM);//full screen refresh.
                    }
                    else
                    {
                        SendMsg(MSG_MEDIAWIN_FRESH_ITEM);
                    }

                    MedialibWinItemData.CurPointer = MedialibWinItemData.TotalDispItem - 1; //point to the last item that can display
                }
                else
                {
                    if (MedialibWinItemData.CurPointer == 0)
                    {
                        MedialibWinItemData.CurPointer = MAX_ITEM_NUM_MEDIA_DISP - 1;//cursor point to end of screen
                        MedialibWinItemData.CurStartIndex -= MAX_ITEM_NUM_MEDIA_DISP;
                        MedialibWinItemData.TotalDispItem = MAX_ITEM_NUM_MEDIA_DISP;
                        SendMsg(MSG_MEDIAWIN_ALL_ITEM);//full screen refresh.
                    }
                    else
                    {
                        MedialibWinItemData.CurPointer --;
                        SendMsg(MSG_MEDIAWIN_FRESH_ITEM);
                    }

                    MedialibWinItemData.CurNum --;
                }
            }
            else
            {
                if (MedialibWinItemData.CurNum == 0) //cursor in the frist item
                {
                    MedialibWinItemData.CurNum = MedialibWinItemData.TotalItem - 1; //the index of cursor set the max

                    if (MedialibWinItemData.TotalItem > MAX_ITEM_NUM_MEDIA_DISP)
                    {
                        MedialibWinItemData.CurStartIndex = ((MedialibWinItemData.TotalItem) / MAX_ITEM_NUM_MEDIA_DISP) * MAX_ITEM_NUM_MEDIA_DISP ;

                        if (MedialibWinItemData.CurStartIndex > MedialibWinItemData.CurNum)
                        {
                            MedialibWinItemData.CurStartIndex = MedialibWinItemData.CurStartIndex - MAX_ITEM_NUM_MEDIA_DISP;
                        }

                        if ((MedialibWinItemData.CurNum - MedialibWinItemData.CurStartIndex) > MAX_ITEM_NUM_MEDIA_DISP)
                        {
                            MedialibWinItemData.TotalDispItem = MAX_ITEM_NUM_MEDIA_DISP;
                        }
                        else
                        {
                            MedialibWinItemData.TotalDispItem = MedialibWinItemData.TotalItem - MedialibWinItemData.CurStartIndex;
                        }

                        SendMsg(MSG_MEDIAWIN_ALL_ITEM);//full screen refresh.
                    }
                    else
                    {
                        SendMsg(MSG_MEDIAWIN_FRESH_ITEM);
                    }

                    MedialibWinItemData.CurPointer = MedialibWinItemData.TotalDispItem - 1; //point to the last item that can display
                }
                else
                {
                    if (MedialibWinItemData.CurPointer == 0)
                    {
                        MedialibWinItemData.CurPointer = MAX_ITEM_NUM_MEDIA_DISP - 1;///cursor point to end of screen
                        MedialibWinItemData.CurStartIndex -= MAX_ITEM_NUM_MEDIA_DISP;
                        MedialibWinItemData.TotalDispItem = MAX_ITEM_NUM_MEDIA_DISP;
                        SendMsg(MSG_MEDIAWIN_ALL_ITEM);//full screen refresh.
                    }
                    else
                    {
                        MedialibWinItemData.CurPointer --;
                        SendMsg(MSG_MEDIAWIN_FRESH_ITEM);
                    }

                    MedialibWinItemData.CurNum --;
                }
            }

            SendMsg(MSG_MEDIALIB_SCROLL_PAINT);
            break;

        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
            MedialibWinItemData.PrePointer = MedialibWinItemData.CurPointer;

            if (MedialibWinItemData.CurNum == (MedialibWinItemData.TotalItem - 1)) //point to the last item
            {
                MedialibWinItemData.CurPointer = 0;//cursor point to start of screen
#ifdef _TEST_NOWPLAYING_

                if (0)
#else
                if (FALSE == ThreadCheck(pMainThread, &MusicThread))//&&(gSysConfig.MusicConfig.HoldOnPlaySaveFlag==0)&&(HoldOnPlayInfo.HoldMusicGetSign==0))//�޺�̨���ִ���
#endif
                {
                    MedialibWinItemData.CurNum = 1;
                    MedialibWinItemData.CurStartIndex = 1;
                }
                else
                {
                    MedialibWinItemData.CurNum = 0;
                    MedialibWinItemData.CurStartIndex = 0;
                }

                if (MedialibWinItemData.TotalItem > MAX_ITEM_NUM_MEDIA_DISP)
                {
                    MedialibWinItemData.TotalDispItem = MAX_ITEM_NUM_MEDIA_DISP;
                    SendMsg(MSG_MEDIAWIN_ALL_ITEM);///full screen refresh.
                }
                else
                {
                    SendMsg(MSG_MEDIAWIN_FRESH_ITEM);
                }
            }
            else
            {
                if (MedialibWinItemData.CurPointer == (MedialibWinItemData.TotalDispItem - 1))//cursor point to end of screen
                {
                    MedialibWinItemData.CurPointer = 0;
                    MedialibWinItemData.CurStartIndex += MAX_ITEM_NUM_MEDIA_DISP ;

                    if ((MedialibWinItemData.CurStartIndex + MAX_ITEM_NUM_MEDIA_DISP - 1) < MedialibWinItemData.TotalItem)
                    {
                        MedialibWinItemData.TotalDispItem = MAX_ITEM_NUM_MEDIA_DISP;
                    }
                    else
                    {
                        MedialibWinItemData.TotalDispItem = MedialibWinItemData.TotalItem - MedialibWinItemData.CurStartIndex;
                    }

                    SendMsg(MSG_MEDIAWIN_ALL_ITEM);
                }
                else
                {
                    MedialibWinItemData.CurPointer++;
                    SendMsg(MSG_MEDIAWIN_FRESH_ITEM);
                }

                MedialibWinItemData.CurNum++;
            }

            SendMsg(MSG_MEDIALIB_SCROLL_PAINT);
            break;

        case KEY_VAL_MENU_SHORT_UP:
            RetVal = SortedMusicMode((UINT16)MediaItem[MedialibWinItemData.CurNum][2]); //enter the specific favourite
            break;

        case KEY_VAL_HOLD_ON://8.4 azg
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&MedialibWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF://8.4 azg
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&MedialibWin, &HoldWin, &TaskArg);
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
_ATTR_MEDIALIBWIN_SERVICE_CODE_
void MedialibWinDisplay(void)
{
    UINT16  i, CurNum;
    UINT16  TempColor, TempCharSize, TextMode;
    UINT16  TotalItem;
    UINT16  *printfbuf;
    UINT32   x, y;
    LCD_RECT r;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;

    TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_MEDIAWIN_DISPLAY_ALL)))
    {
        //display backgroud image
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);
        // display title
        DisplayMenuStrWithIDNum(MEDIALIB_TITLE_TXT_X, MEDIALIB_TITLE_TXT_Y, MEDIALIB_TITLE_TXT_XSIZE,
                                MEDIALIB_TITLE_TXT_YSIZE, LCD_TEXTALIGN_CENTER, SID_MEDIALIB );
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + BrowserBatteryLevel, 105, 146);
        SendMsg(MSG_MEDIAWIN_ALL_ITEM);
    }

    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + BrowserBatteryLevel, 105, 146);
    }

    if (GetMsg(MSG_MEDIAWIN_ALL_ITEM))
    {
        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, 3, 143);
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_NOWPLAY, &PicInfo1);//the icon of every low
        TotalItem = MedialibWinItemData.TotalItem - MedialibWinItemData.CurStartIndex;

        for (i = 0; ((i < MAX_ITEM_NUM_MEDIA_DISP) && (i < TotalItem )); i++)
        {
            r.x0 = 20;
            r.y0 = i * 17 + 3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0 + 17; //r.y0 + CH_CHAR_XSIZE_12 - 1;
            DispPictureWithIDNumAndXY(MediaItem[MedialibWinItemData.CurStartIndex + i][0],
                                      PicInfo1.x, PicInfo.y + 5 + 17 * i);

            if (i == MedialibWinItemData.CurPointer)
            {
                DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON, 19, r.y0);
            }

            r.y0 += 2;
            r.y1 =  r.y0 + CH_CHAR_XSIZE_12 - 1;
            DisplayMenuStrWithIDNum(MEDIA_ITEM_X, (MEDIA_ITEM_Y + (MEDIA_ITEM_YSIZE * i)), PicInfo.xSize, MEDIA_ITEM_CHARYSIZE,
                                    LCD_TEXTALIGN_LEFT, MediaItem[MedialibWinItemData.CurStartIndex + i][1]);
            SendMsg(MSG_MEDIALIB_SCROLL_PAINT);
        }
    }

    if (GetMsg(MSG_MEDIAWIN_FRESH_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_NOWPLAY, &PicInfo1);//ÿ�е�ͼ��

        for (i = 0; i < MAX_ITEM_NUM_MEDIA_DISP; i++)
        {
            if ((i == MedialibWinItemData.PrePointer) || (i == MedialibWinItemData.CurPointer))
            {
                r.x0 = 20;
                r.y0 = i * 17 + 3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
                r.x1 = r.x0 + PicInfo.xSize;
                r.y1 = r.y0 + 17; //r.y0 + CH_CHAR_XSIZE_12 - 1;

                if (i == MedialibWinItemData.PrePointer)
                {
                    DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, r.y0, 17);
                }

                if (i == MedialibWinItemData.CurPointer)
                {
                    DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON, 19, r.y0);
                }

                DispPictureWithIDNumAndXY(MediaItem[MedialibWinItemData.CurStartIndex + i][0],
                                          PicInfo1.x, PicInfo.y + 5 + 17 * i);
                r.y0 += 2;
                r.y1 =  r.y0 + CH_CHAR_XSIZE_12 - 1;
                DisplayMenuStrWithIDNum(MEDIA_ITEM_X, (MEDIA_ITEM_Y + (MEDIA_ITEM_YSIZE * i)), PicInfo.xSize, MEDIA_ITEM_CHARYSIZE,
                                        LCD_TEXTALIGN_LEFT, MediaItem[MedialibWinItemData.CurStartIndex + i][1]);
                SendMsg(MSG_MEDIALIB_SCROLL_PAINT);
            }
        }
    }

    if (GetMsg(MSG_MEDIALIB_SCROLL_PAINT))
    {
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL_BLOCK, &PicInfo1);

        if (TRUE == ThreadCheck(pMainThread, &MusicThread))//if it is not in backgroud play,paly the frist item of all music
        {
            if ( MedialibWinItemData.CurNum > 0)
            {
                i = (PicInfo.ySize - PicInfo1.ySize) *  MedialibWinItemData.CurNum / (  MedialibWinItemData.TotalItem - 1);
            }
            else
            {
                i = 0;
            }
        }
        else
        {
            if ( MedialibWinItemData.CurNum > 1)
            {
                i = (PicInfo.ySize - PicInfo1.ySize) *  (MedialibWinItemData.CurNum - 1) / (  MedialibWinItemData.TotalItem - 2);
            }
            else
            {
                i = 0;
            }
        }

        if ((i + PicInfo1.ySize) > PicInfo.ySize)
        {
            i = PicInfo.ySize - PicInfo1.ySize;
        }

        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_SCOLL, 122, 0);
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_SCOLL_BLOCK, 0 + 123, i);
    }

    LCD_SetCharSize(TempCharSize);
    LCD_SetTextMode(TextMode);
}

#endif



/*
********************************************************************************
*
*                         End of MainMenu.c
*
********************************************************************************
*/


