/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��  TextBookmark.c
*
* Description:  ebook module
*
* History:      <author>          <time>        <version>
*             yangwenjie          2009-3-2        1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_TEXT_BOOKMARK

#include "SysInclude.h"

#ifdef _EBOOK_
#include "TextConfig.h"
//#include "config.h"
#include "SysFindFile.h"
#include "TextWin.h"
#include "TextInterface.h"
#include "TextBookmark.h"
#include "Hold.h"
/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void BookMarkWinInit(void *pArg)
{
     KeyReset();
     BookMaskInfo.SystickCount = SysTickCounter;
     SendMsg(MSG_BOOK_MARK_DISPLAY_ALL);
}
/*
--------------------------------------------------------------------------------
  Function name :void BookMarkWinDeInit (void)
  Author        : yangwenjie
  Input         :

  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void BookMarkWinDeInit(void)
{
   BookMaskInfo.BookMarkCont = 0;
   BookMaskInfo.IsBookMarkDislay = FALSE;
   BookMaskInfo.IsScrollBookMask =FALSE;
   TextAutoScanCountBack = SysTickCounter;

}
/*
--------------------------------------------------------------------------------
  Function name :void BookMarkWinService (void)
  Author        : yangwenjie
  Input         :

  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
UINT32  BookMarkWinService(void)
{
    if(TRUE==BookMaskInfo.IsScrollBookMask)
    {
        //BookMarkPageDisPlay();
        SendMsg(MSG_BOOK_MARK_SCROLL_UPDATE);
    }

    return 0;
}
/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
UINT32  BookMarkWinKey(void)
{
    UINT16  i;
    UINT32 BookMarkKeyVal;
    UINT16  retval = 0;
    TASK_ARG TaskArg;

    BookMarkKeyVal = GetKeyVal();

    //5 Key function adjustment
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch(BookMarkKeyVal)
        {
        case KEY_VAL_FFD_DOWN:
        case KEY_VAL_FFD_PRESS:
            BookMarkKeyVal = KEY_VAL_DOWN_DOWN;
            break;

        case KEY_VAL_FFW_DOWN:
        case KEY_VAL_FFW_PRESS:
            BookMarkKeyVal = KEY_VAL_UP_DOWN;
            break;

        default:
            break;
        }
    }

    //6 Key function adjustment
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
        switch(BookMarkKeyVal)
        {
        case KEY_VAL_FFW_SHORT_UP:
            BookMarkKeyVal = KEY_VAL_ESC_SHORT_UP;
            break;

        default:
            break;
        }
    }

    switch (BookMarkKeyVal)
    {
       case KEY_VAL_UP_PRESS:
       case KEY_VAL_UP_DOWN:
            if(BookMaskInfo.BookMarkCont > 0)
            {
                BookMaskInfo.BookMarkCont--;
            }
            else
            {
                BookMaskInfo.BookMarkCont = TXT_BOOKMARK_COUNTER-1;
            }

            if(TRUE==BookMaskInfo.IsScrollBookMask)
            {
                BookMaskInfo.IsScrollBookMask =FALSE;
            }
            SendMsg(MSG_BOOK_MARK_UPDATE);
            ClearMsg(MSG_BOOK_MARK_SCROLL_UPDATE);
            break;

       case KEY_VAL_DOWN_PRESS:
       case KEY_VAL_DOWN_DOWN:
            if(BookMaskInfo.BookMarkCont < TXT_BOOKMARK_COUNTER-1)
            {
                BookMaskInfo.BookMarkCont++;
            }
            else
            {
                BookMaskInfo.BookMarkCont = 0;
            }

            if(TRUE==BookMaskInfo.IsScrollBookMask)
            {
                BookMaskInfo.IsScrollBookMask =FALSE;
            }
            SendMsg(MSG_BOOK_MARK_UPDATE);
            ClearMsg(MSG_BOOK_MARK_SCROLL_UPDATE);
            break;

        case KEY_VAL_MENU_SHORT_UP:
            if(BookMaskInfo.BookMaskType == TXT_BOOKMARK_TYPE_DISTILL)
            {
                TextBookmarkLoad(BookMaskInfo.BookMarkCont+1);
            }
            else
            {
                TextBookmarkSave(BookMaskInfo.BookMarkCont+1);
            }

            #ifdef _DISPLAY_ToTALPAGE_
            BookmarkSaveTotalPage();
            #endif

            WinDestroy(&BookMarkWin);

            ClearMsg(MSG_BOOK_MARK_SCROLL_UPDATE);
            SendMsg(MSG_BOOK_DISPLAY_ALL);
            break;

        case KEY_VAL_ESC_SHORT_UP:

            #ifdef _DISPLAY_ToTALPAGE_
            BookmarkSaveTotalPage();
            #endif

            WinDestroy(&BookMarkWin);
            ClearMsg(MSG_BOOK_MARK_SCROLL_UPDATE);
            SendMsg(MSG_BOOK_DISPLAY_ALL);
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&BookMarkWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&BookMarkWin, &HoldWin, &TaskArg);
            break;

            default:
            break;
    }

    return retval;
}

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void BookMarkWinPaint(void)
{
    UINT16 color_tmp,TextMode;
    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_BOOK_MARK_DISPLAY_ALL)))
    {
        //display backgroud picture,battery and hold icon.
        //send all initial message
        SendMsg(MSG_BOOK_MARK_DISPLAY_BATT);
        SendMsg(MSG_BOOK_MARK_DISPLAY_HOLD);
        SendMsg(MSG_BOOK_MARK_DISPLAY_BACKGROUND);
        SendMsg(MSG_BOOK_MARK_DISPLAY_TITLE);
        SendMsg(MSG_BOOK_MARK_DISPLAY_ITEM);
        SendMsg(MSG_BOOK_MARK_UPDATE);
        BookMaskInfo.IsScrollBookMask =FALSE;
    }

    if(TRUE == GetMsg(MSG_BOOK_MARK_DISPLAY_BACKGROUND))
    {
        DispPictureWithIDNumAndXY(IMG_ID_BOOKMASK_WIN, TXT_BOOKMASK_SELECT_X, TXT_BOOKMASK_SELECT_Y);
    }

    if(TRUE == GetMsg(MSG_BOOK_MARK_DISPLAY_TITLE))
    {
        BookmarkDisplayTitle();
    }

    if(TRUE == GetMsg(MSG_BOOK_MARK_DISPLAY_ITEM))
    {
    }

    if(TRUE == GetMsg(MSG_BOOK_MARK_UPDATE))
    {
        BookMaskDisplay(BookMaskInfo.BookMarkCont);
    }

    if(TRUE == GetMsg(MSG_BOOK_MARK_SCROLL_UPDATE))
    {
        TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        ScrollStringCommon(ScrollBookMask);
        LCD_SetTextMode(TextMode);

    }

}


/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void BookmarkDisplayTitle (void)
{
    UINT16      TextFort;

    TextFort = LCD_SetCharSize(FONT_12x12);
    if(BookMaskInfo.BookMaskType==TXT_BOOKMARK_TYPE_SAVE)
    {
        DisplayMenuStrWithIDNum(TXT_BOOKMASK_SELECT_X, TXT_BOOKMASK_SELECT_Y+4, TXT_BOOKMASK_SELECT_XSIZE,
                    16, LCD_TEXTALIGN_CENTER,SID_SAVE_BOOKMARK);
    }
    else
    {
        DisplayMenuStrWithIDNum(TXT_BOOKMASK_SELECT_X, TXT_BOOKMASK_SELECT_Y+4, TXT_BOOKMASK_SELECT_XSIZE,
                    16, LCD_TEXTALIGN_CENTER, SID_LOAD_BOOKMARK);
    }
    LCD_SetCharSize(TextFort);
}

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void BookmarkDisplayItem (void)
{

}

/*
--------------------------------------------------------------------------------
  Function name : void BookmarkDelete(INT16 Item)
  Author        : yangwenjie
  Description   : delete the book mark number.

  Input         : Item : mark index

  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void BookmarkDelete(INT16 Item)
{
    TEXT_BOOKMARK_UNION     TextBookmark;
    TEXT_BOOKMARK_STRUCT    *pBookmark;

    ReadReservedData(BOOKMARK_BLK, (uint8*)TextBookmark.Buffer, MAX_BOOKMARK_LENGTH);

    pBookmark           = &(TextBookmark.S.Bookmark[Item]);
    pBookmark->bIsEmpty = TRUE;

    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
    WriteReservedData(BOOKMARK_BLK, (UINT8*)TextBookmark.Buffer, MAX_BOOKMARK_LENGTH);

}
/*
--------------------------------------------------------------------------------
  Function name : void TextDistillBookmark (TEXT_BOOKMARK_STRUCT *pBookmark, INT16 id)
  Author        : yangwenjie
  Description   : get book mark

  Input         : pBookmark: Bookmark info
                  Item : mark index

  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void BookmarkDistill(TEXT_BOOKMARK_UNION *TextBookmark,TEXT_BOOKMARK_STRUCT *pBookmark, INT16 Item)
{
    //TEXT_BOOKMARK_UNION     TextBookmark;

    //read all book mark
    ReadReservedData(BOOKMARK_BLK, (uint8*)TextBookmark->Buffer, MAX_BOOKMARK_LENGTH);

    //pointer point current bookmark
    *pBookmark  = TextBookmark->S.Bookmark[Item];

}

/*
--------------------------------------------------------------------------------
  Function name :void TextBookMaskSelectDisp(uint16 type, uint16 mode, uint16 id)
  Author        : yangwenjie
  Description   : display book mark.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                yangwenjie     2009-3-2         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void BookMaskDisplay(UINT16 Item)
{
    UINT16      i,j,k;
    UINT16      color_tmp;
    UINT16      mode_tmp;
    UINT16      MaskDispBuf[10];
    UINT16      TxtLineBuf[MAX_BOOK_MARK_NAME_LENGTH];
    UINT16      *pTxtLineBuf;
    UINT16      CharsPerLine;
    UINT16      ReadBytes;
    LCD_RECT    r;
    UINT16      TextFort;
    TEXT_BOOKMARK_STRUCT Bookmark;
    TEXT_BOOKMARK_UNION  TextBookmark;
    UINT16      PageCharNum;
    UINT16      CharsSize;
    UINT16      StringNum;
    BOOL        NeedSaveBookMask = FALSE;

    TextFort = LCD_SetCharSize(FONT_12x12);

    r.x0 = TXT_BOOKMASK_SELECT_X+27;
    r.y0 = TXT_BOOKMASK_SELECT_Y;
    r.x1 = r.x0 + CH_CHAR_XSIZE_12*5;
    r.y1 = r.y0 + CH_CHAR_YSIZE_12;


    MaskDispBuf[0] = '(';
    MaskDispBuf[1] = 'E';
    MaskDispBuf[2] = 'm';
    MaskDispBuf[3] = 'p';
    MaskDispBuf[4] = 't';
    MaskDispBuf[5] = 'y';
    MaskDispBuf[6] = ')';
    MaskDispBuf[7] = 0;


    mode_tmp  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    color_tmp = LCD_GetColor();

    for(i=0; i<TXT_BOOKMARK_COUNTER; i++)
    {
        LCD_SetColor(COLOR_BLACK);
        if(i == Item)
        {
         // LCD_SetTextMode(LCD_DRAWMODE_REV);
         // LCD_SetColor(COLOR_RED);
        }

        DisplayPicture_part(IMG_ID_BOOKMASK_WIN,TXT_BOOKMASK_SELECT_X,TXT_BOOKMASK_SELECT_Y, 19+(i*17), 17);
        BookmarkDistill(&TextBookmark,&Bookmark,i+1);
        if(FALSE == Bookmark.bIsEmpty)
        {
            if ((0 == strncmp((char *)TextFileInfo.FilePath, (char *)Bookmark.FilePath,MAX_PATH_NAME_LENGTH))
                && (0 == strncmp((char *)TextFileInfo.FileName, (char *)Bookmark.FileName,MAX_FILE_NAME_LENGTH)))
            {
               PageCharNum = TextPagePro(Bookmark.BookCurrentPage+1,0,TxtLineBuf);
            }
            else
            {
               PageCharNum =  TextPagePro(Bookmark.BookCurrentPage+1,0,TxtLineBuf);
            }
            TxtLineBuf[PageCharNum] = 0;
            //StringNum = LCD_GetStringNum(TxtLineBuf);
            StringNum = TextCat(TxtLineBuf,Bookmark.LongFileName,PageCharNum);
            TxtLineBuf[StringNum] = 0;

            r.y0 = TXT_BOOKMASK_SELECT_Y+20+i*17;
            r.y1 = r.y0 + CH_CHAR_YSIZE_12-1;

            if(i == Item)
            {
                CharsSize = LCD_GetStringSize(TxtLineBuf);
                if(CharsSize>(r.x1 - r.x0))
                {
                    memset((UINT8*)ScrollBookMask,0,2*(MAX_BOOK_MARK_NAME_LENGTH+9));
                    if(BookMaskInfo.IsScrollBookMask ==FALSE)
                    {
                        BookMaskInfo.PictureInfo.x = 41;//TXT_BOOKMASK_SELECT_X;
                        BookMaskInfo.PictureInfo.y = 61+i*16+i;//TXT_BOOKMASK_SELECT_Y;

                        BookMaskInfo.PictureInfo.yoffset =0; //19+(i*16);

                        BookMaskInfo.PictureInfo.ysize   = 15;//16;
                        BookMaskInfo.PictureInfo.pictureIDNump = IMG_ID_TEXT_MARKBACKGROUND;
                        SetScrollStringInfo(&r,BookMaskInfo.PictureInfo,TxtLineBuf,30);
                        memcpy((UINT8*)ScrollBookMask,(UINT8*)TxtLineBuf,StringNum*2);
                        memcpy((UINT8*)BookMaskInfo.LongFileName,(UINT8*)Bookmark.LongFileName,(StringNum-PageCharNum)*2);
                        BookMaskInfo.BookCurrentPage = Bookmark.BookCurrentPage;
                       // BookMaskInfo.BookTotalPage   = EbookTotalPages;
                        BookMaskInfo.r = r;
                        BookMaskInfo.SystickCount = SysTickCounter;
                        BookMaskInfo.IsScrollBookMask =TRUE;

                        //DispPictureWithIDNumAndXY(IMG_ID_TEXT_BOOKMARKBACKSEL0,30,59);
                    }
                }
                DispPictureWithIDNumAndXY(IMG_ID_TEXT_MARKBACKGROUND,41,61+i*17+i);
            }

             LCD_DispStringInRect(&r, &r, TxtLineBuf,LCD_TEXTALIGN_LEFT);
        }
        else{
           if(i == Item)
           {
               DispPictureWithIDNumAndXY(IMG_ID_TEXT_MARKBACKGROUND,41,61+i*16+i);
               LCD_NFDispStringAt(TXT_BOOKMASK_SELECT_X+27,TXT_BOOKMASK_SELECT_Y+20+(i*17),MaskDispBuf);
           }
           else
           {
               DisplayPicture_part(IMG_ID_BOOKMASK_WIN,TXT_BOOKMASK_SELECT_X,TXT_BOOKMASK_SELECT_Y, 19+(i*16), 16);
               LCD_NFDispStringAt(TXT_BOOKMASK_SELECT_X+27,TXT_BOOKMASK_SELECT_Y+20+(i*17),MaskDispBuf);
           }
        }

        if(i == Item){
          //  LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        }
    }
    LCD_SetTextMode(mode_tmp);
    LCD_SetColor(color_tmp);
    LCD_SetCharSize(TextFort);
}
/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void BookMarkPageDisPlay(void)
{
    //UINT16 StringNum;
     UINT16     Count;
    Count = SysTickCounter -  BookMaskInfo.SystickCount;
    if(Count>100)
    {
         TextPagePro(BookMaskInfo.BookCurrentPage+1,EbookTotalPages,ScrollBookMask);
         BookMaskInfo.SystickCount = SysTickCounter;
    }
}

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
#ifdef   _DISPLAY_ToTALPAGE_
_ATTR_BOOKMASK_CODE_
void BookmarkSaveTotalPage ()
{
    UINT32 i;
    TEXT_BOOKMARK_UNION     TextBookmark;

    for(i=1; i<TXT_BOOKMARK_COUNTER+1; i++){

        ReadReservedData(BOOKMARK_BLK, (uint8*)TextBookmark.Buffer, MAX_BOOKMARK_LENGTH);
        if(FALSE==TextBookmark.S.Bookmark[i].bIsEmpty)
        {
            if ((0 == strncmp((char *)TextFileInfo.FilePath, (char *)TextBookmark.S.Bookmark[i].FilePath,MAX_PATH_NAME_LENGTH))
                && (0 == strncmp((char *)TextFileInfo.FileName, (char *)TextBookmark.S.Bookmark[i].FileName,MAX_FILE_NAME_LENGTH)))
            {
                //TextBookmark.S.Bookmark[i].BookTotalPage= EbookTotalPages;
            }
        }

        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
        WriteReservedData(BOOKMARK_BLK, (uint8*)TextBookmark.Buffer, MAX_BOOKMARK_LENGTH);
    }
}
#endif

/*
--------------------------------------------------------------------------------
  Function name :void TextBookmarkInit (void)
  Author        : yangwenjie
  Description   : 1.check whethe every book that has book mark is presence,if not,delete this bookmark,
                  2.load the default book mark.

  Input         :

  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_TEXT_INIT_CODE_
UINT16 TextBookmarkInit (void)
{
    INT16                   i;
    UINT32                  uStartOffset;
    HANDLE                  hTextFile;          // �������ļ����.
    TEXT_BOOKMARK_UNION     TextBookmark;
    TEXT_BOOKMARK_STRUCT    *pBookmark;
    INT8                    TextHandle;
    UINT16                   ReturnValue = 0;

    for(i=0;i<TXT_BOOKMARK_COUNTER+1;i++)
    {
          ReadReservedData(BOOKMARK_BLK, (uint8*)TextBookmark.Buffer, MAX_BOOKMARK_LENGTH);
          pBookmark = &(TextBookmark.S.Bookmark[i]);
          if(FALSE == pBookmark->bIsEmpty)
          {
                if(i>0)
                {
                    //if ((TextHandle = FileOpen(TxtSysFileInfo.Fdt.Name,TxtSysFileInfo.FindData.Clus, TxtSysFileInfo.FindData.Index - 1,FS_FAT, (uint8*)"R")) == NOT_OPEN_FILE)
                    if ((TextHandle = FileOpenA(pBookmark->FilePath, pBookmark->FileName,(uint8*)"R")) == NOT_OPEN_FILE)
                    {
                        BookmarkDelete(i);

                        FileClose(TextHandle);

                        continue;
                    }
                    FileClose(TextHandle);
                }
                else
                {
                     if ((0 == strncmp((char *)TextFileInfo.FilePath, (char *)pBookmark->FilePath,MAX_PATH_NAME_LENGTH))
                    && (0 == strncmp((char *)TextFileInfo.FileName, (char *)pBookmark->FileName,MAX_FILE_NAME_LENGTH)))
                    {
                        if( TextBookmarkLoad(i))
                        {
                            ReturnValue =  1;
                        }

                    }

                }
          }
    }

#if 0
    BookmarkDistill(&TextBookmark,pBookmark,0);
    if (FALSE == pBookmark->bIsEmpty)
    {
        if ((0 == strcmp((char *)TextFileInfo.FilePath, (char *)pBookmark->FilePath))
                && (0 == strcmp((char *)TextFileInfo.FileName, (char *)pBookmark->FileName)))
          {
            if( TextBookmarkLoad(0))
            {
              return 1;
            }
          }
    }
#endif

    return ReturnValue;
}

/*
--------------------------------------------------------------------------------
  Function name : void TextSaveBookmark (INT16 Item)
  Author        : yangwenjie
  Description   : save book mark

  Input         : Item : index of book mark

  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
void TextBookmarkSave (INT16 Item)
{
    TEXT_BOOKMARK_UNION     TextBookmark;
    TEXT_BOOKMARK_STRUCT    Bookmark;

    memset(TextBookmark.Buffer,0,MAX_BOOKMARK_LENGTH);

    //read out all book marks
    ReadReservedData(BOOKMARK_BLK, (uint8*)TextBookmark.Buffer, MAX_BOOKMARK_LENGTH);

    Bookmark.bIsEmpty           = FALSE;
    Bookmark.uFileSize          = TextFileInfo.FileSize;
    Bookmark.uStartOffset       = EbookStartFileOffset;
    Bookmark.BookCurrentPage    = EbookCurrentPages;
#ifdef  _DISPLAY_ToTALPAGE_
    Bookmark.BookTotalPage      = EbookTotalPages;
    Bookmark.BookPageOffset     = EbookPageCountOffset;
#endif
    Bookmark.PreviousPageOffset = gPreviousPageOffset;
    Bookmark.uEndOffset         = EbookEndFileOffset;

    memcpy((UINT8*)Bookmark.FilePath, (UINT8*)TextFileInfo.FilePath,    MAX_PATH_NAME_LENGTH);
    memcpy((UINT8*)Bookmark.FileName, (UINT8*)TextFileInfo.FileName,    MAX_FILE_NAME_LENGTH);
    memcpy((UINT8*)Bookmark.LongFileName,(UINT8*)TextFileInfo.LongFileName,2*MAX_LONG_FILE_NAME_LENGTH);

    //change current book mark
    TextBookmark.S.Bookmark[Item] = Bookmark;

    //save current book mark
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
    WriteReservedData(BOOKMARK_BLK, (uint8*)TextBookmark.Buffer, MAX_BOOKMARK_LENGTH);

}
/*
--------------------------------------------------------------------------------
  Function name : void TextDistillBookmark (TEXT_BOOKMARK_STRUCT *pBookmark, INT16 id)
  Author        : yangwenjie
  Description   : load ebook mark

  Input         : pBookmark: book mark information
                  Item : index of book mark

  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_CODE_
UINT16 TextBookmarkLoad (INT16 Item)
{
    TEXT_BOOKMARK_STRUCT Bookmark;
    TEXT_BOOKMARK_UNION  TextBookmark;
    HANDLE TextHandle;
    UINT32 i;
    uint8  TempBuff[3];

    BookmarkDistill(&TextBookmark,&Bookmark,Item);
    if(FALSE==Bookmark.bIsEmpty)
    {
        //if ((TextHandle = FileOpen(TxtSysFileInfo.Fdt.Name,TxtSysFileInfo.FindData.Clus, TxtSysFileInfo.FindData.Index - 1,FS_FAT, (uint8*)"R")) != NOT_OPEN_FILE)
        if ((TextHandle = FileOpenA(Bookmark.FilePath, Bookmark.FileName,(uint8*)"R")) != NOT_OPEN_FILE)
        {
            FileClose(hTextFile);
            hTextFile = TextHandle;

            FileRead((uint8 *)TempBuff, 2, hTextFile);

            if((TempBuff[0] == 0xFF) && (TempBuff[1] == 0xFE))
            {
                TextFileInfo.CodeType = UNICODE_TEXT;
            }
            else
            {
                TextFileInfo.CodeType = ANSI_TEXT;
            }

            memcpy(TextFileInfo.FilePath,Bookmark.FilePath,MAX_PATH_NAME_LENGTH);
            memcpy(TextFileInfo.FileName,Bookmark.FileName,MAX_FILE_NAME_LENGTH);
            memcpy((UINT8*)TextFileInfo.LongFileName,(UINT8*)Bookmark.LongFileName,2*MAX_LONG_FILE_NAME_LENGTH);

            FileSeek(0, SEEK_SET, hTextFile);
            TextFileInfo.FileSize = Bookmark.uFileSize;
            EbookStartFileOffset = Bookmark.uStartOffset;
            EbookEndFileOffset   = EbookStartFileOffset;
            gPreviousPageOffset  = EbookStartFileOffset;

            for(i = 0; i < TXT_DISP_LINE_MAX; i++)
            {
                TextKeyDownProc();
            }

            EbookStartFileOffset = Bookmark.uStartOffset;

            EbookCurrentPages    = Bookmark.BookCurrentPage;
 #ifdef   _DISPLAY_ToTALPAGE_
            EbookTotalPages      = Bookmark.BookTotalPage;
            EbookPageCountOffset = Bookmark.BookPageOffset;
 #endif
            return 1;
        }
        else
        {
            BookmarkDelete(Item);
            FileClose(TextHandle);
            return 0;
        }
    }
}

/*
********************************************************************************
*
*                         End of TextBookmark.c
*
********************************************************************************
*/
#endif

