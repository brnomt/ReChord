/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                                    (c) Copyright 2013, RockChip.Ltd
*                                          All Rights Reserved
*File    : APP.C
* By      : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            DESCRIPTION
*  RK_NANO_OS is a system designed specifically for real-time embedded SOC operating system ,before using
*RK_NANO_OS sure you read the user's manual
*  The TASK NAME TABLE:
*
*
*  The DEVICE NAME TABLE:
*  "UartDevice",              Uart Serial communication devices
*  "ADCDevice",               The analog signal is converted to a digital signal device
*  "KeyDevice",               Key driver device
*
*
*
*
*
*
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include "SysInclude.h"
#include "stdio.h"
#include "mailbox.h"
#include "main2_msgbox.h"
#include "audio_globals.h"
#include "audio_main.h"
#include "fsinclude.h"
#include "audio_file_access.h"
#include "recordcontrol.h"

/*
*********************************************************************************************************
*                                        Macro Define
*********************************************************************************************************
*/
#define mainTimer_TASK_PRIORITY     configMAX_PRIORITIES-1
#define mainTest1_TASK_PRIORITY     2
#define mainTest2_TASK_PRIORITY     1
#define mainTest_QUEUE_SIZE         3

/*
*********************************************************************************************************
*                                        Variable Define
*********************************************************************************************************
*/
_ATTR_BB_SYS_DATA_
uint32 gCmd = 0;
_ATTR_BB_SYS_DATA_
uint32 gData = 0;
_ATTR_BB_SYS_DATA_
uint8  gDecStatus = 0;

_ATTR_BB_SYS_DATA_
uint8  gFileOpStatus = 0;

_ATTR_BB_SYS_DATA_
uint32 BBSystemHoldTimeout = 0;

extern uint8   gCmdDone;
extern uint32  CodecBufSize2;
extern uint16  gPCMDataWidth;

typedef struct _BBCore_Pcb_
{
    uint32 audio_decode_status;
    uint32 audio_decode_param;
}BbCore_PCB;

enum CORE2_RUN_STATUS
{
    //audio decode
    AUDIO_DECODE_IDLE = 0,
    AUDIO_DECODE_OPEN,
    AUDIO_DECODE_DECODE,
    AUDIO_DECODE_SEEK,
    AUDIO_DECODE_GETBUFFER,
    AUDIO_DECODE_GETTIME,
    AUDIO_DECODE_CLOSE,

    //file operate
    AUDIO_DECODE_FILE_OPEN,
    AUDIO_DECODE_FILE_SEEK,
    AUDIO_DECODE_FILE_READ,
    AUDIO_DECODE_FILE_TELL,
    AUDIO_DECODE_FILE_GETLENGTH,
    AUDIO_DECODE_FILE_CLOSE,

    //record encord cmd
    RECORD_ENCORD_INIT,
    RECORD_ENCORD_ENCODE,
    RECORD_ENCORD_GETDATA,
    RECORD_ENCORD_CLOSE,
};


enum AUDIO_FILE_OPT_STATUS
{
    AUDIO_FILE_OPT_IDLE = 0,
    AUDIO_FILE_OPEN_CMPL,
    AUDIO_FILE_SEEK_CMPL,
    AUDIO_FILE_READ_CMPL,
    AUDIO_FILE_WRITE_CMPL,
    AUDIO_FILE_TELL_CMPL,
    AUDIO_FILE_GET_LEN_CMPL,
    AUDIO_FILE_CLOSE_CMPL,

    RECORD_FILE_CREATE_CMPL,
    RECORD_FILE_CLOSE_CMPL,
};

static BbCore_PCB pcb;
static MediaBlock gMediaBlockInfo;

static FILE_HANDLE_t * gFileHandle;
static RecFileHdl_t  * gRecFileHdl;
static RecordBlock     gRecordBlockInfo;
static uint8 *         gRecWavEncDataBuf;
static unsigned char * gRecMp3EncDataBuf;
/*
*********************************************************************************************************
*                                      extern varible
*********************************************************************************************************
*/
extern int CurrentCodec2;
extern uint32 FileTotalSize;
extern uint32 CurFileOffset[8];
extern uint32  AudioFileBufBusy2;
extern uint32  AudioFileBufSize2[2];
extern uint32  AudioFileWrBufID2;
extern FILE_READ_OP_t gFileReadParam;
extern uint8 gBufByPass;


