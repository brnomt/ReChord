/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   Config.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                               2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _CONFIG_H
#define _CONFIG_H

/*
*-------------------------------------------------------------------------------
*  
*                           Include File
*  
*-------------------------------------------------------------------------------
*/
#include "typedef.h"     /* base integer types first */
#include "SysConfig.h"   /* FW_IN_DEV/_EMMC_: device config macros */
#include "MemDev.h"        /* DataDiskID/UserDisk0ID: device selection */
#include "AddrSaveMacro.h" /* MUSIC_TREE_ and RECORD_TREE_ sector starts */
#include "sortfileinfo/SortInfoGetMacro.h"
#include <stdio.h>
#include <string.h>
#include "SysConfig.h"
#include "typedef.h"
#include "macro.h"

#include "hw_memap.h"
#include "Hook.h"

#include "FsConfig.h"
#include "Decode.h"

#include "USBConfig.h"
#include "MDConfig.h"


#include "filesys/FDT.h"
#include "filesys/nFAT.h"
#include "filesys/FileSeek.h"
#include "filesys/fat.h"
#include "filesys/dir.h"
#include "filesys/file.h"
#include "LongFileName.h"

#include "driverlib_def.h"
#include "delay.h"
#include "sortfileinfo/AddrSaveMacro.h"

/*
********************************************************************************
*
*                         End of Config.h
*
********************************************************************************
*/
#endif

/* Integration fix (2026-10-09): RecordControl.h needs FS/OS types
 * (FDT, FIND_DATA, MAX_*, MSG_ID, THREAD), so it belongs AFTER the vendor
 * headers above. It supplies BYTE_NUM_SAVE_PER_FILE and the RECORD_SAVE_*
 * sector constants used by filesys/file.c. */
#include "Msg.h"
#include "Thread.h"
#include "audio/RecordControl/RecordControl.h"
#include "bb_compat.h"
