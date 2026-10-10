/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   ASICTOUNICODE.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             zhuzhe      2013-11-13          1.0
*    desc:    ORG.
********************************************************************************
*/
//******************************************************************************
#define   _IN_ASICTOUNICODE_
#include "fsinclude.h"
#include "LcdInclude.h"
#include "ModuleOverlay.h"
#include "audio_main.h"

#define UNDEFINED_ASICIICODE 0x3000

_ATTR_SYS_BSS_ uint32 CP1251TabLogicAddress;
_ATTR_SYS_BSS_ uint32 CP932TabLogicAddress;
_ATTR_SYS_BSS_ uint32 CP932Tab1LogicAddress;
_ATTR_SYS_BSS_ uint32 CP932Tab2LogicAddress;
_ATTR_SYS_BSS_ uint32 CP950TabLogicAddress;
_ATTR_SYS_BSS_ uint32 CP949TabLogicAddress;
_ATTR_SYS_BSS_ uint32 CP949Tab1LogicAddress;

/*
--------------------------------------------------------------------------------
  Function name : UINT32 Gbk2Unicode(UINT32 code)
  Author        : yangwenjie
  Description   : GBK coding turn to unicode

  Input     code : GBK code
  Return         :

  History:     <author>         <time>         <version>
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_ID3_TEXT_
UINT8 LcdCharCodePage[LANGUAGE_MAX_COUNT] =
{
    FONT_CODEPAGE_CP936,    //LANGUAGE_CHINESE_S               0      //Simplified Chinese ��������
    FONT_CODEPAGE_CP950,    //LANGUAGE_CHINESE_T               1      //traditional chinese ��������
    FONT_CODEPAGE_CP936,    //LANGUAGE_ENGLISH                 2      //english
    FONT_CODEPAGE_CP949,    //LANGUAGE_KOREAN                  3      //korean
    FONT_CODEPAGE_CP936,    //LANGUAGE_JAPANESE                4      //japanese
    FONT_CODEPAGE_CP1252,   //LANGUAGE_SPAISH                  5      //spanish
    FONT_CODEPAGE_CP1252,   //LANGUAGE_FRENCH                  6      //french
    FONT_CODEPAGE_CP1252,   //LANGUAGE_GERMAN                  7      //german
    FONT_CODEPAGE_CP1252,   //LANGUAGE_ITALIAN                 8      //italian
    FONT_CODEPAGE_CP1252,   //LANGUAGE_PORTUGUESE              9      //portuguess
    FONT_CODEPAGE_CP1251,   //LANGUAGE_RUSSIAN                10      //russian
    FONT_CODEPAGE_CP1252,   //LANGUAGE_SWEDISH                11      //swedish
    FONT_CODEPAGE_CP874,    //LANGUAGE_THAI                   12      //thai
    FONT_CODEPAGE_CP1250,   //LANGUAGE_POLAND                 13      //polish
    FONT_CODEPAGE_CP1252,   //LANGUAGE_DENISH                 14      //danish
    FONT_CODEPAGE_CP1252,   //LANGUAGE_DUTCH                  15      //dutch
    FONT_CODEPAGE_CP1253,   //LANGUAGE_HELLENIC               16      //greek
    FONT_CODEPAGE_CP1250,   //LANGUAGE_CZECHIC                17      //czech
    FONT_CODEPAGE_CP1254,   //LANGUAGE_TURKIC                 18      //trukish
    FONT_CODEPAGE_CP1255,   //LANGUAGE_RABBINIC               19      //hebrew
    FONT_CODEPAGE_CP1252,   //LANGUAGE_ARABIC                 20      //arabic
    //LANGUAGE_MAX_COUNT              21
};



/*
--------------------------------------------------------------------------------
  Function name : UINT32 Gbk2Unicode(UINT32 code)
  Author        : yangwenjie
  Description   : GBK coding turn to unicode

  Input     code : GBK code
  Return         :

  History:     <author>         <time>         <version>
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_SYS_INIT_CODE_
void GetAsicTableAdd(void)
{
    uint32 TabLen;
    //DEBUG("GetAsicTableAdd");
    GetBeepSourceInf( MODULE_CP1251_UNICODE_TABLE,&CP1251TabLogicAddress,&TabLen);
    GetBeepSourceInf( MODULE_CP932_UNICODE_TABLE, &CP932TabLogicAddress, &TabLen);


    GetBeepSourceInf( MODULE_CP932_UNICODE_TABLE1,&CP932Tab1LogicAddress,&TabLen);
    GetBeepSourceInf( MODULE_CP932_UNICODE_TABLE2,&CP932Tab2LogicAddress,&TabLen);
    GetBeepSourceInf( MODULE_CP950_UNICODE_TABLE, &CP950TabLogicAddress, &TabLen);
    //DEBUG("the LENTH IS %x",TabLen);
    GetBeepSourceInf( MODULE_CP949_UNICODE_TABLE, &CP949TabLogicAddress, &TabLen);
    GetBeepSourceInf( MODULE_CP949_UNICODE_TABLE1,&CP949Tab1LogicAddress,&TabLen);
}

//_ATTR_ID3_TEXT_
UINT16 CP932toUnicode(UINT16 CP932Code,UINT16 codemode)
{
    uint16 UniCode;
    uint16 CP932UserCode;
    uint8  UniCode8[2];

    DEBUG("the cp932 code == %x",CP932Code);
    if(codemode ==1)
    {
        Lcdchang(&CP932Code,2);

        if(CP932Code <= 0xfc4b)
        {
            if(CP932Code >0xeefc && CP932Code <0xfa40 )
                UniCode = 0x0000;
            else if(CP932Code <= 0xeefc)
            {
                ModuleOverlayLoadData(CP932Tab1LogicAddress + (CP932Code - 0x8140)*2,(uint32)UniCode8,2);
                UniCode =((UniCode8[1] << 8) | UniCode8[0]) ;
            }
            else
            {
                CP932UserCode = CP932Code - 0xfa40;
                if(CP932UserCode > 0x0200)
                {
                    CP932UserCode = CP932UserCode - 0x86;
                    ModuleOverlayLoadData(CP932Tab2LogicAddress + (CP932UserCode)*2,(uint32)UniCode8,2);
                    UniCode =((UniCode8[1] << 8) | UniCode8[0]) ;
                }
                else if((CP932UserCode > 0x0100)  && (CP932UserCode < 0x01bd))
                {
                    CP932UserCode = CP932UserCode - 0x43;
                    ModuleOverlayLoadData(CP932Tab2LogicAddress + (CP932UserCode)*2,(uint32)UniCode8,2);
                    UniCode =((UniCode8[1] << 8) | UniCode8[0]) ;
                }
                else if(CP932UserCode  < 0x00bd)
                {
                    ModuleOverlayLoadData(CP932Tab2LogicAddress + (CP932UserCode)*2,(uint32)UniCode8,2);
                    UniCode =((UniCode8[1] << 8) | UniCode8[0]) ;
                }
                else
                    UniCode = 0;
             }
        }
        else
            UniCode = 0x0000;
    }
    else
    {
        if(CP932Code < 0X7F)
            UniCode = CP932Code;
        else
        {
            ModuleOverlayLoadData(CP932TabLogicAddress + (CP932Code - 0x80)*2,(uint32)UniCode8,2);
            UniCode =((UniCode8[1] << 8) | UniCode8[0]) ;
        }
    }
    DEBUG("the unicode == %x",UniCode);
    if((CP932Code != 0x0000) && (UniCode == 0x0000))
    UniCode = UNDEFINED_ASICIICODE;
    return UniCode;
}


//_ATTR_ID3_TEXT_
UINT16 CP949toUnicode(UINT16 CP949Code,UINT16 codemode)
{
    uint16 UniCode;
    uint8  UniCode8[2];

    if(codemode == 1)
    {
        Lcdchang(&CP949Code,2);
        if(CP949Code <= 0xfdfe)
        {
            if((CP949Code >= 0x8141) && (CP949Code <= 0xc8fe))
            {
                ModuleOverlayLoadData(CP949TabLogicAddress + (CP949Code - 0x8141)*2,(uint32)UniCode8,2);
                UniCode =((UniCode8[1] << 8) | UniCode8[0]) ;
            }
            else if(CP949Code >= 0xcaa1)
            {
                ModuleOverlayLoadData(CP949Tab1LogicAddress + (CP949Code - 0xcaa1)*2,(uint32)UniCode8,2);
                UniCode =((UniCode8[1] << 8) | UniCode8[0]) ;
            }
        }
        else
            UniCode = 0x0000;
    }
    else
    {
        if(CP949Code <0x80)
            UniCode = CP949Code;
        else
            UniCode = 0x00;
    }

    if((CP949Code != 0x0000) && (UniCode == 0x0000))
        UniCode = UNDEFINED_ASICIICODE;
    return UniCode;
}
//_ATTR_ID3_TEXT_
UINT16 CP950toUnicode(UINT16 CP950Code,UINT16 codemode)
{
    uint16 UniCode;
    uint8 UniCode8[2];

    if(codemode == 1)
    {
        Lcdchang(&CP950Code,2);
        if(CP950Code <= 0xf9fe)
        {
            DEBUG("the CP950Code is %x",CP950Code);
            if(CP950Code >= 0xa140)
            {
                //UniCode = CP950Tab[CP950Code - 0xa140];
                ModuleOverlayLoadData(CP950TabLogicAddress + (CP950Code - 0xa140)*2,(uint32)UniCode8,2);
                UniCode =((UniCode8[1] << 8) | UniCode8[0]) ;
            }
            else
                UniCode = 0x0000;
        }
        else
        {
            UniCode = 0x0000;
        }
    }
    else
    {
        if(CP950Code <0x80)
            UniCode = CP950Code;
        else
            UniCode = 0x00;
    }

    if((CP950Code != 0x0000) && (UniCode == 0x0000))
      UniCode = UNDEFINED_ASICIICODE;
    return UniCode;
}

//_ATTR_ID3_TEXT_
UINT16 CP1251toUnicode(UINT16 CP1251Code,UINT16 codemode)
{
    uint16 UniCode;
    uint8 UniCode8[2];

    if(CP1251Code >= 0x80)
    {
        ModuleOverlayLoadData(CP1251TabLogicAddress+(CP1251Code - 0x80)*2,(uint32)UniCode8,2);
        UniCode =UniCode =((UniCode8[1] << 8) | UniCode8[0]);
    }
    else
    {
        UniCode = CP1251Code;
    }
    if((CP1251Code != 0x0000) && (UniCode == 0x0000))
        UniCode = UNDEFINED_ASICIICODE;
    return UniCode;
}
//_ATTR_ID3_TEXT_
UINT16 CP1252toUnicode(UINT16 code,UINT16 codemode)
{
 uint16 unicode;
 unicode = code;
 return unicode;
}

//_ATTR_ID3_TEXT_
UINT16 GetCP932CodeType(UINT8 GbkCode, UINT8 GbkCode1)
{
 if (((0x81 <= GbkCode) && (GbkCode <= 0x9f))||( (0xe0 <= GbkCode) && (GbkCode <= 0xFc)) && (0x40 <= GbkCode1) && (GbkCode1 <= 0xFE))
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
//_ATTR_ID3_TEXT_
UINT16 GetCP949CodeType(UINT8 GbkCode, UINT8 GbkCode1)
{
    if ((0x81 <= GbkCode) && (GbkCode <= 0xFD) && (0x41 <= GbkCode1) && (GbkCode1 <= 0xFE))
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
//_ATTR_ID3_TEXT_
UINT16 GetCP950CodeType(UINT8 GbkCode, UINT8 GbkCode1)
{
    if ((0xA1 <= GbkCode) && (GbkCode <= 0xF9) && (0x40 <= GbkCode1) && (GbkCode1 <= 0xFE))
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
the japan is cp932
the korean is cp949
the big5 chinese_t is cp950
the russian is cp1251
the euro is cp1252
*/
//_ATTR_ID3_TEXT_
UINT16 Gbk2Unicode(UINT16 code,UINT16 codemode)
{
    UINT32 CharInNFAddr;
    UINT16   DispCharBuf[2];
    UINT32 ContUnicodeNfa;
    UINT32   iCodePage;
    UINT16  OriginalCode;

    OriginalCode = code;
    iCodePage = (uint32)(LcdCharCodePage[Language]);
    ContUnicodeNfa = GBKLogicAddress + (UINT32)(iCodePage)*128*1024;

    if(1==codemode)
    {
        Lcdchang(&code,2);
        CharInNFAddr = ContUnicodeNfa + (UINT32)(((UINT32)(code))<<1);

        LcdGetResourceData(CharInNFAddr, (UINT8*)DispCharBuf, 2);
    }
    else
    {
        CharInNFAddr = ContUnicodeNfa + (UINT32)(((UINT32)(code))<<1);
        LcdGetResourceData(CharInNFAddr, (UINT8*)DispCharBuf, 2);
    }

    //if(code!=0 && DispCharBuf[0] == 0)DispCharBuf[0]=0x3000;
    //if cbk can not find unicode ,should try big5 zhuzhe
    if(code!=0 && DispCharBuf[0] == 0)
       {
        //DEBUG("the gbk code is %x,the code mode is %x",OriginalCode,codemode);
        DispCharBuf[0] = CP950toUnicode(OriginalCode,codemode);
       }
    return(DispCharBuf[0]);
}
