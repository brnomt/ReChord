
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
#include "sacd_dsdiff.h"
#ifdef DSDIFF_DEC_INCLUDE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "audio_file_access.h"
#include "audio_globals.h"



#pragma arm section code = "DsdiffDecCode", rodata = "DsdiffDecCode", rwdata = "DsdiffDecData", zidata = "DsdiffDecBss"

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

//#ifdef A_CORE_DECODE
extern unsigned long SRC_Num_Forehead;
//#else
//#define SRC_Num_Forehead 0
//#endif
uint8 *gDffPingPangBuf[2];


unsigned int Dff_out_Length ;  //取采样点
unsigned int gDffPingPangIndex = 0;
extern sacd_dsdiff_t dff_t;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern MediaBlock * gmediaBlock;

FILE *dff_file_handle;
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
static unsigned int CheckID3V2Tag(unsigned	char *pucBuffer)
{
    if ((pucBuffer[0] !=	'I') ||	(pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }
    if (pucBuffer[3]	<2  && pucBuffer[3]> 4)
    {
        return(0);
    }
    if (pucBuffer[4]	== 0xff)
    {
        return(0);
    }
    if ((pucBuffer[6] >=	0x80) || (pucBuffer[7] >= 0x80)	||
            (pucBuffer[8] >=	0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }
    return((pucBuffer[6] <<	21)	| (pucBuffer[7]	<< 14) |
           (pucBuffer[8] <<	 7)	|  pucBuffer[9]);
}
//int ID3_len = 0;
/*******************************************************************************
** Name: FLACDecFunction
** Input:unsigned long ulSubFn, unsigned long ulParam1,
** Return: unsigned long
** Owner:WJR
** Date: 2014.12.23
** Time: 19:31:54
*******************************************************************************/
READ API unsigned long  DSDIFFDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3)
{
    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                 int ret;

                 dff_file_handle = pRawFileCache;

                RKFIO_FSeek(0,0 ,dff_file_handle);
				#if 0
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
				#endif
                ret = dsdiff_decode_init(dff_file_handle);
                if(ret < 0)
                {
                  return 0;
                }
				Dff_out_Length = 1176;
                return 1;
            }
        case SUBFN_CODEC_DECODE:
            {
				long i;
                int ret;
                ret = dsdiff_decode(&gDffPingPangBuf[gDffPingPangIndex][SRC_Num_Forehead] ,&Dff_out_Length);
                //dsd_printf("ret");
                if(ret <= 0)
                {
                    return 0;
                }
                if(dsdiff_decode_end())
                {
                  dsd_printf("END\n");
                  return 0;
                }
                return 1;
            }
        case SUBFN_CODEC_GETBUFFER:
            {
				*(unsigned long *)ulParam1 = (unsigned long) &gDffPingPangBuf[gDffPingPangIndex][SRC_Num_Forehead];
                *(unsigned long *)ulParam2 = (unsigned long) Dff_out_Length;

                gDffPingPangIndex ^= 1;
                return 1;
            }

        case SUBFN_CODEC_SEEK:
            {
                dsdiff_seek( ulParam1/1000); /* seconds */
                return 1;
            }
         // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                extern int metsize;
                unsigned long *curtime;
                curtime = (unsigned long *)ulParam1;

                *curtime = (unsigned long)((long long)dff_t.m_decode_frame_count*1000/75);
				//bb_printf1("DFF curtime:%d,m_decode_frame_count:%d\n",*curtime,dff_t.m_decode_frame_count);
                return(1);
            }
        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *SampleRate;
                SampleRate = (unsigned long *)ulParam1;
                *SampleRate = dff_t.pcm_out_samplerate;
                return  1;
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *channels;
                channels = (unsigned long *)ulParam1;
                *channels = dff_t.pcm_out_channels;
                return 1;
            }
        case SUBFN_CODEC_GETBPS:
            {
                unsigned long *bps;
                bps = (unsigned long *)ulParam1;
                *bps = dff_t.bps;
                return 1;
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *bitrate;
                bitrate = (unsigned long *)ulParam1;
                *bitrate= dff_t.bitrate +500;
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *TimeLength;
                TimeLength = (unsigned long *)ulParam1;
                *TimeLength = (long long)dff_t.m_frame_count * 1000 / 75;

				//Hifi_Dff_Printf("TimeLength:%d\n",*TimeLength);
                return 1;
            }


            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {
              return 1;
            }
        default:
            {
                return(0);
            }
    }
}

#pragma arm section code
#endif

