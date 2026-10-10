
#include <string.h>
#include "FontUseSetting.h"
#include "getThaiChar.h"
#include "stdint.h"


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define _D3CODE_THAI    0x0E33
#define _EDCODE_THAI    0x0E4D
#define _D2CODE_THAI    0x0E32

#define ROUNDUP(x)  (((x)+7)/8)

#ifdef USEFONT_12dot
extern unsigned char FontAdr_THAI12[];
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
extern unsigned char FontAdr_THAI16[];
#endif //USEFONT_16dot

typedef struct {
    short   nType;
    short   nWidth;
    short   nHeight;
    short   nDataWidth;
    short   nDataHeight;
    unsigned char *pFontData;
} ThaiFontInfo;

#ifdef USEFONT_12dot
static const ThaiFontInfo _thaiFontInfo12[] = {
    {THAI_TYPE_12,
THAI_TYPE_12_WIDTH, 
THAI_TYPE_12_HEIGHT, 
THAI_TYPE_12_DATA_WIDTH, 
THAI_TYPE_12_DATA_HEIGHT, 
FontAdr_THAI12}
};
#endif //USEFONT_12dot

#ifdef USEFONT_16dot
static const ThaiFontInfo _thaiFontInfo16[] = {
    {THAI_TYPE_16,
THAI_TYPE_16_WIDTH, 
THAI_TYPE_16_HEIGHT, 
THAI_TYPE_16_DATA_WIDTH, 
THAI_TYPE_16_DATA_HEIGHT, 
FontAdr_THAI16} 
};
#endif //USEFONT_16dot

static ThaiFontInfo* _getThaiCharInfo(short nType);
static unsigned short _checkCharTHAI(unsigned short nUnicode, short nD3ChangeOverlay);
static short _checkCharTHAI2(unsigned short *pString);
static short _checkOverlayCharTHAI(unsigned short nAscii);
static void _copyFontDataTHAI(unsigned char *pDst, unsigned char *pSrc, short nSize, short nBitOR);
static unsigned short _getType1FontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode);
static unsigned short _getType2FontNumTHAI(unsigned short nCode);
static short _getAddFontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode);
static short _execOverlayTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode,
                              unsigned char *pFontData, short nDataSize, unsigned char *pImage);


#define TRUE    1
#define FALSE   0
ThaiFontInfo* _getThaiCharInfo(short nType)
{
    switch( nType ){
#ifdef USEFONT_12dot
    case THAI_TYPE_12:
        return (ThaiFontInfo*)&_thaiFontInfo12[0];
#endif //USEFONT_12dot
#ifdef USEFONT_16dot
    case THAI_TYPE_16:
        return (ThaiFontInfo*)&_thaiFontInfo16[0];
#endif //USEFONT_16dot
    default:
        return NULL;
    }
}

static unsigned short _checkCharTHAI(unsigned short nUnicode, short nD3ChangeOverlay)
{
    if( (nUnicode >= 0x0E01 && nUnicode <= 0x0E3A) || (nUnicode >= 0x0E3F && nUnicode <= 0x0E5B) ){
        if( nUnicode == _D3CODE_THAI ){
            if( nD3ChangeOverlay == TRUE ){
                nUnicode = _EDCODE_THAI;
            }else{
                nUnicode = _D2CODE_THAI;
            }
        }
        return (nUnicode - 0x0D60);
    }else{
        return 0x0;
    }
}

static short _checkCharTHAI2(unsigned short *pString)
{
    if( *pString >= 0x0E48 && *pString <= 0x0E4B ){
        switch( *(++pString) ){
        case 0x0E4D:    
        case 0x0E33:    
            return TRUE;
        default:
            return FALSE;
        }
    }else{
        return FALSE;
    }
}

static short _checkOverlayCharTHAI(unsigned short nAscii)
{
    if( (nAscii == 0xD1) ||
        (nAscii >= 0xD4 && nAscii <= 0xDA) ||
        (nAscii >= 0xE7 && nAscii <= 0xEE) ){
        return TRUE;
    }else{
        return FALSE;
    }
}

