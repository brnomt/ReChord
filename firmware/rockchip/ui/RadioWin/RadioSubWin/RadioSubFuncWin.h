/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                         All rights reserved.
*
* File Name£º  RadioSubFuncWin.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               WangBo         2009-3-20         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef  _RADIOSUBFUNCWIN_H
#define  _RADIOSUBFUNCWIN_H

#undef  EXT
#ifdef  _IN_RADIOSUBFUNCWIN
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
#define _ATTR_RADIOSUBFUNCWIN_CODE_         
#define _ATTR_RADIOSUBFUNCWIN_DATA_         
#define _ATTR_RADIOSUBFUNCWIN_BSS_          

//#define     RADIO_STATE_MANUAL              0
//#define     RADIO_STATE_PRESET              1
//#define     RADIO_STATE_SAVE                2
//#define     RADIO_STATE_DEL                 3
//#define     RADIO_STATE_AUTO                4

//SubMenu Display macro define

#define     RADIO_SUB_MENU_TXT_YSTEP        16   // child menu display singal step length

#define     RADIO_SCREEN_PER_LINE           8   // lines number of one screen.
#define     RADIO_SUB_MENU_TXT_X            34
#define     RADIO_SUB_MENU_TXT_Y            146
#define     RADIO_SUB_MENU_TXT_XSIZE        61
#define     RADIO_SUB_MENU_TXT_YSIZE        12

typedef enum _eFMUIUpdateType
{
    UPDATE_ALL,    
    UPDATE_UP,
    UPDATE_DOWN,
    
}FMUIUpdateType;

//page
/*_ATTR_RADIOSUBFUNCWIN_BSS_*/  EXT UINT16   FmKeyCounter;
_ATTR_RADIOSUBFUNCWIN_BSS_  EXT UINT16   PreFmKeyCounter;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/
typedef struct _FMBRO_STRUCT{
    struct _FMBRO_STRUCT *pPrev;
    struct _FMBRO_STRUCT *pNext;
    UINT16  ItemSidNumber;
}FMBRO_STRUCT;

EXT _ATTR_RADIOSUBFUNCWIN_BSS_ FMBRO_STRUCT          FMBroItem[RADIO_SCREEN_PER_LINE];
EXT _ATTR_RADIOSUBFUNCWIN_BSS_ FMBRO_STRUCT          *pFMBro;

/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
extern void RadioSubFuncWinIntInit(void);
extern void RadioSubFuncWinIntDeInit(void);
extern void RadioSubFuncWinMsgInit(void);
extern void RadioSubFuncWinMsgDeInit(void);
extern void RadioSubFuncWinInit(void);
extern void RadioSubFuncWinDeInit(void);
extern UINT32 RadioSubFuncWinService(void);
extern UINT32 RadioSubFuncWinKey(void);
extern void RadioSubFuncWinDisplay(void);

/*
--------------------------------------------------------------------------------

  Description:  window sturcture definition

--------------------------------------------------------------------------------
*/
#ifdef _IN_RADIOSUBFUNCWIN
_ATTR_RADIOSUBFUNCWIN_DATA_ WIN RadioSubFuncWin =
{

    NULL,
    NULL,

    RadioSubFuncWinService,               //window service handle function.
    RadioSubFuncWinKey,                   //window key service handle function.
    RadioSubFuncWinDisplay,               //window display service handle function.

    RadioSubFuncWinInit,                  //window initial handle function.
    RadioSubFuncWinDeInit                 //window auti-initial handle function.

};
#else
_ATTR_RADIOSUBFUNCWIN_DATA_ EXT WIN RadioSubFuncWin;
#endif

/*
********************************************************************************
*
*                         End of RADIOSUBWIN.h
*
********************************************************************************
*/
#endif



