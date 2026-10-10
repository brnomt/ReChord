
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
/*
********************************************************************************************
*
*                Copyright (c): 2014 - 2014 + 5, WJR
*                             All rights reserved.
*
* FileName: BBSystem\Codecs\Audio\Decode\flac\pFlac.c
* Owner: WJR
* Date: 2014.12.23
* Time: 19:31:51
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.12.23     19:31:51   1.0
********************************************************************************************
*/

#include "SysInclude.h"
#include "audio_main.h"
#ifdef HIFI_FlAC_DECODE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "audio_file_access.h"
#include "audio_globals.h"
#include "flacdec.h"

#pragma arm section code = "FlacHDecCode", rodata = "FlacHDecCode", rwdata = "FlacHDecData", zidata = "FlacHDecBss"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_READ_  
#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_WRITE_ 
#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_INIT_  
#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_SHELL_  


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

//uint8 gFlacPingPangBuf[2][4096*8];
#ifdef A_CORE_DECODE
extern unsigned long SRC_Num_Forehead;
#else
//#define SRC_Num_Forehead 0
#endif
uint8 *gFlacPingPangBuf[2];
unsigned int Flac_out_Length ;  //取采样点
unsigned int gFlacPingPangIndex = 0;
extern FLACStreaminfo s_FLAC_INFO;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern MediaBlock * gmediaBlock;

FILE *flac_file_handle;
extern FILE *pRawFileCache;
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
static unsigned int CheckID3V2Tag(unsigned  char *pucBuffer)
{
    if ((pucBuffer[0] !=    'I') || (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }
    if (pucBuffer[3]    <2  && pucBuffer[3]> 4)
    {
        return(0);
    }
    if (pucBuffer[4]    == 0xff)
    {
        return(0);
    }
    if ((pucBuffer[6] >=    0x80) || (pucBuffer[7] >= 0x80) ||
            (pucBuffer[8] >=    0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }
    return((pucBuffer[6] << 21) | (pucBuffer[7] << 14) |
           (pucBuffer[8] <<  7) |  pucBuffer[9]);
}
int ID3_len = 0;
/*******************************************************************************
** Name: FLACDecFunction
** Input:unsigned long ulSubFn, unsigned long ulParam1,
** Return: unsigned long
** Owner:WJR
** Date: 2014.12.23
** Time: 19:31:54
*******************************************************************************/
READ API unsigned long  HIFI_FLACDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3)
{
    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                 unsigned char flag[20];
                 int ret;
                 flac_file_handle =pRawFileCache;

                RKFIO_FSeek(0,0 ,flac_file_handle);
                RKFIO_FRead(flag,20,flac_file_handle);
                ID3_len = CheckID3V2Tag(flag);
                if (ID3_len == 0)
                {
                    RKFIO_FSeek(0,0 ,flac_file_handle);

                }
                else
                {
                    ID3_len += 10;
                    RKFIO_FSeek(ID3_len,0 ,flac_file_handle);
                }
                HIFI_DMA_TO_register();
                ret = FLAC_decode_init();
                if(ret < 0)
                {
                  return 0;
                }

                Flac_out_Length = s_FLAC_INFO.max_blocksize;
                return 1;
            }
        case SUBFN_CODEC_DECODE:
            {
                int ret;
                ret = FLAC_frame_decode(&gFlacPingPangBuf[gFlacPingPangIndex][SRC_Num_Forehead] ,&Flac_out_Length);
                if(ret <= 0)
                {
                    return 0;
                }
                if(s_FLAC_INFO.samples_decoded >= s_FLAC_INFO.samples )
                {
                  Hifi_Flac_Printf("END\n");
                  return 0;
                }
                return 1;
            }
        case SUBFN_CODEC_GETBUFFER:
            {
                *(unsigned long *)ulParam1 = (unsigned long) &gFlacPingPangBuf[gFlacPingPangIndex][SRC_Num_Forehead];
                *(unsigned long *)ulParam2 = (unsigned long) Flac_out_Length;

                gFlacPingPangIndex ^= 1;
                return 1;
            }


        case SUBFN_CODEC_SEEK:
            {
                hifi_flac_seek( ulParam1/1000); /* seconds */
                return 1;
            }

         // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                extern int metsize;
                unsigned long *curtime;
                curtime = (unsigned long *)ulParam1;

                *curtime = (unsigned long)((long long)s_FLAC_INFO.samples_decoded *1000/ s_FLAC_INFO.samplerate);
                return(1);
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *SampleRate;
                SampleRate = (unsigned long *)ulParam1;
                *SampleRate = s_FLAC_INFO.samplerate;
                return  1;
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *channels;
                channels = (unsigned long *)ulParam1;
                *channels = s_FLAC_INFO.channels;
                return 1;
            }
        case SUBFN_CODEC_GETBPS:
            {
                unsigned long *bps;
                bps = (unsigned long *)ulParam1;
                *bps = s_FLAC_INFO.bps;
                return 1;
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *bitrate;
                bitrate = (unsigned long *)ulParam1;
                *bitrate= s_FLAC_INFO.bitrate + 500;
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *TimeLength;
                TimeLength = (unsigned long *)ulParam1;
                *TimeLength = (long long)s_FLAC_INFO.samples*1000 / s_FLAC_INFO.samplerate;
                return 1;
            }


            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {
              HIFI_DMA_TO_Unregister();
              return 1;
            }
        default:
            {
                // Return a failure.
                return(0);
            }
    }
}

#pragma arm section code
#endif
