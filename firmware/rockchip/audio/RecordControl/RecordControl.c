/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                         All rights reserved.
*
* File Name��   RecordControl.c
*
* Description:  system record layer,the module of back server.
*
* History:      <author>          <time>        <version>
*               WangBo           2009-4-16          1.0
*    desc:    ORG.
*
* Log    :     sampling rate is 16k no care is quality or normal record.
               cut off half after the average to sample points in normal record
*              RecordEncodeCounter relate with RecordInputBufferLength,but RecordInputBufferLength is relate with RecordSampleRate.
********************************************************************************
*/

#define _IN_RECORD_CONTROL

#include  "SysInclude.h"

#ifdef _RECORD_

#include  "fsinclude.h"
#include  "File.h"
#include  "FDT.h"

#include  "pcm.h"
#include  "pmu.h"

#include  "audio_main.h"
#include  "AudioControl.h"
#include  "RecordControl.h"
#include  "FMControl.h"

extern tPCM_enc   PCM_s;
extern  uint32      UserIsrDisableFlag;
_ATTR_RECORD_CONTROL_BSS_ static uint32  RecordFileW_cnt;
_ATTR_RECORD_CONTROL_BSS_ static uint32  RecordFileR_cnt;
_ATTR_RECORD_CONTROL_BSS_ static uint32  RecordFile_cnt_A;
_ATTR_RECORD_CONTROL_BSS_ static uint32  RecordFile_cnt_B;
_ATTR_RECORD_CONTROL_BSS_ static uint32  RecordFile_cnt_C;

#if(I2S_CH == I2S_CH0)
_ATTR_RECORD_CONTROL_DATA_ DMA_CFGX RecordControlDmaCfg  = {DMA_CTLL_I2S0_RX, DMA_CFGL_I2S0_RX, DMA_CFGH_I2S0_RX, 0};
#else
_ATTR_RECORD_CONTROL_DATA_ DMA_CFGX RecordControlDmaCfg  = {DMA_CTLL_I2S1_RX, DMA_CFGL_I2S1_RX, DMA_CFGH_I2S1_RX, 0};
#endif

