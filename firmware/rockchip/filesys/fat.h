/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   fat.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _FAT_H
#define _FAT_H

#undef  EXT
#ifdef  IN_FAT
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
/* FAT type */
#define     FAT12   0
#define     FAT16   1
#define     FAT32   2

#define     FLASH0         DISK_FLASH_USER
#define     FLASH1         DISK_FLASH_USER2
#define     USBHOST_FLASH  DISK_FLASH_USER3

#define     CARD        4
#define     CDROM       5

/*
--------------------------------------------------------------------------------
  
                           Struct define
  
--------------------------------------------------------------------------------
*/
//direction item(32×Ö½Ú)
/*
------------------------------------------------------------
    Offset      Length      Name            Contents
    0           8B          Name            File name
    8           3B          Extension       File extension
    B           1B          Attributes      file attributes
    C           10B         Reserved        RESERVED
    16          word        Time            Time stamp
    18          word        Date            Date stamp
    1A          word        StartCluster    Starting cluster number
    1C          dWord       FileSize        File size
------------------------------------------------------------
*/
typedef __packed struct
{
    uint8   fileName[8];        /*file name*/
    uint8   extName[3];         /*extension*/
    
    uint8   rdOnly:1;           /*read only property*/
    uint8   hide:1;             /*implied property*/
    uint8   sys:1;              /*system property*/
    uint8   label:1;            /*labe falg*/
    uint8   dir:1;              /*driection flag*/
    uint8   arc:1;              /*archive property*/
    uint8   res:2;              /*reserved*/
        
    uint8   reserve[5];         /*reserved*/
    uint16  modTime;            /*file modify time*/
    uint16  modDate;            /*file modify date*/
    uint16  createTime;         /*file create time*/
    uint16  createDate;         /*file create date*/
    uint16  startClu;           /*start cluster number*/
    uint32  fileSize;           /*file size*/
    
} FDT_REC;

//direction item(32 bytes)
typedef __packed struct 
{
    uint8   name[8];            /*file name*/
    uint8   ext[3];             /*extension*/
    uint8   attrib;             /*proporty*/
    uint8   reserved;           /*reserved*/
    uint8   ctime_ms;           /*file create time ms*/
    uint16  ctime;              /*file create time*/
    uint16  cdate;              /*file modify time*/
    uint16  adate;              /*access time*/
    uint16  st_clust_msw;       /*start cluster number*/
    uint16  mtime;              /*file modify time*/
    uint16  mdate;              /*file modify date*/
    uint16  st_clust;           /*start cluster number*/
    uint32  file_size;          /*file size(low 16 bit)*/
        
}_FAT_DIRENT;

//long direction item(32 byte)
typedef __packed struct 
{
    uint8   LDIR_Ord;
    uint8   LDIR_Name1[10];
    uint8   LDIR_Attr;
    uint8   LDIR_Type;
    uint8   LDIR_Chksum;
    uint8   LDIR_Name2[12];
    uint16  LDIR_FstClusLO;
    uint8   LDIR_Name3[4];
    
}_FAT_LDIRENT, *pFAT_LDIRENT;


//the disk format parameter.
typedef __packed struct 
{
    uint32  DiskSize;           /*disk size(sector)*/
    uint8   SecPerClus;         /*the sector number of one cluster*/
        
}_DSKSIZETOSECPERCLUS;

//guide sector (512 byte)
typedef __packed struct
{
    uint16  BPB_BytsPerSec;     //bytes_per_sector; // usu. =512 
    uint8   BPB_SecPerClus;     //sectors_per_cluster;
    uint16  BPB_ResvdSecCnt;    //num_boot_sectors; // usu. =1
    uint8   BPB_NumFATs;        //num_fats;     // usu. =2 
    uint16  BPB_RootEntCnt;     //num_root_dir_ents;
    //uint16  BPB_TotSec16;       //total_sectors;    // 16-bit; 0 if num sectors > 65535
    //uint8   media_ID_byte;      // usu. =0F0h
    //uint16  BPB_FATSz16;        //sectors_per_fat;
    //uint16  sectors_per_track;
    //uint16  heads;
    //uint32  hidden_sectors;     // =LBA partition start
    //uint32  BPB_TotSec32;       //total_sectors_large;// 32-bit; 0 if num sectors < 65536 
    //uint8   boot_code[474];

    //uint32  BPB_FATSz32;
    //uint8   boot_code1[4];
    uint32  BPB_RootClus;
    uint16  BPB_FSInfo;
    //uint8   boot_code2[460];
    //uint8   magic[2];           // 55h, 0AAh 

// modify and addition variable
    uint32 FATSz;
    uint32 RootDirSectors;
    uint32 CountofClusters;
    uint32 FirstDataSector;
    uint32 PBRSector;
    uint32 TotSec;              //add by lxs @2005.4.19
    
}_FAT_BOOTSECTOR;                              /* 512 bytes total */

/*
--------------------------------------------------------------------------------
  
                           Variable Define
  
--------------------------------------------------------------------------------
*/
DRAM_FAT    EXT     uint8              FATType;
DRAM_FAT    EXT     uint8              LogSecPerClus;
DRAM_FAT    EXT     uint8              LogBytePerSec;
DRAM_FAT    EXT     _FAT_BOOTSECTOR    BootSector;
DRAM_FAT    EXT     uint32             FileDiskID;
DRAM_FAT    EXT     uint8              CurMemSel;

/*
*-------------------------------------------------------------------------------
*
*                            Function Declaration
*
*-------------------------------------------------------------------------------
*/
extern int lg2(uint32 arg);

//extern void LowFormat(void);
extern void Format(uint8 media, uint8 type);

extern void WriteDBRSector(uint8 media, uint8 type, uint32 StartSec);
extern void WriteMBRSector(uint8 media, uint8 type);
extern void WriteRootDir(uint8 type);
extern void WriteFAT2(uint8 type);

extern uint32 CheckFileSystem(void);
extern uint8 GetBootInfo(uint32 PBRSector, uint8 *DbrBuf);
extern int32 CheckFatBootSector(uint8 *DbrBuf);
extern uint32 CheckMbr(uint8 *DbrBuf);
extern uint32 GetPartAddr(uint8 *MBRBuf, uint8 PartNum);
extern void GetFATPosition(uint32 cluster, uint32 *FATSecNum, uint16 *FATEntOffset);

extern  uint32  FileSysSetup(uint8 memory);
uint32 GetMemDevID(uint8 memory);

/*
********************************************************************************
*
*                         End of fat.h
*
********************************************************************************
*/
#endif
