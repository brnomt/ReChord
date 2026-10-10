/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   nFat.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-10-21          1.0
*    desc:    ORG.
********************************************************************************
*/
#define  IN_NFAT
#include "FsInclude.h"


/***************************************************************************
Description:读扇区
Input      :LBA
Output     :
Callback   :
***************************************************************************/
IRAM_FAT
bool FATReadSector(uint32 LBA, void *pData)
{
    return (bool)MDRead(FileDiskID, LBA, 1, pData);
}

/***************************************************************************
Description:写扇区
Input      :LBA和数据缓冲区
Output     : null
Callback   :
***************************************************************************/
IRAM_FAT
bool FATWriteSector(uint32 LBA, void *pData)
{
    return (bool)MDWrite(FileDiskID, LBA, 1, pData);
}


/*********************************************************************************************************
** Name :
** Description  :返回FAT表指定簇的下一个簇号
** Input    :Index：簇号
** Output       :下一个簇号
** global   :FATType, BootSector
** call module  :GetFATPosition, NandFlashReadSector
********************************************************************************************************/
IRAM_FAT
uint32 FATGetNextClus(uint32 Index, uint32 Count)
{
    uint16 ByteIndex;
    uint32 SecIndex;
    uint32 Rt;

    if (Index >= BootSector.CountofClusters + 2)
    {
        return (BAD_CLUS);
    }

    Rt=Index;
    while (Count-- != 0)
    {
        /* 计算扇区号和字节索引 */
        GetFATPosition(Index, &SecIndex, &ByteIndex);
        /* 读取FAT表数据 */
        if (SecIndex != gFatCache.Sec)
        {
            if (gFatCache.Sec!=0 && gFatCache.Flag!=0)
            {
                gFatCache.Flag=0;
                FATWriteSector(gFatCache.Sec, gFatCache.Buf);
            }
            gFatCache.Sec = SecIndex;
            if (OK != FATReadSector(SecIndex, gFatCache.Buf))
            {
                return (BAD_CLUS);
            }
        }

        switch (FATType)
        {
            case FAT16:
                Rt = gFatCache.Buf[ByteIndex>>1];
                break;

            case FAT32:
                Rt = gFatCache.Buf[ByteIndex>>1] | ((uint32)gFatCache.Buf[(ByteIndex+2)>>1] << 16);
                break;
            default:
                Rt = BAD_CLUS;
                break;
       }
        Index=Rt;
    }
    if (FATType==FAT16 && Rt>=(BAD_CLUS & 0xffff))
    {
        Rt |= 0x0ffful << 16;
    }
    Rt &= 0x0fffffff;
    return (Rt);
}

#ifdef ENCODE
/*****************************************************************************************
** Name :FATDelClusChain
** Description  :删除指定簇链
** Input    :Index：簇链中首簇号
** Output       : null
** global   :FATType
** call module  :
*****************************************************************************************/
IRAM_ENCODE
void FATDelClusChain(uint32 Index)
{
    uint16 FATEntOffset;
    uint32 ThisClus;
    uint32 NextClus;
    uint32 FATSecNum;
    uint8 * pBuf;

    pBuf = (uint8 *)gFatCache.Buf;

    if (Index <= EMPTY_CLUS_1 || Index >= BAD_CLUS)
    {
        return;
    }

    ThisClus  = Index;


    do
    {
        NextClus = FATGetNextClus(ThisClus, 1);                     //从FAT1中获取下一簇

        GetFATPosition(ThisClus, &FATSecNum, &FATEntOffset);        //获取该簇在FAT表的偏移

        ThisClus = NextClus;

        if (gFatCache.Sec != FATSecNum)
        {
            if (gFatCache.Flag != 0)
            {
                FATWriteSector(gFatCache.Sec, gFatCache.Buf);
                gFatCache.Flag = 0;
            }
            gFatCache.Sec = FATSecNum;
            FATReadSector(FATSecNum, gFatCache.Buf);
        }

        switch (FATType)        //设置簇链
        {
            case FAT16:
                pBuf[FATEntOffset + 0] = EMPTY_CLUS;
                pBuf[FATEntOffset + 1] = EMPTY_CLUS;
                gFatCache.Flag = 1;
                UpdataFreeMemFast(1 << (LogSecPerClus - 1));
                break;

            case FAT32:
                pBuf[FATEntOffset + 0] = EMPTY_CLUS;
                pBuf[FATEntOffset + 1] = EMPTY_CLUS;
                pBuf[FATEntOffset + 2] = EMPTY_CLUS;
                pBuf[FATEntOffset + 3] = EMPTY_CLUS;
                gFatCache.Flag = 1;
                UpdataFreeMemFast(1 << (LogSecPerClus - 1));
                break;

            default:
                break;
        }
    }while (NextClus > EMPTY_CLUS_1 && NextClus < BAD_CLUS);

    FATWriteSector(gFatCache.Sec, gFatCache.Buf);
    gFatCache.Flag = 0;

}


