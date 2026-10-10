/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   SetMenu.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               anzhiguo      2009-3-3          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SETMENU_H_
#define _SETMENU_H_

#undef  EXT
#ifdef _IN_SETMENU_
#define EXT
#else
#define EXT extern
#endif

/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                      */
/*                                                                            */
/******************************************************************************/

//setting menu permanent code.
#define _ATTR_SYS_SET_CODE_         
#define _ATTR_SYS_SET_DATA_         
#define _ATTR_SYS_SET_BSS_          

//setting menu initial code
#define _ATTR_SYS_SET_INIT_CODE_    
#define _ATTR_SYS_SET_INIT_DATA_    
#define _ATTR_SYS_SET_INIT_BSS_     

//setting menu auti-initial code
#define _ATTR_SYS_SET_DEINIT_CODE_  
#define _ATTR_SYS_SET_DEINIT_DATA_  
#define _ATTR_SYS_SET_DEINIT_BSS_   

//setting menu dispatch code
#define _ATTR_SYS_SET_SERVICE_CODE_ 
#define _ATTR_SYS_SET_SERVICE_DATA_ 
#define _ATTR_SYS_SET_SERVICE_BSS_  

//SysSetup Text Info define
#define 	SETMENU_TITLE_TXT_X 			0	 //title info
#define 	SETMENU_TITLE_TXT_Y 			146    
#define 	SETMENU_TITLE_TXT_XSIZE 		128   
#define 	SETMENU_TITLE_TXT_YSIZE 		12	 
			
#define 	SETMENU_ITEM_TXT_X				20	  //frist item info
#define 	SETMENU_ITEM_TXT_Y				5	 
#define 	SETMENU_ITEM_TXT_XSIZE			100	 
#define 	SETMENU_ITEM_TXT_YSIZE			12	  

//SysSetup Text Info define
#define 	SETMENU_PIC_X			  5    
#define 	SETMENU_PIC_Y			  5    
#define 	SETMENU_PIC_XSIZE		  12   
#define 	SETMENU_PIC_YSIZE		  12   

                          
#define  SYS_SET_MAX_ITEM_PER_SCREEN          8                            

#define  SYS_SET_ID_NONE                   0xFFFF   //if no this ID,then read out 0xffff

#define  SYS_SET_GLOBE_TXT_INFO_STRUCT_NUM    3     //menu information structure group size

//the macros below is the same order with enum of audio.only use EQ_PLAYFX in setmenu.
typedef enum _DIS_EQ
{
    DIS_EQ_HEAVY, 
    DIS_EQ_POP,
    DIS_EQ_JAZZ, 
    DIS_EQ_UNIQUE,
    DIS_EQ_USER,
    DIS_EQ_USER_EDIT,
    DIS_EQ_NOR,
    DIS_EQ_BASS
}
DIS_EQ;

//------------------------------------------------------------------------------


/******************************************************************************/
/*                                                                            */
/*                          Struct Define                                     */
/*                                                                            */
/******************************************************************************/

typedef struct SetMenuItemInfoStruct
{  
    UINT8  TotalDispItem;     //one screen max display item number.
    UINT8  CurDispItemFirst;  //the frist item id in current screen.
    UINT8  CurDispItem;       //the corsor position of current screen
    UINT8  PrevItem;                          
    UINT8  CurItemInNum;      //current item index in resource ID.
    UINT8  CurItemInID;       //the ID of current cursor item in all items,  
    UINT8  SelItemInID;       //the ID of selected items,  
    UINT8 FatherItemID;       //the father item id of current item,it is used to get menu information from resource when return to up level.
                              //if its value is 0xffff, it means current menu is frist level menu
    MENU_TEXT_INFO_STRUCT  *SetMenuTxtInfo; //setting menu text structure.
   
} SETMENU_INFO_STRUCT;

/******************************************************************************/
/*                                                                            */
/*                          Variable Define                                   */
/*                                                                            */
/******************************************************************************/
_ATTR_SYS_SET_BSS_    EXT   SETMENU_INFO_STRUCT SetMenuItem[5];
     
_ATTR_SYS_SET_BSS_    EXT    UINT32  SysSetEqBackup;     

_ATTR_SYS_SET_BSS_    EXT    UINT32 MusicPlayMSEqSwitch;

_ATTR_SYS_SET_BSS_    EXT    UINT32  BacklightBackup;

_ATTR_SYS_SET_BSS_    EXT    UINT16  SetLanguageItem[LANGUAGE_MAX_COUNT];

/*
--------------------------------------------------------------------------------------------
structure group g_SYSSET_sMenuTxtInfo explanation:
    this structure group are used to store father and child menu item information,when switch the 
    parent-child relationship,it need to apply for new structure to store new informatin,at the same 
    time to free the old structure,the algorithm to apply for new sturct is follow:
    SysSet_sMenuTxtInfo[(SysSet_ReqMenuTxtInfoStructFlag++) % SYS_SET_GLOBE_TXT_INFO_STRUCT_NUM]
    we kown that the range of (SysSet_ReqMenuTxtInfoStructFlag++) % SYS_SET_GLOBE_TXT_INFO_STRUCT_NUM
    is {0,1,...(SYS_SET_GLOBE_TXT_INFO_STRUCT_NUM-1)} by mathematical operation,so this value can be
    the index of this group.
    
    
--------------------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BSS_    EXT    MENU_TEXT_INFO_STRUCT    SetMenuTxtInfo[SYS_SET_GLOBE_TXT_INFO_STRUCT_NUM]; 

_ATTR_SYS_SET_BSS_    EXT    UINT32                   ReqMenuTxtInfoStructFlag;  //apply for menu sturcture information flag bit

/******************************************************************************/
/*                                                                            */
/*                         Function Declare                                   */
/*                                                                            */
/******************************************************************************/

EXT UINT32 ExcuteSetMenuFunc(UINT32 menuId, UINT32 itemNum);
EXT UINT32 MenuKeyPro(UINT16 itemNum);

EXT void   SetMenuInit(void *pArg);
EXT void   SetMenuDeInit(void);

EXT UINT32 SetMenuService(void);
EXT UINT32 SetMenuKey(void);
EXT void SetMenuPaint(void);


/*
--------------------------------------------------------------------------------
  
  Description:  window sturcture definition
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_SETMENU_
_ATTR_SYS_SET_DATA_ WIN SetMenuWin = {
    
    NULL,
    NULL,
    
    SetMenuService,               //window service handle function.
    SetMenuKey,                   //window key service handle function.
    SetMenuPaint,                 //window display service handle function.
    
    SetMenuInit,                  //window initial handle function.
    SetMenuDeInit                 //window auti-initial handle function.
    
};
#else 
_ATTR_SYS_SET_DATA_ EXT WIN SetMenuWin;
#endif

/*
********************************************************************************
*
*                         End of Win.h
*
********************************************************************************
*/
#endif