#if(defined WAV_DEC_INCLUDE2) || (defined MP3_ENC_INCLUDE2)
extern INT16    hRecFile;
extern uint32   RecFileOffset;
extern tPCM_enc PCM_source;
extern uint32   EncodeInputBufferPIPO2[WAV_AD_PIPO_BUFFER_SIZE];  //Buffer must align 4
extern UINT16   *pRecordEncodeInputBuffer2;
extern UINT16   *pRecordPCMInputBuffer2;
extern UINT16   *pRecordInputBuffer2;

//extern uint32   MP3_EncodeOutputBuffer[1152];  //Buffer must align 4
UINT8   *pMP3_EncodeOutputBuffer;
UINT32  pMP3_EncodeOutputlen;
UINT32  pMP3_EncodeLOSTFramecount;
extern SF_PRIVATE sf_enc;

extern uint16 RecordInputBufIndex;
uint32 Get_NS_Init_len(unsigned int sampleRate,unsigned int channel,unsigned int encodeType)          //ebcode channel)
{
    uint32 len = 0;

    if (encodeType == RECORD_ENCODE_TYPE_WAV)
    {
        if (channel == RECORD_CHANNEL_STERO)
        {
            switch (sampleRate)
            {
                case RECORD_FS_8000Hz:
                case RECORD_FS_11025Hz:
                case RECORD_FS_12KHz:
                case RECORD_FS_16KHz:
                    len = 122;
                    break;

                case RECORD_FS_22050Hz:
                case RECORD_FS_24KHz:
                    len = 250;

                    break;

                case RECORD_FS_32KHz:
                case RECORD_FS_44100Hz:
                case RECORD_FS_48KHz:
                    len = 506;
                    break;

                default:
                    break;

            }
        }
        else
        {
            switch (sampleRate)
            {
                case RECORD_FS_8000Hz:
                case RECORD_FS_11025Hz:
                case RECORD_FS_12KHz:
                case RECORD_FS_16KHz:
                    len = 122;
                    break;

                case RECORD_FS_22050Hz:
                case RECORD_FS_24KHz:
                    len = 253;

                    break;

                case RECORD_FS_32KHz:
                case RECORD_FS_44100Hz:
                case RECORD_FS_48KHz:
                    len = 509;
                    break;

                default:
                    break;

            }
        }
    }
    else if(encodeType == RECORD_ENCODE_TYPE_REV)
    {
        //reserve for other record format...
        switch (sampleRate)
        {

            default:
                break;
        }
    }
    return len;
}
#endif
/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
#ifdef _LOG_DEBUG_
//this fun just for debuging
static uint8* Cmd2String2(uint32 cmd)
{
    uint8* pstr;

    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_FILE_OPEN_CMPL:
            pstr = "FILE OPEN CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL:
            pstr = "FILE SEEK CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_READ_CMPL:
            pstr = "FILE READ CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL:
            pstr = "FILE WRITE CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_TELL_CMPL:
            pstr = "FILE TELL CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL:
            pstr = "FILE GET_LEN CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL:
            pstr = "FILE CLOSE CMPL";
            break;

        default:
            pstr = "NOT FOUND CMD";
            break;
    }

    return pstr;
}

void dumpMemoryByte(uint8* addr, uint32 size)
{
    uint32 index = 0;
    uint8 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        bb_printf1("%02x", *ptemp);
        ptemp++;
    }

}

void dumpMemoryShort(uint16* addr, uint32 size)
{
    uint32 index = 0;
    uint8 data[2];
    uint16 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        if (index == 0)
            bb_printf1("\r\n");

        if (index % 8 == 0 && index != 0)
            bb_printf1("\n");

        data[1] = *ptemp >> 8 & 0xff;
        data[0] = *ptemp & 0xff;

        bb_printf1("%02x ", data[1] );
        bb_printf1("%02x ", data[0] );
        ptemp++;
    }
}

void dumpMemoryInt(uint32* addr, uint32 size)
{
    uint32 index = 0, i;
    uint8 data[4];
    uint32 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        if (index == 0)
            bb_printf1("\r\n");

        if (index % 4 == 0 && index != 0)
            bb_printf1("\n");

        data[3] = *ptemp >> 24 & 0xff;
        data[2] = *ptemp >> 16 & 0xff;
        data[1] = *ptemp >> 8  & 0xff;
        data[0] = *ptemp & 0xff;

        for (i = 4; i > 0; i--)
        {
            bb_printf1("%02x ", data[i - 1] );
        }

        ptemp++;
    }
}

