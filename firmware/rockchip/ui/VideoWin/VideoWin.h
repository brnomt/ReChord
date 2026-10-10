/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   VideoWin.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                Chenwei           2009-02-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _VIDEOWIN_H_
#define _VIDEOWIN_H_

#undef  EXT
#ifdef _IN_VIDEOWIN_
#define EXT
#else
#define EXT extern
#endif

/*
********************************************************************************
*
*                        Macro Define 
*
********************************************************************************
*/
//section define
#define _ATTR_VIDEOWIN_CODE_         
#define _ATTR_VIDEOWIN_DATA_         
#define _ATTR_VIDEOWIN_BSS_          

EXT UINT8               VideoWinDialogType;
#define             VIDEOWIN_DIALOG_NOFILE              1
#define             VIDEOWIN_DIALOG_FORMAT_ERROR        2
_ATTR_VIDEOWIN_BSS_ EXT UINT32				VideoTimeGuageVal;
_ATTR_VIDEOWIN_BSS_ EXT UINT32				VideoTimeSecBack;

#define VideoBatteryLevel               gBattery.Batt_Level

//bar Buffer
#define VIDEO_SCHED_BUF_SIZE                    (1*7*2)
#define VIDEO_SCHED_BUF_NUM                     2
_ATTR_VIDEOWIN_BSS_ EXT UINT8                   VideoSchedBuffer[VIDEO_SCHED_BUF_SIZE * VIDEO_SCHED_BUF_NUM];
_ATTR_VIDEOWIN_BSS_ EXT PICTURE_INFO_STRUCT     VideoSchedPicInfo;

/*
********************************************************************************
*
*                         Functon Declaration
*
********************************************************************************
*/
extern void VideoWinMsgInit(void);
extern void VideoWinInit(void *pArg);
extern void VideoWinDeInit(void);
extern UINT32 VideoWinService(void);
extern UINT32 VideoWinKeyProc(void);
extern void VideoWinPaint(void);

/*
********************************************************************************
*
*                         Description:  window sturcture definition
*
********************************************************************************
*/
#ifdef _IN_VIDEOWIN_
_ATTR_VIDEOWIN_DATA_ WIN VideoWin = {
    
    NULL,
    NULL,
    
    VideoWinService,                    //window service handle function.
    VideoWinKeyProc,                    //window key service handle function.
    VideoWinPaint,                      //window display service handle function.
    
    VideoWinInit,                       //window initial handle function.
    VideoWinDeInit                      //window auti-initial handle function.
    
};
#else 
_ATTR_VIDEOWIN_DATA_ EXT WIN VideoWin;
#endif
#endif


