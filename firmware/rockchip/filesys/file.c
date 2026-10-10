/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   File.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-10-21          1.0
*    desc:    ORG.
********************************************************************************
*/
#define  IN_FILE
#include "FsInclude.h"

#ifdef _RK_CUE_
#include "cue.h"
#endif

/*********************************************************************************************************
** Name :FileInit
** Description  :file system initial
** Input    : null
** Output       : null
** global   : null
** call module  : null
********************************************************************************************************/
IRAM_FAT
void FileInit(void)
{
    HANDLE i;

    gFatCache.Sec=0;        //FAT扇区号
    gFatCache.Flag=0;
    FdtCacheSec = -1;       //FDT扇区号
//  FTLInit();              //变量初始化

    for (i = 0; i < MAX_OPEN_FILES; i++)
    {
        OpenFileInfo[i].Flags = 0;
    }

    FdtData.DirClus = -1;
}

#ifdef ENCODE
/*********************************************************************************************************
** Name :FileCreate
** Description  :create file
** Input    :Path, DirFileName:user use filename.
** Output       :RETURN_OK：成功
** other reference: the return value explaination in file fat.h
** global   :OpenFileInfo
** call module  :AddFDT, GetDirClusIndex
********************************************************************************************************/
IRAM_ENCODE
HANDLE FileCreateSub(uint8 *Path, uint8 *DirFileName, uint8 Attr) // Attr 文件属性，一般为ATTR_ARCHIVE
{
    MY_FILE *fp;
    HANDLE   Rt, OsRt;
    FDT     temp;
    uint8   i;
    uint32 index;

    OsRt = NOT_OPEN_FILE;

    /* 查找空闲文件登记项 */
    for (Rt = 0; Rt < MAX_OPEN_FILES; Rt++)
    {
        if (OpenFileInfo[Rt].Flags == 0)
        {
            break;
        }
    }

    if (Rt < MAX_OPEN_FILES)
    {
        fp = OpenFileInfo + Rt;     //指向文件句柄
        for (i = 0; i < 11; i++)
        {
            temp.Name[i] = DirFileName[i];
        }

        temp.Attr         = Attr;//ATTR_ARCHIVE;        //存档
        temp.FileSize     = 0;
        temp.NTRes        = 0;
        temp.CrtTimeTenth = 0;
        temp.CrtTime      = 0;
        temp.CrtDate      = (1) | (1 << 5) |((2016 - 1980) << 9);
        temp.LstAccDate   = (1) | (1 << 5) |((2016 - 1980) << 9);
        temp.WrtTime      = 0;
        temp.WrtDate      = (1) | (1 << 5) |((2016 - 1980) << 9);
        temp.FstClusLO    = 0;
        temp.FstClusHI    = 0;

        fp->DirClus = GetDirClusIndex(Path);

        if (fp->DirClus < BAD_CLUS)
        {
//          FTLInit();
            if (AddFDT(fp->DirClus, &temp, &index) == RETURN_OK)        //增加文件
            {
                /* 设置文件信息 */
                for (i = 0; i < 11; i++)
                {
                    fp->Name[i] = temp.Name[i];
                }

                fp->Flags    = FILE_FLAGS_READ | FILE_FLAGS_WRITE;
                fp->FileSize = 0;
                fp->FstClus  = 0;
                fp->Clus     = 0;
                fp->Offset   = 0;
                fp->Index = index;

                OsRt = Rt;
            }
        }
    }

    return (OsRt);
}



/*******************************************************************************
** Name: FATGetShortFromLongName
** Input:uint8 * ShortName, uint16 * LongName
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.19
** Time: 10:16:35
*******************************************************************************/
IRAM_ENCODE
uint8 FATGetShortFromLongName(uint8 * ShortName, uint16 * LongName)
{
    uint32 ShortCnt, i;
    uint8 Name[11];
    uint32 NameNum;
    uint16 * pLong;

    Name[10] = 0;
    ShortCnt = Unicode2Ascii(Name, LongName, 4);
    if ((ShortCnt <= 0) || (ShortCnt > 5))
    {
        return RETURN_FAIL;
    }

    memset(ShortName, 0x20, 11);

    //ShortCnt = StrLenA(ShortName);

    NameNum = SysTickCounter % 1000;

    for (i = 0; i < 8; i++)
    {
        if (i < ShortCnt)
        {
            ShortName[i] = Name[i];
        }
        else if (i < 5)
        {
            ShortName[i] = '~';
        }
        else if (i == 5)
        {
            ShortName[i] = NameNum / 100 + 0x30;
            NameNum = NameNum % 100;
        }
        else if (i == 6)
        {
            ShortName[i] = NameNum / 10 + 0x30;
            NameNum = NameNum % 10;
        }
        else if (i == 7)
        {
            ShortName[i] = NameNum  + 0x30;
        }
    }

    i = StrLenW(LongName);

    pLong = LongName + i;

    while ((*pLong != '.') && i != 0)
    {
        pLong--;
        i--;
    }

    if (i == 0)
    {
        return RETURN_OK;
    }

    i++;

    ShortCnt = Unicode2Ascii(Name, LongName + i, 3);
    if ((ShortCnt < 0) || (ShortCnt > 3))
    {
        return RETURN_FAIL;
    }

    for (i = 0; i < ShortCnt; i++)
    {
        ShortName[i + 8] = Name[i];
    }


    for (i = 0; i < 11; i++)
    {
        if ((ShortName[i] <= 'z') && (ShortName[i] >= 'a'))
            ShortName[i] = ('A' + (ShortName[i] - 'a'));
    }

    return RETURN_OK;

}


IRAM_ENCODE
HANDLE FileCreate(uint8 *Path, uint8 *DirFileName)
{
    return (FileCreateSub(Path, DirFileName, ATTR_ARCHIVE)); // Attr 文件属性，一般为ATTR_ARCHIVE
}

IRAM_ENCODE
HANDLE FileCreate_Hidden(uint8 *Path, uint8 *DirFileName)
{
    return (FileCreateSub(Path, DirFileName, ATTR_ARCHIVE | ATTR_HIDDEN)); // Attr 文件属性，一般为ATTR_ARCHIVE
}

IRAM_ENCODE
uint8 FileCreateFileW(uint16 *Path, uint16 *DirFileName)
{
    uint32 Clus, Index;
    FDT temp;
    uint32 ret;

    Clus = GetDirClusIndexLong(Path, StrLenW(Path));
    if (Clus < 0)
    {
        goto ERROR1;
    }

    if (FSIsLongName(DirFileName, StrLenW(DirFileName)) == 1)
    {
        if (FATGetShortFromLongName(temp.Name, DirFileName) == RETURN_OK)
        {
            //temp.Attr = 0;
            temp.Attr         = ATTR_ARCHIVE;
            temp.FileSize     = 0;
            temp.NTRes        = 0;
            temp.CrtTimeTenth = 0;
            temp.CrtTime      = 0;
            temp.CrtDate      = (1) | (1 << 5) | ((2016 - 1980) << 9);
            temp.LstAccDate   = (1) | (1 << 5) | ((2016 - 1980) << 9);
            temp.WrtTime      = 0;
            temp.WrtDate      = (1) | (1 << 5) | ((2016 - 1980) << 9);
            temp.FstClusLO    = 0;
            temp.FstClusHI    = 0;
            ret = AddFDTLong(Clus, &temp, &Index, DirFileName);
            if (ret != RETURN_OK)
            {
                goto ERROR1;
            }

        }
        else
        {
            goto ERROR1;
        }

    }
    else
    {
        FSRealname(temp.Name, DirFileName);
        //temp.Attr = 0;
        temp.Attr         = ATTR_ARCHIVE;
        temp.FileSize     = 0;
        temp.NTRes        = 0;
        temp.CrtTimeTenth = 0;
        temp.CrtTime      = 0;
        temp.CrtDate      = (1) | (1 << 5) | ((2016 - 1980) << 9);
        temp.LstAccDate   = (1) | (1 << 5) | ((2016 - 1980) << 9);
        temp.WrtTime      = 0;
        temp.WrtDate      = (1) | (1 << 5) | ((2016 - 1980) << 9);
        temp.FstClusLO    = 0;
        temp.FstClusHI    = 0;

        ret = AddFDT(Clus, &temp, &Index);

        if (ret != RETURN_OK)
        {
            goto ERROR1;
        }

    }

    return RETURN_OK;

ERROR1:

    return RETURN_FAIL;

}