#endif

__irq  void MailBoxDecService()
{
    uint32 cmd;
    uint32 data;
    MailBoxClearA2BInt(MAILBOX_ID_0, MAILBOX_INT_1);

    cmd = MailBoxReadA2BCmd(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    data = MailBoxReadA2BData(MAILBOX_ID_0, MAILBOX_CHANNEL_1);

    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_DEC_OPEN:
            pcb.audio_decode_status = AUDIO_DECODE_OPEN;
            break;

        case MEDIA_MSGBOX_CMD_DECODE:
            pcb.audio_decode_status = AUDIO_DECODE_DECODE;
            break;

        case MEDIA_MSGBOX_CMD_DECODE_GETBUFFER:
            pcb.audio_decode_status = AUDIO_DECODE_GETBUFFER;
            break;

        case MEDIA_MSGBOX_CMD_DECODE_GETTIME:
            pcb.audio_decode_status = AUDIO_DECODE_GETTIME;
            break;

        case MEDIA_MSGBOX_CMD_DECODE_SEEK:
            pcb.audio_decode_status = AUDIO_DECODE_SEEK;
            pcb.audio_decode_param  = data;
            break;

        case MEDIA_MSGBOX_CMD_DECODE_CLOSE:
            pcb.audio_decode_status = AUDIO_DECODE_CLOSE;
            pcb.audio_decode_param  = data;
            break;

//------------------------------------------------------------------------------
        case MEDIA_MSGBOX_CMD_ENCODE_INIT:
            pcb.audio_decode_status = RECORD_ENCORD_INIT;
            pcb.audio_decode_param  = data;
            break;

        case MEDIA_MSGBOX_CMD_ENCODE:
            pcb.audio_decode_status = RECORD_ENCORD_ENCODE;
            pcb.audio_decode_param  = data;
            break;
//------------------------------------------------------------------------------
        default:
            break;
    }
}

void RegHifiDecodeServer()
{
    IntRegister2(INT_ID_MAILBOX1 , (void*)MailBoxDecService);
    IntPendingClear2(INT_ID_MAILBOX1);
    IntEnable2(INT_ID_MAILBOX1);
    MailBoxEnableA2BInt(MAILBOX_ID_0, MAILBOX_INT_1);
}


