/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                         All rights reserved.
*
* File Name��   pWAVEnc.c
*
* Description:  WAV coding processing control.
*
* History:      <author>          <time>        <version>
*               WangBo           2009-4-16          1.0
*    desc:    ORG.
********************************************************************************
*/

#include  "SysInclude.h"
#include  "audio_main.h"

#ifdef ADPCM_ENC_INCLUDE

#include  "fsinclude.h"
#include  "File.h"
#include  "FDT.h"

#include  "PCM.H"

#include  "audio_globals.h"
#include  "recordcontrol.h"
extern RecordBlock  gRecordBlock;

extern UINT32 gEncodeDone;

//------------------------------------------------------------------------------

_ATTR_RECORD_CONTROL_CODE_
void ADPCM_Init(void * ptr)
{
    uint32 timeout = 20000000;
    gEncodeDone = 0;
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_ENCODE_INIT, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxWriteA2BData((uint32)ptr, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

    while (!gEncodeDone)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        BBDebug();
        //__WFI();
        DelayUs(1);
        if (--timeout == 0)
        {
            DEBUG("ADPCM_Init: timeout!!!");
            break;
        }
    }
    gEncodeDone = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void WavEncodeVariableInit()
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_BSS_      tPCM_enc   PCM_s;          //structure.
_ATTR_RECORD_CONTROL_CODE_
void WavEncodeVariableInit(void)
{
    UINT32 i;

    RecordInputBufferIndex   = 0;   //used for buffer switch.
    pRecordInputBuffer       = (UINT16*)WavEncodeInputBuffer;
    pRecordPCMInputBuffer    = pRecordInputBuffer;
    pRecordEncodeInputBuffer = pRecordInputBuffer + WAV_AD_PIPO_BUFFER_SIZE;

    for (i = 0; i < WAV_AD_PIPO_BUFFER_SIZE * 2; i++)
    {
        *(pRecordInputBuffer + i) = 0;
    }

    //WriteBuffer initialization.
    WriteBuffer.uHead     = 0;
    WriteBuffer.uTail     = 0;
    WriteBuffer.uCounter  = 0;

    //cache buffer for write initialization.
    for (i = 0; i < WAV_ENCODER_WRITE_BUFFER_LENGTH; i++)
    {
        WriteBuffer.Data[i] = 0;
    }

    DEBUG("[RECORD] WavEncodeVariableInit start.");

    PCM_s.usSampleRate = RecordSampleRate;    //sampling rate.
    PCM_s.ucChannels = RecordChannel;

    if (RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
    {
        PCM_s.wFormatTag = WAVE_FORMAT_ADPCM;
    }
    else if (RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
    {
        PCM_s.wFormatTag = WAVE_FORMAT_PCM;
    }
    memset((uint8*)&gRecordBlock,0,sizeof(RecordBlock));
    gRecordBlock.channel        = RecordChannel;
    gRecordBlock.dataWidth      = RecordDataWidth;
    gRecordBlock.encodeType     = RecordEncodeType;
    gRecordBlock.sampleRate     = RecordSampleRate;
    gRecordBlock.PCM_source     = (UINT32)&PCM_s;

    #ifdef _NS_
    if(((RecordType == RECORD_TYPE_MIC_STERO)
        || (RecordType == RECORD_TYPE_MIC1_MONO)
        || (RecordType == RECORD_TYPE_MIC2_MONO)) && (RecordSampleRate <= RECORD_FS_48KHz) && (RecordDataWidth == RECORD_DATAWIDTH_16BIT))
    {
        gRecordBlock.FilterFlag = 1;
    }
    else
    {
        gRecordBlock.FilterFlag = 0;
    }
    #else
    gRecordBlock.FilterFlag = 0;
    #endif
    ADPCM_Init(&gRecordBlock);

    DEBUG("[RECORD] WavEncodeVariableInit over");
    DEBUG("[RECORD] ByteRate = %d",PCM_s.usByteRate);
    DEBUG("[RECORD] channls = %d samplerate = %d",PCM_s.ucChannels,PCM_s.usSampleRate);
    DEBUG("[RECORD] usBytesPerBlock = %d usSamplesPerBlock = %d",PCM_s.usBytesPerBlock, PCM_s.usSamplesPerBlock);
    if((RecordChannel == RECORD_CHANNEL_MONO)
        && ((RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
        || (RecordEncodeType == RECORD_ENCODE_TYPE_PCM)))
    {
        RecordInputBufferLength = PCM_s.usSamplesPerBlock * RecordChannel * 2;
    }
    else
    {
        RecordInputBufferLength = PCM_s.usSamplesPerBlock * RecordChannel;     //the number of AD sampling.
    }

    RecordFileOffset   = 0;

    RecordConFlag = RECORD_CON_NULL;                     //the flag that be used to control coding.
    RecordStaFlag = RECORD_STA_PCMBUF_EMPTY;             //the flag that be used to control input buffer empty or full.

    RecordNOISECount = RECORD_STARTNOISEBLOCK;
}

/*
--------------------------------------------------------------------------------
  Function name : void WavEncodeHeaderInit()
  Author        : WangBo
  Description   : write the head of wav file.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void WavEncodeHeaderInit()
{
    PCMWAVEFORMAT sWaveFormat;
    int ulIdx,i;
    int AdaptCoeff11 [7] =
        {       256, 512, 0, 192, 240, 460, 392
        } ;
    int AdaptCoeff22 [7] =
        {       0, -256, 0, 64, 0, -208, -232
        } ;

    WavEncodeHeadBuffer = &(WriteBuffer.Data[0]);

    for (i=4095;i>=0;i--)                   //init  WavEncodeHeadBuffer[512]
    {
        WavEncodeHeadBuffer[i]=0x00;
    }

    WavEncodeHeadBuffer[0] = 'R';
    WavEncodeHeadBuffer[1] = 'I';
    WavEncodeHeadBuffer[2] = 'F';
    WavEncodeHeadBuffer[3] = 'F';

    WavEncodeHeadBuffer[8]  = 'W';
    WavEncodeHeadBuffer[9]  = 'A';
    WavEncodeHeadBuffer[10] = 'V';
    WavEncodeHeadBuffer[11] = 'E';

    WavEncodeHeadBuffer[12] = 'f';
    WavEncodeHeadBuffer[13] = 'm';
    WavEncodeHeadBuffer[14] = 't';
    WavEncodeHeadBuffer[15] = ' ';
    WavEncodeHeadBuffer[16] = 50;
    WavEncodeHeadBuffer[17] = 0;
    WavEncodeHeadBuffer[18] = 0;
    WavEncodeHeadBuffer[19] = 0;

    if (RECORD_ENCODE_TYPE_PCM == RecordEncodeType)
    {
        sWaveFormat.wFormatTag = 1;
        sWaveFormat.nChannels = PCM_s.ucChannels;
        sWaveFormat.nSamplesPerSec = PCM_s.usSampleRate;
        sWaveFormat.nAvgBytesPerSec = PCM_s.usByteRate;
        if (RECORD_DATAWIDTH_24BIT == RecordDataWidth)
        {
            sWaveFormat.nBlockAlign = 6;
            sWaveFormat.wBitsPerSample = 24;
        }
        else
        {
            sWaveFormat.nBlockAlign = PCM_s.usBytesPerBlock;
            sWaveFormat.wBitsPerSample = 16;
        }
        sWaveFormat.cbSize = 32;
        sWaveFormat.wSamplesPerBlock = PCM_s.usSamplesPerBlock;
        sWaveFormat.wNumCoef = 0;
        for (ulIdx = 0; ulIdx < 7; ulIdx++)
        {
            sWaveFormat.aCoef[ulIdx].iCoef1 = 0;
            sWaveFormat.aCoef[ulIdx].iCoef2 = 0;
        }
    }
    else
    {
        sWaveFormat.wFormatTag = 2;
        sWaveFormat.nChannels = PCM_s.ucChannels;
        sWaveFormat.nSamplesPerSec = PCM_s.usSampleRate;
        sWaveFormat.nAvgBytesPerSec = PCM_s.usByteRate;
        sWaveFormat.nBlockAlign = PCM_s.usBytesPerBlock;
        sWaveFormat.wBitsPerSample = 4;
        sWaveFormat.cbSize = 32;
        sWaveFormat.wSamplesPerBlock = PCM_s.usSamplesPerBlock;
        sWaveFormat.wNumCoef = 7;

        for (ulIdx = 0; ulIdx < 7; ulIdx++)
        {
            sWaveFormat.aCoef[ulIdx].iCoef1 = AdaptCoeff11[ulIdx];
            sWaveFormat.aCoef[ulIdx].iCoef2 = AdaptCoeff22[ulIdx];
        }
    }

    memcpy(WavEncodeHeadBuffer + 20, (void *)&sWaveFormat, 50);

    WavEncodeHeadBuffer[70] = 'f';
    WavEncodeHeadBuffer[71] = 'a';
    WavEncodeHeadBuffer[72] = 'c';
    WavEncodeHeadBuffer[73] = 't';
    WavEncodeHeadBuffer[74] = 4;
    WavEncodeHeadBuffer[75] = 0;
    WavEncodeHeadBuffer[76] = 0;
    WavEncodeHeadBuffer[77] = 0;

    WavEncodeHeadBuffer[78] = PCM_s.ulTimePos;
    WavEncodeHeadBuffer[79] = PCM_s.ulTimePos >> 8;
    WavEncodeHeadBuffer[80] = PCM_s.ulTimePos >> 16;
    WavEncodeHeadBuffer[81] = PCM_s.ulTimePos >> 24;

    WavEncodeHeadBuffer[82] = 0x4c;//"list"
    WavEncodeHeadBuffer[83] = 0x49;
    WavEncodeHeadBuffer[84] = 0x53;
    WavEncodeHeadBuffer[85] = 0x54;

    WavEncodeHeadBuffer[86] = 0x9e;
    WavEncodeHeadBuffer[87] = 0x0f; //by Vincent

    WavEncodeHeadBuffer[90] = 0x49;//INFO
    WavEncodeHeadBuffer[91] = 0x4e;
    WavEncodeHeadBuffer[92] = 0x46;
    WavEncodeHeadBuffer[93] = 0x4f;

    WavEncodeHeadBuffer[94] = 'R';
    WavEncodeHeadBuffer[95] = 'o';
    WavEncodeHeadBuffer[96] = 'c';
    WavEncodeHeadBuffer[97] = 'k';

    WavEncodeHeadBuffer[98]  = 0x92; //by Vincent
    WavEncodeHeadBuffer[99]  = 0x0f; //by Vincent

    WavEncodeHeadBuffer[4088] = 'd';
    WavEncodeHeadBuffer[4089] = 'a';
    WavEncodeHeadBuffer[4090] = 't';
    WavEncodeHeadBuffer[4091] = 'a';

    WavEncodeHeadBuffer[4092] = 0Xff;
    WavEncodeHeadBuffer[4093] = 0Xff;
    WavEncodeHeadBuffer[4094] = 0Xff;
    WavEncodeHeadBuffer[4095] = 0Xff;

    WavEncodeHeadBuffer[4] = 0Xff;
    WavEncodeHeadBuffer[5] = 0Xff;
    WavEncodeHeadBuffer[6] = 0Xff;
    WavEncodeHeadBuffer[7] = 0Xff;

}

#endif
//#endif
