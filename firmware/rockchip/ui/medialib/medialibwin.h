/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   medialibwin.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _MEDIALIBWIN_H_
#define _MEDIALIBWIN_H_

#undef  EXT
#ifdef _IN_MEDIALIBWIN_
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
//常驻代码
#define _ATTR_MEDIALIBWIN_CODE_         
#define _ATTR_MEDIALIBWIN_DATA_         
#define _ATTR_MEDIALIBWIN_BSS_          

//初始化代码
#define _ATTR_MEDIALIBWIN_INIT_CODE_    
#define _ATTR_MEDIALIBWIN_INIT_DATA_    
#define _ATTR_MEDIALIBWIN_INIT_BSS_     

//反初始化代码
#define _ATTR_MEDIALIBWIN_DEINIT_CODE_  
#define _ATTR_MEDIALIBWIN_DEINIT_DATA_  
#define _ATTR_MEDIALIBWIN_DEINIT_BSS_   

//可调度代码
#define _ATTR_MEDIALIBWIN_SERVICE_CODE_  
#define _ATTR_MEDIALIBWIN_SERVICE_DATA_ 
#define _ATTR_MEDIALIBWIN_SERVICE_BSS_   

#define     MEDIALIB_TITLE_TXT_X            34
#define     MEDIALIB_TITLE_TXT_Y            146
#define     MEDIALIB_TITLE_TXT_XSIZE        61
#define     MEDIALIB_TITLE_TXT_YSIZE        12

#define     MEDIA_ITEM_X                    20  //x direction start point
#define     MEDIA_ITEM_Y                    5  //y direction start point
#define     MEDIA_ITEM_XSIZE                77  //item x size
#define     MEDIA_ITEM_YSIZE                17  //item y size
#define     MEDIA_ITEM_CHARYSIZE            12  //the height of item
/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
#ifdef _MEDIAUPDATE_
#define  MAX_TOTLE_ITEM_NUM             11
#elif defined(_RECORD_)
#define  MAX_TOTLE_ITEM_NUM             7
#else
#define  MAX_TOTLE_ITEM_NUM             7
#endif
#define  MAX_ITEM_NUM_MEDIA_DISP       8//	3	//the max display item number that one screeen display

#define     SORT_TYPE_SEL_NOW_PLAY      0
#define     SORT_TYPE_SEL_ID3TITLE      1
#define     SORT_TYPE_SEL_ID3SINGER     2
#define     SORT_TYPE_SEL_ID3ALBUM      3

#define     SORT_TYPE_SEL_GENRE         4//流派
#define     SORT_TYPE_PLAY_LIST         5
#define     SORT_TYPE_SEL_FOLDER        6//文件夹浏览

#ifdef _RECORD_
#define     MUSIC_TYPE_SEL_RECORDFILE   7
#define     MUSIC_TYPE_SEL_FMFILE       8
#define     MUSIC_TYPE_SEL_MYFAVORITE   9
#define     MEDIA_SORT_UPDATE_SEL       10
#define     MEDIA_MUSIC_BREAKPOINT      11
#define     MUSIC_TYPE_SEL_RECORDFILE_DEL 12

#else
#define     MUSIC_TYPE_SEL_MYFAVORITE     7
#define     MEDIA_SORT_UPDATE_SEL         8
#define     MEDIA_MUSIC_BREAKPOINT        9
#endif

typedef struct _MedialibWinDataStruct
{

    UINT8                      TotalItem;                      //total items.
    UINT8                      CurStartIndex;                  //the start index in current page
    UINT8                      CurPointer;                     //cursor location.
    UINT8                      PrePointer;                     //last time cursor location
    UINT8                      TotalDispItem;                  //one screen item number
    UINT8                      CurNum;                         //cursor item index

} MedialibWinDataStruct;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/

//#define MainMenuBatteryLevel       gBattery.Batt_Level
//#define MainmenuHoldState          0

_ATTR_MEDIALIBWIN_BSS_ EXT MedialibWinDataStruct    MedialibWinItemData;

#ifdef _IN_MEDIALIBWIN_

#ifdef _MEDIAUPDATE_
EXT _ATTR_MEDIALIBWIN_DATA_ UINT16 MediaItem[MAX_TOTLE_ITEM_NUM][3] ={
                                                    {IMG_ID_NOWPLAY,SID_PLAYING,SORT_TYPE_SEL_NOW_PLAY}, //playing
                                                    {IMG_ID_ALLMUSIC,SID_ALL_MUISC_FILE,SORT_TYPE_SEL_ID3TITLE},//all music
                                                    {IMG_ID_SINGER,SID_ARTIST,SORT_TYPE_SEL_ID3SINGER}, //artist
                                                    {IMG_ID_ABLUM,SID_ALBUM,SORT_TYPE_SEL_ID3ALBUM},//album
                                                    {IMG_ID_GENRE,SID_GENRE,SORT_TYPE_SEL_GENRE},  //genre
                                                    {NULL,SID_PLAY_LIST,SORT_TYPE_PLAY_LIST},  //play list
                                                    {NULL,SID_DIR_LIST,SORT_TYPE_SEL_FOLDER},//目录列表
                                                    {NULL,SID_RECORE_FILE,MUSIC_TYPE_SEL_RECORDFILE}, //录音文件
                                                    {NULL,SID_RECORE_FILE,MUSIC_TYPE_SEL_FMFILE}, //录音文件
                                                    {NULL,SID_MY_FAVORITE,MUSIC_TYPE_SEL_MYFAVORITE}, //我的收藏夹
                                                    {NULL,SID_UPDATE_MEDIALIB,MEDIA_SORT_UPDATE_SEL}, //更新媒体库
                                                    //{NULL,SID_HOLDON_PLAY,MEDIA_MUSIC_BREAKPOINT} //断点播放

                                                   };