_ATTR_RECORD_CONTROL_DATA_  UINT8 MicPathStr[]      = {'U', ':', '\\', 'R', 'E', 'C', 'O', 'R', 'D', ' ', ' ', ' ', ' ', ' ', '\\', 'V', 'O', 'I', 'C', 'E', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
_ATTR_RECORD_CONTROL_DATA_  UINT8 MicFileNameStr[]  = {'R', 'E', 'C', '0', '0', '0', '0', '0', 'W', 'A', 'V', '\0'};
_ATTR_RECORD_CONTROL_DATA_  UINT8 FMPathStr[]       = {'U', ':', '\\', 'R', 'E', 'C', 'O', 'R', 'D', ' ', ' ', ' ', ' ', ' ', '\\', 'F', 'M', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
_ATTR_RECORD_CONTROL_DATA_  UINT8 FMFileNameStr[]   = {'F', 'R', '0', '0', '0', '0', ' ', ' ', 'W', 'A', 'V', '\0'};

//------------------------------------------------------------------------------
_ATTR_RECORD_CONTROL_BSS_ RecFileHdl_t gRecordFileHdl; //file handle and other info struct
_ATTR_RECORD_CONTROL_BSS_ uint8        gACKFlag;       //file open / close handle flag
_ATTR_RECORD_CONTROL_BSS_ RecordBlock  gRecordBlock;   //record encord block
_ATTR_RECORD_CONTROL_BSS_ uint32       gEncodeDone;   //flag of mailbox irq complete

//_ATTR_RECORD_CONTROL_BSS_ uint8    gIsRecord;
_ATTR_RECORD_CONTROL_BSS_ uint8     RecordNeedEncode;
_ATTR_RECORD_CONTROL_BSS_ uint8     gRecordBlockencodeOver;
_ATTR_RECORD_CONTROL_BSS_ uint32    gRecordBlockencodeLen;
_ATTR_RECORD_CONTROL_BSS_ uint8     RecordEncodeFirst;
_ATTR_RECORD_CONTROL_BSS_ uint8     EncBufID;

_ATTR_RECORD_CONTROL_BSS_ uint32    RecInputBufferOverFlow;
_ATTR_RECORD_CONTROL_BSS_ uint32    RecInputBufferLen;
_ATTR_RECORD_CONTROL_BSS_ uint16    *pRecInputBuffer;

static UINT16 RecordDmaTranferCallback;
static UINT16 DmaTranferCb;

uint32 DMASendDataLen;
_ATTR_RECORD_CONTROL_CODE_
static void DMATranferCb1(void)
{
    RecordDmaTranferCallback = 1;
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_ENCODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxWriteA2BData(DMASendDataLen, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
}

_ATTR_RECORD_CONTROL_CODE_
static void DMATranferCb2(void)
{
    DmaTranferCb = 1;
}

_ATTR_RECORD_CONTROL_CODE_
static UINT16 GetDMATranferCb1(void)
{
    return RecordDmaTranferCallback;
}

_ATTR_RECORD_CONTROL_CODE_
static UINT16 GetDMATranferCb2(void)
{
    return DmaTranferCb;
}
//------------------------------------------------------------------------------
_ATTR_RECORD_CONTROL_CODE_
void RecordGetDataFromFM(UINT32 *DataBuf, uint32 encodeDataLen)
{
    memcpy((UINT32 *)pRecordPCMInputBuffer, DataBuf, encodeDataLen * 4);
    pRecordEncodeInputBuffer = pRecordPCMInputBuffer;
    RecordInputBufferIndex   = 1 - RecordInputBufferIndex;
    pRecordPCMInputBuffer    = pRecordInputBuffer + RecordInputBufferIndex * WAV_AD_PIPO_BUFFER_SIZE;
    REC_Flag = RECORD_BEING;
    RecordEncodeing = 1;
    RecordStaFlag &= ~RECORD_STA_PCMBUF_EMPTY;
    RecordStaFlag |= RECORD_STA_PCMBUF_FULL;
    IntPendingSet(INT_ID_REV0);
}

_ATTR_RECORD_CONTROL_CODE_
void RecordCopyEncDataToBuf(UINT16 *DataBuf, uint32 encodeDataLen)
{
    UINT32  i, uTail;
    uint32  remain_size,need_size;

    RecInputBufferOverFlow    = 0;

    if (RECORD_DATAWIDTH_16BIT == RecordDataWidth)
    {
        uint8 *pData8 = (uint8 *)DataBuf;

        remain_size = WAV_ENCODER_WRITE_BUFFER_LENGTH - WriteBuffer.uCounter;
        need_size   = encodeDataLen;
        if (need_size > remain_size)
        {
            RecInputBufferOverFlow = 1;
            pRecInputBuffer        = DataBuf;
            RecInputBufferLen      = encodeDataLen;
            //DEBUG("remain_size = %d, need_size = %d", remain_size, need_size);
            return;
        }

        WriteBuffer.uTail = WriteBuffer.uTail % WAV_ENCODER_WRITE_BUFFER_LENGTH;
        for (i = 0; i < encodeDataLen; i++)
        {
            WriteBuffer.Data[WriteBuffer.uTail] = pData8[i];
            WriteBuffer.uTail = (WriteBuffer.uTail + 1) % WAV_ENCODER_WRITE_BUFFER_LENGTH;
        }
        WriteBuffer.uCounter += encodeDataLen;
    }
    else    //24bit datawidth
    {
        uint8 *pData8 = (uint8 *)DataBuf;

        remain_size = WAV_ENCODER_WRITE_BUFFER_LENGTH - WriteBuffer.uCounter;
        need_size   = (encodeDataLen / 4) * 3;
        if (need_size > remain_size)
        {
            RecInputBufferOverFlow = 1;
            pRecInputBuffer        = DataBuf;
            RecInputBufferLen      = encodeDataLen;
            DEBUG("remain_size = %d, need_size = %d", remain_size, need_size);
            return;
        }

        WriteBuffer.uTail = WriteBuffer.uTail % WAV_ENCODER_WRITE_BUFFER_LENGTH;
        for (i = 0; i < encodeDataLen / 4; i++)
        {
            WriteBuffer.Data[WriteBuffer.uTail] = pData8[4 * i + 1] ;
            WriteBuffer.uTail = (WriteBuffer.uTail + 1) % WAV_ENCODER_WRITE_BUFFER_LENGTH;

            WriteBuffer.Data[WriteBuffer.uTail] = pData8[4 * i + 2];
            WriteBuffer.uTail = (WriteBuffer.uTail + 1) % WAV_ENCODER_WRITE_BUFFER_LENGTH;

            WriteBuffer.Data[WriteBuffer.uTail] = pData8[4 * i + 3];
            WriteBuffer.uTail = (WriteBuffer.uTail + 1) % WAV_ENCODER_WRITE_BUFFER_LENGTH;

        }
        WriteBuffer.uCounter += (encodeDataLen / 4) * 3;
    }
}

_ATTR_RECORD_CONTROL_CODE_
void RecordExitType(UINT32 Type)
{
    switch (Type)
    {
        case RECORD_TYPE_MIC_STERO:
            Codec_ExitMode(Codec_MicStero);
            break;

        case RECORD_TYPE_LINEIN1:
            Codec_ExitMode(Codec_Line1ADC);
            break;

        case RECORD_TYPE_LINEIN2:
            Codec_ExitMode(Codec_Line2ADC);
            break;

        case RECORD_TYPE_MIC1_MONO:
            Codec_ExitMode(Codec_Mic1Mono);
            break;

        case RECORD_TYPE_MIC2_MONO:
            Codec_ExitMode(Codec_Mic2Mono);
            break;

        default:
            break;
    }
}

_ATTR_RECORD_CONTROL_CODE_
void RecordSetType(UINT32 Type)
{
#if (CODEC_CONFIG == CODEC_ROCKC)

    switch (Type)
    {
        case RECORD_TYPE_MIC_STERO:
            DEBUG("record mic stero");
            Codec_SetMode(Codec_MicStero, RecordDataWidth);
            ACodec_Set_MIC_AnaVol(2);
            ACodec_Set_ADCMUX_Vol(6);
            ACodec_Set_ADC_DigVol(0);
            break;

        case RECORD_TYPE_LINEIN1:
            Codec_SetMode(Codec_Line1ADC, RecordDataWidth);
            ACodec_Set_ADCMUX_Vol(6);
            ACodec_Set_ADC_DigVol(0);
            break;

        case RECORD_TYPE_LINEIN2:
            Codec_SetMode(Codec_Line2ADC, RecordDataWidth);
            ACodec_Set_ADCMUX_Vol(6);
            ACodec_Set_ADC_DigVol(0);
            break;

        case RECORD_TYPE_MIC1_MONO:
            Codec_SetMode(Codec_Mic1Mono, RecordDataWidth);
            ACodec_Set_MIC_AnaVol(2);
            ACodec_Set_ADCMUX_Vol(6);
            ACodec_Set_ADC_DigVol(0);
            break;

        case RECORD_TYPE_MIC2_MONO:
            Codec_SetMode(Codec_Mic2Mono, RecordDataWidth);
            ACodec_Set_MIC_AnaVol(2);
            ACodec_Set_ADCMUX_Vol(6);
            ACodec_Set_ADC_DigVol(0);
            break;

        default:
            break;
    }
#else
    //if using other codec ,defint under here
    //todo.....
#endif
}
_ATTR_RECORD_CONTROL_CODE_
void RecordDmaIsrHandler(void)
{
    uint32 RX_FIFO_ADDR, i;

    if (RecordEncodeing == 1)
    {
        DEBUG("encoding");
        RecordNeedEncode = 1;
        RecordDmaTransting = 0;
        return;
    }

    if (REC_Flag == RECORD_BEING)
    {
        RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH);
        pRecordEncodeInputBuffer = pRecordPCMInputBuffer;
        RecordInputBufferIndex   = 1 - RecordInputBufferIndex;
        pRecordPCMInputBuffer    = pRecordInputBuffer + RecordInputBufferIndex * WAV_AD_PIPO_BUFFER_SIZE;
        DmaStart(RECORD_DMACHANNEL_IIS, RX_FIFO_ADDR, (uint32)pRecordPCMInputBuffer,
                 RecordInputBufferLength / 2, &RecordControlDmaCfg, RecordDmaIsrHandler);
        REC_Flag = RECORD_BEING;
        RecordEncodeing = 1;
        RecordStaFlag &= ~RECORD_STA_PCMBUF_EMPTY;
        RecordStaFlag |= RECORD_STA_PCMBUF_FULL;

        IntPendingSet(INT_ID_REV0);         //cause decode interrupt.
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void RecordAdcInit()
  Author        : WangBo
  Description   : control the amplifier of MIC.
  Description   : coding cup initialization.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordCpuInit(void)
{
    FREQ_EnterModule(FREQ_RECORDADPCM);
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordServiceCpuResume()
  Author        : WangBo
  Description   :
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordServiceCpuResume(void)
{
    FREQ_ExitModule(FREQ_RECORDADPCM);
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordGetFileName()
  Author        : WangBo
  Description   : get file name.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_BSS_ uint8 RecordNameFlag[BYTE_NUM_SAVE_PER_FILE];

_ATTR_RECORD_CONTROL_CODE_
BOOL RecordGetFileName(void)
{
    UINT32 i;
    UINT32 RecordClus;
    UINT8 temp;
    FDT Fdt;
    UINT16 CurIndex;

    MakeDir("\\", "RECORD");

    if (RecordExitTaskID == TASK_ID_RADIO)
    {
        MakeDir("\\RECORD", "FM");

        for (i = 0; i < 99 ; i++)
        {
            RecordFileInfo.Path[i]     = FMPathStr[i];

            if (FMPathStr[i] == '\0')
                break;
        }

        for (i = 0; i < 11; i ++)
        {
            RecordFileInfo.Fdt.Name[i] = FMFileNameStr[i];
        }
    }
    else
    {
        MakeDir("\\RECORD", "VOICE");

        for (i = 0; i < 99 ; i++)
        {
            RecordFileInfo.Path[i]     = MicPathStr[i];

            if (MicPathStr[i] == '\0')
                break;
        }

        for (i = 0; i < 11; i ++)
        {
            RecordFileInfo.Fdt.Name[i] = MicFileNameStr[i];
        }
    }

    RecordClus = GetDirClusIndex(RecordFileInfo.Path);

    if (RecordClus == BAD_CLUS)
    {
        //failure to search file name.
        SendMsg(MSG_RECORD_STATUS_GETFILE_FAIL);
        //printf("error recor dir\n");
        return FALSE;
    }

    if (gSysConfig.MedialibPara.gRecordFmFileNum >= 999)
    {
        SendMsg(MSG_RECORD_STATUS_GETFILE_FAIL);
        return FALSE;
    }

    i = 0;
    memset(RecordNameFlag, 0, 1250); // 9999/8

    //printf("RecordClus = %d\n", RecordClus);

    while (1)
    {
        temp = GetFDTInfo(&Fdt, RecordClus, i++);

        //printf("i = %d\n", i);
        if (temp != RETURN_OK)
        {
            break;
        }

        if (Fdt.Name[0] == 0)
        {
            break;
        }

        if (Fdt.Name[0] == FILE_NOT_EXIST)
        {
            continue;
        }

        if (Fdt.Name[0] == '.')
        {
            continue;
        }

        if (Fdt.Attr == ATTR_LFN_ENTRY)
        {
            continue;
        }

        if ((Fdt.Attr & ATTR_DIRECTORY) != 0)
        {
            continue;
        }

        if ((Fdt.Name[0] != 'F') || (Fdt.Name[1] != 'R') || (Fdt.Name[6] != ' ') || (Fdt.Name[7] != ' ')
            || (Fdt.Name[8] != 'W') || (Fdt.Name[9] != 'A') || (Fdt.Name[10] != 'V'))
        {
            continue;
        }

        if ((Fdt.Name[2] > '9') || (Fdt.Name[2] < '0'))
        {
            continue;
        }

        if ((Fdt.Name[3] > '9') || (Fdt.Name[3] < '0'))
        {
            continue;
        }

        if ((Fdt.Name[4] > '9') || (Fdt.Name[4] < '0'))
        {
            continue;
        }

        if ((Fdt.Name[5] > '9') || (Fdt.Name[5] < '0'))
        {
            continue;
        }

        CurIndex = 0;
        CurIndex = (Fdt.Name[2] - '0');
        CurIndex = CurIndex * 10;
        CurIndex += (Fdt.Name[3] - '0');
        CurIndex = CurIndex * 10;
        CurIndex += (Fdt.Name[4] - '0');
        CurIndex = CurIndex * 10;
        CurIndex += (Fdt.Name[5] - '0');
        //printf("CurIndex = %d\n", CurIndex);
        RecordNameFlag[CurIndex / 8] |= 1 << (CurIndex % 8);
    }

    if ((gSysConfig.RecordConfig.RecordIndex > 9999) || (gSysConfig.RecordConfig.RecordIndex == 0))
    {
        gSysConfig.RecordConfig.RecordIndex = 1;
    }

    CurIndex = gSysConfig.RecordConfig.RecordIndex;
    //printf("gSysConfig.RecordConfig.RecordIndex = %d\n", gSysConfig.RecordConfig.RecordIndex);

    while (1)
    {
        if (RecordNameFlag[CurIndex / 8] & (1 << (CurIndex % 8)))
        {
            CurIndex++;

            if (CurIndex == gSysConfig.RecordConfig.RecordIndex)
            {
                //failure to search file name.
                SendMsg(MSG_RECORD_STATUS_GETFILE_FAIL);
                return FALSE;
            }

            if (CurIndex > 9999)
            {
                CurIndex = 1;
            }
        }
        else
        {
            gSysConfig.RecordConfig.RecordIndex = CurIndex + 1;
            break;
        }
    }

    RecordFileInfo.Fdt.Name[5] = '0' + (CurIndex % 10 );
    RecordFileInfo.Fdt.Name[4] = '0' + ((CurIndex / 10 ) % 10);
    RecordFileInfo.Fdt.Name[3] = '0' + ((CurIndex / 100 ) % 10);
    RecordFileInfo.Fdt.Name[2] = '0' + ((CurIndex / 1000) % 10);

    for (i = 0; i < 8; i ++)
    {
        RecordLongFileName[i] = RecordFileInfo.Fdt.Name[i];
    }

    RecordLongFileName[8] =   '.';
    RecordLongFileName[9] =   'W';
    RecordLongFileName[10] =  'A';
    RecordLongFileName[11] =  'V';
    RecordLongFileName[12] =  '\0';

    FileClose(hRecordFile);
    hRecordFile = -1;
    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordServiceGetTotalTime()
  Author        : WangBo
  Description   : get the left space,the compute the total recoding time by this left space.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
BOOL RecordServiceGetTotalTime (void)
{
    RecordFreeMemory = sysFreeMemeryFlash0;   //get disk left space,unit K

    if ((MIN_RECORD_RESERVE_MEMORY / 1024) > RecordFreeMemory)
    {
        SendMsg(MSG_RECORD_STATUS_DISK_FULL);
        return FALSE;
    }

    //printf("RecordFreeMemory=%d RecordBitrate=%d\n", RecordFreeMemory, RecordBitrate);
    RecordCurrTime  = 0xffffffff;
    //RecordTotalTime = ((RecordFreeMemory - (MIN_RECORD_RESERVE_MEMORY / 1024)) * 8) / ((UINT32) RecordBitrate);  //RecordFreeMemory �ĵ�λByte, RecordBitrate �ĵ�λ�� kbps

    if ( RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
    {
        if ((RECORD_QUALITY_HIGH == gbRecordQuality) && (RECORD_DATAWIDTH_24BIT == RecordDataWidth))
        {
            RecordTotalTime = ((RecordFreeMemory - (MIN_RECORD_RESERVE_MEMORY /1024)) ) / ((UINT32) RecordBitrate);   //RecordFreeMemory �ĵ�λByte, RecordBitrate �ĵ�λ�� kbps
        }
    }
    else
    {
        RecordTotalTime = ((RecordFreeMemory - (MIN_RECORD_RESERVE_MEMORY /1024)) * 8) / ((UINT32) RecordBitrate);   //RecordFreeMemory �ĵ�λByte, RecordBitrate �ĵ�λ�� kbps
    }

    DEBUG(" ### TOTAL TIME = %d ### ", RecordTotalTime);
    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordGetCurTime()
  Author        : WangBo
  Description   : get current and total record left time.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordGetCurTime(void)
{
    RecordCurrTime = (RecordFileOffset - WAV_HEAD_BUFFER_SIZE) / ((UINT32)RecordBitrate * 128);//Bitrate / 8 * 1000 ,the other algorithm to compute record time.
    //RecordCurrTime = RecordCurrTime * 9765 / 10000;   //HJ
    RecordRemainTime = RecordTotalTime - RecordCurrTime; //total record left time.
}


/*
--------------------------------------------------------------------------------
  Function name : void RecordGetInfo()
  Author        : WangBo
  Description   :
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordGetInfo(void)
{
    BOOL    bStatus1, bStatus2;
    unsigned short usBytesPerBlock;
    unsigned short usSamplesPerBlock;
    unsigned short uBitsPerSample;
    gbRecordQuality = gSysConfig.RecordConfig.RecordQuality; //get record volume from menu or system backup.

    if ( RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
    {
        if ((RECORD_QUALITY_HIGH == gbRecordQuality) && (RECORD_DATAWIDTH_24BIT == RecordDataWidth))
        {
            RecordBitrate = (RecordChannel * RecordSampleRate * 3) / 1000;
        }
        else //normal quality & 16bit
        {
            RecordBitrate = (RecordChannel * RecordSampleRate * 2) / 1000;
        }
    }
    else if ( RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
    {
        switch (RecordSampleRate)
        {
            case    8000:
            case    11025:
                usBytesPerBlock = 256;
                break;

            case    16000:
                usBytesPerBlock = 256;
                break;

            case    22050:
            case    24000:
                usBytesPerBlock = 512;
                break;

            case    32000:
            case    44100:
            case    48000:
            case    64000:
            case    88200:
            case    96000:
            case    128000:
            case    176400:
            case    192000:
                usBytesPerBlock = 1024;
                break;

            default:
                usBytesPerBlock = 1024;
                break;
        }

        usSamplesPerBlock = ((usBytesPerBlock << 1) >> (RecordChannel - 1)) - 12;
        DEBUG("usSamplesPerBlock = %d", usSamplesPerBlock);
        RecordBitrate = (RecordSampleRate * usBytesPerBlock / usSamplesPerBlock) * 8;
        RecordBitrate = RecordBitrate / 1000;
    }

    FREQ_EnterModule(FREQ_MAX);
    bStatus1 = RecordServiceGetTotalTime();
    bStatus2 = RecordGetFileName();
    FREQ_ExitModule(FREQ_MAX);

    if ((bStatus1 == TRUE) && (bStatus2 == TRUE))
    {
        SendMsg(MSG_RECORDWIN_DISPFLAG_RECQUALITY);  //notice the record quality,time and file name.
        SendMsg(MSG_RECORD_STATUS_PREPARE);
        SendMsg(MSG_RECORDWIN_DISPFLAG_TIME);
        SendMsg(MSG_RECORDWIN_DISPFLAG_FILENAME);
        SendMsg(MSG_RECORDWIN_DISPFLAG_TOTALTIME);
    }
    else
    {
        DEBUG("GetFileName and total time Fail!");
    }
}


/*
--------------------------------------------------------------------------------
  Function name : void GetEncodeType()
  Author        : WangBo
  Description   : select record code type.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void GetEncodeType(void)
{
}



/*
--------------------------------------------------------------------------------
  Function name : void WavEncodeIsr()
  Author        : WangBo
  Description   : interrupt callback to record.
                  ���޸ĵı����жϺ���ֻ�����������д��д����Buffer,
                  д�ļ����� WinService �д�����
                  ���Խ�� flash д���ٶ��������⣬���ܲ�����ֶ���������
                  the interrupt function that is be changed newly just only write the data to cache buffer.
                  write to file service be put in WinService.
                  it can solve the slowly write problem,may don't happen the phenomenon of staccato .
  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_RECORD_CONTROL_CODE_
void DataEncodeIsr(void)
{
    UINT32 i, j;
    UINT16  *pSampleBuffer;
    UINT32  *pSampleBuffer32;
    UINT8   *Data24bit;
    uint32 index, index_old;
    uint32 DmaTransferLen8 = RecordInputBufferLength * 2;
    uint32 DmaTransferLen16 = RecordInputBufferLength;
    uint32 DmaTransferLen32 = RecordInputBufferLength / 2;
    uint8  DMA_Get_Data = 0;

    eDMA_CHN channel;
    DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

    if (RecordConFlag & RECORD_CON_ENCODE_ENABLE) //code enter into force.
    {
        if (RecordStaFlag & RECORD_STA_PCMBUF_FULL) //input buffer is full.
        {
            if ( RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
            {
                RecordStaFlag &= ~RECORD_STA_PCMBUF_FULL;
            }

            pSampleBuffer =  pRecordEncodeInputBuffer;
            pSampleBuffer32 = (uint32*)pRecordEncodeInputBuffer;

            if ((gRecordBlockencodeOver == 1) || (RecordEncodeFirst == 0))
            {
                gRecordBlockencodeOver = 0;

                //Input PCM Data to B Core
                if (RecordChannel == RECORD_CHANNEL_MONO)
                {
                    if (RecordType == RECORD_TYPE_MIC1_MONO)
                    {
                        index_old = 0;

                        for (index = 0; index < DmaTransferLen16 / 2; index++)
                        {
                            if (RECORD_DATAWIDTH_16BIT == RecordDataWidth)
                            {
                                //pSampleBuffer[index] = pSampleBuffer[2*index_old];
                                pSampleBuffer[index] = pSampleBuffer32[index_old];
                                index_old++;
                            }
                            else    //RECORD_DATAWIDTH_24BIT
                            {
                                pSampleBuffer32[index] = pSampleBuffer32[2 * index];
                            }
                        }
                        DmaTransferLen16 = DmaTransferLen16 / 2;
                        DmaTransferLen8 = DmaTransferLen16 * 2;
                    }
                    else if (RecordType == RECORD_TYPE_MIC2_MONO)
                    {
                        index_old = 0;

                        for (index = 0; index < DmaTransferLen16 / 2; index++)
                        {
                            if (RECORD_DATAWIDTH_16BIT == RecordDataWidth)
                            {
                                //pSampleBuffer[index] = pSampleBuffer[2*index_old+1];
                                pSampleBuffer[index] = pSampleBuffer32[index_old + 1];
                                index_old++;
                            }
                            else    //RECORD_DATAWIDTH_24BIT
                            {
                                pSampleBuffer32[index] = pSampleBuffer32[2 * index + 1];
                            }
                        }
                        DmaTransferLen16 = DmaTransferLen16 / 2;
                        DmaTransferLen8 = DmaTransferLen16 * 2;
                    }
                }
                else if ( RecordChannel == RECORD_CHANNEL_STERO)
                {
                    if (RecordType == RECORD_TYPE_MIC1_MONO)
                    {
                        for (index = 0; index < DmaTransferLen16 / 2; index++)
                        {
                            if (RECORD_DATAWIDTH_16BIT == RecordDataWidth)
                            {
                                //pSampleBuffer[2*index+1] = pSampleBuffer[2*index];
                                pSampleBuffer[2 * index + 1] = pSampleBuffer32[index];
                            }
                            else    //RECORD_DATAWIDTH_24BIT
                            {
                                pSampleBuffer32[2 * index + 1] = pSampleBuffer32[2 * index];
                            }
                        }

                        DmaTransferLen8 = DmaTransferLen16 * 2;
                    }
                    else if (RecordType == RECORD_TYPE_MIC2_MONO)
                    {
                        for (index = 0; index < DmaTransferLen16 / 2; index++)
                        {
                            if (RECORD_DATAWIDTH_16BIT == RecordDataWidth)
                            {
                                //pSampleBuffer[2*index] = pSampleBuffer[2*index+1];
                                pSampleBuffer[2 * index] = pSampleBuffer32[index + 1];
                            }
                            else    //RECORD_DATAWIDTH_24BIT
                            {
                                pSampleBuffer32[2 * index] = pSampleBuffer32[2 * index + 1];
                            }
                        }

                        DmaTransferLen8 = DmaTransferLen16 * 2;
                    }
                    else
                    {
                        ;
                    }
                }

                if ( RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
                {
                    RecordCopyEncDataToBuf(pSampleBuffer, DmaTransferLen8);
                    gRecordBlockencodeOver = 1;
                }
                else
                {
                    RecordDmaTranferCallback = 0;
                    channel = DmaGetChannel();

                    if (channel != DMA_FALSE)
                    {
                        DMASendDataLen = DmaTransferLen8;
                        DmaStart((uint32)(channel),
                                 (UINT32)(pSampleBuffer),
                                 (uint32)(gRecordBlock.unenc_bufptr),
                                 DmaTransferLen8 / 4,
                                 &DmaCfg,
                                 DMATranferCb1);
                    }
                }

                if (RecordEncodeFirst == 0)
                {
                    RecordEncodeFirst = 1;
                    RecordEncodeing = 0;
                    return;
                }

                if ((RecordExitTaskID != TASK_ID_RADIO) || (FM_OUT_CONFIG != FM_I2S_HP))
                {
                    IntDisable(INT_ID_DMA);
                    RecordEncodeing = 0;

                    if (RecordDmaTransting == 0)
                    {
                        RecordDmaIsrHandler();
                    }

                    RecordDmaTransting = 1;
                    IntEnable(INT_ID_DMA);
                }
            }
            else
            {
                RecordNeedEncode = 1;
            }
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name :void RecordWaitBBStart(void)
  Author        : chad.ma
  Description   : mail cmd of file create success.
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                  chad.ma         2015.05.15     1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordWaitBBStart(void)
{
    uint32 timeout = 20000000;
    gACKFlag = 0;
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_CREATE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteA2BData((int)&gRecordFileHdl, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    while (!gACKFlag)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        BBDebug();
        //__WFI();
        DelayUs(1);

        if (--timeout == 0)
        {
            DEBUG("RecordWaitBBStart: timeout!!!");
            break;
        }
    }

    gACKFlag = 0;
}

/*
--------------------------------------------------------------------------------
  Function name :void RecordWaitBBStop(void)
  Author        : chad.ma
  Description   : wait for BB core file close
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                 chad.ma         2015/05/15         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordWaitBBStop(void)
{
    uint32 timeout = 20000000;

    gACKFlag = 0;
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_REC_FILE_CLOSE, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteA2BData(1, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    while (!gACKFlag)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        BBDebug();
        //__WFI();
        DelayUs(1);

        if (--timeout == 0)
        {
            DEBUG("RecordWaitBBStop: timeout!!!");
            break;
        }
    }

    gACKFlag = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordHWInit()
  Author        : chad.ma
  Description   : for record module B core start and regester isr servers.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               chad.ma           2015-5-15          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordHWInit()
{
    StartBBSystem(MODULE_ID_WAV_DECODE_BIN);

    DEBUG("[RECORD] RecordHWInit OK");
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordHWDeInit()
  Author        : chad.ma
  Description   : for record module B core stop and de-regester isr servers.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               chad.ma           2015-5-15          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordHWDeInit()
{
    ShutOffBBSystem();
    DEBUG("[RECORD] RecordHWDeInit");
}

_ATTR_RECORD_CONTROL_CODE_
void RecordEncodeGetData(uint32 data)
{
    //Get B core Encode Info
    memcpy(&gRecordBlock, (RecordBlock *)data, sizeof(RecordBlock));

    //Copy B core Encode data to A buffer
    RecordCopyEncDataToBuf((UINT16*)gRecordBlock.enc_bufptr, gRecordBlock.length);
    gRecordBlockencodeOver = 1;

    if (RecordNeedEncode == 1)
    {
        IntPendingSet(INT_ID_REV0);         //cause decode interrupt.
    }
    RecordNeedEncode = 0;
}

/*
--------------------------------------------------------------------------------
  Function name :void RecordEncodeGetOutBuffer(void)
  Author        :
  Description   : Get encode  data from calculate core
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                   mlc             2015/05/15         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
__irq void RecordEncodeGetOutBuffer(void)
{
    uint32 cmd;
    uint32 data;

    cmd  = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxClearB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);

    //printf("\n # cmd = %d #",cmd);
    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_ENCODE_INIT_CMPL:
        {
            memcpy(&gRecordBlock, (RecordBlock *)data, sizeof(RecordBlock));
            memcpy((uint8*)&PCM_s, (uint8*)gRecordBlock.PCM_source, sizeof(tPCM_enc));

            gEncodeDone = 1;
        }
            break;

        case MEDIA_MSGBOX_CMD_ENCODE_CMPL:
        {
            //encode complte ,wait to write to file
            RecordEncodeGetData(data);
        }
            break;

        default:
            break;
    }
}

_ATTR_RECORD_CONTROL_CODE_
void RegMBoxRecEncodeSvc()
{
    IntRegister(INT_ID_MAILBOX1, (void*)RecordEncodeGetOutBuffer);
    IntPendingClear(INT_ID_MAILBOX1);
    IntEnable(INT_ID_MAILBOX1);
    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);
}

_ATTR_RECORD_CONTROL_CODE_
void DeRegMBoxRecEncodeSvc()
{
    IntUnregister(INT_ID_MAILBOX1);
    IntPendingClear(INT_ID_MAILBOX1);
    IntDisable(INT_ID_MAILBOX1);
    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);
}

/*
--------------------------------------------------------------------------------
  Function name :void RecordEncodingFileBuffer(void)
  Author        :
  Description   : input file data stream to calculate core
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                   mlc             2015/04/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
__irq void RecordEncodingFileBuffer(void)
{
    uint32 cmd;
    uint32 data;

    cmd  = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxClearB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);

    //printf("\n ### cmd = %d ###\n ",cmd);
    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_FILE_CREATE_HANDSHK:
            gACKFlag = 1;
            break;

        case MEDIA_MSGBOX_CMD_REC_FILE_CLOSE_HANDSHK:
            gACKFlag = 1;
            break;

        default:
            break;
    }
}

_ATTR_RECORD_CONTROL_CODE_
void RegMBoxRecFileSvc()
{
    IntRegister(INT_ID_MAILBOX2, (void*)RecordEncodingFileBuffer);
    IntPendingClear(INT_ID_MAILBOX2);
    IntEnable(INT_ID_MAILBOX2);
    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
}

_ATTR_RECORD_CONTROL_CODE_
void DeRegMBoxRecFileSvc()
{
    IntUnregister(INT_ID_MAILBOX2);
    IntPendingClear(INT_ID_MAILBOX2);
    IntDisable(INT_ID_MAILBOX2);
    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
}

//------------------------------------------------------------------------------


/*
--------------------------------------------------------------------------------
  Function name : void RecordInit()
  Author        : WangBo
  Description   : the initialization of record.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordInit(void)
{
#ifdef ADPCM_ENC_INCLUDE
    ModuleOverlay(MODULE_ID_RECORD_MSADPCM, MODULE_OVERLAY_ALL);
    WavEncodeVariableInit();
    WavEncodeHeaderInit();
#endif
}

extern  uint32   FlashSec[3];
/*
--------------------------------------------------------------------------------
  Function name : void RecordDBAddFile()
  Author        : WangBo
  Description   : build file in disk.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
BOOLEAN RecordDBAddFile (uint32 Clus, uint32 Index)
{
    uint16 TreeIndex, SortIndex;
    uint16 temp;
    uint32 CurSec, SecOffset;
    UINT32 i;
    UINT8 *pBuffer, *pPath;
    FILE_TREE_BASIC FileTreeBasic, * pFileTreeBasic;
    uint16 longStr1[MEDIA_ID3_SAVE_CHAR_NUM];
    uint16 longStr2[MEDIA_ID3_SAVE_CHAR_NUM];
    uint8 cmpflag;
    uint16 max, min, mid;

    if (gSysConfig.MedialibPara.gRecordFmFileNum > 999)
    {
        return;
    }

    TreeIndex = 0;
    cmpflag = 0;

    if (gSysConfig.MedialibPara.gRecordFmFileNum == 0)
    {
        TreeIndex = 0;
        cmpflag = 2;
    }
    else
    {
        do
        {
            MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START) << 9) + sizeof(FILE_TREE_BASIC) * (UINT32)(TreeIndex), sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);

            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DELETED)
            {
                cmpflag = 1;
                //printf("find a deleted\n");
                break;
            }

            TreeIndex++;

            if (TreeIndex > 999)
            {
                TreeIndex = 0;
                cmpflag = 2;
                break;
            }
        }
        while (FileTreeBasic.dwNextBrotherID != 0xffffffff);
    }

    if (cmpflag == 1)
    {
        CurSec = MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START + (TreeIndex * sizeof(FILE_TREE_BASIC)) / SECTOR_BYTE_SIZE;
        SecOffset = (TreeIndex % (SECTOR_BYTE_SIZE / sizeof(FILE_TREE_BASIC))) * sizeof(FILE_TREE_BASIC);
        MDRead(DataDiskID, CurSec, 1, RecordNameFlag);
        pFileTreeBasic = (FILE_TREE_BASIC *)&RecordNameFlag[SecOffset];
        pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_FILE;
        memcpy((uint8 *)&FileTreeBasic, (uint8 *)(pFileTreeBasic), sizeof(FileTreeBasic));
        MDWrite(DataDiskID, CurSec, 1, RecordNameFlag);
    }
    else if (cmpflag == 0)
    {
        CurSec = MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START + ((TreeIndex - 1) * sizeof(FILE_TREE_BASIC)) / SECTOR_BYTE_SIZE;
        SecOffset = ((TreeIndex - 1) % (SECTOR_BYTE_SIZE / sizeof(FILE_TREE_BASIC))) * sizeof(FILE_TREE_BASIC);
        MDRead(DataDiskID, CurSec, 2, RecordNameFlag);
        pFileTreeBasic = (FILE_TREE_BASIC *)&RecordNameFlag[SecOffset];
        pFileTreeBasic->dwNextBrotherID = TreeIndex;
        pFileTreeBasic++;
        pFileTreeBasic->dwBasicInfoID = TreeIndex + (MEDIAINFO_BLOCK_SIZE * 15  * SECTOR_BYTE_SIZE)  / BYTE_NUM_SAVE_PER_FILE;
        pFileTreeBasic->dwExtendTreeID = 0xffffffff;
        pFileTreeBasic->dwNextBrotherID = 0xffffffff;
        pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_FILE;
        memcpy((uint8 *)&FileTreeBasic, (uint8 *)(pFileTreeBasic), sizeof(FileTreeBasic));
        MDWrite(DataDiskID, CurSec, 2, RecordNameFlag);
    }
    else if (cmpflag == 2)
    {
        CurSec = MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START + (TreeIndex * sizeof(FILE_TREE_BASIC)) / SECTOR_BYTE_SIZE;
        SecOffset = (TreeIndex % (SECTOR_BYTE_SIZE / sizeof(FILE_TREE_BASIC))) * sizeof(FILE_TREE_BASIC);
        MDRead(DataDiskID, CurSec, 1, RecordNameFlag);
        pFileTreeBasic = (FILE_TREE_BASIC *)&RecordNameFlag[SecOffset];
        pFileTreeBasic->dwNextBrotherID = 0xffffffff;
        pFileTreeBasic->dwExtendTreeID = 0xffffffff;
        pFileTreeBasic->dwBasicInfoID =  pFileTreeBasic->dwBasicInfoID = TreeIndex + (MEDIAINFO_BLOCK_SIZE * 15  * SECTOR_BYTE_SIZE)  / BYTE_NUM_SAVE_PER_FILE;
        pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_FILE;
        memcpy((uint8 *)&FileTreeBasic, (uint8 *)(pFileTreeBasic), sizeof(FileTreeBasic));
        MDWrite(DataDiskID, CurSec, 1, RecordNameFlag);
    }
    else
    {
        return;
    }

    CurSec = MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START + (BYTE_NUM_SAVE_PER_FILE * (UINT32)(FileTreeBasic.dwBasicInfoID)) / SECTOR_BYTE_SIZE;
    MDRead(DataDiskID, CurSec, BYTE_NUM_SAVE_PER_FILE / SECTOR_BYTE_SIZE, RecordNameFlag);
    //printf("Clus = %d, Index = %d\n", Clus, Index);
    GetLongFileName(Clus, Index, FS_FAT, longStr1);
    pBuffer = RecordNameFlag + FILE_NAME_SAVE_ADDR_OFFSET;

    for (i = 0; i < MEDIA_ID3_SAVE_CHAR_NUM; i++) //���泤�ļ�����Ϣ
    {
        *pBuffer++ = longStr1[i] & 0xff;//���ֽ�
        //printf("%02x ", *(pBuffer - 1));
        *pBuffer++ = ((longStr1[i]) >> 8) & 0xff; //���ֽ�

        if (longStr1[i] == 0)
        {
            break;
        }
    }

    pBuffer = RecordNameFlag + DIR_CLUS_SAVE_ADDR_OFFSET;
    *((uint32 *)pBuffer) = Clus;
    pBuffer = RecordNameFlag + DIR_INDEX_SAVE_ADDR_OFFSET;
    *((uint32 *)pBuffer) = Index + 1;
    pBuffer = RecordNameFlag + ATTR_SAVE_ADDR_OFFSET;
    *pBuffer++ = 0;
    *pBuffer++ = 'F';
    *pBuffer++ = 0;
    *pBuffer++ = 'I';
    *pBuffer++ = 0;
    *pBuffer++ = 'L';
    *pBuffer++ = 0;
    *pBuffer++ = 'E';
    MDWrite(DataDiskID, CurSec, BYTE_NUM_SAVE_PER_FILE / SECTOR_BYTE_SIZE, RecordNameFlag);

    //sort
    if (gSysConfig.MedialibPara.gRecordFmFileNum == 0)
    {
        SortIndex = 0;
    }
    else
    {
        min = 0;
        max = gSysConfig.MedialibPara.gRecordFmFileNum - 1;
        //Get max string
        MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START) << 9) + 2 * (UINT32)(max), 2, (uint8 *)&temp);
        MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START) << 9) + sizeof(FILE_TREE_BASIC) * (UINT32)(temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START) << 9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(FileTreeBasic.dwBasicInfoID) + FILE_NAME_SAVE_ADDR_OFFSET, MEDIA_ID3_SAVE_CHAR_NUM * 2, (uint8 *) & (longStr2));
        cmpflag = 1;

        for (i = 0; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)
        {
            if (longStr1[i] > longStr2[i])
            {
                cmpflag = 2;
                break;
            }
            else if (longStr1[i] < longStr2[i])
            {
                cmpflag = 0;
                break;
            }
            else if (longStr1[i] == NULL)
            {
                //printf("i = %d\n", i);
                cmpflag = 1;
                break;
            }
        }

        //printf("cmpflag = %d\n", cmpflag);

        if ((cmpflag == 2) || (cmpflag == 1)) //>=max
        {
            SortIndex = max + 1;
            goto SORT_OK;
        }
        else if (max == 0)
        {
            SortIndex = max;
            goto SORT_OK;
        }

        //Get min string
        MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START) << 9) + 2 * (UINT32)(min), 2, (uint8 *)&temp);
        MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START) << 9) + sizeof(FILE_TREE_BASIC) * (UINT32)(temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START) << 9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(FileTreeBasic.dwBasicInfoID) + FILE_NAME_SAVE_ADDR_OFFSET, MEDIA_ID3_SAVE_CHAR_NUM * 2, (uint8 *) & (longStr2));
        cmpflag = 1;

        for (i = 0; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)
        {
            if (longStr1[i] > longStr2[i])
            {
                cmpflag = 2;
                break;
            }
            else if (longStr1[i] < longStr2[i])
            {
                cmpflag = 0;
                break;
            }
            else if (longStr1[i] == NULL)
            {
                //printf("i = %d\n", i);
                cmpflag = 1;
                break;
            }
        }

        if ((cmpflag == 0) || (cmpflag == 1)) // <= min
        {
            SortIndex = min;
            goto SORT_OK;
        }
        else if (max == 0)
        {
            SortIndex = min + 1;
            goto SORT_OK;
        }

        while (1)
        {
            mid = (min + max) / 2;
            //Get mid string
            MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START) << 9) + 2 * (UINT32)(mid), 2, (uint8 *)&temp);
            MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START) << 9) + sizeof(FILE_TREE_BASIC) * (UINT32)(temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
            MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + RECORD_SAVE_INFO_SECTOR_START) << 9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(FileTreeBasic.dwBasicInfoID) + FILE_NAME_SAVE_ADDR_OFFSET, MEDIA_ID3_SAVE_CHAR_NUM * 2, (uint8 *) & (longStr2));
            cmpflag = 1;

            for (i = 0; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)
            {
                if (longStr1[i] > longStr2[i])
                {
                    cmpflag = 2;
                    break;
                }
                else if (longStr1[i] < longStr2[i])
                {
                    cmpflag = 0;
                    break;
                }
                else if (longStr1[i] == NULL)
                {
                    //printf("i = %d\n", i);
                    cmpflag = 1;
                    break;
                }
            }

            //printf("\n min = %d, mid = %d, max = %d", min, mid, max);
            //printf("\n cmpflag = %d", cmpflag);

            if (cmpflag == 1) // == min
            {
                SortIndex = mid;
                goto SORT_OK;
            }
            else if (cmpflag == 2) //>mid
            {
                min = mid;
            }
            else  // < mid
            {
                max = mid;
            }

            if ((min == max) || (min == max - 1))
            {
                SortIndex = max;
                goto SORT_OK;
            }
        }
    }

SORT_OK:
    MDRead(DataDiskID, MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START, 4, RecordNameFlag);

    //printf("SortIndex = %d, max = %d\n", SortIndex, gSysConfig.MedialibPara.gRecordFmFileNum - 1);

    if (SortIndex >= gSysConfig.MedialibPara.gRecordFmFileNum)
    {
        ((uint16 *)RecordNameFlag)[SortIndex] = TreeIndex;
    }
    else
    {
        for (i = gSysConfig.MedialibPara.gRecordFmFileNum; i > SortIndex; i--)
        {
            ((uint16 *)RecordNameFlag)[i] = ((uint16 *)RecordNameFlag)[i - 1];
            //printf("RemoveData = %d\n", ((uint16 *)RecordNameFlag)[i]);
        }

        ((uint16 *)RecordNameFlag)[SortIndex] = TreeIndex;
    }

    MDWrite(DataDiskID, MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START, 4, RecordNameFlag);
    FlashSec[0] = 0xffffffff;
    FlashSec[1] = 0xffffffff;
    FlashSec[2] = 0xffffffff;
    gSysConfig.MedialibPara.gRecordFmFileNum++;
    SaveSysInformation(1);
}



/*
--------------------------------------------------------------------------------
  Function name : void RecordCreateFile()
  Author        : WangBo
  Description   : build file in disk.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
BOOLEAN RecordCreateFile (void)
{
    //hRecordFile = FileOpen(RecordFileInfo.Path,RecordFileInfo.Fdt.Name ,"W");
    if (0)
    {
        FileSeek(0, SEEK_END, hRecordFile);
    }
    else
    {
        hRecordFile = FileCreate(RecordFileInfo.Path, RecordFileInfo.Fdt.Name);

        if (hRecordFile == NOT_OPEN_FILE)
        {
            DEBUG("===## RecordFile Create Fail ##===");
            return FALSE;
        }

        #ifdef _MEDIA_MODULE_
        RecordDBAddFile(FileInfo[hRecordFile].DirClus, FileInfo[hRecordFile].Index);
        #endif
    }

    if (4096 != FileWrite(WavEncodeHeadBuffer, FileInfo[hRecordFile].FileSize, 4096, hRecordFile))
    {
        FileClose(hRecordFile);
        return FALSE;
    }

    RecordFileOffset = WAV_HEAD_BUFFER_SIZE;

    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordWriteFile()
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
BOOLEAN RecordWriteFile (void)
{
    UINT32  i, uWriteBytes, uHead, TempLength;
    uint32  MbIsrTemp;
    UINT8  *pWriteBuffer;

    if (((RecordFileOffset / 1024) + 1) > (RecordFreeMemory - MIN_RECORD_RESERVE_MEMORY / 1024))
    {
        DEBUG("disk full err");
        SendMsg(MSG_RECORD_STATUS_DISK_FULL);         //disk capacity is insufficient.
        SendMsg(MSG_RECORDWIN_DISPFLAG_TIME);
        REC_Flag =  RECORD_ERROR;
        return FALSE;
    }

    if (RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
    {
        TempLength = WAV_ENCODER_WRITE2FLASH_LENGTH * 16;        // Max to gRecEncbuf[] size
    }
    else
    {
        TempLength = WAV_ENCODER_WRITE2FLASH_LENGTH * 8;        // Max to gRecEncbuf[] size
    }

    if (WriteBuffer.uCounter >= TempLength)     //write 2 k data every time.
    {
        if ( RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
        {
            MbIsrTemp = IsrDisable(INT_ID_REV0);
        }
        else
        {
            MbIsrTemp = IsrDisable(INT_ID_MAILBOX1);
        }

        uHead           = WriteBuffer.uHead;
        pWriteBuffer    = &(WriteBuffer.Data[uHead]);

        if ((WAV_ENCODER_WRITE_BUFFER_LENGTH - uHead) >= TempLength)
        {
            memcpy(gRecEncbuf, pWriteBuffer, TempLength);
        }
        else
        {
            uint32 temp = WAV_ENCODER_WRITE_BUFFER_LENGTH - uHead;
            memcpy(gRecEncbuf, pWriteBuffer, temp);
            pWriteBuffer = &(WriteBuffer.Data[0]);
            memcpy(gRecEncbuf + temp, pWriteBuffer, TempLength - temp);
        }
        WriteBuffer.uCounter  -= TempLength;
        WriteBuffer.uHead     = (uHead + TempLength) % WAV_ENCODER_WRITE_BUFFER_LENGTH;

        if ( RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
        {
            IsrEnable(INT_ID_REV0, MbIsrTemp);
        }
        else
        {
            IsrEnable(INT_ID_MAILBOX1, MbIsrTemp);
        }

        UserIsrDisableFlag  = 1;
        uWriteBytes  = FileWrite(gRecEncbuf, FileInfo[hRecordFile].FileSize, TempLength, hRecordFile);
        UserIsrDisableFlag  = 0;

        if (TempLength == uWriteBytes)
        {
            RecordFileOffset += uWriteBytes;
        }
        else
        {
            DEBUG("write file err!!! write bytes = %d ",uWriteBytes);
            SendMsg(MSG_RECORD_STATUS_WRITEFILE_FAIL);
            REC_Flag =  RECORD_ERROR;
            return FALSE;
        }

        if (((RecordFileOffset / 1024 / 1024)) > 4094) //4// 4G
        {
            DEBUG("write 4g file");
            SendMsg(MSG_RECORD_ENCODESTART);
            REC_Flag =  RECORD_ERROR;
            return FALSE;
        }

        SendMsg(MSG_RECORDWIN_DISPFLAG_TIME); //Send the flag that mark up time update.
    }

    if (RecInputBufferOverFlow)
    {
        if ( RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
        {
            MbIsrTemp = IsrDisable(INT_ID_REV0);
        }
        else
        {
            MbIsrTemp = IsrDisable(INT_ID_MAILBOX1);
        }

        RecordCopyEncDataToBuf(pRecInputBuffer, RecInputBufferLen);
        gRecordBlockencodeOver = 1;

        if ( RecordEncodeType == RECORD_ENCODE_TYPE_PCM)
        {
            IsrEnable(INT_ID_REV0, MbIsrTemp);
        }
        else
        {
            IsrEnable(INT_ID_MAILBOX1, MbIsrTemp);
        }
    }

    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordWriteFileTail()
  Author        : WangBo
  Description   : before close file,write all cache data to flash once,for avoid data lose.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordWriteFileTail (void)
{
    UINT32  i, j;
    UINT32  uHead;
    UINT32  uWriteBytes;
    uint32  TempLength;
    UINT8  *pWriteBuffer;

    //filled data align 512 byte.
    TempLength = ((WriteBuffer.uCounter + 511) / 512) * 512;

    for (i = 0; i < TempLength - WriteBuffer.uCounter; i++)
    {
        j = WriteBuffer.uTail;
        WriteBuffer.Data[j] = 0x00;
        WriteBuffer.uTail = (j + 1) % WAV_ENCODER_WRITE_BUFFER_LENGTH;
    }

    uHead           = WriteBuffer.uHead;
    pWriteBuffer    = &(WriteBuffer.Data[uHead]);

    if ((WAV_ENCODER_WRITE_BUFFER_LENGTH - uHead) >= TempLength)
    {
        uWriteBytes     = FileWrite(pWriteBuffer, FileInfo[hRecordFile].FileSize, TempLength, hRecordFile);
    }
    else
    {
        uint32 temp = WAV_ENCODER_WRITE_BUFFER_LENGTH - uHead;
        memcpy(&(WriteBuffer.Data[0]) + temp, &(WriteBuffer.Data[0]), TempLength - temp);
        memcpy(&(WriteBuffer.Data[0]), &(WriteBuffer.Data[uHead]), temp);
        uWriteBytes     = FileWrite(&(WriteBuffer.Data[0]), FileInfo[hRecordFile].FileSize, TempLength, hRecordFile);
    }

    if (TempLength == uWriteBytes)
    {
        RecordFileOffset += uWriteBytes;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordIsrInstall()
  Author        : WangBo
  Description   : register timer interruption and code interruption.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordCodecInit(void)
{
    IntPrioritySet(INT_ID_REV0, 0xe0);

    IntPendingClear(INT_ID_REV0);

    if (RecordCodecConfig == 0)
    {
        I2SInit(I2S_CH,I2S_PORT, I2S_MODE,RecordSampleRate,I2S_FORMAT,RecordDataWidth,I2S_NORMAL_MODE);
        RecordSetType(RecordType);
        Codec_SetSampleRate(RecordSampleRate);
    }

    IntRegister(INT_ID_REV0, DataEncodeIsr);
    IntEnable(INT_ID_REV0);
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordIsrUninstall()
  Author        : WangBo
  Description   : timer and code interruptions auti-initialization.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordCodecDeInit(void)
{
    IntPendingClear(INT_ID_REV0);
    IntDisable(INT_ID_REV0);
    IntUnregister(INT_ID_REV0);

    if (RecordCodecConfig == 0)
    {
        RecordExitType(RecordType);
        I2SStop(I2S_CH,I2S_START_DMA_RX);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordTimerIsrEnable()
  Author        : WangBo
  Description   : start timer interruption.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordEnable(void)
{
    RecordConFlag |= RECORD_CON_ENCODE_ENABLE;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordTimerIsrDisable()
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordDisable(void)
{
    RecordConFlag &= ~RECORD_CON_ENCODE_ENABLE;
}
_ATTR_RECORD_CONTROL_CODE_
BOOLEAN RecordResume(void)
{
    uint32 RX_FIFO_ADDR;
    DEBUG("=== RecordResume in ===  ");

    if (REC_Flag == RECORD_BEING)
    {
        return TRUE;
    }

    if (REC_Flag == RECORD_PAUSE)
    {
        AutoPowerOffDisable();
    }

    if ((RecordExitTaskID == TASK_ID_RADIO) && (FM_OUT_CONFIG == FM_I2S_HP))
    {
        if (REC_Flag == RECORD_PAUSE)
        {
            RecordEnable();
            RecordDmaTransting = 1;
            REC_Flag = RECORD_BEING;
            RecordEncodeing = 0;
            RecordStaFlag &= ~RECORD_STA_PCMBUF_EMPTY;
            RecordStaFlag |= RECORD_STA_PCMBUF_FULL;
        }
    }
    else
    {
        RecordCodecInit();
        RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH);
        DEBUG("REC_Flag = %d", REC_Flag);

        if (REC_Flag != RECORD_BEING)
        {
            RecordEnable();
            RecordDmaTransting = 1;
            DmaStart(RECORD_DMACHANNEL_IIS, RX_FIFO_ADDR, (uint32)pRecordPCMInputBuffer,
                     RecordInputBufferLength / 2, &RecordControlDmaCfg, RecordDmaIsrHandler);
            REC_Flag = RECORD_BEING;
            RecordEncodeing = 0;
            RecordStaFlag &= ~RECORD_STA_PCMBUF_EMPTY;
            RecordStaFlag |= RECORD_STA_PCMBUF_FULL;

            I2SStart(I2S_CH,I2S_START_DMA_RX);
            if (RecordCodecConfig == 0)
            {
                RecordSetType(RecordType);
                Codec_SetSampleRate(RecordSampleRate);
            }

            DEBUG("Record DMA & I2S start...");
        }

        DEBUG("=== RecordResume out ===");
    }

    return TRUE;
}
_ATTR_RECORD_CONTROL_CODE_
BOOLEAN RecordPause(void)
{
    uint32 timeout = 2000;
    DEBUG("=== RecordPause in ===");

    if (REC_Flag == RECORD_BEING)
    {
        if ((RecordExitTaskID == TASK_ID_RADIO) && (FM_OUT_CONFIG == FM_I2S_HP))
        {
            REC_Flag = RECORD_PAUSE;
            RecordDisable();
        }
        else
        {
            REC_Flag = RECORD_PAUSE;

            while (DmaGetState(RECORD_DMACHANNEL_IIS) == DMA_BUSY)
            {
                DelayMs(1);

                if (--timeout == 0)
                {
                    break;
                }

                DEBUG("DMA_BUSY");
            }

            I2SStop(I2S_CH,I2S_START_DMA_RX);
            I2SDeInit(I2S_CH, I2S_PORT);
            RecordCodecDeInit();
            RecordDisable();
            RecordEncodeing = 0;
            RecordDmaTransting = 0;
        }
    }

    DEBUG("=== RecordPause out ===");
    return TRUE;
}
/*
--------------------------------------------------------------------------------
  Function name : void RecordStop()
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecordStop(void)
{
    UINT32  TempFileSize;
    UINT32  TempSectorIndex;
    UINT32  TempFileLength;
    TASK_ARG TaskArg;
    uint32 timeout = 2000;

    DEBUG("RecordStop");

    if ((REC_Flag == RECORD_BEING) || (REC_Flag == RECORD_PAUSE) || (REC_Flag == RECORD_ERROR)) //if the current status had been recording status or suspend status.
    {
        if ((RecordExitTaskID == TASK_ID_RADIO) && (FM_OUT_CONFIG == FM_I2S_HP))
        {
            REC_Flag = RECORD_NULL;
            RecordDisable();
        }
        else
        {
            REC_Flag = RECORD_NULL;

            while (DmaGetState(RECORD_DMACHANNEL_IIS) == DMA_BUSY)
            {
                DelayMs(1);

                if (--timeout == 0)
                {
                    DEBUG("DMA_BUSY");
                    break;
                }
            }

            I2SStop(I2S_CH,I2S_START_DMA_RX);
            I2SDeInit(I2S_CH, I2S_PORT);
        }

        RecordCodecDeInit();
        RecordDisable();
        RecordEncodeing = 0;
        RecordDmaTransting = 0;

        if (NOT_OPEN_FILE != hRecordFile)
        {
            RecordWriteFileTail();
            //TempFileSize    = FileInfo[hRecordFile].FileSize;
            TempFileSize    = FileInfo[hRecordFile].FileSize - 8;   //RIFF size
            TempSectorIndex = ((FileInfo[hRecordFile].FstClus - 2) << LogSecPerClus) + BootSector.FirstDataSector;
            FileClose(hRecordFile);
            hRecordFile = -1;

            //TempFileLength  = TempFileSize - WAV_HEAD_BUFFER_SIZE;
            TempFileLength  = (TempFileSize + 8) - WAV_HEAD_BUFFER_SIZE;
            WavEncodeHeaderInit();
            WavEncodeHeadBuffer[4]   = (TempFileSize >> 0) & 0xff; //RIFF WAVE Chunk Size
            WavEncodeHeadBuffer[5]   = (TempFileSize >> 8) & 0xff;
            WavEncodeHeadBuffer[6]   = (TempFileSize >> 16) & 0xff;
            WavEncodeHeadBuffer[7]   = (TempFileSize >> 24) & 0xff;
            WavEncodeHeadBuffer[4092]   = (TempFileLength >> 0) & 0xff; // Data Chunk Size
            WavEncodeHeadBuffer[4093]   = (TempFileLength >> 8) & 0xff;
            WavEncodeHeadBuffer[4094]   = (TempFileLength >> 16) & 0xff;
            WavEncodeHeadBuffer[4095]   = (TempFileLength >> 24) & 0xff;

            if (TempFileSize > 0)
            {
                MDWrite(FileDiskID, TempSectorIndex, 8, WavEncodeHeadBuffer);
            }

            //if there is record files,the number of record files will changed,need to clear breakpoint.?
#ifdef AUDIOHOLDONPLAY
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif
            RecordWaitBBStop();
            RecordHWDeInit();
        }

        RecordFileOffset = 0;
        //FREQ_Enable();
        RecordServiceCpuResume();
    }
    else
    {
        //
    }
}

_ATTR_RECORD_CONTROL_CODE_
int RecordStart(void)
{
    int ret, i;
    uint32 timeout = 200;
    int16* pCodecReceiveData;
    uint32 RX_FIFO_ADDR;

    RecordEncodeFirst = 0;
    EncBufID = 0;
    gRecordBlockencodeOver = 1;
    gRecordBlockencodeLen  = 0;
    RecordNeedEncode = 0;
    RecordCurrTime  = 0xffffffff;

    DEBUG("RecordStart Enter...");
    RecordCpuInit();
    DelayMs(10);
    //FREQ_Disable(); //pmu disable for usb otg stability
    RecordHWInit();

    RecordInit();
    RecordCodecInit();

    ret = RecordCreateFile();     //build file in disk.

    if (FALSE == ret )
    {
        RecordServiceCpuResume();
        SendMsg(MSG_RECORD_STATUS_CREATEFILE_FAIL);
        return;
    }

    memset(&gRecordFileHdl, 0, sizeof(gRecordFileHdl));
    gRecordFileHdl.fileHandle = hRecordFile;
    gRecordFileHdl.fileOffset = RecordFileOffset;
    RecordWaitBBStart();
    RecordEncodeing = 0;
    RecordDmaTransting = 0;

    if ((RecordExitTaskID == TASK_ID_RADIO) && (FM_OUT_CONFIG == FM_I2S_HP))
    {
        if (REC_Flag == RECORD_PREPARE)
        {
            RecordEnable();
            REC_Flag = RECORD_BEING;
            RecordStaFlag &= ~RECORD_STA_PCMBUF_EMPTY;
            RecordStaFlag |= RECORD_STA_PCMBUF_FULL;
        }
    }
    else
    {
        timeout = 2000;

        while (DmaGetState(RECORD_DMACHANNEL_IIS) == DMA_BUSY)
        {
            DelayMs(1);

            if (--timeout == 0)
            {
                break;
            }
        }

        RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH);

        if (REC_Flag == RECORD_PREPARE)
        {
            RecordEnable();
            RecordDmaTransting = 1;
            DmaStart(RECORD_DMACHANNEL_IIS, RX_FIFO_ADDR, (uint32)pRecordPCMInputBuffer,
                     RecordInputBufferLength / 2, &RecordControlDmaCfg, RecordDmaIsrHandler);
            REC_Flag = RECORD_BEING;
            RecordStaFlag &= ~RECORD_STA_PCMBUF_EMPTY;
            RecordStaFlag |= RECORD_STA_PCMBUF_FULL;
            I2SStart(I2S_CH,I2S_START_DMA_RX);
        }
    }

    DEBUG("RecordStart Leave...");
}


/*
--------------------------------------------------------------------------------
  Function name : void RecordEncodeProc()
  Author        : WangBo
  Description   : the backgroud control program for recording.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
BOOLEAN RecordEncodeProc(MSG_ID id, void * msg)
{
    BOOLEAN ret = TRUE;

    switch (id)
    {
        case MSG_RECORD_TYPE:        //reverse interface,select record type(WAV MP3)
        {
            GetEncodeType();
            break;
        }

        case MSG_RECORD_GETINFO:
        {
            RecordGetInfo();
            break;
        }

        case MSG_RECORD_WRITEFILE:        //writer file
        {
            return ( RecordWriteFile());
        }

        case MSG_RECORD_INIT:
        {
            break;
        }

        case MSG_RECORD_STARTE:      //start record,write file head,timer interrupt start.
        {
            RecordStart();
            AutoPowerOffDisable();
            break;
        }

        case MSG_RECORD_PAUSE:      //record suspend,close timer interrupt,stop accept the sample value.
        {
            RecordPause();
            //AutoPowerOffEnable();//hj
            break;
        }

        case MSG_RECORD_RESUME:    //start timer interrupt.continue accept sample value.
        {
            RecordResume();
            AutoPowerOffDisable();
            break;
        }

        case MSG_RECORD_STOP:        //stop record.
        {
            RecordStop();
            //AutoPowerOffEnable(); //HJ
            break;
        }

        default:
            ret = FALSE;
            break;
    }

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecInit(void *pArg)
  Author        : WangBo
  Description   : record thread initialization.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-28          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecInit(void *pArg)
{
    if (MemorySelect != FLASH0)
    {
        FileSysSetup(FLASH0);
    }

    //overlay  flash write
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
    RecordExitTaskID = ((RECORD_WIN_ARG*)pArg)->RecordExitTaskID;
    RecordType    = ((RECORD_WIN_ARG*)pArg)->RecordType;
    RecordCodecConfig = ((RECORD_WIN_ARG*)pArg)->RecordCodecConfig;
    RecordSampleRate = ((RECORD_WIN_ARG*)pArg)->RecordSampleRate;
    RecordChannel = ((RECORD_WIN_ARG*)pArg)->RecordChannel;
    RecordDataWidth = ((RECORD_WIN_ARG*)pArg)->RecordDataWidth;
    RecordEncodeType = ((RECORD_WIN_ARG*)pArg)->RecordEncodeType;

    DEBUG("RecordExitTaskID = %d", RecordExitTaskID);
    DEBUG("RecordType = %s", (RecordType ==RECORD_TYPE_MIC_STERO)?"Mic Stero " : "Other") ;
    DEBUG("RecordCodecConfig =%d", RecordCodecConfig);
    DEBUG("RecordSampleRate = %d", RecordSampleRate);
    DEBUG("RecordChanel =     %d", RecordChannel);
    DEBUG("RecordDataWidth =  %d", RecordDataWidth);
    DEBUG("RecordEncodeType = %d", RecordEncodeType);

    //falsh write protect.
    FLASH_PROTECT_OFF();
    BBSystemInit();
    RegMBoxRecEncodeSvc();
    RegMBoxRecFileSvc();

    //DEBUG("----- record encode irq server registered------");
    RecordHWInit();
    DEBUG("----- RecInit OUT------");
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 RecService(void)
  Author        : WangBo
  Description   : record thread take care of write file and err handling.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-28          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
UINT32 RecService(void)
{
    int Retval = 0;

    if (REC_Flag == RECORD_BEING)
    {
        //put the write file handle to service cycle,constantly check buffer,if there is have 2k data,
        //write to flash.Note after processing this task,do not exit message handle function.
        //write file put the head of WavEncodeStop, avoid the lost data.
        RecordEncodeProc(MSG_RECORD_WRITEFILE,NULL);
    }

    return Retval;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecDeInit(void)
  Author        : WangBo
  Description   : record thread auti-initialization.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-28          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORD_CONTROL_CODE_
void RecDeInit(void)
{
    //close record file.auti-initialzation.
    RecordEncodeProc(MSG_RECORD_STOP, NULL); //Timer close interrupt.stop sampling receive value,stop code,write file,close file.
    DeRegMBoxRecEncodeSvc();
    DeRegMBoxRecFileSvc();
    BBSystemDeInit();
    //falsh write protect.
    FLASH_PROTECT_ON();
    FREQ_EnterModule(FREQ_MAX);

    if (MemorySelect != FLASH0)
    {
        FileSysSetup(MemorySelect);
    }

    FREQ_ExitModule(FREQ_MAX);
    DEBUG("----- RecDeInit OUT------");
}

#endif

