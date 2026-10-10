/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  DIALOGBOX.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-2-27         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef  _MESSAGE_BOX_H
#define  _MESSAGE_BOX_H

#undef  EXT
#ifdef  _IN_MESSAGE_BOX
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
#define _ATTR_MESSAGEBOX_CODE_         
#define _ATTR_MESSAGEBOX_DATA_         
#define _ATTR_MESSAGEBOX_BSS_          

//message box text display position
#define     MESSAGE_TEXT_TITLE_X             29
#define     MESSAGE_TEXT_TITLE_Y             50 //35
#define     MESSAGE_TEXT_TITLE_XSIZE         90
#define     MESSAGE_TEXT_TITLE_YSIZE         12

#define     MESSAGE_TEXT_CONTENT_X           20
#define     MESSAGE_TEXT_CONTENT_Y           65 //62
#define     MESSAGE_TEXT_CONTENT_XSIZE       90
#define     MESSAGE_TEXT_CONTENT_YSIZE       12
#define     MESSAGE_DISPLAY_TIME             3//message box display 5 second.

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/
_ATTR_MESSAGEBOX_BSS_   EXT UINT16    MessageBoxTitleID;
_ATTR_MESSAGEBOX_BSS_   EXT UINT16    MessageBoxContentID;
_ATTR_MESSAGEBOX_BSS_   EXT UINT32    MessageBoxSystickCounterBack;
_ATTR_MESSAGEBOX_BSS_   EXT UINT16    MessageBoxHoldTime;

_ATTR_MESSAGEBOX_BSS_   EXT UINT16    MessageBoxCurDisFrameIndex;
_ATTR_MESSAGEBOX_BSS_   EXT UINT16    MessageBoxUnsupportFrameIndex;

_ATTR_MESSAGEBOX_BSS_   EXT void     (*MessageBoxHook)(void);

/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
extern void MessageBoxInit(void *pArg);
extern void MessageBoxDeInit(void);
extern uint32 MessageBoxService(void);
extern uint32 MessageBoxKey(void);
extern void MessageBoxDisplay(void);

/*
--------------------------------------------------------------------------------

  Description:  window sturcture definition

--------------------------------------------------------------------------------
*/
#ifdef _IN_MESSAGE_BOX
_ATTR_MESSAGEBOX_DATA_ WIN MessageBoxWin = {

    NULL,
    NULL,

    MessageBoxService,               //window service handle function.
    MessageBoxKey,                   //window key service handle function.
    MessageBoxDisplay,               //window display service handle function.

    MessageBoxInit,                  //window initial handle function.
    MessageBoxDeInit                 //window auti-initial handle function.

};
#else
_ATTR_MESSAGEBOX_DATA_ EXT WIN MessageBoxWin;
#endif

/*
********************************************************************************
*
*                         End of DIALOGBOX.h
*
********************************************************************************
*/
#endif

