/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   AudioControl.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               ZS      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _AUDIO_CONTROL_H_
#define _AUDIO_CONTROL_H_

#include "fsinclude.h"
#include "audio_globals.h"
#include "HoldonPlay.h"

#undef  EXT
#ifdef _IN_AUDIO_CONTROL_
#define EXT
#else
#define EXT extern
#endif

//#define SINATAB   //output sine wave.
//#define NOISR

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/

//section define
#define _ATTR_AUDIO_TEXT_     
#define _ATTR_AUDIO_DATA_     
#define _ATTR_AUDIO_BSS_      
#define _ATTR_AUDIO_INIT_TEXT_  
//
#define AUDIO_DMACHANNEL_IIS    (DMA_CHN_MAX - 1)

#define AUDIOCONSOLE            1
#define AUDIOTRYTIME            10

#define AUDIO_STEPLEN            (2000)//ms the step of FFW.

#define AUDIO_STOP_NORMAL       0
#define AUDIO_STOP_FORCE        1

#define Audio_Direct_BackWord   (0<<8)
#define Audio_Direct_Forword    (1<<8)


#define AUDIONEXTFILE           1
#define AUDIOCURRENTFILE        0
#define AUDIOPREFILE            -1

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
typedef struct
{
    uint32 ABRequire;
    uint32 AudioABStart;
    uint32 AudioABEnd;
    uint32 PlayDirect;
    uint8 playVolume;
    uint8 VolumeCnt;

}AUDIOCONTROL;

//define the play status.
typedef enum
{
    AUDIO_STATE_PLAY,
    AUDIO_STATE_FFD,
    AUDIO_STATE_FFW,
    AUDIO_STATE_PAUSE,
    AUDIO_STATE_STOP,

    //PAGE
    AUDIO_STATE_PLAY_FFD,
    AUDIO_STATE_PLAY_FFW,
    AUDIO_STATE_PAUSE_FFD,
    AUDIO_STATE_PAUSE_FFW,
    AUDIO_STATE_FFD_PLAY,
    AUDIO_STATE_FFW_PLAY

}AUDIOSTATE;


//reapeat mode
typedef enum
{
    AUDIO_FOLDER_ONCE,    //once directory.
    AUIDO_FOLDER_REPEAT,  //directory cycle
    AUDIO_REPEAT,         //repeat one song
    AUDIO_ALLONCE,        //repeat once all song.
    AUDIO_ALLREPEAT,      //cycle play all song.
    AUDIO_REPEAT1,
    AUDIO_ONCE,           //repeat once one song.
    AUDIO_TRY              //lyrics

}AUDIOREPEATMODE;



//define A-B status.
typedef enum
{
    AUDIO_AB_NULL,
    AUDIO_AB_A,
    AUDIO_AB_PLAY

}AUDIOABSTATE;


//EQ mode define.
//need be keep consisten with modes define in file effect.h.
typedef enum
{
    AUDIO_NORMAL,
    AUDIO_MSEQ,
    AUDIO_ROCK,
    AUDIO_POP,
    AUDIO_CLASSIC,
    AUDIO_BASS,
    AUDIO_JAZZ,
    AUDIO_USER

}AUDIOEQTYPE;

//audio effect define of user.
typedef enum
{
    EQ_N12DB,
    EQ_N8DB,
    EQ_N4DB,
    EQ_0DB,
    EQ_4DB,
    EQ_8DB,
    EQ_12DB

}USER_EQ;

//display mode define.
typedef enum
{
    AUDIO_ID3,
    AUDIO_LRC,
    AUDIO_FREQCHART,
    AUDIO_CLOSE

}AUDIODISPMODE;

typedef  struct _eqfilter {
long a[5];
long b[2][4];
}eqfilter;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/

//�ⲿ�����ӿ�
#define     MusicOutputVol                  gSysConfig.OutputVolume
extern int              CurrentCodec;
extern int              gisaacorm4a;

_ATTR_AUDIO_BSS_    EXT int   				AudioStopMode;   //normal stop or Force//Audio_Stop_Normal or Audio_Stop_Force
_ATTR_AUDIO_BSS_    EXT UINT16              MusicLongFileName[MAX_FILENAME_LEN];
_ATTR_AUDIO_BSS_    EXT AudioInOut_Type     AudioIOBuf;
_ATTR_AUDIO_BSS_    EXT track_info*         pAudioRegKey;
_ATTR_AUDIO_BSS_    EXT AUDIOCONTROL        AudioPlayInfo;
_ATTR_AUDIO_BSS_    EXT uint32              AudioPlayFileNum;
_ATTR_AUDIO_BSS_    EXT uint32              AudioPlayFileNumSave;
_ATTR_AUDIO_BSS_    EXT BOOLEAN             MusicUIState; //audio interface UI status.0 mean display audio interface.1 mean not.
_ATTR_AUDIO_BSS_    EXT UINT16              AudioErrorFileCount;//sen #20090803#1 all audio file is not support

