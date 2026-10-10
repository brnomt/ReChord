/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  MusicWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               zhangshuai      2009-02-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_VIDEOWIN_

#include "SysInclude.h"

#ifdef _VIDEO_

#include "mainmenu.h"
#include "VideoControl.h"
#include "BrowserUI.h"
#include "MessageBox.h"
#include "SysFindFile.h"
#include "VideoWin.h"
#include "Hold.h"
#include "AviFile.h"

#include "xvid_dec_main.h"

#define VideoCurrFileNum                VideoFileInfo.CurrentFileNum
#define VideoTotalFileNum               VideoFileInfo.TotalFiles
#define VideoBatteryLevel               gBattery.Batt_Level
#define VidelHoldState					HoldState
#define VIDEO_TIME_GUAGE_CONST          (121-7)            //total item number of progress

extern UINT8 VideoDisplayTime;
extern	UINT32 VideoPlayErrorNum;
extern bool VideoUiFlag;
extern UINT32 VideoPlayFileNum;
extern bool VedioDirection;

_ATTR_VIDEOWIN_BSS_ UINT32  VideoTimeGuageVal;
/*
--------------------------------------------------------------------------------
  Function name : void VideoWinMsgInit(void)
  Author        : ZHengYongzhi
  Description   : main menu message initial function,it go to initial setting to the message parameter of main menu will use.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VIDEOWIN_CODE_
void VideoWinMsgInit(void)
{
    SendMsg(MSG_XVID_DISPLAY_ALL);
    ClearMsg(MSG_VIDEO_CARD_CHECK);
    // SendMsg(MSG_VIDEO_DISPFLAG_FILENUM);
}

/*
--------------------------------------------------------------------------------
  Function name : void VideoWinInit(void *pArg)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VIDEOWIN_CODE_
void VideoWinInit(void *pArg)
{
    UINT16  ColorBk;

    VedioDirection = 0;
    VideoTimeSecBack = 0XFFFFFFFF;
    VideoPlayErrorNum =0;
    VideoPlayFileNum = 0;

#ifndef _FRAME_BUFFER_
    MP4_LCD_Init();
#endif

    LCD_SetDiaplayMode(LCD_MODE_90);

    KeyReset();

    GetPictureInfoForBuffer(IMG_ID_FM_FREGUAGE, &VideoSchedPicInfo);
    GetPictureResourceToBuffer(IMG_ID_FM_FREGUAGE, VideoSchedBuffer, VIDEO_SCHED_BUF_SIZE);
    GetPictureResourceToBuffer(IMG_ID_FM_FREGUAGE + 1, (uint8*)(&VideoSchedBuffer[0] + VIDEO_SCHED_BUF_SIZE), VIDEO_SCHED_BUF_SIZE);

    BLOffDisable();
    LcdStandbyDisable();
    AutoPowerOffDisable();

    VideoWinMsgInit();
    VideoWinSvcStart(pArg);
}

/*
--------------------------------------------------------------------------------
  Function name : void MusicWinDeInit(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VIDEOWIN_CODE_
void VideoWinDeInit(void)
{
    //  ModuleOverlay(MODULE_ID_VIDEO, MODULE_OVERLAY_ALL);

    ClearMsg(MSG_VIDEO_VERTICAL_DISPLAY);

    LCD_SetDiaplayMode(LCD_MODE_0);

    #ifndef _FRAME_BUFFER_
    MP4_LCD_DeInit();
    #endif

    LcdStandbyEnable();
    BLOffEnable();
    AutoPowerOffEnable();
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 VideoWinService(void)
  Author        : ZHengYongzhi
  Description   : main menu service progarm

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VIDEOWIN_CODE_
UINT32 VideoWinService(void)
{
    UINT32 Retval = 0;
    TASK_ARG TaskArg;
    INT32 NextFile;

    if ( TRUE == GetMsg(MSG_VIDEO_EXIT_BROWSER))
    {
        ThreadDelete(&pMainThread, &VideoThread);
        if (CheckMsg(MSG_BROW_FROM_MAINMENU))
        {
            TaskArg.Browser.FileType = FileTypeALL;
            TaskArg.Browser.FileNum  = 0;
        }
        else
        {
            TaskArg.Browser.FileType = FileTypeVideo;
            TaskArg.Browser.FileNum  = gbVideoFileNum;
        }
        TaskArg.Browser.FromWhere = 0;
        TaskSwitch(TASK_ID_BROWSER,&TaskArg);
        Retval = 1;
    }
    if (TRUE == GetMsg(MSG_VIDEO_CARD_CHECK))
    {
        ThreadDelete(&pMainThread, &VideoThread);

        TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
        Retval = 1;
    }

    if (TRUE == GetMsg(MSG_XVID_NOFILE))
    {
        ThreadDelete(&pMainThread, &VideoThread);
        VideoWinDialogType = VIDEOWIN_DIALOG_NOFILE;
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        SendMsg(MSG_VIDEO_VERTICAL_DISPLAY);
        WinCreat(&VideoWin, &MessageBoxWin, &TaskArg);
    }

    if (TRUE == GetMsg(MSG_VIDEO_FILENOTSUPPORT))
    {
        VideoWinDialogType = VIDEOWIN_DIALOG_FORMAT_ERROR;
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_FILE_FORMAT_ERROR;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        SendMsg(MSG_VIDEO_VERTICAL_DISPLAY);
        WinCreat(&VideoWin, &MessageBoxWin, &TaskArg);
    }

    if (TRUE == GetMsg(MSG_MESSAGEBOX_DESTROY))
    {
#ifdef FILE_ERROR_DIALOG
        VideoPlayErrorNum++;
        if (VideoPlayErrorNum == VideoFileInfo.TotalFiles)
        {
            ThreadDelete(&pMainThread, &VideoThread);// by zs 06.09
            if (CheckMsg(MSG_BROW_FROM_MAINMENU))
            {
                TaskArg.Browser.FileType = FileTypeALL;
                TaskArg.Browser.FileNum  = 0;
            }
            else
            {
                TaskArg.Browser.FileType = FileTypeVideo;
                TaskArg.Browser.FileNum  = gbVideoFileNum;
            }
            TaskArg.Browser.FromWhere = 0;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg); //tiantian
            return  1;
        }

        if (VedioDirection)
        {
            VideoDisplayTime = VIDEOUITIME;
            VideoUiFlag = 0;
            NextFile = -1;
            VideoControlProc(MSG_XVID_NEXTFILE, (void*)NextFile);
            SendMsg(MSG_XVID_DISPLAY_ALL);
        }
        else
        {
            VideoDisplayTime = VIDEOUITIME;
            VideoUiFlag = 0;
            NextFile = 1;
            VideoControlProc(MSG_XVID_NEXTFILE, (void*)NextFile);
            SendMsg(MSG_XVID_DISPLAY_ALL);
        }
#else
        ThreadDelete(&pMainThread, &VideoThread);// by zs 06.09

        if (CheckMsg(MSG_BROW_FROM_MAINMENU))
        {
            TaskArg.Browser.FileType = FileTypeALL;
            TaskArg.Browser.FileNum  = 0;
        }
        else
        {
            TaskArg.Browser.FileType = FileTypeVideo;
            TaskArg.Browser.FileNum  = gbVideoFileNum;
        }
        TaskArg.Browser.FromWhere = 0;
        TaskSwitch(TASK_ID_BROWSER, &TaskArg); //tiantian
        Retval = 1;
#endif
    }

    //UI display message
    if ((VideoPlayState == VIDEO_STATE_PLAY)&&(VideoDisplayTime))
    {
        //SendMsg(MSG_XVID_DISPLAY_ALL);
        SendMsg(MSG_VIDEO_DISPFLAG_SCHED);
        SendMsg(MSG_VIDEO_DISPFLAG_CURRENT_TIME);
        SendMsg(MSG_XVID_DISPFLAG_STATUS);
    }
    if ((CheckMsg(MSG_BATTERY_UPDATE))&&(VideoDisplayTime))
    {
        SendMsg(MSG_VIDEO_DISPFLAG_BATTERY);
    }
    return (Retval);
}


/*
--------------------------------------------------------------------------------
  Function name :  UINT32 VideoWinKeyProc(void)
  Description   :

  Input         : NULL
  Return        : TRUE\FALSE

  History:     <author>         <time>         <version>
             zhangshuai     2009/02/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VIDEOWIN_CODE_
UINT32 VideoWinKeyProc(void)
{
    UINT32 VideoKeyVal;
    UINT32 RetVal = RETURN_OK;
    INT32 NextFile;
    TASK_ARG TaskArg;

    VideoKeyVal =  GetKeyVal();

    //5 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (VideoKeyVal)
        {
            case KEY_VAL_ESC_PRESS_START:           //long key switch VOL function�����л�VOL����
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

                    VideoDisplayTime = VIDEOUITIME;
                    if (!VideoUiFlag)
                    {
                        SendMsg(MSG_XVID_DISPLAY_ALL);
                    }
                }
                break;

            case KEY_VAL_FFD_DOWN:
            case KEY_VAL_FFD_PRESS:
                if (CheckMsg(MSG_KEY_VOL_STATE))
                {
                    VideoKeyVal = KEY_VAL_UP_DOWN;
                }
                break;

            case KEY_VAL_FFW_DOWN:
            case KEY_VAL_FFW_PRESS:
                if (CheckMsg(MSG_KEY_VOL_STATE))
                {
                    VideoKeyVal = KEY_VAL_DOWN_DOWN;
                }
                break;

            case KEY_VAL_FFD_SHORT_UP:
            case KEY_VAL_FFD_LONG_UP:
            case KEY_VAL_FFW_SHORT_UP:
            case KEY_VAL_FFW_LONG_UP:
                if (CheckMsg(MSG_KEY_VOL_STATE))
                {
                    VideoKeyVal = KEY_VAL_NONE;
                }
                break;

            default:
                break;
        }
    }

    //6 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
    }

    //7 key function
    switch (VideoKeyVal)
    {
        case KEY_VAL_MENU_SHORT_UP:
        case KEY_VAL_ESC_SHORT_UP:
#if 1
            ThreadDelete(&pMainThread, &VideoThread);
            if (CheckMsg(MSG_BROW_FROM_MAINMENU))
            {
                TaskArg.Browser.FileType = FileTypeALL;
                TaskArg.Browser.FileNum  = GlobalFilenum;
            }
            else
            {
                TaskArg.Browser.FileType = FileTypeVideo;
                TaskArg.Browser.FileNum  = gbVideoFileNum;
            }
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            RetVal = 1;
            break;
#else
            ThreadDelete(&pMainThread, &VideoThread); //exit to main interface
            TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
            TaskSwitch(TASK_ID_MAINMENU,&TaskArg);
            RetVal = 1;
            break;
#endif
        case KEY_VAL_MENU_PRESS_START:
            ThreadDelete(&pMainThread, &VideoThread);
            TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
            TaskSwitch(TASK_ID_MAINMENU,&TaskArg);
            RetVal = 1;
            break;

        case KEY_VAL_PLAY_SHORT_UP:                             //play/pause key

            VideoDisplayTime = VIDEOUITIME;
            if (VIDEO_STATE_PLAY == VideoPlayState)
            {
                BLOffEnable();
                AutoPowerOffEnable();
                VideoControlProc(MSG_XVID_PAUSE, NULL);
            }
            else //if (VIDEO_STATE_PAUSE == VideoPlayState)
            {
                BLOffDisable();
                AutoPowerOffDisable();
                VideoPlayStateBack = VIDEO_STATE_PLAY;
                VideoControlProc(MSG_XVID_RESUME, NULL);
            }

            if (VideoUiFlag)
                SendMsg(MSG_XVID_DISPFLAG_STATUS);
            else
                SendMsg(MSG_XVID_DISPLAY_ALL);

            break;

        case KEY_VAL_UP_DOWN:
        case KEY_VAL_UP_PRESS :
            VideoDisplayTime = VIDEOUITIME;
            if (VideoOutputVol < MAX_VOLUME)
            {
                VideoOutputVol++;
                VideoControlProc(MSG_XVID_VOLUMESET,VideoOutputVol);
            }
            if (VideoUiFlag)
                SendMsg(MSG_VIDEO_DISPFLAG_VOL);
            else
                SendMsg(MSG_XVID_DISPLAY_ALL);

            break;

        case KEY_VAL_DOWN_DOWN:
        case KEY_VAL_DOWN_PRESS:
            VideoDisplayTime = VIDEOUITIME;
            if (VideoOutputVol)
            {
                VideoOutputVol--;
                VideoControlProc(MSG_XVID_VOLUMESET,VideoOutputVol);
            }
            if (VideoUiFlag)
                SendMsg(MSG_VIDEO_DISPFLAG_VOL);
            else
                SendMsg(MSG_XVID_DISPLAY_ALL);

            break;


        case KEY_VAL_FFD_PRESS:                                 //fast forward
            VideoDisplayTime = VIDEOUITIME;
            VideoControlProc(MSG_XVID_FFD,NULL);
            if (VideoUiFlag)
                SendMsg(MSG_XVID_DISPFLAG_STATUS);
            else
                SendMsg(MSG_XVID_DISPLAY_ALL);
            break;

        case KEY_VAL_FFW_PRESS:                                  //rewind
            VideoDisplayTime = VIDEOUITIME;
            VideoControlProc(MSG_XVID_FFW,NULL);
            if (VideoUiFlag)
                SendMsg(MSG_XVID_DISPFLAG_STATUS);
            else
                SendMsg(MSG_XVID_DISPLAY_ALL);
            break;

        case KEY_VAL_FFD_LONG_UP: // restore
        case KEY_VAL_FFW_LONG_UP:
            VideoDisplayTime = VIDEOUITIME;
            if (VideoPlayStateBack != VIDEO_STATE_PAUSE)
            {
                VideoControlProc(MSG_XVID_RESUME,NULL);
            }
            break;

        case KEY_VAL_FFD_SHORT_UP://next file
            VedioDirection = 0;
            VideoDisplayTime = VIDEOUITIME;
            VideoStopMode = Video_Stop_Force;
            NextFile = 1;
            VideoControlProc(MSG_XVID_NEXTFILE, (void*)NextFile);
            SendMsg(MSG_XVID_DISPLAY_ALL);

            break;

        case KEY_VAL_FFW_SHORT_UP://previous file
            VedioDirection = 1;
            VideoDisplayTime = VIDEOUITIME;
            VideoStopMode = Video_Stop_Force;
            NextFile = -1;
            VideoPlayState = VIDEO_STATE_PLAY;
            VideoPlayStateBack = VideoPlayState;
            VideoControlProc(MSG_XVID_NEXTFILE, (void*)NextFile);
            SendMsg(MSG_XVID_DISPLAY_ALL);
            break;

        case KEY_VAL_HOLD_ON:
            VideoDisplayTime = VIDEOUITIME;
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            SendMsg(MSG_XVID_DISPLAY_ALL);
//            SendMsg(MSG_VIDEO_DISPLAY_HOLD);
            break;

        case KEY_VAL_HOLD_OFF:
            VideoDisplayTime = VIDEOUITIME;
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            SendMsg(MSG_XVID_DISPLAY_ALL);
//			SendMsg(MSG_VIDEO_DISPLAY_UNHOLD);
            break;

        default:
            RetVal = FALSE;
            break;
    }

    return RetVal;
}

_ATTR_VIDEOWIN_CODE_
void VedioDisplayTime(UINT16 Hour,UINT16 Min,UINT16 Sec,UINT16 mode)
{
    UINT16 TxtDrawMode;
    UINT16 DisplayBuf[9];

    DisplayBuf[0] = Hour/10+'0';
    DisplayBuf[1] = Hour%10+'0';
    DisplayBuf[2] = ':';
    DisplayBuf[3] = Min/10+'0';
    DisplayBuf[4] = Min%10+'0';
    DisplayBuf[5] = ':';
    DisplayBuf[6] = Sec/10+'0';
    DisplayBuf[7] = Sec%10+'0';
    DisplayBuf[8] = 0;

    TxtDrawMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    if (mode==0)
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_VEDIO_TIME_BACK,50,2);
        LCD_NFDispStringAt(50, 2, DisplayBuf);
    }
    else
    {
        //DisplayPicture_part(IMG_ID_MIC_BACKGROUND,0,0,108,16);
        LCD_NFDispStringAt(114, 118, DisplayBuf);
    }
    LCD_SetTextMode(TxtDrawMode);
}


_ATTR_VIDEOWIN_CODE_
void VedioDisplayVol(UINT32 VOL)
{
    UINT16 temp;
    UINT16 TxtDrawMode;
    UINT16 DisplayBuf[4];

    DisplayBuf[0] = VOL/10+'0';
    DisplayBuf[1] = VOL%10+'0';
    DisplayBuf[2] = 0;

    TxtDrawMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    LCD_NFDispStringAt(8, 2, DisplayBuf);
    LCD_SetTextMode(TxtDrawMode);

}

/*
--------------------------------------------------------------------------------
  Function name : void VIdeoWinPaint(void)
  Author        : Chenwei
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               Chenwei        2009/03/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VIDEOWIN_CODE_
void VideoWinPaint(void)
{
    PICTURE_INFO_STRUCT     PicInfo;
    UINT16                  i;
    UINT8                   j, k,l;
    UINT32                  CurrentTimeSec, TotalTimeSec;

    if (TRUE == GetMsg(MSG_XVID_DISPLAY_ALL))
    {
        VideoUiFlag = 1;

        DispPictureWithIDNumAndXYoffset(IMG_ID_VEDIO_DOWNBACKGROUND,0,111);
        DispPictureWithIDNumAndXYoffset(IMG_ID_VEDIO_UPBACKGROUND,0,0);
        DispPictureWithIDNum(IMG_ID_VIDEO_BATTERY01+ VideoBatteryLevel);
        DispPictureWithIDNum(IMG_ID_VIDEO_SMALLLOCK+VidelHoldState);

        SendMsg(MSG_VIDEO_DISPFLAG_CURRENT_TIME);
        SendMsg(MSG_VIDEO_DISPFLAG_SCHED);
        SendMsg(MSG_VIDEO_DISPFLAG_VOL);
        SendMsg(MSG_XVID_DISPFLAG_STATUS);
    }

    if (TRUE == GetMsg(MSG_XVID_DISPFLAG_STATUS))
    {
        if (VIDEO_STATE_PLAY == VideoPlayState)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_VEDIO_STAUT_PLAY,8,111);
        }
        else if (VIDEO_STATE_PAUSE == VideoPlayState)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_VEDIO_STAUT_PAUSE,8,111);
        }
        else
        {
            SendMsg(MSG_VIDEO_DISPFLAG_CURRENT_TIME);
            SendMsg(MSG_VIDEO_DISPFLAG_SCHED);
        }

        if (TRUE == GetMsg(MSG_VIDEO_DISPLAY_HOLD))
        {
            DispPictureWithIDNum(IMG_ID_VIDEO_SMALLLOCK+VidelHoldState);
        }
    }

    if (TRUE==GetMsg(MSG_VIDEO_DISPFLAG_BATTERY))
    {
        DispPictureWithIDNum(IMG_ID_VIDEO_BATTERY01+ VideoBatteryLevel);
    }

    if (TRUE==GetMsg(MSG_VIDEO_DISPFLAG_VOL))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_VEDIO_VOLUME_BACK,8,2);
        VedioDisplayVol(VideoOutputVol);
    }

    if (TRUE==GetMsg(MSG_VIDEO_DISPFLAG_CURRENT_TIME))
    {
        CurrentTimeSec = Avi_GetCurrentTime();
        GetTimeHMS(CurrentTimeSec,&i,&j,&k);
        if (VideoTimeSecBack != CurrentTimeSec)
        {
            VideoTimeSecBack=CurrentTimeSec;
            VedioDisplayTime(i,j,k,0);
        }
    }

    if (TRUE==GetMsg(MSG_VIDEO_DISPFLAG_SCHED))//progress bar
    {
        CurrentTimeSec = Avi_GetCurrentTime();
        TotalTimeSec = Avi_GetTotalTime();
        VideoTimeGuageVal = CurrentTimeSec * (VIDEO_TIME_GUAGE_CONST - 1) / TotalTimeSec;

        if (VideoTimeGuageVal>=VIDEO_TIME_GUAGE_CONST) VideoTimeGuageVal=0;

        for (i=0; (i<VideoTimeGuageVal && i<VIDEO_TIME_GUAGE_CONST); i++)
        {
            DisplayPictureFromBuffer(VideoSchedPicInfo.x + 32 + i, VideoSchedPicInfo.y + 117,
                                     VideoSchedPicInfo.xSize, VideoSchedPicInfo.ySize,
                                     (UINT16 *)VideoSchedBuffer);
        }

        for (i=VideoTimeGuageVal; i<VIDEO_TIME_GUAGE_CONST; i++)
        {
            DisplayPictureFromBuffer(VideoSchedPicInfo.x + 32 + i, VideoSchedPicInfo.y + 117,
                                     VideoSchedPicInfo.xSize, VideoSchedPicInfo.ySize,
                                     (uint16 *)(&VideoSchedBuffer[0] + VIDEO_SCHED_BUF_SIZE));
        }
    }
}

/* when decode a slice, we should do some thing here */
_ATTR_VIDEOWIN_CODE_
int xvid_dec_slice_hook(int stride)
{
    // convert to rgb565
    xvid_yuv_rgb565(xvid_y_slice_buf[0], xvid_u_slice_buf[0], xvid_v_slice_buf[0], (int*)(xvid_rgb565_slice_buf[0]), stride,MAX_FRAME_WIDTH) ;

    // skip row 0-15
    if ((xvid_slice_count == 0) && (VideoDisplayTime))
    {
        return 0;
    }

    // skip row 16-16
    if ((xvid_slice_count==1)&&(VideoDisplayTime))
    {
        LCD_DrawBmpVideo(0, 16*xvid_slice_count+1, MAX_FRAME_WIDTH-1, 16*xvid_slice_count+15, 16,  (short *)&xvid_rgb565_slice_buf[0][0] + 1*MAX_FRAME_WIDTH, MAX_FRAME_WIDTH);
        return 0;
    }

    // skip last 7
    if ((xvid_slice_count == 6) && (VideoDisplayTime))
    {
        LCD_DrawBmpVideo(0, 16*xvid_slice_count, MAX_FRAME_WIDTH-1, 16*xvid_slice_count+15-1, 16, xvid_rgb565_slice_buf[0], MAX_FRAME_WIDTH);
        return 0;
    }

    // skip last 16
    if ((xvid_slice_count == 7) && (VideoDisplayTime))
    {
        return 0;
    }

    LCD_DrawBmpVideo(0, 16*xvid_slice_count, MAX_FRAME_WIDTH-1, 16*xvid_slice_count+15, 16, xvid_rgb565_slice_buf[0], MAX_FRAME_WIDTH);

    return 0;
}


#endif

