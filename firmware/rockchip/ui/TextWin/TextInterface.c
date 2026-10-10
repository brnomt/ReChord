/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��  TextInterface.c
* 
* Description:  ebook module
*
* History:      <author>          <time>        <version>       
*             yangwenjie          2009-3-2        1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_TEXTINTERFACE_

#include "SysInclude.h"
#ifdef _EBOOK_

#include "fsinclude.h"
#include "SysFindFile.h"
#include "TextWin.h"
#include "TextInterface.h"
#include "TextBookmark.h"


/*
--------------------------------------------------------------------------------
  Function name :void TextChang(UINT16*pSrc,UINT16 len)
  Author        : yangwenjie
  Description   : exchange the high 8 bits and low 8 bits
                  
  Input         : pSrc      ��buffer that will be turn,
                  len       ��buffer length

  Return        : null 

  History:     <author>         <time>         <version>       
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
void TextChang(UINT16*pSrc,UINT16 len)
{
   UINT16 i;
   UINT16 temp;

   for(i=0;i<len/2;i++)
   {
      temp = (pSrc[i]<<8);
      pSrc[i] = temp|(pSrc[i]>>8);
   }
}

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
UINT16 TextCat(UINT16*pSrc,UINT16 *pDes,UINT16 pStrNum)
{
   UINT16 CharpDesNum = 0;
   UINT16 temp;

   while(*pDes)
   {
		*(pSrc+pStrNum) =  *pDes;
        pSrc++;
        pDes++;
        CharpDesNum++;
		if((CharpDesNum+pStrNum)>MAX_BOOK_MARK_NAME_LENGTH-2)
        {
            break;
        }

   }
   return(CharpDesNum+pStrNum);
}

/*
--------------------------------------------------------------------------------
  Function name : void TextGetResourceData(UINT32 Addr,UINT8*pData,UINT16 Length)
  Author        : yangwenjie
  Description   : get text resource data.
                  
  Input         : Addr      ��Flash resource address
                  Buffer    ��charater Buffer
				  Length    ��get data lenght
  Return        : null 

  History:     <author>         <time>         <version>       
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
void TextGetResourceData(UINT32 Addr,UINT8*pData,UINT16 Length)
{
     ReadResourceData(Addr,pData,Length);
     TextChang((UINT16*)pData,Length);
}

/*
********************************************************************************
*
*                         End of TextInterface.c
*
********************************************************************************
*/
#endif
