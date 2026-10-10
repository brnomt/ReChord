/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   fat.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-10-21          1.0
*    desc:    ORG.
********************************************************************************
*/

#define  IN_FAT
#include "FsInclude.h"

#define PBR_OFFSET 0x40
#ifdef ENABLE_MBR

unsigned char MBR[512] = {
    0x33, 0xC0, 0x8E, 0xD0, 0xBC, 0x00, 0x7C, 0xFB, 0x50, 0x07, 0x50, 0x1F, 0xFC, 0xBE, 0x1B, 0x7C,
    0xBF, 0x1B, 0x06, 0x50, 0x57, 0xB9, 0xE5, 0x01, 0xF3, 0xA4, 0xCB, 0xBD, 0xBE, 0x07, 0xB1, 0x04,
    0x38, 0x6E, 0x00, 0x7C, 0x09, 0x75, 0x13, 0x83, 0xC5, 0x10, 0xE2, 0xF4, 0xCD, 0x18, 0x8B, 0xF5,
    0x83, 0xC6, 0x10, 0x49, 0x74, 0x19, 0x38, 0x2C, 0x74, 0xF6, 0xA0, 0xB5, 0x07, 0xB4, 0x07, 0x8B,
    0xF0, 0xAC, 0x3C, 0x00, 0x74, 0xFC, 0xBB, 0x07, 0x00, 0xB4, 0x0E, 0xCD, 0x10, 0xEB, 0xF2, 0x88,
    0x4E, 0x10, 0xE8, 0x46, 0x00, 0x73, 0x2A, 0xFE, 0x46, 0x10, 0x80, 0x7E, 0x04, 0x0B, 0x74, 0x0B,
    0x80, 0x7E, 0x04, 0x0C, 0x74, 0x05, 0xA0, 0xB6, 0x07, 0x75, 0xD2, 0x80, 0x46, 0x02, 0x06, 0x83,
    0x46, 0x08, 0x06, 0x83, 0x56, 0x0A, 0x00, 0xE8, 0x21, 0x00, 0x73, 0x05, 0xA0, 0xB6, 0x07, 0xEB,
    0xBC, 0x81, 0x3E, 0xFE, 0x7D, 0x55, 0xAA, 0x74, 0x0B, 0x80, 0x7E, 0x10, 0x00, 0x74, 0xC8, 0xA0,
    0xB7, 0x07, 0xEB, 0xA9, 0x8B, 0xFC, 0x1E, 0x57, 0x8B, 0xF5, 0xCB, 0xBF, 0x05, 0x00, 0x8A, 0x56,
    0x00, 0xB4, 0x08, 0xCD, 0x13, 0x72, 0x23, 0x8A, 0xC1, 0x24, 0x3F, 0x98, 0x8A, 0xDE, 0x8A, 0xFC,
    0x43, 0xF7, 0xE3, 0x8B, 0xD1, 0x86, 0xD6, 0xB1, 0x06, 0xD2, 0xEE, 0x42, 0xF7, 0xE2, 0x39, 0x56,
    0x0A, 0x77, 0x23, 0x72, 0x05, 0x39, 0x46, 0x08, 0x73, 0x1C, 0xB8, 0x01, 0x02, 0xBB, 0x00, 0x7C,
    0x8B, 0x4E, 0x02, 0x8B, 0x56, 0x00, 0xCD, 0x13, 0x73, 0x51, 0x4F, 0x74, 0x4E, 0x32, 0xE4, 0x8A,
    0x56, 0x00, 0xCD, 0x13, 0xEB, 0xE4, 0x8A, 0x56, 0x00, 0x60, 0xBB, 0xAA, 0x55, 0xB4, 0x41, 0xCD,
    0x13, 0x72, 0x36, 0x81, 0xFB, 0x55, 0xAA, 0x75, 0x30, 0xF6, 0xC1, 0x01, 0x74, 0x2B, 0x61, 0x60,
    0x6A, 0x00, 0x6A, 0x00, 0xFF, 0x76, 0x0A, 0xFF, 0x76, 0x08, 0x6A, 0x00, 0x68, 0x00, 0x7C, 0x6A,
    0x01, 0x6A, 0x10, 0xB4, 0x42, 0x8B, 0xF4, 0xCD, 0x13, 0x61, 0x61, 0x73, 0x0E, 0x4F, 0x74, 0x0B,
    0x32, 0xE4, 0x8A, 0x56, 0x00, 0xCD, 0x13, 0xEB, 0xD6, 0x61, 0xF9, 0xC3, 0x49, 0x6E, 0x76, 0x61,
    0x6C, 0x69, 0x64, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x74, 0x61,
    0x62, 0x6C, 0x65, 0x00, 0x45, 0x72, 0x72, 0x6F, 0x72, 0x20, 0x6C, 0x6F, 0x61, 0x64, 0x69, 0x6E,
    0x67, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74,
    0x65, 0x6D, 0x00, 0x4D, 0x69, 0x73, 0x73, 0x69, 0x6E, 0x67, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61,
    0x74, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x44, 0x63, 0xBC, 0xC3, 0x49, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x0C, 0xFE, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0xE2, 0x0D, 0xCF, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
};

