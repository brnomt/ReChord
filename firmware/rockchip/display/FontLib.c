/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:Fontlib.c
*
* Description:
*
* History:      <author>          <time>        <version>
*    desc:    ORG.
********************************************************************************
*/
#include "FontLib.h"
#include "getThaiChar.h"
#include "stdint.h"
#include <string.h>

#include "SysConfig.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

extern uint8_t       SetMenuLanguageInfo[LANGUAGE_MAX_COUNT];
extern uint8_t       gLangSel;
//extern uint16_t FontCompTable8859[];
//extern uint16_t FontCompTableAsia[];

extern uint16_t FontCompTable8859(uint16_t);
extern uint16_t FontCompTableAsia(uint16_t);

#ifdef USEFONT_12dot
//extern uint8_t Font12_8859CompData[];
extern uint8_t Font12_8859CompData(uint8_t *, uint16_t);
//extern uint8_t Font12_8859Width[];
extern uint8_t Font12_8859Width(uint16_t);
//extern uint8_t Font12_JKC_CompData[];
extern uint8_t Font12_JKC_CompData(uint8_t *, uint16_t);
//extern uint8_t Font12_CJK_CompData[];
extern uint8_t Font12_CJK_CompData(uint8_t *, uint16_t);
#endif // USEFONT_12dot

#ifdef USEFONT_16dot
extern uint8_t Font16_8859CompData[];
extern uint8_t Font16_8859Width[];
extern uint8_t Font16_JKC_CompData[];
extern uint8_t Font16_CJK_CompData[];
#endif // USEFONT_16dot

#ifdef __cplusplus
}
#endif  /* __cplusplus */


uint8_t getFontSize(FONT_TYPE fontType) {
#ifdef USEFONT_12dot
    if ((fontType == TYPE12_CJK) || (fontType == TYPE12_JKC)) {
        return SIZE12;
    }
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
    if ((fontType == TYPE16_CJK) || (fontType == TYPE16_JKC)) {
        return SIZE16;
    }
#endif //USEFONT_16dot
    return 0; //error
}

void getFontData8859(uint8_t fontSize, struct FontInfo* pInfo,
                                    uint16_t offset, GET_INFOMODE getMode) {
    pInfo->height = fontSize;
#ifdef USEFONT_12dot
    if (fontSize == SIZE12) {
        if(getMode == MODE_ALLDATA) {
            //::memcpy(pInfo->buff, Font12_8859CompData+offset*BYTESIZE12,BYTESIZE12);
            Font12_8859CompData(pInfo->buff,offset);

        }
        //pInfo->width = Font12_8859Width[offset];
        pInfo->width = Font12_8859Width(offset);
        return;
    }
#endif //USEFONT_12dot

#ifdef USEFONT_16dot
    if (fontSize == SIZE16) {
        if(getMode == MODE_ALLDATA) {
            ::memcpy(pInfo->buff, Font16_8859CompData+offset*BYTESIZE16,BYTESIZE16);
        }
        pInfo->width = Font16_8859Width[offset];
        return;
    }
#endif //USEFONT_16dot

    return; //error
}

short getFontDataThai(uint8_t fontSize, struct FontInfo* pInfo,
                                    Ucs2* pUni, int16_t* pNum, GET_INFOMODE getMode) {
    short ref;
    short tempWidth, tempHeight;

    uint8_t * buff;

#ifdef USEFONT_12dot
    if (fontSize == SIZE12) {

        if(getMode == MODE_ALLDATA)
        {
            buff = pInfo->buff;
        }
        else
        {
            buff = NULL;
        }

        ref = getThaiCharImage(THAI_TYPE_12, (unsigned short*)pUni, buff,
                                &tempWidth, &tempHeight, (short*)pNum);
        //pInfo->width  = static_cast<uint8_t>( tempWidth);
        //pInfo->height = static_cast<uint8_t>( tempHeight);
        pInfo->width  = (uint8_t)tempWidth;
        pInfo->height = (uint8_t)tempHeight;
        return ref;
    }
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
    if (fontSize == SIZE16) {
        ref = getThaiCharImage(THAI_TYPE_16, (unsigned short*)pUni, pInfo->buff,
                                &tempWidth, &tempHeight, (short*)pNum);
        pInfo->width  = static_cast<uint8_t>( tempWidth);
        pInfo->height = static_cast<uint8_t>( tempHeight);
        return ref;
    }
#endif //USEFONT_16dot
    return -1; //error
}