IRAM_ENCODE
uint8 FileCreateDirW(uint16 *Path, uint16 *DirFileName)
{
    uint32 Clus, Index, SubClus, i;
    FDT temp;
    uint32 ret;

    Clus = GetDirClusIndexLong(Path, StrLenW(Path));
    if (Clus < 0)
    {
        goto ERROR1;    //never run here ,because clus is unsigend
    }

    SubClus = 0;

    ret = FATAddClus(0);

    if (ret < EMPTY_CLUS)
    {
        goto ERROR1;
    }

    SubClus = (uint32)ret;

    if (ClearClus(SubClus) != RETURN_OK)
    {
        goto ERROR1;
    }

    if (FSIsLongName(DirFileName, StrLenW(DirFileName)) == 1)
    {
        if (FATGetShortFromLongName(temp.Name, DirFileName) == RETURN_OK)
        {
            //printf("subclus = %d\n", SubClus);
            {
                uint8 i;
                for (i = 0; i < 11; i++)
                {
                    //printf("\n%c", temp.Name[i]);
                }
            }

            temp.Attr = ATTR_DIRECTORY;
            temp.FileSize     = 0;
            temp.NTRes        = 0;
            temp.CrtTimeTenth = 0;
            temp.CrtTime      = 0;
            temp.CrtDate      = (1) | (1 << 5) | ((2016 - 1980) << 9);
            temp.LstAccDate   = (1) | (1 << 5) | ((2016 - 1980) << 9);
            temp.WrtTime      = 0;
            temp.WrtDate      = (1) | (1 << 5) | ((2016 - 1980) << 9);
            temp.FstClusLO    = SubClus & 0x0000FFFF;
            temp.FstClusHI    = (SubClus >> 16) & 0x0000FFFF;

            ret = AddFDTLong(Clus, &temp, &Index, DirFileName);
            if (ret != RETURN_OK)
            {
                if (ret == DISK_FULL)
                {
                    FATDeleteClus(SubClus);
                }

                goto ERROR1;
            }

            /* 建立'.'目录 */
            temp.Name[0] = '.';
            for (i = 1; i < 11; i++)
            {
                temp.Name[i] = ' ';
            }
            AddFDT(SubClus, &temp, &Index);

            /* 建立'..'目录 */
            temp.Name[1] = '.';
            if (Clus == BootSector.BPB_RootClus)
            {
                Clus = 0;
            }

            temp.FstClusLO = Clus & 0xffff;
            temp.FstClusHI = Clus / 0x10000;
            AddFDT(SubClus, &temp, &Index);

        }
        else
        {
            FATDeleteClus(SubClus);
            goto ERROR1;
        }

    }
    else
    {
        temp.Attr = ATTR_DIRECTORY;
        temp.FileSize     = 0;
        temp.NTRes        = FSRealname(temp.Name, DirFileName);
        temp.CrtTimeTenth = 0;
        temp.CrtTime      = 0;
        temp.CrtDate      = (1) | (1 << 5) | ((2016 - 1980) << 9);
        temp.LstAccDate   = (1) | (1 << 5) | ((2016 - 1980) << 9);
        temp.WrtTime      = 0;
        temp.WrtDate      = (1) | (1 << 5) | ((2016 - 1980) << 9);
        temp.FstClusLO    = SubClus & 0x0000FFFF;
        temp.FstClusHI    = (SubClus >> 16) & 0x0000FFFF;

        ret = AddFDT(Clus, &temp, &Index);

        if (ret != RETURN_OK)
        {
            if (ret == DISK_FULL)
            {
                FATDeleteClus(SubClus);
            }

            goto ERROR1;
        }

        /* 建立'.'目录 */
        temp.Name[0] = '.';
        for (i = 1; i < 11; i++)
        {
            temp.Name[i] = ' ';
        }
        AddFDT(SubClus, &temp, &Index);

        /* 建立'..'目录 */
        temp.Name[1] = '.';
        if (Clus == BootSector.BPB_RootClus)
        {
            Clus = 0;
        }

        temp.FstClusLO = Clus & 0xffff;
        temp.FstClusHI = Clus / 0x10000;
        AddFDT(SubClus, &temp, &Index);
    }

    //回写FAT Cache
    if (gFatCache.Sec!=0 && gFatCache.Flag!=0)
    {
        gFatCache.Flag=0;
        FATWriteSector(gFatCache.Sec, gFatCache.Buf);
    }

    return RETURN_OK;
ERROR1:
    return RETURN_FAIL;

}

/*********************************************************************************************************
** Name :FileDelete
** Description  :delete file
** Input    :Path, DirFileName:user use filename.
** Output       :RETURN_OK：成功
** other reference: the return value explaination in file fat.h
** global   :OpenFileInfo
** call module  :FindFDTInfo,FATDelClusChain,DelFDT
********************************************************************************************************/
IRAM_ENCODE
uint8 FileDelete(uint8 *Path, uint8 *DirFileName)
{
    uint32 ClusIndex1;
    uint32 DirClus;
    uint8  Rt;
    FDT    temp;

    DirClus = GetDirClusIndex(Path);    //获取路径所在的簇号

    Rt = PATH_NOT_FIND;
    if (DirClus != BAD_CLUS)            //确定路径存在
    {
        Rt = NOT_FIND_FILE;
        if (RETURN_OK == FindFDTInfo(&temp, DirClus, DirFileName))          //找到目录项
        {
            if ((temp.Attr & ATTR_DIRECTORY) == 0)                          // 是文件才删除
            {
                Rt = FILE_LOCK;
                if (FindOpenFile(DirClus, DirFileName) >= MAX_OPEN_FILES)   //文件没有打开才删除
                {

                    ClusIndex1 = temp.FstClusLO + ((uint32)(temp.FstClusHI) << 16);

                    FATDelClusChain(ClusIndex1);

                    Rt = DelFDT(DirClus, DirFileName);


                }
            }
            else
            {
                //Rt = DirTreeDelete(DirClus, &temp);
            }
        }
    }

    return (Rt);
}

/*********************************************************************************************************
** Name :FileDelete1
** Description  :delete file
** Input    :Path, DirFileName:user use filename.
** Output       :RETURN_OK：成功
** other reference: the return value explaination in file fat.h
** global   :OpenFileInfo
** call module  :FindFDTInfo,FATDelClusChain,DelFDT
********************************************************************************************************/
IRAM_ENCODE
uint8 FileDelete1(uint32 DirClus, uint32 Index, FS_TYPE FsType)
{
    uint32 ClusIndex1;
    uint8  Rt;
    FDT    fdt;

    FILE_TREE_BASIC FileTreeBasic;
    uint16 temp;

    if (FsType != RECORD_DB)
    {
        return NOT_FIND_FILE;
    }
    else
    {
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));
        Index--;
    }

    Rt = PATH_NOT_FIND;
    if (DirClus != BAD_CLUS)            //确定路径存在
    {
        Rt = NOT_FIND_FILE;
        if (RETURN_OK == GetFDTInfo(&fdt, DirClus, Index))          //找到目录项
        {
            if ((fdt.Attr & ATTR_DIRECTORY) == 0)                       // 是文件才删除
            {
                Rt = FILE_LOCK;
                if (FindOpenFile1(DirClus, Index) >= MAX_OPEN_FILES)    //文件没有打开才删除
                {
                    ClusIndex1 = fdt.FstClusLO + ((uint32)(fdt.FstClusHI) << 16);

                    FATDelClusChain(ClusIndex1);

                    Rt = DelFDT(DirClus, fdt.Name);
                }
            }

        }
    }

    return (Rt);
}
#endif

/*********************************************************************************************************
** Name :FileOpen
** Description  :open one file by specified mode
** Input    :Path:路径, DirFileName:user use file name, Type:open type.
** Output       :Not_Open_FILE is can not open,other is the file handle
** global: OpenFileInfo
** call module: NULL
********************************************************************************************************/
IRAM_FAT
HANDLE FileOpen(uint8 * shortname, int32 DirClus, int32 Index, FS_TYPE FsType, uint8 *Type)
{
    uint8   i;
    MY_FILE *fp;
    HANDLE  Rt, OsRt;
    FDT     FileFDT;

    FILE_TREE_BASIC FileTreeBasic;
    uint16 temp;

    OsRt = NOT_OPEN_FILE;

    if (FsType == MUSIC_DB)
    {
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));
        Index--;
    }
    else if (FsType == RECORD_DB)
    {
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+RECORD_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+RECORD_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));
        Index--;
    }

    /* 查找空闲文件登记项 */
    for (Rt = 0; Rt < MAX_OPEN_FILES; Rt++)
    {
        if (OpenFileInfo[Rt].Flags == 0)
        {
            break;
        }
    }

    if (Rt < MAX_OPEN_FILES)
    {
        fp = OpenFileInfo + Rt;

        //fp->DirClus = GetDirClusIndex(Path);
        fp->DirClus = DirClus;
        if (fp->DirClus < BAD_CLUS) //找到路径
        {
            if (RETURN_OK == GetFDTInfo(&FileFDT, fp->DirClus, Index))
            {
                if ((FileFDT.Attr & ATTR_DIRECTORY) == 0)   //是文件
                {
                    for (i = 0; i < 11; i++)
                    {
                        fp->Name[i] = FileFDT.Name[i];
                        shortname[i] = FileFDT.Name[i];
                    }

                    fp->FileSize  = FileFDT.FileSize;
                    fp->FstClus   = FileFDT.FstClusLO | (uint32)FileFDT.FstClusHI << 16;
                    fp->Clus      = fp->FstClus;
                    fp->Offset    = 0;
                    fp->RefClus   = fp->Clus;
                    fp->RefOffset = 0;
                    fp->Index = Index;
                    {
                        {
                            fp->Flags = FILE_FLAGS_READ;
                            if (Type[0] == 'W' || Type[1] == 'W')
                            {
                                fp->Flags |= FILE_FLAGS_WRITE;
                            }
                            OsRt = Rt;
                        }
                    }
                }
            }
            else
            {
                printf("\n file open err: clus = %d, index =%d\n", fp->DirClus, Index);
            }
        }
        else
        {
            printf("\n file open err: clus = %d, index =%d\n", fp->DirClus, Index);
        }
    }

    return (OsRt);
}


