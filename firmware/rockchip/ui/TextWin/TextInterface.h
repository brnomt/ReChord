/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   TextInterface.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*              yangwenjie         2009-3-2          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _TEXTINTERFACE_H_
#define _TEXTINTERFACE_H_

#undef  EXT
#ifdef _IN_TEXTINTERFACE_
#define EXT
#else
#define EXT extern
#endif
/*
*-------------------------------------------------------------------------------
*
*                            Macro define
*
*-------------------------------------------------------------------------------
*/

#define     MAX_PATH_NAME_LENGTH            (2+MAX_DIR_DEPTH*(11+1)+4)  // 单位: 字节.
#define     MAX_FILE_NAME_LENGTH             12                          // 单位: 字节.
#define     MAX_LONG_FILE_NAME_LENGTH       ((MAX_LFN_ENTRIES * 13 * 2 + 2)/2)  // 单位: 字节.



/*
--------------------------------------------------------------------------------
  
                        Struct Define
  
--------------------------------------------------------------------------------
*/
typedef struct TXT_DISP_BUF_STRUCT{
    uint16    BytesPerLine;
    uint16    TxtDispBuf[TXT_PER_LINE_CHARS];
    struct          TXT_DISP_BUF_STRUCT *pPerv;
    struct          TXT_DISP_BUF_STRUCT *pNext;
}TxtDispBufStruct;


typedef struct 
{
    UINT32  CodeType;
    UINT32  FileSize;
    UINT32  StartFileOffset;
    TxtDispBufStruct    *pTxtBuf;
    UINT8   FilePath[MAX_PATH_NAME_LENGTH];                // ebook file path
    UINT8   FileName[MAX_FILE_NAME_LENGTH];                // ebook short file name
    UINT16  LongFileName[MAX_LONG_FILE_NAME_LENGTH];    // ebook long file name         
}TXT_FILE_INFO;
/*
--------------------------------------------------------------------------------
  
                        Variable Define
  
--------------------------------------------------------------------------------
*/
#define     gTxtAutoBrowserTime             gSysConfig.TextConfig.AutoPlayTime
#define     gTxtAutoBrowserOn               gSysConfig.TextConfig.AutoPlay
#define     TextBatteryLevel                gBattery.Batt_Level
#define     TextHoldState                0



_ATTR_TEXT_BSS_ EXT TxtDispBufStruct    TxtBuf[TXT_DISP_LINE_MAX];
_ATTR_TEXT_BSS_ EXT TXT_FILE_INFO       TextFileInfo;  //read out the file information from browser
_ATTR_TEXT_BSS_ EXT HANDLE              hTextFile;
_ATTR_TEXT_BSS_ EXT SYS_FILE_INFO       TxtSysFileInfo;
_ATTR_TEXT_BSS_ EXT UINT16              TextTitleInfo[MAX_LONG_FILE_NAME_LENGTH+9];



/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/
EXT void TextGetResourceData(UINT32 Addr,UINT8*pData,UINT16 Length);
EXT UINT16  TextCat(UINT16*pSrc,UINT16 *pDes,UINT16 pStrNum );
EXT void TextChang(UINT16*pSrc,UINT16 len);


/*
********************************************************************************
*
*                         End of  TextInterface.h
*
********************************************************************************
*/
#endif