typedef __packed struct  _PART_TABLE_INFO
{
    uint8 bState;
    uint8 bStartHead;
    uint16 wBeginSec;
    uint8 bFsType;
    uint8 bEndHead;
    uint16 wEndSec;
    uint32 dwSecAddr;
    uint32 dwSecSize;

}PART_TABLE_INFO;

#endif


/***************************************************************************
Description:check whether is the power of 2
Input      :
Output     :-1--no,
Callback   :
***************************************************************************/
IRAM_FAT
int lg2(uint32 arg)
{
    uint16 log;

    for(log = 0; log < 32; log++)
    {
        if(arg & 1)
        {
            arg >>= 1;
            return (arg != 0) ? -1 : log;
        }
        arg >>= 1;
    }
    return (-1);
}

#ifdef FAT32FORMAT

/***************************************************************************
Description:advanced format.
Input      :type:FAT type
Output     :NULL
Callback   :
***************************************************************************/
IRAM_FAT
void Format(uint8 media, uint8 type)
{
    uint32 ret = OK;

    if( type != FAT32)
    {
        return;
    }

    #ifdef ENABLE_MBR
    WriteMBRSector(media, type);
    WriteDBRSector(media, type, PBR_OFFSET);           //Write DBR
    #else
    WriteDBRSector(media, type, 0);
    #endif

    ret = FileSysSetup(media);              //Write FAT

    if(ret == OK)
    {
        WriteFAT2(type);
        WriteRootDir(type);             //Write Root Dir
        FileInit();

        if(FLASH0 == media || FLASH1 == media)
        {
            MakeDir("\\", "MUSIC");
            #ifdef _RECORD_
            MakeDir("\\", "RECORD");
            MakeDir("\\RECORD", "FM");
            MakeDir("\\RECORD", "VOICE");
            #endif
            MakeDir("\\", "PICTURE");
        }
        else if(CARD == media ) //chad.ma add,user can be modified by himself.
        {
            MakeDir("\\", "MUSIC");
        }
    }
    else
    {
        DEBUG("foramt fail.......");
    }
}

#ifdef ENABLE_MBR

/***************************************************************************
Description:write 512 bytes MBR sector.
Input      :type:FAT type
Output     :NULL
Callback   :
***************************************************************************/
IRAM_FAT
void WriteMBRSector(uint8 media, uint8 type)
{
    uint8 MBRAddr;
    uint8 MBRBuf[512];
    PART_TABLE_INFO * pPartTableInfo;

    memset(MBRBuf, 0, 512);
    memcpy(MBRBuf, MBR, 512);

    pPartTableInfo = (PART_TABLE_INFO * )(MBRBuf + 0x1BE);
    pPartTableInfo->bState = 0; // 0x80, 0x00
    pPartTableInfo->bFsType = 0x0B; // FAT32
    pPartTableInfo->dwSecAddr = PBR_OFFSET;
    if(MDGetCapacity(FileDiskID) == 0)  //error happen
        return;
    pPartTableInfo->dwSecSize = (MDGetCapacity(FileDiskID) - PBR_OFFSET);
    MBRBuf[510] = 0x55;
    MBRBuf[511] = 0xAA;

    FATWriteSector(0, MBRBuf);
}
#endif


