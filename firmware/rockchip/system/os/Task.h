/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   Task.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _TASK_H_
#define _TASK_H_

#undef  EXT
#ifdef _IN_TASK_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------

  Description:  task ID definition

--------------------------------------------------------------------------------
*/
typedef enum
{
    TASK_ID_MAINMENU = (UINT32)0,
    TASK_ID_MEDIALIB,
    TASK_ID_MEDIABRO,
    TASK_ID_MUSIC,
    TASK_ID_VIDEO,
    TASK_ID_RADIO,
    TASK_ID_PICTURE,
    TASK_ID_RECORD,
    TASK_ID_EBOOK,
    TASK_ID_BROWSER,
    TASK_ID_SYSSET,
    TASK_ID_USB,
    TASK_ID_CHARGE,
    TASK_ID_MDB,
    TASK_ID_MAX

} TASK_ID;

/*
--------------------------------------------------------------------------------

  Description:  task initial parameter.

--------------------------------------------------------------------------------
*/
//parameter structure that is for main menu window tasks switch.
typedef __packed struct _MAINMENU_WIN_ARG_
{
    uint8 MenuID;

} MAINMENU_WIN_ARG;

//music playing window task switch parameters.
typedef __packed struct _MUSIC_WIN_ARG_
{
    uint16 FileNum;
    uint16 MediaTitleAdd;

} MUSIC_WIN_ARG;

typedef __packed struct _MEDIAWIN_WIN_ARG_
{
    uint16 CurId;

} MEDIAWIN_WIN_ARG;

typedef __packed struct _MEDIABRO_WIN_ARG_
{
    uint16 CurId;
    uint16 TitleAdd;
    SYS_MEDIABRO_DIR_TREE_STRUCT MediaDirTreeInfo;

} MEDIABRO_WIN_ARG;

//video playing window task switch parameters.
typedef __packed struct _VIDEO_WIN_ARG_
{
    uint16 FileNum;
} VIDEO_WIN_ARG;

//Radio window task switch parameters.
typedef __packed struct _RADIO_WIN_ARG_
{
    uint16 Freq;
    uint16 AutoPreset;
} RADIO_WIN_ARG;

//RecordWin window task switch parameters.
typedef __packed struct _RECORD_WIN_ARG_
{
    uint32 RecordExitTaskID;         //when exit record,Exit to TASK ID mainmenu or FM
    uint32 RecordType;               //Record Mode : 0 : MIC_STERO / 1 : LINE1 / 2 : LINE2 / 3 : MIC1_MONO / 4 : MIC2_MONO
    uint32 RecordSampleRate;        //record Samplerate  8K - 192K : 8000 - 192000;
    uint32 RecordChannel;            //record Channels : 1 : mono / 2 : stero
    uint32 RecordDataWidth;         //Record DataWidth : 0x0F : 16bit / 0x17 : 24bit
    uint32 RecordEncodeType;        //Record EncodeType : 0 : wav / 1 : mp3 / 2 : pcm
    uint32 RecordCodecConfig;       //Record CodecConfig : 0 : Codec no config / 1 : Codec config

} RECORD_WIN_ARG;

typedef __packed struct _PICTURE_WIN_ARG_
{
    uint16 FileNum;

} PICTURE_WIN_ARG;

//TextWin window task switch parameters.
typedef __packed struct _TEXT_WIN_ARG_
{
    uint16 FileNum;

} TEXT_WIN_ARG;

//BrowserWin窗口任务切换参数
typedef __packed struct _BROWSER_WIN_ARG_
{
    uint8  FileType;
    uint16 FileNum;
    uint8 FromWhere;

} BROWSER_WIN_ARG;

//SysSetWin window task switch parameters.
typedef __packed struct _SYSSET_WIN_ARG_
{
    uint16 FileNum;

} SYSSET_WIN_ARG;

//UsbWin parameter window.
typedef __packed struct _USB_WIN_ARG_
{
    uint16 FunSel;

} USB_WIN_ARG;

//dialog  parameter window..
typedef __packed struct _DIALOG_WIN_ARG_
{
    uint8  Button;
    uint16 TitleID;
    uint16 ContentID;

} DIALOG_WIN_ARG;

//message box  parameter window.
typedef __packed struct _MESSAGE_WIN_ARG_
{
    uint16 TitleID;
    uint16 ContentID;
    uint16 HoldTime;
    uint16 CurDisFrameIndex;
    uint16 UnsupportFrameIndex;
} MESSAGE_WIN_ARG;

//Hold dialog  parameters
typedef __packed struct _HOLD_WIN_ARG_
{
    uint16 HoldAction;

} HOLD_WIN_ARG;

//MDB win parameter
typedef __packed struct _MDB_WIN_ARG_
{
    uint16 TaskID;
}MDB_WIN_ARG;

typedef __packed struct _PICBRO_WIN_ARG_
{
    uint16 CurId;
    uint16 TitleAdd;
    SYS_PICBRO_DIR_TREE_STRUCT JpegMediaDirTreeInfo;

} PICBRO_WIN_ARG;
//Volume dialog  parameters
typedef __packed struct _VOLUME_WIN_ARG_
{
    uint16 VolumeAction;

} VOLUME_WIN_ARG;

typedef union {

    MAINMENU_WIN_ARG    MainMenu;
    MEDIAWIN_WIN_ARG    Medialib;
    MEDIABRO_WIN_ARG    MediaBro;
    MUSIC_WIN_ARG       Music;
    VIDEO_WIN_ARG       Video;
    RADIO_WIN_ARG       Radio;
    PICTURE_WIN_ARG     Pic;
    RECORD_WIN_ARG      Record;
    TEXT_WIN_ARG        Text;
    BROWSER_WIN_ARG     Browser;
    SYSSET_WIN_ARG      Sysset;
    USB_WIN_ARG         Usb;
    DIALOG_WIN_ARG      Dialog;
    MESSAGE_WIN_ARG     Message;
    HOLD_WIN_ARG        Hold;
    MDB_WIN_ARG         Mdb;
    VOLUME_WIN_ARG      Volume;

    PICBRO_WIN_ARG      PicBro;
}TASK_ARG;

/*
--------------------------------------------------------------------------------

  Description:  任务结构体参数定义

--------------------------------------------------------------------------------
*/
typedef struct TASK_STRUCK {

    TASK_ID TaskID;                         //task ID

    WIN     *TaskMainWinList[TASK_ID_MAX];  //main window pointer table.
    UINT32  CodeIDList[TASK_ID_MAX];        //task modules code id table.

    TASK_ARG Arg;

}TASK;

/*
--------------------------------------------------------------------------------

  Description:  variables redefinition

--------------------------------------------------------------------------------
*/
extern TASK Task;

/*-----------------------------------------------------------------------------*/
extern WIN* TaskInit(void **pArg);
extern void TaskSwitch(TASK_ID TaskId, TASK_ARG *pTaskArg);
/*
********************************************************************************
*
*                         End of Task.h
*
********************************************************************************
*/
#endif