__irq void MailBoxFileService()
{
    MailBoxClearA2BInt(MAILBOX_ID_0, MAILBOX_INT_2);

    gCmd = MailBoxReadA2BCmd(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    gData = MailBoxReadA2BData(MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    switch (gCmd)
    {
        case MEDIA_MSGBOX_CMD_FILE_OPEN_CMPL:
            gFileHandle = (FILE_HANDLE_t *)gData;
            pRawFileCache = (FILE *)gFileHandle->handle1;
            FileTotalSize = gFileHandle->filesize;
            CurFileOffset[(uint32)pRawFileCache] = gFileHandle->curfileoffset[0];
            CurrentCodec2 = gFileHandle->codecType;

#ifdef AAC_DEC_INCLUDE
            if (CurrentCodec2 == CODEC_AAC_DEC)
            {
                pAacFileHandleOffset = (FILE *)gFileHandle->handle2;
                CurFileOffset[(uint32)pAacFileHandleOffset] = gFileHandle->curfileoffset[1];
                pAacFileHandleSize = (FILE *)gFileHandle->handle3;
                CurFileOffset[(uint32)pAacFileHandleSize] = gFileHandle->curfileoffset[2];
            }
#endif
#ifdef HIFI_AlAC_DECODE
            if (CurrentCodec2 == CODEC_HIFI_ALAC_DEC)
            {
                pAacFileHandleOffset = (FILE *)gFileHandle->handle2;
                CurFileOffset[(uint32)pAacFileHandleOffset] = gFileHandle->curfileoffset[1];
                pAacFileHandleSize = (FILE *)gFileHandle->handle3;
                CurFileOffset[(uint32)pAacFileHandleSize] = gFileHandle->curfileoffset[2];
            }
#endif

#ifdef FLAC_DEC_INCLUDE
            if (CurrentCodec2 == CODEC_FLAC_DEC)
            {
                pFlacFileHandleBake = (FILE *)gFileHandle->handle2;
                CurFileOffset[(uint32)pFlacFileHandleBake] = gFileHandle->curfileoffset[1];
            }
#endif

            gFileOpStatus = AUDIO_FILE_OPEN_CMPL;

            break;

        case MEDIA_MSGBOX_CMD_FILE_CREATE_CMPL:
#if(defined WAV_DEC_INCLUDE2) || (defined MP3_ENC_INCLUDE2)
        {
            gRecFileHdl   = (RecFileHdl_t *)gData;
            hRecFile      = gRecFileHdl->fileHandle;
            RecFileOffset = gRecFileHdl->fileOffset;

            gFileOpStatus = RECORD_FILE_CREATE_CMPL;
        }

#endif
        break;

        case MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL:
            gFileOpStatus = AUDIO_FILE_SEEK_CMPL;
            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FILE_READ_CMPL:
            gFileOpStatus = AUDIO_FILE_READ_CMPL;

            if (gBufByPass == 0)
            {
                CurFileOffset[gFileReadParam.handle] = gFileReadParam.NumBytes;

                AudioFileBufSize2[AudioFileWrBufID2] = gData;

                AudioFileBufBusy2 = 0;

                gBufByPass = 1;

            }

            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL:
            gFileOpStatus = AUDIO_FILE_WRITE_CMPL;
            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FILE_TELL_CMPL:
            gFileOpStatus = AUDIO_FILE_TELL_CMPL;
            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL:
            gFileOpStatus = AUDIO_FILE_GET_LEN_CMPL;
            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL:

            if (gData)
                gFileOpStatus = AUDIO_FILE_CLOSE_CMPL;

            gCmdDone = 1;
            break;

        //record file close
        case MEDIA_MSGBOX_CMD_REC_FILE_CLOSE:
            if (gData)
                gFileOpStatus = RECORD_FILE_CLOSE_CMPL;

            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_CMPL:
        case MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_INFO_CMPL:
            gCmdDone = 1;
            break;


        default:
            return;
    }

}

void RegHifiFileServer()
{
    gCmdDone   = 0;
    IntRegister2(INT_ID_MAILBOX2 , (void*)MailBoxFileService);
    IntPendingClear2(INT_ID_MAILBOX2);
    IntEnable2(INT_ID_MAILBOX2);
    MailBoxEnableA2BInt(MAILBOX_ID_0, MAILBOX_INT_2);
}
/*
*********************************************************************************************************
*                                              Main(void)
*
* Description:  This Function is the first function.
*
* Argument(s) : none
*
* Return(s)   : int
*
* Note(s)     : none.
*********************************************************************************************************
*/



/*
--------------------------------------------------------------------------------
  Function name :
  Author        : ZHengYongzhi
  Description   : ģ����Ϣ��

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

extern uint32 Image$$BB_SYS_DATA$$ZI$$Base;
extern uint32 Image$$BB_SYS_DATA$$ZI$$Length;

void ScatterLoader2(void)
{
    uint32 i, len;
    uint8  *pDest;

    //���Bss��
    pDest = (uint8*)((uint32)(&Image$$BB_SYS_DATA$$ZI$$Base));
    len = (uint32)((uint32)(&Image$$BB_SYS_DATA$$ZI$$Length));

    for (i = 0; i < len; i++)
    {
        *pDest++ = 0;
    }
}

int Main2(void)
{
    uint outptr;
    uint OutLength;

    ScatterLoader2();

#ifdef BB_SYS_JTAG
    MailBoxWriteB2ACmd(MSGBOX_CMD_SYSTEM_START_OK, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
#endif

    BSP_Init2();

    RegHifiDecodeServer();

    RegHifiFileServer();

    ClearMsg(MSG_AUDIO_DECODE_FILL_BUFFER);

    MailBoxWriteB2ACmd(MSGBOX_CMD_SYSTEM_START_OK, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_0);

    MemSet2(&gMediaBlockInfo,  0, sizeof(gMediaBlockInfo));
    MemSet2(&gRecordBlockInfo, 0, sizeof(gRecordBlockInfo));

    while (1)
    {
        /* ReChord V0.17 heartbeat: flip the whole framebuffer 0x03024868
         * red/black AND push it through the loader's own display API
         * (fef124 wait -> fea848/fea824 save -> feb0f6 color -> fea8f4
         * rect -> feabea refresh -> restore), slowly (~4 Hz) so it is
         * visible. V0.1 (this SDK-kernel boot path) was the ONLY build
         * that showed colors on screen; V0.8-V0.16 pure-C builds over the
         * ROM never showed anything. Also write the BOOT_DONE marker so
         * rechord_ui_event (application_start stub) can draw too.
         * RECHORD_QEMU_TEST build: fb/boot-marker/ROM-display calls are
         * redirected to QEMU RAM + stubs (qemu_echo_main.c); the delay is
         * lengthened so the monitor can catch a red phase. */
#ifdef RECHORD_QEMU_TEST
        extern uint32_t rch_qemu_rom_wait(uint32_t);
        extern uint32_t rch_qemu_rom_ctx(uint32_t);
        extern void rch_qemu_rom_color(uint32_t);
        extern void rch_qemu_rom_rect(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
        extern void rch_qemu_rom_refresh(uint32_t);
        #define RCH_QEMU_FB      ((volatile uint16_t *)0x20010000u)
        #define RCH_QEMU_BOOTLOG ((volatile uint32_t *)0x20008018u)
        #define RCH_QEMU_WAIT    rch_qemu_rom_wait
        #define RCH_QEMU_CTX     rch_qemu_rom_ctx
        #define RCH_QEMU_COLOR   rch_qemu_rom_color
        #define RCH_QEMU_RECT    rch_qemu_rom_rect
        #define RCH_QEMU_REFR    rch_qemu_rom_refresh
        #define RCH_QEMU_DELAY   60000000u
#else
        #define RCH_QEMU_FB      ((volatile uint16_t *)0x03024868u)
        #define RCH_QEMU_BOOTLOG ((volatile uint32_t *)0x03000118u)
        #define RCH_QEMU_DELAY   4000000u
#endif
        {
            static uint32 hb = 0;
            uint32_t col = (hb & 1) ? 0xF800u : 0x0000u;   /* red / black */
            uint32_t i;
            volatile uint16_t *fb = RCH_QEMU_FB;
            for (i = 0; i < (320 * 170) / 2; i++)          /* whole fb */
                fb[i] = (uint16_t)col;
            /* loader display API (known-good top-bar rect + adjacent
             * palette codes 0x94/0x95 — the loader's status-bar colors) */
#ifdef RECHORD_QEMU_TEST
            if (RCH_QEMU_WAIT(0x19b) == 0) {
                uint32_t c1 = RCH_QEMU_CTX(1);
                uint32_t c2 = RCH_QEMU_CTX(2);
                RCH_QEMU_COLOR((hb & 1) ? 0x95u : 0x94u);
                RCH_QEMU_RECT(0, 3, 320, 16, 2, 0x58);
                RCH_QEMU_REFR(1);
                RCH_QEMU_CTX(c1);
                RCH_QEMU_CTX(c2);
            }
#else
            /* ROM display API DISABLED — the hardcoded 0x02feXXXX addresses
             * were wrong (zero occurrences in the stock IMG, and even-valued
             * ARM-state pointers into a Thumb ROM), so they faulted here and
             * prevented Main2 from ever reaching the mailbox-decode loop.
             * We keep only the safe framebuffer write + delay. */
            (void)hb;
#endif
            /* BOOT_DONE marker (0xfeed0002) for rechord_ui_event */
            RCH_QEMU_BOOTLOG[2] = 0xfeed0002u;
            hb++;
            for (i = 0; i < RCH_QEMU_DELAY; i++) ;
        }

        //system enter IDLE
        IntMasterDisable2();
        if(0) /* ReChord debug: no WFI so the heartbeat is visible */
        {
            __WFI2();
        }
        IntMasterEnable2();

        #if 1
        if (BBSystemHoldTimeout == 1)       //only for debug
        {
            bb_printf1("FreqChange timeout!!!");
            BBSystemHoldTimeout = 0;
        }
        #endif

        //process audio decode
        switch (pcb.audio_decode_status)
        {
            case AUDIO_DECODE_IDLE:
#if 0
                IntMasterDisable2();
                if((pcb.audio_decode_status == AUDIO_DECODE_IDLE) && (gFileOpStatus == AUDIO_FILE_OPT_IDLE))
                {
                    __WFI2();
                }

                IntMasterEnable2();
#endif
                break;

            case AUDIO_DECODE_OPEN:
            {
                pcb.audio_decode_status = AUDIO_DECODE_IDLE;

                if (1 != CodecOpen2(0, CODEC_OPEN_DECODE))
                {
                    bb_printf1("###AUDIO_DECODE_OPEN error!###");
                    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DEC_OPEN_ERR, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    break;
                }

                {
                    int retflag;
                    CodecGetSampleRate2(&gMediaBlockInfo.SampleRate);
                    CodecGetChannels2(&gMediaBlockInfo.Channel);
                    retflag = CodecGetBitrate2(&gMediaBlockInfo.BitRate);

                    if (retflag == 0)
                    {
                        //bb_printf1("BitRate = %d,Invalid bitrate number",gMediaBlockInfo.BitRate);
                        bb_printf1("###AUDIO_DECODE_OPEN error!###");
                        MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DEC_OPEN_ERR, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                        MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                        break;
                    }

                    if (gMediaBlockInfo.Channel > 2)
                    {
                        //bb_printf1("Channels = %d, > 2",gMediaBlockInfo.Channel);
                        bb_printf1("### channels bigger than 2!###");
                        MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DEC_OPEN_ERR, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                        MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                        break;
                    }

                    CodecGetLength2(&gMediaBlockInfo.TotalPlayTime);
                    CodecGetBps2(&gMediaBlockInfo.Bps);


#ifdef MP3_DEC_INCLUDE2

                    if (CurrentCodec2 == CODEC_MP3_DEC)
                    {
                        mp3_wait_synth();
                    }

#endif

                    AudioCodecGetBufferSize2(CurrentCodec2, gMediaBlockInfo.SampleRate);

                    //AudioCodecGetBufferSize2(CurrentCodec2,gMediaBlockInfo.SampleRate);

#ifdef FLAC_DEC_INCLUDE2
                    if (CurrentCodec2 != CODEC_FLAC_DEC)
#endif
                    {
#ifdef AAC_DEC_INCLUDE2
                        //if (CurrentCodec2 != CODEC_AAC_DEC)
#endif
                        {
#ifdef HIFI_AlAC_DECODE2
                            //if (CurrentCodec2 != CODEC_HIFI_ALAC_DEC)
#endif
                            {
                                AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
                            }
                        }
                    }

                    //bb_printf1("SampleRate = %d",gMediaBlockInfo.SampleRate);
                    //bb_printf1("Channel = %d",gMediaBlockInfo.Channel);
                    //bb_printf1("BitRate = %d",gMediaBlockInfo.BitRate);
                    //bb_printf1("CurrentPlayTime = %d",gMediaBlockInfo.CurrentPlayTime);
                    //bb_printf1("TotalPlayTime = %d",gMediaBlockInfo.TotalPlayTime);
                    //bb_printf1("Bps = %d",gMediaBlockInfo.Bps);
                }

                CodecGetCaptureBuffer2((short*)&gMediaBlockInfo.Outptr, &gMediaBlockInfo.OutLength);

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DEC_OPEN_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData((UINT32)&gMediaBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;

            case AUDIO_DECODE_DECODE:
            {
                //bb_printf1("AUDIO_DECODE_DECODE \n");
                pcb.audio_decode_status = AUDIO_DECODE_IDLE;

                if (GetMsg(MSG_AUDIO_DECODE_FILL_BUFFER))
                {
                    AudioFileInput2(pRawFileCache);
                }

                if (1 != CodecDecode2())
                {
                    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_ERR, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    break;
                }

#ifdef MP3_DEC_INCLUDE2

                if (CurrentCodec2 == CODEC_MP3_DEC)
                {
                    mp3_wait_synth();
                }

#endif

                CodecGetTime2(&gMediaBlockInfo.CurrentPlayTime);

                CodecGetCaptureBuffer2((short*)&gMediaBlockInfo.Outptr, &gMediaBlockInfo.OutLength);

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData((UINT32)&gMediaBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;

            case AUDIO_DECODE_SEEK:
            {
                pcb.audio_decode_status = AUDIO_DECODE_IDLE;

                if ( 1 != CodecSeek2(pcb.audio_decode_param , 0))
                {
                    //bb_printf1("codec seek fail");
                    //TODO...
                    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteB2AData(1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);    //seek fail,send data '1'
                }
                else
                {
                    //bb_printf1("codec seek ok");
                    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);    //seek susscess,send data '0'
                }
            }
            break;

            case AUDIO_DECODE_GETBUFFER:
            {
//                CodecGetCaptureBuffer2((short*)&gMediaBlockInfo->Outptr,&gMediaBlockInfo->OutLength);
//
//                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_GETBUFFER_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
//                MailBoxWriteB2AData((UINT32)gMediaBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;

            case AUDIO_DECODE_GETTIME:
            {
                /*
                pcb.audio_decode_status = AUDIO_DECODE_IDLE;
                CodecGetTime2(&gMediaBlockInfo.CurrentPlayTime);

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_GETTIME_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData((UINT32)&gMediaBlockInfo,MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                */
            }
            break;

            case AUDIO_DECODE_CLOSE:
            {
                pcb.audio_decode_status = AUDIO_DECODE_IDLE;
                CodecClose2();
                //bb_printf1("AUDIO_DECODE_CLOSE");

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;


            case AUDIO_DECODE_FILE_OPEN:
            {
                pcb.audio_decode_status = AUDIO_DECODE_IDLE;
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);

                //RKFileFuncInit();
            }
            break;

//------------------------------------------------------------------------------
            case RECORD_ENCORD_INIT:
#if(defined WAV_DEC_INCLUDE2) || (defined MP3_ENC_INCLUDE2)
            {
                pcb.audio_decode_status = AUDIO_DECODE_IDLE;
                MemCpy2((uint8*)&gRecordBlockInfo, (uint8*)pcb.audio_decode_param, sizeof(gRecordBlockInfo));

                switch ( gRecordBlockInfo.encodeType )
                {
                    case RECORD_ENCODE_TYPE_PCM:
                    case RECORD_ENCODE_TYPE_WAV:
                    {
#ifdef WAV_DEC_INCLUDE2
                        uint32 NSLen;

                        if (gRecordBlockInfo.FilterFlag == 1)
                        {
                            NSLen = Get_NS_Init_len(gRecordBlockInfo.sampleRate, gRecordBlockInfo.channel, gRecordBlockInfo.encodeType);
                            NS_init(NSLen, gRecordBlockInfo.sampleRate);
                        }

                        gPCMDataWidth = gRecordBlockInfo.dataWidth;
                        MemCpy2(&PCM_source, (tPCM_enc*)gRecordBlockInfo.PCM_source, sizeof(PCM_source));
                        RecordADPCMInit(&PCM_source);
                        RecordBufferInit();

                        gRecordBlockInfo.PCM_source = (UINT32)&PCM_source;
                        pRecordEncodeInputBuffer2 = pRecordPCMInputBuffer2;
                        gRecordBlockInfo.unenc_bufptr = (UINT32)pRecordEncodeInputBuffer2;

                        MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_ENCODE_INIT_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                        MailBoxWriteB2AData((UINT32)&gRecordBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
#endif
                    }
                    break;

                    case RECORD_ENCODE_TYPE_REV:
                    {
                        //reserve other record format
                    }
                    break;

                    default:
                        break;
                }
            }
#endif
            break;

            case RECORD_ENCORD_ENCODE:
            {
                pcb.audio_decode_status = AUDIO_DECODE_IDLE;

                switch ( gRecordBlockInfo.encodeType )
                {
                    case RECORD_ENCODE_TYPE_PCM:
#ifdef WAV_DEC_INCLUDE2
                    {
                        uint32 EncInputDataLen;
                        uint32 EncOutputDataLen;

                        EncInputDataLen = pcb.audio_decode_param;

                        if (gRecordBlockInfo.FilterFlag == 1)
                        {
                            NS_do(gRecordBlockInfo.unenc_bufptr, EncInputDataLen / 2);
                        }

                        gRecordBlockInfo.enc_bufptr = (UINT32)pRecordEncodeInputBuffer2;
                        gRecordBlockInfo.length = pcb.audio_decode_param;
                        RecordInputBufIndex   = 1 - RecordInputBufIndex;
                        pRecordPCMInputBuffer2    = pRecordInputBuffer2 + RecordInputBufIndex * WAV_AD_PIPO_BUFFER_SIZE;  //switch buffer
                        pRecordEncodeInputBuffer2 = pRecordPCMInputBuffer2;
                        gRecordBlockInfo.unenc_bufptr = (UINT32)pRecordEncodeInputBuffer2;

                        MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_ENCODE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                        MailBoxWriteB2AData((UINT32)&gRecordBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    }
#endif
                    break;

                    case RECORD_ENCODE_TYPE_WAV:
#ifdef WAV_DEC_INCLUDE2
                    {
                        uint32 EncInputDataLen;
                        uint32 EncOutputDataLen;

                        EncInputDataLen = pcb.audio_decode_param;

                        if (gRecordBlockInfo.FilterFlag == 1)
                        {
                            NS_do(gRecordBlockInfo.unenc_bufptr, EncInputDataLen / 2);
                        }

                        EncOutputDataLen = msadpcm_write_s(&sf_enc, gRecordBlockInfo.unenc_bufptr , EncInputDataLen / 2, &gRecWavEncDataBuf);

                        gRecordBlockInfo.enc_bufptr = (UINT32)gRecWavEncDataBuf;
                        RecordInputBufIndex   = 1 - RecordInputBufIndex;
                        pRecordPCMInputBuffer2    = pRecordInputBuffer2 + RecordInputBufIndex * WAV_AD_PIPO_BUFFER_SIZE;  //switch buffer
                        pRecordEncodeInputBuffer2 = pRecordPCMInputBuffer2;
                        gRecordBlockInfo.unenc_bufptr = (UINT32)pRecordEncodeInputBuffer2;
                        gRecordBlockInfo.length = EncOutputDataLen;

                        MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_ENCODE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                        MailBoxWriteB2AData((UINT32)&gRecordBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    }
#endif
                    break;

                    case RECORD_ENCODE_TYPE_REV:
                    {
                        //reserve for other record format...
                    }
                    break;

                    default:
                        break;
                }
            }
            break;

            default :
                break;
        }

        //process file operation
        if (GetMsg(MSG_AUDIO_DECODE_FILL_BUFFER))
        {
            AudioFileInput2(pRawFileCache);
        }

        switch (gFileOpStatus)
        {
            case AUDIO_FILE_OPT_IDLE:
#if 0
                IntMasterDisable2();
                if((pcb.audio_decode_status == AUDIO_DECODE_IDLE) && (gFileOpStatus == AUDIO_FILE_OPT_IDLE))
                {
                    __WFI2();
                }

                IntMasterEnable2();
#endif
                break;

            case AUDIO_FILE_OPEN_CMPL:
            {
                gFileOpStatus = AUDIO_FILE_OPT_IDLE;
                AudioIntAndDmaInit2();
                AudioHWInit2();

                AudioCodecGetBufferSize2(CurrentCodec2, FS_44100Hz);

                if (CurrentCodec2 == CODEC_MP3_DEC)
                {
                    AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
                }

#if 1
#ifdef AAC_DEC_INCLUDE
                else if (CurrentCodec2 == CODEC_AAC_DEC)
                {
                    ClearMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
                    //RKFileFuncInit2();
                    //HifiFileSeek(0, SEEK_SET, pRawFileCache);

                    AudioFileMhFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
                    //AudioFileMhSeek2(0, SEEK_SET, pRawFileCache);
                }
#endif
#ifdef HIFI_AlAC_DECODE
                else if (CurrentCodec2 == CODEC_HIFI_ALAC_DEC)
                {
                    ClearMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
                    //RKFileFuncInit2();
                    //HifiFileSeek(0, SEEK_SET, pRawFileCache);

                    AudioFileMhFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
                    //AudioFileMhSeek2(0, SEEK_SET, pRawFileCache);
                }
#endif
#endif
                else
                {
                    AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
                }

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_OPEN_HANDSHK, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            break;

            case AUDIO_FILE_SEEK_CMPL:
                gFileOpStatus = AUDIO_FILE_OPT_IDLE;
                break;

            case AUDIO_FILE_READ_CMPL:
                gFileOpStatus = AUDIO_FILE_OPT_IDLE;
                break;

            case AUDIO_FILE_WRITE_CMPL:
                gFileOpStatus = AUDIO_FILE_OPT_IDLE;
                break;

            case AUDIO_FILE_TELL_CMPL:
                gFileOpStatus = AUDIO_FILE_OPT_IDLE;
                break;

            case AUDIO_FILE_GET_LEN_CMPL:
                gFileOpStatus = AUDIO_FILE_OPT_IDLE;
                break;

            case AUDIO_FILE_CLOSE_CMPL:
            {
                gFileOpStatus = AUDIO_FILE_OPT_IDLE;
                AudioIntAndDmaDeInit2();
                AudioHWDeInit2();

                //bb_printf1("file close cmpl");

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_CLOSE_HANDSHK, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            break;

//------------------------------------------------------
            case RECORD_FILE_CREATE_CMPL:    //used for record
                gFileOpStatus = AUDIO_FILE_OPT_IDLE;
                RKFileFuncInit2();
                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_CREATE_HANDSHK, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                break;

            case RECORD_FILE_CLOSE_CMPL:
                gFileOpStatus = AUDIO_FILE_OPT_IDLE;
                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_REC_FILE_CLOSE_HANDSHK, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                break;
        }
    }
}


