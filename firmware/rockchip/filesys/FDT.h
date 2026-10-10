/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   FDT.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _FDT_H_
#define _FDT_H_

#undef  EXT
#ifdef  IN_FDT
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
/* FDT file property */
#define     ATTR_READ_ONLY          0x01
#define     ATTR_HIDDEN             0x02
#define     ATTR_SYSTEM             0x04
#define     ATTR_VOLUME_ID          0x08
#define     ATTR_LFN_ENTRY          0x0F      /* LFN entry attribute */
#define     ATTR_DIRECTORY          0x10
#define     ATTR_ARCHIVE            0x20
#define     ATTR_LONG_NAME          ATTR_READ_ONLY |\
									ATTR_HIDDEN |\
									ATTR_SYSTEM |\
									ATTR_VOLUME_ID
#define     ATTR_LONG_NAME_MASK     ATTR_READ_ONLY | \
									ATTR_HIDDEN |\
									ATTR_SYSTEM |\
									ATTR_VOLUME_ID |\
									ATTR_DIRECTORY |\
									ATTR_ARCHIVE

/* cluster type */
#define     EMPTY_CLUS              0
#define     EMPTY_CLUS_1            1
#define     BAD_CLUS                0x0ffffff7L
#define     EOF_CLUS_1              0x0ffffff8L
#define     EOF_CLUS_END            0x0fffffffL

/* FDT type */
#define     FILE_NOT_EXIST          0
#define     FILE_DELETED            0xe5
#define     ESC_FDT                 0x05

/*
--------------------------------------------------------------------------------

                           Struct define

--------------------------------------------------------------------------------
*/
//find structure.
#ifndef _FIND_DATA_DEFINED
#define _FIND_DATA_DEFINED
typedef __packed struct _FIND_DATA
{
    uint32  Clus;                               //current cluster
    uint32  Index;
    uint32  TotalItem;
    uint32  DirOffsetInRDB;

#ifdef _RK_CUE_
    uint32  IsCue;
    uint32  CueStartTime;
    uint32  CueEndTime;
#endif

} FIND_DATA;
#endif /* _FIND_DATA_DEFINED — duplicate SDK tag, see tools/fix_include_case.py notes */


//direction item link structure.
typedef __packed struct _FDT_DATA
{
    uint32  DirClus;                            //dir first cluster
    uint32  CurClus;                            //current cluster
    uint16  Cnt;          //cluster chain counter
} FDT_DATA;

#ifndef _FDT_DEFINED
#define _FDT_DEFINED
typedef __packed  struct _FDT
{
    uint8   Name[11];                           //short file name
    uint8   Attr;                               //file attribute
    uint8   NTRes;                              //reseverd for NT
    uint8   CrtTimeTenth;                       //create time

    uint16  CrtTime;                            //create time
//  uint16  reserve1;                           //reserved

    uint16  CrtDate;                            //create date
//  uint16  reserve2;                           //reseverd

    uint16  LstAccDate;                         //last access date
//  uint16  reserve3;                           //reseverd

    uint16  FstClusHI;                          //the hight two byte of first cluster
//  uint16  reserve4;                           //reseverd

    uint16  WrtTime;                            //last write time
//  uint16  reserve5;                           //reseverd

    uint16  WrtDate;                            //last write date
//  uint16  reserve6;                           //reseverd

    uint16  FstClusLO;                          //the low two byte of first cluster
//  uint16  reserve7;                           //reseverd

    uint32  FileSize;                           //file size
//  uint16  reserve8;                           //reseverd
//  uint16  reserve9;                           //reseverd

} FDT;
#endif /* _FDT_DEFINED */


/*
--------------------------------------------------------------------------------

                           Variable Define

--------------------------------------------------------------------------------
*/
DRAM_FAT EXT     uint8       FdtBuf[512];            //fdt cache
DRAM_FAT EXT     uint32      FdtCacheSec;           //the cluster in cahce number
DRAM_FAT EXT     FDT_DATA    FdtData;


/*
*-------------------------------------------------------------------------------
*
*                            Function Declaration
*
*-------------------------------------------------------------------------------
*/
uint8 ReadFDTInfo(FDT *Rt, uint32 SecIndex, uint16 ByteIndex);

#ifdef ENCODE
uint8 WriteFDTInfo(FDT *FDTData, uint32 SecIndex, uint16 ByteIndex);
uint8 SetRootFDTInfo(uint32 Index, FDT *FDTData);
uint8 SetFDTInfo(uint32 ClusIndex, uint32 Index, FDT *FDTData);
uint8 DelFDT(uint32 ClusIndex, uint8 *FileName);
uint8 ChangeFDT(uint32 ClusIndex, FDT *FDTData);
uint8 AddFDT(uint32 ClusIndex, FDT *FDTData, uint32 *Index);
uint8 FDTIsLie(uint32 ClusIndex, uint8 *FileName);
#endif

uint8 GetRootFDTInfo(FDT *Rt, uint32 Index);
uint8 GetFDTInfo(FDT *Rt, uint32 ClusIndex, uint32 Index);
uint8 FindFDTInfo(FDT *Rt, uint32 ClusIndex, uint8 *FileName);
uint8 FindFDTInfoLong(FDT *Rt, uint32 ClusIndex, uint32 * pIndex, uint16 *FileName);
uint8 AddFDTLong(uint32 Clus, FDT *FDTData, uint32 * pIndex, uint16 * FileName);



/*
********************************************************************************
*
*                         End of FDT.h
*
********************************************************************************
*/
#endif
