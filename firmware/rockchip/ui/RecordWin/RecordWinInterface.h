/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                         All rights reserved.
*
* File Name��   RecordWinInterface.h
*
* Description:  the interface that record provide for extenal part
*
* History:      <author>          <time>        <version>
*                WangBo         2009-4-10          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _RECORDWIN_INTERFACE_H
#define _RECORDWIN_INTERFACE_H

#undef  EXT
#ifdef _IN_RECORDWIN_INTERFACE_
#define EXT
#else
#define EXT extern
#endif


//------------------------------------------------------------------------------

#include  "fsinclude.h"
#include  "File.h"
#include  "FDT.h"

#include  "pcm.h"

#include "RecordWin.h"

#include "recordcontrol.h"    //recording backgroud service

#include "MessageBox.h" //to popup dialog when disk full or OVERLAYOVERLAY create file fail,exit recording.  
   
//------------------------------------------------------------------------------

#define RecordBatteryLevel       gBattery.Batt_Level

extern void RecordWinSvcStart(void *pArg);
extern void RecordWinSvcStop(void);


/*
********************************************************************************
*
*                         End of RecordWinInterface.h
*
********************************************************************************
*/
#endif