/*********************************************************************************************************
** Name :FileOpen
** Description  :open one file by specified mode
** Input    :Path:路径, DirFileName:user use file name, Type:open type.
** Output       :Not_Open_FILE is can not open,other is the file handle
** global: OpenFileInfo
** call module: NULL
********************************************************************************************************/
IRAM_FAT
HANDLE FileOpenA(uint8 *Path, uint8 *DirFileName, uint8 *Type)
{
    uint8   i;
    MY_FILE *fp;
    HANDLE  Rt, OsRt;
    FDT     FileFDT;

    OsRt = NOT_OPEN_FILE;

    /* 查找空闲文件登记项 */
    for (Rt = 0; Rt < MAX_OPEN_FILES; Rt++)
    {
        if (OpenFileInfo[Rt].Flags == 0)
        {
            break;
        }
    }

    if (Rt < MAX_OPEN_FILES)
    {
        fp = OpenFileInfo + Rt;

        fp->DirClus = GetDirClusIndex(Path);

        if (fp->DirClus < BAD_CLUS) //找到路径
        {
            if (RETURN_OK == FindFDTInfo(&FileFDT, fp->DirClus, DirFileName))
            {
                if ((FileFDT.Attr & ATTR_DIRECTORY) == 0)   //是文件
                {
                    for (i = 0; i < 11; i++)
                    {
                        fp->Name[i] = *DirFileName++;
                    }

                    fp->FileSize  = FileFDT.FileSize;
                    fp->FstClus   = FileFDT.FstClusLO | (uint32)FileFDT.FstClusHI << 16;
                    fp->Clus      = fp->FstClus;
                    fp->Offset    = 0;
                    fp->RefClus   = fp->Clus;
                    fp->RefOffset = 0;
                    fp->Index = 0;
                    {
                        {
                            fp->Flags = FILE_FLAGS_READ;
                            if (Type[0] == 'W' || Type[1] == 'W')
                            {
                                fp->Flags |= FILE_FLAGS_WRITE;
                            }
                            OsRt = Rt;
                        }
                    }
                }
            }
        }
    }

    return (OsRt);
}


#ifdef LONG_DIR_PATH
uint32 StrLenWide(uint16* str)
{
    uint32 len = 0;
    while (str[len] != 0)
    {
        len++;
    }
    return len;
}

/*********************************************************************************************************
** Name :FileOpen
** Description  :open one file by specified mode
** Input    :Path:路径, DirFileName:user use file name, Type:open type.
** Output       :Not_Open_FILE is can not open,other is the file handle
** global: OpenFileInfo
** call module: NULL
********************************************************************************************************/
IRAM_FAT
HANDLE FileOpenW(uint8 * shortname, uint16 *Path, uint16 *DirFileName, uint8 *Type)
{
    uint8   i, ret;
    MY_FILE *fp;
    HANDLE  Rt, OsRt;
    FDT     FileFDT;
    uint8   SName[13];
    uint32 Index;


    OsRt = NOT_OPEN_FILE;


    /* 查找空闲文件登记项 */
    for (Rt = 0; Rt < MAX_OPEN_FILES; Rt++)
    {
        if (OpenFileInfo[Rt].Flags == 0)
        {
            break;
        }
    }

    if (Rt < MAX_OPEN_FILES)
    {
        fp = OpenFileInfo + Rt;

        fp->DirClus = GetDirClusIndexLong(Path, StrLenWide(Path));

        if (fp->DirClus < BAD_CLUS) //找到路径
        {

            if (FSIsLongName(DirFileName, StrLenWide(DirFileName)))
            {
                ret = FindFDTInfoLong(&FileFDT, fp->DirClus, &Index, DirFileName);
                //printf("ret = %d\n", ret);
            }
            else
            {
                //printf("short name clus = %d\n", fp->DirClus);
                FSRealname(SName, DirFileName);
                ret = FindFDTInfo(&FileFDT, fp->DirClus, SName);
                //printf("ret = %d\n", ret);

            }


            if (RETURN_OK == ret)
            {
                if ((FileFDT.Attr & ATTR_DIRECTORY) == 0)   //是文件
                {
                    for (i = 0; i < 11; i++)
                    {
                        fp->Name[i] = FileFDT.Name[i];
                        if (shortname != NULL)
                            shortname[i] = FileFDT.Name[i];
                    }

                    fp->FileSize  = FileFDT.FileSize;
                    fp->FstClus   = FileFDT.FstClusLO | (uint32)FileFDT.FstClusHI << 16;
                    fp->Clus      = fp->FstClus;
                    fp->Offset    = 0;
                    fp->RefClus   = fp->Clus;
                    fp->RefOffset = 0;
                    fp->Index = 0;
                    {
                        {
                            fp->Flags = FILE_FLAGS_READ;
                            if (Type[0] == 'W' || Type[1] == 'W')
                            {
                                fp->Flags |= FILE_FLAGS_WRITE;
                            }
                            OsRt = Rt;
                        }
                    }
                }
            }


        }
    }

    return (OsRt);
}

#endif


/*********************************************************************************************************
** Name :FileClose
** Description  :close the specified file.
** Input        :Path, DirFileName:user use filename.
** Output       :RETURN_OK：success
** other reference: the return value explaination in file fat.h
** global   :OpenFileInfo
** call module  : null
********************************************************************************************************/
IRAM_FAT
uint8 FileClose(HANDLE Handle)
{
    uint8  Rt;
    uint8  ChipSel;
    uint32 TempRow;
    uint16 mod;
    uint32 DestBlock;
    uint32 SrcBlock;
    uint16 len;
    FDT    FileFDT;
    MY_FILE *fp;

    Rt = PARAMETER_ERR;

    if (Handle >= 0 && Handle < MAX_OPEN_FILES)
    {
        Rt = RETURN_OK;
        fp = OpenFileInfo + Handle;

#ifdef ENCODE
        if ((fp->Flags & FILE_FLAGS_WRITE) == FILE_FLAGS_WRITE)
        {
            if (fp->FileSize > 0)
            {
#if 0
                if (RETURN_OK == FileSeek(0,SEEK_END,Handle))
                {
                    FATSetNextClus(fp->Clus, EOF_CLUS_END);
                }
#endif
                if (gFatCache.Sec!=0 && gFatCache.Flag!=0)
                {
                    gFatCache.Flag=0;
                    FATWriteSector(gFatCache.Sec, gFatCache.Buf);
                }
            }

            Rt = FindFDTInfo(&FileFDT, fp->DirClus, fp->Name);

            if (Rt == RETURN_OK)
            {
                FileFDT.FileSize = fp->FileSize;
                if (FileFDT.FstClusLO == 0 && FileFDT.FstClusHI == 0)   //是新建文件的情况
                {
                    FileFDT.FstClusLO = fp->FstClus & 0xffff;
                    FileFDT.FstClusHI = (fp->FstClus >> 16) & 0xffff;
                }

                ChangeFDT(fp->DirClus, &FileFDT);
            }
        }
#endif
        fp->Flags = 0;
    }

    return (Rt);
}


