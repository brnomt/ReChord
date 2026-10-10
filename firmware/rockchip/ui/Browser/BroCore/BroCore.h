/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  BroCore.h
* 
* Description: 
*
* History:      <author>          <time>        <version>       
*               chenfen          2008-3-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _BRO_CORE_H_
#define _BRO_CORE_H_

#undef  EXT
#ifdef _IN_BRO_CORE_
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
//section define

#define _ATTR_BRO_CORE_CODE_         
#define _ATTR_BRO_CORE_DATA_         
#define _ATTR_BRO_CORE_BSS_          


#ifndef PATH_MAXLEN
    #define PATH_MAXLEN                 (3 + (MAX_DIR_DEPTH-1)*12 +1)
#endif


/*
*-------------------------------------------------------------------------------
*  
*                           Struct define
*  
*-------------------------------------------------------------------------------
*/

//数据结构
typedef struct _BroFileInfo
{
    FileType FileType;                                          //file type
    UINT8   *FileExtName;                                       //file extension
    
}BroFileInfo;


typedef struct _FolderIndexInfo
{
    
    UINT16  StartIndex;
    UINT16  CurIndex;
//    int32u  DirClus;

} FolderIndexInfo;


typedef struct _FileListInfo
{
    UINT32 DirClus; 
    UINT32 Index; 
} FileListInfo;


typedef struct _FolderInfoStruct
{
    
    UINT16 TotalItems;                                         //total item number.
    UINT16 TotalFiles;                                         //total file number,
    UINT16  TotalFolders;                                       //total direction number,
    UINT32  DirClus;                                            //direction cluster
    
} FolderInfoStruct;


typedef struct _BrowserFileStruct {
    
    struct _BrowserFileStruct   *pPrev;
	struct _BrowserFileStruct   *pNext;
    struct _FIND_DATA           FileLocInfo;                    //it used to locating of file
    FileType                    FileType;
    FDT                         FileFDT;                         //pointer
    UINT16                      LongFileName[MAX_FILENAME_LEN];
    
} BrowserFileStruct;


typedef struct _BrowserDataStruct
{
    struct _BrowserFileStruct   *pBrowserFile;
    uint16                      CurDiskSelect;                  //current disk select
    uint16                      PreDiskSelect;                  //it used for disk switch
    uint16                      TotleDisk;                      //all disk
    
    FolderInfoStruct            FolderInfo[MAX_DIR_DEPTH];
    UINT16                      CurDeep;                        //direction deepth
    UINT16                      CurStartIndex[MAX_DIR_DEPTH];   //current page start Index
    UINT16                      CurPointerBack[MAX_DIR_DEPTH];  //current page display start Index
    UINT16                      CurPointer;                     //cursor postion
    UINT16                      PrePointer;                     //last cursor postion

    BroFileInfo                 *pSearchFileInfo;   
    
    UINT16                      BrowserTitleId ;            //display titile address
    UINT16                      ScrollBar;                    
    
} BrowserDataStruct;


/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/
/*_ATTR_BRO_CORE_BSS_ */EXT BrowserDataStruct   BrowserData;
_ATTR_BRO_CORE_BSS_     EXT BrowserFileStruct   BrowserFileItem[BROWSER_SCREEN_PER_LINE];
    
#ifdef _IN_BRO_CORE_
_ATTR_BRO_CORE_DATA_    EXT BroFileInfo     gBroFileInfo[] = 
{
    {FileTypeAudio,   (UINT8*)AudioFileExtString    },
    {FileTypeVideo,   (UINT8*)VideoFileExtString    },
    {FileTypePicture, (UINT8*)PictureFileExtString  },
    {FileTypeText,    (UINT8*)TextFileExtString     },
    {FileTypeALL,     (UINT8*)ALLFileExtString      },
};
#else
_ATTR_BRO_CORE_DATA_    EXT BroFileInfo     gBroFileInfo[];
#endif


/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
BroFileInfo *GetSearchFileInfo(FileType FileType);
UINT16 FolderSearchFile(uint8 * ExtName,FolderInfoStruct *pFolderInfo, FS_TYPE FsType);
void FolderGetItemInfo(uint8 * ExtName,BrowserFileStruct*pBrowserFile,
                FolderInfoStruct *pFolderInfo , UINT16 StartItem, FS_TYPE FsType);
BroFileInfo * BroGetFileInfo( uint8 * FileName );
uint8 BroFileDelete(uint32 DirClus, uint8 *DirFileName);

/*
********************************************************************************
*
*                         End of file
*
********************************************************************************
*/
#endif

