/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  SysService.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:
********************************************************************************
*/
#define _IN_SYSSERVICE_

#include "SysInclude.h"

#include "display/MenuResourceID.h"
#include "ui/MainMenu/MainMenu.h"
#include "filesys/fat.h"            /* USBHOST_FLASH & disk IDs (vendor) */    /* MAINMENU_ID_* menu IDs */ /* MAINMENU_ID_* */
#include "driver/USB/USBConfig.h"  /* USB_CLASS_TYPE_*, USBHOST_FLASH */
#include "driver/NVIC/hw_nvic.h"   /* nvic, NVIC_SYSTICKCTRL_* */
#include "driver/CRU/Hw_cru.h"     /* CRU instance */
#include "driver/CRU/cru.h"        /* chip_freq */
#include "driver/pmc/pmc.h"         /* PMU control API */
#include "driver/PMU/hw_pmu.h"         /* Pmu_Reg instance */
#include "driver/GRF/hw_grf.h"     /* Grf instance */
#include "driver/GRF/grf.h"        /* NOC_BOOT_ROM enum */
#include "driver/GPIO/gpio.h"       /* GPIO_CH0..2 */

#include "driver/DMA/Hw_dma.h"   /* DMA_CFGX, DMA_CTLL_* */
#include "driver/DMA/Dma.h"      /* DMA_CHN_MAX & channel API */
#include "driver/IIS/Hw_i2s.h"   /* I2s_Reg */
#include "UsbAdapterProbe.h"

#include "FunUSBInterface.h"
#include "fsinclude.h"
#include "mainmenu.h"
#include "RecordWinInterface.h"
#include "FMControl.h"
#include "AudioControl.h"
#include "VideoControl.h"
#include "PowerOn_Off.h"
#include "Fade.h"
#include "MediaBroWin.h"
#include "rockcodec.h"

extern void AudioSbcEncodeInit(void);
extern BOOLEAN VideoPause(void);

#if(CODEC_CONFIG == CODEC_ROCKC)
_ATTR_SYS_DATA_ DMA_CFGX AudioControlDmaCfg  = {DMA_CTLL_I2S0_TX, DMA_CFGL_I2S0_TX, DMA_CFGH_I2S0_TX, 0};
#else   //other codec( which not in i2s channel 0) dma config
_ATTR_SYS_DATA_ DMA_CFGX AudioControlDmaCfg  = {DMA_CTLL_I2S1_TX, DMA_CFGL_I2S1_TX, DMA_CFGH_I2S1_TX, 0};
#endif

////////////////////////////////////////////////////////////////////////////////
_ATTR_OVERLAY_CODE_
void GetBeepSourceInf(uint32 ModuleNum, uint32 *baseAddr, uint32 *moduleLen)
{
    uint32 i, Len;
    uint32 CodeInfoAddr;
    uint32 LoadStartBase;
    uint32 LoadBase;
    uint32 ImageBase;
    uint32 ImageLength;
    uint8  FlashBuf[512];
    uint8  *pBss;
    uint8  tempIsr;
    CODE_INFO_T Module;

    FIRMWARE_INFO_T *pFirmwareModuleInfo;

    //�ȶ�ȡ�̼������ʼ��ַ
    MDReadData(SysDiskID, CodeLogicAddress, 512, FlashBuf);
    pFirmwareModuleInfo = (FIRMWARE_INFO_T *)FlashBuf;
    LoadStartBase = pFirmwareModuleInfo -> LoadStartBase;

    //��ȡģ����ϢCODE_INFO_T
    CodeInfoAddr  = CodeLogicAddress + sizeof(pFirmwareModuleInfo -> LoadStartBase);
    CodeInfoAddr  = CodeInfoAddr + sizeof(pFirmwareModuleInfo -> ModuleInfo.ModuleNum) + ModuleNum * sizeof(CODE_INFO_T);
    MDReadData(SysDiskID, CodeInfoAddr, sizeof(CODE_INFO_T), FlashBuf);
    Module = *(CODE_INFO_T *)FlashBuf;


    *baseAddr    = Module.CodeLoadBase - LoadStartBase + CodeLogicAddress;
    *moduleLen   = Module.CodeImageLength;

}

