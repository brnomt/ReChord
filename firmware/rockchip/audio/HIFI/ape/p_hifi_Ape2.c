
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
* FileName: BBSystem\Codecs\Audio\Decode\ape\pApe.c
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
#ifdef HIFI_APE_DECODE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "audio_file_access.h"
#include "ape.h"
#include "audio_globals.h"



#pragma arm section code = "ApeHDecCode", rodata = "ApeHDecCode", rwdata = "ApeHDecData", zidata = "ApeHDecBss"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BBSYSTEM_CODECS_AUDIO_DECODE_APE_PAPE_READ_  
#define _BBSYSTEM_CODECS_AUDIO_DECODE_APE_PAPE_WRITE_ 
#define _BBSYSTEM_CODECS_AUDIO_DECODE_APE_PAPE_INIT_  
#define _BBSYSTEM_CODECS_AUDIO_DECODE_APE_PAPE_SHELL_  


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

//uint8 gHAPEPingPangBuf[2][4096*4];
u32 APE_out_Length = 1024;
uint8 *gHAPEPingPangBuf[2];
extern unsigned long SRC_Num_Forehead;
u32 gHAPEPingPangIndex;
int ID3_len;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

extern  MediaBlock *gmediaBlock;
FILE *ape_file_handle;
extern FILE *pRawFileCache;
APEContext apeobj;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/

static unsigned int CheckID3V2Tag(unsigned	char *pucBuffer)
{
    // The first three bytes of	the	tag	should be "ID3".
    if ((pucBuffer[0] !=	'I') ||	(pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }

    // The next	byte should	be the value 3 (i.e. we	support	ID3v2.3.0).
    //if(pucBuffer[3]	!= 3)
    if (pucBuffer[3]	<2  && pucBuffer[3]> 4)
    {
        return(0);
    }

    // The next	byte should	be less	than 0xff.
    if (pucBuffer[4]	== 0xff)
    {
        return(0);
    }

    // We don't	care about the next	byte.  The following four bytes	should be
    // less	than 0x80.
    if ((pucBuffer[6] >=	0x80) || (pucBuffer[7] >= 0x80)	||
            (pucBuffer[8] >=	0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }

    // Return the length of	the	ID3v2 tag.
    return((pucBuffer[6] <<	21)	| (pucBuffer[7]	<< 14) |
           (pucBuffer[8] <<	 7)	|  pucBuffer[9]);
}

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
READ API unsigned long  HIFI_APEDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
        unsigned long ulParam2, unsigned long ulParam3)
{
    APEContext *pAPE;
    pAPE = (APEContext *)&apeobj;
    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                int ret = 0;
                unsigned char flag[20];
                ape_file_handle = pRawFileCache;
                RKFIO_FSeek(0,0 ,ape_file_handle);
                RKFIO_FRead(flag,20,ape_file_handle);
                ID3_len = CheckID3V2Tag(flag);
                if (ID3_len == 0)
                {
                    RKFIO_FSeek(0,0 ,ape_file_handle);
                }
                else
                {
                    ID3_len += 10;
                    RKFIO_FSeek(ID3_len,0 ,ape_file_handle);
                }
                init_ape();
                ret = ape_read_header();
#ifdef HIFI_ACC
                HIFI_DMA_TO_register();
#endif

                if (ret < 0)
                {
                    bb_printf1("ape open FAIL!");
                    return 0;
                }

                bb_printf1("ape open OK ");
                return 1;
            }
        case SUBFN_CODEC_DECODE:
            {
                ape_decode(&gHAPEPingPangBuf[gHAPEPingPangIndex][SRC_Num_Forehead],&APE_out_Length);
                if (APE_out_Length <= 0 )
                {
                    return 0;
                }
                if (pAPE->TimePos >=pAPE->total_blocks)
                {
                    return 0;
                }

                return 1;
            }
        case SUBFN_CODEC_GETBUFFER:
            {
                *(unsigned long *)ulParam1 = (unsigned long)&gHAPEPingPangBuf[gHAPEPingPangIndex][SRC_Num_Forehead];
                *(unsigned long *)ulParam2 = (unsigned long) APE_out_Length;

                gHAPEPingPangIndex ^= 1;;
                return(1);
            }


        case SUBFN_CODEC_SEEK:
            {
                unsigned long ulPos;
                int file_offset = 0;

                if (ulParam1 > pAPE->file_time)//the unit is ms for ulParam1,the unit is also ms.
                {
                    ulParam1 = pAPE->file_time;
                }


                ulPos = (((ulParam1 / 1000) * pAPE->samplerate/*+pAPE->blocksperframe*/) / pAPE->blocksperframe) +
                        (((ulParam1 % 1000) * pAPE->samplerate ) / (pAPE->blocksperframe * 1000));//the frame number that is equal with current time.
                if (ulPos >= pAPE->totalframes)
                {
                    ulPos = pAPE->totalframes ;
                    file_offset =(long long) ulPos * pAPE->blocksperframe/pAPE->samplerate*pAPE->bitrate;
                    RKFIO_FSeek(file_offset, SEEK_SET,ape_file_handle); //获取新帧位置
                }


                pAPE->TimePos = ulPos * pAPE->blocksperframe;
                pAPE->APE_Frm_NUM = ulPos;
                pAPE->frm_left_sample= 0;

                return(1);
            }

        case SUBFN_CODEC_GETTIME:
            {
                unsigned long *pulTime;
                pulTime = (unsigned long *)ulParam1;

                if (pAPE->samplerate)
                {
                    *pulTime = ((pAPE->TimePos / pAPE->samplerate) * 1000) +
                               (((pAPE->TimePos % pAPE->samplerate) * 1000) /
                                pAPE->samplerate);
                }

                return(1);
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *SampleRate;
                SampleRate = (unsigned long *)ulParam1;
                *SampleRate = pAPE->samplerate;
                return  1;
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *channels;
                channels = (unsigned long *)ulParam1;
                *channels = pAPE->channels;
                return 1;
            }
        case SUBFN_CODEC_GETBPS:
            {
                unsigned long *bps;
                bps = (unsigned long *)ulParam1;
                *bps = pAPE->bps;
                return 1;
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *bitrate;
                bitrate = (unsigned long *)ulParam1;
                *bitrate = pAPE->bitrate+500;
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *TimeLength;
                TimeLength = (unsigned long *)ulParam1;
                *TimeLength = pAPE->file_time;
                return 1;
            }


            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {
                APE_free();
#ifdef HIFI_ACC
                HIFI_DMA_TO_Unregister();
#endif
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
