
#ifndef _SYS_FINDFILE_H
#define _SYS_FINDFILE_H

#include "filesys/FDT.h"   /* FIND_DATA (search cursor) - explicit path: include/ has a shadow FDT.h */
#include "fsinclude.h"

#undef  EXT
#ifdef  IN_SYS_FINDFILE
#define EXT
#else
#define EXT extern
#endif

/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                      */
/*                                                                            */
/******************************************************************************/
//Service Section define
#define     _ATTR_SYS_FINDFILE_TEXT_        
#define     _ATTR_SYS_FINDFILE_DATA_        
#define     _ATTR_SYS_FINDFILE_BSS_         

//------------------------------------------------------------------------------


/******************************************************************************/
/*                                                                            */
/*                          Struct Define                                     */
/*                                                                            */
/******************************************************************************/
//����˳����
typedef enum
{
    AUDIO_INTURN,
    AUDIO_RAND

}AUDIOPLAYMODE;

//Play Range define
#define     FIND_FILE_RANGE_DIR            2// once direction
#define     FIND_FILE_RANGE_ALL            3// cycle in direction

typedef struct
{
    UINT16      TotalFiles;     //the total number of current direction/disk
    UINT16      CurrentFileNum;

    UINT16      TotalFileNum;

    UINT16      PlayedFileNum;

    UINT16      DiskTotalFiles; //he total number of current disk

    UINT16      Range;//cycle direction or once direction
    UINT16      PlayOrder; // random or order

    uint8      *pExtStr;       //file type

    FDT         Fdt;
    FIND_DATA   FindData;

    UINT16      RandomBuffer[16];
    UINT8       Path[3 + (MAX_DIR_DEPTH - 1) * 12 + 1];

    UINT16  ucSelPlayType;  // add by phc


    UINT16  ucCurDeep; // add by phc
    UINT32  ulFullInfoSectorAddr;  // add by phc
    UINT32  ulSortInfoSectorAddr;  // add by phc
    UINT16  uiCurId[MAX_DIR_DEPTH]; // add by phc

    UINT16 uiBaseSortId[4]; // add by phc

} SYS_FILE_INFO;

/******************************************************************************/
/*                                                                            */
/*                          Variable Define                                   */
/*                                                                            */
/******************************************************************************/
_ATTR_SYS_BSS_  EXT SYS_FILE_INFO   AudioFileInfo ;
_ATTR_SYS_BSS_  EXT SYS_FILE_INFO   VideoFileInfo ;
_ATTR_SYS_BSS_  EXT SYS_FILE_INFO   PicSysFileInfo ;

_ATTR_SYS_BSS_  EXT uint32 MusicFileNumBack;
_ATTR_SYS_BSS_  EXT uint32 PrevLastNumBack;
_ATTR_SYS_BSS_  EXT uint32 ShuffleNextFlag;
_ATTR_SYS_BSS_  EXT uint32 ShufflePrevFlag;
_ATTR_SYS_BSS_  EXT int32  ShuffleNum;

/******************************************************************************/
/*                                                                            */
/*                         Function Declare                                   */
/*                                                                            */
/******************************************************************************/
//dgl audio UINT16 SysCheckTotalFileNum(uint8 *pExtStr);

INT16 SysFindFileInit(SYS_FILE_INFO *pSysFileInfo,UINT16 GlobalFileNum,UINT16 FindFileRange,UINT16 PlayMode, uint8 *pExtStr);


INT16 SysFindFileExt(SYS_FILE_INFO *pSysFileInfo, INT16 Offset);

INT16 SysFindFile(SYS_FILE_INFO *pSysFileInfo,    INT16 Offset);

void SysFindFileModify(SYS_FILE_INFO *pSysFileInfo, UINT16       Range,UINT16 Mode);

void GetPlayInfo(UINT16 PlayMode, UINT16 *pRange, UINT16 *pRepMode);
void GetDirPath(UINT8 *pPath);
void CreatRandomFileList(UINT16 CurrentFileNum,UINT16 TotalFileNum,UINT16 *pBuffer);


//------------------------------------------------------------------------------

#endif
//******************************************************************************