_ATTR_SYS_CODE_
void AudioDmaIsrHandler(void)
{
    if (AudioDecodeing == 1)
    {
        //printf("\nAudioDmaIsrHandler AudioDecodeing = 1");
        DmaTransting = 0;
        return;
    }

    if (AUDIO_STATE_PLAY == AudioPlayState)
    {
        DmaStart(AUDIO_DMACHANNEL_IIS, (UINT32)AudioPtr,
                 (uint32)(&(I2s_Reg->I2S_TXDR)),  AudioLen,
                 &AudioControlDmaCfg, AudioDmaIsrHandler);
        AudioDecodeing = 1;
        UserIsrRequest();
    }
    else
    {
    }
}

/*
--------------------------------------------------------------------------------
  Function name : Bit_Convertor
  Author        :
  Description   :
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>

  desc          :
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void Bit_Convertor(short *ppsBuffer, long *plLength, int bps) //ֱ���������λ��0
{
    int i = 0;
    int offset = 0 ;

    if (bps == 16)
    {
        short *pOut = ppsBuffer;
        offset = *plLength * 2;

        for (i = offset - 1; i >= 0 ; i--)
        {
            pOut[2 * i + 1] = pOut[i];
            pOut[2 * i]     = 0;
        }

        *plLength = offset;
    }
    else if (bps == 24)
    {
        char *pOut = (char *) ppsBuffer;
        offset = *plLength * 2;

        for (i = offset; i > 0 ; i--)
        {
            pOut[4 * i - 1] = pOut[3 * i - 1];
            pOut[4 * i - 2] = pOut[3 * i - 2];
            pOut[4 * i - 3] = pOut[3 * i - 3];
            pOut[4 * i - 4] = 0;
        }

        *plLength = *plLength * 2;
    }
    else if (bps == 32)
    {
        *plLength = *plLength * 2;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : Bit_Convertor_DEC
  Author        :
  Description   : Convert to 24bit
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>

  desc          : Convert to low 24bit for EQ, fade process...;
                  paired with Bit_Convertor_shift() after EQ process to 32bit
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void Bit_Convertor_DEC(short *ppsBuffer, long *plLength, int bps) //���ӽ������������λ��0������EQ������
{
    int i = 0;
    int offset = 0 ;

    if (bps == 16)
    {
        short *pOut = ppsBuffer;
        long *pOut32 = (long *) ppsBuffer;
        offset = *plLength * 2;

        for (i = offset - 1; i >= 0 ; i--)
        {
            pOut32[i] = (pOut[i] << 16) >> 8;
        }
    }
    else if (bps == 24)
    {
        char *pOut = (char *) ppsBuffer;
        long *pOut32 = (long *) ppsBuffer;
        offset = *plLength * 2;

        for (i = offset; i > 0 ; i--)
        {
            pOut32[i - 1]  = ( pOut[3 * i - 1] << 24 | pOut[3 * i - 2] << 16 | pOut[3 * i - 3] << 8) >> 8;
        }
    }
    else if (bps == 32)
    {
        long *pOut = (long *) ppsBuffer;
        offset = *plLength * 2;

        for (i = offset - 1; i >= 0 ; i--)
        {
            pOut[i] = pOut[i] >> 8;
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : Bit_Convertor_shift
  Author        :
  Description   : shift 24bit to 32bit
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>

  desc          : shift 24bit to 32bit
                  paired with Bit_Convertor_DEC() after EQ process to 32bit
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void Bit_Convertor_shift(short *ppsBuffer, long *plLength, int bps) //����EQ�ȴ���������ݴ���
{
    int i = 0;
    int offset = 0 ;

    long *pOut = (long *) ppsBuffer;
    offset = *plLength * 2;

    for (i = offset - 1; i >= 0 ; i--)
    {
        pOut[i] = pOut[i] << 8;
    }

    *plLength = *plLength * 2;
}

extern FILE *pRawFileCache, *pFlacFileHandleBake;
/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN AudioPlay(void)
  Author        : zs
  Description   : audio decode,decode next frame.
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                   zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AudioDecoding(void)
{
    short *pBuffer1, * pBuffer2, * pBuffer3;
    int  i, j, k = 0;
    int32 DecodeErr;
    uint32 readlen, totalRead;
    int32 DataTemp;

#if (2 == FW_IN_DEV)
    if (CheckCard() == 0)
    {
        //printf("\nNO Card audio decoding!!!");
        return;
    }
#endif

    AudioDecodeCnt++;

    if (AUDIO_STATE_PLAY == AudioPlayState)
    {
        if (AudioCodecOpenErr)
        {
            AudioPlayState = AUDIO_STATE_STOP;
            SendMsg(MSG_AUDIO_FILE_ERROR);
        }

        DecodeErr = CodecGetCaptureBuffer((short*)&AudioPtr, &AudioLen);

        if (0 == DecodeErr)   // Decoding end or error
        {
            if (FileInfo[(uint32)pRawFileCache].Offset <= FileInfo[(uint32)pRawFileCache].FileSize / 2
                && (gSysConfig.MusicConfig.RepeatMode != AUDIO_REPEAT)
                && (gSysConfig.MusicConfig.RepeatMode != AUDIO_REPEAT1))
            {
                printf("\n------1 file error------ ");
                AudioPlayState = AUDIO_STATE_STOP;
                memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                SendMsg(MSG_AUDIO_FILE_ERROR);
            }
            else
            {
                printf("\n------2 decode end------ ");
                AudioPlayState = AUDIO_STATE_STOP;
                memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                SendMsg(MSG_AUDIO_DECODE_END);
            }

            AudioDecodeing = 0;
            return;
        }

#ifdef _RK_CUE_
        if(AudioFileInfo.FindData.IsCue == 1)
        {
            if (pAudioRegKey->CurrentTime >= AudioFileInfo.FindData.CueEndTime)
            {
                printf("\n------pAudioRegKey->CurrentTime = %d  End time = %d------ ", pAudioRegKey->CurrentTime, AudioFileInfo.FindData.CueEndTime);
                printf("\n------3 decode end------ ");
                AudioPlayState = AUDIO_STATE_STOP;

                memset((uint8*)AudioPtr, 0x00, AudioLen * 4);
                SendMsg(MSG_AUDIO_DECODE_END);

                AudioDecodeing = 0;
                return;
            }
        }
#endif

        /*else*/ if (2 == DecodeErr)
        {
            //printf("\nDecodeStatus = 2 wait B decode");
            AudioNeedDecode = 1;
            return;
        }