void getFontDataAsia(FONT_TYPE fontType, uint8_t fontSize,
            struct FontInfo* pInfo, uint16_t offset, GET_INFOMODE getMode) {

    pInfo->height = fontSize;
    pInfo->width  = fontSize;

    if(getMode == MODE_ALLDATA) {
#ifdef USEFONT_12dot
        if (fontSize == SIZE12) {
                if(fontType == TYPE12_CJK) {
                    //::memcpy(pInfo->buff, Font12_CJK_CompData+offset*BYTESIZE12,BYTESIZE12);
                    Font12_CJK_CompData(pInfo->buff,offset);
                } else {
                    //::memcpy(pInfo->buff, Font12_JKC_CompData+offset*BYTESIZE12,BYTESIZE12);
                    Font12_JKC_CompData(pInfo->buff,offset);
                }
                return;
        }
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
        if (fontSize == SIZE16) {
                if(fontType == TYPE16_CJK) {
                    ::memcpy(pInfo->buff, Font16_CJK_CompData+offset*BYTESIZE16,BYTESIZE16);
                } else {
                    ::memcpy(pInfo->buff, Font16_JKC_CompData+offset*BYTESIZE16,BYTESIZE16);
                }
                return;
        }
#endif //USEFONT_16dot
    }
    return;
}

void exchageUniICDSpec(Ucs2* pUni) {
    uint16_t usCode = *pUni;

    if(usCode == 0x005C){
        //CONFIG_LANGUAGE_MODE lang;
        //ConfigDatabaseIf::GetInstance()->GetLanguageMode(&lang);
        //if( CONFIG_LANGUAGE_MODE_JAPANESE == lang) {
            //usCode = 0x00A5;
        //}
        if(gLangSel == SetMenuLanguageInfo[LANGUAGE_JAPANESE])
        {//if lang is JAPANESE  "?��?��"
            usCode = 0x00A5;
        }
//        else if(gLangSel == SetMenuLanguageInfo[LANGUAGE_KOREAN])
//        {//if lang is Korea   "?"
//            usCode = 0x20a9;
//        }
        else
            ;
    }

    if( usCode == 0x201A ){
        usCode = 0x002C;
    }else if( usCode == 0x2039 ){
        usCode = 0x003C;
    }else if( usCode == 0x2022 ){
        usCode = 0x30FB;
    }else if( usCode == 0x203A ){
        usCode = 0x003E;
    }

    *pUni = usCode;
    return;
}

Ucs2* getFontData(FONT_TYPE fontType, Ucs2* pUni,
                            struct FontInfo* pInfo, GET_INFOMODE getMode) {
    const uint16_t TOFU_OFFSET = 0x023E;
    Ucs2 usCode = *pUni;
    uint16_t offset_Adr;
    short nResult;
    const uint8_t fontSize = getFontSize(fontType);

    if (fontSize == 0) {
        return NULL;
    }

    exchageUniICDSpec(&usCode);

    if((0x0020 <= usCode) && (usCode <= 0x2116)) {
        offset_Adr = FontCompTable8859((usCode - 0x0020));
        if( offset_Adr != 0xFFFF ){
            getFontData8859(fontSize, pInfo, offset_Adr, getMode);
           return pUni+1;
        }
    }

    if(((0x0E01 <= usCode) && (usCode <= 0x0E3A)) || ((0x0E3F <= usCode) && (usCode <= 0x0E5B))) {
        int16_t nNum;
        nResult = getFontDataThai(fontSize, pInfo, pUni, &nNum, getMode);
        //BaseLoggerPrintf("Thai Width = %u Height = %u\n", pInfo->width, pInfo->height);
        if( nResult != THAI_ERR_INVALID_PARAM ){
            if( nResult == THAI_ERR_INVALID_GRIF ){
                pInfo->width = 0;
            }
            return pUni+nNum;
        }
    }

    offset_Adr = FontCompTableAsia(usCode);

    if( offset_Adr != 0x0000 ) {
            offset_Adr = offset_Adr - 1;
            getFontDataAsia(fontType, fontSize, pInfo, offset_Adr, getMode);

           if((0xFF61 <= usCode) && (usCode <= 0xFF9F)) {
               pInfo->width = fontSize/2;
            }
           return pUni+1;
    }

    getFontDataAsia(fontType, fontSize, pInfo, TOFU_OFFSET-1, getMode);
    return pUni+1;
}
