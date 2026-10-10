/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   SysReservedOperation.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SYSRESERVED_OP_H_
#define _SYSRESERVED_OP_H_

#undef  EXT
#ifdef _IN_SYSRESERVED_OP_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/

/*reserved area data storage,bit definition logic address LBA*/
#define SYSDATA_BLK        0

#define BOOKMARK_BLK       1

#define APP_BLK            2

#define MEDIA_BLK          3

//section define
#define _ATTR_SYSRESERVED_OP_CODE_         
#define _ATTR_SYSRESERVED_OP_DATA_         
#define _ATTR_SYSRESERVED_OP_BSS_          

/*
*-------------------------------------------------------------------------------
*
*                           Struct Address
*
*-------------------------------------------------------------------------------
*/


/*
*-------------------------------------------------------------------------------
*
*                            Function Declaration
*
*-------------------------------------------------------------------------------
*/
extern void LoadSysInformation(void);
extern void SaveSysInformation(int flag);
extern void ReadReservedData(uint32 LBA , uint8 *Buf, int16 Length);
extern void WriteReservedData(uint32 LBA , uint8 *Buf, uint16 Length);

/*
********************************************************************************
*
*                         End of SysReservedOperation.h
*
********************************************************************************
*/
#endif