_ATTR_AUDIO_BSS_    EXT UINT32              AudioErrorFrameNum;
_ATTR_AUDIO_BSS_    EXT UINT32              AudioDecodeCntBak;
_ATTR_AUDIO_BSS_    EXT UINT32              AudioEndFade;
//the offset value of audio file seek.
_ATTR_AUDIO_BSS_    EXT UINT32              AudioFileSeekOffset;
_ATTR_AUDIO_BSS_    EXT BOOL                IsEQNOR;//turn to NOR when user EQ's DB value turn to 0.

_ATTR_AUDIO_BSS_    EXT UINT32              oldtick;  //used for musicserver
_ATTR_AUDIO_BSS_    EXT UINT32              gAudioPlayTime;
_ATTR_AUDIO_BSS_    EXT UINT32              CurrentTimeSecBk;
_ATTR_AUDIO_BSS_    EXT UINT32              MusicStepTime;

_ATTR_AUDIO_BSS_    EXT uint32              audio_have_album_pic;
_ATTR_AUDIO_BSS_    EXT uint32              audio_dec_album_done;
_ATTR_AUDIO_BSS_    EXT uint32              audio_dec_album_save;
_ATTR_AUDIO_BSS_    EXT FILE                *AudioAlbumHandle;

#ifdef _RK_SPECTRUM_
#define SPECTRUM_LINE_M      10     // 10��Ƶ����
#define SpectrumLoopTime     3
_ATTR_AUDIO_BSS_    EXT char               SpectrumOut[SPECTRUM_LINE_M];//���ת�������M�����ߵ�����
_ATTR_AUDIO_BSS_    EXT int                SpectrumLoop;
_ATTR_AUDIO_BSS_    EXT int                SpectrumCnt;
#endif

#ifdef _IN_AUDIO_CONTROL_
_ATTR_AUDIO_DATA_ EXT INT32      MusicNextFile = 1;
_ATTR_AUDIO_DATA_ EXT track_info gRegAudioConfig =
{
    1,                          //unsigned long TotalTime;    total play time of song.
    0,                          //unsigned long CurrentTime;  current time of song play.
    1,                          //unsigned long Bitrate;      the bit rate of song.
    1,                          //unsigned long Samplerate;   sample rate of song.
    1,                          //channels
    1,                          //total_length
    0,                          //cur_pos
    AUDIO_ALLONCE,            //unsigned long RepeatMode;  cycle mode
    AUDIO_INTURN,               //unsigned long PlayOrder;   play mode
    FIND_FILE_RANGE_ALL,        //directory file or all file.
    //eq
    {
        EQ_NOR,
        12,
		#ifdef _RK_EQ_
        {EQ_0DB, EQ_0DB, EQ_0DB, EQ_0DB, EQ_0DB}
		#endif
    },//tEffect       UserEQ; �û�EQ

    0,                          //artist
    0,                          //title
    0,                          //album
};
#else
_ATTR_AUDIO_DATA_ EXT INT32      MusicNextFile;
_ATTR_AUDIO_DATA_ EXT track_info gRegAudioConfig;
#endif


/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
void Audio_I2SInit(void);
void AudioHWInit(void);

BOOLEAN AudioIsPlaying(void);
BOOLEAN AudioIsPause(void);

void AudioSetVolume(void);
void AudioSetEQ(int vol);

UINT8 GetFileType(UINT8 *pBuffer, UINT8 *pStr);
void AudioCodecOverLay(void);
void AudioGetNextNum(UINT16 Direct);
BOOLEAN AudioGetNextMusic(UINT32 msg);
void AudioStart(void);
BOOLEAN AudioPause(void);
BOOLEAN AudioFFW(unsigned long StepLen);
BOOLEAN AudioFFD(unsigned long StepLen);
BOOLEAN AudioStop(UINT16 ReqType);
BOOLEAN AudioResume(void);
BOOLEAN AudioSetPosition(unsigned long Desti_Posi);
void AudioSetAB_A(void);
void AudioSetAB(void);
void AudioABStop(void);
BOOLEAN AudioDecodeProc(MSG_ID id,void * msg);
void MusicInit(void *pArg);
UINT32 MusicService(void);
void MusicDeInit(void);
void SetMusicUIState(bool State);
void AudioSysFindFile(UINT32 FindDirection,UINT16 FindRange,UINT16 FindOrder);

#ifdef MP3_DEC_INCLUDE
extern void AcceleratorHWInit(void);
extern void dma1_handler();
extern void imdct36_handler();
extern void synth_handler();
extern volatile int synth_hw_busy;

#endif

/*
--------------------------------------------------------------------------------

  Description:  audio thread definition.

--------------------------------------------------------------------------------
*/
#ifdef _IN_AUDIO_CONTROL_
_ATTR_AUDIO_DATA_ EXT THREAD MusicThread = {

    NULL,
    NULL,

    MusicInit,
    MusicService,
    MusicDeInit,

    NULL
};
#else
_ATTR_AUDIO_DATA_ EXT THREAD MusicThread;
#endif

/*
********************************************************************************
*
*                         End of AudioControl.h
*
********************************************************************************
*/

#endif