static void _copyFontDataTHAI(unsigned char *pDst, unsigned char *pSrc, short nSize, short nBitOR)
{
    short i;

    uint32_t CharInNFAddr;
    uint8_t Src[24];
    
    CharInNFAddr = FontLogicAddress + pSrc - _thaiFontInfo12[0].pFontData + FONT_TAIWEN_START;  

    if( nBitOR == FALSE ){
        //memcpy( pDst, pSrc, nSize );

        LcdGetResourceData(CharInNFAddr, pDst, FONT_TAIWEN_LEN);        
        
    }else{

        LcdGetResourceData(CharInNFAddr, Src, FONT_TAIWEN_LEN);     
        
        for( i = 0; i < nSize; i++ ){
            pDst[i] |= Src[i];
        }
    }
}


#define THAI_TYPE1_SELECT_NUM   4
#define THAI_TYPE1_FONT_NUM     13

typedef struct {
    unsigned short nCode;   
    unsigned short nFontNum[THAI_TYPE1_SELECT_NUM]; 
} ThaiFontNumSelect;

static const ThaiFontNumSelect _thaiFontNumSelect[THAI_TYPE1_FONT_NUM] = {
    {0xD1, {0x31, 0x60, 0x00, 0x00}},   /* [00] */
    {0xD4, {0x34, 0x61, 0x00, 0x00}},   /* [01] */
    {0xD5, {0x35, 0x62, 0x00, 0x00}},   /* [02] */
    {0xD6, {0x36, 0x63, 0x00, 0x00}},   /* [03] */
    {0xD7, {0x37, 0x64, 0x00, 0x00}},   /* [04] */
    {0xE7, {0x47, 0x65, 0x00, 0x00}},   /* [05] */
    {0xE8, {0x48, 0x68, 0x70, 0x78}},   /* [06] */
    {0xE9, {0x49, 0x69, 0x71, 0x79}},   /* [07] */
    {0xEA, {0x4A, 0x6A, 0x72, 0x7A}},   /* [08] */
    {0xEB, {0x4B, 0x6B, 0x73, 0x7B}},   /* [09] */
    {0xEC, {0x4C, 0x6C, 0x74, 0x7C}},   /* [10] */
    {0xED, {0x4D, 0x6D, 0x00, 0x00}},   /* [11] */
    {0xEE, {0x4E, 0x6E, 0x00, 0x00}}    /* [12] */
};

static unsigned short _getType1FontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode)
{
    short i;
    unsigned short nFontNum = 0;
    unsigned short nCondition = 0x0;
    const ThaiFontNumSelect *pTable = NULL;

    for( i = 0; i < THAI_TYPE1_FONT_NUM; i++ ){
        if( _thaiFontNumSelect[i].nCode == nCode ){
            pTable = &_thaiFontNumSelect[i];
            break;
        }
    }

    if( pTable == NULL ){
        return 0;
    }

    switch( nL1Code ){
    case 0xBB:
    case 0xBD:
    case 0xBF:
    case 0xCB:
        nCondition = 0x10;
        break;
    default:
        break;
    }

    switch( nBfCode ){
    case 0xD1:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
    case 0xE7:
    case 0xED:
        nCondition |= 0x01;
        break;
    default:
        break;
    }

    switch( nCondition ){
    case 0x00:
        nFontNum = pTable->nFontNum[0];
        break;
    case 0x10:
        nFontNum = pTable->nFontNum[1];
        break;
    case 0x01:
        nFontNum = pTable->nFontNum[2];
        break;
    case 0x11:
        nFontNum = pTable->nFontNum[3];
        break;
    default:
        break;
    }

    return nFontNum;
}

static unsigned short _getType2FontNumTHAI(unsigned short nCode)
{
    unsigned short nFontNum = 0;

    switch( nCode ){
    case 0xD8:
        nFontNum = 0x38;
        break;
    case 0xD9:
        nFontNum = 0x39;
        break;
    case 0xDA:
        nFontNum = 0x3A;
        break;
    default:
        break;
    }

    return nFontNum;
}

static short _getAddFontNumTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode)
{
    unsigned short nFontNum = 0;

    switch( nCode ){
    case 0xD1:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
    case 0xE7:
    case 0xE8:
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
        nFontNum = _getType1FontNumTHAI( nCode, nL1Code, nBfCode );
        break;

    case 0xD8:
    case 0xD9:
    case 0xDA:
        if( nL1Code == 0xAD ){
            nFontNum = 0x8000;
        }else if( nL1Code == 0xB0 ){
            nFontNum = 0x8100;
        }
        nFontNum |= _getType2FontNumTHAI( nCode );
        break;

    default: 
        break;
    }

    return nFontNum;
}