/****************************************************************************************
** Name :FATSetNextClus
** Description  :设置下一个簇
** Input    :Index：簇号
**                   Next：下一个簇号
** Output       : null
** global   :FATType, BootSector
** call module  :GetFATPosition, NandFlashReadSector, NandFlashWriteSector
*****************************************************************************************/
IRAM_ENCODE
void FATSetNextClus(uint32 Index, uint32 Next)
{
    uint16 ByteIndex;
    uint32 SecIndex;

    if (Index <= EMPTY_CLUS_1 || Index >= BootSector.CountofClusters + 2)
    {
        return;
    }

    /* 计算扇区号和字节索引 */
    switch (FATType)
    {
        case FAT16:
        case FAT32:
            GetFATPosition(Index, &SecIndex, &ByteIndex);
            break;

        default:
            return;
    }

    if (SecIndex != gFatCache.Sec)
    {
        if (gFatCache.Sec!=0 && gFatCache.Flag!=0)
        {
            gFatCache.Flag=0;
            FATWriteSector(gFatCache.Sec, gFatCache.Buf);
        }
        gFatCache.Sec = SecIndex;
        FATReadSector(SecIndex, gFatCache.Buf);
    }

    switch (FATType)
    {

        case FAT16:
            gFatCache.Buf[ByteIndex>>1] = Next;
            break;
        case FAT32:
            gFatCache.Buf[ByteIndex>>1] = Next & 0xffff;
            gFatCache.Buf[(ByteIndex+2)>>1] = (Next >> 16) & 0xffff;
            break;
        default:
            break;
    }
    gFatCache.Flag=1;

}

/*******************************************************************************
** Name: FATDeleteClus
** Input:HDC dev, uint32 Clus
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.20
** Time: 10:11:57
*******************************************************************************/
IRAM_ENCODE
uint32 FATDeleteClus(uint32 Clus)
{
    uint32 PrevClus, ThisClus, MaxClus, ClusCnt;

    uint32 ret;

    if (Clus >= BAD_CLUS)
    {
        return BAD_CLUS;
    }
    else if (Clus == EMPTY_CLUS_1)
    {
        return BAD_CLUS;
    }

    MaxClus = BootSector.CountofClusters + 2;

    ClusCnt = 0;

    //查找簇链的最后一个簇
    ThisClus = Clus;
    PrevClus = Clus;

    if (ThisClus != EMPTY_CLUS)
    {
        while (1)
        {
            ret = FATGetNextClus(ThisClus, 1);
            if (ret < 0)
            {
                return BAD_CLUS;
            }
            else if (ret >= EOF_CLUS_1 || ret <= EMPTY_CLUS_1)
            {
                break;                                          //查找到空簇或结束簇
            }

            if (ret == BAD_CLUS)
            {
                return BAD_CLUS;
            }

            PrevClus = ThisClus;
            ThisClus = (uint32)ret;
            if (ClusCnt++ > MaxClus)
            {
                return BAD_CLUS;
            }
        }
    }

    //若收到为结尾簇增加一个簇
    if (PrevClus != ThisClus)
    {
        FATSetNextClus(PrevClus, EOF_CLUS_END);
    }

    FATSetNextClus(ThisClus, EMPTY_CLUS);
    UpdataFreeMemFast(1 << (LogSecPerClus - 1));

    return (PrevClus);
}