/*********************************************************************************************************
** Name :FileRead
** Description  :read file
** Input    :Buf:the buffer pointer to read data.
**                   Size: would read number of byte.not bigger than 64k
                 Handle specified file handle.
** Output       :the real number that had readed.
** global   :OpenFileInfo,BootSector
** call module: NULL
********************************************************************************************************/
IRAM_FAT
uint32 FileRead(uint8 *pData, uint32 NumBytes, HANDLE Handle)
{
    MY_FILE *fp;
    uint32 offsetInSec, offsetInClu;
    uint32 i;
    int64 SecIndex, ClusCnt, NextClus, remain, Len, cnt, wcnt;
    uint8  buf[512], *pBuf;

    cnt = NumBytes;
    remain = 0;
    fp = OpenFileInfo + Handle; //指向指定文件
    pBuf = pData;
    if (Handle >= 0 && Handle < MAX_OPEN_FILES)
    {
        if (fp->Flags == 0) // 对应的文件没打开
        {
            goto ReadErr;
        }

        if ((cnt + fp->Offset) > fp->FileSize) //判断所读的数据是不是超出文件大小
        {
            cnt = fp->FileSize - fp->Offset;
            remain = NumBytes - cnt;
        }

        UserIsrDisable();

        while (cnt > 0) //判文件结束
        {
            if (fp->Clus >= BAD_CLUS) //增加出错判断
            {
                break;
            }
            if (fp->Offset >= fp->FileSize)
            {
                break;
            }

            offsetInSec = fp->Offset & (BootSector.BPB_BytsPerSec - 1);
            offsetInClu = fp->Offset & ((1 << (LogBytePerSec + LogSecPerClus)) - 1);
            SecIndex = ((fp->Clus - 2) << LogSecPerClus) + BootSector.FirstDataSector + (offsetInClu >> LogBytePerSec);

            //UserIsrDisable();

            if (offsetInClu == 0 && cnt >= (1 << (LogSecPerClus + LogBytePerSec)))
            {
                ClusCnt = cnt >> (LogSecPerClus + LogBytePerSec);
                for (wcnt = 0; wcnt < ClusCnt; wcnt++)
                {
                    NextClus = FATGetNextClus(fp->Clus, 1);
                    if (NextClus == BAD_CLUS)
                    {
                        break;
                    }
                    if (NextClus == (fp->Clus + 1))
                    {
                        fp->Clus = NextClus;
                    }
                    else
                    {
                        wcnt++;

                        if (NextClus != EOF_CLUS_END)
                        {
                            fp->Clus = NextClus;
                        }
                        break;
                    }
                }
                if (NextClus == BAD_CLUS)
                {
                    break;
                }
                wcnt <<= (LogSecPerClus + LogBytePerSec);
                //地址对齐处理
                if (((uint32)(pBuf) & 0x03) == 0)
                {
                    if (RETURN_OK != MDRead(FileDiskID, SecIndex, wcnt >> LogBytePerSec, pBuf))
                    {
                        break;
                    }
                }
                else
                {
                    for (i = 0; i < (wcnt >> LogBytePerSec); i++)
                    {
                        if (RETURN_OK != MDRead(FileDiskID, SecIndex + i, 1, buf))
                        {
                            break;
                        }
                        memcpy(pBuf + i * 512, buf, 512);
                    }
                    if (i < (wcnt >> LogBytePerSec))
                    {
                        break;
                    }
                }
                pBuf += wcnt;
                cnt -= wcnt;
                //memcpy(Buf, &tmp[offsetInSec], len);
            }
            else
            {
                if (offsetInSec != 0)
                {
                    if (RETURN_OK != MDRead(FileDiskID, SecIndex++, 1, buf))
                    {
                        break;
                    }
                    Len = 512 - offsetInSec; //读到该扇区结束
                    if (Len > cnt)
                    {
                        Len = cnt;
                    }
                    memcpy(pBuf, buf + offsetInSec, Len);
                    wcnt = Len;
                    pBuf += Len;
                    cnt -= Len;
                    Len = (1 << (LogSecPerClus + LogBytePerSec)) - offsetInClu - Len; //到该簇结束未读的字节数
                }
                else
                {
                    wcnt = 0;
                    Len = (1 << (LogSecPerClus + LogBytePerSec)) - offsetInClu; //到该簇结束未读的字节数
                }

                // 接下来是若干个连续扇区
                if (Len > cnt)
                {
                    Len = cnt & (~0x1fful); //实际要读的字节数
                }
                if ((Len >> 9) > 0)
                {
                    //地址对齐处理
                    if (((uint32)(pBuf) & 0x03) == 0)
                    {
                        if (RETURN_OK != MDRead(FileDiskID, SecIndex, Len >> 9, pBuf))
                        {
                            break;
                        }
                    }
                    else
                    {
                        for (i = 0; i < (Len >> 9); i++)
                        {
                            if (RETURN_OK != MDRead(FileDiskID, SecIndex + i, 1, buf))
                            {
                                break;
                            }
                            memcpy(pBuf + i * 512, buf, 512);
                        }
                        if (i < (Len >> 9))
                        {
                            break;
                        }
                    }
                    wcnt += Len;
                    pBuf += Len;
                    cnt -= Len;
                    SecIndex += (Len >> 9);
                }

                // 最后是本簇内最后不足一扇区的数据(数据都在当前簇)，或者调整到下一簇
                if ((offsetInClu + wcnt) >= (1 << (LogSecPerClus + LogBytePerSec)))
                {
                    //调整到下一簇
                    NextClus = FATGetNextClus(fp->Clus, 1);
                    if (NextClus == BAD_CLUS)
                    {
                        break;
                    }
                    else if (NextClus != EOF_CLUS_END)
                    {
                        fp->Clus = NextClus;
                    }
                }
                else
                {
                    //数据都在当前簇，再读后面扇区内的零头
                    if (cnt < 512 && cnt > 0)
                    {
                        if (RETURN_OK != MDRead(FileDiskID, SecIndex++, 1, buf))
                        {
                            break;
                        }
                        memcpy(pBuf, buf, cnt);
                        wcnt += cnt;
                        pBuf += cnt;
                        cnt = 0;
                    }
                }
                //  FtlRead(MediaSel, SecIndex, 1, Buf);
            }
            fp->Offset += wcnt;
            //UserIsrEnable();

            if (fp->Offset >= fp->FileSize) //判文件结束
            {
                cnt = fp->Offset - fp->FileSize;
                fp->Offset = fp->FileSize;
                break;
            }
        }

        UserIsrEnable();

        return (NumBytes - cnt - remain);
    }
    ReadErr: return (0);
}

/*********************************************************************************************************
** Name :FileRead
** Description  :read file
** Input    :Buf:the buffer pointer to read data.
**                   Size: would read number of byte.not bigger than 64k
                 Handle specified file handle.
** Output       :the real number that had readed.
** global   :OpenFileInfo,BootSector
** call module: NULL
********************************************************************************************************/
IRAM_FAT
uint32 FileGetSize(HANDLE Handle)
{
    if (Handle >= 0 && Handle < MAX_OPEN_FILES)
    {
        MY_FILE *fp;
        fp = OpenFileInfo + Handle;     //指向指定文件
        if (fp->Flags == 0)         // 对应的文件没打开
        {
            return 0;
        }

        return fp->FileSize;
    }
    return 0;
}

IRAM_FAT
uint32 FileTell(HANDLE Handle)
{
    if (Handle >= 0 && Handle < MAX_OPEN_FILES)
    {
        MY_FILE *fp;
        fp = OpenFileInfo + Handle;     //指ˇ指定文件
        if (fp->Flags == 0)         // 对应的文件没打开
        {
            return 0;
        }

        return fp->Offset;
    }
    return 0;
}

#ifdef ENCODE

/*********************************************************************************************************
 ** Name    :FileWrite
 ** Description :write file
 ** Input   :Buf:the buffer pointer would to write
 **         Size:the size of would write
 Handle point to file handle.
 ** Output   :real writed byte number.
 ** global  :OpenFileInfo,BootSector
 ** call module : null
//功能: 提取某簇链的最后一簇
 ********************************************************************************************************/
IRAM_ENCODE
uint32 FAT_GetEofClus(uint32 Index)
{
    uint32 Count, thisclus, nextclus;

    if (Index >= BootSector.CountofClusters + 2)
    {
        return (BAD_CLUS);
    }

    if (FATType == FAT12)
    {
        Count = 4085;
    }
    else if (FATType == FAT16)
    {
        Count = 65525;
    }
    else
    {
        Count = 0x0FFFFFF0;
    }

    thisclus = Index;

    while (Count-- != 0)
    {
        nextclus = FATGetNextClus(thisclus, 1);
        if (nextclus >= BAD_CLUS)
        {
            break;
        }
        thisclus = nextclus;
    }

    if ((nextclus == BAD_CLUS) || (Count == 0))
    {
        thisclus = BAD_CLUS;
    }

    return thisclus;

}

/*********************************************************************************************************
 ** Name    :FileWrite
 ** Description :write file
 ** Input   :Buf:the buffer pointer would to write
 **         Size:the size of would write
 Handle point to file handle.
 ** Output   :real writed byte number.
 ** global  :OpenFileInfo,BootSector
 ** call module : null
 ********************************************************************************************************/
