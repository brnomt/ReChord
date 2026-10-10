/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Fontlib.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*    desc:    ORG.
********************************************************************************
*/
#ifndef __Font_Lib_H__
#define __Font_Lib_H__

#include "stdint.h"
#include "FontUseSetting.h"


typedef enum  {
#ifdef USEFONT_12dot
		BYTESIZE12  = 24,
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
		BYTESIZE16  = 32,
#endif //USEFONT_16dot
	}BYTESIZE;

typedef enum FONTSIZE {
#ifdef USEFONT_12dot
			SIZE12  = 12,
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
			SIZE16  = 16,
#endif //USEFONT_16dot
	};

typedef enum  {
#ifdef USEFONT_12dot
			TYPE12_CJK,
			TYPE12_JKC,
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
			TYPE16_CJK,
			TYPE16_JKC,
#endif //USEFONT_16dot
	}FONT_TYPE;

typedef enum{
		   MODE_SIZEONLY,
		   MODE_ALLDATA,
	}GET_INFOMODE;

	struct FontInfo {
		uint8_t  width;
		uint8_t  height;
#ifdef USEFONT_16dot
		uint8_t  buff[BYTESIZE16];
#else
		uint8_t  buff[BYTESIZE12];
#endif //USEFONT_16dot
	};

uint8_t getFontSize(FONT_TYPE fonType);
void    getFontData8859(uint8_t fonSize, struct FontInfo* pInfo,
								uint16_t offset, GET_INFOMODE getMode);
short   getFontDataThai(uint8_t fontSize, struct FontInfo* pInfo,
								Ucs2* pUni, int16_t* pNum, GET_INFOMODE getMode);
void    getFontDataAsia(FONT_TYPE fontType, uint8_t fontSize,struct FontInfo* pInfo,
								uint16_t offset, GET_INFOMODE getMode);
void    exchageUniICDSpec(Ucs2* pUni);

Ucs2* getFontData(FONT_TYPE fonType, Ucs2* pUni, struct FontInfo* pInfo, GET_INFOMODE getMode);

#endif // __Font_Lib_H__

