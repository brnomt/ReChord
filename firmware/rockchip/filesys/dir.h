/*
********************************************************************************
*                   Copyright (c) 2009,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   dir.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2009-02-06         1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _DIR_H
#define _DIR_H
#include "filesys/FDT.h"   /* FIND_DATA (search cursor) - explicit path: include/ has a shadow FDT.h */

#undef  EXT
#ifdef  IN_DIR
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
//1configurable parameter
#define MAX_DIR_DEPTH       8               //the deepth of direction is 4
#define MAX_PATH_NAME       (2 + (MAX_DIR_DEPTH  - 1) * (11 + 1) + 4)

/*
*-------------------------------------------------------------------------------
*
*                          Struct Define
*
*-------------------------------------------------------------------------------
*/
typedef __packed struct
{
    uint16 TotalFile;
    uint16 TotalSubDir;
    uint16 CurDirNum;
    uint8  DirName[11];                         //short direction name take the occupation of 11 charactors.
    uint32 DirNum;
    uint32 DirClus;    //SubDirInfo[0].DirClus boot diretion
    uint32 Index;      //SubDirInfo[0].Index   the child direction index of boot direction.

} SUB_DIR;


typedef __packed struct _FILE_TREE_BASIC
{
    uint32 dwNextBrotherID;
    uint32 dwExtendTreeID;
    uint32 dwBasicInfoID;
    uint32 dwNodeFlag; // dir , file or cue
}
FILE_TREE_BASIC, * P_FILE_TREE_BASIC;


typedef __packed struct _FILE_TREE_EXTEND
{
    uint32 dwSonID;
}
FILE_TREE_EXTEND, * P_FILE_TREE_EXTEND;


typedef enum
{
    MEDIA_FILE_TYPE_DIR,
    MEDIA_FILE_TYPE_FILE,

#ifdef _RK_CUE_
    MEDIA_FILE_TYPE_CUE,
#endif

    MEDIA_FILE_TYPE_DELETED
}
MEDIA_FILE_TYPE;


#ifndef _FS_TYPE_DEFINED
#define _FS_TYPE_DEFINED
typedef enum
{
   MUSIC_DB = 1,
   RECORD_DB,
   FS_FAT,
   FS_EXFAT,
   FS_NTFS,
   FS_FAT_EX_VOICE,
   FS_EXFAT_EX_VOICE,
   FS_NTFS_EX_VOICE
   
}FS_TYPE;
#endif /* _FS_TYPE_DEFINED */






/*
*-------------------------------------------------------------------------------
*
*                          Variable Define
*
*-------------------------------------------------------------------------------
*/
DRAM_FAT    EXT     uint8   CurDirDeep;                 //current direction deepth
DRAM_FAT    EXT     uint32 TotalAllDir;                 //all dir
DRAM_FAT    EXT     SUB_DIR SubDirInfo[MAX_DIR_DEPTH + 1];  //child direction infomation list.

/*
*-------------------------------------------------------------------------------
*
*                          Function Declaration
*
*-------------------------------------------------------------------------------
*/
extern uint16 BuildDirInfo(uint8* ExtName, FS_TYPE FsType);
extern uint16 GetCurFileNum(uint16 FileNum, FIND_DATA * FindData, uint8 *ExtName, FS_TYPE FsType);
extern void GotoNextDir(uint8* ExtName, FS_TYPE FsType);
extern uint32 GetCurDir(uint32 DirClus, uint32 index, FS_TYPE FsType);
extern void GotoRootDir(uint8* ExtName, FS_TYPE FsType);
extern void GotoCurDir(uint8* ExtName, FS_TYPE FsType);
extern long GetRootDirClus(FS_TYPE FsType);
extern uint32 ChangeDir(uint16 SubDirIndex, FS_TYPE FsType);
extern uint16 GetTotalSubDir(uint32 DirClus, FS_TYPE FsType);
extern uint32 GetDirClusIndex(uint8 *Path);
extern uint32 GetDirClusIndexLong(uint16 *Path, uint16 len);
extern uint8 MakeDir(uint8 *Path, uint8 *DirFileName);
extern uint8 ClearClus(uint32 Index);

/*
********************************************************************************
*
*                         End of dir.h
*
********************************************************************************
*/
#endif
