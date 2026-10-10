/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��  PicWin.c
*
* Description:  picture module
*
* History:      <author>          <time>        <version>
*             anzhiguo          2009-3-2        1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_PICWIN_H_
#include "SysInclude.h"

#ifdef _PICTURE_
#pragma arm section code = "PicWinCode", rodata = "PicWinCode", rwdata = "PicWinData", zidata = "PicWinBss"
#include "fsinclude.h"

#include "SysFindFile.h"

#include "PicWin.h"
#include "PicInterface.h"
#include "mainmenu.h"
#include "ImageControl.h"

#include "BrowserUI.h"
#include "MessageBox.h"
#include "Hold.h"

//__align(4)
//UINT16 PicBuffer[LCD_HEIGHT][LCD_WIDTH];
/*
--------------------------------------------------------------------------------
  Function name : void PicVariableInit(void)
  Author        : anzhiguo
  Description   : variables initial of picture file

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

static void PicVariableInit(void)
{
    PicDirection = 0;
    gPicAutoBrowserOn = 0;//the automatic scan should be close when frist enter picture,
    ImageErrorNum = 0;
    ImageSlideShowTickBake = SysTickCounter;//add by evan wu
}

/*
--------------------------------------------------------------------------------
  Function name : void PicWinInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void PicWinInit(void *pArg)
{
    PicVariableInit();
    KeyReset();

    //BLOffDisable();
    if (gPicAutoBrowserOn)//automatic scan open
    {
        BLOffDisable();
        AutoPowerOffDisable();
    }

    LcdStandbyDisable();

    PictureWinSvcStart(pArg);
}

/*
--------------------------------------------------------------------------------
  Function name : void PicWinDeInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void PicWinDeInit(void)
{
    LcdStandbyDisable();
    BLOffEnable();
    if(gPicAutoBrowserOn)//automatic scan open
    {
        AutoPowerOffEnable();
    }
}

static void HorizontalDisplay(void)
{
#ifdef _FRAME_BUFFER_
    Lcd_FreshEn();
#endif

    LCD_SetDiaplayMode(LCD_MODE_90);
    SendMsg(MSG_VIDEO_VERTICAL_DISPLAY);
}
/*
---------------------------------------


-----------------------------------------
*/
#ifdef IMAGE_DEC_ADVANCE
//�Զ����ģʽ: ��������ͼƬ֮���ʱ����
static BOOLEAN  isTimeToDecNext(void)
{
    return (SysTickCounter - gCurImageSlideShowTick >= (gPicAutoBrowserTime + 1)*2*100);
}

//�Զ����ģʽ: ����play������һ�β�����һ��ͼƬ��ʱ����
//        ����: �Է��Ѿ�Ԥ������һ��ͼƬ��play���£�������ʾ��һ��
static BOOLEAN  isTimeToDisNext(void)
{
    return (SysTickCounter - gMarkSysTickCounter >= (gPicAutoBrowserTime + 1)*100);
}

static BOOLEAN  isDisNextImage(void)
{
    return(1 == gIsDisNextImage);
}

static BOOLEAN  isDisPreImage(void)
{
    return(1 == gIsDisPreImage);
}

//ˢ��ָ��Index��Framebuffer
static void DisplaySpecificImage(int FrameBufferIndex)
{
    LCD_Set_Current_BufferFrame_Index(FrameBufferIndex);
    LCD_SetDiaplayMode(LCD_MODE_0);
    Lcd_SetWindow(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);
    Lcd_FreshEn();
    Lcd_BuferTranfer();

    gCurDisFrameIndex = FrameBufferIndex;

    CurImageDecState = IMAGE_DEC_STOP;
    NextImageDecState = IMAGE_DEC_WAITING;
    PreImageDecState = IMAGE_DEC_WAITING;

    gClearLCD = 0;
}

