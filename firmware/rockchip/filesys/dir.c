/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   Dir.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-10-21          1.0
*    desc:    ORG.
********************************************************************************
*/
#define  IN_DIR

#include "FsInclude.h"
#include "driver/MemDev/MemDev.h"        /* DataDiskID, MEDIAINFO_BLOCK_SIZE */
#include "sortfileinfo/AddrSaveMacro.h" /* MUSIC_TREE_*/RECORD_TREE_* sector starts */
#include "sortfileinfo/SortInfoGetMacro.h"


#ifdef FIND_FILE
/*********************************************************************************************************
** Name        :BuildDirInfo
** Description :create direction information
** Input       :expand name related file.
** Output      :file number
** global      :SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint16 BuildDirInfo(uint8* ExtName, FS_TYPE FsType)
{
    uint16 TotalFiles = 0;

    GotoCurDir(ExtName, FsType);

    while (1)
    {
        TotalFiles += SubDirInfo[CurDirDeep].TotalFile;

        //DEBUG("total file = %d, FsType = %d\n", TotalFiles, FsType);

        do
        {
            GotoNextDir(ExtName, FsType);           // 遍历下一个目录，找完子目录再找同级目录
        }
        while ((SubDirInfo[CurDirDeep].TotalFile == 0) && (CurDirDeep != 0));

        if (CurDirDeep == 0)
        {
            break;
        }
    }

    return (TotalFiles);
}


/*********************************************************************************************************
** Name         :GetCurFileNum
** Description  :get current direction file index pointer from global file index pointer.
** Input        :global file pointer FileNum, file type ExtName
** Output       :the file index pointer of current direction.return 0 if happen err.
** global       :CurDirDeep
** call module  :GotoRootDir, GotoNextDir
** explain     :it will enter the direction automatically after call it.
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint16 GetCurFileNum(uint16 FileNum, FIND_DATA * FindData, uint8 *ExtName, FS_TYPE FsType)
{
    uint16 temp;
    FILE_TREE_BASIC FileTreeBasic;

    //GotoRootDir(ExtName, FsType);
    GotoCurDir(ExtName, FsType);

    while (FileNum > SubDirInfo[CurDirDeep].TotalFile)
    {
        FileNum -= SubDirInfo[CurDirDeep].TotalFile;

        do
        {
            GotoNextDir(ExtName, FsType);           // 遍历下一个目录，找完子目录再找同级目录

        }
        while ((SubDirInfo[CurDirDeep].TotalFile == 0) && (CurDirDeep != 0));

        if (CurDirDeep == 0)
        {
            FileNum = 0;
            break;
        }
    }
    FindData->Clus = SubDirInfo[CurDirDeep].DirClus;
    FindData->TotalItem = SubDirInfo[CurDirDeep].TotalFile + SubDirInfo[CurDirDeep].TotalSubDir;

    return (FileNum);
}


/*********************************************************************************************************
** Name        : GotoNextDir
** Description : turn to next direction
** Input       : extension related files.
** Output      : null
** global      : SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
void GotoNextDir(uint8* ExtName, FS_TYPE FsType)
{
    if ((SubDirInfo[CurDirDeep].TotalSubDir == 0) || (CurDirDeep == (MAX_DIR_DEPTH - 1)))    //该目录下没有目录即为叶结点,要找父目录或同级目录
    {
        while (1)
        {
            if (CurDirDeep == 0)                    //找到根目录了不能再往上找
            {
                return;
            }

            CurDirDeep--;                       //即指向上一级目录
            if (SubDirInfo[CurDirDeep].CurDirNum < SubDirInfo[CurDirDeep].TotalSubDir)
            {
                SubDirInfo[CurDirDeep].CurDirNum++;
                CurDirClus = SubDirInfo[CurDirDeep].DirClus;
                CurDirDeep++;
                SubDirInfo[CurDirDeep].DirNum = TotalAllDir;
                CurDirClus = ChangeDir(SubDirInfo[CurDirDeep - 1].Index, FsType);
                break;
            }
        }
    }
    else            //该目录下还有子目录,要找它的第一个子目录
    {
        SubDirInfo[CurDirDeep].CurDirNum = 0;
        SubDirInfo[CurDirDeep].Index = 0;
        SubDirInfo[CurDirDeep].CurDirNum++;
        SubDirInfo[CurDirDeep].DirClus = CurDirClus;
        CurDirDeep++;
        SubDirInfo[CurDirDeep].DirNum = TotalAllDir;
        CurDirClus = ChangeDir(0, FsType);
    }

    SubDirInfo[CurDirDeep].TotalFile = GetTotalFiles(CurDirClus, ExtName, FsType);   //add by lxs @2005.02.24

    SubDirInfo[CurDirDeep].TotalSubDir = GetTotalSubDir(CurDirClus, FsType);     //获取该目录下的子目录数
    TotalAllDir += SubDirInfo[CurDirDeep].TotalSubDir;

}



/*********************************************************************************************************
** Name        : GotoNextDir
** Description : turn to next direction
** Input       : extension related files.
** Output      : null
** global      : SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint32 GetCurDir(uint32 DirClus, uint32 index, FS_TYPE FsType)
{
    FILE_TREE_BASIC FileTreeBasic;
    UINT32 cluster;
    UINT16 temp;

    if (FsType == MUSIC_DB)
    {
        //rk_printf("DirClus = %d, index =%d", DirClus, index);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 2)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);
        }
        else
        {
            cluster = 0;
        }
    }
    else if (FsType == RECORD_DB)
    {
        //printf("DirClus = %d, index =%d\n", DirClus, index);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 6)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);
            //printf("son = %d\n", cluster);
        }
        else
        {
            cluster = 0;
        }
    }
    else
    {

    }

    return cluster;
}



/*********************************************************************************************************
** Name        : GotoRootDir
** Description : goto boot direction.
** Input       : extension related file
** Output      : null
** global      : SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
void GotoRootDir(uint8* ExtName, FS_TYPE FsType)
{
    CurDirDeep = 0;                       //direction deepth.

    if (FsType == MUSIC_DB)
    {
        CurDirClus = 0;
    }
    else if (FsType == RECORD_DB)
    {
        CurDirClus = 0;
    }
    else
    {
        // FS_FAT, FS_FAT_EX_VOICE
        CurDirClus = BootSector.BPB_RootClus; //direction cluster boot direction.
    }

    SubDirInfo[CurDirDeep].TotalSubDir = GetTotalSubDir(CurDirClus, FsType);
    SubDirInfo[CurDirDeep].TotalFile   = GetTotalFiles(CurDirClus, ExtName, FsType);
    SubDirInfo[CurDirDeep].DirClus = CurDirClus;
    SubDirInfo[CurDirDeep].Index = 0;
    SubDirInfo[CurDirDeep].DirNum = 0;
    TotalAllDir = SubDirInfo[CurDirDeep].TotalSubDir;
}


/*********************************************************************************************************
** Name        : GotoRootDir
** Description : goto boot direction.
** Input       : extension related file
** Output      : null
** global      : SubDirInfo[],CurDirDeep,CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
void GotoCurDir(uint8* ExtName, FS_TYPE FsType)
{
    CurDirDeep = 0;                       //direction deepth.

    CurDirClus = gwSaveDirClus;

    SubDirInfo[CurDirDeep].TotalSubDir = GetTotalSubDir(CurDirClus, FsType);
    SubDirInfo[CurDirDeep].TotalFile   = GetTotalFiles(CurDirClus, ExtName, FsType);
    SubDirInfo[CurDirDeep].DirClus = gwSaveDirClus;
    SubDirInfo[CurDirDeep].Index = 0;
    SubDirInfo[CurDirDeep].DirNum = 0;
    TotalAllDir = SubDirInfo[CurDirDeep].TotalSubDir;
}

/*********************************************************************************************************
** Name :GetRootDirClus
** Description :
** Input   :
** Output         : null
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
long GetRootDirClus(FS_TYPE FsType)
{
    if (FsType == MUSIC_DB)
    {
        return 0;
    }
    else if (FsType == RECORD_DB)
    {
        return 0;
    }
    else
    {
        return(BootSector.BPB_RootClus);
    }
}

/*********************************************************************************************************
** Name         :ChangeDir
** Description  :change direction,turn to child direction point by index of current direction.
** Input        :SubDirIndex
** Output       :cluster number of index child direction.
** global       :CurDirClus
** call module  : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint32 ChangeDir(uint16 SubDirIndex, FS_TYPE FsType)
{
    FDT Rt;
    uint32 cluster;
    uint32 index;
    uint16 temp;
    FILE_TREE_BASIC FileTreeBasic;


    index= SubDirIndex;
    cluster = 0;

    if (FsType == MUSIC_DB)
    {
        while (1)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(CurDirClus + index), 2, (uint8 *)&temp);
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(CurDirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
            index++;
            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
            {
                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 2)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);
                break;
            }
        }

    }
    else if (FsType == RECORD_DB)
    {
        while (1)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(CurDirClus + index), 2, (uint8 *)&temp);
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(CurDirClus + temp), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
            index++;
            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
            {
                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 6)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), 4, (uint8 *)&cluster);
                break;
            }
        }

    }
    else
    {
        while (1)
        {
            if (RETURN_OK != GetFDTInfo(&Rt, CurDirClus, index++))
            {
                break;
            }

            if (Rt.Name[0] == 0x00)         //null mean end
            {
                break;
            }

            if (Rt.Name[0] == FILE_DELETED) //had deletec
            {
                continue;
            }

            if (Rt.Name[0] == 0x2e)
            {
                continue;
            }

            if (Rt.Attr & ATTR_HIDDEN)
            {
                continue;
            }

            if ((Rt.Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY)
            {
                if (CurDirDeep == 1)
                {
                    if (FsType == FS_FAT_EX_VOICE)
                    {
                        if (memcmp(Rt.Name, "RECORD      ", 11) == 0)
                        {
                            continue;
                        }
                    }
                }

                cluster   = Rt.FstClusHI;
                cluster <<= 16;
                cluster  |= Rt.FstClusLO;
                break;

            }
        }
    }

    SubDirInfo[CurDirDeep].DirClus = cluster;
    SubDirInfo[CurDirDeep-1].Index = index;
    return (cluster);
}


/*********************************************************************************************************
** Name        :GetTotalSubDir
** Description :get the total number of child direction.
** Input       :Path
** Output      :child direction number
** global      :CurDirClus
** call module : null
********************************************************************************************************/
_ATTR_FAT_FIND_CODE_
uint16 GetTotalSubDir(uint32 DirClus, FS_TYPE FsType)
{
    FDT Rt;
    uint32 index;
    uint16 TotSubDir=0;
    //uint8 buf[512];

    FILE_TREE_BASIC  FileTreeBasic;

    if (FsType == MUSIC_DB)
    {
        if (DirClus != 0xffffffff)
        {
            for (index = DirClus; ; index++)
            {
                /*
                if(index % (512 / sizeof(FILE_TREE_BASIC)) == 0)
                {
                    MDRead(DataDiskID, MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + index / (512 / sizeof(FILE_TREE_BASIC)), 1, buf);
                    FileTreeBasic = (FILE_TREE_BASIC *)buf;
                }
                */

                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(index), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);

                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
                {
                    TotSubDir++;
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
            for (index = DirClus; ; index++)
            {
                /*
                if(index % (512 / sizeof(FILE_TREE_BASIC)) == 0)
                {
                    MDRead(DataDiskID, MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START + index / (512 / sizeof(FILE_TREE_BASIC)), 1, buf);
                    FileTreeBasic = (FILE_TREE_BASIC *)buf;
                }
                */

                MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(index), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);

                if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
                {
                    TotSubDir++;
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
        if (DirClus == BAD_CLUS)
        {
            return (0);
        }

        for (index = 0; ; index++)
        {
            if (RETURN_OK != GetFDTInfo(&Rt, DirClus, index))
            {
                break;
            }

            if (Rt.Name[0] == FILE_NOT_EXIST)   //找到空的意为结束
            {
                break;
            }
            else if (Rt.Name[0] == FILE_DELETED)     //已删除
            {
                continue;
            }
            else if (Rt.Name[0] == 0x2e) // removed . and ..
            {
                continue;
            }
            else if ((FsType == FS_FAT_EX_VOICE) && (Rt.Attr & ATTR_DIRECTORY))
            {
                if (CurDirDeep == 0)
                {
                    if ((memcmp(Rt.Name, "RECORD     ", 11) == 0) )
                    {
                        //printf("find a record\n");
                        continue;
                    }
                }
            }

            if (Rt.Attr & ATTR_HIDDEN)
            {
                continue;
            }

            if ((Rt.Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY)
            {
                TotSubDir++;
            }
        }
    }

    return (TotSubDir);
}
#endif

/*********************************************************************************************************
** Name        : GetDirClusIndex
** Description : get start cluster number of sepecified direction.
** Input       : Path:(no include file name)
** Output      : start cluster number.
** global      : CurDirClus
** call module : FindFDTInfo
** explain     : '\' can not be the path end,suppot 1~11 path name.
********************************************************************************************************/
IRAM_FAT
uint32 GetDirClusIndex(uint8 *Path)
{
    uint8  i;
    uint32 DirClusIndex;
    FDT    temp;
    uint8  PathName[12];

    DirClusIndex = BAD_CLUS;

    if (Path != NULL)       //null pointer
    {
//***********************************************************************
//支持盘符如A:
//***********************************************************************
        if (Path[1] == ':')
        {
            Path += 2;
        }

        DirClusIndex = BootSector.BPB_RootClus; //根目录
//***********************************************************************
//A:TEMP、TEMP和.\TEMP都是指当前目录下的TEMP子目录
        if (Path[0] != '\\')            //* 不是目录分隔符号,表明起点是当前路径
        {
            DirClusIndex = CurDirClus;
        }
        else
        {
            Path++;
        }

        if (Path[0] == '.')             // '\.'表明起点是当前路径
        {
            DirClusIndex = CurDirClus;
            if (Path[1] == '\0' || Path[1] == '\\')     //case "." or ".\"
            {
                Path++;
            }
        }

        while (Path[0] != '\0')
        {
            if (Path[0] == ' ')         //首个字符不允许为空格
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            for (i = 0; i < 11; i++)        //目录项填空格
            {
                PathName[i] = ' ';
            }

            for (i = 0; i < 12; i++)
            {
                if (*Path == '\0')      //到路径结束
                {
                    break;
                }

                PathName[i] = *Path++;
            }

            if (FindFDTInfo(&temp, DirClusIndex, PathName) != RETURN_OK)    //获取FDT信息
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            if ((temp.Attr & ATTR_DIRECTORY) == 0)  //FDT是否是目录
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            DirClusIndex = ((uint32)(temp.FstClusHI) << 16) + temp.FstClusLO;
        }
    }

    return (DirClusIndex);

}


#ifdef LONG_DIR_PATH

IRAM_FAT
uint8 FSRealname(uint8 *dname, uint16 *pDirName)
{
    uint8 i;
    uint8 *pdname = dname;
    uint16 len;
    uint8 NTRes = 0;

    if ((*pDirName >= 'a') && (*pDirName <= 'z'))
    {
        NTRes |= 0x08;
    }


    for (i = 0; i < 11; i++)        //目录项填空格
    {
        dname[i] = ' ';
    }

    for (i = 0; i < 13; i++)   // 11 + '.' + '\0' = 13
    {
        if (*pDirName == '\0' || *pDirName == '\\' )      //到路径结束,
        {
            break;
        }

        if (*pDirName == '.') //文件名要去掉后缀名的.
        {
            pDirName++;
            pdname = &dname[8];

            if ((*pDirName >= 'a') && (*pDirName <= 'z'))
            {
                NTRes |= 0x10;
            }
            continue;
        }

        if ((*pDirName >= 'a') && (*pDirName <= 'z'))
        {
            *pdname = *pDirName - ('a' - 'A');
        }
        else
        {
            *pdname = *pDirName;
        }
        pdname++;
        pDirName++;

    }

    return NTRes;

}


IRAM_FAT
uint8 FSIsLongName(uint16 *dname, uint32 len)
{
    uint32  i = len;
    uint16 *p = dname;
    uint16 dot = 0;
    uint8 UpCase, LowCase;

    if (len > 12)
    {
        return 1;
    }

    while (i--)
    {
        if ( (*p > 127)
                ||(*p == '+')
                ||(*p == '=')
                ||(*p == ',')
                ||(*p == ';')
                ||(*p == '[')
                ||(*p == ']')
                ||(*p == ' ')) //有"+ = , ; [ ]"，也是长文件名
        {
            return 1;
        }
        else if (*p == '.')
        {
            dot++;
            if (dot > 1) /* 小数点多于1个，是长文件名 */
            {
                return 1;
            }

            if ((len-i-1) > 8)  /* 文件主名超过8，是长文件名 */
            {
                return 1;
            }

            if (i > 3)   /* 扩展名超过3，是长文件名 */
            {
                return 1;
            }
        }
        p++;
    }

    if ((len == 12) && (dot == 0))
    {
        return 1;
    }

    UpCase = 0;
    LowCase = 0;
    p = dname;

    for (i = 0; i < 8; i++)
    {
        if ((*p >='a') && (*p <='z'))
        {
            LowCase = 1;
        }
        else if ((*p >='A') && (*p <='Z'))
        {
            UpCase = 1;
        }

        //printf("%c\n", *p);
        p++;
        if (*p == '.')
        {
            p++;
            break;
        }
        else if ((*p == 0) || (*p == '\\'))
        {
            if ((LowCase == 1) && (UpCase == 1))
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }

    }

    if ((LowCase == 1) && (UpCase == 1))
    {
        return 1;
    }

    UpCase = 0;
    LowCase = 0;

    for (i = 0; i < 3; i++)
    {
        if ((*p >='a') && (*p <='z'))
        {
            LowCase = 1;
        }
        else if ((*p >='A') && (*p <='Z'))
        {
            UpCase = 1;
        }
        else if ((*p == 0) || (*p == '\\'))
        {
            break;
        }

        p++;
    }

    if (*p != 0)
    {
        return 1;
    }

    if ((LowCase == 1) && (UpCase == 1))
    {
        return 1;
    }

    return 0;
}

/*********************************************************************************************************
********************************************************************************************************/
IRAM_FAT
uint32 GetDirClusIndexLong(uint16 *Path, uint16 len)
{
    uint16  i;
    uint32 DirClusIndex, Index;
    FDT    temp;
    uint8   SName[13];
    uint16 *start;
    uint8 ret;

    DirClusIndex = BAD_CLUS;

    if (Path != NULL)       //null pointer
    {
//***********************************************************************
//支持盘符如A:
//***********************************************************************
        if (Path[1] == ':')
        {
            Path += 2;
            len -= 2;
        }

        DirClusIndex = BootSector.BPB_RootClus; //根目录
//***********************************************************************
//A:TEMP、TEMP和.\TEMP都是指当前目录下的TEMP子目录
        if (Path[0] != '\\')            //* 不是目录分隔符号,表明起点是当前路径
        {
            DirClusIndex = CurDirClus;
        }
        else
        {
            Path++;
            len--;
        }

        if (Path[0] == '.')             // '\.'表明起点是当前路径
        {
            DirClusIndex = CurDirClus;
            if (Path[1] == '\0' || Path[1] == '\\')     //case "." or ".\"
            {
                Path++;
                len--;
            }
        }

        while (len > 0)
        {
            if (Path[0] == ' ')         //首个字符不允许为空格
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            start = Path;
            for (i = 1; i < 256; i++)
            {
                Path++;
                len--;
                if (*Path == '\\')
                {
                    Path++;
                    len--;
                    break;
                }
            }

            if (FSIsLongName(start, i))
            {
                *(start + i) = L'\0';
                ret = FindFDTInfoLong(&temp, DirClusIndex, &Index, start);
                *(start + i) = L'\\';
            }
            else
            {
                FSRealname(SName, start);
                ret = FindFDTInfo(&temp, DirClusIndex, SName);
            }

            if (ret != RETURN_OK)    //获取FDT信息
            {
                DirClusIndex = BAD_CLUS;
                break;
            }


            if ((temp.Attr & ATTR_DIRECTORY) == 0)  //FDT是否是目录
            {
                DirClusIndex = BAD_CLUS;
                break;
            }

            DirClusIndex = ((uint32)(temp.FstClusHI) << 16) + temp.FstClusLO;
            // printf("long: DirClus = %d\n", DirClusIndex);
        }
    }

    return (DirClusIndex);

}

#endif

#ifdef ENCODE
/*********************************************************************************************************
** Name        :ClearClus
** Description :clear the specified cluster to zero.
** Input       :Path
** Output      :RETURN_OK：success
** other reference:the return value explain in file fat.h中关于返回值的说明
** global :FileInfo
** call module :AddFDT, GetDirClusIndex
********************************************************************************************************/
IRAM_ENCODE
uint8 ClearClus(uint32 Index)
{
    uint16 i;
    uint32 SecIndex;
    uint8  buf[512];

    memset(buf, 0, 512);

    if (Index < (BootSector.CountofClusters + 2))
    {
        SecIndex = ((Index - 2) << LogSecPerClus) + BootSector.FirstDataSector;

        for (i = 0; i < BootSector.BPB_SecPerClus; i++)
        {
            FATWriteSector(SecIndex++, buf);
        }
        return (RETURN_OK);
    }
    else
    {
        return (CLUSTER_NOT_IN_DISK);
    }

}

/*********************************************************************************************************
** Name        :MakeDir
** Description :create direction
** Input       :Path: DirFileName the path name use the 8.3 format.
** Output      :RETURN_OK：成功
** other reference the return value explain in file fat.h
** global :FileInfo
** call module :AddFDT, GetDirClusIndex
********************************************************************************************************/
IRAM_ENCODE
uint8 MakeDir(uint8 *Path, uint8 *DirFileName)
{
    uint8 Rt;
    uint8 i;
    uint32 ClusIndex, Temp1;
    FDT temp;
    uint32 index;

    ClusIndex = GetDirClusIndex(Path);

    Rt = PATH_NOT_FIND;
    if (ClusIndex != BAD_CLUS)
    {
        for (i = 0; i < 11; i++)        //目录项填空格
        {
            temp.Name[i] = ' ';
        }

        for (i = 0; i < 11; i++)
        {
            if (*DirFileName == '\0')   //到路径结束
            {
                break;
            }

            temp.Name[i] = *DirFileName++;
        }

        /* FDT是否存在 */
        Rt = FDTIsLie(ClusIndex, temp.Name);
        if (Rt == NOT_FIND_FDT)
        {
            /* 不存在 */
            Temp1 = FATAddClus(0);                  /* 获取目录所需磁盘空间 */
            Rt    = DISK_FULL;                      /* 没有空闲空间 */

            if ((Temp1 > EMPTY_CLUS_1) && (Temp1 < BAD_CLUS))
            {
                ClearClus(Temp1);                       /* 清空簇 */

                /* 设置FDT属性 */
                temp.Attr         = ATTR_DIRECTORY;
                temp.FileSize     = 0;
                temp.NTRes        = 0;
                temp.CrtTimeTenth = 0;
                temp.CrtTime      = 0;
                temp.CrtDate      = 0;
                temp.LstAccDate   = 0;
                temp.WrtTime      = 0;
                temp.WrtDate      = 0;
                temp.FstClusLO    = Temp1 & 0xffff;
                temp.FstClusHI    = Temp1 / 0x10000;

                Rt = AddFDT(ClusIndex, &temp, &index);       /* 增加目录项 */

                if (Rt == RETURN_OK)
                {
                    /* 建立'.'目录 */
                    temp.Name[0] = '.';
                    for (i = 1; i < 11; i++)
                    {
                        temp.Name[i] = ' ';
                    }
                    AddFDT(Temp1, &temp, &index);

                    /* 建立'..'目录 */
                    temp.Name[1] = '.';
                    if (ClusIndex == BootSector.BPB_RootClus)
                    {
                        ClusIndex = 0;
                    }

                    temp.FstClusLO = ClusIndex & 0xffff;
                    temp.FstClusHI = ClusIndex / 0x10000;
                    Rt = AddFDT(Temp1, &temp, &index);
                }
                else
                {
                    FATDelClusChain(Temp1);
                }

                Rt = RETURN_OK;
            }
        }
    }

    ///////////////////////////////////////////////////////////
    //回写FAT Cache
    if (gFatCache.Sec!=0 && gFatCache.Flag!=0)
    {
        gFatCache.Flag=0;
        FATWriteSector(gFatCache.Sec, gFatCache.Buf);
    }
    ///////////////////////////////////////////////////////////

    return (Rt);
}
#endif
/*
********************************************************************************
*
*                         End of Dir.c
*
********************************************************************************
*/

