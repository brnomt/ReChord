/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Hw_codec.h
*
* Description:
*
* History:      <author>          <time>        <version>
*                               2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_CODEC_H
#define _HW_CODEC_H

#define CODEC_ROCKC         0
#define CODEC_WM8987        1
#define CODEC_ALC5633       2
#define CODEC_CONFIG        CODEC_ROCKC

#define MAX_VOLUME          32

#define RECORD_QUALITY_HIGH     0
#define RECORD_QUALITY_NORMAL   1

#define RECORD_FMT_MP3     0
#define RECORD_FMT_WAV     1

#define RECORD_PREPARE     0
#define RECORD_BEING       1
#define RECORD_PAUSE       2
#define RECORD_ERROR       3
#define RECORD_STOP        4
#define RECORD_NULL        5

typedef enum CodecMode
{
    Codec_DACoutHP,         // for MP3, MP4 playback
    Codec_DACoutLINE,       // for BT/WIFI BOX
    Codec_Line1ADC,         // for Line1 in recording
    Codec_Line1in,          // for Line1 in playback
    Codec_Line2ADC,         // for Line2 in recording
    Codec_Line2in,          // for Line2 in playback
    Codec_MicStero,         // for MIC Stero recording
    Codec_Mic1Mono,         // for MIC 1 mono recording
    Codec_Mic2Mono,         // for MIC 2 mono recording
    Codec_Standby           // for player no application work, like main menu, or exit from FM
}CodecMode_en_t;

typedef enum CodecPower_Status
{
    Codec_Power_on,
    Codec_Power_down,
    Codec_Power_null
}CodecPower_Status_t;

typedef enum
{
    CodecOut_Sel_HP   = ((UINT32)(0x01 << 0)),
    CodecOut_Sel_LINE = ((UINT32)(0x01 << 1)),
}CodecOut_sel_t;

typedef enum CodecFS
{
    FS_8000Hz   = 8000,
    FS_11025Hz  = 11025,
    FS_12KHz    = 12000,
    FS_16KHz    = 16000,
    FS_22050Hz  = 22050,
    FS_24KHz    = 24000,
    FS_32KHz    = 32000,
    FS_44100Hz  = 44100,
    FS_48KHz    = 48000,
    FS_64KHz    = 64000,
    FS_88200Hz   = 88200,
    FS_96KHz    = 96000,
    FS_128KHz   = 128000,
    FS_1764KHz  = 176400,
    FS_192KHz   = 192000,
    FSSTOP      = 192000
} CodecFS_en_t;

typedef enum
{
    CodecMIX_ENABLE,
    CodecMIX_DISABLE,
}CodecMIX_Mode_t;

void Codec_PowerOnInitial(void);
void Codec_SetMode(CodecMode_en_t Codecmode, uint32 datawidth);
void Codec_SetSampleRate(CodecFS_en_t CodecFS);
void Codec_SetVolumet(unsigned int Volume);
void Codec_DACMute(void);
void Codec_DACUnMute(void);
void Codec_DeInitial(void);
void Codec_Suspend(void);

void Codec_ExitMode(CodecMode_en_t Codecmode);
void Codec_ADC_UnMute(void);
void Codec_ADC_MUTE(void);
void ACodec_hp_init();
void ACodec_lineout_init();
void ACodec_ADC2DAC_MIX(CodecMIX_Mode_t MIX_en);
uint8 ACodec_get_over_current_value(void);

/*
********************************************************************************
*
*                         End of Codec.h
*
********************************************************************************
*/

#endif

