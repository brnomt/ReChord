/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   file.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _FILE_H
#define _FILE_H
#include "include/SysConfig.h"   /* SYS_SUPPROT_STRING_MAX_LEN & config */
#include "filesys/FDT.h"
#include "filesys/dir.h"   /* FS_TYPE enum */   /* FIND_DATA (search cursor) - explicit path: include/ has a shadow FDT.h */

#undef  EXT
#ifdef  IN_FILE
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
//define file handle type
typedef     int8                    HANDLE;

/* file system data width ,not change this define*/
#define     X8                      1
#define     X16                     2
#define     X32                     4

/* function return value list */
//#define   RETURN_OK               0x00        /* operate success*/
#define     NOT_FIND_DISK           0x01        /* logic disk do not presence*/
#define     DISK_FULL               0x02        /* logic disk full*/
#define     SECTOR_NOT_IN_CACHE     0x03        /* sector do not be cache  */
#define     NOT_EMPTY_CACHE         0x04        /* no idle cache*/
#define     SECTOR_READ_ERR         0x05        /* read sector error*/
#define     CLUSTER_NOT_IN_DISK     0x06        /* no this cluster in logic disk*/
#define     NOT_FIND_FDT            0x07        /* do not find file/dir*/
#define     NOT_FAT_DISK            0x08        /* it is not FAT file system*/
#define     FDT_OVER                0x09        /* index of FDT exceed range*/
#define     FDT_EXISTS              0x0a        /* file/dir had presence*/
#define     ROOT_FDT_FULL           0x0b        /* root direction full*/
#define     DIR_EMPTY               0x0C        /* direction empty*/
#define     DIR_NOT_EMPTY           0x0d        /* direction do not empty*/
#define     PATH_NOT_FIND           0x0e        /* do not find the path*/
#define     FAT_ERR                 0x0f        /* FAT table err*/
#define     FILE_NAME_ERR           0x10        /* name of file/direction err.*/
#define     FILE_EOF                0x11        /* file end*/
#define     FILE_LOCK               0x12        /* file be locked */
#define     NOT_FIND_FILE           0x13        /* no find specified file*/
#define     NOT_FIND_DIR            0x14        /* no find specified direction*/
#define     NOT_RUN                 0xfd        /* command do not execution*/
#define     BAD_COMMAND             0xfe        /* err commadn*/
#define     PARAMETER_ERR           0xff        /* err parameter*/
#define     NOT_OPEN_FILE           -1          /* do not open file or file handle full*/

#define     FILENAME_LENGTH_OVER    0xf8        /* 文件名太长*/
#define     FDT_LENGTH_OVER         0xf9        /* 文件项的长度太长*/
#define     FILE_RW_ERR             0xfa        /* 文件的的数据读写错误*/
#define     FILE_TRAIL_OVER         0xfb        /* 文件的尾数超过最大限制*/
#define     API_INTERRUPT           0xfc        /* API函数执行被应用层中断*/

#define     NOT_RUN                 0xfd        /* 命令未执行*/
#define     BAD_COMMAND             0xfe        /* 错误命令*/
#define     PARAMETER_ERR           0xff        /* 非法参数*/
#define     NOT_OPEN_FILE           -1          /*不能打开文件,文件句柄满*/

#define     MAX_OPEN_FILES          8           /*the number that can open at the same time*/
#define     MAX_LFN_ENTRIES         20          /*the max length is 20*13 charactors.*/

/* long file name*/
#define     MAX_FILENAME_LEN        (SYS_SUPPROT_STRING_MAX_LEN + 1)//the null of end occupy 2 bytes.
#define     LFN_SEQ_MASK            0x3f



/* file open mode */
#define     FILE_FLAGS_READ         1 << 0      //can read,
#define     FILE_FLAGS_WRITE        1 << 1      //can write

