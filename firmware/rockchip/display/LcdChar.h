/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£∫   LcdChar.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef     _LCDCHAR_H_
#define     _LCDCHAR_H_

#undef  EXT
#ifdef _IN_LCDCHAR_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------

                        Macro define

--------------------------------------------------------------------------------
*/
#define     FONT_12x12                      1

#define     FONT_16x16                      2


#define     SYSTEM_CHAR_FONT                FONT_16x16

//#define     TXT_PER_LINE_CHARS            80

#define     TXT_PER_LINE_CHARS              32    /* bigger than reality. */

//Codepage∂®“Â
#define     UNICODE_IN_NF_BASE_ADDR         (UINT32)0x3c0000           /*0x40000*/

#define     GBK2UNICODE_NF_BASE_ADDR        (UINT32)0x5c0000

#define     FONT_CODEPAGE_CP874             0

#define     FONT_CODEPAGE_CP932             1

#define     FONT_CODEPAGE_CP936             2

#define     FONT_CODEPAGE_CP949             3

#define     FONT_CODEPAGE_CP950             4


#define     FONT_CODEPAGE_CP1250            5

#define     FONT_CODEPAGE_CP1251            6

#define     FONT_CODEPAGE_CP1252            7

#define     FONT_CODEPAGE_CP1253            8

#define     FONT_CODEPAGE_CP1254            9


#define     FONT_CODEPAGE_CP1255            10

#define     FONT_CODEPAGE_CP1256            11

#define     FONT_CODEPAGE_CP1257            12

#define     FONT_CODEPAGE_CP1258            13

//define font size
#define     AS_CHAR_XSIZE_12                6

#define     AS_CHAR_YSIZE_12                12

#define     CH_CHAR_XSIZE_12                12

#define     CH_CHAR_YSIZE_12                 AS_CHAR_YSIZE_12


#define     AS_CHAR_XSIZE_16                8

#define     AS_CHAR_YSIZE_16                16

#define     CH_CHAR_XSIZE_16                16

#define     CH_CHAR_YSIZE_16                AS_CHAR_YSIZE_16

//define display language
#define     MENU_PER_BYTE                   64
#define     MENU_LANGUE_PER_LINE            320

#define     MENU_LANGUE_PER_BYTE            ((UINT32)(MENU_PER_BYTE) * MENU_LANGUE_PER_LINE)

//#define     LANGUAGE_MAX_COUNT              21

//LCD displsy mode definition.
#define     LCD_MODE_0                      0   // ˙∆¡
#define     LCD_MODE_90                     1   //∫·∆¡

//LCD rotation mode definition.
#define     LCD_ASPECT_0                    0
#define     LCD_ASPECT_90                   90
#define     LCD_ASPECT_180                  180
#define     LCD_ASPECT_270                  270

//LCD draw mode definition.
#define     LCD_DRAWMODE_NORMAL             (0)   //no display
#define     LCD_DRAWMODE_TRANS              (1<<0)//display only backgroud color
#define     LCD_DRAWMODE_REV                (1<<1)//just display charactor,display fontcolor.
#define     LCD_DRAWMODE_CIR90              (1<<2)


//LCD align mode definition.
#define     LCD_TEXTALIGN_LEFT              0x0000
#define     LCD_TEXTALIGN_RIGHT             0x0001
#define     LCD_TEXTALIGN_CENTER            0x0002
#define     LCD_TEXTALIGN_HORIZONTAL        0x0003
#define     LCD_TEXTALIGN_ANL               0x8000

//LCD color definition.
#define     COLOR_BLACK                     0x0000
#define     COLOR_WHITE                     0xFFFF
#define     COLOR_RED                       0xF800
#define     COLOR_GREEN                     0x07E0
#define     COLOR_BLUE                      0x001F
#define     COLOR_YELLOW                    (COLOR_RED | COLOR_GREEN)
#define     COLOR_PURPLE                    (COLOR_RED | COLOR_BLUE)
#define     COLOR_CYAN                      (COLOR_GREEN | COLOR_BLUE)


#define     LCD_TEXTALIGN_HORIZONTAL        0x0003
#define     LCD_TEXTALIGN_LEFT              0x0000
#define     LCD_TEXTALIGN_RIGHT             0x0001
#define     LCD_TEXTALIGN_CENTER            0x0002
#define     LCD_TEXTALIGN_ANL               0x8000


/*
--------------------------------------------------------------------------------

  Description:  LCD_CHAR structure definition.

--------------------------------------------------------------------------------
*/
//LCD DOT define
typedef struct LCD_PONIT_STRUCT {

    UINT16 x;                       //X coordinate
    UINT16 y;                       //Y coordinate

}LCD_POINT;

