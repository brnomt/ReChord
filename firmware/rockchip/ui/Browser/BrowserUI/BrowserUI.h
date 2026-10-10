/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  BrowserUI.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               chenfen          2008-3-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _BROWSER_UI_H
#define _BROWSER_UI_H

#undef EXT
#ifdef _IN_BROSWER_UI_
    #define EXT
#else
    #define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/
//section define
//brower window permanent code.
#define _ATTR_BRO_UI_CODE_         
#define _ATTR_BRO_UI_DATA_         
#define _ATTR_BRO_UI_BSS_          

//------------------------------------------------------------------------------
//related interface
#define     BROWSER_SCREEN_PER_LINE     8  // 3  added by chenz //2 oled screen
#define     BROWSER_SCROLL_BAR_COUNT    123

//------------------------------------------------------------------------------
#define     BROWSER_TITLE_TXT_X            34
#define     BROWSER_TITLE_TXT_Y            146
#define     BROWSER_TITLE_TXT_XSIZE        61
#define     BROWSER_TITLE_TXT_YSIZE        12

//------------------------------------------------------------------------------
//display mark
#define     BROW_DIS_NULL               0x0000
#define     BROW_DIS_BAR                0x0001
#define     BROW_DIS_ALL_ITEM           0x0002
#define     BROW_DIS_SELE_ITEM          0x0004
#define     BROW_DIS_BATT               0x0008
#define     BROW_DIS_HOLD               0x0010
#define     BROW_DIS_MEM_SEL            0x0020
#define     BROW_DIS_SCLLO_ITEM         0x0040
#define     BROW_DIS_ALL                (BROW_DIS_BAR|BROW_DIS_ALL_ITEM|BROW_DIS_BATT|BROW_DIS_HOLD)

//------------------------------------------------------------------------------
#define     MSG_MODULE_OK               0x0000
#define     MSG_MODULE_EXIT             0x0001
#define     MSG_MODULE_ERROR            0x0002
//------------------------------------------------------------------------------

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/

//data structure
//type is correspond with display icon and find file type,do not change order.
typedef enum _FileType
{                       //correspond icon   finding file type
    FileTypeFolder = 0, //diretion
    FileTypeAudio,      //music       AudioFileExtString
    FileTypeVideo,      //video        VideoFileExtString
    FileTypeText,       //text       PictureFileExtString
    FileTypePicture,    //picture        TextFileExtString
    FileTypeALL,        //unkown       ALLFileExtString
    FileTypeDisk1,      //disk
    FileTypeDisk2,      //SD card
    FileTypeNull,       //No File Type
} FileType;

typedef enum _BROW_TYPE
{
    BROW_FILE_FROM_DATABASE = 0,
    BROW_FILE_FROM_FS,

}BROW_TYPE;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/
//extenal variables interface
#define BrowserHoldState                    0
#define BrowserBatteryLevel                 gBattery.Batt_Level

//system permanent variables
_ATTR_BRO_UI_BSS_        EXT int16            gBrowserIsNeedInit;
_ATTR_BRO_UI_BSS_        EXT FileType         gBrowserFindFileType;
_ATTR_BRO_UI_BSS_        EXT INT16            gBrowserFildFileNum;
_ATTR_BRO_UI_BSS_        EXT INT16            gBrowSerFsType;

_ATTR_BRO_UI_BSS_        EXT INT16           gBrowserMemSelEn;
_ATTR_BRO_UI_BSS_        EXT INT16           gBrowserFileNum;

//UI variables
_ATTR_BRO_UI_BSS_       EXT INT16            BrowserMemSel;
_ATTR_BRO_UI_BSS_       EXT INT16            BrowserMemSelEn;//local vaiables of gBrowserMemSelEn,it is use to judge.
_ATTR_BRO_UI_BSS_       EXT uint8            gBrowserKeep;

_ATTR_BRO_UI_BSS_       EXT UINT16           PrintfCount;
_ATTR_BRO_UI_BSS_       EXT UINT16           PrintfADD;
_ATTR_BRO_UI_BSS_       EXT UINT16          *PrintfBuf;
_ATTR_BRO_UI_BSS_       EXT UINT16           PrintfScrollFlag;
_ATTR_BRO_UI_BSS_       EXT UINT16           LrcScrollFlag;

#ifdef _IN_BROSWER_UI_
    #if(defined (_MULT_DISK_)|| defined (_USB_HOST_))
    _ATTR_BRO_UI_DATA_       EXT UINT16          BrowserMem[3]={FLASH0,USBHOST_FLASH,CARD};
    #else
    _ATTR_BRO_UI_DATA_       EXT UINT16          BrowserMem[2]={FLASH0,CARD};
    #endif
#else
    #ifdef _MULT_DISK_
    _ATTR_BRO_UI_DATA_       EXT UINT16          BrowserMem[3];
    #else
    _ATTR_BRO_UI_DATA_       EXT UINT16          BrowserMem[2];
    #endif
#endif

/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
void BrowserWinInit(void *pArg);
void BrowserWinDeInit(void);
uint32 BrowserWinService(void);
UINT32 BrowserWinKey(void);
void BrowserWinPaint(void);
void BrowserWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed);

void BrowserValueInit(UINT16 NeedInitAll);
void BroGotoCurFile(UINT16 GlobeFileNum);

void BroKeyDownProc(void);
void BroKeyUpProc(void);
int16 BroKeyEnterPro(void);
void BroDeleteFilePro(void);
int16 BroKeyExitPro(void);

void BroSDCardCheck(void);
void BroUSBHostCheck(void);
void BroLoadMemSelStr(void);
void BroMemSelKeyDown(void);
void BroMemSelKeyUp(void);
void BroMemSelKeyMenu(void);
int16 BroMemSelKeyExit(void);

UINT32 BroDeleteFile(void);
/*
--------------------------------------------------------------------------------

  Description:  window structure definition

--------------------------------------------------------------------------------
*/
#ifdef _IN_BROSWER_UI_
_ATTR_BRO_UI_DATA_ WIN BrowserWin = {

    NULL,
    NULL,

    BrowserWinService,               //window service handle
    BrowserWinKey,                   //window key handle
    BrowserWinPaint,                 //window display handle

    BrowserWinInit,                  //window initial handle
    BrowserWinDeInit                 //window auti-initial handle

};
#else
_ATTR_BRO_UI_DATA_ EXT  WIN BrowserWin;
#endif


/*
********************************************************************************
*
*                         End of file
*
********************************************************************************
*/

#endif //_BROWSER_UI_H

