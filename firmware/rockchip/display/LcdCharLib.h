/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   LcdCharLib.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _LCDCHARLIB_H_
#define _LCDCHARLIB_H_

#undef  EXT
#ifdef  _IN_LCDCHARLIB_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/

extern UINT16 IsArabicLetter(UINT16 uUnicode);
extern UINT16 IsHebrewLetter(UINT16  uUnicode);
extern UINT32 IsBidirectionalString(UINT16*   pStr);
extern UINT32 IsLetter(UINT32 uAsciiChar);
extern UINT32 GetAsciiLength(UINT16   *Str);
extern UINT16 BidirectionalStringPro(UINT16*src, UINT16* dest);
extern void   ConTime2String(INT16 Sign,   UINT16 Hour, UINT16 Minute, UINT16 Second,UINT16 *pStr);
extern UINT16 GetGbkCodeType(UINT8 GbkCode, UINT8 GbkCode1);
extern UINT16 Check_Special_TaiWen(UINT16 c);
extern UINT32 LCD_GetStringNum(UINT16 *s);
extern void   Lcdchang(UINT16*pSrc,UINT16 len);

#else

typedef UINT16 (*pIsArabicLetter)(UINT16 uUnicode);
typedef UINT16 (*pIsHebrewLetter)(UINT16  uUnicode);
typedef UINT32 (*pIsBidirectionalString)(UINT16*   pStr);
typedef UINT32 (*pIsLetter)(UINT32 uAsciiChar);
typedef UINT32 (*pGetAsciiLength)(UINT16   *Str);
typedef UINT16 (*pBidirectionalStringPro)(UINT16*src, UINT16* dest);
typedef void   (*pConTime2String)(INT16 Sign,   UINT16 Hour, UINT16 Minute, UINT16 Second,UINT16 *pStr);
typedef UINT16 (*pGetGbkCodeType)(UINT8 GbkCode, UINT8 GbkCode1);
typedef UINT16 (*pCheck_Special_TaiWen)(UINT16 c);
typedef UINT32 (*pLCD_GetStringNum)(UINT16 *s);
typedef void   (*pLcdchang)(UINT16*pSrc,UINT16 len);

#define IsArabicLetter(uUnicode)                         (((pIsArabicLetter        )(Addr_IsArabicLetter        ))(uUnicode))
#define IsHebrewLetter(uUnicode)                         (((pIsHebrewLetter        )(Addr_IsHebrewLetter        ))(uUnicode))
#define IsBidirectionalString(pStr)                      (((pIsBidirectionalString )(Addr_IsBidirectionalString ))(pStr))
#define IsLetter(uAsciiChar)                             (((pIsLetter              )(Addr_IsLetter              ))(uAsciiChar))
#define GetAsciiLength(tr)                               (((pGetAsciiLength        )(Addr_GetAsciiLength        ))(tr))
#define BidirectionalStringPro(src, dest)                (((pBidirectionalStringPro)(Addr_BidirectionalStringPro))(src, dest))
#define ConTime2String(Sign, Hour, Minute, Second, pStr) (((pConTime2String        )(Addr_ConTime2String        ))(Sign, Hour, Minute, Second, pStr))
#define GetGbkCodeType(GbkCode, GbkCode1)                (((pGetGbkCodeType        )(Addr_GetGbkCodeType        ))(GbkCode, GbkCode1))
//#define Check_Special_TaiWen(c)                          (((pCheck_Special_TaiWen  )(Addr_Check_Special_TaiWen  ))(c))
#define LCD_GetStringNum(s)                              (((pLCD_GetStringNum      )(Addr_LCD_GetStringNum      ))(s))
#define Lcdchang(pSrc, len)                              (((pLcdchang              )(Addr_Lcdchang              ))(pSrc, len))
extern UINT16 Check_Special_TaiWen(UINT16 c);

#endif

/*
********************************************************************************
*
*                         End of LcdCharLib.h
*
********************************************************************************
*/


