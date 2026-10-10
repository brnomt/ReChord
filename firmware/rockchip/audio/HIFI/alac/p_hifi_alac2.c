
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
* FileName: BBSystem\Codecs\Audio\Decode\alac\pAlac.c
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
#include "audio_file_access.h"
#ifdef HIFI_AlAC_DECODE


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define                                   
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include "audio_globals.h"
#include "alac.h"
#include "hifi_alac_MovFile.h"

#pragma arm section code = "AlacHDecCode", rodata = "AlacHDecCode", rwdata = "AlacHDecData", zidata = "AlacHDecBss"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define     					 
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_READ_  
#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_WRITE_ 
#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_INIT_  
#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_SHELL_  


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define     	     				 
*
*---------------------------------------------------------------------------------------------------------------------
*/

//uint8 gAlacPingPangBuf[2][4096*8];
int Alac_out_Length ;
unsigned int  gAlacPingPangIndex = 0;
extern ALACContext alac_con;
#ifdef A_CORE_DECODE
extern unsigned long SRC_Num_Forehead;
#else
//#define SRC_Num_Forehead 0
#endif
uint8 *gAlacPingPangBuf[2];
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define     	     				 
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern MediaBlock * gmediaBlock;

FILE *alac_file_handle;
extern FILE *pRawFileCache;
int ALAC_filelen ;
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
/*******************************************************************************
** Name: APEDecFunction
** Input:unsigned long ulSubFn, unsigned long ulParam1,
** Return: unsigned long 
** Owner:WJR
** Date: 2014.12.23
** Time: 19:31:54
*******************************************************************************/
READ API unsigned long  HIFI_ALACDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3)
{
    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
		        int ret;
                alac_file_handle=(FILE*)pRawFileCache;
                HIFI_DMA_TO_register();
                ALAC_filelen = RKFIO_FLength(pRawFileCache);
                ret =  Alac_decode_init(alac_file_handle);
			    if(ret < 0)
				{
					return ret;
				}
                Alac_header_parse();  
                Alac_out_Length =alac_con.max_samples_per_frame;
                return 1;
            }
        case SUBFN_CODEC_DECODE:
            {
                int out_size;
        		Alac_out_Length = Alac_frame_decode(&gAlacPingPangBuf[gAlacPingPangIndex][SRC_Num_Forehead] ,&out_size); 
                if(Alac_out_Length <= 0)
                {
                    return 0;
                }
                return 1;
            }           
        case SUBFN_CODEC_GETBUFFER:
            {
                *(unsigned long *)ulParam1 = (unsigned long) &gAlacPingPangBuf[gAlacPingPangIndex][SRC_Num_Forehead];
                *(unsigned long *)ulParam2 = (unsigned long) Alac_out_Length;

				 gAlacPingPangIndex ^= 1;;
                 return(1);
            }


        case SUBFN_CODEC_SEEK:
            {
                 MovIF_SynAudio2Video_h((unsigned int )ulParam1 );
                 return(1);
            }

                    // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                *(int *)ulParam1 = (int ) MovIF_AudioGetCurrentTime_h();
                return(1);
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *SampleRate;
                SampleRate = (unsigned long *)ulParam1;
                *SampleRate = alac_con.samplerate;
                return  1;
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *channels;
                channels = (unsigned long *)ulParam1;
                *channels = alac_con.channels;
                return 1;
            }
        case SUBFN_CODEC_GETBPS:
            {
                unsigned long *bps;
                bps = (unsigned long *)ulParam1;
                *bps = alac_con.sample_size;
                return 1;
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *bitrate;
                 extern int alac_data_size;
                bitrate = (unsigned long *)ulParam1;
                *bitrate = alac_con.average_bitrate ;
                if(*bitrate == 0)
                    *bitrate =((long long )alac_data_size*8*gMovFile_h.audioTimeScale)/(gMovFile_h.uint32AudioDuration);
                *bitrate  += 500;
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *TimeLength; 
                TimeLength = (unsigned long *)ulParam1;
                *TimeLength =  (gMovFile_h.uint32AudioDuration/gMovFile_h.audioTimeScale)*1000;
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
