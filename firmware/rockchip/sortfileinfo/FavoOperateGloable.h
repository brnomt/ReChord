/*
********************************************************************************
*          Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All Rights Reserved
*                                    V1.00
* FileName   : FavoOperateGloable.h
* Author     : phc
* Description:
* History    :
*           <author>        <time>     <version>       <desc>
*            phc
*
********************************************************************************
*/
#ifndef _FAVORITEOPERATE_H
#define _FAVORITEOPERATE_H



#undef  EXT
#ifdef _IN_FAVORITE_OPERATE_
#define EXT
#else
#define EXT     extern
#endif

#define BUFFER_CHAR_NUM      512
//EXT MUSICBROWSER_DATA UINT8 gBroFavoSaveBuffer[2048];
//EXT  _ATTR_MEDIABROWIN_BSS_ UINT8 gBroFavoSaveBuffer[FLASH_PAGE_SIZE];
//EXT  _ATTR_MEDIABROWIN_BSS_ UINT8 gBroFavoSaveBuffer1[BUFFER_CHAR_NUM];

//EXT  _ATTR_MEDIABROWIN_BSS_ UINT8 gBroFavoSaveBuffer[500];
//EXT  _ATTR_MEDIABROWIN_BSS_ UINT8 gBroFavoSaveBuffer1[400];

#define   _FAVORESET_CODE_    
#define   _FAVORESET_DATA_    
#define   _FAVORESET_BSS_    

#ifdef _IN_FAVORESET
_FAVORESET_BSS_ UINT8 gFavoResetBuffer[MEDIAINFO_PAGE_SIZE];
_FAVORESET_BSS_ UINT8 gFavoResetBuffer1[MEDIAINFO_PAGE_SIZE];
#else
extern  _FAVORESET_BSS_ UINT8 gFavoResetBuffer[MEDIAINFO_PAGE_SIZE];
extern  _FAVORESET_BSS_ UINT8 gFavoResetBuffer1[MEDIAINFO_PAGE_SIZE];
#endif

EXT UINT16 FindFileByShortPath(FDT *Rt, UINT8 *PathBuffer, UINT8 *ShortFileName);
EXT void BroFavoSaveBufferInit();
EXT void FavoBlockInit(void);
EXT void ChangeFavoSaveBlock(void);
EXT UINT16 SaveGroupFavoSong(UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiBaseSortId, UINT16 uiSaveItemNum);
//EXT UINT16 GetFavoInfo(FDT *Rt, UINT16 uiFavoID);
EXT void RemoveOneFavoSong(UINT16 uiDelID);
EXT void RemoveAllFavoSong(void);
//------------------------------------------------------------------------------
#endif
//******************************************************************************


