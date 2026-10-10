
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
//#include "sacd_dsdiff.h"
#include "sacd_dsf.h"
#ifdef DSF_DEC_INCLUDE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "audio_file_access.h"
#include "audio_globals.h"



#pragma arm section code = "DsfDecCode", rodata = "DsfDecCode", rwdata = "DsfDecData", zidata = "DsfDecBss"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

//#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_READ_  
//#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_WRITE_ 
//#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_INIT_  
//#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_SHELL_  


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
uint8 *gDsfPingPangBuf[2];


unsigned int Dsf_out_Length ;  //取采样点
unsigned int gDsfPingPangIndex = 0;
extern sacd_dsf_t dsf_t;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern MediaBlock * gmediaBlock;

FILE *dsf_file_handle;
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
#if 0
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
#endif
/*******************************************************************************
** Name: DsfDecFunction
** Input:unsigned long ulSubFn, unsigned long ulParam1,
** Return: unsigned long
** Owner:WJR
** Date: 2014.12.23
** Time: 19:31:54
*******************************************************************************/
READ API unsigned long  DSFDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3)
{

    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {

                 int ret;
                 //dsf_printf("#########eee");
                 dsf_file_handle = pRawFileCache;

                RKFIO_FSeek(0,0 ,dsf_file_handle);
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
                ret = dsf_decode_init(dsf_file_handle);
                if(ret < 0)
                {
                  return 0;
                }
				Dsf_out_Length = 1176;
                return 1;
            }
        case SUBFN_CODEC_DECODE:
            {
				long i;
                int ret;
                ret = dsf_decode(&gDsfPingPangBuf[gDsfPingPangIndex][SRC_Num_Forehead] ,&Dsf_out_Length);
                //dsd_printf("ret");
                if(ret <= 0)
                {
                    return 0;
                }
                if(dsf_decode_end())
                {
                  dsf_printf("END\n");
                  return 0;
                }
                return 1;
            }
        case SUBFN_CODEC_GETBUFFER:
            {
				*(unsigned long *)ulParam1 = (unsigned long) &gDsfPingPangBuf[gDsfPingPangIndex][SRC_Num_Forehead];
                *(unsigned long *)ulParam2 = (unsigned long) Dsf_out_Length;

                gDsfPingPangIndex ^= 1;
                return 1;
            }

        case SUBFN_CODEC_SEEK:
            {
                dsf_seek( ulParam1/1000); /* seconds */
                return 1;
            }
         // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                extern int metsize;
                unsigned long *curtime;

                curtime = (unsigned long *)ulParam1;
               // *curtime = (unsigned long)((long long)dsf_t.m_sampleCount/dsf_t.m_samplerate);

               *curtime = (unsigned long)( dsf_t.m_decode_frame_count * dsf_t.frameLength) ;
				//bb_printf1("DFF curtime:%d,m_decode_frame_count:%d\n",*curtime,dff_t.m_decode_frame_count);
                return(1);
            }
        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *SampleRate;
                SampleRate = (unsigned long *)ulParam1;
                *SampleRate = dsf_t.pcm_out_samplerate;
                return  1;
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *channels;
                channels = (unsigned long *)ulParam1;
                *channels = dsf_t.pcm_out_channels;
                return 1;
            }
        case SUBFN_CODEC_GETBPS:
            {
                unsigned long *bps;
                bps = (unsigned long *)ulParam1;
                *bps = dsf_t.bps;
                return 1;
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *bitrate;
                bitrate = (unsigned long *)ulParam1;
                *bitrate= dsf_t.bitrate +500;
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *TimeLength;

                TimeLength = (unsigned long *)ulParam1;
                //*TimeLength = (long long)dsf_t.m_frame_count * 1000 / 75;



                *TimeLength = (unsigned long)(dsf_t.frameLength * dsf_t.m_frame_count);

                //dsf_printf("TimeLength:%lu\n",*TimeLength);
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

