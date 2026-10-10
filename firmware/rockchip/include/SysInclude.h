/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   SysInclude.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef __SYSTEM_INCLUDE_H__
#define __SYSTEM_INCLUDE_H__

/*
*-------------------------------------------------------------------------------
*
*                            include File
*
*-------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <string.h>
#include "SysConfig.h"
#include "typedef.h"
#include "macro.h"
#include "global.h"
#include "debug.h"

#ifdef RECHORD_BB_BUILD
#include "bb_compat.h"
#endif

#include "backlight.h"
#include "LcdInclude.h"
#include "DriverInclude.h"
#include "OsInclude.h"
#include "battery.h"
#include "lowpower.h"
#include "SysReservedOperation.h"

#endif 
/*
********************************************************************************
*
*                         End of SysInclude.h
*
********************************************************************************
*/

