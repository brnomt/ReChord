/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��  TextDisplay.c
*
* Description:  ebook module
*
* History:      <author>          <time>        <version>
*             yangwenjie          2009-3-2        1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_TEXTDISPLAY_
#include "SysInclude.h"

#ifdef _EBOOK_

#include "fsinclude.h"
#include "TextConfig.h"
#include "SysFindFile.h"
#include "TextWin.h"
#include "TextInterface.h"
#include "TextBookmark.h"
#include "BrowserUI.h"
#include "mainmenu.h"
#include "Hold.h"

/*
--------------------------------------------------------------------------------
  Function name : void SaveTextInformation(UINT8 count)
  Author        : yangwenjie
  Description   : open the ebook,set to display the ldentifier if open success,

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_INIT_CODE_
void TextFileOpen(void)
{
    UINT16              i;
    LCD_RECT       r;
    UINT8         TempBuff[2];

    r.x0 = TXT_ZONE_LEFT;
    r.y0 = TXT_ZONE_TOP;
    r.x1 = TXT_ZONE_RIGHT;
    r.y1 = TXT_ZONE_BOTTOM;

    FileRead(TempBuff, 2, hTextFile);

    TextFileInfo.CodeType = ANSI_TEXT;
    if((TempBuff[0] == 0xFF) && (TempBuff[1] == 0xFE))
    {
        TextFileInfo.CodeType = UNICODE_TEXT;
        EbookStartFileOffset = 2;
        gPreviousPageOffset  =2;
    }

   // if(Text.NFBookMark != -1){
   //     EbookStartFileOffset = Text.NFBookMark;
   // }else{
        //EbookStartFileOffset = 0;
   // }
    EbookEndFileOffset = EbookStartFileOffset;

    TextFileInfo.pTxtBuf         = TxtBuf;

    for(i = 0; i < TXT_DISP_LINE_MAX; i++)
    {
        TextFileInfo.pTxtBuf->BytesPerLine = 0;
        TextKeyDownProc();
    }
    TextFileInfo.pTxtBuf         = TxtBuf;
}

/*
--------------------------------------------------------------------------------
  Function name : void TextVariableInit(void)
  Author        : yangwenjie
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_INIT_CODE_
void TextVariableInit(void)
{
    UINT16  i;
    TextFileInfo.FilePath[0]        = '\0';
    TextFileInfo.FileName[0]        = '\0';
    TextFileInfo.LongFileName[0]    = '\0';

    for(i=0; i<TXT_DISP_LINE_MAX-1; i++)
            TxtBuf[i].pNext = &TxtBuf[i+1];

    for(i=1; i<TXT_DISP_LINE_MAX; i++)
        TxtBuf[i].pPerv  = &TxtBuf[i-1];

    for(i=0; i<TXT_DISP_LINE_MAX; i++){
        TxtBuf[i].BytesPerLine = 0;
        TxtBuf[i].TxtDispBuf[0] = 0;
    }
    TxtBuf[TXT_DISP_LINE_MAX-1].pNext   = &TxtBuf[0];
    TxtBuf[0].pPerv = &TxtBuf[TXT_DISP_LINE_MAX-1];
    TextFileInfo.pTxtBuf         = TxtBuf;

    TextFileInfo.CodeType = ANSI_TEXT;

    BookMaskInfo.BookMaskType = 0;
    BookMaskInfo.BookMarkCont = 0;
    BookMaskInfo.IsBookMarkDislay   = FALSE;

    EbookStartFileOffset = 0;
    EbookEndFileOffset   = 0;
    EbookCurrentPages    = 0;
    EbookTotalPages      = 0;
    EbookPageCountOffset = 0;

    hTextFile   =-1;

    TextAutoScanCountBack = SysTickCounter;
    TextAutoPlayEnable = 0;
    memset(TextTitleInfo,0,(MAX_LONG_FILE_NAME_LENGTH+9)*2);

}

/*
--------------------------------------------------------------------------------
  Function name : void TextInit(void)
  Author        : yangwenjie
  Description   : display initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_CODE_
void TextWinInit(void *pArg)
{
    INT16    bStatus ;

    gbTextFileNum = ((TEXT_WIN_ARG*)pArg)->FileNum;
    KeyReset();

    TextVariableInit();

    gwSaveDirClus = 0;
    TxtSysFileInfo.ucSelPlayType = 0;
    bStatus = SysFindFileInit(&TxtSysFileInfo, gbTextFileNum,FIND_FILE_RANGE_ALL,NULL,(UINT8*)TextFileExtString);

    if (0 == bStatus)
    {
        TextFileInfo.FileSize = TxtSysFileInfo.Fdt.FileSize;                 // �ļ���С.
        strncpy((char *)TextFileInfo.FilePath, (char *)TxtSysFileInfo.Path,MAX_PATH_NAME_LENGTH);  // ·��.

        // long file name
        GetLongFileName(TxtSysFileInfo.FindData.Clus,TxtSysFileInfo.FindData.Index - 1, FS_FAT, TextFileInfo.LongFileName);

        memcpy(TextFileInfo.FileName, TxtSysFileInfo.Fdt.Name, 11);
        TextFileInfo.FileName[11]='\0';

        // ebook bookmark initial
        if(0==TextBookmarkInit())
        {
            hTextFile = FileOpen(TxtSysFileInfo.Fdt.Name,TxtSysFileInfo.FindData.Clus, TxtSysFileInfo.FindData.Index - 1,FS_FAT, (uint8*)"R");
            TextFileOpen();
        }

    }

    SendMsg(MSG_BOOK_DISPLAY_ALL);
    SendMsg(MSG_BOOK_DISPLAY_WIN_FALG);

}

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_CODE_
void TextWinDeInit(void)
{
    if (TextAutoPlayEnable == 1)
    {
        BLOffEnable();
        AutoPowerOffEnable();
    }

    ClearMsg(MSG_BOOK_DISPLAY_TITLE_SCROLL);
    ClearMsg(MSG_BOOK_DISPLAY_WIN_FALG);
    FileClose(hTextFile);
    hTextFile = -1;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 TextMsgProc(void)
  Author        : yangwenjie
  Description   : ebook message service handle.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_CODE_
UINT32 TextWinService(void)
{
   UINT16 i;

    if(FALSE == BookMaskInfo.IsBookMarkDislay)
    {
        if( TextAutoPlayEnable)
        {
            TextAutoScanCount = SysTickCounter -TextAutoScanCountBack;

            if(TextAutoScanCount > (gTxtAutoBrowserTime + 1) * 200)
            {
                if(EbookEndFileOffset<TextFileInfo.FileSize)
                {
                    gPreviousPageOffset = EbookStartFileOffset;
                    for(i=0; i<TXT_DISP_LINE_MAX; i++)
                        TextKeyDownProc();

                    EbookCurrentPages++;
                    SendMsg(MSG_BOOK_DISPLAY_UPDATE);
                    SendMsg(MSG_BOOK_DISPLAY_TITLE_CURRENTPAGE);
                }
                else
                {
                    TextAutoPlayEnable= FALSE;
                    SendMsg(MSG_BOOK_DISPLAY_PLAYSTATUS);

                }
                TextAutoScanCountBack = SysTickCounter;
            }
        }
    }

    #ifdef  _DISPLAY_ToTALPAGE_
    if(EbookPageCountOffset<TextFileInfo.FileSize)
    {
        TextPageCountProc();
    }
    #endif

    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 TextKey(void)
  Author        : yangwenjie
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
UINT32 TextWinKey(void)
{
    UINT16  i;
    UINT32 BookKeyVal;
    UINT16  retval = 0;
    TASK_ARG TaskArg;
    UINT8 EbookStartoffset;

    BookKeyVal = GetKeyVal();

    switch (BookKeyVal)
    {
        case KEY_VAL_MENU_SHORT_UP:
            #ifdef _AUTO_BROWSER_
            if(TextAutoPlayEnable)
            {
                TextAutoPlayEnable = 0;
                BLOffEnable();
                AutoPowerOffEnable();
            }
            else
            {
                TextAutoPlayEnable = 1;
                TextAutoScanCountBack = SysTickCounter;//by YWJ 09.07.13
                BLOffDisable();
                AutoPowerOffDisable();
            }
            SendMsg(MSG_BOOK_DISPLAY_PLAYSTATUS);
            #endif
            break;

        case KEY_VAL_PLAY_PRESS_START:
             WinCreat(&TextMenuWin,&BookMarkWin,NULL);
             BookMaskInfo.BookMaskType=TXT_BOOKMARK_TYPE_SAVE;
             BookMaskInfo.IsBookMarkDislay = TRUE;
             break;

        case KEY_VAL_PLAY_SHORT_UP:
            WinCreat(&TextMenuWin,&BookMarkWin,NULL);
            BookMaskInfo.BookMaskType=TXT_BOOKMARK_TYPE_DISTILL ;
            BookMaskInfo.IsBookMarkDislay = TRUE;
            break;

        case KEY_VAL_ESC_SHORT_UP:
            TextBookmarkSave(0);
            if(CheckMsg(MSG_BROW_FROM_MAINMENU))
            {
                TaskArg.Browser.FileType = FileTypeALL;
                TaskArg.Browser.FileNum  = GlobalFilenum;
            }
            else
            {
                TaskArg.Browser.FileType = FileTypeText;
                TaskArg.Browser.FileNum  = gbTextFileNum;
            }
            TaskArg.Browser.FromWhere = 0;
            TaskSwitch(TASK_ID_BROWSER, &TaskArg);
            retval = 1;
            break;

        case KEY_VAL_FFD_SHORT_UP:
        case KEY_VAL_FFD_PRESS:
            if(EbookEndFileOffset<TextFileInfo.FileSize)
            {
                gPreviousPageOffset = EbookStartFileOffset;
                for(i=0; i<TXT_DISP_LINE_MAX; i++)
                    TextKeyDownProc();

                EbookCurrentPages++;
                SendMsg(MSG_BOOK_DISPLAY_UPDATE);
                SendMsg(MSG_BOOK_DISPLAY_TITLE_CURRENTPAGE);
                //SendMsg(MSG_BOOK_DISPLAY_TITLE_TOTAIPAGE);
            }
            break;

        case KEY_VAL_FFW_SHORT_UP:
        case KEY_VAL_FFW_PRESS:
            if(TextFileInfo.CodeType==UNICODE_TEXT)
            {
                EbookStartoffset = 2;
            }
            else
            {
                EbookStartoffset = 0;
            }
            if((gPreviousPageOffset >=EbookStartoffset)&&(EbookStartFileOffset!=EbookStartoffset))
            {
                TextKeyUpProc(TXT_DISP_LINE_MAX);

                EbookCurrentPages--;

                SendMsg(MSG_BOOK_DISPLAY_UPDATE);
                SendMsg(MSG_BOOK_DISPLAY_TITLE_CURRENTPAGE);
                //SendMsg(MSG_BOOK_DISPLAY_TITLE_TOTAIPAGE);
            }
            break;

        case KEY_VAL_UP_PRESS   :
        case KEY_VAL_UP_SHORT_UP:
            if(TextFileInfo.CodeType==UNICODE_TEXT)
            {
                EbookStartoffset = 2;
            }
            else
            {
                EbookStartoffset = 0;
            }
            if((gPreviousPageOffset >=EbookStartoffset)&&(EbookStartFileOffset!=EbookStartoffset))
            {
                TextKeyUpProc(TXT_DISP_LINE_MAX);

                EbookCurrentPages--;

                SendMsg(MSG_BOOK_DISPLAY_UPDATE);
                SendMsg(MSG_BOOK_DISPLAY_TITLE_CURRENTPAGE);
                //SendMsg(MSG_BOOK_DISPLAY_TITLE_TOTAIPAGE);
            }
            break;

        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
            if(EbookEndFileOffset<TextFileInfo.FileSize)
            {
                gPreviousPageOffset = EbookStartFileOffset;

                for(i=0; i<TXT_DISP_LINE_MAX; i++)
                    TextKeyDownProc();

                EbookCurrentPages++;

                SendMsg(MSG_BOOK_DISPLAY_UPDATE);
                SendMsg(MSG_BOOK_DISPLAY_TITLE_CURRENTPAGE);
                //SendMsg(MSG_BOOK_DISPLAY_TITLE_TOTAIPAGE);
            }
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&TextMenuWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&TextMenuWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return(retval);
}

/*
--------------------------------------------------------------------------------
  Function name : void TextPaint(void)
  Author        : yangwenjie
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
void TextWinPaint(void)
{

    UINT16 TextFort, TempColor, TempBkColor, TempCharSize,TempTxtMode;
    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_BOOK_DISPLAY_ALL)))
    {
        //display backgroud picture,battery and hold icon.
        //send all initial message
        SendMsg(MSG_BATTERY_UPDATE);
        SendMsg(MSG_BOOK_DISPLAY_HOLD);
        SendMsg(MSG_BOOK_DISPLAY_BACKGROUND);
        SendMsg(MSG_BOOK_DISPLAY_TITLE);
        SendMsg(MSG_BOOK_DISPLAY_TITLE_CURRENTPAGE);
        SendMsg(MSG_BOOK_DISPLAY_PLAYSTATUS);
        SendMsg(MSG_BOOK_DISPLAY_UPDATE);

        ////clear scroll message,if not,it still fresh scrolllist when the long file name folder is empty.
        ClearMsg(MSG_BOOK_DISPLAY_TITLE_SCROLL);
    }

    if(TRUE == GetMsg(MSG_BOOK_DISPLAY_BACKGROUND))
    {
         DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BACKGROUND,0,0);
    }

    if(TRUE == GetMsg(MSG_BOOK_DISPLAY_PLAYSTATUS))
    {
        if(0==TextAutoPlayEnable)
        {
            DispPictureWithIDNum(IMG_ID_TEXT_STATUS_PAUSE);
        }
        else
        {
            DispPictureWithIDNum(IMG_ID_TEXT_STATUS_PLAY);
        }
    }

    if(TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01+BrowserBatteryLevel,105,146);
    }

    if(TRUE == GetMsg(MSG_BOOK_DISPLAY_HOLD))
    {
        //DispPictureWithIDNum(IMG_ID_SETMENU_HOLDOFF + TextHoldState);
    }

    if(TRUE ==GetMsg(MSG_BOOK_DISPLAY_TITLE_CURRENTPAGE))
    {
        //TextDisplayCurrentPages(EbookCurrentPages+1);
    }

    if(TRUE == GetMsg(MSG_BOOK_DISPLAY_TITLE))
    {
        TextDisplayTitle();
    }

#ifdef  _DISPLAY_ToTALPAGE_
    if(TRUE == GetMsg(MSG_BOOK_DISPLAY_TITLE_TOTAIPAGE))
    {
        TextPagePro(EbookCurrentPages+1,EbookTotalPages,TextTitleInfo);
    }
#endif

    if(TRUE == CheckMsg(MSG_BOOK_DISPLAY_TITLE_SCROLL))
    {
#ifdef  _DISPLAY_ToTALPAGE_
        ScrollStringCommon(TextTitleInfo);
#endif
        ScrollStringCommon(TextFileInfo.LongFileName);
    }

    if(TRUE == GetMsg(MSG_BOOK_DISPLAY_UPDATE))
    {
        TextDisplay();
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
void TextDisplayCurrentPages (UINT32  CurrentPage)
{
    UINT16 DataBuffer[5];

    DataBuffer[0] = CurrentPage/1000 + '0';
    DataBuffer[1] = CurrentPage%1000/100 + '0';
    DataBuffer[2] = CurrentPage%100/10 + '0';
    DataBuffer[3] = CurrentPage%10 + '0';
    DataBuffer[4] = 0  ;
    DispPictureWithIDNumAndXYoffset(IMG_ID_TEXT_PAGEBACKGROUND,40,0);
    LCD_NFDispStringAt(130,4,DataBuffer);
}

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
void TextDisplayTitle (void)
{
    LCD_RECT      r;
    PicturePartInfo TextPictureInfo;
    UINT32 PageCharNum;
    PICTURE_INFO_STRUCT  PicInfo;

    r.x0 = 23;
    r.x1 = 23+81;
    r.y0 = 145;
    if(LcdContext.TextFort==FONT_12x12)
    r.y1 = r.y0+AS_CHAR_YSIZE_12-1;
    if(LcdContext.TextFort==FONT_16x16)
    r.y1 = r.y0+AS_CHAR_YSIZE_16-1;

     GetPictureInfoWithIDNum(IMG_ID_TEXT_MANE_BACK, &PicInfo);

    TextPictureInfo.x = PicInfo.x;
    TextPictureInfo.y = PicInfo.y;
    TextPictureInfo.pictureIDNump = IMG_ID_TEXT_MANE_BACK;
    TextPictureInfo.yoffset = 0;
    TextPictureInfo.ysize   =PicInfo.ySize;


    //display file name
#ifdef  _DISPLAY_ToTALPAGE_
    PageCharNum = TextPagePro(EbookCurrentPages+1,EbookTotalPages,TextTitleInfo);
    TextCat(TextTitleInfo,TextFileInfo.LongFileName,PageCharNum);
    if(LCD_GetStringSize(TextTitleInfo)>TEXT_TITLE_DISPLAY_WIDTH)
    {
        SetScrollStringInfo(&r,TextPictureInfo,TextTitleInfo,30);
        SendMsg(MSG_BOOK_DISPLAY_TITLE_SCROLL);
    }
    LCD_DispStringInRect(&r, &r,TextTitleInfo, LCD_TEXTALIGN_CENTER);
#endif

    if(LCD_GetStringSize(TextFileInfo.LongFileName)>PicInfo.xSize)
    {
        SetScrollStringInfo(&r,TextPictureInfo,TextFileInfo.LongFileName,30);
        SendMsg(MSG_BOOK_DISPLAY_TITLE_SCROLL);
    }
    LCD_DispStringInRect(&r, &r,TextFileInfo.LongFileName, LCD_TEXTALIGN_CENTER);

}

/*
--------------------------------------------------------------------------------
  Function name : void TextKeyDownProc(void)
  Author        : yangwenjie
  Description   : ebook page to down handle function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_CODE_
void TextKeyDownProc(void)
{
    UINT16              i;
    UINT16              ReadBytes;
    LCD_RECT            r;
    TxtDispBufStruct    *pTxt;


    pTxt = TextFileInfo.pTxtBuf;

    r.x0 = TXT_ZONE_LEFT;
    r.y0 = TXT_ZONE_TOP;
    r.x1 = TXT_ZONE_RIGHT;
    r.y1 = TXT_ZONE_BOTTOM;

    EbookStartFileOffset    += pTxt->BytesPerLine;
    pTxt->BytesPerLine      = 0;
#if 0
    for (i = 0; i < TXT_PER_LINE_CHARS; i++)
    {
        pTxt->TxtDispBuf[i] = 0;
    }
#endif
    memset(pTxt->TxtDispBuf,0,TXT_PER_LINE_CHARS);

    ReadBytes = TextGetNextLine (pTxt->TxtDispBuf, EbookEndFileOffset, &r, hTextFile);
    if (ReadBytes > 0)
    {
        pTxt->BytesPerLine  = ReadBytes;
        EbookEndFileOffset  += pTxt->BytesPerLine;
    }
    else
    {
        pTxt->TxtDispBuf[0] = '\0';
        pTxt->BytesPerLine  = 0;
    }

    TextFileInfo.pTxtBuf = pTxt->pNext;

}
/*
--------------------------------------------------------------------------------
  Function name : void TextKeyUpProc(uint8 lines)
  Author        : yangwenjie
  Description   : ebook page to up handle function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
void TextKeyUpProc(uint8 lines)
{
   UINT16    i;

   gPreviousPageOffset = TextGetPreviousPageOffset(EbookStartFileOffset,hTextFile);

    //EbookEndFileOffset   =    EbookStartFileOffset;
    EbookStartFileOffset = gPreviousPageOffset;
    EbookEndFileOffset   = gPreviousPageOffset;

    lines = TXT_DISP_LINE_MAX + (TXT_DISP_LINE_MAX - lines);

    for(i = 0; i < lines; i++)
    {
       TextFileInfo.pTxtBuf->BytesPerLine = 0;
       TextKeyDownProc();
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void TextDisplay(void)
  Author        : yangwenjie
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
void TextDisplay(void)
{
    UINT32 i;
    TxtDispBufStruct *pTxtDisp;
    LCD_RECT      r;
    UINT16 color;
    UINT16 TextFort;


    r.x0 = TXT_ZONE_LEFT;   //text display x start coordinate
    r.y0 = TXT_ZONE_TOP;  ///text display y start coordinate
    r.x1 = TXT_ZONE_RIGHT;  //the x end coordinate of refresh basic color.
    r.y1 = TXT_ZONE_BOTTOM; //the y end coordinate of refresh basic color. limit fresh the window of basic window

    DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,TXT_ZONE_TOP,TXT_ZONE_BOTTOM);
    if(EbookEndFileOffset < TextFileInfo.FileSize){
        i = (((UINT32)(EbookStartFileOffset)*(TXT_SCROLL_BAR_COUNT-1))/TextFileInfo.FileSize);
    }else{
        i = TXT_SCROLL_BAR_COUNT;
    }

    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_SCOLL,122,0);
    DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_SCOLL_BLOCK,122, i);

    pTxtDisp = TextFileInfo.pTxtBuf;
    for(i=0; i<TXT_DISP_LINE_MAX; i++)
    {
        LCD_DispStringInRect(&r, &r, pTxtDisp->TxtDispBuf, LCD_TEXTALIGN_LEFT);
        pTxtDisp = pTxtDisp->pNext;
        if(LcdContext.TextFort==FONT_12x12)
        r.y0 += CH_CHAR_YSIZE_12+5;
        if(LcdContext.TextFort==FONT_16x16)
        r.y0 += CH_CHAR_YSIZE_16+2;
    }
}

/*
--------------------------------------------------------------------------------
  Function name :UINT16 TextCheckChar(UINT16 s)
  Author        : yangwenjie
  Description   : check char

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG function only can get the singal row text length
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_CODE_
UINT16 TextCheckChar(UINT16 s)
{
    if((s >= '0')&&(s <= '9'))                                              // digit
    {
        return (0x0001);
    }

    if(((s >= 'a')&&(s <= 'z')) || ((s >= 'A')&&(s <= 'Z'))

        || ((s >= 0xC0)&&(s <= 0xFF)&&(s != 0xD7)&&(s != 0xF7))             // Latin-1 Supplement, CP1250, CP1252, CP1254
        || ((s >= 0x100)&&(s <= 0x17F))                                     // Latin Extended-A
        || ((s >= 0x180)&&(s <= 0x1BF)) || ((s >= 0x1C4)&&(s <= 0x241))     // Latin Extended-B

        || ((s >= 0x401)&&(s <= 0x45F)) || ((s >= 0x490)&&(s <= 0x491))     // russian , CP1251
        || ((s >= 0x386)&&(s <= 0x3FF)&&(s != 0x387))                       // creek, CP1253
        || (1 == IsHebrewLetter (s))                                        // hebrew, CP1255
        || (1 == IsArabicLetter (s))                                        // arbic, CP1256
      )
    {
        return (0x0002);
    }

    if((s == ' ') || (s == ',') || (s == '.') || (s == '-') || (s == 0x3000) || (s > 0x00FF))
    {
        return (0x8000);
    }

    return(0);
}

/*
--------------------------------------------------------------------------------
  Function name :UINT16 TxtGetCharInRect(LCD_RECT *r, UINT16 *pString)
  Author        : yangwenjie
  Description   : get the chinese text length in rectangular box

  Input         : r       recture
                  pStr    string pointer
  Return        : NumChar chinese text length in rectangular box

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG function only can get the singal row text length
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_CODE_
UINT16 TextGetCharInRect(LCD_RECT *r, UINT16 *pString)
{
    UINT16 i;
    UINT16 NumChar    = 0;
    UINT16 StrDistX   = 0;
    UINT16 *pStr;

    pStr = pString;

    while(*pStr)
    {
        StrDistX +=  LCD_GetCharXSize(*pStr);
        if(((r->x1 - r->x0) <= StrDistX) || (*pStr == 0x0D) || (*pStr == 0x0A))
        {
            break;
        }
        NumChar++;
        pStr++;
    }

    //this time,pStr point to the frist character of next low string..

    if(((r->x1 - r->x0) <= StrDistX)
        && (0x03&(TextCheckChar(*pStr)))         // the frist character of next low string..
        && (0x03&(TextCheckChar(*(pStr - 1)))))  // the end character of current low string..
    {
        pStr--;
        for(i = 0; i < NumChar; i++)
        {
            if(TextCheckChar(*pStr) == 0x8000)
            {
                break;
            }
            else
            {
                pStr--;
            }
        }

        if(i != NumChar)
        {
            NumChar -= i;
        }
    }
    else if (('\r' == pStr[0]) && ('\n' == pStr[1]))    //the \r\n is the end of ebook in Dos and windows
    {
        NumChar += 2;
    }
    else if (('\r' == pStr[0]) && ('\n' != pStr[1]))    //the \r is the end of ebook in MAC
    {
        NumChar += 1;
    }
    else if ('\n' == (pStr[0]))                         //the \n is the end of ebook in UNIX and linux
    {
        NumChar += 1;
    }

    return(NumChar);
}
/*
--------------------------------------------------------------------------------
  Function name : uint16 TextGetBytes(uint16 *GbkString, uint16 CharNumber)
  Author        : yangwenjie
  Description   :  compute byte number.

  Input         : GbkString     the string that is not unicode coding.for example:GBK string.
                  CharNumber    character number.
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         for the singal byte code language,one byte is one charater,such as english.
                for the double bytes code language,two bytes is one charater,such as simplified chinese,japan.
                this function compute the total byte in specified tring.
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_CODE_
uint16 TextGetBytes(uint16 *GbkString, uint16 CharNumber)
{
    uint16    GbkBytes;
    uint16    Chars;
    uint16    GbkChar;
    uint16    iType;          // 1:signal byte charater,2:double bytes character.
    uint8    *pGbk;

    pGbk            = (UINT8*)GbkString;
    Chars           = 0;
    GbkBytes        = 0;
    iType           = 1;

    while ('\0' != (*pGbk))
    {
        if (Chars >= CharNumber)
        {
            break;
        }

        GbkChar = (*pGbk);
        switch (Language)
        {
            case LANGUAGE_CHINESE_S:    //simplified chinese , CP936
            case LANGUAGE_CHINESE_T:    //traditional chinese , CP950
            case LANGUAGE_KOREAN:       //korean , CP949
            case LANGUAGE_JAPANESE:     //japan , CP932
            {
                if ((0x81 <= GbkChar) && (GbkChar <= 0xFE))
                {
                    iType  = 2;
                }
                else
                {
                    iType  = 1;
                }
            }
            break;

            #if 0
            case LANGUAGE_JAPANESE:     // ����, CP932
            {
                if (((0x81 <= GbkChar) && (GbkChar <= 0x9F))
                    || ((0xE0 <= GbkChar) && (GbkChar <= 0xFC)))
                {
                    iType  = 2;
                }
                else
                {
                    iType  = 1;
                }
            }
            break;
            #endif

            default:                    //english,etc west language
            {
                iType  = 1;
            }
            break;
        }

        if (1 == iType)
        {
            GbkBytes++;
            pGbk++;

            Chars++;
        }
        else if (2 == iType)
        {
            GbkBytes    = GbkBytes + 2;
            pGbk        = pGbk + 2;

            Chars++;
        }
    }

    return (GbkBytes);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 TxtGetNextLine (UINT16 *str, UINT32 iOffset, LCD_RECT *r, HANDLE Handle)
  Author        : yangwenjie
  Description   : read out one line text from specified position,

  Input         : str         store the character data group pointer
                  iOffset     offset
                  r           text display area
                  Handle      file pointer
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_CODE_
UINT16 TextGetNextLine (UINT16 *str, UINT32 iOffset, LCD_RECT *r, HANDLE Handle)
{
    UINT16    ReadBytes;
    uint16    CharsPerLine;
    uint8     TxtLineBuf[2*TXT_PER_LINE_CHARS];
    UINT8*    pStr;

    pStr = (UINT8*)str;
    FileSeek(iOffset, SEEK_SET, Handle);

   if(TextFileInfo.CodeType==UNICODE_TEXT)
   {
        ReadBytes = FileRead((uint8 *)str, 2*TXT_PER_LINE_CHARS - 2, Handle);
        pStr[ReadBytes] = '\0';
        //memcopy((UINT8*)str,(UINT8*)TxtLineBuf,TXT_PER_LINE_CHARS);
        CharsPerLine    = TextGetCharInRect(r, str);
        ReadBytes       = CharsPerLine*2;
   }
   else
   {
        ReadBytes = FileRead((uint8 *)TxtLineBuf, TXT_PER_LINE_CHARS - 2, Handle);
        TxtLineBuf[ReadBytes] = '\0';
        TxtGbk2Unicode((UINT16*)TxtLineBuf, str, ReadBytes);
        CharsPerLine    = TextGetCharInRect(r, str);
        ReadBytes       = TextGetBytes((UINT16*)TxtLineBuf, CharsPerLine);
   }

   str[CharsPerLine]   = '\0';

    return ReadBytes;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 TxtGetPreviousPageOffset (UINT32 iCurrentPageOffset, HANDLE Handle)
  Author        : yangwenjie
  Description   : find the paragraph end character and compute offset,

  Input         :iCurrentPageOffset      current page offset
                 Handle                  file pointer
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
UINT32 TextGetStartPageOffset(UINT32 StartPageOffset,UINT16*TextBuf,UINT32 TxtStartOffset)
{
    UINT16      i;
    UINT32      Offset=0;
    UINT32      SearchOffset=0;
    UINT16      ReadBytes;
    UINT8      *pTxtLineBuf;
    UINT32      Count;

    SearchOffset   = StartPageOffset;
    Offset         = TxtStartOffset;
    while (1)
    {
        if (SearchOffset > TEXT_KEY_UP_OFFSET)
        {
            SearchOffset   -= TEXT_KEY_UP_OFFSET;
            Offset         = SearchOffset;
        }
        else
        {
            Offset         = TxtStartOffset;
            break;
        }

        FileSeek(SearchOffset, SEEK_SET, hTextFile);
        ReadBytes = FileRead((uint8 *)TextBuf, TEXT_KEY_UP_OFFSET, hTextFile);
        TextBuf[ReadBytes] = '\0';

        if(ReadBytes <= 0)
        {
            Offset  = TxtStartOffset;
            break;
        }

        pTxtLineBuf =(UINT8*) TextBuf;
        for(i = 0; i < (ReadBytes - 2); i++)
        {
            if ((StartPageOffset - Offset) > TXT_DISP_BUF_MAX)
            {
                if(TextFileInfo.CodeType == UNICODE_TEXT)
                {
                    if (('\r' == pTxtLineBuf[0])
                        && ('\0' == pTxtLineBuf[1])
                        && ('\n' == pTxtLineBuf[2])
                        && ('\0' == pTxtLineBuf[3]) )   //the \r\n is the end of ebook in Dos and windows
                    {
                        pTxtLineBuf += 4;
                        Offset     += 4;
                        break;
                    }
                    else if (('\r' == pTxtLineBuf[0])
                        && ('\0' == pTxtLineBuf[1]))   //the \r is the end of ebook in MAC
                    {
                        pTxtLineBuf += 2;
                        Offset     += 2;
                        break;
                    }
                    else if (('\n' == pTxtLineBuf[0])
                        && ('\0' == pTxtLineBuf[1]))  //the \n is the end of ebook in UNIX and linux
                    {
                        pTxtLineBuf += 2;
                        Offset     += 2;
                        break;
                    }
                }
                else
                {
                    if (('\r' == pTxtLineBuf[0]) && ('\n' == pTxtLineBuf[1]))   //the \r\n is the end of ebook in Dos and windows
                    {
                        pTxtLineBuf += 2;
                        Offset     += 2;
                        break;
                    }
                    else if ('\r' == pTxtLineBuf[0])  //the \r is the end of ebook in MAC
                    {
                        pTxtLineBuf += 1;
                        Offset     += 1;
                        break;
                    }
                    else if ('\n' == (pTxtLineBuf[0])) //the \n is the end of ebook in UNIX and linux
                    {
                        pTxtLineBuf += 1;
                        Offset     += 1;
                        break;
                    }
                }
            }

            pTxtLineBuf++;
            Offset++;
        }

        if ((ReadBytes - 2) != i)
        {
            break;
        }
    }

   return Offset;
}


/*
--------------------------------------------------------------------------------
  Function name : UINT32 TxtGetPreviousPageOffset (UINT32 iCurrentPageOffset, HANDLE Handle)
  Author        : yangwenjie
  Description   : compute the previous page offset by current page offset.

  Input         :iCurrentPageOffset
                 Handle
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
UINT32 TextGetPreviousPageOffset (UINT32 iStartPageOffset, HANDLE Handle)
{
    UINT16      i;
    UINT32      iPreviousPageOffset =0;
    UINT16      lines;
    UINT16      ReadBytes;
    UINT32      iOffset=0;
    UINT32      iSearchOffset=0;
    UINT32      iTxtStartOffset;
    UINT16      TxtLineBuf[TEXT_KEY_UP_OFFSET+16];
    volatile UINT16     LineLength[TXT_DISP_LINE_MAX];
    LCD_RECT            r;

    if(TextFileInfo.CodeType == UNICODE_TEXT)
    {
        iTxtStartOffset = 2;
    }
    else
    {
        iTxtStartOffset = 0;
    }

    // check iCurrentPageOffset's legitimacy
    if (iStartPageOffset <= iTxtStartOffset)
    {
        iPreviousPageOffset = iTxtStartOffset;

        return iPreviousPageOffset;
    }

    if (iStartPageOffset > TextFileInfo.FileSize)
    {
        iPreviousPageOffset = iTxtStartOffset;

        return iPreviousPageOffset;
    }
        iOffset = TextGetStartPageOffset(iStartPageOffset,TxtLineBuf,iTxtStartOffset);

        // compute the previous page offset
        r.x0 = TXT_ZONE_LEFT;
        r.y0 = TXT_ZONE_TOP;
        r.x1 = TXT_ZONE_RIGHT;
        r.y1 = TXT_ZONE_BOTTOM;
        iSearchOffset       = iOffset;
        iPreviousPageOffset = iOffset;
        lines               = 0;
        while (1)
        {
            ReadBytes = TextGetNextLine (TxtLineBuf, iSearchOffset, &r, Handle);
            if (ReadBytes > 0)
            {
                lines++;
                if (lines > TXT_DISP_LINE_MAX)
                {
                    iPreviousPageOffset += LineLength[0];
                }

                for (i = 1; i < TXT_DISP_LINE_MAX; i++)
                {
                    LineLength[i - 1] = LineLength[i];
                }

                LineLength[TXT_DISP_LINE_MAX - 1] = ReadBytes;

                iSearchOffset       += ReadBytes;
                if (iSearchOffset >= iStartPageOffset)
                {
                    break;
                }
            }
            else
            {
                iPreviousPageOffset = iOffset;
                break;
            }
        }

    return iPreviousPageOffset;
}

/*
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
void TextPageCountProc(void)
{
    UINT16              i;
    UINT16              ReadBytes;
    LCD_RECT            r;
    UINT16              pTxt[TXT_PER_LINE_CHARS];

    r.x0 = TXT_ZONE_LEFT;
    r.y0 = TXT_ZONE_TOP;
    r.x1 = TXT_ZONE_RIGHT;
    r.y1 = TXT_ZONE_BOTTOM;

    for (i = 0; i < TXT_DISP_LINE_MAX-1; i++)
    {
        ReadBytes = TextGetNextLine (pTxt, EbookPageCountOffset, &r, hTextFile);
        if (ReadBytes > 0)
        {
            EbookPageCountOffset  += ReadBytes;
        }
    }
    EbookTotalPages++;
    if(EbookTotalPages>9999)
       EbookTotalPages=9999;

    SendMsg(MSG_BOOK_DISPLAY_TITLE_TOTAIPAGE);
}


/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_SERVICE_CODE_
UINT16  TextPagePro(UINT32  CurrentPage,UINT32 TotalPage,UINT16 *DataBuffer)
{
    UINT16 CharNum;

    DataBuffer[0] = CurrentPage/1000 + '0';
    DataBuffer[1] = CurrentPage%1000/100 + '0';
    DataBuffer[2] = CurrentPage%100/10 + '0';
    DataBuffer[3] = CurrentPage%10 + '0';
#ifdef _DISPLAY_ToTALPAGE_
    DataBuffer[4] = '/';
    DataBuffer[5] =TotalPage/1000 + '0';
    DataBuffer[6] = TotalPage%1000/100 + '0';
    DataBuffer[7] = TotalPage%100/10 + '0';
    DataBuffer[8] = TotalPage%10 + '0';
    DataBuffer[9] = ' ';
    CharNum = 10;
#endif
    DataBuffer[4] = ' ';
    CharNum = 5 ;

    return CharNum;
}

/*
********************************************************************************
*
*                         End of TextDisplay.c
*
********************************************************************************
*/
#endif
