/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   nFAT.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _NFAT_H_
#define _NFAT_H_

#undef  EXT
#ifdef  IN_NFAT
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

/*
--------------------------------------------------------------------------------
  
                           Struct define
  
--------------------------------------------------------------------------------
*/


/***************************************************************************
文件系统FS_CHCHE
***************************************************************************/
typedef struct _FS_CHCHE
{
	uint16	Buf[256];   //数据BUF
	uint8	Flag;		//读写标记
	uint32	Sec;        //扇区地址
}FS_CHCHE, *pFS_CHCHE;

/*
--------------------------------------------------------------------------------
  
                           Variable Define
  
--------------------------------------------------------------------------------
*/
DRAM_FAT EXT        FS_CHCHE    gFatCache;

_ATTR_FS_GET_MEM_BSS_  EXT __align(4) uint8  gbFatMemCache[64*1024];

EXT UINT32  sysFreeMemeryFlash0;    //  by hj
EXT UINT32  sysFreeMemeryFlash1;    //  by hj
EXT UINT32  sysFreeMemeryCard;      //  by hj
EXT UINT32  sysTotalMemeryFlash0;   //  by Aaron.sun
EXT UINT32  sysTotalMemeryFlash1;   //  by Aaron.sun
EXT UINT32  sysTotalMemeryCard;     //  by Aaron.sun
EXT UINT32  flash0TotalMem;         //  by hj
EXT UINT32  flash1TotalMem;         //  by hj
EXT UINT32  cardTotalMem;           //  by hj

/*
*-------------------------------------------------------------------------------
*
*                            Function Declaration
*
*-------------------------------------------------------------------------------
*/
#ifdef ENCODE
extern void FATDelClusChain(uint32 Index);
#endif

extern uint32 FATGetNextClus(uint32 Index, uint32 Count);
extern bool FATReadSector(uint32 LBA, void *pData);
extern bool FATWriteSector(uint32 LBA, void *pData);
extern uint32 FATDeleteClus(uint32 Clus);

#ifdef ENCODE
extern void FATSetNextClus(uint32 Index, uint32 Next);
extern uint32 FATAddClus(uint32 Index);
extern uint32 GetFreeMemFast(uint8 memory);

#endif

extern uint32 GetTotalMem(uint8 memory);
extern uint32 GetFreeMem(uint8 memory, uint32 * pdwTotalMem, uint32 * pdwFreeMem);
extern void UpdataFreeMemFast(int32 size);
/*
********************************************************************************
*
*                         End of nFAT.h
*
********************************************************************************
*/
#endif