IRAM_ENCODE
uint32 FileWrite(uint8 *Buf, uint32 fileOffset, uint32 Size, HANDLE Handle)
{
    MY_FILE *fp;
    uint32 SecIndex;
    uint16 offsetInClu;
    uint16 len;
    uint32 count = Size;
    uint16 i;
    uint16 clusCnt;
    uint32 lastClus;
    uint8  TempBuf[512];

    fp = OpenFileInfo + Handle;
    if ((Size == 0) || (fileOffset % 512))
    {
        return (0);
    }

    if (fileOffset > fp->FileSize)
    {
        return (0);
    }

    //指向指定文件
    if (Handle >= 0 && Handle < MAX_OPEN_FILES)
    {
        if ((fp->Flags & FILE_FLAGS_WRITE) == 0) // 对应的文件是否以写方式打开
        {
            return 0;
        }

        if (fp->FstClus == 0)
        {
            lastClus = FATAddClus(fp->FstClus);
            if (lastClus == BAD_CLUS)
            {
                return 0;
            }
            else if (lastClus == EOF_CLUS_END)
            {
                // not space
                return 0;
            }
            fp->FstClus = lastClus;
            fp->Clus = lastClus;
        }


        //check fileoffset
        if (fileOffset > fp->Offset)
        {
            clusCnt = (fileOffset / (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus))
                      - ((fp->Offset / (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus)));
            if (fileOffset == fp->FileSize)
            {
                clusCnt--;
            }

            fp->Offset = fileOffset;

            if (clusCnt != 0)
            {
                lastClus = FATGetNextClus(fp->Clus, clusCnt);

                if (lastClus == BAD_CLUS)
                {
                    return 0;
                }
                fp->Clus = lastClus;
            }
        }
        else if (fileOffset < fp->Offset)
        {
            clusCnt = (fileOffset / (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus));

            fp->Offset = fileOffset;
            if (clusCnt != 0)
            {
                lastClus = FATGetNextClus(fp->FstClus, clusCnt);
                if (lastClus == BAD_CLUS)
                {
                    return 0;
                }
                fp->Clus = lastClus;
            }
            else
            {
                fp->Clus = fp->FstClus;
            }
        }
        if ((fileOffset == fp->FileSize) && (fp->FileSize != 0) && ((fileOffset % (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus)) == 0))
        {

            lastClus = FATAddClus(fp->Clus);
            if (lastClus == BAD_CLUS)
            {
                lastClus = FATAddClus(fp->FstClus);
                if (lastClus == BAD_CLUS)
                {
                    return 0;
                }
                else if (lastClus == EOF_CLUS_END)
                {
                    // not space
                    return 0;
                }
            }
            else if (lastClus == EOF_CLUS_END)
            {
                // not space
                return 0;
            }
            fp->Clus = lastClus;
        }

        while (count > 0)
        {
            offsetInClu = fp->Offset % (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus);
            SecIndex = ((fp->Clus - 2) * BootSector.BPB_SecPerClus) + BootSector.FirstDataSector + (offsetInClu / BootSector.BPB_BytsPerSec);
            if (count <= ((BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus) - offsetInClu))
            {
                len = (count / 512) + ((count % 512) ? 1 : 0);
                fp->Offset += count;
                if (fp->Offset > fp->FileSize)
                {
                    fp->FileSize = fp->Offset;
                }
                else if ((fp->Offset % (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus)) == 0)
                {
                    lastClus = FATGetNextClus(fp->Clus, 1);
                    if (lastClus == BAD_CLUS)
                    {
                        return 0;
                    }
                    fp->Clus = lastClus;
                }

                if (((uint32)(Buf) & 0x03) == 0)
                {
                    MDWrite(FileDiskID, SecIndex, len, Buf);
                }
                else
                {
                    for (i = 0; i < len; i++)
                    {
                        memcpy(TempBuf, Buf + i * 512, 512);
                        MDWrite(FileDiskID, SecIndex, 1, TempBuf);
                    }
                }
                return count;
            }
            else
            {
                len = ((BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus) - offsetInClu) / 512;
                fp->Offset += (len * 512);
                if (fp->Offset > fp->FileSize)
                {
                    fp->FileSize = fp->Offset;
                }

                count -= len * 512;
                do
                {
                    lastClus = FATGetNextClus(fp->Clus, 1);
                    if (lastClus == BAD_CLUS)
                    {
                        return 0;
                    }
                    else if (lastClus == EOF_CLUS_END)
                    {
                        lastClus = FATAddClus(fp->Clus);
                        if (lastClus == BAD_CLUS)
                        {
                            return 0;
                        }
                        else if (lastClus == EOF_CLUS_END)
                        {
                            // not space
                            return (Size - count);
                        }
                    }

                    if (lastClus == (fp->Clus + 1))
                    {
                        if (count > (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus))
                        {
                            len += ((BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus) / 512);
                            count -= (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus);
                            fp->Offset += (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus);
                        }
                        else
                        {
                            len += (count / 512);
                            if (count%512)
                            {
                                len+=1;
                            }
                            fp->Offset += count;
                            count = 0;
                        }

                        if (fp->Offset > fp->FileSize)
                        {
                            fp->FileSize = fp->Offset;
                        }
                        fp->Clus = lastClus;
                    }
                    else
                    {
                        fp->Clus = lastClus;
                        break;
                    }
                } while (count > 0);

                if (((uint32)(Buf) & 0x03) == 0)
                {
                    MDWrite(FileDiskID, SecIndex, len, Buf);
                }
                else
                {
                    for (i = 0; i < len; i++)
                    {
                        memcpy(TempBuf, Buf + i * 512, 512);
                        MDWrite(FileDiskID, SecIndex, 1, TempBuf);
                    }
                }
                Buf += (len * 512);
            }

        }

        if (((fp->Offset % (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus)) == 0) && (fp->Offset < fp->FileSize))
        {
            lastClus = FATGetNextClus(fp->Clus, 1);
            if (lastClus == BAD_CLUS)
            {
                return 0;
            }
            fp->Clus = lastClus;
        }

        return (Size - count);
    }

}
#endif


/*********************************************************************************************************
** Name :FileEof
** Description  :check whether it is had read/wrote to the end of file.
** Input    : null
** Output       :0:no,1:yes
** global: OpenFileInfo
** call module: NULL
********************************************************************************************************/
IRAM_FAT
bool FileEof(HANDLE Handle)
{
    bool Rt;

    Rt = TRUE;

    if (Handle < MAX_OPEN_FILES)
    {
        if (OpenFileInfo[Handle].Offset < OpenFileInfo[Handle].FileSize)
        {
            Rt = FALSE;
        }
    }

    return (Rt);
}


/*********************************************************************************************************
** Name :FileSeek
** Description  :move the postion of read/write
** Input    :offset:movement amount
**               Whence:move mode
                 SEEK_SET:
                 SEEK_CUR:
                 SEEK_END:
** Output       : null
** global   :OpenFileInfo
** call module  : null
********************************************************************************************************/
IRAM_FAT
uint8 FileSeek(int32 offset, uint8 Whence, HANDLE Handle)
{
    uint8   Rt;
    uint32  OldClusCnt;
    uint32  NewClusCnt;
    uint32  Clus;
    MY_FILE *fp;

    UserIsrDisable();
    Rt = PARAMETER_ERR;

    if (Handle >= 0 && Handle < MAX_OPEN_FILES)
    {
        fp = OpenFileInfo + Handle;

        if (fp->Flags  != 0)                                    // 对应的文件是否已打开
        {
            Rt = RETURN_OK;

            OldClusCnt = fp->Offset >> (LogSecPerClus + LogBytePerSec);

            switch (Whence)
            {
                case SEEK_END:                   /* 从文件尾计算 */
                    fp->Offset = fp->FileSize - offset;
                    offset = -offset;
                    break;

                case SEEK_SET:
                    fp->Offset = offset;            /* 从文件头计算 */
                    break;

                case SEEK_CUR:                          /* 从当前位置计算 */
                    fp->Offset += offset;
                    break;

                case SEEK_REF:
                    fp->Offset += offset;
                    if (fp->Offset >= fp->RefOffset)
                    {
                        OldClusCnt = fp->RefOffset >> (LogSecPerClus + LogBytePerSec);
                        fp->Clus = fp->RefClus;
                    }
                    break;

                default:
                    Rt = PARAMETER_ERR;
                    break;

            }

            if (Rt == RETURN_OK)
            {
                if (fp->Offset > fp->FileSize)
                {
                    fp->Offset = (offset > 0) ? fp->FileSize : 0;
                }

                /* 改变当前簇号 */
                NewClusCnt = fp->Offset >> (LogSecPerClus + LogBytePerSec);
                if (NewClusCnt >= OldClusCnt)
                {
                    if (((fp->Offset % (BootSector.BPB_BytsPerSec * BootSector.BPB_SecPerClus)) == 0) && (fp->Offset == fp->FileSize))
                    {
                        NewClusCnt = 1;
                        Clus = fp->Clus;
                    }
                    else
                    {
                        NewClusCnt -= OldClusCnt;
                        Clus = fp->Clus;
                    }
                }
                else
                {
                    Clus = fp->FstClus;
                }

                OldClusCnt = FATGetNextClus(Clus, NewClusCnt);
                if (OldClusCnt == BAD_CLUS)
                {
                    Rt = FAT_ERR;
                }
                else
                {
                    fp->Clus = OldClusCnt;
                }
            }
        }
    }

    UserIsrEnable();
    return (Rt);
}

#ifdef ENCODE
/*********************************************************************************************************
** Name         :FindOpenFile
** Description  :find the file handle that had been opened by specified file
** Input        :FileName:internal file name.
** Output       :file handle
** global       :OpenFileInfo
** call module  :NULL
********************************************************************************************************/
IRAM_ENCODE
HANDLE FindOpenFile(uint32 DirClus, uint8 *FileName)
{
    HANDLE Rt;
    MY_FILE *fp;
    uint8 i;

    fp = OpenFileInfo;
    for (Rt = 0; Rt < MAX_OPEN_FILES; Rt++)
    {
        if (fp->Flags != 0)
        {
            if (fp->DirClus == DirClus)
            {
                for (i = 0; i < 11; i++)
                {
                    if (fp->Name[i] != FileName[i])
                    {
                        break;
                    }
                }
                if (i == 11)
                {
                    break;
                }
            }
        }
        fp++;
    }

    return (Rt);
}

/*********************************************************************************************************
** Name         :FindOpenFile
** Description  :find the file handle that had been opened by specified file
** Input        :FileName:internal file name.
** Output       :file handle
** global       :OpenFileInfo
** call module  :NULL
********************************************************************************************************/
IRAM_ENCODE
HANDLE FindOpenFile1(uint32 DirClus, uint32 index)
{
    HANDLE Rt;
    MY_FILE *fp;
    uint8 i;

    fp = OpenFileInfo;
    for (Rt = 0; Rt < MAX_OPEN_FILES; Rt++)
    {
        if (fp->Flags != 0)
        {
            if (fp->DirClus == DirClus)
            {
                if (fp->Index == index)
                {
                    break;
                }
            }
        }
        fp++;
    }

    return (Rt);
}
#endif

#ifdef FIND_FILE