//�����ļ���ʽ��֧�ֵ�ͼƬ״̬
static void SetUnsupportedDecState(void)
{
    if(CurImageIsDecoding())
    {
        CurImageDecState = IMAGE_DEC_UNSUPPORTED;
    }
    else if(NextImageIsDecoding())
    {
        NextImageDecState = IMAGE_DEC_UNSUPPORTED;
    }
    else if(PreImageIsDecoding())
    {
        PreImageDecState = IMAGE_DEC_UNSUPPORTED;
    }

    if(CurImageIsUnSupported())
    {
        int NextFrameIndex = gCurDisFrameIndex + 1;
        if(NextFrameIndex > 2)
            NextFrameIndex = 0;

        gUnsupportFrameIndex = NextFrameIndex;
    }
    else
    {
        gUnsupportFrameIndex = LCD_Get_Current_Index();
    }

}

static void ProcessUnsupportedDec(TASK_ARG TaskArg)
{
    if(CurImageIsUnSupported())
    {
        ImageErrorNum++;
        HorizontalDisplay();
        WinCreat(&PicWin, &MessageBoxWin, &TaskArg);
    }
    else if(NextImageIsUnSupported())
    {
        NextImageTaskArg.Message.ContentID = TaskArg.Message.ContentID;
        NextImageTaskArg.Message.HoldTime = TaskArg.Message.HoldTime;
        NextImageTaskArg.Message.TitleID = TaskArg.Message.TitleID;
        NextImageTaskArg.Message.CurDisFrameIndex = TaskArg.Message.CurDisFrameIndex;
        NextImageTaskArg.Message.UnsupportFrameIndex = TaskArg.Message.UnsupportFrameIndex;

        ImageDecodeProc(MSG_IMAGE_STOP,0);
        if(gIsContinueDecode == 1)
        {
            SendMsg(MSG_IMAGE_DECNEXT);
        }
    }
    else if(PreImageIsUnSupported())
    {
        PreImageTaskArg.Message.ContentID = TaskArg.Message.ContentID;
        PreImageTaskArg.Message.HoldTime = TaskArg.Message.HoldTime;
        PreImageTaskArg.Message.TitleID = TaskArg.Message.TitleID;
        PreImageTaskArg.Message.CurDisFrameIndex = TaskArg.Message.CurDisFrameIndex;
        PreImageTaskArg.Message.UnsupportFrameIndex = TaskArg.Message.UnsupportFrameIndex;

        ImageDecodeProc(MSG_IMAGE_STOP,0);
        if(gIsContinueDecode == 1)
        {
            SendMsg(MSG_IMAGE_DECNEXT);
        }
    }
}
#else
static BOOLEAN  isTimeToDecNext(void)
{
    return (SysTickCounter - ImageSlideShowTickBake >=  (gPicAutoBrowserTime + 1)*2*100);
}
#endif

