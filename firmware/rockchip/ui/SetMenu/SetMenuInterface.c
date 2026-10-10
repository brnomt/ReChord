/*
********************************************************************************
*                   Copyright (c) 2009,Anzhiguo
*                         All rights reserved.
*
* File Name��  SetMenuInterface.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               Anzhiguo        2009-3-3          1.0
*    desc:    ORG.
********************************************************************************
*/

#define  _SETMENUINTERFACE_IN_

#include "SysInclude.h"

#ifdef _SYSSET_

#include "SetCommon.h"
#include "SetMenuInterface.h"
#ifdef _RADIO_
#include "FMControl.h"
#endif
/*
--------------------------------------------------------------------------------
  Function name : UINT16 ExcuteSetMenuFunc( UINT16 itemNum )
  Author        : anzhiguo
  Description   : execute the setting menu function it is in the last item,that is it be called when there is not
                  have child item.
  Input         : menu text id
                  the item index of setting item,base 0,part ITEM number.
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_COMMON_CODE_
void ResetFMPesetData(void)
{
    //HJ
    UINT32 i;
#ifdef _RADIO_
    if (TRUE == ThreadCheck(pMainThread, &FMThread))
    {
        ThreadDelete(&pMainThread, &FMThread);
    }

    gSysConfig.RadioConfig.FmSaveNum = 0;

    for (i = 0; i < 40; i++)
    {
        gSysConfig.RadioConfig.FmFreqArray[i] = 0;
    }
#endif
}
_ATTR_SYS_SET_COMMON_CODE_
void SetFirmwareUpgrade(void)//here it is just a interface.
{
    // LoadModule(OTHER_CODE_ID);


}

_ATTR_SYS_SET_COMMON_CODE_
void SetLightOffTimerReset(void)
{
}

void SetLightOffTimerResetClone(void)
{
}

_ATTR_SYS_SET_COMMON_CODE_
void SetLcdBackLightOn(void)
{
}


void SetLcdBackLightOnClone(void)
{
}

_ATTR_SYS_SET_COMMON_CODE_
void SetPowerOffTimerReset(void)
{
}

void SetPowerOffTimerResetClone(void)
{
}

_ATTR_SYS_SET_COMMON_CODE_
uint32 SetGetFirmwareVersion(uint16 *Year,
                             UINT16 *Date,
                             UINT16 *MasterVer,
                             UINT16 *SlaveVer,
                             UINT16 *SmallVer)
{

    GetFirmwareVersion(Year, Date, MasterVer,SlaveVer,SmallVer);

}

/*
********************************************************************************
*
*                         End of SetMenuInterface.c
*
********************************************************************************
*/

#endif