/***************************************************************************
Description:write 512 bytes DBR sector.
Input      :type:FAT type
Output     :NULL
Callback   :
***************************************************************************/
IRAM_FAT
void WriteDBRSector(uint8 media, uint8 type,  uint32 StartSec)
{
    uint32 TotLogicSec;
    uint32 DBRAddr;
    uint16 i;
    uint8  DbrBuf[512];
    uint8  FSInfoBuf[512];
    uint32 TmpVal1;
    uint32 TmpVal2;

//write DBR
{
    memset(DbrBuf,0,512);

    DbrBuf[0]  = 0xeb;
    DbrBuf[1]  = 0x58;//0x3e
    DbrBuf[2]  = 0x90;

    DbrBuf[3]  = 'M';
    DbrBuf[4]  = 'S';
    DbrBuf[5]  = 'D';
    DbrBuf[6]  = 'O';
    DbrBuf[7]  = 'S';
    DbrBuf[8]  = '5';
    DbrBuf[9]  = '.';
    DbrBuf[10] = '0';

    //2 0x0b-0x23:BPB
    DbrBuf[12] = 0x02;                                          //每扇区512B
    DbrBuf[14] = 0x20;                                          //保留扇区数 0x20
    DbrBuf[16] = 0x02;                                          //FAT份数

    DbrBuf[21] = 0xf8;                                          //硬盘

//  DbrBuf[22] = 63;                                            //每FAT占用的扇区数
    DbrBuf[24] = 63;                                            //每磁道扇区数
    DbrBuf[26] = 255;                                           //磁头数

    DbrBuf[44] = 2;                                             //BPB_RootClus
    DbrBuf[48] = 1;                                             //BPB_FSInfo ,Usually 1.
    DbrBuf[50] = 6;                                             //BPB_BkBootSec , 6.
    DbrBuf[66] = 0x29;                                          //扩展引导标签,必须是0x28或0x29

    DbrBuf[82] = 'F';                                           //文件系统类型(8B)
    DbrBuf[83] = 'A';
    DbrBuf[84] = 'T';
    DbrBuf[85] = '3';
    DbrBuf[86] = '2';
    DbrBuf[87] = ' ';
    DbrBuf[88] = ' ';
    DbrBuf[89] = ' ';

    //2 0x01fe-0x01ff:signtrue
    DbrBuf[510] = 0x55;
    DbrBuf[511] = 0xaa;

    #ifdef ENABLE_MBR
    if(GetTotalMem(media) == 0)
    {
        TotLogicSec = 0;
        return;
    }
    TotLogicSec = (GetTotalMem(media) - PBR_OFFSET);
    #else
    TotLogicSec = GetTotalMem(media);
    #endif

    if(TotLogicSec <= 66600)
    {
        DbrBuf[13] = 0;
    }
    else if(TotLogicSec <= 532480)
    {
        DbrBuf[13] = 1;
    }
    else if(TotLogicSec <= 16777216) //8G
    {
        DbrBuf[13] = 8;
    }
    else if(TotLogicSec <= 33554432) //16G
    {
        DbrBuf[13] = 16;
    }
    else if(TotLogicSec <= 67108864) //32G
    {
        DbrBuf[13] = 32;
    }
    else if(TotLogicSec <= 134217728) //64G
    {
        DbrBuf[13] = 64;
    }
    else if(TotLogicSec <= 268435456) //128G
    {
        DbrBuf[13] = 128;
    }
    else if(TotLogicSec <= 0xFFFFFFFF) //>128G
    {
        DbrBuf[13] = 128;
    }


    if (TotLogicSec > 65535)                                    //NOT 128M(256M~1G)
    {
        DbrBuf[32] = (uint8)((TotLogicSec      ) & 0x00ff);     //大扇区数
        DbrBuf[33] = (uint8)((TotLogicSec >>  8) & 0x00ff);
        DbrBuf[34] = (uint8)((TotLogicSec >> 16) & 0x00ff);
        DbrBuf[35] = (uint8)((TotLogicSec >> 24));
    }
    else
    {
        DbrBuf[19] = (uint8)(TotLogicSec & 0x00ff);             //小扇区数
        DbrBuf[20] = (uint8)((TotLogicSec >> 8) & 0x00ff);
    }

    TmpVal1 = TotLogicSec - DbrBuf[14];                         //总扇区-保留扇区-根目录扇区
    TmpVal2 = TmpVal1 / ((128 * DbrBuf[13]) + 2) + 7;           //1026
    TmpVal2 &= 0xFFFFFFF8;                                      //保证4KB对齐

    DbrBuf[36] = (uint8)((TmpVal2      ) & 0x00ff);             //FATSz
    DbrBuf[37] = (uint8)((TmpVal2 >>  8) & 0x00ff);
    DbrBuf[38] = (uint8)((TmpVal2 >> 16) & 0x00ff);
    DbrBuf[39] = (uint8)((TmpVal2 >> 24));

    FATWriteSector(StartSec + 0, DbrBuf);
    FATWriteSector(StartSec + 6, DbrBuf);
}

//WriteFSInfo_32
{
    memset(FSInfoBuf,0,512);

    FSInfoBuf[0]   = 'R';
    FSInfoBuf[1]   = 'R';
    FSInfoBuf[2]   = 'a';
    FSInfoBuf[3]   = 'A';

    FSInfoBuf[484] = 'r';
    FSInfoBuf[485] = 'r';
    FSInfoBuf[486] = 'A';
    FSInfoBuf[487] = 'a';
    FSInfoBuf[488] = 0xFF;
    FSInfoBuf[489] = 0xFF;
    FSInfoBuf[490] = 0xFF;
    FSInfoBuf[491] = 0xFF;
    FSInfoBuf[492] = 02;

    FSInfoBuf[510] = 0x55;
    FSInfoBuf[511] = 0xaa;

    FATWriteSector(StartSec + 1, FSInfoBuf);
    FATWriteSector(StartSec + 7, FSInfoBuf);

    memset(FSInfoBuf, 0, 510);

    FATWriteSector(StartSec + 2, FSInfoBuf);
    FATWriteSector(StartSec + 8, FSInfoBuf);
}

}