static short _execOverlayTHAI(unsigned short nCode, unsigned short nL1Code, unsigned short nBfCode,
                              unsigned char *pFontData, short nDataSize, unsigned char *pImage)
{
    unsigned short nAddFont = 0;
    unsigned short nRefFont = 0;
    unsigned char *pData = NULL;

    nAddFont = _getAddFontNumTHAI( nCode, nL1Code, nBfCode );
    if( nAddFont == 0x0 ){
        return FALSE;
    }

    if( pImage ){
        nRefFont = (nAddFont >> 8) & 0xFF;
        if( nRefFont != 0 ){
            pData = pFontData + nRefFont * nDataSize;
            _copyFontDataTHAI( pImage, pData, nDataSize, FALSE );
            nAddFont &= 0xFF;
        }

        pData = pFontData + nAddFont * nDataSize;
        _copyFontDataTHAI( pImage, pData, nDataSize, TRUE );
    }

    return TRUE;
}

short getThaiCharSize( short nType, short *pnWidth, short *pnHeight )
{
    ThaiFontInfo *pInfo = _getThaiCharInfo( nType );
    if( pInfo == NULL ){
        *pnWidth  = 0;
        *pnHeight = 0;
        return FALSE;
    }else{
#ifdef THAI_FONT_BYTEUNIT
        *pnWidth  = pInfo->nWidth;
        *pnHeight = pInfo->nHeight;
#else
        *pnWidth  = pInfo->nDataWidth;
        *pnHeight = pInfo->nDataHeight;
#endif
        return TRUE;
    }
}

short getThaiCharImage(short nType, unsigned short* pString, unsigned char *pImage, short *pnWidth, short *pnHeight, short *pnNum)
{
    unsigned short nCode = 0;
    unsigned short nL1Code = 0;
    unsigned short nNxCode = 0;
    unsigned short nBfCode = 0;
    unsigned short nDataSize = 0;
    ThaiFontInfo *pInfo = NULL;  
    unsigned char *pFontData = NULL;

    *pnNum = 0;
    *pnWidth = 0;
    *pnHeight = 0;

    if( (pInfo = _getThaiCharInfo( nType )) == NULL ){
        return THAI_ERR_INVALID_PARAM;
    }else{
#ifdef THAI_FONT_BYTEUNIT
        *pnWidth  = pInfo->nWidth;
        *pnHeight = pInfo->nHeight;
#else
        *pnWidth  = pInfo->nDataWidth;
        *pnHeight = pInfo->nDataHeight;
#endif
    }

    if( (nCode = _checkCharTHAI( *pString, FALSE )) == 0x0 ){
        return THAI_ERR_INVALID_PARAM;
    }

    *pnNum = 1;

    nDataSize = ROUNDUP((pInfo->nDataWidth) * (pInfo->nDataHeight));

    if( _checkOverlayCharTHAI( nCode ) == TRUE ){
        /* *pnWidth = 0;*/
        return THAI_ERR_INVALID_GRIF;
    }

    if( pImage ){
        pFontData = pInfo->pFontData + ((nCode - 0xA0) * nDataSize);
        _copyFontDataTHAI( pImage, pFontData, nDataSize, FALSE );
    }
    
    nL1Code = nBfCode = nCode;

    while(1){
        nNxCode = *(pString + *pnNum);

        if( nNxCode == 0x0 ){
            goto Exit;
        }

        if( (nNxCode = _checkCharTHAI( nNxCode, TRUE )) == 0x0 ){
            goto Exit;
        }

        if( _checkCharTHAI2( (pString + *pnNum) ) == TRUE ){
            if( _execOverlayTHAI( 0xED, nL1Code, nBfCode, pInfo->pFontData, nDataSize, pImage ) == FALSE ){
            goto Exit;
        }
            nBfCode = 0xED;
            }

        if( _execOverlayTHAI( nNxCode, nL1Code, nBfCode, pInfo->pFontData, nDataSize, pImage ) == FALSE ){
            goto Exit;
        }
        
        nBfCode = nNxCode;


        if( *(pString + *pnNum) == _D3CODE_THAI ){
            goto Exit;
        }


        *pnNum = *pnNum + 1;
    }

Exit:
    return THAI_ERR_NONE;
}


short getThaiCharNum(short nType, unsigned short* pString, short *pnWidth, short *pnHeight, short *pnNum)
{
    short nRet = getThaiCharImage(nType, pString, NULL, pnWidth, pnHeight, pnNum);
    return nRet;
}

#ifdef __cplusplus
};
#endif  /* __cplusplus */
