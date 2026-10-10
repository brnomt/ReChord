/* Copyright (C) 2013 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File	: \Audio\pOGG.c
Desc	: floe chart of FLAC decode

Author	: wu jiangrui (wjr@rock-chips.com)
Date	: Aug 24 , 2013
Notes	:

$Log	:
*
*
*/
/****************************************************************/

#include "audio_main.h"
#include "audio_globals.h"
#include "audio_file_access.h"


#include "lib/ivorbiscodec.h"
#include "lib/ivorbisfile.h"

#ifdef OGG_DEC_INCLUDE
#include "audio_globals.h"
#include "audio_file_access.h"
#include "OsInclude.h"
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
//SUBFN_CODEC_SEEK	   :   location by time directly.[unit:ms]
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
#pragma arm section code = "OggDecCode", rodata = "OggDecCode", rwdata = "OggDecData", zidata = "OggDecBss"

static FILE *rawfile;
static OggVorbis_File vf;
#define OUT_SIZE 2048
//char pcmout[2][OUT_SIZE];//4096 /* take 4k out of the data segment, not the stack */
short *OggPcmOut[2];
extern  unsigned long SRC_Num_Forehead;

int select = 0;
int out_size = OUT_SIZE;
int i = 0;
signed char  f_open_dec_table;
signed char f_pcm_out;
signed char f_dec_table;
char dec_buf[OUT_SIZE*2];
int pos = 0;
//int ogg_cnt = 0;


unsigned long OGGDecFunction2(unsigned long ulIoctl, unsigned long ulParam1,
                              unsigned long ulParam2, unsigned long ulParam3)
{
    int current_section = 0;
    char **ptr;
    vorbis_info *vi	;
    int ret;
    int wr_ret;
    int i;
    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                rawfile=(FILE*)pRawFileCache;

                RKFIO_FSeek(0, SEEK_SET, rawfile);

                if (ov_open(rawfile, &vf, NULL, 0) < 0)
                {
                    return 0;
                }
                //ogg_cnt =1;
                return 1;
            }

        case SUBFN_CODEC_GETBUFFER:
            {
                *(unsigned long*)ulParam1 = (unsigned long)(&OggPcmOut[select][SRC_Num_Forehead]);

                *(unsigned long *)ulParam2 = (unsigned long) (OUT_SIZE/4); // char ת��long

                select^=1;

                return 1;
            }

        case SUBFN_CODEC_DECODE:
            {
                int tick_end;

                if (vf.vi.channels == 2)
                {
                    char *output = (char*)&OggPcmOut[select][SRC_Num_Forehead];
                    while (pos < OUT_SIZE)
                    {
                        //ret=ov_read(&vf,&pcmout[select][pos],OUT_SIZE-pos,&current_section);
                        ret=ov_read(&vf,&output[pos],OUT_SIZE-pos,&current_section);

                        if (ret <= 0)
                        {
                            //FileClose(f_pcm_out);
                            return 0;
                        }
                        else
                        {
                            pos += ret;
                        }
                    }
                    pos -= OUT_SIZE;
                }
                else
                {
                    while (pos < OUT_SIZE/2)
                    {
                        ret=ov_read(&vf,&dec_buf[pos],OUT_SIZE/2-pos,&current_section);
                        if (ret <= 0)
                        {
                            //FileClose(f_pcm_out);
                            return 0;
                        }
                        else
                        {
                            pos += ret;
                        }
                    }
                    {
                        short * input;
                        short *output;

                        output = (short *)&OggPcmOut[select][SRC_Num_Forehead];
                        input = (short *)dec_buf;
                        for (i=0;i<OUT_SIZE/4;i++)
                        {
                            output[2*i] = input[i];
                            output[2*i+1] = input[i];
                        }
                    }
                    pos -= OUT_SIZE/2;
                }
                // if(ogg_cnt ==1)
                {
                    //ogg_cnt =0;
                    //tick_end= GetSysTick();
                    //printf("��ʼ %d ",tick_end);
                }
                return 1;
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = vf.vi.rate;
                return(1);
            }
        case SUBFN_CODEC_GETBPS:
            {
                *(int *)ulParam1 = 16;
                return(1);
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = vf.vi.channels;
                return(1);
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                *(long *)ulParam1 = vf.vi.bitrate_nominal;

                return(1);
            }

        case SUBFN_CODEC_GETLENGTH:
            {
                *(long *)ulParam1 = ov_time_total(&vf,-1);
                return 1;
            }

        case SUBFN_CODEC_GETTIME:
            {
                *(long *)ulParam1 = ov_time_tell(&vf);
                return 1;
            }

        case SUBFN_CODEC_SEEK:
            {
                ov_time_seek_page(&vf, ulParam1);
                return 1;
            }

        case SUBFN_CODEC_CLOSE:
            {
                //ov_clear(&vf);
                //int tick_start = GetSysTick();
                //printf("���� %d ",tick_start);
                //ogg_cnt = 0;
                return 1;
            }

        default:
            {
                return 0;
            }
    }
}

#endif

