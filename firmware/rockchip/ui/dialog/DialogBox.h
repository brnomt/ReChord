/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:    DIALOGBOX.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               anzhiguo      2009-2-27         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef  _DIALOG_BOX_H
#define  _DIALOG_BOX_H

#undef  EXT
#ifdef  _IN_DIALOG_BOX
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
#define _ATTR_DIALOGBOX_CODE_         
#define _ATTR_DIALOGBOX_DATA_         
#define _ATTR_DIALOGBOX_BSS_          

#define DIALOG_BUTTON_YES             1//4.22 an 
#define DIALOG_BUTTON_NO              0//4.22 an 

//对话框文字显示位置
#define     DIALOG_TEXT_TITLE_X             41-12
#define     DIALOG_TEXT_TITLE_Y             50//30+5
#define     DIALOG_TEXT_TITLE_XSIZE         90
#define     DIALOG_TEXT_TITLE_YSIZE         12

#define     DIALOG_TEXT_CONTENT_X           28
#define     DIALOG_TEXT_CONTENT_Y           65//50
#define     DIALOG_TEXT_CONTENT_XSIZE       90
#define     DIALOG_TEXT_CONTENT_YSIZE       12

#define     DIALOG_BUTTON_YES_X             44-16
#define     DIALOG_BUTTON_YES_Y             68+15//68
#define     DIALOG_BUTTON_YES_XSIZE         30
#define     DIALOG_BUTTON_YES_YSIZE         12

#define     DIALOG_BUTTON_NO_X              89-16
#define     DIALOG_BUTTON_NO_Y              68+15//68
#define     DIALOG_BUTTON_NO_XSIZE          30
#define     DIALOG_BUTTON_NO_YSIZE          12

/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/
_ATTR_DIALOGBOX_BSS_    EXT UINT16    ButtonSel;
_ATTR_DIALOGBOX_BSS_    EXT UINT16    DialogBoxTitleID;
_ATTR_DIALOGBOX_BSS_    EXT UINT16    DialogBoxContentID;

/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
extern void DialogInit(void *pArg);
extern void DialogDeInit(void);
extern UINT32 DialogService(void);
extern UINT32 DialogKey(void);
extern void DialogDisplay(void);

/*
--------------------------------------------------------------------------------
  
  Description:  window structrue definition
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_DIALOG_BOX
_ATTR_DIALOGBOX_DATA_ WIN DialogWin = {
    
    NULL,
    NULL,
    
    DialogService,               //dialog window service function
    DialogKey,                   //dialog window key service function
    DialogDisplay,               //dialog window display service function
    
    DialogInit,                  //dialog window initial function
    DialogDeInit                 //dialog window auti-initial function
    
};
#else 
_ATTR_DIALOGBOX_DATA_ EXT WIN DialogWin;
#endif

/*
********************************************************************************
*
*                         End of DIALOGBOX.h
*
********************************************************************************
*/
#endif

