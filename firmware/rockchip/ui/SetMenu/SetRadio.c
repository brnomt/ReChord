/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  SetRe.c
*
* Description:  set menu execution function configuration of FM
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-3-3          1.0
*    desc:      the functions below will be excuted when press sure key and it has not child item,
                the itemNum be force set to CurItemInFatherID,that is the index of current item,so it can
                be used to do porper contorl in inside of function.
********************************************************************************
*/
#define  _SETRADIO_IN_

#include "SysInclude.h"

#ifdef _SYSSET_
#ifdef _RADIO_
#include "SetCommon.h"
#include "SetRadio.h"
#include "SetMenuInterface.h"

/*
--------------------------------------------------------------------------------
  Function name : UINT16 RadioSetMenuFmArea (UINT16 iSelectedItem)
  Author        : anzhiguo
  Description   :  set receive area.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#include"FmControl.h"
_ATTR_SYS_SET_RADIO_CODE_
UINT32 RadioSetMenuFmArea (UINT32 iSelectedItem)
{
    UINT32 retVal = 0;

    SetRadioArea = iSelectedItem;
    if (TRUE == ThreadCheck(pMainThread, &FMThread))
    {
        if (gFMConfig.FmArea !=  SetRadioArea)
        {
            ThreadDelete(&pMainThread, &FMThread);
            SetRadioArea = iSelectedItem;
            ThreadCreat(&pMainThread, &FMThread, NULL);
            FmStandbyFlag = FALSE;//HJ
        }
    }

    return retVal;
}
/*
--------------------------------------------------------------------------------
  Function name : UINT16 RadioSetMenuFmStereo (UINT16 iSelectedItem)
  Author        : anzhiguo
  Description   :  stereo switch

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_RADIO_CODE_
UINT32 RadioSetMenuFmStereo(UINT32 iSelectedItem)
{
    UINT32 retVal = 0;

    SetRadioStereo= iSelectedItem;

    SendMsg(MSG_RADIO_STERO_UPDATES);

    return retVal;
}
/*
--------------------------------------------------------------------------------
  Function name : UINT16 RadioSetMenuFmScanSensitivity (UINT16 iSelectedItem)
  Author        : Jeff Wu
  Description   :  set scan sensitivity.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                Jeff Wu     2013-10-1       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_RADIO_CODE_
UINT32 RadioSetMenuFmScanSensitivity (UINT32 iSelectedItem)
{
    UINT32 retVal = 0;

    SetScanSensitivity = iSelectedItem;

    return retVal;
}
/*
********************************************************************************
*
*                         End of SetRadio.c
*
********************************************************************************
*/
#endif

#endif

