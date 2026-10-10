/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   FileInfo.h
*
* Description:
*
* History:      <author>          <time>        <version>
*                               2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _FILEINFO_H
#define _FILEINFO_H

#undef  EXT
#ifdef _IN_FILEINFOSAVE_
#define EXT
#else
#define EXT extern
#endif


/*
*-------------------------------------------------------------------------------
*
*                           Include File
*
*-------------------------------------------------------------------------------
*/

#define   _FILE_INFO_SAVE_CODE_    

#define   _FILE_INFO_SAVE_DATA_    
#define   _FILE_INFO_SAVE_BSS_     


#define   _FILE_INFO_SORT_CODE_      
#define   _FILE_INFO_SORT_DATA_      
#define   _FILE_INFO_SORT_BSS_       


extern void SearchAndSaveMusicInfo(void);
extern void SortUpdateFun(MEDIALIB_CONFIG * Sysfilenum ,UINT32 MediaInfoAddress);

/*
********************************************************************************
*
*                         End of FileInfo.h
*
********************************************************************************
*/
#endif


