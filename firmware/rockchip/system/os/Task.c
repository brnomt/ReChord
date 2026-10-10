/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  Task.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:
********************************************************************************
*/
#define _IN_TASK_

#include "SysInclude.h"
#include "ui/USB/FunUSB.h"   /* FunUSBWin window struct */
#include "mainmenu.h"
#include "setmenu.h"
#include "RecordWin.h"
#include "MusicWin.h"
#include "RadioWin.h"
#include "PicWin.h"
#include "VideoWin.h"
#include "fsinclude.h"
#include "FunUSB.h"
#include "TextWin.h"
#include "BrowserUI.h"
#include "ChargeWin.h"
#include "medialibwin.h"
#include "MediaBroWin.h"
#include "MDBBuildWin.h"
//#include "BlueToothA2doSourceWin.h"

/*
--------------------------------------------------------------------------------
  Function name :
  Author        : ZHengYongzhi
  Description   : system stucture initial value

  Input         : pWin ���� function return window pointer.���صĴ���ָ��
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_OS_DATA_
TASK Task =
{
    (TASK_ID)(TASK_ID_MAINMENU),

    {
        &MainMenuWin,

        #ifdef _MEDIA_MODULE_
        &MedialibWin,
        &MediaBroWin,
        #else
        0,
        0,
        #endif

        #ifdef _MUSIC_
        &MusicWin,
        #else
        0,
        #endif

        #ifdef _VIDEO_
        &VideoWin,
        #else
        0,
        #endif

        #ifdef _RADIO_
        &RadioWin,
        #else
        0,
        #endif

        #ifdef _PICTURE_ ////an 4.25
        &PicWin,
        #else
        0,
        #endif

        #ifdef _RECORD_
        &RecordWin,
        #else
        0,
        #endif

        #ifdef _EBOOK_
        &TextMenuWin,
        #else
        0,
        #endif

        #ifdef _BROWSER_
        &BrowserWin,
        #else
        0,
        #endif

        #ifdef _SYSSET_
        &SetMenuWin,
        #else
        0,
        #endif

        #ifdef _USB_
        &FunUSBWin,
        #else
        0,
        #endif
        &ChargeWin,

        &MdbBuildWin,
    },

    {
        MODULE_ID_MAINMENU,
        MODULE_ID_MEDIALIBWIN,
        MODULE_ID_MEDIABRO,
        MODULE_ID_MUSIC,
        MODULE_ID_VIDEO,
        MODULE_ID_RADIOWIN,
        MODULE_ID_PICTURE,
        MODULE_ID_RECORDWIN,
        MODULE_ID_TEXTWIN,
        MODULE_ID_BROWSER,
        MODULE_ID_SETMENU,
        MODULE_ID_USB,
        MODULE_ID_CHARGE_WIN,
        MODULE_ID_MDB_WIN,
    },

    0
};

#ifdef _LOG_DEBUG_
_ATTR_OS_CODE_
uint8* TaskID2Str(TASK_ID taskID)
{
    uint8* pstr;

    switch (taskID)
    {
        case TASK_ID_MAINMENU:// 0
            pstr = "MainMenu";
            break;

        case TASK_ID_MEDIALIB:// 1
            pstr = "MediaLib";
            break;

        case TASK_ID_MEDIABRO:// 2
            pstr = "MediaBro";
            break;

        case TASK_ID_MUSIC:   // 3
            pstr = "Music";
            break;

        case TASK_ID_VIDEO:   // 4
            pstr = "Video";
            break;

        case TASK_ID_RADIO:   // 5
            pstr = "Radio";
            break;

        case TASK_ID_PICTURE:  // 6
            pstr = "Picture";
            break;

        case TASK_ID_RECORD:   // 7
            pstr = "Record";
            break;

        case TASK_ID_EBOOK:   // 8
            pstr = "Ebook";
            break;

        case TASK_ID_BROWSER:   // 9
            pstr = "Browser";
            break;

        case TASK_ID_SYSSET:   // 10
            pstr = "Systemset";
            break;

        case TASK_ID_USB:      // 11
            pstr = "USB";
            break;

        case TASK_ID_CHARGE:    //12
            pstr = "Charge";
            break;

        case TASK_ID_MDB:       //13
            pstr = "MDB";
            break;

        default:
            pstr = "NO THIS TASK";
            break;
    }

    return pstr;

}
#endif

/*
--------------------------------------------------------------------------------
  Function name : void TaskInit(WIN *pWin)
  Author        : ZHengYongzhi
  Description   : task initial,tasts switch code,get task main window pointer.

  Input         : pWin ���� window pointer.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_OS_CODE_
WIN* TaskInit(void **pArg)
{
    DEBUG("Task Enter is :%s", TaskID2Str(Task.TaskID));
    ModuleOverlay(Task.CodeIDList[Task.TaskID], MODULE_OVERLAY_ALL);
    #if 1
    *pArg = &Task.Arg;
    #else

    switch(Task.TaskID)
    {
    case TASK_ID_MAINMENU:
        *pArg = &Task.Arg.MainMenu;
        break;

    case TASK_ID_MEDIALIB:
        *pArg = &Task.Arg.Medialib;
        break;
    case TASK_ID_MEDIABRO:
        *pArg = &Task.Arg.MediaBro;
        break;

    case TASK_ID_MUSIC:
        *pArg = &Task.Arg.Music;
        break;

     case TASK_ID_VIDEO:
        *pArg = &Task.Arg.Video;
        break;

    case TASK_ID_RADIO:
        *pArg = &Task.Arg.Radio;
        break;

    case TASK_ID_RECORD:
        *pArg = &Task.Arg.Record;
        break;

    case TASK_ID_EBOOK:
        *pArg = &Task.Arg.Text;
        break;

    case TASK_ID_BROWSER:
        *pArg = &Task.Arg.Browser;
        break;

    case TASK_ID_SYSSET:
        *pArg = &Task.Arg.Sysset;
        break;

      case TASK_ID_GAME:

        break;

    case TASK_ID_USB:
        *pArg = &Task.Arg.Usb;
        break;

    default:
        break;
    }
    #endif

    return(Task.TaskMainWinList[Task.TaskID]);
}

/*
--------------------------------------------------------------------------------
  Function name : void TaskSwitch(TASK_ID TaskId, TASK_ARG *pTaskArg)
  Author        : ZHengYongzhi
  Description   : task switch,call by application.

  Input         : TaskId   ���� new task id.
                  pTaskArg ���� task parameters.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_OS_CODE_
void TaskSwitch(TASK_ID TaskId, TASK_ARG *pTaskArg)
{
    Task.TaskID = TaskId;

    //clear parameter table message
    memset((uint8*)&Task.Arg, 0, sizeof(Task.Arg));

    if (pTaskArg == (TASK_ARG *)NULL)
    {
        return;
    }

    memcpy((uint8*)&Task.Arg, (uint8*)pTaskArg, sizeof(TASK_ARG));

}
/*
********************************************************************************
*
*                         End of Task.c
*
********************************************************************************
*/