/*********************************************************************************************************
** Name :FindFile
** Description  :find the specified index file that is been in current direction or all direction.
**               FileNum:,Path:,ExtName:, Attr:
** Output       :Rt:the diretion information of found file items.
** global       :LongFileName,OpenFileInfo
** call module  :null
** explain      :if the extension is "*",it also will find direction.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint8 FindFileSub(FDT *Rt, uint16 FileNum, uint32 DirClus, uint8 *ExtName, FIND_DATA * FindData, FIND_TYPE FindType, FS_TYPE FsType)
{
    uint16 num;
    uint32 index;
    uint16 temp;
    FDT    tmp;
    FILE_TREE_BASIC FileTreeBasic;

    index = FindData->Index;
    num   = 0;

    if (FsType == MUSIC_DB)
    {
        while (1)
        {
            //printf("DirClus = %d, Index = %d\n", DirClus, index);
            if (index >= (FindData->TotalItem))
            {
                break;
            }

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + index), 2, (uint8 *)&temp);
            //printf("temp = %d \n", temp);
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
            index++;

            //printf("FileTreeBasic.dwNodeFlag = %d\n", FileTreeBasic.dwNodeFlag);
            if (FindType == FIND_TYPE_ALL)
            {
                if (++num == FileNum)
                {
                    FindData->Index = index;
#ifdef _RK_CUE_
                    if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE || FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_CUE)
#else
                    if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE)
#endif
                    {
                        Rt->Attr &= ~ ATTR_DIRECTORY;
#ifdef _RK_CUE_
                        if(FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_CUE)
                        {
                             FindData->IsCue = 1;
                             MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ CUE_START_SAVE_ADDR_OFFSET, 4, (uint8 *)&(FindData->CueStartTime));
                             MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ CUE_END_SAVE_ADDR_OFFSET, 4, (uint8 *)&(FindData->CueEndTime));
                        }
                        else
                        {
                             FindData->IsCue = 0;
                        }
#endif
                    }
                    else
                    {
                        Rt->Attr |= ATTR_DIRECTORY;
                    }

                    return (RETURN_OK);
                }
            }
            else if (FindType == FIND_TYPE_DIR)
            {
                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
                {
                    if (++num == FileNum)
                    {
                        Rt->Attr |= ATTR_DIRECTORY;
                        FindData->Index = index;
                        return (RETURN_OK);
                    }
                }
            }
            else
            {
#ifdef _RK_CUE_
                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE || FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_CUE)
#else
                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE)
#endif
                {
                    if (++num == FileNum)
                    {
                        FindData->Index = index;
                        Rt->Attr &= ~ ATTR_DIRECTORY;
#ifdef _RK_CUE_
                        if(FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_CUE)
                        {
                             FindData->IsCue = 1;
                             MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ CUE_START_SAVE_ADDR_OFFSET, 4, (uint8 *)&(FindData->CueStartTime));
                             MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ CUE_END_SAVE_ADDR_OFFSET, 4, (uint8 *)&(FindData->CueEndTime));
                        }
                        else
                        {
                             FindData->IsCue = 0;
                        }
#endif
                        return (RETURN_OK);
                    }
                }
            }
        }
    }
    else if (FsType == RECORD_DB)
    {
#ifdef _RK_CUE_
        FindData->IsCue = 0;
#endif
        while (1)
        {
            //printf("DirClus = %d, Index = %d\n", DirClus, index);
            if (index >= (FindData->TotalItem))
            {
                break;
            }

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + index), 2, (uint8 *)&temp);
            //printf("temp = %d \n", temp);
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
            index++;

            //printf("FileTreeBasic.dwNodeFlag = %d\n", FileTreeBasic.dwNodeFlag);
            if (FindType == FIND_TYPE_ALL)
            {
                if (++num == FileNum)
                {
                    FindData->Index = index;
                    if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE)
                    {
                        Rt->Attr &= ~ ATTR_DIRECTORY;
                    }
                    else
                    {
                        Rt->Attr |= ATTR_DIRECTORY;
                    }
                    FindData->DirOffsetInRDB = temp;
                    return (RETURN_OK);
                }
            }
            else if (FindType == FIND_TYPE_DIR)
            {
                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
                {
                    if (++num == FileNum)
                    {
                        Rt->Attr |= ATTR_DIRECTORY;
                        FindData->Index = index;
                        FindData->DirOffsetInRDB = temp;
                        return (RETURN_OK);
                    }
                }
            }
            else
            {
                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE)
                {
                    if (++num == FileNum)
                    {
                        FindData->Index = index;
                        FindData->DirOffsetInRDB = temp;
                        Rt->Attr &= ~ ATTR_DIRECTORY;
                        return (RETURN_OK);
                    }
                }
            }
        }
    }
    else
    {
#ifdef _RK_CUE_
        FindData->IsCue = 0;
#endif
        while (1)
        {
            if (RETURN_OK != GetFDTInfo(Rt, DirClus, index++))
            {
                break;
            }

            if (Rt->Name[0]==FILE_NOT_EXIST)                //空目录项,后面不再有文件
            {
                break;
            }

            if (Rt->Name[0] == 0x2e)
            {
                continue;
            }

            if (Rt->Attr & ATTR_VOLUME_ID)
            {
                continue;
            }

            if (Rt->Attr == ATTR_LFN_ENTRY)
            {
                continue;
            }

            if (Rt->Attr & ATTR_HIDDEN)
            {
                continue;
            }

            if (Rt->Attr & ATTR_DIRECTORY)
            {
                if (CurDirDeep == 0)
                {
                    if (FsType == FS_FAT_EX_VOICE)
                    {
                        if ((memcmp(Rt->Name, "RECORD     ", 11) == 0))
                        {
                            continue;
                        }
                    }
                }
            }
#ifdef _RK_CUE_
            else
            {
                if(FsType == FS_FAT_EX_VOICE)
                {
                    if(Rt->NTRes == 0x55)
                    {
                        //Rt->NTRes = 0x56;
                        Rt->NTRes = 0x00;
                        SetFDTInfo(DirClus, index - 1, Rt);
                        continue;
                    }
                }
            }
#endif

            if (Rt->Name[0] != FILE_DELETED)
            {
                if (FindType == FIND_TYPE_ALL)
                {
                    if (Rt->Attr & ATTR_DIRECTORY)
                    {
                        if (++num == FileNum)
                        {
                            FindData->Index = index;
                            return (RETURN_OK);
                        }
                    }
                    else
                    {
                        if (FileExtNameMatch(&Rt->Name[8], ExtName, Rt->Attr))
                        {
                            if (++num == FileNum)
                            {
                                FindData->Index = index;
                                return (RETURN_OK);
                            }
                        }
                    }
                }
                else if (FindType == FIND_TYPE_DIR)
                {
                    if (Rt->Attr & ATTR_DIRECTORY)
                    {
                        if (++num == FileNum)
                        {
                            FindData->Index = index;
                            return (RETURN_OK);
                        }
                    }
                }
                else
                {
                    if (FileExtNameMatch(&Rt->Name[8], ExtName, Rt->Attr) && (!(Rt->Attr & ATTR_DIRECTORY)))
                    {
                        if (++num == FileNum)
                        {
                            FindData->Index = index;
                            return (RETURN_OK);
                        }
                    }
                }

            }
        }
    }
    return (NOT_FIND_FILE);
}


/*********************************************************************************************************
** Name :FindFirst
** Description  :find the frist file in specified direction.
                FindData:file find structure,Path:ExtName:extension
**              FileNum:,Path:,ExtName:, Attr:
** Output       :Rt:the diretion information of found file items.
** global       :LongFileName,OpenFileInfo
** call module  :null
** explain      :if the extension is "*",it also will find direction.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint8 FindFirst(FDT *Rt, FIND_DATA* FindData, uint8 *ExtName, FS_TYPE FsType)
{
    FindData->Index = 0;
    return (FindNext(Rt, FindData, ExtName, FsType));
}

/*********************************************************************************************************
** Name :FindNext
** Description  :find the next file in specified direction.
                FindData:file find structure,Path:ExtName:extension
**              FileNum:,Path:,ExtName:, Attr:
** Output       :Rt:the diretion information of found file items.
** global       :LongFileName,OpenFileInfo
** call module  :null
** explain      :if the extension is "*",it also will find direction.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint8 FindNext(FDT *Rt, FIND_DATA* FindData, uint8 *ExtName, FS_TYPE FsType)
{
    uint8 OsRt;

    if (FindData->Clus >= BAD_CLUS)
    {
        OsRt = PATH_NOT_FIND;
        goto FileBErr;
    }

    OsRt = FindFileSub(Rt, 1, FindData->Clus, ExtName, FindData, FIND_TYPE_ALL, FsType);

FileBErr:

    return (OsRt);
}


/*********************************************************************************************************
** Name :FindFirst
** Description  :find the frist file in specified direction.
                FindData:file find structure,Path:ExtName:extension
**              FileNum:,Path:,ExtName:, Attr:
** Output       :Rt:the diretion information of found file items.
** global       :LongFileName,OpenFileInfo
** call module  :null
** explain      :if the extension is "*",it also will find direction.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint8 FindFirstDir(FDT *Rt, FIND_DATA* FindData, FS_TYPE FsType)
{
    FindData->Index = 0;
    return (FindNextDir(Rt, FindData, FsType));
}

/*********************************************************************************************************
** Name :FindNext
** Description  :find the next file in specified direction.
                FindData:file find structure,Path:ExtName:extension
**              FileNum:,Path:,ExtName:, Attr:
** Output       :Rt:the diretion information of found file items.
** global       :LongFileName,OpenFileInfo
** call module  :null
** explain      :if the extension is "*",it also will find direction.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint8 FindNextDir(FDT *Rt, FIND_DATA* FindData, FS_TYPE FsType)
{
    uint8 OsRt;

    if (FindData->Clus >= BAD_CLUS)
    {
        OsRt = PATH_NOT_FIND;
        goto FileBErr;
    }

    OsRt = FindFileSub(Rt, 1, FindData->Clus, "*", FindData, FIND_TYPE_DIR, FsType);

FileBErr:

    return (OsRt);
}


/*********************************************************************************************************
** Name :FindFirst
** Description  :find the frist file in specified direction.
                FindData:file find structure,Path:ExtName:extension
**              FileNum:,Path:,ExtName:, Attr:
** Output       :Rt:the diretion information of found file items.
** global       :LongFileName,OpenFileInfo
** call module  :null
** explain      :if the extension is "*",it also will find direction.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint8 FindFirstFile(FDT *Rt, FIND_DATA* FindData, uint8 *ExtName, FS_TYPE FsType)
{
    FindData->Index = 0;
    return (FindNextFile(Rt, FindData, ExtName, FsType));
}

/*********************************************************************************************************
** Name :FindNext
** Description  :find the next file in specified direction.
                FindData:file find structure,Path:ExtName:extension
**              FileNum:,Path:,ExtName:, Attr:
** Output       :Rt:the diretion information of found file items.
** global       :LongFileName,OpenFileInfo
** call module  :null
** explain      :if the extension is "*",it also will find direction.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint8 FindNextFile(FDT *Rt, FIND_DATA* FindData, uint8 *ExtName, FS_TYPE FsType)
{
    uint8 OsRt;

    if (FindData->Clus >= BAD_CLUS)
    {
        OsRt = PATH_NOT_FIND;
        goto FileBErr;
    }

    OsRt = FindFileSub(Rt, 1, FindData->Clus, ExtName, FindData, FIND_TYPE_FILE, FsType);

FileBErr:

    return (OsRt);
}


/*********************************************************************************************************
** Name :FileExtNameMatch
** Description  :filter the file by file extension.
** Input        :SrcExtName source extension,Filter
** Output       :TRUE match,FALSE no
** global       :null
** call module  :null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
bool FileExtNameMatch(uint8 *SrcExtName, uint8 *Filter,uint8 FileAttr)
{
    if (*Filter == '*')
    {
        return (TRUE);
    }

    while (*Filter != '\0')
    {
        if (SrcExtName[0] == Filter[0])
        {
            if (SrcExtName[1] == Filter[1])
            {
                if (SrcExtName[2] == Filter[2])
                {
                    if (FileAttr&ATTR_HIDDEN)
                    {
                        return (FALSE);
                    }
                    else
                    {
                        return (TRUE);
                    }
                }
            }
        }
        Filter += 3;
    }
    return (FALSE);
}

/*********************************************************************************************************
** Name :FileExtNameRemove
** Description  :filter the file extension.
** Input        :Long file name,Filter
** Output       :TRUE
** global       :null
** call module  :null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
bool FileExtNameRemove(uint16 *LongFileName, uint8 *Filter)
{
    uint16 i;
    uint16 j = MAX_FILENAME_LEN;
    uint16 TempBuf[4];

    for (i = 0; (i < MAX_FILENAME_LEN) || (LongFileName[i] == '\0'); i++)
    {
        if (LongFileName[i] == '.')
        {
            j = i;
        }
    }

    if ( j < (MAX_FILENAME_LEN - 4))
    {
        for (i = 0; i < 3; i++)
        {
            TempBuf[i] = LongFileName[j+i+1];

            if (TempBuf[i] >= 'a' && TempBuf[i] <= 'z')
            {
                TempBuf[i] = (TempBuf[i] - 'a' + 'A');
            }
        }

        while (*Filter != '\0')
        {
            if (TempBuf[0] == Filter[0])
            {
                if (TempBuf[1] == Filter[1])
                {
                    if (TempBuf[2] == Filter[2])
                    {
                        LongFileName[j] = '\0';
                    }
                }
            }
            Filter += 3;
        }
    }
    else if ((j >= (MAX_FILENAME_LEN - 4)) && (j < MAX_FILENAME_LEN))
    {
        LongFileName[j] = '\0';
    }

    return (TRUE);
}

/*********************************************************************************************************
** Name         :GetTotalFiles
** Description  :get total file number of current direction.
** Input        :ExtName:file extension
** Output       :file total
** global       :OpenFileInfo
** call module  :null
********************************************************************************************************/
#ifdef _RK_CUE_
extern  CUE_INFO CUEInfo;
#endif

