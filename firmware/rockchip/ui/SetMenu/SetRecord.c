/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  SetRecord.c
* 
* Description:  set menu execution function configuration of recording
*
* History:      <author>          <time>        <version>       
*               anzhiguo      2009-3-3          1.0
*    desc:      the functions below will be excuted when press sure key and it has not child item,
                the itemNum be force set to CurItemInFatherID,that is the index of current item,so it can 
                be used to do porper contorl in inside of function.
********************************************************************************
*/
#define  _SETRECORD_IN_

#include "SysInclude.h"

#ifdef _SYSSET_
#ifdef _RECORD_
#include "SetCommon.h"
#include "SetRecord.h"
#include "SetMenuInterface.h"

/*
--------------------------------------------------------------------------------
  Function name : UINT16 RecordSetMenuQuality (UINT16 iSelectedItem) 
  Author        : anzhiguo
  Description   :  ¼����������.
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_RECORD_CODE_//_ATTR_SYS_SET_SERVICE_CODE_
UINT32 RecordSetMenuQuality (UINT32 iSelectedItem)
{
    UINT32 retVal = 0;
     
    SetRecordQuality = iSelectedItem;

    return retVal;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 RecordSetMenuFormat (UINT16 iSelectedItem) 
  Author        : 
  Description   : record file format.
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_RECORD_CODE_
UINT32 RecordSetMenuFormat (UINT32 iSelectedItem)
{
    UINT32 retVal = 0;
     
    SetRecordFormat = iSelectedItem;

    return retVal;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 RecordSetMenuVolume (UINT16 iSelectedItem) 
  Author        : anzhiguo
  Description   :  
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_RECORD_CODE_//_ATTR_SYS_SET_SERVICE_CODE_
UINT32 RecordSetMenuVolume (UINT32 iSelectedItem)
{
    UINT32 retVal = 0;
    
    SetRecordVol = iSelectedItem;

    return retVal;
}
/*
********************************************************************************
*
*                         End of SetRecord.c
*
********************************************************************************
*/
#endif

#endif