/****************************************************************************************
** Name :FATAddClus
** Description  :为指定簇链增加一个簇
** Input    :Index：簇链中任意一个簇号,如果为0,则为一个空链增加一个簇
** Output       :增加的簇号
** global   :BootSector
** call module  :
*****************************************************************************************/
IRAM_ENCODE
uint32 FATAddClus(uint32 Index)
{
    uint32 NextClus,ThisClus,MaxClus;

    if (Index >= BAD_CLUS)
    {
        return (BAD_CLUS);
    }

    MaxClus = BootSector.CountofClusters+2;

    //查找簇链的最后一个簇
    ThisClus = Index;
    if (ThisClus > EMPTY_CLUS_1)
    {
        while (1)
        {
            NextClus = FATGetNextClus(ThisClus, 1);
            if (NextClus >= EOF_CLUS_1 || NextClus <= EMPTY_CLUS_1)
            {
                break;                                          //查找到空簇或结束簇
            }

            if (NextClus == BAD_CLUS)
            {
                return (BAD_CLUS);
            }

            ThisClus = NextClus;
        }
    }
    else
    {
        ThisClus = EMPTY_CLUS_1;
    }

    //从簇链结尾处开始收索一个空簇
    for (NextClus = ThisClus + 1; NextClus < MaxClus; NextClus++)
    {
        if (FATGetNextClus(NextClus, 1) == EMPTY_CLUS)
        {
            break;
        }
    }

    //收到尾还没收到就再从头收起
    if (NextClus >= MaxClus)
    {
        for (NextClus = EMPTY_CLUS_1 + 1; NextClus < ThisClus; NextClus++)
        {
            if (FATGetNextClus(NextClus, 1) == EMPTY_CLUS)
            {
                break;
            }
        }
    }

    //若收到为结尾簇增加一个簇
    if (FATGetNextClus(NextClus, 1) == EMPTY_CLUS)
    {
        if (ThisClus > EMPTY_CLUS_1)
        {
            FATSetNextClus(ThisClus, NextClus);
        }

        FATSetNextClus(NextClus, EOF_CLUS_END);
        UpdataFreeMemFast(-(1 << (LogSecPerClus - 1)));

        return (NextClus);
    }
    else
    {
        return (EOF_CLUS_END);
    }
}
#endif


/*********************************************************************************************************
** Name :GetTotalMem
** Description  :返回剩余空间
** 入口参数 :memory=0选择FLASH, memory=1选择SD卡
** Output       :总K字节数
** global   :
** call module  :
********************************************************************************************************/
IRAM_FAT
uint32 GetTotalMem(uint8 memory)
{
    uint32 MemDevID = GetMemDevID(memory);

    return MDGetCapacity(MemDevID);
}