#ifdef _A2DP_SOUCRE_
        AudioSbcEncodeInit();
#endif
        Bit_Convertor_DEC((short*)AudioPtr, &AudioLen,  pAudioRegKey->bps);

        // fade in init
        if (AudioErrorFrameNum < 4)
        {
            if (++AudioErrorFrameNum >= 4)
            {
                //MusicNextFile = 1;
                SendMsg(MSG_MUSIC_FADE_IN);
                #ifdef _FADE_PROCESS_
                AudioPlayInfo.VolumeCnt = AudioPlayInfo.playVolume;
                Codec_SetVolumet(AudioPlayInfo.playVolume);
                FadeInit(pAudioRegKey->samplerate, pAudioRegKey->samplerate / 32, FADE_IN);
                SendMsg(MSG_MUSIC_FADE_OK);
                AudioEndFade = 0;
                #endif
            }
            else
            {
                memset((uint8*)AudioPtr, 0x00, AudioLen * 8);
            }
        }

#ifdef _FADE_PROCESS_
        // last 500ms for Fadeout
#ifdef _RK_CUE_
        if(AudioFileInfo.FindData.IsCue == 1)
        {
            if ((AudioFileInfo.FindData.CueEndTime - pAudioRegKey->CurrentTime) <= 300)
            {
                if (FadeIsFinished())
                {
                    if (AudioEndFade == 0)
                    {
                        FadeInit(0, pAudioRegKey->samplerate / 4, FADE_OUT);
                        AudioEndFade = 1;
                    }
                }
            }
        }
        else