/***************************************************************************
Description:format by itself,used for frist.
Input      :type:FAT type
Output     :NULL
Callback   :
***************************************************************************/
IRAM_FAT
void WriteRootDir(uint8 type)
{
    uint16 j;
    uint32 RootDirAddr;
    uint32 PBA;
    uint8  DirBuf[512];

    RootDirAddr = (uint32)BootSector.BPB_ResvdSecCnt + (uint32)(BootSector.BPB_NumFATs * BootSector.FATSz) + (uint32)BootSector.PBRSector;//First

    memset(DirBuf, 0, 512);

    for(j = 0; j < BootSector.BPB_SecPerClus; j++)
    {
        FATWriteSector(RootDirAddr + j, DirBuf);
    }
}


/***************************************************************************
Description:write FAT table,use for frist using,
Input      :type:FAT type
Output     :NULL
Callback   :
***************************************************************************/
IRAM_FAT
void WriteFAT2(uint8 type)
{
    uint16 i;
    uint32 FAT1LBA;
    uint32 FAT1PBA;
    uint32 EraseSectors;
    uint8  Buf[4096];
    uint8  FATFlag = 1;

    uint32 OldPBA;
    uint16 Offset;
    uint16 len;
    uint8 flag;

    EraseSectors = BootSector.FATSz;    //总擦除连续逻辑扇区数
    FAT1LBA      = BootSector.BPB_ResvdSecCnt + BootSector.PBRSector;
    //printf("EraseSectors = %d\n", EraseSectors);

    for (i = 0; i < EraseSectors; i++)
    {
#ifdef _WATCH_DOG_
        if(i % 200 == 0)
        {
            WatchDogReload();
        }
#endif
        memset(Buf,0,512);
        if(FATFlag)
        {
            FATFlag = 0;
            Buf[0] = 0xF8;
            Buf[1] = 0xFF;
            Buf[2] = 0xFF;
            Buf[3] = 0x0F;
            Buf[4] = 0xFF;
            Buf[5] = 0xFF;
            Buf[6] = 0xFF;
            Buf[7] = 0x0F;
            Buf[8] = 0xFF;
            Buf[9] = 0xFF;
            Buf[10] = 0xFF;
            Buf[11] = 0x0F;
        }
        FATWriteSector(i+FAT1LBA, Buf);
    }
    //printf("\n EraseSectors Over\n");
}