static BOOLEAN  isSlideShowOn(void)
{
    return(1 == gPicAutoBrowserOn);
}
/*
--------------------------------------------------------------------------------
  Function name : UINT16 PIcWinService(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        : 0:success

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef IMAGE_DEC_ADVANCE
UINT32 PIcWinService(void)
{
    TASK_ARG TaskArg;
    UINT32 Retval = 0;

//----------------------error take care------------------------------------
#ifndef FILE_ERROR_DIALOG

    if (GetMsg(MSG_MESSAGEBOX_DESTROY))//destory message fo message box
    {
        //the message box appear only when no file or open file failure.
        //return to main interface when receive the box's destroy message.
        TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
        return 1;
    }

#else

    if (GetMsg(MSG_MESSAGEBOX_DESTROY))//message of message box destory.
    {
        //the message box appear only when no file or open file failure.
        //return to main interface when receive the box's destroy message.
        if (ImageErrorNum >= PicSysFileInfo.TotalFiles)
        {
            ThreadDelete(&pMainThread, &PictureThread);
            {
                TaskArg.Browser.FileType = FileTypePicture;
                TaskArg.Browser.FileNum  = gbPicFileNum;
            }
            TaskArg.Browser.FromWhere = 0;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            return 1;
        }

        if (!PicDirection)
        {
            ClearMsg(MSG_IMAGE_DECNEXT);
            ImageDecodeProc(MSG_IMAGE_STOP,0);
            ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,(void*)1);
            AdvanceDecVariableInit();
            SendMsg(MSG_IMAGE_DECSTART);
        }
        else
        {
            ClearMsg(MSG_IMAGE_DECNEXT);
            ImageDecodeProc(MSG_IMAGE_STOP,0);
            ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,0);
            AdvanceDecVariableInit();
            SendMsg(MSG_IMAGE_DECSTART);
        }
        return Retval;
    }

#endif

    //Hold dialog return to handle program
    if (CheckMsg(MSG_NEED_PAINT_ALL) && CheckMsg(MSG_HOLDDIALOG_DESTROY))
    {
        ClearMsg(MSG_NEED_PAINT_ALL);
        ClearMsg(MSG_HOLDDIALOG_DESTROY);

        ImageDecodeProc(MSG_IMAGE_STOP,0);
        SendMsg(MSG_IMAGE_DECSTART);
    }

    if (GetMsg(MSG_IMAGE_NOFILE))
    {
        //no pictrue file,delete thread, popup dialog
        ThreadDelete(&pMainThread, &PictureThread);

        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
        TaskArg.Message.HoldTime  = 3;

        SetUnsupportedDecState();
        TaskArg.Message.CurDisFrameIndex = gCurDisFrameIndex;
        TaskArg.Message.UnsupportFrameIndex = gUnsupportFrameIndex;
        ProcessUnsupportedDec(TaskArg);
        return Retval;
    }

    if (GetMsg(MSG_IMAGE_OPENERROR))
    {
        //ThreadDelete(&pMainThread, &PictureThread);
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_FILE_FORMAT_ERROR;
        TaskArg.Message.HoldTime  = 3;

        SetUnsupportedDecState();
        TaskArg.Message.CurDisFrameIndex = gCurDisFrameIndex;
        TaskArg.Message.UnsupportFrameIndex = gUnsupportFrameIndex;
        ProcessUnsupportedDec(TaskArg);
        return Retval;
    }

    if (GetMsg(MSG_IMAGE_DEC_ERROR)||GetMsg(MSG_IMAGE_INIT_ERROR))//decode error
    {
#ifdef FILE_ERROR_DIALOG
        DEBUG(" Image dec error or Init Error ");
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_FILE_FORMAT_ERROR;
        TaskArg.Message.HoldTime  = 3;

        SetUnsupportedDecState();
        TaskArg.Message.CurDisFrameIndex = gCurDisFrameIndex;
        TaskArg.Message.UnsupportFrameIndex = gUnsupportFrameIndex;
        ProcessUnsupportedDec(TaskArg);
#endif
        return Retval;
    }

//---------------decode head or decode --------------------------------
    if (GetMsg(MSG_IMAGE_INIT_SUCESS))//initial sucess,send decode message
    {
        SendMsg(MSG_IMAGE_DEC);
        return 0;
    }

    if (isSlideShowOn())
    {
        if((NextImageIsStop() || NextImageIsUnSupported()) && isTimeToDecNext() && isTimeToDisNext())
        {
            int NextFrameIndex = gCurDisFrameIndex + 1;
            if(NextFrameIndex > 2)
                NextFrameIndex = 0;

            ImageDecodeProc(MSG_IMAGE_STOP,0);
            ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,(void*)1);

            if(NextImageIsStop())
            {
                DisplaySpecificImage(NextFrameIndex);
                gCurImageSlideShowTick = SysTickCounter;
                SendMsg(MSG_IMAGE_DECNEXT);
            }
            else
            {
                CurImageDecState = IMAGE_DEC_WAITING;
                NextImageDecState = IMAGE_DEC_WAITING;
                PreImageDecState = IMAGE_DEC_WAITING;
                ImageErrorNum++;
                HorizontalDisplay();
                WinCreat(&PicWin, &MessageBoxWin, &NextImageTaskArg);
                return Retval;
            }
        }
    }

    if(isDisNextImage())
    {
        if(NextImageIsStop())
        {
            int NextFrameIndex = gCurDisFrameIndex + 1;
            if(NextFrameIndex > 2)
                NextFrameIndex = 0;

            ImageDecodeProc(MSG_IMAGE_STOP,0);
            ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,(void*)1);

            DisplaySpecificImage(NextFrameIndex);
            gIsDisNextImage = 0;

            SendMsg(MSG_IMAGE_DECNEXT);
        }
        else
        {
            if(gClearLCD)
            {
                //������ ��ʾ'���ڽ���'��ͼƬ �滻 ��������
                gClearLCD = 0;
                SystemLcdPara.pLcd_ClrSrc();
            }
        }
    }

    if(isDisPreImage())
    {
        if(PreImageIsStop())
        {
            int PreFrameIndex = gCurDisFrameIndex - 1;
            if(PreFrameIndex < 0)
                PreFrameIndex = 2;

            ImageDecodeProc(MSG_IMAGE_STOP,0);
            ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,0);

            DisplaySpecificImage(PreFrameIndex);
            gIsDisPreImage = 0;

            SendMsg(MSG_IMAGE_DECNEXT);
        }
        else
        {
            if(gClearLCD)
            {
                //������ ��ʾ'���ڽ���'��ͼƬ �滻 ��������
                gClearLCD = 0;
                SystemLcdPara.pLcd_ClrSrc();
            }
        }
    }

    return Retval;
}
#else
UINT32 PIcWinService(void)
{
    TASK_ARG TaskArg;
    UINT32 Retval = 0;

//----------------------error take care------------------------------------
#ifndef FILE_ERROR_DIALOG

    if (GetMsg(MSG_MESSAGEBOX_DESTROY))//destory message fo message box
    {
        //the message box appear only when no file or open file failure.
        //return to main interface when receive the box's destroy message.
        TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
        return 1;
    }

#else

    if (GetMsg(MSG_MESSAGEBOX_DESTROY))//message of message box destory.
    {
        //the message box appear only when no file or open file failure.
        //return to main interface when receive the box's destroy message.
        if (ImageErrorNum >= PicSysFileInfo.TotalFiles)
        {
            ThreadDelete(&pMainThread, &PictureThread);
            {
                TaskArg.Browser.FileType = FileTypePicture;
                TaskArg.Browser.FileNum  = gbPicFileNum;
            }
            TaskArg.Browser.FromWhere = 0;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            return 1;
        }

        if (!PicDirection)
        {
            ImageDecodeProc(MSG_IMAGE_STOP,0);
            ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,(void*)1);
            SendMsg(MSG_IMAGE_DECSTART);
        }
        else
        {
            ImageDecodeProc(MSG_IMAGE_STOP,0);
            ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,0);
            SendMsg(MSG_IMAGE_DECSTART);
        }
        return Retval;
    }

#endif

    //Hold dialog return to handle program
    if (CheckMsg(MSG_NEED_PAINT_ALL) && CheckMsg(MSG_HOLDDIALOG_DESTROY))
    {
        ClearMsg(MSG_NEED_PAINT_ALL);
        ClearMsg(MSG_HOLDDIALOG_DESTROY);

        ImageDecodeProc(MSG_IMAGE_STOP,0);
        SendMsg(MSG_IMAGE_DECSTART);
    }

    if (GetMsg(MSG_IMAGE_NOFILE))
    {
        //no pictrue file,delete thread, popup dialog
        ThreadDelete(&pMainThread, &PictureThread);

        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&PicWin, &MessageBoxWin, &TaskArg);
        HorizontalDisplay();
        return Retval;
    }

    if (GetMsg(MSG_IMAGE_OPENERROR))
    {
        ImageErrorNum++;
        ThreadDelete(&pMainThread, &PictureThread);
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_FILE_FORMAT_ERROR;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&PicWin, &MessageBoxWin, &TaskArg);
        HorizontalDisplay();
        return Retval;
    }

    if (GetMsg(MSG_IMAGE_DEC_ERROR)||GetMsg(MSG_IMAGE_INIT_ERROR))//decode error
    {
        ImageErrorNum++;
#ifdef FILE_ERROR_DIALOG
        DEBUG(" Image dec error or Init Error ");
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_FILE_FORMAT_ERROR;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;

        HorizontalDisplay();
        WinCreat(&PicWin, &MessageBoxWin, &TaskArg);

#else
        SendMsg(MSG_IMAGE_UNSPORTED_PIC);
#endif

        return Retval;
    }

//---------------decode head or decode --------------------------------
    if (GetMsg(MSG_IMAGE_INIT_SUCESS))//initial sucess,send decode message
    {
        SendMsg(MSG_IMAGE_DEC);
        return 0;
    }

    if (isSlideShowOn())
    {
        //if slide open
        if (ImageIsStop()&&isTimeToDecNext())//if current picture had deocded or the time come of decoding the next picture,that is TimeToDecNext() is ture,
        {
            ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,(void*)1);
            SendMsg(MSG_IMAGE_DECSTART);
        }
    }

    return Retval;
}
#endif
/*
--------------------------------------------------------------------------------
  Function name : UINT16 PicWinKey(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
UINT32 PicWinKey(void)
{
    UINT16  i, index, NextFile;
    UINT32 KeyVal;
    UINT16  retval = 0;
    TASK_ARG TaskArg;

    KeyVal = GetKeyVal();

    //5 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
    }

    //6 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
    }

    switch (KeyVal)
    {
        case KEY_VAL_MENU_PRESS:// exit to main interface
            ThreadDelete(&pMainThread, &PictureThread);
            TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
            TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            retval = 1;
            break;

        case KEY_VAL_MENU_SHORT_UP:// exit to browser
        case KEY_VAL_ESC_SHORT_UP:
            ThreadDelete(&pMainThread, &PictureThread);
            if (CheckMsg(MSG_BROW_FROM_MAINMENU))
            {
                TaskArg.Browser.FileType = FileTypeALL;
            }
            else
            {
                TaskArg.Browser.FileType = FileTypePicture;
            }

            TaskArg.Browser.FileNum  = gbPicFileNum;
            TaskArg.Browser.FromWhere = 0;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            retval = 1;
            break;

        case KEY_VAL_FFD_SHORT_UP://next picture.
            if (0 == gPicAutoBrowserOn)//only when automatic slide close,the page to previous or next can work
            {
#ifdef IMAGE_DEC_ADVANCE
                int CurKeyInfo = gCurKeyInfo;
                PicDirection = 0;

                //������ʾ��һ��ǰ�����ڵȴ���ʾ��һ��
                if(CurKeyInfo == FFW_KEY && gIsDisPreImage == 1 && PreImageIsDecoding())
                {
                    ImageDecodeProc(MSG_IMAGE_STOP,0);
                    DisplaySpecificImage(gCurDisFrameIndex);

                    gIsDisNextImage = 0;
                    gIsDisPreImage = 0;
                    gIsContinueDecode = 1;
                    gCurKeyInfo = FFD_KEY;

                    SendMsg(MSG_IMAGE_DECNEXT);
                    break;
                }

                gIsDisPreImage = 0;
                gCurKeyInfo = FFD_KEY;

                if(NextImageIsUnSupported())
                {
                    ImageErrorNum++;
                    ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,(void*)1);
                    HorizontalDisplay();
                    WinCreat(&PicWin, &MessageBoxWin, &NextImageTaskArg);
                    break;
                }

                if(NextImageIsWaiting())  //��һ��ͼƬ��δ��ʼ����
                {
                    gIsDisNextImage = 1;
                }

                if(!isDisNextImage())
                {
                    gIsDisNextImage = 1;
                    gClearLCD = 1;
                }
                else    //���ڵȴ���ʾ��һ�ţ��ְ�����ʾ������
                {
                    ClearMsg(MSG_IMAGE_DECNEXT);
                    ImageDecodeProc(MSG_IMAGE_STOP,0);

                    if(NextImageIsWaiting())
                    {
                        ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,(void*)1);
                    }
                    else
                    {
                        ImageDecodeProc(MSG_IMAGE_GET_TWO_NEXT_FILE,(void*)1);
                    }

                    LCD_Set_Current_BufferFrame_Index(gCurDisFrameIndex);
                    if(CurKeyInfo == FFW_KEY && PreImageIsDecoding())
                    {
                    }
                    else    //�嵱ǰ��ʾbuffer����ֹ������ͼƬ�ļ���ʽ��֧�֣�����bufferֻ��һ����
                    {
                        LCD_ClrSrc();
                    }

                    gIsDisNextImage = 0;
                    gIsContinueDecode = 1;
                    CurImageDecState = IMAGE_DEC_DECODING;
                    NextImageDecState = IMAGE_DEC_WAITING;
                    PreImageDecState = IMAGE_DEC_WAITING;
                    gClearLCD = 0;

                    SendMsg(MSG_IMAGE_DECSTART);
                }
#else
                PicDirection = 0;
                ImageDecodeProc(MSG_IMAGE_STOP,0);
                //PictureGetNext(1);
                ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,(void*)1);
                SendMsg(MSG_IMAGE_DECSTART);
#endif
            }
            break;

        case KEY_VAL_FFW_SHORT_UP://previous
            if (0 == gPicAutoBrowserOn)//only when automatic slide close,the page to previous or next can work
            {
#ifdef IMAGE_DEC_ADVANCE
                int CurKeyInfo = gCurKeyInfo;
                PicDirection = 1;

                //������ʾ��һ��ǰ�����ڵȴ���ʾ��һ��
                if(CurKeyInfo == FFD_KEY && gIsDisNextImage == 1 && NextImageIsDecoding())
                {
                    ImageDecodeProc(MSG_IMAGE_STOP,0);
                    DisplaySpecificImage(gCurDisFrameIndex);

                    gIsDisNextImage = 0;
                    gIsDisPreImage = 0;
                    gIsContinueDecode = 1;
                    gCurKeyInfo = FFW_KEY;

                    SendMsg(MSG_IMAGE_DECNEXT);
                    break;
                }

                gIsDisNextImage = 0;
                gCurKeyInfo = FFW_KEY;

                if(PreImageIsUnSupported()) //�ļ���ʽ��֧��
                {
                    ImageErrorNum++;
                    ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,0);
                    HorizontalDisplay();
                    WinCreat(&PicWin, &MessageBoxWin, &PreImageTaskArg);
                    break;
                }

                if(PreImageIsWaiting())  //��һ��ͼƬ��δ��ʼ����
                {
                    gIsDisPreImage = 1;
                }

                if(!isDisPreImage())
                {
                    gIsDisPreImage = 1;
                    gClearLCD = 1;
                }
                else    //���ڵȴ���ʾ��һ�ţ��ְ�����ʾ������
                {
                    ClearMsg(MSG_IMAGE_DECNEXT);
                    ImageDecodeProc(MSG_IMAGE_STOP,0);

                    if(PreImageIsWaiting())
                    {
                        ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,0);
                    }
                    else
                    {
                        ImageDecodeProc(MSG_IMAGE_GET_TWO_NEXT_FILE,0);
                    }

                    LCD_Set_Current_BufferFrame_Index(gCurDisFrameIndex);
                    if((CurKeyInfo == FFD_KEY || CurKeyInfo == MENU_KEY) && NextImageIsDecoding())
                    {
                    }
                    else    //�嵱ǰ��ʾbuffer����ֹ������ͼƬ�ļ���ʽ��֧�֣�����bufferֻ��һ����
                    {
                        LCD_ClrSrc();
                    }

                    gIsDisPreImage = 0;
                    gIsContinueDecode = 1;
                    CurImageDecState = IMAGE_DEC_DECODING;
                    NextImageDecState = IMAGE_DEC_WAITING;
                    PreImageDecState = IMAGE_DEC_WAITING;
                    gClearLCD = 0;

                    SendMsg(MSG_IMAGE_DECSTART);
                }
#else
                PicDirection = 1;
                ImageDecodeProc(MSG_IMAGE_STOP,0);
                //PictureGetNext(0);
                ImageDecodeProc(MSG_IMAGE_GET_NEXT_FILE,0);
                SendMsg(MSG_IMAGE_DECSTART);
#endif
            }
            break;

        case KEY_VAL_PLAY_SHORT_UP://pause automatic scan,restore automatic scan,extension
            PicDirection = 0;
            if (gPicAutoBrowserOn)//automatic scan open
            {
#ifdef IMAGE_DEC_ADVANCE
                gCurKeyInfo = FFD_KEY;
#endif
                BLOffEnable();
                AutoPowerOffEnable();
                ImageDecodeProc(MSG_IMAGE_PAUSE,0);
            }
            else
            {
#ifdef IMAGE_DEC_ADVANCE
                gCurKeyInfo = PLAY_KEY;
                gIsDisNextImage = 0;
                gIsDisPreImage = 0;
                gMarkSysTickCounter = SysTickCounter;
#endif
                BLOffDisable();
                AutoPowerOffDisable();
                ImageDecodeProc(MSG_IMAGE_RESUME,0);
            }
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&PicWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&PicWin, &HoldWin, &TaskArg);
            break;

        default:
            retval = FALSE;
            break;
    }
    return(retval);
}

/*
--------------------------------------------------------------------------------
  Function name : void TextPaint(void)
  Author        : yangwenjie
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#if 0
void PicWinPaint(void)
{
    //modified by evan wu,2009-4-28
    if (GetMsg(MSG_IMAGE_UNSPORTED_PIC))
    {
        //display default pic
        DispPictureWithIDNum(IMG_ID_UNSPORTTEDPIC);
        SendMsg(MSG_IMAGE_STOP);
    }
    else if (GetMsg(MSG_IMAGE_DEC_SUCESS))//����ɹ���ˢһ��,������Ϣ������һ��
    {
        LCD_DrawBmp(0, PicFileInfo.dispOffset , PicFileInfo.Len, PicFileInfo.dispOffset, 16, (UINT16*)(PicFileInfo.pPicBuf));
        SendMsg(MSG_IMAGE_DEC);
    }
    else if (GetMsg(MSG_IMAGE_DEC_END))//���������ˢ���һ��
    {
        LCD_DrawBmp(0, PicFileInfo.dispOffset , PicFileInfo.Len, PicFileInfo.dispOffset, 16, (UINT16*)(PicFileInfo.pPicBuf));
        SendMsg(MSG_IMAGE_STOP);
    }
}
#else
static __inline void swap_data(UINT16 *start,UINT16 *end,const UINT16 len)
{
    //int i;
    int cnt = len/2;
    UINT16 temp;
    //for(i = 0;i<LCD_HEIGHT/2;i++)
    while (cnt--)
    {
        temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}
static void pic_draw_one_line(UINT16 row_index, UINT16 pos0, UINT16 pos1, UINT16 Pixel, UINT16 *pData)
{
#ifdef AUTO_ADAPT_TO_LCD
    if (pos1 - pos0 == ImageMaxWidth)
    {
        LCD_DrawBmp(pos0,row_index,pos1,row_index,Pixel,pData);
    }
    else if (pos1 - pos0 == ImageMaxHeight)
    {
        //swap data
        //stupid method,to adapt to inflexible driver
        //swap_data(pData,pData + LCD_HEIGHT - 1,LCD_HEIGHT);

        LCD_DrawBmp(ImageMaxWidth - row_index - 1,pos0, ImageMaxWidth - row_index - 1,pos1,Pixel,pData);
    }
    else //default
        LCD_DrawBmp(pos0,row_index,pos1,row_index,Pixel,pData);
#else
    LCD_DrawBmp(pos0,row_index,pos1,row_index,Pixel,pData);
    //swap_data(pData,pData + LCD_HEIGHT - 1,LCD_HEIGHT);

    //LCD_DrawBmp(LCD_WIDTH - row_index - 1,pos0,LCD_WIDTH - row_index - 1,pos1,Pixel,pData);


#endif
}

#if 0
static void draw_one_picture(UINT16 x0, UINT16 y0, UINT16 xsize, UINT16 ysize, UINT16 Pixel)
{
#ifdef AUTO_ADAPT_TO_LCD
    if (xsize - x0 == LCD_WIDTH)
    {
        LCD_DrawBmp(x0, y0, xsize, ysize, Pixel, &PicBuffer[0][0]);
    }
    else if (xsize - x0 == LCD_HEIGHT)
    {
        LCD_DrawBmp(LCD_WIDTH - ysize - 1, x0, LCD_WIDTH - y0 - 1, xsize, Pixel, &PicBuffer[0][0]);
    }
    else //default
        LCD_DrawBmp(x0, y0, xsize, ysize, Pixel, &PicBuffer[0][0]);
#else
    LCD_DrawBmp(x0, y0, xsize, ysize, Pixel, &PicBuffer[0][0]);
#endif
}

static void pic_draw_to_buffer(UINT16 row_index, UINT16 pos0, UINT16 pos1, UINT16 *pData)
{
    int i;

    //printf("row_index: %d, pos0: %d, pos1: %d\n", row_index, pos0, pos1);

    if (pos0 < 0)
        pos0 = 0;

    if (pos1 > LCD_WIDTH)
        pos1 = LCD_WIDTH;

    if ((row_index >= LCD_HEIGHT) || (row_index < 0))
        return;

    for (i = pos0; i < pos1; i++)
    {
        PicBuffer[row_index][i] = *pData++;
    }
}

#endif

void PicWinPaint(void)
{
    //modified by evan wu,2009-4-28
    if (GetMsg(MSG_IMAGE_UNSPORTED_PIC))
    {
        //display default pic
        DispPictureWithIDNum(IMG_ID_UNSPORTTEDPIC);
        SendMsg(MSG_IMAGE_STOP);
    }
    else if (GetMsg(MSG_IMAGE_DEC_SUCESS))//decode success,refresh one row,and send message to deode next row
    {
        if(PicFileInfo.dispOffset - PicFileInfo.FirstOffset >= PicFileInfo.OutputHeight)
        {
            SendMsg(MSG_IMAGE_STOP);

            #ifdef IMAGE_DEC_ADVANCE
            if(gIsContinueDecode)
                SendMsg(MSG_IMAGE_DECNEXT);
            #endif
        }
        else
        {
#ifdef AUTO_ADAPT_TO_LCD
            if(PicFileInfo.Len == ImageMaxWidth)
            {
                LCD_DrawBmp(ImageLeft, ImageTop+PicFileInfo.dispOffset , ImageMaxWidth, 1, 16, (UINT16*)(PicFileInfo.pPicBuf));
            }
            else
            {
                LCD_DrawBmp(ImageLeft, ImageTop+PicFileInfo.dispOffset , ImageMaxHeight, 1, 16, (UINT16*)(PicFileInfo.pPicBuf));
            }
#else
            LCD_DrawBmp(ImageLeft, ImageTop+PicFileInfo.dispOffset , ImageMaxWidth, 1, 16, (UINT16*)(PicFileInfo.pPicBuf));
#endif
            //pic_draw_one_line(PicFileInfo.dispOffset, 0, PicFileInfo.Len,16,(UINT16*)(PicFileInfo.pPicBuf));

            //pic_draw_to_buffer(PicFileInfo.dispOffset, 0, PicFileInfo.Len, (UINT16*)(PicFileInfo.pPicBuf));

            SendMsg(MSG_IMAGE_DEC);
        }
    }
    else if (GetMsg(MSG_IMAGE_DEC_END))//decode finish,fresh the last row
    {
        if(PicFileInfo.dispOffset - PicFileInfo.FirstOffset < PicFileInfo.OutputHeight)
        {
#ifdef AUTO_ADAPT_TO_LCD
            if(PicFileInfo.Len == ImageMaxWidth)
            {
                LCD_DrawBmp(ImageLeft, ImageTop+PicFileInfo.dispOffset , ImageMaxWidth, 1, 16, (UINT16*)(PicFileInfo.pPicBuf));
            }
            else
            {
                LCD_DrawBmp(ImageLeft, ImageTop+PicFileInfo.dispOffset , ImageMaxHeight, 1, 16, (UINT16*)(PicFileInfo.pPicBuf));
            }
#else
            LCD_DrawBmp(ImageLeft, ImageTop+PicFileInfo.dispOffset , ImageMaxWidth, 1, 16, (UINT16*)(PicFileInfo.pPicBuf));
#endif
            //pic_draw_one_line(PicFileInfo.dispOffset, 0, PicFileInfo.Len,16,(UINT16*)(PicFileInfo.pPicBuf));

            //pic_draw_to_buffer(PicFileInfo.dispOffset, 0, PicFileInfo.Len, (UINT16*)(PicFileInfo.pPicBuf));
            //draw_one_picture(0, 0, LCD_WIDTH, LCD_HEIGHT, 16);
        }

        SendMsg(MSG_IMAGE_STOP);

        #ifdef IMAGE_DEC_ADVANCE
        if(gIsContinueDecode)
            SendMsg(MSG_IMAGE_DECNEXT);
        #endif
    }
}
#endif

#pragma arm section code
/*
********************************************************************************
*
*                         End of PicWin.c
*
********************************************************************************
*/
#endif

