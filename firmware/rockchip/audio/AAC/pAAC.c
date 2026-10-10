
/* Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File	: \Audio\APEDec
Desc	: decode for AAC format.

Author	: huangxd , Vincent Hisung
Date	: 2007-08-xx
Notes	:

$Log	:
* huangxd . create the file at 08.xx.2007
*
* vincent .	amendment at 08.xx.2007.
*
*/
/****************************************************************/
#include "SysInclude.h"
#include "audio_main.h"
#include "pAAC.h"

#ifdef AAC_DEC_INCLUDE

#include "audio_globals.h"
#include "audio_file_access.h"

#include "typedef.h"
#include "mailbox.h"
//#include "APEDec.h"

#pragma arm section code = "AacDecCode", rodata = "AacDecCode", rwdata = "AacDecData", zidata = "AacDecBss"


#define INREAD 2048
#define OUTWRITE 2048

#include "SysInclude.h"
#include "DriverInclude.h"
extern MediaBlock gpMediaBlock;
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

#define ICT ((((*((volatile unsigned int *)(0x60000000+0x10))))-((*((volatile unsigned int *)(0x60000000+0x4)))))>>2)

unsigned long
AACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
               unsigned long ulParam2, unsigned long ulParam3)

{
    AAC_TIMEINFO *info;
    int curread;
    static int test = 0;
    int ret,err;
    int m4a_len;
    uint32 timeout;

    switch (ulSubFn)
    {
            // Decode a frame of data.
        case SUBFN_CODEC_DECODE:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                return 1;
            }

            // Prepare the codec to decode a file.
        case SUBFN_CODEC_OPEN_DEC:
            {
                gOpenDone = 0;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DEC_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                timeout = 20000000;
                while(!gOpenDone)
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

                if( gError )    //codec decode open error
                    return 0;
                else
                    return(1);
            }

            // Seek to the specified time position.
        case SUBFN_CODEC_SEEK:
            {
                gSeekDone = 0;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_SEEK, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                timeout = 200000;
                while(!gSeekDone)
                {
                    #ifdef _WATCH_DOG_
                    WatchDogReload();
                    #endif
                    BBDebug ();
                    //__WFI();
                    DelayUs(1);
                    if (--timeout == 0)
                    {
                        DEBUG("SUBFN_CODEC_SEEK: timeout!!!");
                        break;
                    }
                }
                gSeekDone = 0;
                return 1;
            }
        case SUBFN_CODEC_GETBUFFER:
            {
                if(gpMediaBlock.DecodeOver == 1)
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
                else if(gpMediaBlock.Decoding == 0)
                {
                    gpMediaBlock.Decoding = 1;
                    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    return 2;
                }
                else if(gpMediaBlock.Decoding == 1)
                {
                    return 2;
                }

                return(0);
            }

            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {
                gCloseDone = 0;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_CLOSE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                timeout = 3000000;
                while(!gCloseDone)
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

            // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                *(int *)ulParam1 = gpMediaBlock.CurrentPlayTime;
                return 1;
            }

            // Return the sample rate at which this file is encoded.
        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = gpMediaBlock.SampleRate;
                return 1;
            }

            // Return the number of channels in the file.
        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = gpMediaBlock.Channel;
                return 1;
            }

            // Return the bitrate at which this file is encoded.
        case SUBFN_CODEC_GETBITRATE:
            {
                *(int *)ulParam1 = gpMediaBlock.BitRate;
                return 1;
            }
        case SUBFN_CODEC_GETBPS:
            {
                *(int *)ulParam1 = gpMediaBlock.Bps;
                return(1);
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                *(int *)ulParam1 = gpMediaBlock.TotalPlayTime;
                return 1;
            }

        default:
            {
                //failure
                return(0);
            }
    }
    return 0;
}


#pragma arm section code

#endif