#endif
        {
            if ((pAudioRegKey->TotalTime - pAudioRegKey->CurrentTime) <= 300)
            {
                if (FadeIsFinished())
                {
                    if (AudioEndFade == 0)
                    {
                        FadeInit(pAudioRegKey->samplerate, pAudioRegKey->samplerate / 4, FADE_OUT);
                        AudioEndFade = 1;
                    }
                }
            }
        }
        #endif

        #ifdef _RK_EQ_
        if(EQ_NOR != AudioIOBuf.EffectCtl.Mode)
        {
            /*audio effect process.*/

            if (pAudioRegKey->samplerate <= 96000)
            {
                EffectProcess((EQ_TYPE *)AudioPtr, AudioLen);
            }
        }
        #endif

        #ifdef _FADE_PROCESS_
        if (!FadeIsFinished())
        {
            FadeProcess((long*)AudioPtr, AudioLen);
        }
        else if (AudioEndFade == 1)
        {

            memset((uint8*)AudioPtr, 0x00, AudioLen * 8);

        }
        #endif

        #ifdef _RK_SPECTRUM_
        if (FALSE == CheckMsg(MSG_BL_OFF))
        {
            short Spectrum_data[128];

            if (TRUE == GetMusicUIState())//backgroud need not to compute the spectrum.
            {
                if (SpectrumLoop == 0)
                {
                    if (SpectrumCnt == 0)
                        memset(SpectrumOut, 0, SPECTRUM_LINE_M * sizeof(char));


                    if ((AUDIO_STATE_PLAY == AudioPlayState) && (SpectrumCnt < 3))
                    {
                        {
                            int i = 0;
                            long *ptr = (long*)AudioPtr;

                            for (i = 0; i < 128; i++)
                            {
                                Spectrum_data[i] = (short)((*ptr) >> 8);
                                ptr++;
                            }
                        }
                        DoSpectrum((short*)Spectrum_data, &SpectrumOut[0]);

                        SpectrumCnt++;

                        if (SpectrumCnt >= 3)
                        {
                            SpectrumLoop = SpectrumLoopTime;
                            SendMsg(MUSIC_UPDATESPECTRUM);
                            SpectrumCnt = 0;
                        }
                    }
                }

                if (SpectrumLoop > 0)
                    SpectrumLoop-- ;
            }
        }
#endif
    }

    Bit_Convertor_shift((short*)AudioPtr, &AudioLen,  pAudioRegKey->bps);
    IntDisable(INT_ID_DMA);
    AudioDecodeing = 0;

    if (DmaTransting == 0)
    {
        //printf("\nDmaTransting == 0");
        AudioDmaIsrHandler();
    }

    DmaTransting = 1;
    IntEnable(INT_ID_DMA);
}



#ifdef _USB_
/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : ϵͳ������򣬸ó������ϵͳ��Ϣ��ϵͳ�̡߳�������Ⱥ�̨��Ҫ
                  ��ɵĹ���

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
extern THREAD USBControlThread;

_ATTR_SYS_BSS_ uint32 USBDebounceCount;
_ATTR_SYS_BSS_ uint8  USBVbusStatus;
_ATTR_SYS_BSS_ uint8  VbusDetStatus;

#ifdef _USB_HOST_

_ATTR_SYS_CODE_
uint32 USBHost_Reinit(void)
{
    USBHostReInit();
}


_ATTR_SYS_CODE_
uint32 USBHostStatusDetect(void)
{
    uint32 retval = RETURN_OK;
    TASK_ARG TaskArg;
    UHC_CHN_INFO *pUsbHC = NULL;

    pUsbHC = (UHC_CHN_INFO *)HostGetChannelInfo();

    if (pUsbHC->ConnectStatus != 1)
    {
        gSysConfig.UsbHost = 0;
        gSysConfig.Listen_UsbHost = 0;
        USBHost_Reinit();
        if (MemorySelect == USBHOST_FLASH)
        {
#ifdef _RADIO_
            if (TRUE != ThreadCheck(pMainThread, &FMThread))
#endif

#ifdef _MUSIC_
            {
                //printf ("USBHostDetect 0\n");
                ThreadDelete(&pMainThread, &MusicThread);
            }

            if (Task.TaskID == TASK_ID_MUSIC)
            {
                retval = RETURN_FAIL;
                TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
                TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                //DEBUG ("HostDetect:0 MemorySelect = FLASH0");
                MemorySelect = FLASH0;
                FileSysSetup(MemorySelect);//USB host�˳�Ĭ�ϰ�װFlash0
                ClearMsg(MSG_MEDIA_NOW_PLAY);
                SendMsg(MSG_USBHOT_UPDATE);
                return(RETURN_FAIL);
            }
#endif
            //DEBUG ("HostDetect:1 MemorySelect = FLASH0");
            MemorySelect = FLASH0;
            FileSysSetup(MemorySelect);//USB host�˳�Ĭ�ϰ�װFlash0
            SendMsg(MSG_USBHOT_UPDATE);
        }
    }
    else
    {
        //todo...
    }

    return (retval);
}
#endif

