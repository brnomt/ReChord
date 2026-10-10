/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\Common\audio_track_control.c
Desc    :

Author  : Vincent Hsiung
Date    : 2009-01-08
Notes   :

$Log    :
*
*
*/
/****************************************************************/

#include "../AudioConfig.h"

#include "audio_file_access.h"
#include "audio_main.h"
#include "audio_globals.h"

#include "audio_track_control.h"

//****************************************************************************
// OpenTrack prepares to decode and play the specified track.
//****************************************************************************
_ATTR_AUDIO_TEXT_
int OpenTrack(void)
{
    unsigned long ulSampleRate, ulChannels, ulBitrate;
    unsigned long Err;
    int result = 1;

    //1. Get ID3 Info

    //2. Open Codec , if it contain more infomation than ID3 , use it
    Err = CodecOpen(0, CODEC_OPEN_DECODE);

    // See if we are able to decode this file.
    if (Err != 0)
    {
        return 0;
    }

    // Set the sample rate of the output processing.
    CodecGetSampleRate(&ulSampleRate);
    CodecGetChannels(&ulChannels);
    CodecGetBitrate(&ulBitrate);

    return result;
}

//****************************************************************************
// CreatTrack prepares to encode and creats new file.
//****************************************************************************
_ATTR_AUDIO_TEXT_
int CreateTrack(void)
{
/*
	unsigned long ulSampleRate;
    short *psData;
    long lLength;
    unsigned char a;
    unsigned int count;

    int dir_num = 1;
    unsigned char ucRecInput = REC_GetCurrRecInput();
    unsigned char *pRecFileName = REC_GetRecFileName();

    #ifdef MP3_ENC_INCLUDE
    CurrentCodec    = CODEC_MP3_ENC;
    #endif

    REC_SetRecordVariable();
    gFileHandleOfRecord =(int) fopen("mp3enc.mp3", "wb");

    // Open the codec.
    if(CodecOpen(0, CODEC_OPEN_ENCODE) == 0)
    {
        fclose((FILE*)gFileHandleOfRecord);
        return(0);
    }

    // Enable the input processing.
    CodecGetSampleRate(&ulSampleRate);
    CodecGetCaptureBuffer(&psData, &lLength);

    if(InputEnable(ulSampleRate, psData, lLength) == 0)
    {
        fclose((FILE*)gFileHandleOfRecord);

        return(0);
    }


    // Set the output buffer for the decoder.
    CodecSetBuffer(InputGetOutputBuffer());
*/
    return(1);
}


//****************************************************************************
// CloseTrack closes the currently opened file.
//****************************************************************************
_ATTR_AUDIO_TEXT_
void CloseTrack(void)
{
    CodecClose();
}
