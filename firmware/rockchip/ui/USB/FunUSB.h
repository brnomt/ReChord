/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   FunUSB.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               CHENFEN      2008-3-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _FUNUSB_H_
#define _FUNUSB_H_

#undef  EXT
#ifdef _IN_FUNUSB_
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
//USB×¤´úÂë

/*
*-------------------------------------------------------------------------------
*  
*                           Struct define
*  
*-------------------------------------------------------------------------------
*/


/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/
#define UsbBatteryLevel       gBattery.Batt_Level


/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
extern void FunUSBIntDeInit(void);
extern void FunUSBInit(void *pArg);
extern void FunUSBDeInit(void);
extern UINT32 FunUSBService(void);
extern UINT32 FunUSBKey(void);
extern void FunUSBDisplay(void);

/*
--------------------------------------------------------------------------------
  
  Description:  window sturcture definition
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_FUNUSB_
_ATTR_USB_UI_DATA_ EXT WIN FunUSBWin = {
    
    NULL,
    NULL,
    
    FunUSBService,               //window service handle function.
    FunUSBKey,                   //window key service handle function.
    FunUSBDisplay,               //window display service handle function.
    
    FunUSBInit,                  //window initial handle function.
    FunUSBDeInit                 //window auti-initial handle function.
    
};
#else
_ATTR_USB_UI_DATA_ EXT WIN FunUSBWin;
#endif


/*
********************************************************************************
*
*                         End of Win.h
*
********************************************************************************
*/
#endif

