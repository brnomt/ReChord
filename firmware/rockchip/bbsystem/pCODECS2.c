/*
********************************************************************************************
*
*                Copyright (c): 2014 - 2014 + 5, WJR
*                             All rights reserved.
*
* FileName: BBSystem\pCODECS.c
* Owner: WJR
* Date: 2014.12.23
* Time: 16:24:28
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.12.23     16:24:28   1.0
********************************************************************************************
*/

#include "SysInclude.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include "audio_globals.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BBSYSTEM_PCODECS2_READ_  
#define _BBSYSTEM_PCODECS2_WRITE_ 
#define _BBSYSTEM_PCODECS2_INIT_  
#define _BBSYSTEM_PCODECS2_SHELL_  


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

int CurrentCodec2;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_BBSYSTEM_PCODECS2_READ_
 static unsigned long (*CodecPFn2[NUMCODECS])(unsigned long ulSubFn,
                                                   unsigned long ulParam1,
                                                   unsigned long ulParam2,
                                                   unsigned long ulParam3) =
{
    #ifdef MP3_DEC_INCLUDE
    #ifdef MP3_DEC_INCLUDE2
    MP3Function2,
    #else
    0,
    #endif
    #else
    0,
    #endif

    0,      //CODEC_REV_DEC

    #ifdef AAC_DEC_INCLUDE
    #ifdef AAC_DEC_INCLUDE2
    AACDecFunction2,
    #else
    0,
    #endif
    #endif

    #ifdef WAV_DEC_INCLUDE
    #ifdef WAV_DEC_INCLUDE2
    PCMFunction2,
    #else
    0,
    #endif
    #endif

    #ifdef APE_DEC_INCLUDE
    #ifdef APE_DEC_INCLUDE2
    APEDecFunction2,
    #else
    0,
    #endif
    #endif

    #ifdef FLAC_DEC_INCLUDE
    #ifdef FLAC_DEC_INCLUDE2
    FLACDecFunction2,
    #else
    0,
    #endif
    #endif

    #ifdef OGG_DEC_INCLUDE
    #ifdef OGG_DEC_INCLUDE2
    OGGDecFunction2,
    #else
    0,
    #endif
    #endif

    #ifdef HIFI_APE_DECODE
    #ifdef HIFI_APE_DECODE2
    HIFI_APEDecFunction2,
    #else
    0,
    #endif
    #endif

    #ifdef HIFI_FlAC_DECODE
    #ifdef HIFI_FlAC_DECODE2
    HIFI_FLACDecFunction2,
    #else
    0,
    #endif
    #endif

    #ifdef HIFI_AlAC_DECODE
    #ifdef HIFI_AlAC_DECODE2
    HIFI_ALACDecFunction2,
    #else
    0,
    #endif
    #endif

    #ifdef DSDIFF_DEC_INCLUDE
    #ifdef DSDIFF_DEC_INCLUDE2
    DSDIFFDecFunction2,
    #else
    0,
    #endif
    #endif

    #ifdef DSF_DEC_INCLUDE
    #ifdef DSF_DEC_INCLUDE2
    DSFDecFunction2,
    #else
    0,
    #endif
    #endif
};

_BBSYSTEM_PCODECS2_READ_
unsigned long CodecOpen2(unsigned long ulCodec, unsigned long ulFlags)
{
    unsigned long ulRet;

    if(CurrentCodec2 == 0xff)
        return -1;
    // Pass the open request to the entry point for the codec.
    ulRet = (CodecPFn2[CurrentCodec2])(SUBFN_CODEC_OPEN_DEC, 0, 0, 0);

    // Return the result to the caller.
    return(ulRet);
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecDecode2(void)
{
    if(CurrentCodec2 == 0xff)
        return -1;
    return((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_DECODE, 0, 0, 0));
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecSeek2(unsigned long ulTime, unsigned long ulSeekType)
{
    // Pass the seek request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentCodec2 == 0xff)
        return -1;

    ret=((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_SEEK, ulTime, ulSeekType,0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetTime2(unsigned long *pulTime)
{
    unsigned long ret;

    if(CurrentCodec2 == 0xff)
        return -1;
    // Pass the time request to the entry point for the specified codec.

    ret=((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_GETTIME, (unsigned long)pulTime, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetBitrate2(unsigned long *pulBitrate)
{
    // Pass the bitrate request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentCodec2 == 0xff)
        return -1;
    ret=((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_GETBITRATE, (unsigned long)pulBitrate, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetSampleRate2(unsigned long *pulSampleRate)
{
    // Pass the sample rate request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentCodec2 == 0xff)
        return -1;

    ret=((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_GETSAMPLERATE, (unsigned long)pulSampleRate, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetChannels2(unsigned long *pulChannels)
{
    // Pass the channels request to the entry point for the specified codec.
    unsigned long ret;

    if(CurrentCodec2 == 0xff)
        return -1;

    ret=((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_GETCHANNELS, (unsigned long)pulChannels, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetLength2(unsigned long *pulLength)
{
    // Pass the length request to the entry point for the specified codec.
    unsigned long ret;

    if(CurrentCodec2 == 0xff)
        return -1;
    ret=((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_GETLENGTH, (unsigned long)pulLength, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecSetBuffer2(short* psBuffer)
{
    // Pass the set buffer request to the entry point for the specified codec.
    unsigned long ret;
    ret=((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_SETBUFFER, (unsigned long)psBuffer, 0, 0));
    return ret;
}

_BBSYSTEM_PCODECS2_READ_
unsigned long CodecClose2(void)
{
    unsigned long ulRet;

    if(CurrentCodec2 == 0xff)
        return -1;
    // Pass the close request to the entry point for the specified codec.
    ulRet = (CodecPFn2[CurrentCodec2])(SUBFN_CODEC_CLOSE, CODEC_OPEN_ENCODE, 0, 0);

    return(ulRet);
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetCaptureBuffer2(short *ppsBuffer, long *plLength)
{
    // Pass the get capture buffer request to the entry point for the specified
    // codec.
    if(CurrentCodec2 == 0xff)
        return -1;

    return((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_GETBUFFER,(unsigned long)ppsBuffer,
            (unsigned long)plLength, 0));
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecEncode2(void)
{
    // Pass the encode request to the entry point for the specified codec.
    return((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_ENCODE, 0, 0, 0));
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetBps2(long *audioBps)
{
    if(CurrentCodec2 == 0xff)
        return -1;
    // Pass the encode request to the entry point for the specified codec.
    return((CodecPFn2[CurrentCodec2])(SUBFN_CODEC_GETBPS, (unsigned long)audioBps, 0, 0));
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

