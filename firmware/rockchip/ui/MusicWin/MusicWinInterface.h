/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   MusicWinInterface.h
* 
* Description:  the interface link external interface with music UI.
*
* History:      <author>          <time>        <version>       
*                  cw           2009-02-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef MUSICWIN_INTERFACE_H_
#define MUSICWIN_INTERFACE_H_

#include "DialogBox.h"
#include "audio_globals.h"
#include "AudioControl.h"
#include "fsinclude.h"
#include "SysFindFile.h"
#include "Spectrum.h"
#include "BrowserUI.h"

#undef  EXT
#ifdef _IN_MUSICWIN_INTERFACE_
#define EXT
#else
#define EXT extern
#endif

extern  AUDIOCONTROL        AudioPlayInfo;
extern  UINT16              AudioPlayState;
extern  track_info*         pAudioRegKey;
extern  SYS_FILE_INFO       AudioFileInfo ;
extern  UINT16              MusicLongFileName[MAX_FILENAME_LEN];
#ifdef _RK_SPECTRUM_
extern  char                SpectrumOut[];//store the spectrum energy of converted
#endif
/*
*-------------------------------------------------------------------------------
*  
*                           Macro define
*  
*-------------------------------------------------------------------------------
*/

/*
*-------------------------------------------------------------------------------
*  
*                           frimware structure definition
*  
*-------------------------------------------------------------------------------
*/

/*
*-------------------------------------------------------------------------------
*  
*                           variables definition
*  
*-------------------------------------------------------------------------------
*/
#if 1
#define MusicWinVol                     MusicOutputVol
#define CurrentCodecType                CurrentCodec
#define MusicPlayState                  AudioPlayerState
#define MusicCurrentTime                (pAudioRegKey->CurrentTime)
#define MusicRepeatMode                 (gSysConfig.MusicConfig.RepeatMode)
#define MusicPlayOrder                  (pAudioRegKey->PlayOrder)
#define MusicEQType                     (pAudioRegKey->UserEQ.Mode)
#define MusicBitrate                    (pAudioRegKey->bitrate)
#define MusicTotalTime                  (pAudioRegKey->TotalTime)
#define MusicABState                    (AudioPlayInfo.ABRequire)

#define MusicBatteryLevel               gBattery.Batt_Level
#define MusicHoldState                  0
#define MusicCurrFileNum                AudioFileInfo.CurrentFileNum
#define MusicTotalFileNum               AudioFileInfo.TotalFiles

#else

//_ATTR_MUSIC_BSS_  EXT UINT8             OutputVolume;
_ATTR_MUSIC_BSS_  EXT UINT32            CurrentTime;
_ATTR_MUSIC_BSS_  EXT UINT8             MusicRepeatMode;
_ATTR_MUSIC_BSS_  EXT UINT8             MusicPlayOrder;
_ATTR_MUSIC_BSS_  EXT UINT8             MusicEQType;
_ATTR_MUSIC_BSS_  EXT UINT16            MusicBitrate;

_ATTR_MUSIC_BSS_  EXT UINT16            MusicBatteryLevel;
_ATTR_MUSIC_BSS_  EXT UINT16            MusicCurrFileNum;
_ATTR_MUSIC_BSS_  EXT UINT16            MusicTotalFileNum;

#endif
/*
*-------------------------------------------------------------------------------
*  
*                           function declare.
*  
*-------------------------------------------------------------------------------
*/
extern void MusicWinSvcStart(void *pArg);
extern void MusicWinSvcStop(void);
extern BOOLEAN MusicDecodeProc(MSG_ID id,void *msg);

/*
********************************************************************************
*
*                           End of MusicWinInterface.h
*
********************************************************************************
*/
#endif