//LCD RECT define
typedef struct LCD_RECT_STRUCT {

    INT16 x0;                       //the upper left x0 coordinate
    INT16 y0;                       //the upper left y0 coordinate
    INT16 x1;                       //the lower right x1 coodinate
    INT16 y1;                       //the lower right y1 coodinate

}LCD_RECT;

typedef struct PicturePartInfo {

    INT16 x;
    INT16 y;
    INT16 yoffset;
    INT16 ysize;
    UINT16 pictureIDNump;

}PicturePartInfo;

typedef struct LCD_CONTEXT_STRUCT {

    UINT16 Color;                   //system foregroud
    UINT16 BkColor;                 //system backgroud

    UINT16 DrawMode;                //system draw mode

    INT16  Mode;                    //LCD display mode
    INT16  LcdMaxWidth;             //LCD display aera x size
    INT16  LcdMaxHeight;             //LCD display aera Y size
    LCD_RECT ClipRect;              //clip area.

    INT16  DispPosX;                //display x coordinate
    INT16  DispPosY;                //display y coordinate

    INT16  XDist;                   //the coordinate x offset of charactor
    INT16  YDist;                   //the coordinate y offset of charactor

    INT16  LBorder;                 //the start horizontal left-aligned postion x.
    INT16  RBorder;                 //the start horizontal right-aligned postion x.

    UINT16 TextMode;                //text display mode.
    UINT16 TextAlign;               //text aligned mode.

    UINT16 TextFort;                //text font

    UINT16 ImageIndex;              //image resource

    UINT16 TextLanguage;            //text language.

}LCD_CONTEXT;

typedef struct SCROLL_SRTING_STRUCT
{
    LCD_RECT  DiplayRect;
    UINT16    TotalSize;
    UINT16    CharNumber;
    UINT16    TotalCharBack;
    UINT16    ScrollNumber;
    UINT16    CharSizePerLine;
    UINT16    StepSize;
    UINT16    pictureID;
    UINT16    TextFort;
    PicturePartInfo pictureInfo;
    UINT16    StartX;
    UINT16    TotalCharNum;
    UINT32    SystickCounterBack;
    UINT16    ScrollSpeed;
}SCROLL_SRTING;

/*resource pictrue information structure,the order of members cannot be changed. */

typedef struct PictureInfoStruct
{
    UINT16  xSize;
    UINT16  ySize;
    UINT16  x;
    UINT16  y;
    UINT32  totalSize;                          /* it equal with xSize multiply ySize */
    UINT32  offsetAddr;                         /* the offset address that picture saved in flash */

} PICTURE_INFO_STRUCT;

/*menu text resource information structure,the order of members cannot be changed. */
typedef struct MenuTextInfoStruct
{
    UINT16  SelfID;                             /* menu item id */
    UINT16  FatherID;                           /* father menu id*/
    UINT16  CurItemInFatherID;                  /* the serial number in father menu */
    UINT16  ChildID ;                           /* frist menu item id */
    UINT16  PrevID;                             /* left brother menu item id.*/
    UINT16  NextID;                             /* right brother menu item id.*/
    UINT16  FunctionIndex;                      /* index of implement function.*/
    UINT16  ChildNum;                           /* total number of child items.*/
    UINT16  ChildIDArray[21];                   /* the id groud of child menu items*/
} MENU_TEXT_INFO_STRUCT;

typedef struct LanguageAddrInfoStruct
{
    UINT16  LanguageNum;                    //the total number of languages.
    unsigned long   LanguegeOffsetAddr[21]; //the offset address in flash.

} LANGUAGE_ADDR_INFO_STRUCT;

/*
--------------------------------------------------------------------------------

                        Variable Define

--------------------------------------------------------------------------------
*/
#define LCD_COLOR       LcdContext.Color
#define LCD_BKCOLOR     LcdContext.BkColor
#define LCD_DRAWMODE    LcdContext.DrawMode
#define LCD_DISPMODE    LcdContext.Mode
#define LCD_TEXTMODE    LcdContext.TextMode
#define LCD_TEXTALIGN   LcdContext.TextAlign
#define LCD_TEXTFORT    LcdContext.TextFort
#define LCD_IMAGEINDEX  LcdContext.ImageIndex
#define Language        gSysConfig.SysLanguage

