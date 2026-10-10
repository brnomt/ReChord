/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   nvic.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_NVIC_

#include "DriverInclude.h"

/*
--------------------------------------------------------------------------------
  Function name : void NVICSystemReset(void)
  Author        : ZHengYongzhi
  Description   : Intended to force a large system reset of all major components
                  except for debug.
  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void NvicSystemReset(void)
{
    nvic->APIntRst = NVIC_APINTRST_VECTKEY | NVIC_APINTRST_SYSRESETREQ;
}

/*
--------------------------------------------------------------------------------
  Function name : void NVICCpuReset(void)
  Author        : ZHengYongzhi
  Description   : CPU reset.
  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void NvicCpuReset(void)
{
    nvic->APIntRst = NVIC_APINTRST_VECTKEY | NVIC_APINTRST_VECTRESET;
}

/*
--------------------------------------------------------------------------------
  Function name : void NVICClearActive(void)
  Author        : ZHengYongzhi
  Description   : Clear all state information for active NMI, fault, and interrupts
                  It is the responsibility of the application to reinitialize the 
                  stack.
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void NvicClearActive(void)
{
    nvic->APIntRst = NVIC_APINTRST_VECTKEY | NVIC_APINTRST_VECTCLRACTIVE;
}

/*
--------------------------------------------------------------------------------
  Function name : void NVICClearActive(void)
  Author        : ZHengYongzhi
  Description   : Clear all state information for active NMI, fault, and interrupts
                  It is the responsibility of the application to reinitialize the 
                  stack.
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void NvicVectorTableSet(uint32 vectortaboffset)
{
    nvic->VectorTableOffset = vectortaboffset;
}

#endif

/*
********************************************************************************
*
*                         End of nvic.c
*
********************************************************************************
*/
