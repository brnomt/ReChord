/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                      All rights reserved.
*
* File Name£º  RadioSubFreqWin.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               WangBo         2009-3-20         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef  _RADIOSUBFREQWIN_H
#define  _RADIOSUBFREQWIN_H

#undef  EXT
#ifdef  _IN_RADIOSUBFREQWIN
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/
#define _ATTR_RADIOSUBFREQWIN_CODE_         
#define _ATTR_RADIOSUBFREQWIN_DATA_         
#define _ATTR_RADIOSUBFREQWIN_BSS_          



//SubMenu Display macro define
#define     RADIO_SUB_FM_DISNUM             2    //fm tune number

#define     RADIO_SUB_LIST_TXT_X            27   //display fm frequency list table
#define     RADIO_SUB_LIST_TXT_Y            88   //
#define     RADIO_SUB_LIST_TXT_XSIZE        104
#define     RADIO_SUB_LIST_TXT_YSIZE        16
#define     RADIO_SUB_LIST_TXT_YSTEP        12

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
extern void RadioSubFreqWinIntInit(void);
extern void RadioSubFreqWinIntDeInit(void);
extern void RadioSubFreqWinMsgInit(void);
extern void RadioSubFreqWinMsgDeInit(void);
extern void RadioSubFreqWinInit(void);
extern void RadioSubFreqWinDeInit(void);
extern UINT32 RadioSubFreqWinService(void);
extern UINT32 RadioSubFreqWinKey(void);
extern void RadioSubFreqWinDisplay(void);

/*
--------------------------------------------------------------------------------

  Description:  window sturcture definition

--------------------------------------------------------------------------------
*/
#ifdef _IN_RADIOSUBFREQWIN
_ATTR_RADIOSUBFREQWIN_DATA_ WIN RadioSubFreqWin =
{

    NULL,
    NULL,

    RadioSubFreqWinService,               //window service handle function.
    RadioSubFreqWinKey,                   //window key service handle function.
    RadioSubFreqWinDisplay,               //window display service handle function.

    RadioSubFreqWinInit,                  //window initial handle function.
    RadioSubFreqWinDeInit                 //window auti-initial handle function.

};
#else
_ATTR_RADIOSUBFREQWIN_DATA_ EXT WIN RadioSubFreqWin;
#endif

/*
********************************************************************************
*
*                         End of RADIOSUBWIN.h
*
********************************************************************************
*/
#endif




