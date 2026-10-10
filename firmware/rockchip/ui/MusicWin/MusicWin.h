/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   AudioWin.h
*
* Description:
*
* History:      <author>          <time>        <version>
*                Chenwei           2009-02-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _MUSICWIN_H_
#define _MUSICWIN_H_
#undef  EXT
#ifdef _IN_MUSICWIN_
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
//music menu permanent code.
#define _ATTR_MUSIC_CODE_         
#define _ATTR_MUSIC_DATA_         
#define _ATTR_MUSIC_BSS_          

//music menu initial code
#define _ATTR_MUSIC_INIT_CODE_    
#define _ATTR_MUSIC_INIT_DATA_    
#define _ATTR_MUSIC_INIT_BSS_     

//music  menu auti-initial code
#define _ATTR_MUSIC_DEINIT_CODE_  
#define _ATTR_MUSIC_DEINIT_DATA_  
#define _ATTR_MUSIC_DEINIT_BSS_   

//music menu dispatch code
#define _ATTR_MUSIC_SERVICE_CODE_ 
#define _ATTR_MUSIC_SERVICE_DATA_ 
#define _ATTR_MUSIC_SERVICE_BSS_  

/*----------------------------------------------------------------------------------------*/
//music playing interface display type.
#define MUSIC_DISPLAY_NOR                   0               //normal display interface
#define MUSIC_DISPLAY_LRC                   1               //display LRC interface

//music playing interface dialog,message box type.
#define MUSICWIN_DIALOG_MESSAGE_NULL        0
#define MUSICWIN_DIALOG_NOFILE              1
#define MUSICWIN_DIALOG_FORMAT_ERROR        2

#define MUSIC_TIME_GUAGE_CONST          (121-7)//47            //total item number of progress
#define MUSIC_SCROLL_WIDTH                  (126 - 26)    //character scroll width,if exclude this width,it will scroll

/*
********************************************************************************
*
*                         Variable Define
*
********************************************************************************
*/
_ATTR_MUSIC_BSS_ EXT   UINT8               Spetrum;
_ATTR_MUSIC_BSS_ EXT   UINT32              MusicTimeGuageVal;  //guage progress
_ATTR_MUSIC_BSS_ EXT   UINT8               MusicDispType;
_ATTR_MUSIC_BSS_ EXT   UINT8               MusicWinDialogType;
_ATTR_MUSIC_BSS_ EXT   UINT8               MusicBgFlag;// 1:have LCR,0: NO,do not need refresh screen.
_ATTR_MUSIC_BSS_ EXT   UINT16              MediaTitleAdd;
//_ATTR_MUSIC_BSS_ EXT   UINT32              CurrentTimeSecBk;
_ATTR_MUSIC_BSS_ EXT   UINT32              CurrentTime200MsBk;
_ATTR_MUSIC_BSS_ EXT   UINT16              MusicPlayStateFF;
//_ATTR_MUSIC_BSS_ EXT   UINT32              MusicStepTime;
_ATTR_MUSIC_BSS_ EXT   bool                SelFromNowPlay;
#define MUSIC_SCHED_BUF_SIZE                (1*7*2)
#define MUSIC_SCHED_BUF_NUM                 2
_ATTR_MUSIC_BSS_ EXT UINT8                  MusicSchedBuffer[MUSIC_SCHED_BUF_NUM * MUSIC_SCHED_BUF_SIZE];
_ATTR_MUSIC_BSS_ EXT PICTURE_INFO_STRUCT    MusicSchedPicInfo;

/*
********************************************************************************
*
*                         Functon Declaration
*
********************************************************************************
*/
extern void MusicWinInit(void *pArg);
extern void MusicWinDeInit(void);

extern void MusicWinIntInit(void);
extern void MusicWinIntDeInit(void);
extern void MusicWinMsgInit(void *pArg);
extern void MusicWinMsgDeInit(void);
extern UINT32 MusicWinService(void);
extern void MusicWinPaint(void);
extern UINT32 MusicWinKeyProc(void);
extern void MusicWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed);
extern void MusicDisplayFileNums(UINT16 CutternFileNum,UINT16 TotalPageFileNum);
extern void MusicDisplayBitrate(UINT32 Bitrate);
extern void MusicDisplayTime(UINT16 Hour,UINT16 Min,UINT16 Sec,UINT16 mode);

/*
********************************************************************************
*
*                         Description:  window sturcture definition
*
********************************************************************************
*/
#ifdef _IN_MUSICWIN_
_ATTR_MUSIC_DATA_ WIN MusicWin = {

    NULL,
    NULL,

    MusicWinService,                    //window service handle function.
    MusicWinKeyProc,                    //window key service handle function.
    MusicWinPaint,                      //window display service handle function.

    MusicWinInit,                       //window initial handle function.
    MusicWinDeInit                      //window auti-initial handle function.

};
#else
_ATTR_MUSIC_DATA_ EXT WIN MusicWin;
#endif

/*
********************************************************************************
*
*                         End of AudioWin.h
*
********************************************************************************
*/
#endif


