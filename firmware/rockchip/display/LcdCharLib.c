/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   LcdCharLib.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             yangwenjie      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#include "SysConfig.h"
#include "typedef.h"
#include "macro.h"
#include "Driverconfig.h"

#define _IN_LCDCHARLIB_

/*
--------------------------------------------------------------------------------
  Function name :UINT32 IsHebrewLetter (UINT32   uUnicode)
  Author        : yangwenjie
  Description   : judge whether a charactor is a alphabet.
                   
  Input       
  Return           

  History:     <author>         <time>         <version>       
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT16 IsArabicLetter (UINT16 uUnicode)
{
    UINT16 bIsArabic;

    if (((uUnicode >= 0x600) && (uUnicode <= 0x6FF))
        || ((uUnicode >= 0xA00) && (uUnicode <= 0xA7F))
        || ((uUnicode >= 0x1980) && (uUnicode <= 0x19DF))
        || ((uUnicode >= 0x1A00) && (uUnicode <= 0x1AFF)))
    {
        bIsArabic = 1;
    }
    else
    {						  
        bIsArabic = 0;
    }
    
    return bIsArabic;
}

/*
--------------------------------------------------------------------------------
  Function name :UINT32 IsHebrewLetter (UINT32   uUnicode)
  Author        : yangwenjie
  Description   : to judge whether is two direction charactor.
                   
  Input       
  Return           

  History:     <author>         <time>         <version>       
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT16 IsHebrewLetter (UINT16  uUnicode)
{
    UINT16 bIsHebrew;

    if (((uUnicode >=  0x5B0) && (uUnicode <= 0x5B9)) ||
        ((uUnicode  >= 0x5BB) && (uUnicode <= 0x5C3)) ||
        ((uUnicode  >= 0x5F0) && (uUnicode <= 0x5F4)) ||
        ((uUnicode  >= 0x5D0) && (uUnicode <= 0x5EA))) 
 	{ 
         bIsHebrew  = 1;
    } 
	else 
	{
         bIsHebrew  = 0;
    }
        
    return bIsHebrew;
}

/*
--------------------------------------------------------------------------------
  Function name :UINT32 IsHebrewLetter (UINT32   uUnicode)
  Author        : yangwenjie
  Description   :to judge whether is two direction string.
                   
  Input       
  Return           

  History:     <author>         <time>         <version>       
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 IsBidirectionalString (UINT16*   pStr)
{
    UINT32 bIsBidirectionalString;

    bIsBidirectionalString = 0;

    while ('\0' != (*pStr))
	{
        if (1 == IsHebrewLetter(*pStr)) 
        {
            bIsBidirectionalString = 1;
            break;
        }
        pStr++;
    }
        
    return bIsBidirectionalString;
  
}

/*
--------------------------------------------------------------------------------
  Function name :UINT32 IsLetter   (UINT32 uAsciiChar)
  Author        : yangwenjie
  Description   : judge whether is letter or digit.
                   
  Input       
  Return           

  History:     <author>         <time>         <version>       
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 IsLetter   (UINT32 uAsciiChar)
{
    UINT32 bIsLetter;

    if (((uAsciiChar >= 'A') && (uAsciiChar <= 'Z')) || 
        ((uAsciiChar >= 'a') && (uAsciiChar <= 'z')) ||
        ((uAsciiChar >= '0') && (uAsciiChar <= '9'))) {
        
        bIsLetter = 1;
        
    } else {
        
        bIsLetter = 0;
    }
        
    return bIsLetter;
}

/*
--------------------------------------------------------------------------------
  Function name :UINT32 IsLetter   (UINT32 uAsciiChar)
  Author        : yangwenjie
  Description   :
                   
  Input       
  Return           

  History:     <author>         <time>         <version>       
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 GetAsciiLength (UINT16   *Str)
{
    INT16   AsciiLength;
    UINT16* pStr;
    
    AsciiLength =   0;
    pStr        =   Str;
    
    while (0 == IsHebrewLetter(*pStr)) {
    
        AsciiLength++;
        pStr++;
    }
    
    pStr--;
    while (0 == IsLetter (*pStr)) {
    
        AsciiLength--;
        pStr--;
    }
    
    if (AsciiLength < 0) {
    
        AsciiLength = 0;
    }

    return AsciiLength;
}


/*
--------------------------------------------------------------------------------
  Function name : void ConTime2String(INT16  Sign, UINT16 Hour, UINT16 Minute, UINT16 Second,UINT16 *pStr)
  Author        : yangwenjie
  Description   : 
                   
  Input        Sign    : 
               Hour    : 
               Minute  : 
               Second  : 
               pStr    : 
  Return               : 

  History:     <author>         <time>         <version>       
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_    
UINT16 BidirectionalStringPro (UINT16*src, UINT16* dest)
{
   UINT16* pSource;
   UINT16* pDest;
   UINT16* pSourcePre ;//= NULL;
   UINT16* pSourceback; //= NULL;
   UINT16  StrLen =0;
   
   pSourcePre=NULL;
   pSourceback=NULL;
   pSource = src;
   pDest = dest ;
   while('\0' != (*pSource))
   {
       if(IsHebrewLetter(*pSource)==0)
       {
         if((pSourcePre!=NULL))
         {  
            if((*pSource ==' ')||(*pSource =='-'))
            {
                pSource++;  
            }
            else
            {
			     pSourceback = pSource;

    			pSource--;

                while(pSourcePre!=pSource)
                {
                    *pDest = *pSource;
                     pSource--;
    				 pDest++;
                     StrLen++;
                }
                pSource = pSourceback;  
    			pSourcePre = NULL;  

            }
         }
         else
         {
             *pDest = *pSource;
             pSource++;
             pDest++ ; 
             StrLen++;
         }
       }
       else
       {
			if(pSourcePre==NULL)
				pSourcePre = pSource;
			pSource++;            
       }
   }

   dest[StrLen] = '\0';

   return StrLen;
}

/*
--------------------------------------------------------------------------------
  Function name : void ConTime2String(INT16  Sign, UINT16 Hour, UINT16 Minute, UINT16 Second,UINT16 *pStr)
  Author        : yangwenjie
  Description   : 
                   
  Input        Sign    : 
               Hour    : 
               Minute  :
               Second  :
               pStr    : 
  Return               : 

  History:     <author>         <time>         <version>       
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void ConTime2String(INT16 Sign,   UINT16 Hour, UINT16 Minute, UINT16 Second,UINT16 *pStr)
{
    if (Sign < 0) {
      
        *pStr++ =  '-';
    }
    
    *pStr++  =  Hour/100+'0';
    if (Hour/100 == 0) *pStr--;
        Hour = Hour % 100;
    *pStr++  =  Hour/10+'0';
    *pStr++  =  Hour%10+'0';
    *pStr++  =  ':';

    *pStr++  =  Minute/10+'0';
    *pStr++  =  Minute%10+'0';
    *pStr++  =  ':';
    *pStr++  =  Second/10+'0';
    *pStr++  =  Second%10+'0';
    *pStr++  =  0;  
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 TxtGbk2Unicode(UINT32 *pSbuf, UINT32 *pTbuf, UINT32 Len)
  Author        : yangwenjie
  Description   : 
                   
  Input         :                
  Return        : 

  History:     <author>         <time>         <version>       
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT16 GetGbkCodeType(UINT8 GbkCode, UINT8 GbkCode1)
{   
    if ((0x81 <= GbkCode) && (GbkCode <= 0xFE) && (0x40 <= GbkCode1) && (GbkCode1 <= 0xFE)) 
    {
        if((0x00<=GbkCode)&&(GbkCode<=0x7F))
        {
            return 1;
        }
        else
        {
           return 2;
        }
    }
                
    return 1;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 Check_Special_TaiWen(UINT32 c)
  Author        : yangwenjie
  Description   : judge whether is thai.
                   
  Input      
  Return       

  History:     <author>         <time>         <version>       
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT16 Check_Special_TaiWen(UINT16 c)
{
    // �����̩�ĵ� �ϱ� ���� �±�, ����Ϊ 0.
    if((0x0E34<=c && c<=0x0E3A)||
       (0x0E47<=c && c<=0x0E4E)||
       (c==0x0E08)||
       (c==0x0E31)||
       (c==0x0E25))
    {
	   return c;
    
    } 
    else 
    {     
       return   0;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : INT32   LCD_GetStringSize(UINT32 *s)
  Author        : yangwenjie
  Description   :
                   
  Input         :  s       :  
      
  Return        :  

  History:     <author>         <time>         <version>       
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 LCD_GetStringNum(UINT16 *s) {
    UINT32    CharSize = 0;
    while(*s){
        CharSize++;
        s++;
    }
    return(CharSize);
}

/*
--------------------------------------------------------------------------------
  Function name :void Lcd_chang(UINT16*pSrc,UINT16 len)
  Author        : yangwenjie
  Description   : exchande the high 8 bits and low 8 bits
                  
  Input         : pSrc      ��source buffer that will be changed.
                  len       ��the length of string pointer by pSrc.

  Return        : null 

  History:     <author>         <time>         <version>       
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void Lcdchang(UINT16*pSrc,UINT16 len)
{
   UINT16 i;
   UINT16 temp;
   
   for(i=0;i<len/2;i++)
   {
      temp = (pSrc[i]<<8);
      pSrc[i] = temp|(pSrc[i]>>8);
   }
} 

_ATTR_DRIVER_CODE_
UINT16 Check_TaiWen(UINT16 c)
{
  // �����̩�ĵ� �ϱ� ���� �±�, ����Ϊ 0.
    if((c>=0x0E01&&c<=0x0E3A)||(c>=0x0E3f&&c<=0x0E5B)) {
	 
       return c;
    
    } else {     
    
       return   0;
    }
}
/*
********************************************************************************
*
*                         End of LcdCharLib.c
*
********************************************************************************
*/