#endif


/***************************************************************************
Description:check type of file system
Input      :NULL
Output     :OK=file system right,ERROR=file system wrong.
Callback   :
***************************************************************************/
IRAM_FAT
uint32 CheckFileSystem(void)
{
    uint32 PBRSector=0;
    uint8  DbrBuf[512];

    FATReadSector(0, DbrBuf);         //读DBR扇区
    if(ERROR == CheckFatBootSector(DbrBuf)) //not a FAT volume
    {
        #ifdef ENABLE_MBR
        if(!CheckMbr(DbrBuf))       //not a MBR
        {
            return (ERROR);
        }
        else
        {
            PBRSector = GetPartAddr(DbrBuf, 0);
            FATReadSector(PBRSector, DbrBuf);         //读DBR扇区
        }
        #else
        return (ERROR);
        #endif
    }

    FATType = GetBootInfo(PBRSector, DbrBuf);

    return (OK);
}


/***************************************************************************
Description:get parameter of DBR sector.
Input      :the sector that has DBR.
Output     :type of file system.
Callback   :
***************************************************************************/
IRAM_FAT
uint8 GetBootInfo(uint32 PBRSector, uint8 *DbrBuf)
{
    uint8 type;
    uint32 TotSec, DataSec;

    BootSector.BPB_BytsPerSec = ((uint16)DbrBuf[12] << 8) + DbrBuf[11];
    BootSector.BPB_SecPerClus = DbrBuf[13];

    LogBytePerSec = lg2(BootSector.BPB_BytsPerSec);
    LogSecPerClus = lg2(BootSector.BPB_SecPerClus);

    BootSector.BPB_NumFATs = DbrBuf[16];
    BootSector.BPB_RootEntCnt = ((uint16)DbrBuf[18] << 8) + DbrBuf[17];
    BootSector.FATSz = ((uint32)DbrBuf[23] << 8) + DbrBuf[22];

    if(BootSector.FATSz == 0)
    {
        BootSector.FATSz = ((uint32)DbrBuf[39] << 8) + DbrBuf[38];
        BootSector.FATSz <<= 16;
        BootSector.FATSz += ((uint32)DbrBuf[37] << 8) + DbrBuf[36];
    }

    TotSec = ((uint32)DbrBuf[20] << 8) + DbrBuf[19];
    if(TotSec == 0)
    {
        TotSec=((uint32)DbrBuf[35] << 8) + DbrBuf[34];
        TotSec <<= 16;
        TotSec += ((uint32)DbrBuf[33] << 8) + DbrBuf[32];
    }

    BootSector.BPB_ResvdSecCnt = ((uint16)DbrBuf[15] << 8) + DbrBuf[14];
    BootSector.RootDirSectors = ((BootSector.BPB_RootEntCnt * 32) + (BootSector.BPB_BytsPerSec - 1)) >> LogBytePerSec;
    BootSector.PBRSector = PBRSector;
    BootSector.FirstDataSector = BootSector.BPB_ResvdSecCnt + (BootSector.BPB_NumFATs * BootSector.FATSz) + BootSector.RootDirSectors + PBRSector;

    DataSec = TotSec - BootSector.FirstDataSector + PBRSector;

    while((DataSec & (BootSector.BPB_SecPerClus - 1)) != 0)   //Modify by lxs @2006.01.10 for No Standard Lib compiler
    {
        DataSec--;
    }

    BootSector.CountofClusters = DataSec >> LogSecPerClus;
    if(BootSector.CountofClusters < 4085)
    {
        type = FAT12;       // Volume is FAT12
    }
    else if(BootSector.CountofClusters < 65525)
    {
        type = FAT16;       // Volume is FAT16
    }
    else
    {
        type = FAT32;       // Volume is FAT32
    }


    if(type == FAT32)
    {
        BootSector.BPB_RootClus = ((uint32)DbrBuf[47] << 8) + DbrBuf[46];
        BootSector.BPB_RootClus <<= 16;
        BootSector.BPB_RootClus += ((uint32)DbrBuf[45] <<8 ) + DbrBuf[44];
    }
    else
        BootSector.BPB_RootClus = 0;

    BootSector.TotSec=TotSec;       //add by lxs @2005.4.19
    return (type);
}


