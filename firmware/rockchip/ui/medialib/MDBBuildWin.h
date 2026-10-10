/*
********************************************************************************
*                   Copyright (c) 2012,SunChuanHu
*                         All rights reserved.
*
* File Name：   MDBBuildWin.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               SunChuanHu      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*//*
********************************************************************************
*                   Copyright (c) 2012,SunChuanHu
*                         All rights reserved.
*
* File Name：   MDBBuildWin.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               SunChuanHu      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#include "SysConfig.h"

#ifndef __MDBBUILDWIN_H_
#define __MDBBUILDWIN_H_

#undef  EXT
#ifdef _IN_MDBBUILDWIN_
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
#define _ATTR_MDBBUILDWIN_CODE_         
#define _ATTR_MDBBUILDWIN_DATA_         
#define _ATTR_MDBBUILDWIN_BSS_          

#define     MEDIA_UPDATA_TXT_X             0    // the start point of x direction.
#define     MEDIA_UPDATA_TXT_Y             80    //the start point of y direction.
#define     MEDIA_UPDATA_TXT_XSIZE         128   //X size
#define     MEDIA_UPDATA_TXT_YSIZE         12   //Y size

#define     MDB_TITLE_TXT_X            34
#define     MDB_TITLE_TXT_Y            146
#define     MDB_TITLE_TXT_XSIZE        61
#define     MDB_TITLE_TXT_YSIZE        12

/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
extern void MdbBuildWinInit(void *pArg);
extern void MdbBuildWinDeInit(void);

extern UINT32 MdbBuildWinService(void);
extern UINT32 MdbBuildWinKey(void);
extern void   MdbBuildWinDisplay(void);

extern void MedialibUpdateDisplayHook(void);

/*
--------------------------------------------------------------------------------
  
  Description:  窗口结构体定义
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_MDBBUILDWIN_
_ATTR_MDBBUILDWIN_DATA_ WIN MdbBuildWin = {
    
    NULL,
    NULL,
    
    MdbBuildWinService,               //窗口服务处理函数
    MdbBuildWinKey,                   //窗口按键处理程序
    MdbBuildWinDisplay,               //窗口显示处理程序
    
    MdbBuildWinInit,                  //窗口初始化函数
    MdbBuildWinDeInit                 //窗口反初始化函数
    
};
#else 
_ATTR_MDBBUILDWIN_DATA_ EXT WIN MdbBuildWin;
#endif

/*
********************************************************************************
*
*                         End of MedialibWin.h
*
********************************************************************************
*/

#endif