_ATTR_FAT_FIND_CODE_
uint16 GetTotalFiles(uint32 DirClus, uint8 *ExtName, FS_TYPE FsType)
{
    uint32 offset, Index;
    uint16 TotalFiles;
    //uint8 buf[512];

    FDT    temp, Fdt;
    FILE_TREE_BASIC FileTreeBasic;

    offset     = 0;
    TotalFiles = 0;

    if (FsType == MUSIC_DB)
    {
        if (DirClus != 0xffffffff)
        {
            for (offset = DirClus; ; offset++)
            {
                /*
                if(offset % (512 / sizeof(FILE_TREE_BASIC)) == 0)
                {
                    MDRead(DataDiskID, MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + offset / (512 / sizeof(FILE_TREE_BASIC)), 1, buf);
                    FileTreeBasic = (FILE_TREE_BASIC *)buf;
                }
                */

                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(offset), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
#ifdef _RK_CUE_
                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE || FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_CUE)
#else
                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE)
#endif
                {
                    TotalFiles++;
                }

                if (FileTreeBasic.dwNextBrotherID == 0xffffffff)
                {
                    break;
                }
                //FileTreeBasic++;
            }
        }
    }
    else if (FsType == RECORD_DB)
    {
        if (DirClus != 0xffffffff)
        {
            for (offset = DirClus; ; offset++)
            {
                /*
                if(offset % (512 / sizeof(FILE_TREE_BASIC)) == 0)
                {
                    MDRead(DataDiskID, MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + offset / (512 / sizeof(FILE_TREE_BASIC)), 1, buf);
                    FileTreeBasic = (FILE_TREE_BASIC *)buf;
                }
                */

                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(offset), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);

                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE)
                {
                    TotalFiles++;
                }

                if (FileTreeBasic.dwNextBrotherID == 0xffffffff)
                {
                    break;
                }
                //FileTreeBasic++;
            }
        }
    }
    else
    {
        if (DirClus != BAD_CLUS)
        {
            while (1)
            {
                if (RETURN_OK != GetFDTInfo(&temp, DirClus, offset++))
                {
                    break;
                }

                if (temp.Name[0] == FILE_NOT_EXIST)
                {
                    break;
                }

                if (temp.Attr & ATTR_HIDDEN)
                {
                    continue;
                }

                if (temp.Name[0] != FILE_DELETED)
                {
                    if ((temp.Attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) == 0)   //是文件
                    {
                        while (temp.Attr == ATTR_LFN_ENTRY)     //长文件名
                        {
                            GetFDTInfo(&temp, DirClus, offset++);
                        }

#ifdef MEDIA_UPDATE
#ifdef _RK_CUE_
{
                        char cue[3] = "CUE";
                        if(GetFileType(cue, ExtName) != 0xff && FsType == FS_FAT_EX_VOICE)  //Is to search music files
                        {
                            if((temp.Name[8] == 'C')        //find CUE file
                                && (temp.Name[9] == 'U')
                                && (temp.Name[10] == 'E'))
                            {
                                int ret;
                                HANDLE hCueFile;

                                hCueFile = FileOpen(temp.Name, DirClus, offset - 1, FsType, "R");
                                if (hCueFile != NOT_OPEN_FILE)
                                {
                                    ParseCueInfo(hCueFile, &CUEInfo, 1);
                                }
                                FileClose(hCueFile);
                                hCueFile = -1;

                                ret = FindFDTInfoLong(&Fdt, DirClus, &Index, (uint16*)(CUEInfo.file));
                                TotalFiles--;

                                if(ret == RETURN_OK)
                                {
                                    Fdt.NTRes = 0x55;
                                    SetFDTInfo(DirClus, Index, &Fdt);
                                }
                                else
                                {//workwaround change language lead to death
                                    temp.NTRes = 0x55;
                                    SetFDTInfo(DirClus, offset-1, &temp);
                                }
                            }
                        }
}
#endif
#endif

                        if (FileExtNameMatch(&temp.Name[8], ExtName,temp.Attr))
                        {
                            TotalFiles++;
                        }
                    }
                }
            }
        }
    }
    return (TotalFiles);
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  GetDirPath()
*  Author:          ZHengYongzhi
*  Description:     get file full path
*
*  Input:   pPath           —— store the start address of file path
*  Output:  pPath           —— file path
*  Return:
*  Calls:
*
*  History:     <author>         <time>         <version>
*             ZhengYongzhi     2006/01/01         Ver1.0
*     desc: ORG
********************************************************************************
*/
_ATTR_FAT_FIND_CODE_
void GetLongFileName(int32 DirClus, int32 Index, FS_TYPE FsType, uint16* lfName )
{
    uint16 i;
    uint16 Item = 1;
    FDT     TempFDT;
    uint16 *buf = lfName;
    uint16 offset;
    uint16 StringCnt;

    FILE_TREE_BASIC FileTreeBasic;
    uint16 temp;

    if (FsType == MUSIC_DB)
    {
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));
#ifdef _RK_CUE_
        if(FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_CUE)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ ID3_TITLE_SAVE_ADDR_OFFSET, MAX_FILENAME_LEN, (uint8 *)lfName);
            return;
        }