#elif defined(_RECORD_)
EXT _ATTR_MEDIALIBWIN_DATA_ UINT16 MediaItem[MAX_TOTLE_ITEM_NUM][3] ={
                                                    {IMG_ID_NOWPLAY,SID_PLAYING,SORT_TYPE_SEL_NOW_PLAY}, //playing
                                                    {IMG_ID_ALLMUSIC,SID_ALL_MUISC_FILE,SORT_TYPE_SEL_ID3TITLE},//all music
                                                    {IMG_ID_SINGER,SID_ARTIST,SORT_TYPE_SEL_ID3SINGER}, //artist
                                                    {IMG_ID_ABLUM,SID_ALBUM,SORT_TYPE_SEL_ID3ALBUM},//album
                                                    {IMG_ID_GENRE,SID_GENRE,SORT_TYPE_SEL_GENRE},  //genre
                                                    {IMG_ID_LIST,SID_DIR_LIST,SORT_TYPE_SEL_FOLDER},//direction list

                                                    //{NULL,SID_PLAY_LIST,SORT_TYPE_PLAY_LIST},  //play list
                                                    //{NULL,SID_RECORE_FILE,MUSIC_TYPE_SEL_RECORDFILE}, //录音文件
                                                    //{NULL,SID_RECORE_FILE,MUSIC_TYPE_SEL_FMFILE}, //录音文件
                                                    {IMG_ID_LIST,SID_MY_FAVORITE,MUSIC_TYPE_SEL_MYFAVORITE}, //我的收藏夹
                                                    //{NULL,SID_UPDATE_MEDIALIB,MEDIA_SORT_UPDATE_SEL}, //更新媒体库
                                                    //{NULL,SID_HOLDON_PLAY,MEDIA_MUSIC_BREAKPOINT} //断点播放

                                                   };
#else
EXT _ATTR_MEDIALIBWIN_DATA_ UINT16 MediaItem[MAX_TOTLE_ITEM_NUM][3] ={
                                                    {IMG_ID_NOWPLAY,SID_PLAYING,SORT_TYPE_SEL_NOW_PLAY}, //playing
                                                    {IMG_ID_ALLMUSIC,SID_ALL_MUISC_FILE,SORT_TYPE_SEL_ID3TITLE},//all music
                                                    {IMG_ID_SINGER,SID_ARTIST,SORT_TYPE_SEL_ID3SINGER}, //artist
                                                    {IMG_ID_ABLUM,SID_ALBUM,SORT_TYPE_SEL_ID3ALBUM},//album
                                                    {IMG_ID_GENRE,SID_GENRE,SORT_TYPE_SEL_GENRE},  //genre
                                                    {IMG_ID_LIST,SID_DIR_LIST,SORT_TYPE_SEL_FOLDER},//direction list
                                                    //{NULL,SID_PLAY_LIST,SORT_TYPE_PLAY_LIST},  //play list
                                                    {IMG_ID_LIST,SID_MY_FAVORITE,MUSIC_TYPE_SEL_MYFAVORITE}, //我的收藏夹
                                                    //{NULL,SID_UPDATE_MEDIALIB,MEDIA_SORT_UPDATE_SEL}, //更新媒体库
                                                    //{NULL,SID_HOLDON_PLAY,MEDIA_MUSIC_BREAKPOINT}, //断点播放
                                                   };
#endif
#else
EXT _ATTR_MEDIALIBWIN_DATA_ UINT16 MediaItem[MAX_TOTLE_ITEM_NUM][3];
#endif

/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
extern void MedialibWinInit(void *pArg);
extern void MedialibWinDeInit(void);

extern UINT32 MedialibWinService(void);
extern UINT32 MedialibWinKey(void);
extern void   MedialibWinDisplay(void);


/*
--------------------------------------------------------------------------------

  Description:  window sturcture definition

--------------------------------------------------------------------------------
*/
#ifdef _IN_MEDIALIBWIN_
_ATTR_MEDIALIBWIN_DATA_ WIN MedialibWin = {

    NULL,
    NULL,

    MedialibWinService,               //window service handle function.
    MedialibWinKey,                   //window key service handle function.
    MedialibWinDisplay,               //window display service handle function.

    MedialibWinInit,                  //window initial handle function.
    MedialibWinDeInit                 //window auti-initial handle function.

};
#else
_ATTR_MEDIALIBWIN_DATA_ EXT WIN MedialibWin;
#endif

/*
********************************************************************************
*
*                         End of MedialibWin.h
*
********************************************************************************
*/
#endif