/*********************************************************************************************************
** Name :GetFreeMem
** Description  :返回剩余空间
** Input    : null
** Output       :总空闲K字节数
** global   :BootSector
** call module  :
********************************************************************************************************/
_ATTR_FS_GET_MEM_CODE_
uint32 GetFreeMem(uint8 memory, uint32 * pdwTotalMem, uint32 * pdwFreeMem)
{
    uint32 TotalFreeMem;
    uint32 TotalMem;
    uint32 UseMem;

    uint32 TotalClus;
    uint32 MemBak;

    uint32 dwSecAddr;
    uint32 dwTotalBlock, dwRemainClus;
    uint32 Block, Clus;
    uint16 *pFat16;
    uint32 *pFat32;
    uint8 i;

    #define BUF_SECTOR 128
    #define DMA_MAX_SEC 32

#if (MAX_LUN > 1)
    MemBak = CurMemSel;
    if (memory != CurMemSel)
    {
        MemDevSetup(memory);
    }
#endif

    TotalFreeMem = 0;
    TotalMem = 0;
    UseMem = 0;

    dwSecAddr = BootSector.BPB_ResvdSecCnt + BootSector.PBRSector;
    //printf("dwSecAddr = %d\n", dwSecAddr);

    TotalClus = BootSector.CountofClusters + 2;

    if(FATType == FAT16)
    {
        dwTotalBlock = (TotalClus * 2) / (512 * BUF_SECTOR);
        dwRemainClus = ((TotalClus * 2) % (512 * BUF_SECTOR)) / 2;

        if(dwTotalBlock != 0)
        {
            for(Block = 0; Block < dwTotalBlock; Block++)
            {
                pFat16 = (uint16 *) gbFatMemCache;

                for(i = 0; i < 4; i++)
                    MDRead(FileDiskID, dwSecAddr + i * DMA_MAX_SEC, DMA_MAX_SEC, (void *)(gbFatMemCache + i * DMA_MAX_SEC * 512));


                dwSecAddr += BUF_SECTOR;

                for (Clus = 0; Clus < ((512 * BUF_SECTOR) / 2); Clus++)
                {

                    if (*pFat16 == EMPTY_CLUS)
                    {
                        TotalFreeMem++;  //总空簇
                    }
                    else if (*pFat16 == BAD_CLUS)
                    {
                        TotalFreeMem = 0;
                        TotalMem = 0;
                        goto out;
                    }
                    pFat16++;
                }
            }

        }

        if(dwRemainClus != 0)
        {
            pFat16 = (uint16 *) gbFatMemCache;

            for(i = 0; i < 4; i++)
               MDRead(FileDiskID, dwSecAddr + i * DMA_MAX_SEC, DMA_MAX_SEC, (void *)(gbFatMemCache + i * DMA_MAX_SEC * 512));

            for (Clus = 0; Clus < dwRemainClus; Clus++)
            {

                if (*pFat16 == EMPTY_CLUS)
                {
                    TotalFreeMem++;  //总空簇
                }
                else if (*pFat16 == BAD_CLUS)
                {
                    TotalFreeMem = 0;
                    TotalMem = 0;
                    goto out;
                }
                pFat16++;
            }

        }
    }
    else if(FATType == FAT32)
    {
        //printf("TotalClus = %d\n", TotalClus);
        dwTotalBlock = (TotalClus * 4) / (512 * BUF_SECTOR);
        dwRemainClus = ((TotalClus * 4) % (512 * BUF_SECTOR)) / 4;
        //printf("totalblock = %d, dwRemainClus = %d\n", dwTotalBlock, dwRemainClus);

        if(dwTotalBlock != 0)
        {
            for(Block = 0; Block < dwTotalBlock; Block++)
            {
                pFat32 = (uint32 *) gbFatMemCache;

                for(i = 0; i < 4; i++)
                    MDRead(FileDiskID, dwSecAddr + i * DMA_MAX_SEC, DMA_MAX_SEC, (void *)(gbFatMemCache + i * DMA_MAX_SEC * 512));

                dwSecAddr += BUF_SECTOR;

                for (Clus = 0; Clus < ((512 * BUF_SECTOR) / 4); Clus++)
                {

                    if (*pFat32 == EMPTY_CLUS)
                    {
                        TotalFreeMem++;  //总空簇
                    }
                    else if (*pFat32 == BAD_CLUS)
                    {
                        TotalFreeMem = 0;
                        TotalMem = 0;
                        goto out;
                    }
                    else
                    {
                       //printf("index = %d\n",  Block * (512 * BUF_SECTOR) /4 + Clus);
                       UseMem++;
                    }
                    pFat32++;
                }

            }

        }

        if(dwRemainClus != 0)
        {
            pFat32 = (uint32 *) gbFatMemCache;

            for(i = 0; i < 4; i++)
               MDRead(FileDiskID, dwSecAddr + i * DMA_MAX_SEC, DMA_MAX_SEC, (void *)(gbFatMemCache + i * DMA_MAX_SEC * 512));

            for (Clus = 0; Clus < dwRemainClus; Clus++)
            {

                if (*pFat32 == EMPTY_CLUS)
                {
                    TotalFreeMem++;  //总空簇
                }
                else if (*pFat32 == BAD_CLUS)
                {
                    TotalFreeMem = 0;
                    TotalMem = 0;
                    goto out;
                }
                else
                {
                    //printf("index = %d\n",  Block * (512 * BUF_SECTOR) /4 + Clus);
                    UseMem++;
                }
                pFat32++;
            }

        }
    }
    TotalMem = TotalClus - 2;

out:
    //printf("useclus = %d\n", UseMem);
    //printf("totalClus = %d\n", TotalMem);
    //printf("TotalFree = %d\n", TotalFreeMem);

    TotalFreeMem <<= (LogSecPerClus - 1);        //转换为Sector数
    TotalMem <<= (LogSecPerClus - 1);

    *pdwFreeMem = TotalFreeMem;
    *pdwTotalMem = TotalMem;
    //printf("TotalFreeMem = %d\n", TotalFreeMem);
    //printf("TotalMem = %d\n", TotalMem);

#if (MAX_LUN > 1)
    if (memory != MemBak)
    {
        MemDevSetup(MemBak);
    }
#endif

    #undef BUF_SECTOR
    #undef DMA_MAX_SEC

    return;
}


/*********************************************************************************************************
** Name :GetFreeMemFast
** Description  :返回剩余空间（快速）
** Input    : null
** Output       :总空闲K字节数
** global   :BootSector
** call module  :
********************************************************************************************************/
IRAM_FAT
void UpdataFreeMemFast(int32 size)
{
    if(CurMemSel == CARD)
    {
        sysFreeMemeryCard = sysFreeMemeryCard + size;
    }
    else if(CurMemSel == FLASH0)
    {
         sysFreeMemeryFlash0 = sysFreeMemeryFlash0 + size;
    }
    else if(CurMemSel == FLASH1)
    {
         sysFreeMemeryFlash1 = sysFreeMemeryFlash1 + size;
    }

}

/*
********************************************************************************
*
*                         End of nFat.c
*
********************************************************************************
*/
