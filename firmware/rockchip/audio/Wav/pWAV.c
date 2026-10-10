/* Copyright (C) 2007 - 2020  ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File : \Audio\ADPCM
Desc : WAV decoding��including PCM WAV , IMA-ADPCM WAV , MS-ADPCM WAV ��

Author : FSH , Vincent Hisung
Date : 2007-08-xx
Notes :

$Log :
* vincent     2007/08/xx create this
*
*/
/****************************************************************/

#include "audio_main.h"

#ifdef WAV_DEC_INCLUDE

#include "audio_globals.h"
#include "audio_file_access.h"
#include "PCM.H"

#include <stdio.h>
#include <string.h>
#include "typedef.h"
#include "mailbox.h"

#include "SysInclude.h"
#include "DriverInclude.h"
extern MediaBlock  gpMediaBlock;
extern unsigned int gDecDone;
extern unsigned int gSeekDone;
extern unsigned int gCloseDone;
extern unsigned int gOpenDone;
extern unsigned int  gError;
extern unsigned char DecDataBuf[2][AUDIO_FILE_PIPO_BUF_SIZE];
extern unsigned char DecBufID;

static UINT16 DmaTranferCallback;

static void DMATranferCallBack(void)
{
    DmaTranferCallback = 1;
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
_ATTR_WAVDEC_TEXT_
unsigned long
PCMFunction(unsigned long ulIoctl, unsigned long ulParam1,
            unsigned long ulParam2, unsigned long ulParam3
           )
{
    uint32 timeout;

    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                gOpenDone = 0;
                DEBUG("wav codec open,A 2 B send dec_open...");
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DEC_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                timeout = 20000000;
                while (!gOpenDone)
                {
#ifdef _WATCH_DOG_
                    WatchDogReload();
#endif
                    BBDebug();
                    //__WFI();
                    DelayUs(1);
                    if (--timeout == 0)
                    {
                        DEBUG("SUBFN_CODEC_OPEN_DEC: timeout!!!");
                        break;
                    }
                }
                gOpenDone = 0;

                if ( gError )   //codec decode open error
                    return 0;
                else
                {
                    return(1);
                }
            }
            /*
                    case SUBFN_CODEC_GETARTIST:
                        {
                            char **ppcName;
                            ppcName = (char **)ulParam2;
                            *ppcName = 0;
                            return(1);
                        }

                    case SUBFN_CODEC_GETTITLE:
                        {
                            char **ppcName;
                            ppcName = (char **)ulParam2;
                            *ppcName = 0;

                            return(1);
                        }
            */

        case SUBFN_CODEC_GETBITRATE:
            {
                *(int *)ulParam1 = gpMediaBlock.BitRate;
                return(1);
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = gpMediaBlock.SampleRate;
                return(1);
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = gpMediaBlock.Channel;
                return(1);
            }

        case SUBFN_CODEC_GETLENGTH:
            {
                *(unsigned long *)ulParam1 = gpMediaBlock.TotalPlayTime;
                return(1);
            }

        case SUBFN_CODEC_GETBPS:
            {
                *(int *)ulParam1 = gpMediaBlock.Bps;
                return(1);
            }

        case SUBFN_CODEC_GETTIME:
            {
                *(unsigned long *)ulParam1 = gpMediaBlock.CurrentPlayTime;
                return(1);
            }

        case SUBFN_CODEC_GETBUFFER:
            {
                //DEBUG("wav get buffer...DecodeOver = %d",gpMediaBlock.DecodeOver);
                if (gpMediaBlock.DecodeOver == 1)
                {
                    if (gpMediaBlock.DecodeErr == 1)
                    {
                        return 0;
                    }

                    // *(int *)ulParam1 = gpMediaBlock.Outptr;
                    *(int *)ulParam2 = gpMediaBlock.OutLength;
                    gpMediaBlock.DecodeOver = 0;


                    gpMediaBlock.Decoding = 1;
                    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                    //memory copy hram 2 lram.
                    if (DecBufID == 0)
                    {
                        #if 1
                        {

                             eDMA_CHN channel;
                             DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};


                             DmaTranferCallback = 0;


                             channel = DmaGetChannel();
                             if (channel != DMA_FALSE)
                             {
                                 DmaStart((uint32)(channel), (UINT32)(gpMediaBlock.Outptr),(uint32)(&DecDataBuf[0][0]),(gpMediaBlock.OutLength * gpMediaBlock.Bps) / 16,&DmaCfg, DMATranferCallBack);

                                 while(1){
                                     __WFI();
                                     if(DmaTranferCallback == 1)
                                         break;
                                 }

                             }
                        }
                        #else

                        memcpy(&DecDataBuf[0][0],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.Bps) / 4);

                        #endif
                        *(int *)ulParam1 = (uint32)&DecDataBuf[0][0];
                        DecBufID = 1;
                    }
                    else
                    {

                        #if 1
                        {

                             eDMA_CHN channel;
                             DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};


                             DmaTranferCallback = 0;


                             channel = DmaGetChannel();
                             if (channel != DMA_FALSE)
                             {
                                 DmaStart((uint32)(channel), (UINT32)(gpMediaBlock.Outptr),(uint32)(&DecDataBuf[1][0]),(gpMediaBlock.OutLength * gpMediaBlock.Bps) / 16,&DmaCfg, DMATranferCallBack);

                                 while(1){
                                     __WFI();
                                     if(DmaTranferCallback == 1)
                                         break;
                                 }

                             }

                        }
                        #else
                        memcpy(&DecDataBuf[1][0],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.Bps) / 4);
                        #endif

                        *(int *)ulParam1 = (uint32)&DecDataBuf[1][0];
                        DecBufID = 0;
                    }

                    return(1);
                }
                else if (gpMediaBlock.Decoding == 0)
                {
                    gpMediaBlock.Decoding = 1;
                    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                    //printf("\n gpMediaBlock.Decoding == 0...decode request");
                    return 2;
                }
                else if (gpMediaBlock.Decoding == 1)
                {
                    //printf("\n gpMediaBlock.Decoding == 1...do nothing");
                    return 2;
                }

                return(0);
            }

        case SUBFN_CODEC_DECODE:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                return 1; //error.
            }

        case SUBFN_CODEC_SEEK:
            {
                gSeekDone = 0;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_SEEK, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                timeout = 200000;
                while (!gSeekDone)
                {
#ifdef _WATCH_DOG_
                    WatchDogReload();
#endif
                    BBDebug();
                    //__WFI();
                    DelayUs(1);
                    if (--timeout == 0)
                    {
                        DEBUG("SUBFN_CODEC_SEEK: timeout!!!");
                        break;
                    }
                }
                gSeekDone = 0;
                return(1);
            }

        case SUBFN_CODEC_CLOSE:
            {
                gCloseDone = 0;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_CLOSE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                timeout = 3000000;
                while (!gCloseDone)
                {
#ifdef _WATCH_DOG_
                    WatchDogReload();
#endif
                    BBDebug();
                    //__WFI();
                    DelayUs(1);
                    if (--timeout == 0)
                    {
                        DEBUG("SUBFN_CODEC_CLOSE: timeout!!!");
                        break;
                    }
                }
                gCloseDone = 0;
                return 1;
            }

        default:
            {
                return 0;
            }
    }
}

#endif
