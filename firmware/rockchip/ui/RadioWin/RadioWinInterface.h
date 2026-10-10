/*
********************************************************************************
*                   Copyright (c) 2008,WangBo
*                         All rights reserved.
*
* File Name£º   RadioWinInterface.h
*
* Description:  FM external interface quote
*
* History:      <author>          <time>        <version>
*                WangBo      2009-3-20          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _RADIOWIN_INTERFACE_H
#define _RADIOWIN_INTERFACE_H

#undef  EXT
#ifdef _IN_RADIOWIN_INTERFACE_
#define EXT
#else
#define EXT extern
#endif


//------------------------------------------------------------------------------
#include "RadioSubFuncWin.h"
#include "RadioSubFreqWin.h"
#include "DialogBox.h"
#include "FMControl.h"

#define RadioBatteryLevel       gBattery.Batt_Level

extern void RadioWinSvcStart(void *pArg);
extern void RadioWinSvcStop(void);
extern BOOLEAN RadioDecodeProc(MSG_ID id,void *msg);

//------------------------------------------------------------------------------

//******************************************************************************
#endif