/* file pointer modify type. */
#define     SEEK_SET                0           //move the file pointer from the file start position.
#define     SEEK_CUR                1           //move the file pointer from the file current position.
#define     SEEK_END                2           //move the file pointer from the file end position.
#define     SEEK_REF                3           //move the file pointer from the file refer position.
#define     OPEN_TYPE_WRITE         "w"
#define     OPEN_TYPE_READ          "b"

/*
--------------------------------------------------------------------------------

                           Struct define

--------------------------------------------------------------------------------
*/
//文件信息结构体
__packed typedef struct _FILE
{
    uint8   Flags;                              //some flag
    uint8   Name[11];                           //file name
    uint32  DirClus;                            //the start cluster number of current direction.
    uint32  FileSize;                           //file size
    uint32  FstClus;                            //start cluster number
    uint32  Clus;                               //current cluster number.
    uint32  Index;                              //Current file short entry index in dir
    uint32  Offset;                             //the offset of file pointer
    uint32  RefClus;                            //current cluster
    uint32  RefOffset;                          //the offset of file pointer

} MY_FILE;


__packed typedef struct _recovery_info
{
    uint8 Media;
    uint8 Flag;
    uint8 FileName[11];
    uint8 FstClus0;
    uint8 FstClus1;
    uint8 FstClus2;
    uint8 FstClus3;
    uint8 DirClus0;
    uint8 DirClus1;
    uint8 DirClus2;
    uint8 DirClus3;

} RCV_INFO, *PRCV_INFO;

typedef __packed struct tagFS_LONG_FDT
{
    uint8  Order;
    char  Name1[10];
    uint8  Attr;
    uint8  Type;
    uint8  Chksum;
    char  Name2[12];
    uint16  FstClusLO;
    char  Name3[4];
}FS_LONG_FDT;


typedef enum
{
    FIND_TYPE_FILE = 1,
    FIND_TYPE_DIR,
    FIND_TYPE_ALL

}FIND_TYPE;


#ifndef _MEDIA_MSGBOX_FILE_CMD_DEFINED
#define _MEDIA_MSGBOX_FILE_CMD_DEFINED
typedef enum _MEDIA_MSGBOX_FILE_CMD
{
    MEDIA_MSGBOX_CMD_FILE_NULL = 100,

  //MEDIA_MSGBOX_CMD_FILE_OPEN,
    MEDIA_MSGBOX_CMD_FILE_OPEN_CMPL,
    MEDIA_MSGBOX_CMD_FILE_OPEN_HANDSHK,

    //MEDIA_MSGBOX_CMD_FILE_CREATE,
    MEDIA_MSGBOX_CMD_FILE_CREATE_CMPL,
    MEDIA_MSGBOX_CMD_FILE_CREATE_HANDSHK,

    MEDIA_MSGBOX_CMD_FILE_SEEK,
    MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL,

    MEDIA_MSGBOX_CMD_FILE_READ,
    MEDIA_MSGBOX_CMD_FILE_READ_CMPL,

    MEDIA_MSGBOX_CMD_FILE_WRITE,
    MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL,

    MEDIA_MSGBOX_CMD_FILE_TELL,
    MEDIA_MSGBOX_CMD_FILE_TELL_CMPL,

    MEDIA_MSGBOX_CMD_FILE_GET_LENGTH,
    MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL,

    MEDIA_MSGBOX_CMD_FILE_CLOSE,
    MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL,
    MEDIA_MSGBOX_CMD_FILE_CLOSE_HANDSHK,

}MEDIA_MSGBOX_FILE_CMD;
#endif /* _MEDIA_MSGBOX_FILE_CMD_DEFINED */



/*
--------------------------------------------------------------------------------

                           Variable Define

--------------------------------------------------------------------------------
*/
DRAM_FAT           EXT     uint32      CurDirClus;                             //current direction cluster
DRAM_FAT         EXT     uint32      gwSaveDirClus;

//DRAM_FAT        EXT     uint32      FreeMem[2];
DRAM_FAT        EXT     MY_FILE     OpenFileInfo[MAX_OPEN_FILES];               //file infromation table opened at the same time.
DRAM_FAT        EXT     uint16      LongFileName[MAX_FILENAME_LEN];         //long file name.UNICODE
//DRAM_FAT        EXT     RCV_INFO    EncRcvInfo ;

