/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  SysSetConfig.c
* 
* Description:  set menu execution function configuration
*
* History:      <author>          <time>        <version>       
*               anzhiguo      2009-3-3          1.0
*    desc:      the functions below will be excuted when press sure key and it has not child item,
                the itemNum be force set to CurItemInFatherID,that is the index of current item,so it can 
                be used to do porper contorl in inside of function.
********************************************************************************
*/
#define _SETTEXT_IN_

#include "SysInclude.h"

#ifdef _SYSSET_

#include "SetCommon.h"
#include "SetText.h"
#include "SetMenuInterface.h"

#ifdef _EBOOK_  
_ATTR_SYS_SET_TEXT_CODE_
UINT32 TextSetAutoBrowseTime (UINT32 iSelectedItem)
{
    UINT32 retVal = 0;
    
    SetTextAutoBrowseTime = iSelectedItem;
    
    return retVal;
}

_ATTR_SYS_SET_TEXT_CODE_
UINT32 TextSetAutoBrowseStatus (UINT32 iSelectedItem)
{
     UINT32 retVal = 0;
     
    SetTextAutoBrowse = iSelectedItem;
    
    return retVal;
}
#endif

#ifdef _PICTURE_  
_ATTR_SYS_SET_TEXT_CODE_
UINT32 PicSetAutoBrowseTime (UINT32 iSelectedItem)
{
    UINT32 retVal = 0;
    
    SetPicBroTimer= iSelectedItem;
    
    return retVal;
}
/*
--------------------------------------------------------------------------------
  Function name : UINT16 TextSetAutoBrowseTime (UINT16 iSelectedItem) 
  Author        : anzhiguo
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_TEXT_CODE_
UINT32 PicSetAutoBrowseStatus (UINT32 iSelectedItem)
{
     UINT32 retVal = 0;
     
    SetPicBroSwitch = iSelectedItem;
    
    return retVal;
}
#endif
#endif