/***************************************************************************
Description:check whether it is guide sector.(DBR)
Input      :NULL
Output     :ERROR=no,OK=yes
Callback   :
***************************************************************************/
IRAM_FAT
int32 CheckFatBootSector(uint8 *DbrBuf)
{
    uint16 temp;
    int32 bad = OK;

    if(DbrBuf[0] == 0xE9);  // OK
    else if(DbrBuf[0] == 0xEB && DbrBuf[2] == 0x90);    // OK
    else
    {
        bad = ERROR;        // Missing JMP/NOP
    }

    // check other stuff
    temp = DbrBuf[13];
    if((lg2(temp)) < 0)
    {
        bad = ERROR;        //Sectors per cluster is not a power of 2
    }

    //very few disks have only 1 FAT, but it's valid
    temp = DbrBuf[16];
    if(temp != 1 && temp != 2)
    {
        bad = ERROR;        // Invalid number of FATs
    }

    // can't check against dev.sects because dev.sects may not yet be set
    temp = (DbrBuf[25]<<8)+DbrBuf[24];
    if(temp == 0 || temp > 63)
    {
        bad = ERROR;        // Invalid number of sectors
    }

    // can't check against dev.heads because dev.heads may not yet be set
    temp = (DbrBuf[27]<<8)+DbrBuf[26];
    if(temp == 0 || temp > 255)
    {
        bad = ERROR;        // Invalid number of heads
    }

    temp = (DbrBuf[511]<<8)+DbrBuf[510];
    if(temp != 0xAA55)
    {
        bad = ERROR;        // Invalid signature of FATs
    }

    return (bad);
}

#ifdef ENABLE_MBR

/***************************************************************************
Description:check whether it is main guide record(MBR)
Input      :NULL
Output     :0--no, other: yes
Callback   :
***************************************************************************/
IRAM_FAT
uint32 GetPartAddr(uint8 *MBRBuf, uint8 PartNum)
{
    uint32 MbrLba;
    uint16 Index;

    Index = 0x1c6 + PartNum * 16;
    MbrLba = *(uint32 *)(MBRBuf + Index);
    return MbrLba;
}

IRAM_FAT
uint32 CheckMbr(uint8 *MBRBuf)
{
    int index;
    PART_TABLE_INFO * pPartTableInfo;

    if((MBRBuf[510] != 0x55) || (MBRBuf[511] != 0xAA))
    {
        printf("\n 00 CheckMbr Error\n");
        return 0;
    }


    for(index = 0;index < 4;index++)
    {
        pPartTableInfo = (PART_TABLE_INFO * )(MBRBuf + 0x1BE + index*0x10);//4个分区信息

        if(pPartTableInfo->bState == 0 && pPartTableInfo->bStartHead== 0)
        {
            continue;
        }

        if(pPartTableInfo->bFsType != 0x0B
            && pPartTableInfo->bFsType != 0x0C  //fat32 基本分区
            && pPartTableInfo->bFsType != 0x04
            && pPartTableInfo->bFsType != 0x06) //fat16 基本分区
        {
            return 0;
        }
        //printf ("dwSecSize_compare=%x\n",(MDGetCapacity(FileDiskID) - pPartTableInfo->dwSecAddr));
    }

    return 1;

}

#endif