_ATTR_SYS_CODE_
uint32 USBStatusDetect(void)
{
    uint32 RetVal = 0;
    int32  UsbAdapterRet = 0;
    TASK_ARG TaskArg;
    uint8 VbusStatus = 0;
    uint8 VbusChanged = 0;

    VbusStatus = CheckVbus();

    if (USBVbusStatus != VbusStatus)
    {
        USBVbusStatus = VbusStatus;
        //USBDEBUG("VBUS Change: ", VbusStatus);
        VbusChanged = 1;
    }

    if (VbusChanged)
    {
        VbusDetStatus = 0;

        if (VbusStatus)
        {
            VbusDetStatus = 1;
        }

        USBDebounceCount = SysTickCounter;
    }
    else
    {
        if (VbusStatus)
        {
            if (1 == VbusDetStatus)
            {
                if ((SysTickCounter - USBDebounceCount) > 50) // 500ms ���� ���������
                {
                    if (FALSE == CheckMsg(MSG_VBUS_INSERT))
                    {
                        USBDEBUG("VBUS INSET");
                        //Vbus �Ӱγ�����������USB HOST�����򷴳�ʼ��USB HOST
                        #ifdef _USB_HOST_
                        USBHost_Reinit();
                        #endif
                        SendMsg(MSG_VBUS_INSERT);

                        AutoPowerOffDisable();
                        ThreadDeleteAll(&pMainThread);

                        UsbAdpterProbeStart();
                        VbusDetStatus = 2;
                    }
                }
            }
            else if (2 == VbusDetStatus)
            {
                UsbAdapterRet = UsbAdpterProbe();

                if (1 == UsbAdapterRet)
                {
                    VbusDetStatus = 0;

                    if (GetMsg(MSG_MES_FIRMWAREUPGRADE))
                    {
                        TaskArg.Usb.FunSel = USB_CLASS_TYPE_UPGRADE;
                    }
                    else
                    {
#if 0
                        TaskArg.Usb.FunSel = USB_CLASS_TYPE_SERIAL;
#else
                        TaskArg.Usb.FunSel = USB_CLASS_TYPE_MSC;
#endif
                    }

                    TaskSwitch(TASK_ID_USB, &TaskArg);
                    RetVal = RETURN_FAIL;
                }
                else if (2 == UsbAdapterRet)
                {
                    //AC
                    SendMsg(MSG_CHARGE_ENABLE);

                    BatteryChargeInit();
                    SendMsg(MSG_CHARGE_START);      //ʹ�ܳ��

                    SendMsg(MSG_SYS_RESUME);

                    if (Task.TaskID != TASK_ID_CHARGE)
                    {
                        TaskSwitch(TASK_ID_CHARGE, NULL);
                        RetVal = RETURN_FAIL;
                    }
                }
            }
        }
        else
        {
            //if ((SysTickCounter-USBDebounceCount) > 20) // 200ms ���� ���������
            {
                if (CheckMsg(MSG_VBUS_INSERT))
                {
                    USBDEBUG("VBUS REMOVE");
                    ClearMsg(MSG_VBUS_INSERT);
                    SendMsg(MSG_SYS_RESUME);

                    UsbAdpterProbeStop();

                    AutoPowerOffEnable();

                    BatteryChargeDeInit();

                    ClearMsg(MSG_CHARGE_ENABLE);
                    ClearMsg(MSG_CHARGE_START);
                }
            }
        }
    }

    return (RetVal);
}

#endif

