/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  DIALOGBOX.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               anzhiguo      2009-2-27         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef  _DIALOG_LOWPOWER_H
#define  _DIALOG_LOWPOWER_H

#undef  EXT
#ifdef  _IN_DIALOG_LOWPOWER_
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
#define _ATTR_LOWERPOWER_CODE_         
#define _ATTR_LOWERPOWER_DATA_         
#define _ATTR_LOWERPOWER_BSS_          

#define LOWERPOWER_DISPLAY_TIME        3
/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/

_ATTR_LOWERPOWER_BSS_ EXT UINT32 DialogLowPowerSystickCounterBack;



/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
extern void DialogLowPowerInit(void *pArg);
extern void DialogLowPowerDeInit(void);
extern uint32 DialogLowPowerService(void);
extern uint32 DialogLowPowerKey(void);
extern void DialogLowPowerDisplay(void);

/*
--------------------------------------------------------------------------------
  
  Description:  窗口结构体定义
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_DIALOG_LOWPOWER_
_ATTR_LOWERPOWER_DATA_ WIN LowPowerWin = {
    
    NULL,
    NULL,
    
    DialogLowPowerService,              //窗口服务处理函数
    DialogLowPowerKey,                  //窗口按键处理程序
    DialogLowPowerDisplay,              //窗口显示处理程序
    
    DialogLowPowerInit,                 //窗口初始化函数
    DialogLowPowerDeInit                //窗口反初始化函数
    
};
#else 
_ATTR_LOWERPOWER_DATA_ EXT WIN LowPowerWin;
#endif

/*
********************************************************************************
*
*                         End of DIALOGBOX.h
*
********************************************************************************
*/
#endif