/*
--------------------------------------------------------------------------------

                        Variable Define

--------------------------------------------------------------------------------
*/
// Ã©Œƒ, fengmingxi@2007-3-14   9:44
_ATTR_LCD_BSS_  EXT UINT32 Special_TaiWen_Buf[2];
_ATTR_LCD_BSS_  EXT LCD_CONTEXT  LcdContext;
_ATTR_LCD_BSS_  EXT SCROLL_SRTING ScrollString;

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
#define LCD_SetColor(color)     (LCD_COLOR = (color))
#define LCD_GetColor()          (LCD_COLOR)
#define LCD_SetBkColor(color)   (LCD_BKCOLOR = (color))
#define LCD_GetBkColor()        (LCD_BKCOLOR)
#define LCD_SetDrawMode(mode)   (LCD_DRAWMODE = (mode))
#define LCD_GetDrawMode()       (LCD_DRAWMODE)
#define LCD_GetTextMode()       (LCD_TEXTMODE)
#define LCD_SetTextAlign(mode)  (LCD_TEXTALIGN = (mode))
#define LCD_GetTextAlign()      (LCD_TEXTALIGN)
#define LCD_GetDiaplayMode()    (LCD_DISPMODE)
#define printk(x,y,format,...)  (LcdContext.DispPosX = (x); LcdContext.DispPosY = (y); printf(const char *format,...))

//EXT void LCD_SetColor(UINT16 color);
//EXT UINT16 LCD_GetColor(void);
//EXT void LCD_SetBkColor(UINT16 color);
//EXT UINT16 LCD_GetBkColor(void);
//EXT void LCD_SetDrawMode(UINT16 mode);
//EXT UINT16 LCD_GetDrawMode(void);
EXT UINT16 LCD_SetTextMode(UINT16 mode);
//EXT UINT16 LCD_GetTextMode(void);
//EXT void LCD_SetTextAlign(UINT16 mode);
//EXT UINT16 LCD_GetTextAlign(void);
EXT void LCD_SetDiaplayMode(INT16 mode);
//EXT INT16 LCD_GetDiaplayMode(void);
EXT void LCD_SetDispRect(INT16 x0, INT16 y0, INT16 x1, INT16 y1);
EXT void LCD_SetPixel(UINT16 x, UINT16 y, UINT16 color);
EXT void LCD_SetCharSizeInit();
EXT UINT16 LCD_SetCharSize(UINT16 size);
EXT void LCD_SetPixel(UINT16 x, UINT16 y, UINT16 color);

EXT UINT16 Gbk2Unicode(UINT16 code,UINT16 codemode);
EXT UINT16 TxtGbk2Unicode(UINT16 *pSbuf, UINT16 *pTbuf, UINT16 Len);
EXT UINT32 LCD_GetUnicodeStringSize(UINT32 *s);
EXT void LCD_NFDispUnicodeCharAt(UINT32 x, UINT32 y, UINT32 c);
EXT void LCD_NFDispUnicodeChar(UINT16 c);
EXT UINT32 LCD_GetStringSize(UINT16 *s);
EXT UINT16 LCD_GetCharXSize(UINT16 s);
EXT UINT16 LCD_GetCharsPerLine(LCD_RECT *r, UINT16 *s);
EXT UINT16 LCD_GetLineDistX(UINT16 *s, UINT32 LineNumChar);
EXT void LCD_DispLineChar(UINT16 *s, UINT32 LineNumChar);
EXT void LCD_NFDispString(UINT16 *pStr);
EXT void LCD_NFDispStringAt(UINT32 x, UINT32 y, UINT16 *pStr);
EXT void LCD_DispStringInRect(LCD_RECT *pDr, LCD_RECT *pSr, UINT16 *pStr, UINT32 AlignMode);
EXT void DisplayString(UINT32 x, UINT32 y, UINT32 xsize, UINT32 ysize,  UINT32 mode, UINT16 TextID);
EXT void LCD_DrawChar(UINT8 Xpos, UINT16 Ypos, UINTC16 *c);
EXT void LCD_DisplayChar(UINT8 Line, UINT16 Column, UINT8 Ascii);
EXT void LCD_DisplayStringLine(UINT8 Line, UINT8 *ptr);

EXT void GetResourceStr(UINT16 menuTextID , UINT16 *pMenuStr ,UINT16 StrLen);
EXT void DisplayMenuStr(UINT16 x, UINT16 y, UINT16 xsize, UINT16 ysize, UINT16 mode, UINT32 flash_addr);
EXT void DisplayMenuStrWithIDNum(UINT16 x, UINT16 y,UINT16 xsize, UINT16 ysize,UINT16 alignMode, UINT16 menuTextID);
EXT void GetMenuTextInfoWithIDNum(UINT32 menuTextID, MENU_TEXT_INFO_STRUCT *pMenuTextInfo);
EXT void DisplayTestString(UINT16 x,UINT16 y, UINT8 *pStr);
EXT void DisplayTestHexNum(UINT16 x, UINT16 y, uint8 bits, UINT64 HexVal);

EXT void ScrollStringCommon(UINT16 *pstr );
EXT void ScrollStringForMusic(UINT16 *pstr );
EXT void SetScrollStringInfo(LCD_RECT *r, PicturePartInfo PictureInfo,UINT16 *pstr,UINT16 Speed);


/*
********************************************************************************
*
*                         End of LcdChar.c
*
********************************************************************************
*/


#endif