#ifdef _SDCARD_
/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : ϵͳ������򣬸ó������ϵͳ��Ϣ��ϵͳ�̡߳�������Ⱥ�̨��Ҫ
                  ��ɵĹ���

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SDCardEnable(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : ϵͳ������򣬸ó������ϵͳ��Ϣ��ϵͳ�̡߳�������Ⱥ�̨��Ҫ
                  ��ɵĹ���

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SDCardDisable(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : ϵͳ������򣬸ó������ϵͳ��Ϣ��ϵͳ�̡߳�������Ⱥ�̨��Ҫ
                  ��ɵĹ���

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
uint32 SDCardStatusDetect(void)
{
    uint32 RetVal = 0;
    TASK_ARG TaskArg;

    if (CheckCard() == 0)
    {
        if (FALSE == CheckMsg(MSG_SDCARD_EJECT))
        {
            SendMsg(MSG_SDCARD_EJECT);
            SendMsg(MSG_SDCARD_UPDATE);
            SendMsg(MSG_SDCARD_MEM_UPDATE);
            SendMsg(MSG_SYS_RESUME);
            FREQ_EnterModule(FREQ_BLON);

#if (2 != FW_IN_DEV)
            if (gSysConfig.Memory == CARD)
#endif
            {
#ifdef _RADIO_
                if (TRUE != ThreadCheck(pMainThread, &FMThread))
#endif
                {
#ifdef _MUSIC_
                    DEBUG("Remove SDCard");
                    ThreadDelete(&pMainThread, &MusicThread);
#endif
#ifdef _RECORD_
                    ThreadDelete(&pMainThread, &RecordThread);
#endif
                }

#ifdef _MUSIC_
                if (Task.TaskID == TASK_ID_MUSIC)
                {
                    RetVal = RETURN_FAIL;
                    TaskArg.MainMenu.MenuID = MAINMENU_ID_MUSIC;
                    TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                }
#endif

#ifdef _VIDEO_//ylz++
                if (Task.TaskID == TASK_ID_VIDEO)
                {
                    RetVal = RETURN_FAIL;
                    TaskArg.MainMenu.MenuID = MAINMENU_ID_VIDEO;
                    TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                }
#endif

#ifdef _RECORD_
                if (Task.TaskID == TASK_ID_RADIO)
                {
                    RetVal = RETURN_FAIL;
                    TaskArg.MainMenu.MenuID = MAINMENU_ID_RADIO;
                    TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                }
#endif

#ifdef _PICTURE_//ylz++
                if (Task.TaskID == TASK_ID_PICTURE)
                {
                    RetVal = RETURN_FAIL;
                    TaskArg.MainMenu.MenuID = MAINMENU_ID_PICTURE;
                    TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
                }
#endif

#if (FW_IN_DEV != 2)
                {
                    ClearMsg(MSG_MEDIA_NOW_PLAY);
                    gSysConfig.Memory = FLASH0;
                    FileSysSetup(gSysConfig.Memory);//dgl second filesystem
                }
#endif
            }

            SDCardDisable();
            SDCardDeInit();//add by dgl
        }
    }
    else
    {
        if (CheckMsg(MSG_SDCARD_EJECT))
        {
            ClearMsg(MSG_SDCARD_EJECT);
            SendMsg(MSG_SDCARD_UPDATE);
            SendMsg(MSG_SDCARD_MEM_UPDATE);
            SendMsg(MSG_SYS_RESUME);
            FREQ_EnterModule(FREQ_BLON);

            SDCardEnable();
            DEBUG("Insert SDCard");
        }
    }

    return (RetVal);
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : AdcKey_SysTimer_Handler()
  Author        : anzhiguo
  Description   : ADC Key Timer Isr

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AutoPowerDownTimerISR(void)
{
    SendMsg(MSG_POWER_DOWN);
}

_ATTR_SYS_CODE_
void SetPowerDownTimerISR(void)
{
    SetPowerOffFlag = 0;
    SendMsg(MSG_POWER_DOWN);
}

_ATTR_SYS_DATA_
SYSTICK_LIST AutoPowerDownTimer =
{
    NULL,
    0,
    10 * 60 * 100,
    1,
    AutoPowerDownTimerISR,
};

_ATTR_SYS_DATA_
SYSTICK_LIST SetPowerDownTimer =
{
    NULL,
    0,
    0,
    1,
    SetPowerDownTimerISR,
};

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : �ػ����

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AutioPowerOffTimerRest(void)
{
    AutoPowerDownTimer.Counter = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : �ػ����

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AutoPowerOffEnable(void)
{
    if (AutoPowerOffDisableCounter > 0)
    {
        AutoPowerOffDisableCounter--;

        if (AutoPowerOffDisableCounter == 0)
        {
            SystickTimerStart(&AutoPowerDownTimer);
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : �ػ����

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void AutoPowerOffDisable(void)
{
    if (AutoPowerOffDisableCounter == 0)
    {
        SystickTimerStop(&AutoPowerDownTimer);
    }

    AutoPowerOffDisableCounter++;
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : �ػ����

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SetPowerOffTimerEnable(void)
{
    uint32 PowerOffBuff[6] = {0, 10 * 60 * 100, 15 * 60 * 100, 30 * 60 * 100, 60 * 60 * 100, 120 * 60 * 100};

    if (gSysConfig.ShutTime)
    {
        SetPowerDownTimer.Counter = 0;
        SetPowerDownTimer.Reload = PowerOffBuff[gSysConfig.ShutTime];

        if (SetPowerOffFlag == 0)
        {
            SetPowerOffFlag = 1;
            SystickTimerStart(&SetPowerDownTimer);
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : �ػ����

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SetPowerOffTimerDisable(void)
{
    if (SetPowerOffFlag == 1)
    {
        SetPowerOffFlag = 0;
        SystickTimerStop(&SetPowerDownTimer);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : �ػ����

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
uint32 PowerOffDetec(void)
{
    uint32 RetVal = RETURN_OK;
    TASK_ARG TaskArg;

    //--------------------------------------------------------------------------
    if (GetMsg(MSG_POWER_DOWN))
    {
#if 0 //zyz: ???? #ifdef _MUSIC_#ifdef _MUSIC_
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 1;
        }
        else
        {
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;//HoldOnPlayInfo.HoldMusicGetSign ;
        }
#endif

#ifdef _VIDEO_
        if (TRUE == ThreadCheck(pMainThread, &VideoThread))
        {
            gSysConfig.VideoConfig.HoldOnPlaySaveFlag = 1;
        }
        else
        {
            gSysConfig.VideoConfig.HoldOnPlaySaveFlag = VideoHoldOnPlayInfo.HoldVideoGetSign ;
        }
#endif

#ifdef _RADIO_
        if ((TRUE == ThreadCheck(pMainThread, &FMThread)) || (FmStandbyFlag == TRUE))
        {
            if (gpRadioplayerRegKey->FmState == FM_State_StepStation)
            {
                gSysConfig.RadioConfig.HoldOnPlaySaveFlag = 2;
            }
            else
            {
                gSysConfig.RadioConfig.HoldOnPlaySaveFlag = 1;
            }
        }
        else
        {
            gSysConfig.RadioConfig.HoldOnPlaySaveFlag = 0;
        }
#endif

#ifdef _RECORD_
        if (TRUE == ThreadCheck(pMainThread, &RecordThread))
        {
            ThreadDelete(&pMainThread, &RecordThread);   //delete current thread,
        }
#endif

        ThreadDeleteAll(&pMainThread);

        if (0)//((CheckVbus()) && (0 == Scu_DCout_Issue_State_Get()))
        {
            //TaskArg.Usb.FunSel = FUN_USB_CHARGE;
            TaskSwitch(TASK_ID_CHARGE, &TaskArg);
            RetVal = RETURN_FAIL;
        }
        else
        {
            SaveSysInformation(1);
            PowerOff();
        }
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOffDetec(void)
  Author        : ZHengYongzhi
  Description   : �ػ����

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SysResume(void)
{
    if (TRUE == GetMsg(MSG_SYS_RESUME))
    {
        //KeyReset();
        BL_Resume();
        AutioPowerOffTimerRest();
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void SysBatteryStateChk(void)

  Author        : yangwenjie
  Description   : shutdown check

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void SysBatteryStateCheck(void)
{
    WIN    * PowerOffWin;

    if (CheckAdcState(ADC_CHANEL_BATTERY))
    {
        Battery_GetLevel();

        //low voltage check,
        if (TRUE == GetMsg(MSG_LOW_POWER) && (CheckVbus( ) == 0) )
        {
            SendMsg(MSG_SYS_RESUME);

#ifdef _VIDEO_//ylz++
            if (TRUE == ThreadCheck(pMainThread, &VideoThread))
            {
                VideoPause();
            }
#endif

            PowerOffWin = WinGetFocus(pMainWin);
            WinCreat(PowerOffWin, &LowPowerWin, NULL);
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : FunUSBSysReboot
  Author        : ZhengYongzhi
  Description   : USB get system version number

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_OVERLAY_CODE_

void SysReboot(uint32 addr, uint8 flag)
{
    void(*pgUpgrade)(void);
    uint32 temp, i;

    DEBUG("Reboot, addr = 0x%x, flag = %d", addr, flag);

    ThreadDeleteAll(&pMainThread);

#ifdef _WATCH_DOG_
    WatchDogDisableInt();
    WatchDogDeInit();
#endif

    BL_Off();
    LcdStandby();
    AdcPowerDown();
    Codec_DeInitial();

    if (flag & 0x01)
    {
        SaveSysInformation(1);
    }

    MDDeInitAll();
    DelayMs(5);

    // N   a  n   o   C  R   e  b    o  o  t   F    l    a   g
    // 4e 61 6e 6f, 43 52 65 62, 6f 6f 74 46, 6c 61 67
    RebootTag[0] = 0x6f6e614e;
    RebootTag[1] = 0x62655243;
    RebootTag[2] = 0x46746f6f;
    RebootTag[3] = 0x0067616c;

    SetPllDefault();

    //disable all interrupt
    nvic->SysTick.Ctrl     &= ~NVIC_SYSTICKCTRL_TICKINT;

    for (i = 0; i < 32; i++)
    {
        nvic->Irq.Disable[i]    = 0xffffffff;
        nvic->Irq.ClearPend[i]  = 0xffffffff;
    }

    for (i = 0; i < 10; i++)
    {
        CRU->CRU_CLKGATE_CON[i] = 0xffff0000;
    }

    ScuClockGateCtr(PCLK_WDT_GATE, FALSE);
    DelayUs(50);

    for (i = 0; i < 4; i++)
    {
        CRU->CRU_SOFTRST[i] = 0xffff0000;
    }

    for (i = 0; i < 4; i++)
    {
        Grf->GPIO_IO0MUX[i]  = 0xffff0000;
        Grf->GPIO_IO1MUX[i]  = 0xffff0000;
        Grf->GPIO_IO2MUX[i]  = 0xffff0000;

        Grf->GPIO_IO0PULL[i] = 0xffff0000;
        Grf->GPIO_IO1PULL[i] = 0xffff0000;
        Grf->GPIO_IO2PULL[i] = 0xffff0000;
    }

    {
        Gpio_SetPortDirec(GPIO_CH0, 0x00000000);
        Gpio_SetPortDirec(GPIO_CH1, 0x00000000);
        Gpio_SetPortDirec(GPIO_CH2, 0x00000000);
    }

    Grf_NOC_Remap_Sel(NOC_BOOT_ROM);

    nvic->VectorTableOffset = (UINT32)0x00000000;

    if (addr == 0x0000008c)//firmware update.
    {
        *((UINT32 *)0X03050000) = chip_freq.armclk;
        pgUpgrade = (void *)(*((uint32 *)addr));

        Grf_otgphy_suspend(0);
        (*pgUpgrade)();
    }
    else
    {
        if (flag & 0x02) //enter loader usb
        {
            //Grf->SOFT_DBG0 = (0x18AF|0xFFFF0000);
            Pmu_Reg->PMU_SYS_REG3 = (0x18AF | 0xFFFF0000);
        }

        nvic->APIntRst = 0x05fa0007;       //system reseet.
    }

    while (1);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : ϵͳ������򣬸ó������ϵͳ��Ϣ��ϵͳ�̡߳�������Ⱥ�̨��Ҫ
                  ��ɵĹ���

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void SysServiceInit(void)
{
    DEBUG("system service initial...");

#ifdef _USB_
    USBVbusStatus = 0xff; //-1
#endif

    LcdStandbyEnable();
    BLOffEnable();

    AutoPowerOffDisableCounter = 1;
    AutoPowerOffEnable();

    SetPowerOffTimerEnable();
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SysService(void)
  Author        : ZHengYongzhi
  Description   : system service progarm,this program will call system backgroud message,system thread
                  and devices check etc to finish work.,

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
extern uint32 keybackup;
_ATTR_SYS_CODE_
UINT32 SysService(void)
{
    uint32 retval;
#ifdef _USB_HOST_
    UHC_CHN_INFO *pUsbHC = NULL;
    pUsbHC = (UHC_CHN_INFO *)HostGetChannelInfo();
#endif

    BBDebug();

#ifdef _WATCH_DOG_
    WatchDogReload();
#endif

    BacklightDetec();

#ifdef _SDCARD_
    if (gSysConfig.SDEnable)
    {
        if (RETURN_FAIL == SDCardStatusDetect())
        {
            return (RETURN_FAIL);
        }
    }
#endif

#ifdef _USB_
    #ifdef _USB_HOST_
    {
        if (gSysConfig.Listen_UsbHost == 1)
        {
            if (RETURN_FAIL == USBHostStatusDetect())
            {
                return(RETURN_FAIL);
            }
        }
    }
    if (pUsbHC->ConnectStatus != 1)
    #endif
    {
        if (RETURN_FAIL == USBStatusDetect())
        {
            return(RETURN_FAIL);
        }
    }

#else
    SendMsg(MSG_CHARGE_ENABLE);
#endif

#if(CODEC_CONFIG == CODEC_ROCKC)
#ifdef HP_DET_CONFIG
    Codec_HP_Detect();
#endif

#else
//...
#endif

    if (RETURN_FAIL == PowerOffDetec())
    {
        return (RETURN_FAIL);
    }

    SysBatteryStateCheck();

    SysResume();

    if (TRUE == CheckMsg(MSG_BL_OFF) && (TRUE != ThreadCheck(pMainThread, &RecordThread)))
    {
        __WFI();
    }

    return (RETURN_OK);
}

/*
********************************************************************************
*
*                         End of SysService.c
*
********************************************************************************
*/

