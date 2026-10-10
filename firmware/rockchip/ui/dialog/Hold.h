/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  Hold.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               anzhiguo      2009-2-27         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef  _DIALOG_HOLD_H
#define  _DIALOG_HOLD_H

#undef  EXT
#ifdef  _IN_DIALOG_HOLD_
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
#define _ATTR_HOLD_CODE_         
#define _ATTR_HOLD_DATA_         
#define _ATTR_HOLD_BSS_          

#define     HOLD_DISPLAY_TIME    3//message box display 5 second
#define     HOLD_STATE_ON        0
#define     HOLD_STATE_OFF       1

/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/
_ATTR_HOLD_BSS_ EXT UINT32      HoldSystickCounterBack;
_ATTR_HOLD_BSS_ EXT uint16      HoldState;


/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
extern void DialogHoldInit(void *pArg);
extern void DialogHoldDeInit(void);
extern uint32 DialogHoldService(void);
extern uint32 DialogHoldKey(void);
extern void DialogHoldDisplay(void);

/*
--------------------------------------------------------------------------------
  
  Description:  window sturcture definition
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_DIALOG_HOLD_
_ATTR_HOLD_DATA_ WIN HoldWin = {
    
    NULL,
    NULL,
    
    DialogHoldService,              //window service funciton
    DialogHoldKey,                  //window key service funciton
    DialogHoldDisplay,              //window display service funciton
    
    DialogHoldInit,                 //window initial funciton
    DialogHoldDeInit                //window auti-initial funciton
    
};
#else 
_ATTR_HOLD_DATA_ EXT WIN HoldWin;
#endif

/*
********************************************************************************
*
*                         End of Hold.h
*
********************************************************************************
*/
#endif

