/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\pMP3.c
Desc    : MP3 decode flow control

Author  : Vincent Hsiung (xw@rock-chips.com)
Date    : Jan 10 , 2009
Notes   :

$Log    :
*
*
*/
/****************************************************************/

#include "audio_main.h"
#include "audio_globals.h"
#include "audio_file_access.h"

#ifdef MP3_DEC_INCLUDE

#include <stdio.h>
#include <string.h> //for memcpy(),memmove()

_ATTR_MP3DEC_BSS_ short *outbuf[2];
_ATTR_MP3DEC_BSS_
unsigned int MP3_FORMAT_FLAG ; //��8λ->ǰ4λ�洢MPEG ���ͣ�����λ�洢layer����
//mpeg 1  = 1   mpeg2 = 2   mpeg 2.5 = 3;
//layer1  = 1   layer2 =2   layer3 = 3;
extern MediaBlock * gmediaBlock;
/*
*******************************************************************************
    dynamically tune
*******************************************************************************
*/
_ATTR_MP3DEC_BSS_
unsigned int backup_arm_acc;

_ATTR_MP3DEC_TEXT_
void SWITCH_ARM_ACC(void)
{
#if 0
    if ((*((volatile unsigned long*)0x40180000) & 0x00000040)  == 0)
    {
        backup_arm_acc = *((volatile unsigned long*)0x40180008) & (0x08);
        *((volatile unsigned long*)0x40180008) = (0x08 << 16)  | 0;
    }
#endif
}

_ATTR_MP3DEC_TEXT_
void SWITCH_ARM_NOR(void)
{
#if 0
    if ((*((volatile unsigned long*)0x40180000) & 0x00000040)  == 0)
    {
        *((volatile unsigned long*)0x40180008) = ((0x08) << 16)  | backup_arm_acc;
    }
#endif
}

//*************************************************************************************************************//
//the achievement of functions.��
//SUBFN_CODEC_GETNAME  :   get decoder name
//SUBFN_CODEC_GETARTIST:   get artist name.
//SUBFN_CODEC_GETTITLE :   get song title.
//SUBFN_CODEC_GETBITRATE:  get bit rate.
//SUBFN_CODEC_GETSAMPLERATE: get sample rate.
//SUBFN_CODEC_GETCHANNELS: get channel number.
//SUBFN_CODEC_GETLENGTH :  get total play time [unit:ms]
//SUBFN_CODEC_GETTIME  :   get current play time.[unit:ms].note:this time get by timestamp,there may be error if file is been demage..
//SUBFN_CODEC_OPEN_DEC :   open deooder(initialization.)
//SUBFN_CODEC_DECODE   :   deocode.
//SUBFN_CODEC_ENCODE   :   not support.
//SUBFN_CODEC_SEEK     :   location by time directly.[unit:ms]
//SUBFN_CODEC_CLOSE    :   close decoder.
//SUBFN_CODEC_SETBUFFER:   set cache area,point out the position to put save result.
/******************************************************
Name:
Desc:
Param: ulIoctl child function number.
    ulParam1 child function parameter 1.
    ulParam2 child function parameter 2.
    ulParam3 child function parameter 3.
    ulParam4 child function parameter 4.

Return:
Global:
Note:
Author:
Log:
******************************************************/

_ATTR_MP3DEC_TEXT_
unsigned long MP3Function2(unsigned long ulIoctl, unsigned long ulParam1,
                           unsigned long ulParam2, unsigned long ulParam3)
{

    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
        {
            return mp3_open(1);
        }

        case SUBFN_CODEC_GETBUFFER:
        {
            mp3_get_buffer(ulParam1, ulParam2);
            return 1;
        }

        case SUBFN_CODEC_DECODE:
        {
            return mp3_decode();
        }

        case SUBFN_CODEC_GETSAMPLERATE:
        {
            *(int *)ulParam1 = mp3_get_samplerate();
            return (1);
        }

        case SUBFN_CODEC_GETCHANNELS:
        {
            *(int *)ulParam1 = mp3_get_channels();
            return (1);
        }

        case SUBFN_CODEC_GETBITRATE:
        {
            *(int *)ulParam1 = mp3_get_bitrate();
            return (1);
        }

        case SUBFN_CODEC_GETLENGTH:
        {
            *(int *)ulParam1 = mp3_get_length();
            return 1;
        }

        case SUBFN_CODEC_GETTIME:
        {
            unsigned long temp, temp1;
            unsigned int sample, timexxx;

            temp = mp3_get_timepos();
            timexxx = (unsigned long)(((unsigned long long) temp) * 1000 / mp3_get_samplerate());
            *(unsigned long *)ulParam1 = timexxx;
            return 1;
        }

        case SUBFN_CODEC_SEEK:
        {
            mp3_seek(ulParam1);
            return 1;
        }

        case SUBFN_CODEC_CLOSE:
        {
            mp3_close();
            return 1;
        }

        case SUBFN_CODEC_GETBPS:
        {
            *(int *)ulParam1 = mp3_get_bps();
            return 1;
        }

        default:
        {
            return 0;
        }
    }

    return -1;
}

#endif
