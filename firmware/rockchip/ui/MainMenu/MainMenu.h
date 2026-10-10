/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   MainMenu.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _MAINMENU_H_
#define _MAINMENU_H_

#undef  EXT
#ifdef _IN_MAINMENU_
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
//main menu permanent code
#define _ATTR_MAIN_MENU_CODE_         
#define _ATTR_MAIN_MENU_DATA_         
#define _ATTR_MAIN_MENU_BSS_          

//main menu initial code
#define _ATTR_MAIN_MENU_INIT_CODE_    
#define _ATTR_MAIN_MENU_INIT_DATA_    
#define _ATTR_MAIN_MENU_INIT_BSS_     

//main menu auti-initial code
#define _ATTR_MAIN_MENU_DEINIT_CODE_  
#define _ATTR_MAIN_MENU_DEINIT_DATA_  
#define _ATTR_MAIN_MENU_DEINIT_BSS_   

//main menu content switch code
#define _ATTR_MAIN_MENU_SERVICE_CODE_ 
#define _ATTR_MAIN_MENU_SERVICE_DATA_ 
#define _ATTR_MAIN_MENU_SERVICE_BSS_  

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
typedef enum
{
    #if 1//def _MUSIC_
    MAINMENU_ID_MUSIC= (UINT32)0,//audio module
    #endif

    #if 1//def _VIDEO_
    MAINMENU_ID_VIDEO,         //video
    #endif

    #if 1//def _RADIO_
    MAINMENU_ID_RADIO,           //FM
    #endif

    #if 1//def _PICTURE_
    MAINMENU_ID_PICTURE,         //picture
    #endif

    #if 1//def _EBOOK_
    MAINMENU_ID_EBOOK,          //picture
    #endif

    #if 1//def _RECORD_
    MAINMENU_ID_RECORD,
    #endif

    #if 1//def _BROWSER_
    MAINMENU_ID_BROWSER,         //brower
    #endif

    #if 1//def _SYSSET_
    MAINMENU_ID_SETMENU,          //setting module
    #endif

    MAINMENU_ID_MAXNUM,

} MAINMENU_ID;

typedef struct _MainConfig
{
    BOOL       DispFlag;    //这项目显示否
    UINT8          Item;        //这个项目是主菜单中的第几项 /暂无用  /可以根据MAINMENU_ID来判断顺序
    MAINMENU_ID  TaskId;     //本项目执行的任务编号
    UINT16 ImageId; //本项目的显示图片ID
    UINT16 TextId;  //本项目的显示文字ID
} Main_Config;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/
#define MainMenuBatteryLevel       gBattery.Batt_Level
#define MainmenuHoldState          HoldState

_ATTR_MAIN_MENU_BSS_ EXT UINT16    MenuId; //moduel id of main menu interface
_ATTR_MAIN_MENU_BSS_ EXT UINT16    HomeDemoMode; //moduel id of main menu interface

/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
extern void MainMenuInit(void *pArg);
extern void MainMenuDeInit(void);

extern UINT32 MainMenuService(void);
extern UINT32 MainMenuKey(void);
extern void MainMenuDisplay(void);

INT16 MainMenuModeKey(void);


/*
--------------------------------------------------------------------------------

  Description:  window sturcture definition

--------------------------------------------------------------------------------
*/
#ifdef _IN_MAINMENU_
_ATTR_MAIN_MENU_DATA_ WIN MainMenuWin = {

    NULL,
    NULL,

    MainMenuService,               //window service handle function.
    MainMenuKey,                   //window key service handle function.
    MainMenuDisplay,               //window display service handle function.

    MainMenuInit,                  //window initial handle function.
    MainMenuDeInit                 //window auti-initial handle function.

};
#else
_ATTR_MAIN_MENU_DATA_ EXT WIN MainMenuWin;
#endif


#ifdef _IN_MAINMENU_
_ATTR_MAIN_MENU_DATA_   Main_Config MainConfig[]= {
    {1,     MAINMENU_ID_MUSIC,          MAINMENU_ID_MUSIC,          IMG_ID_MAINMENU_MUSIC,      SID_MUSIC},         //音乐

#if 1//def _RECORD_
    {1,     MAINMENU_ID_RECORD,         MAINMENU_ID_RECORD,         IMG_ID_MAINMENU_RECOD,      SID_RECORD},        //录音
#endif

#if 1//def _EBOOK_
    {1,     MAINMENU_ID_EBOOK,          MAINMENU_ID_EBOOK,          IMG_ID_MAINMENU_BOOK,       SID_TEXT},          //电子书
#endif

#if 1// def _BROWSER_
    {1,     MAINMENU_ID_BROWSER,        MAINMENU_ID_BROWSER,        IMG_ID_MAINMENU_BROWSER,    SID_EXPLORER},      //浏览器
#endif

#if 1//def _SYSSET_
    {1,     MAINMENU_ID_SETMENU,        MAINMENU_ID_SETMENU,        IMG_ID_MAINMENU_SETMENU,    SID_SETTINGS},      //设置
#endif

#if 1 //def _VIDEO_
    {1,     MAINMENU_ID_VIDEO,          MAINMENU_ID_VIDEO,          IMG_ID_MAINMENU_VEDIO,      SID_VIDEO},         //视频
#endif

#if 1//def _PICTURE_
    {1,     MAINMENU_ID_PICTURE,        MAINMENU_ID_PICTURE,        IMG_ID_MAINMENU_PHOTO,      SID_PHOTO},         //图片
#endif

#if 1//def _RADIO_
    {1,     MAINMENU_ID_RADIO,          MAINMENU_ID_RADIO,          IMG_ID_MAINMENU_FM,         SID_RADIO},         //收音机
#endif

};
#else
_ATTR_MAIN_MENU_DATA_ EXT  Main_Config MainConfig;
#endif

/*
********************************************************************************
*
*                         End of Win.h
*
********************************************************************************
*/
#endif

