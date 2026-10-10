/*
********************************************************************************
*                   Copyright (c) 2008,Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   MusicLrc.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               Chenwei           2009-02-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _MUSIC_LRC_H
#define _MUSIC_LRC_H

#undef  EXT
#ifdef  _IN_MUSIC_LRC
#define EXT 
#else
#define EXT extern
#endif

/*
********************************************************************************
*
*                        Macro Define 
*
********************************************************************************
*/
//section define
#define _ATTR_MUSIC_LRC_INIT_CODE_         
#define _ATTR_MUSIC_LRC_INIT_DATA_         
#define _ATTR_MUSIC_LRC_INIT_BSS_          

#define _ATTR_MUSIC_LRCPLAY_CODE_         
#define _ATTR_MUSIC_LRCPLAY_DATA_         
#define _ATTR_MUSIC_LRCPLAY_BSS_          

#define _ATTR_MUSIC_LRCCOMMON_CODE_         
#define _ATTR_MUSIC_LRCCOMMON_DATA_         
#define _ATTR_MUSIC_LRCCOMMON_BSS_          
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Lrc
#define     LRC_BUFFER_LENGTH               256

//LrcFlag
#define     LRC_FLAG_NULL                   (0x0000)
#define     LRC_FLAG_FILE_END               (0x0001 << 0)

/*
********************************************************************************
*
*                         Struct Define
*
********************************************************************************
*/
typedef union _LRC_BUF
{
    UINT16      ArrayU16[LRC_BUFFER_LENGTH / 2];
    UINT8       ArrayU8 [LRC_BUFFER_LENGTH];
}GBK_LRC_BUF;

/*
********************************************************************************
*
*                         Variable Define
*
********************************************************************************
*/
_ATTR_MUSIC_LRCCOMMON_BSS_    EXT SYS_FILE_INFO   LrcFileInfo ;

_ATTR_MUSIC_LRCCOMMON_BSS_    EXT UINT16          LrcFlag;

_ATTR_MUSIC_LRCCOMMON_BSS_    EXT UINT16          LrcBufferLen;                    //read length of byte
_ATTR_MUSIC_LRCCOMMON_BSS_    EXT UINT16          LrcBufferOffset;

_ATTR_MUSIC_LRCCOMMON_BSS_    EXT UINT8           LrcReadoutBuf[LRC_BUFFER_LENGTH];//get the cache data that is left for process.
_ATTR_MUSIC_LRCCOMMON_BSS_    EXT GBK_LRC_BUF     GBKLrcBuffer;  //after deal with,data to put into this GBK cache buffer it is used to display
_ATTR_MUSIC_LRCCOMMON_BSS_    EXT UINT16          UnicodeLrcBuffer[LRC_BUFFER_LENGTH / 2];//the cache buffer that is used to put UNICODE data. 
_ATTR_MUSIC_LRCCOMMON_BSS_    EXT UINT32          PrevLrcPlayTimeSec;
_ATTR_MUSIC_LRCCOMMON_BSS_    EXT UINT32          NextLrcPlayTimeSec;

/*
********************************************************************************
*
*                         Functon Declaration
*
********************************************************************************
*/

INT16 MusicLrcCheck(UINT16 *pLongFileName);
INT16 MusicLrcInit(void);
void  MusicLrcPlay(INT32 LrcOffsetMS);
INT16 MusicLrcBufferOffsetInc(void);
INT16 MusicLrcSeach(void);
INT16 GetNextLrcTime(UINT32 *TimeSec);
INT16 GetLrcBuffer(UINT32 TimeSec);

/*
********************************************************************************
*
*                         End of MusicLrc.h
*
********************************************************************************
*/
#endif
