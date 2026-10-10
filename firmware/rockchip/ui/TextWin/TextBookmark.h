/*
********************************************************************************
*          Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All Rights Reserved
*
*Description: ebook book mark
*
$Header: /cvs_database/cvs_server_rk26/mp3_project/RK260x_SDK/Application/Text/TextSet/TextBookmark.h,v 1.3 2008/01/03 07:04:06 Fengmx Exp $
$Author: Fengmx $
$Date: 2008/01/03 07:04:06 $
$Revision: 1.3 $
********************************************************************************
*/
#ifndef _TEXT_BOOKMARK_H
#define _TEXT_BOOKMARK_H

#undef  EXT
#ifdef  _IN_TEXT_BOOKMARK
#define EXT 
#else
#define EXT extern
#endif


#define _ATTR_BOOKMASK_CODE_                    
#define _ATTR_BOOKMASK_DATA_                    
#define _ATTR_BOOKMASK_BSS_                     

/*
*-------------------------------------------------------------------------------
*
*                            Macro define
*
*-------------------------------------------------------------------------------
*/
#define     MAX_BOOKMARK_LENGTH         2048    //total occupy memory space it has relationship wich flash type,it can not changed.
#define     SINGLE_BOOKMARK_LENGTH      (sizeof(TEXT_BOOKMARK_STRUCT))  //occupy memory of each mark. 

#define     MAX_BOOKMARK_NUMBER         (MAX_BOOKMARK_LENGTH / SINGLE_BOOKMARK_LENGTH)  //max book mark number. 
#define     MAX_USER_BOOKMARK_NUMBER    (MAX_BOOKMARK_NUMBER - 1)       // the max book mark number that user can use,left one for system bookmark.

#define     SYSTEM_BOOKMARK_ID          (MAX_BOOKMARK_NUMBER - 1)       // system bookmark ID

#define     TEXT_SAVE_BOOKMARK          0        
#define     TEXT_LOAD_BOOKMARK          1        


#define     MAX_BOOK_MARK_NAME_LENGTH       MAX_LONG_FILE_NAME_LENGTH

#define     TXT_BOOKMARK_COUNTER        3
#define     TXT_BOOKMARK_BUF_SIZE       12
#define     TXT_BOOKMARK_Pagecount      2 //row number of one screen

#define     TXT_BOOKMARK_TYPE_SAVE      1
#define     TXT_BOOKMARK_TYPE_DISTILL   2

#define     TXT_BOOKMASK_SELECT_YSIZE   68
#define     TXT_BOOKMASK_SELECT_XSIZE   115
#define     TXT_BOOKMASK_SELECT_X       15
#define     TXT_BOOKMASK_SELECT_Y       42

#define     TXT_BOOKMAKS_TEXT_OFFS_X    TXT_BOOKMASK_SELECT_X + 12
#define     TXT_BOOKMAKS_TEXT_OFFS_Y    TXT_BOOKMASK_SELECT_Y + 2
#define     TXT_BOOKMAKS_TEXT_DIST_Y    0

/*
--------------------------------------------------------------------------------
  
                        Struct Define
  
--------------------------------------------------------------------------------
*/

typedef struct {
    BOOL   IsEmpty;  
    BOOL   IsBookMarkDislay;
    UINT8  BookMaskType;
    UINT8  BookMarkCont;
    UINT8  BookMarkContBack;
    BOOL   IsScrollBookMask;
#ifdef   _DISPLAY_ToTALPAGE_
    UINT32 BookTotalPage;
#endif
    UINT32 BookCurrentPage;
    PicturePartInfo PictureInfo;
    LCD_RECT    r;
    UINT16  LongFileName[MAX_LONG_FILE_NAME_LENGTH];
    UINT32 SystickCount;
}BOOK_MARK_INFO;


typedef struct {
    BOOL    bIsEmpty;           //whether current bookmark is null.
                                // FALSE: not empty; TRUE: empty
    UINT32  uFileSize;          // text file size
    UINT32  uStartOffset;       // current display text file start 
    UINT32  uEndOffset;  
#ifdef   _DISPLAY_ToTALPAGE_
    UINT32  BookTotalPage;
    UINT32  BookPageOffset;
#endif
    UINT32  BookCurrentPage;    
    UINT32  PreviousPageOffset ; 
    UINT8   FilePath[MAX_PATH_NAME_LENGTH];             //ebook file path 
    UINT8   FileName[MAX_FILE_NAME_LENGTH];             // ebook short file name
    UINT16  LongFileName[MAX_LONG_FILE_NAME_LENGTH];    // ebook long file name
    
} TEXT_BOOKMARK_STRUCT;

typedef union 
{
    struct 
    {
        TEXT_BOOKMARK_STRUCT    Bookmark[MAX_BOOKMARK_NUMBER];
    }S;
    
    UINT8  Buffer[MAX_BOOKMARK_LENGTH];
} TEXT_BOOKMARK_UNION;


/*
--------------------------------------------------------------------------------
  
                        Variable Define
  
--------------------------------------------------------------------------------
*/
_ATTR_BOOKMASK_BSS_ EXT BOOK_MARK_INFO      BookMaskInfo;
_ATTR_BOOKMASK_BSS_ EXT UINT16              ScrollBookMask[MAX_BOOK_MARK_NAME_LENGTH+9];

/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/

EXT void BookMarkWinInit(void *pArg);
EXT void BookMarkWinDeInit (void);
EXT UINT32  BookMarkWinService(void);
EXT UINT32  BookMarkWinKey(void);
EXT void    BookMarkWinPaint(void);
EXT void BookmarkDisplayTitle (void);
EXT void BookmarkDisplayItem(void);
EXT void BookmarkDelete(INT16 Item);
EXT void BookmarkDistill(TEXT_BOOKMARK_UNION *TextBookmark,TEXT_BOOKMARK_STRUCT *pBookmark, INT16 Item);
EXT void BookMaskDisplay(UINT16 Item);
EXT void BookMarkPageDisPlay(void);
EXT UINT16 TextBookmarkInit (void);
EXT void TextBookmarkSave (INT16 Item);
EXT UINT16 TextBookmarkLoad (INT16 Item);
EXT void BookmarkSaveTotalPage ();




#ifdef _IN_TEXT_BOOKMARK
_ATTR_TEXT_DATA_ WIN BookMarkWin = {
    
    NULL,
    NULL,
    
    BookMarkWinService,               //window service handle function.
    BookMarkWinKey,                   //window key service handle function.
    BookMarkWinPaint,               //window display service handle function.
    
    BookMarkWinInit,                  //window initial handle function.
    BookMarkWinDeInit                 //window auti-initial handle function.
    
};
#else
_ATTR_TEXT_DATA_ EXT WIN BookMarkWin;
#endif


#endif