//#ifdef ENCODE
//DRAM_ENCODE     EXT     uint32      FileWriteResidue;                       //file write
//#endif

/*
*-------------------------------------------------------------------------------
*
*                            Function Declaration
*
*-------------------------------------------------------------------------------
*/
extern void FileInit(void);

#ifdef ENCODE
extern HANDLE FileCreateSub(uint8 *Path, uint8 *DirFileName, uint8 Attr);
extern HANDLE FileCreate(uint8 *Path, uint8 *DirFileName);
extern HANDLE FileCreate_Hidden(uint8 *Path, uint8 *DirFileName);
extern uint32 FileWrite(uint8 *Buf, uint32 fileOffset, uint32 Size, HANDLE Handle);
extern uint8 FileDelete(uint8 *Path, uint8 *DirFileName);

extern uint8 FileDelete1(uint32 DirClus, uint32 Index,FS_TYPE FsType);

extern HANDLE FindOpenFile(uint32 DirClus, uint8 *FileName);
extern HANDLE FindOpenFile1(uint32 DirClus, uint32 index);
#endif


#ifdef LONG_DIR_PATH
extern uint32 StrLenWide(uint16* str);
#endif

extern HANDLE FileOpen(uint8 * shortname, int32 DirClus, int32 Index, FS_TYPE FsType, uint8 *Type);
extern HANDLE FileOpenA(uint8 *Path, uint8 *DirFileName, uint8 *Type);
extern uint32 FileRead(uint8 *Buf, uint32 Size, HANDLE Handle);
extern uint8 FileSeek(int32 offset, uint8 Whence, HANDLE Handle);
extern bool FileEof(HANDLE Handle);
extern uint8 FileClose(HANDLE Handle);

extern uint8 FindFileSub(FDT *Rt, uint16 FileNum, uint32 DirClus, uint8 *ExtName, FIND_DATA * FindData, FIND_TYPE FindType, FS_TYPE FsType);
extern uint8 FindFile(FDT *Rt, uint16 FileNum, uint8 *Path, uint8 *ExtName);
extern uint8 FindFileBrowser(FDT *Rt, uint16 FileNum, uint8 *Path, uint8 *ExtName);
extern uint8 FindFirst(FDT *Rt, FIND_DATA* FindData, uint8 *ExtName, FS_TYPE FsType);
extern uint8 FindNext(FDT *Rt, FIND_DATA* FindData, uint8 *ExtName, FS_TYPE FsType);
extern uint8 FindFirstFile(FDT *Rt, FIND_DATA* FindData, uint8 *ExtName, FS_TYPE FsType);
extern uint8 FindNextFile(FDT *Rt, FIND_DATA* FindData, uint8 *ExtName, FS_TYPE FsType);
extern uint8 FindFirstDir(FDT *Rt, FIND_DATA* FindData, FS_TYPE FsType);
extern uint8 FindNextDir(FDT *Rt, FIND_DATA* FindData, FS_TYPE FsType);

extern bool FileExtNameMatch(uint8 *SrcExtName, uint8 *Filter,uint8 FileAttr);

extern bool FileExtNameRemove(uint16 *LongFileName, uint8 *Filter);
extern uint16 GetTotalFiles(uint32 DirClus, uint8 *ExtName, FS_TYPE FsType);
extern void GetLongFileName( int32 DirClus, int32 Index, FS_TYPE FsType, uint16* lfName );
extern uint16 GetGlobeFileNum(uint16 FileNum, uint32 DirClus, uint8 *ExtName, FS_TYPE FsType);

extern void VolumeCreate(uint8 *DirFileName);

extern uint32 FileTell(HANDLE Handle);
/*
********************************************************************************
*
*                         End of FileSeek.h
*
********************************************************************************
*/
#endif