/***************************************************************************
Description:get the postion of one cluster in FAT table.
Input      :cluster number.
Output     :sector number and sector offset
Callback   :
***************************************************************************/
IRAM_FAT
void GetFATPosition(uint32 cluster, uint32 *FATSecNum, uint16 *FATEntOffset)
{
    uint32 FATOffset;

    if(FATType == FAT16)
    {
        FATOffset = cluster << 1;
    }
    else if (FATType == FAT32)
    {
        FATOffset = cluster << 2;
    }
#ifdef K_FAT12
    else if(FATType == FAT12)
    {
        FATOffset = (cluster * 3) >> 1;
    }
#endif

    *FATSecNum    = (FATOffset >> LogBytePerSec) + BootSector.BPB_ResvdSecCnt + BootSector.PBRSector;//+BootSector.FATSz;//改为写FAT1
    *FATEntOffset = (uint16)(FATOffset & (BootSector.BPB_BytsPerSec - 1));
}

_ATTR_SYS_CODE_
uint32 GetMemDevID(uint8 memory)
{
    uint32 DevID = UserDisk0ID;

    switch (memory)
    {
        case FLASH0:
            DevID = UserDisk0ID;
            break;
        case FLASH1:
            DevID = UserDisk1ID;
            break;
        case CARD:    //现在IC不支持8bit
            DevID = UserDisk2ID;
            break;
        #ifdef _USB_HOST_
        case USBHOST_FLASH:
            DevID = UserDisk3ID;
            break;
        #endif
        default:
            return DevID;
    }

    if (0 == DevID)
    {
        DevID = UserDisk0ID;
    }

    return DevID;
}
/*
--------------------------------------------------------------------------------
  Function name : FileSysSetup
  Author        : chenfen
  Description   :

    函数描述:媒体设置
    入口参数:memory=0选择FLASH, memory=1选择SD卡
    出口参数:=OK设置成功, ERROR=错误

  Input         :
  Return        : 无

  History:     <author>         <time>         <version>
                chenfen      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
/*_ATTR_SYS_INIT_CODE_*/
_ATTR_SYS_CODE_
uint32 MemDevProb(uint8 memory)
{
    uint32 DiskID;

    DiskID = GetMemDevID(memory);
    if (!MDChkValid(DiskID))
    {
        if (MDChkPresent(DiskID))
        {
            if (OK != MDProbe(DiskID))
            {
                DEBUG("MemDevSetup MDProbe Error\n");
                return ERROR;
            }
        }
    }
    CurMemSel = memory;
    FileDiskID = DiskID;

    return OK;
}

/*
--------------------------------------------------------------------------------
  Function name : FileSysSetup
  Author        : chenfen
  Description   :

    函数描述:媒体设置
    入口参数:memory=0选择FLASH, memory=1选择SD卡
    出口参数:=OK设置成功, ERROR=错误

  Input         :
  Return        : 无

  History:     <author>         <time>         <version>
                chenfen      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
IRAM_FAT
uint32 MemDevSetup(uint8 memory)
{
    uint32 DiskID;

    DiskID = GetMemDevID(memory);
    if (!MDChkValid(DiskID))
    {
        if (MDChkPresent(DiskID))
        {
            if (OK != MDProbe(DiskID))
            {
                DEBUG("MemDevSetup MDProbe Error\n");
                return ERROR;
            }
        }
    }

    CurMemSel = memory;
    FileDiskID = DiskID;
    if (OK != CheckFileSystem())
    {
        DEBUG("MemDevSetup Check FileSystem Error\n");
        return ERROR;
    }

    return OK;
}

/*
--------------------------------------------------------------------------------
  Function name : FileSysSetup
  Author        : chenfen
  Description   :

    函数描述:媒体设置
    入口参数:memory=0选择FLASH, memory=1选择SD卡
    出口参数:=OK设置成功, ERROR=错误

  Input         :
  Return        : 无

  History:     <author>         <time>         <version>
                chenfen      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
IRAM_FAT
uint32 FileSysSetup(uint8 memory)
{
    if (ERROR == MemDevSetup(memory))
    {
        return ERROR;
    }

    FileInit();

    return OK;
}

/*
********************************************************************************
*
*                         End of fat.c
*
********************************************************************************
*/