#endif
        Index--;
    }
    else if (FsType == RECORD_DB)
    {
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));
        Index--;
    }

    memset((void*)lfName, 0, (MAX_FILENAME_LEN * 2));
    GetFDTInfo(&TempFDT, DirClus, Index);//读数据，判断是不是长文件名
//------------------------------------------------------------------------------
//先得到短文件名
    for (i=0; i<8; i++)
    {
        if (TempFDT.Name[i] == ' ')
            break;
        if ((TempFDT.NTRes & 0x08) && ((TempFDT.Name[i] >= 'A')&&(TempFDT.Name[i] <= 'Z')))
            *buf= TempFDT.Name[i]+32;
        else
            *buf= TempFDT.Name[i];
        buf++;
    }

    if (TempFDT.Name[8] != ' ')
    {
        *buf++ = '.';                //追加扩展名

        for (i=8; i<11; i++)
        {
            if ((TempFDT.NTRes & 0x10) && ((TempFDT.Name[i] >= 'A')&&(TempFDT.Name[i] <= 'Z')))
                *buf= TempFDT.Name[i]+32;
            else
                *buf= TempFDT.Name[i];
            buf++;
        }
    }

//    lfName[i] = '\0';                 //结束标志符unicode码的NUL
//------------------------------------------------------------------------------
//如果有长文件名，获取长文件名
    if (Index == 0)
        return;

    StringCnt = 0;
    while (Item <= MAX_LFN_ENTRIES)
    {
        uint16 * plfName;
        uint8 *buf;
        GetFDTInfo(&TempFDT, DirClus, Index - Item);//读数据，判断是不是长文件名
        if (ATTR_LFN_ENTRY != TempFDT.Attr)
            break;//不是长文件名，退出

        buf = (uint8 *)&TempFDT;

        if ((buf[0] & LFN_SEQ_MASK) <= MAX_LFN_ENTRIES)
        {
            plfName = lfName + 13 * ((buf[0] & LFN_SEQ_MASK) - 1);
            buf++;
            for (i = 0;i<5;i++)
            {//前面10个是byte
                *plfName = (uint16)*buf++;
                *plfName |= ((uint16)(*buf++))<<8;
                plfName++;
                StringCnt++;

                if (StringCnt >= MAX_FILENAME_LEN)
                {
                    *plfName = 0;
                    return;
                }
            }
            buf += 3;
            for (i = 0;i<6;i++)
            {
                *plfName = (uint16)*buf++;
                *plfName |= ((uint16)(*buf++))<<8;
                plfName++;
                StringCnt++;

                if (StringCnt >= MAX_FILENAME_LEN)
                {
                    *plfName = 0;
                    return;
                }
            }
            buf += 2;
            for (i = 0;i<2;i++)
            {
                *plfName = (uint16)*buf++;
                *plfName |= ((uint16)(*buf++))<<8;
                plfName++;
                StringCnt++;

                if (StringCnt >= MAX_FILENAME_LEN)
                {
                    *plfName = 0;
                    return;
                }
            }
        }

        if ((Index - Item) == 0)
        {
            *plfName = 0;
            return;
        }

        Item++;
    };
}


/*********************************************************************************************************
** Name :GetGlobeFileNum
** Description  :get global file index pointer from current direction file index pointer.
** Input    :current direction file index pointerFileNum, current file direction Path, file type ExtName
** Output       :global file pointer, return 0 if happen err.
** global   :CurDirDeep, CurDirClus
** call module  :GetDirClusIndex, GotoRootDir, GotoNextDir
** explain     :it will enter the direction automatically after call it.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint16 GetGlobeFileNum(uint16 FileNum, uint32 DirClus, uint8 *ExtName, FS_TYPE FsType)
{

    if (DirClus == BAD_CLUS)
    {
        FileNum = 0;
    }
    else
    {
        GotoRootDir(ExtName, FsType);

        while (DirClus != CurDirClus)       //直到找到当前目录
        {
            FileNum += SubDirInfo[CurDirDeep].TotalFile;

            GotoNextDir(ExtName, FsType);

            if (CurDirDeep == 0)
            {
                FileNum=0;
                break;
            }
        }
    }

    return (FileNum);
}
#endif

#ifdef ENCODE

/*********************************************************************************************************
** Name        :VolumeCreate
** Description :build disk lable
** Input       :DirFileName:lable name must be 8.3 format.
** Output      :
** global      :
** call module :
********************************************************************************************************/
IRAM_ENCODE
void VolumeCreate(uint8 *DirFileName)
{
    FDT temp, temp1;
    uint8 i;
    uint8 Result;
    uint32 index;

    for (i = 0; i < 11; i++)
    {
        temp.Name[i] = ' ';
    }
    for (i = 0; i < 11; i++)
    {
        if (DirFileName[i] == '\0')
            break;
        temp.Name[i] = DirFileName[i];
    }
    temp.Attr = ATTR_VOLUME_ID;
    temp.FileSize = 0;
    temp.NTRes = 0;
    temp.CrtTimeTenth = 0;
    temp.CrtTime = 0;
    temp.CrtDate = 0;
    temp.LstAccDate = 0;
    temp.WrtTime = 0;
    temp.WrtDate = 0;
    temp.FstClusLO = 0;
    temp.FstClusHI = 0;
    index=0;
    while (1)
    {
        Result = GetFDTInfo(&temp1, BootSector.BPB_RootClus, index);
        if (Result == FDT_OVER || Result != RETURN_OK)
        {
            break;
        }

        if (temp1.Name[0] == FILE_NOT_EXIST)
        {
            SetFDTInfo(BootSector.BPB_RootClus, index, &temp);
            break;
        }

        if ((temp1.Attr == ATTR_VOLUME_ID) || (temp1.Attr == (ATTR_ARCHIVE | ATTR_VOLUME_ID)))
        {
            for (i=0; i<11; i++)
            {
                if (temp1.Name[i] != temp.Name[i])
                {
                    SetFDTInfo(BootSector.BPB_RootClus, index, &temp);
                    break;
                }
            }
            break;
        }
        index++;
    }
}
#endif

#if 0
//3   Below code Just for create file to known Dir. --chad.ma 20160715
void CreateFiletoDir()
{
    HANDLE  hFile;
    uint32 i,ret;
    uint8 TmpBuf[512];

    #if 0   //短文件名指定目录创建文件示例
    uint8 * path = "\\RECORD     \\FM         "; //every dir path must be 11 bytes
    uint8 * filename= "TEST0000TXT";    //file name must be 8.3 format,11 bytes
    uint8 * tpye = "WR";

    if((hFile = FileOpenA(path, filename, tpye)== NOT_OPEN_FILE))   //file not exist
    {
        hFile = FileCreate(path, filename);

        DEBUG("---------- hFile = %d",hFile);
    }

    FileSeek(OpenFileInfo[hFile].FileSize, SEEK_SET, hFile);

    for(i = 0;i < 512;i++)
    {
        TmpBuf[i] = 0x31;
    }

    ret = FileWrite(TmpBuf, OpenFileInfo[hFile].FileSize, 512, hFile);
    DEBUG("------------ ret = %d",ret);

    FileClose(hFile);

    #else      //长文件名指定路径创建文件示例

    //uint16 * path = L"\\RECORD\\FM\\";
    uint16 * path = L"\\Recordxxx\\here\\"; //last dir path must be add '\\' to end.
    uint16 * filename = L"TEST11221122.txt";
    uint8 * tpye = "WR";

    ret = FileCreateDirW(L"\\",L"Recordxxx");
    DEBUG("---------- Create DIR status: %s",(ret == 0)? "OK":"Fail");

    ret = FileCreateDirW(L"\\Recordxxx\\",L"here");
    DEBUG("---------- Create DIR status: %s",(ret == 0)? "OK":"Fail");

    if((hFile = FileOpenW(NULL,path, filename, tpye)== NOT_OPEN_FILE))   //file not exist
    {
        ret = FileCreateFileW(path,filename);
        DEBUG("---------- Create file status: %s",(ret == 0)? "OK":"Fail");
    }

    if(ret == RETURN_OK)
    {
        hFile =  FileOpenW(NULL, path, filename, tpye);
        if(hFile == -1)
        {
            DEBUG("Open file fail.");
            return;
        }
    }


    FileSeek(OpenFileInfo[hFile].FileSize, SEEK_SET, hFile);

    for(i = 0;i < 512;i++)
    {
        TmpBuf[i] = 0x31;
    }

    ret = FileWrite(TmpBuf, OpenFileInfo[hFile].FileSize, 512, hFile);
    DEBUG("------------ ret = %d",ret);

    FileClose(hFile);
    #endif
}

#endif


/*
********************************************************************************
*
*                         End of File.c
*
********************************************************************************
*/
