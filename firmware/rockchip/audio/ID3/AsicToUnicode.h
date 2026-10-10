/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Asictounicode.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                zhuze             2013-11-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef     _ASICTOUNICODE_H_
#define     _ASICTOUNICODE_H_

#undef  EXT
#ifdef _IN_ASICTOUNICODE_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------
  
                        Macro define  
  
--------------------------------------------------------------------------------
*/



/*
--------------------------------------------------------------------------------
  
  Description:  LCD_CHAR structure definition.
  
--------------------------------------------------------------------------------
*/


/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/
EXT void GetAsicTableAdd(void);
EXT UINT16 CP932toUnicode(UINT16 code, UINT16 codemode);
EXT UINT16 CP949toUnicode(UINT16 code, UINT16 codemode);
EXT UINT16 CP950toUnicode(UINT16 CP950Code, UINT16 codemode);
EXT UINT16 CP1251toUnicode(UINT16 code, UINT16 codemode);
EXT UINT16 CP1252toUnicode(UINT16 code, UINT16 codemode);

EXT UINT16 GetCP932CodeType(UINT8 GbkCode, UINT8 GbkCode1);
EXT UINT16 GetCP949CodeType(UINT8 GbkCode, UINT8 GbkCode1);
EXT UINT16 GetCP950CodeType(UINT8 GbkCode, UINT8 GbkCode1);

/*
********************************************************************************
*
*                         End of LcdChar.c
*
********************************************************************************
*/


#endif


