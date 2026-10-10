/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   Hook.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_HOOK_

#include "SysInclude.h"
#include "driver/GRF/hw_grf.h"   /* Grf macro (GRF_BASE) */
#include "driver/GRF/grf.h"   /* Grf register-block instance */

/*
--------------------------------------------------------------------------------
  Function name : void UsbBusyHook(void)
  Author        : ZhengYongzhi
  Description   : USB busy status display function
                  
  Input         : 
  Return        : null

  History:     <author>         <time>         <version>       
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

extern uint8 NandIOMuxRef;  //在MemDevInit 初始化此变量

/*
--------------------------------------------------------------------------------
  Function name : void UsbBusyHook(void)
  Author        : ZhengYongzhi
  Description   : 
                  
  Input         : 
  Return        : null

  History:     <author>         <time>         <version>       
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
bool CheckCard(void)
{
    bool insert=FALSE;

    #ifdef _SDCARD_
    insert = SDCardCheck(0);
    #endif
    
    return (insert);
}

/*
--------------------------------------------------------------------------------
  Function name : void UsbBusyHook(void)
  Author        : ZhengYongzhi
  Description   : USB State Detect
                  
  Input         : 
  Return        : 无

  History:     <author>         <time>         <version>       
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
bool CheckVbus(void)
{
    #if 1
    return ((Grf->GRF_SOC_USB_STATUS & 0x2)==0x2)? TRUE : FALSE;  //utmisrp_bvalid
    #else
    uint32 temp;
    temp = Gpio_GetPinLevel(GPIO_CH2,GPIOPortA_Pin2);
    DEBUG("temp = %d", temp);
    if (temp)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
    #endif
}

/*
********************************************************************************
*
*                         End of hook.c
*
********************************************************************************
*/

