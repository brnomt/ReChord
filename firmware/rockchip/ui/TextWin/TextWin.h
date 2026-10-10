/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   TextDisplay.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*              yangwenjie         2009-3-2          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _TEXTDISPLAY_H_
#define _TEXTDISPLAY_H_

#undef  EXT
#ifdef _IN_TEXTDISPLAY_
#define EXT
#else
#define EXT extern
#endif
/*
*-------------------------------------------------------------------------------
*
*                            book definition
*
*-------------------------------------------------------------------------------
*/

#define _ATTR_TEXT_CODE_                    
#define _ATTR_TEXT_DATA_                    
#define _ATTR_TEXT_BSS_                     

#define _ATTR_TEXT_INIT_CODE_               
#define _ATTR_TEXT_INIT_DATA_               
#define _ATTR_TEXT_INIT_TBSS_               

#define _ATTR_TEXT_DEINIT_CODE_             
#define _ATTR_TEXT_DEINIT_DATA_             
#define _ATTR_TEXT_DEINIT_BSS_              

#define _ATTR_TEXT_SERVICE_CODE_            
#define _ATTR_TEXT_SERVICE_DATA_            
#define _ATTR_TEXT_SERVICE_BSS_             

/*
*-------------------------------------------------------------------------------
*
*                            Macro define
*
*-------------------------------------------------------------------------------
*/
#define     TXT_DISP_LINE_MAX           8

#define     UNICODE_TEXT                1
#define     ANSI_TEXT                   2

#define     TXT_ZONE_LEFT               3
#define     TXT_ZONE_RIGHT              118
#define     TXT_ZONE_TOP                5
#define     TXT_ZONE_BOTTOM             138

#define     TXT_DISP_BUF_MAX            512

#define     TXT_OFFSET_BUF_MAX          180

#define     TEXT_KEY_UP_OFFSET          512

#define     TEXT_TITLE_DISPLAY_WIDTH    140

#define     DISP_MENU                   0

#define     TXT_SCROLL_BAR_COUNT        143-16  //97-15      //the move unit is 1,select bar length

#define     TEXT_INDEX_MAX              128
/*
--------------------------------------------------------------------------------
  
                        Struct Define
  
--------------------------------------------------------------------------------
*/ 

/*
--------------------------------------------------------------------------------
  
                        Variable Define
  
--------------------------------------------------------------------------------
*/

_ATTR_TEXT_BSS_ EXT UINT32              EbookStartFileOffset;                 
_ATTR_TEXT_BSS_ EXT UINT32              EbookEndFileOffset;

_ATTR_TEXT_BSS_ EXT UINT32              TextAutoPlayEnable;
_ATTR_TEXT_BSS_ EXT UINT16              TxtFileFormatError; 

_ATTR_TEXT_BSS_ EXT UINT32              TextAutoScanCount;
_ATTR_TEXT_BSS_ EXT UINT32              TextAutoScanCountBack;


//_ATTR_TEXT_BSS_ EXT UINT16              TextStartPageCounter;
_ATTR_TEXT_BSS_ EXT INT32               TextIndexBuf[TEXT_INDEX_MAX];
_ATTR_TEXT_BSS_ EXT INT32               TextIndex;
_ATTR_TEXT_BSS_ EXT UINT32              TextBufMinValue;

_ATTR_TEXT_BSS_ EXT UINT32              EbookPageCountOffset;
_ATTR_TEXT_BSS_ EXT UINT32              EbookTotalPages;

_ATTR_TEXT_BSS_ EXT UINT32              EbookCurrentPages;

_ATTR_TEXT_BSS_ EXT UINT32                gPreviousPageOffset;


/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/
EXT void TextKeyDownProc(void);
EXT void    TextKeyUpProc(uint8 lines);
EXT void    TextFileOpen(void);
EXT UINT16  TextCheckChar(UINT16 s);
EXT UINT16  TextGetCharInRect(LCD_RECT *r, UINT16 *pString);
EXT uint16  TextGetBytes(uint16 *GbkString, uint16 CharNumber);
EXT UINT16  TextGetNextLine (UINT16 *str, UINT32 iOffset, LCD_RECT *r, INT8 Handle);
EXT UINT32  TxtGetPreviousPageOffset (UINT32 iCurrentPageOffset, INT8 Handle);
EXT void    TextDisplay(void);
EXT UINT32  TextGetPreviousPageOffset (UINT32 iCurrentPageOffset, INT8 Handle);
EXT UINT32  TextWinKey(void);
EXT void    TextWinInit(void *pArg);
EXT void    TextWinDeInit(void);
EXT UINT32  TextWinService(void);
EXT void    TextWinPaint(void);
EXT void    TextPageCountProc(void);
EXT void    TextDisplayTitle (void);
EXT UINT16  TextPagePro(UINT32  CurrentPage,UINT32 TotalPage,UINT16 *DataBuffer);
EXT void TextDisplayCurrentPages (UINT32  CurrentPage);



/******************************************************************************
/
                                                                          
*/
/*                    Description:  window sturcture definition                            
*/
/*                                                                            
*/
/******************************************************************************
*/
#ifdef _IN_TEXTDISPLAY_
_ATTR_TEXT_DATA_ WIN TextMenuWin = {
    
    NULL,
    NULL,
    
    TextWinService,               //window service handle function.
    TextWinKey,                   //window key service handle function.
    TextWinPaint,               //window display service handle function.
    
    TextWinInit,                  //window initial handle function.
    TextWinDeInit                 //window auti-initial handle function.
    
};
#else
_ATTR_TEXT_DATA_ EXT WIN TextMenuWin;
#endif

/*
********************************************************************************
*
*                         End of  TextDisplay.h
*
********************************************************************************
*/
#endif
