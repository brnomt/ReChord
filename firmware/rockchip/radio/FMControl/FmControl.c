
#define _IN_FM_CONTROL_
#include "SysInclude.h"
#include "fsinclude.h"

#ifdef _RADIO_
#include "FmControl.h"
#include "RadioWin.h"

_ATTR_FMCONTROL_DATA_ UINT32 FM_I2S_RW_AudioBuf[2][3072];
_ATTR_FMCONTROL_DATA_ uint32 FM_I2S_RW_BuferIndex = 0;

#if (I2S_CH == I2S_CH0)
_ATTR_FMCONTROL_DATA_ DMA_CFGX FM_I2SDmaCfg_TX  = {DMA_CTLL_I2S0_TX, DMA_CFGL_I2S0_TX, DMA_CFGH_I2S0_TX, 0};
_ATTR_FMCONTROL_DATA_ DMA_CFGX FM_I2SDmaCfg_RX  = {DMA_CTLL_I2S0_RX, DMA_CFGL_I2S0_RX, DMA_CFGH_I2S0_RX, 0};
#else   //i2c ch1  dma config ???
_ATTR_FMCONTROL_DATA_ DMA_CFGX FM_I2SDmaCfg_TX  = {DMA_CTLL_I2S1_TX, DMA_CFGL_I2S1_TX, DMA_CFGH_I2S1_TX, 0};
_ATTR_FMCONTROL_DATA_ DMA_CFGX FM_I2SDmaCfg_RX  = {DMA_CTLL_I2S1_RX, DMA_CFGL_I2S1_RX, DMA_CFGH_I2S1_RX, 0};

#endif
/*******************************************************************************
** Name: I2sIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
_ATTR_FMCONTROL_TEXT_
void FM_Record_len_Config(UINT16 RecordQuality, UINT16 RecordFmt)
{
    if (RECORD_QUALITY_HIGH == RecordQuality)  //quality record.
    {
        FM_RecordLen = 3072;
    }
    else
    {
        {
            FM_RecordLen = 1012;
        }
    }
}
/*******************************************************************************
** Name: I2sIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
_ATTR_FMCONTROL_TEXT_
void FM_Codec_Config(UINT16 RecordQuality, UINT16 RecordFmt)
{
    if (RECORD_QUALITY_HIGH == RecordQuality)  //quality record.
    {
        FM_FS = ENC_WAV_H_FS;
        ACodec_I2S_Data_Width = ACodec_I2S_DATA_WIDTH24;
        M_I2S_Data_Width      = I2S_DATA_WIDTH24;
    }
    else    //16bit
    {
        FM_FS = ENC_WAV_N_FS;

        ACodec_I2S_Data_Width = ACodec_I2S_DATA_WIDTH16;
        M_I2S_Data_Width      = I2S_DATA_WIDTH16;
    }
}

_ATTR_FMCONTROL_TEXT_
void FM_RW_DmaIsr_W(void)
{
    uint32 TX_FIFO_ADDR;

    if (!fm_is_stop)
    {
        TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH);
        DmaReStart(DMA_CHN4, (uint32)FM_I2S_RW_AudioBuf[1 - FM_I2S_RW_BuferIndex], TX_FIFO_ADDR, FM_RecordLen, &FM_I2SDmaCfg_TX, FM_RW_DmaIsr_W);
    }
}

_ATTR_FMCONTROL_TEXT_
void FM_RW_DmaIsr_R(void)
{
    uint32 RX_FIFO_ADDR;

    if (!fm_is_stop)
    {
        RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH);
        FM_I2S_RW_BuferIndex = 1 - FM_I2S_RW_BuferIndex;
        DmaReStart(DMA_CHN5, RX_FIFO_ADDR, (uint32)FM_I2S_RW_AudioBuf[FM_I2S_RW_BuferIndex], FM_RecordLen, &FM_I2SDmaCfg_RX, FM_RW_DmaIsr_R);

#ifdef _RECORD_
        if (REC_Flag == RECORD_BEING)
        {
            RecordGetDataFromFM((uint32*)(&FM_I2S_RW_AudioBuf[1 - FM_I2S_RW_BuferIndex]), FM_RecordLen);
        }
#endif
    }
}
/*******************************************************************************
** Name: Acodec_Dac2HP_test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
_ATTR_FMCONTROL_TEXT_
void FM_Start(uint32 flag)
{
    uint32 i = 0;
    uint32 TX_FIFO_ADDR, RX_FIFO_ADDR;
    pFunc pCallBack_TX, pCallBack_RX;
    CodecFS_en_t I2S_TEST_FS;

    I2SInit(I2S_CH, I2S_PORT, I2S_MODE, FM_FS, I2S_FORMAT, M_I2S_Data_Width, I2S_NORMAL_MODE);

    if (FM_OUT_CONFIG == FM_MIX_HP)
    {
        if (flag)
        {
            Codec_SetMode(Codec_Line1in, ACodec_I2S_Data_Width);
        }
    }
    else
    {
        if (flag)
        {
            //FM playback
            Codec_SetMode(Codec_Line1ADC, ACodec_I2S_Data_Width);

            //4 if U want to use line in,Pls open below code,close above .
            //Codec_SetMode(Codec_Line2in, ACodec_I2S_Data_Width);
        }

        fm_is_stop = 0;
        //I2S init
        FM_Record_len_Config(FM_RecordQuality, FM_RecordFmt);

        DmaEnableInt(DMA_CHN5);
        RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_CH);
        DmaStart(DMA_CHN5, RX_FIFO_ADDR, (uint32)FM_I2S_RW_AudioBuf[FM_I2S_RW_BuferIndex], FM_RecordLen, &FM_I2SDmaCfg_RX, FM_RW_DmaIsr_R);
        DelayMs(10);

        DmaEnableInt(DMA_CHN4);
        TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_CH);
        DmaStart(DMA_CHN4, (uint32)FM_I2S_RW_AudioBuf[1 - FM_I2S_RW_BuferIndex], TX_FIFO_ADDR, FM_RecordLen, &FM_I2SDmaCfg_TX, FM_RW_DmaIsr_W);

        I2SDMAEnable(I2S_CH, I2S_START_DMA_RTX);
        I2SStart(I2S_CH, I2S_START_DMA_RTX);
    }

    FM_Process(MSG_RADIO_VOLUMESET,NULL);
    Codec_SetSampleRate(FM_FS);
    Codec_DACUnMute();
}

_ATTR_FMCONTROL_TEXT_
BOOLEAN FM_Stop(uint32 flag)
{
    uint32 timeout = 2000;

    if (FM_OUT_CONFIG == FM_MIX_HP)
    {
        if (flag)
        {
            Codec_ExitMode(Codec_Line1in);
        }
    }
    else
    {
        if (fm_is_stop == 0)
        {
            fm_is_stop = 1;
            while (DmaGetState(DMA_CHN5) == DMA_BUSY)
            {
                DelayMs(1);
                if (--timeout == 0)
                {
                    break;
                }
            }

            timeout = 2000;
            while (DmaGetState(DMA_CHN4) == DMA_BUSY)
            {
                DelayMs(1);
                if (--timeout == 0)
                {
                    break;
                }
            }
        }
        if (flag)
        {
            Codec_ExitMode(Codec_Line1ADC);
        }
    }

    I2SStop(I2S_CH, I2S_START_DMA_RTX);
    I2SDeInit(I2S_CH, I2S_PORT);
    Codec_DACMute();

    return TRUE;
}

/**************************************************************************************
*   function name:GetNextStation
*   function:get next fm station
*   input:
*   output:
*   return:
*   description:
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void GetNextStation(void)
{
    UINT16 PreFmSaveNum = gpRadioplayerRegKey->FmSaveNum;

    for (gpRadioplayerRegKey->FmSaveNum++; gpRadioplayerRegKey->FmSaveNum != PreFmSaveNum; gpRadioplayerRegKey->FmSaveNum++)
    {
        if (gpRadioplayerRegKey->FmSaveNum >= FREQMAXNUMBLE)
        {
            gpRadioplayerRegKey->FmSaveNum = 0;
        }

        gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum];

        if (!((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal)))
        {
            break;
        }
    }

    if (gpRadioplayerRegKey->FmSaveNum >= FREQMAXNUMBLE)
    {
        gpRadioplayerRegKey->FmSaveNum = 0;
    }

    gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum];

    if ((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal))
    {
        gpRadioplayerRegKey->FmSaveNum = 0;
    }

    gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum];

    if ((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal))
    {
        gpRadioplayerRegKey->FmFreq = FmFreqMinVal;
    }

#if 0
    gpRadioplayerRegKey->FmSaveNum++;

    if (gpRadioplayerRegKey->FmSaveNum >= FREQMAXNUMBLE)
    {
        gpRadioplayerRegKey->FmSaveNum = 0;
    }

    gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum];

    if ((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal))
    {
        gpRadioplayerRegKey->FmSaveNum = 0;
    }

    gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum];

    if ((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal))
    {
        gpRadioplayerRegKey->FmFreq = FmFreqMinVal;
    }

#endif
}
/**************************************************************************************
*   function name��GetPrevStation
*   function     :get prevous fm station
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void GetPrevStation(void)
{
    INT16 j;
    UINT16 PreFmSaveNum = gpRadioplayerRegKey->FmSaveNum;
    j = gpRadioplayerRegKey->FmSaveNum;

    for (j--; j != PreFmSaveNum; j--)
    {
        if (j < 0)
        {
            j = (FREQMAXNUMBLE - 1);
        }

        gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[j];

        if (!((gpRadioplayerRegKey->FmFreqArray[j] < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreqArray[j] > FmFreqMaxVal)))
        {
            gpRadioplayerRegKey->FmSaveNum = j;
            break;
        }
    }

    gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum];

    if ((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal))
    {
        gpRadioplayerRegKey->FmFreq = FmFreqMinVal;
    }

#if 0

    for (j = 0; j < 30; j++)
    {
        if (gpRadioplayerRegKey->FmFreqArray[j] < FmFreqMinVal || gpRadioplayerRegKey->FmFreqArray[j] > FmFreqMaxVal)
            break;
    }

    if (gpRadioplayerRegKey->FmSaveNum >= j)
    {
        gpRadioplayerRegKey->FmSaveNum = 0;
    }

    if (gpRadioplayerRegKey->FmSaveNum == 0)
    {
        if (j > 1)
            gpRadioplayerRegKey->FmSaveNum = j - 1;
        else
            gpRadioplayerRegKey->FmSaveNum = 0;
    }
    else
        gpRadioplayerRegKey->FmSaveNum--;

    gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum];

    if ((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal))
    {
        gpRadioplayerRegKey->FmFreq = FmFreqMinVal;
    }

#endif
}

/**************************************************************************************
*   function name��FmStepChangeFreq
*   function     :
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void FmStepChangeFreq(UINT16 updownflag, UINT16 step)
{
    if (updownflag)
        gpRadioplayerRegKey->FmFreq += step;
    else
        gpRadioplayerRegKey->FmFreq -= step;

    if (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal)
    {
        gpRadioplayerRegKey->FmFreq = FmFreqMinVal;
    }

    if (gpRadioplayerRegKey->FmFreq < FmFreqMinVal)
    {
        gpRadioplayerRegKey->FmFreq = FmFreqMaxVal;
    }
}


/**************************************************************************************
*   function name��RadioPlayerGetFreqArray
*   function     :
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void RadioPlayerGetFreqArray(void)
{
    UINT8 i = 0;
#if 0

    if (gpRadioplayerRegKey->FmArea == 1)//SID_RadioListAreaJapan)
    {
        for (i = 0; i < 30; i++)
        {
            gpRadioplayerRegKey->FmFreqArray[i] = gpRadioplayerRegKey->FmFreqJpArray[i];
        }
    }
    else if (gpRadioplayerRegKey->FmArea == 2)//SID_RadioListAreaCamp)
    {
        for (i = 0; i < 30; i++)
        {
            gpRadioplayerRegKey->FmFreqArray[i] = gpRadioplayerRegKey->FmFreqCaArray[i];
        }
    }
    else
    {
        for (i = 0; i < 30; i++)
        {
            gpRadioplayerRegKey->FmFreqArray[i] = gpRadioplayerRegKey->FmFreqChArray[i];
        }
    }

#endif
}

/**************************************************************************************
*   function name��RadioPlayerFreqInit
*   function     : initial the max and min frequency of 5767 or 5807 FM module.
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void RadioPlayerFreqInit(void)
{
    gpRadioplayerRegKey->FmArea = gSysConfig.RadioConfig.FmArea;
    gpRadioplayerRegKey->FmStereo = gSysConfig.RadioConfig.FmStereo;

    if (gpRadioplayerRegKey->FmArea == RADIO_JAPAN)//SID_RadioListAreaJapan)
    {
        FmFreqMinVal = 7600;
        FmFreqMaxVal = 9000;
        gbFmTuneRegion = 1;
    }
    else    //(gpRadioplayerRegKey->FmArea == 2)//SID_RadioListAreaCamp)
    {
        FmFreqMinVal = 8750;
        FmFreqMaxVal = 10800;
        gbFmTuneRegion = 0;
    }
}

/*--------------------------------------------------------------------------------
  Function name :  FMVol_Resume
    Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_FMCONTROL_TEXT_
void FMVol_Resume(void)
{
    FmFirstEntryCount = SysTickCounter;
    SendMsg(MSG_RADIO_VOLUMRESET);
}

/**************************************************************************************
*   function name��FmModuleInitial
*   function     :
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void FmModuleInitial(void)
{
    RadioPlayerFreqInit();
    FmDriverTable.Tuner_SetInitArea(gSysConfig.RadioConfig.FmArea);
}

/**************************************************************************************
*   function name��RadioStartSeekByHand
*   function     :
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void RadioStartSeekByHand(FM_DIRECT dir)
{
    if (gpRadioplayerRegKey->FmSearchMode == FM_SearchModeState_ByHand)
    {
        if (gpRadioplayerRegKey->FmState != FM_State_HandSearch)
        {
            gpRadioplayerRegKey->FmState = FM_State_HandSearch;
        }

        gpRadioplayerRegKey->FmSearchDirect = dir;
        gpRadioplayerRegKey->FmHandSearchSt = 1;
        FM_Process(MSG_FM_HANDSEARCH, (void *)1);
    }
}
/**************************************************************************************
*   function name��RadioQuickSeekStation
*   function     : quikly step to next freqency point,but this frequncy value do not write to FM chip.
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void RadioQuickSeekStation(FM_DIRECT dir)
{
    if (gpRadioplayerRegKey->FmState == FM_State_HandStepFreq)
    {
        FmStepChangeFreq(dir, TUNERFREQSTEP);
        FmDriverTable.Tuner_MuteControl(1);
        PaintCHAndFreq();
    }
}

/**************************************************************************************
*   function name��RadioSetStepStation
*   function     : skip to next station.
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void RadioSetStepStation(FM_DIRECT dir)
{
    if ((gpRadioplayerRegKey->FmState == FM_State_StepStation) && (gpRadioplayerRegKey->FmSaveNum < FREQMAXNUMBLE)) // Ԥ��ģʽ
    {
        gpRadioplayerRegKey->FmSearchDirect = dir;
        FM_Process(MSG_FM_STEPSTATION, NULL);
    }
}
/**************************************************************************************
*   function     : receive the FM staticon by step..
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void RadioSetStepFreq(FM_DIRECT dir)
{
    gpRadioplayerRegKey->FmHandSearchSt = 0;

    if (gpRadioplayerRegKey->FmState == FM_State_HandStepFreq || (gpRadioplayerRegKey->FmState == FM_State_HandSearch && !gpRadioplayerRegKey->FmHandSearchSt))
    {
        gpRadioplayerRegKey->FmSearchDirect = dir; // 1 -- ��������
        FM_Process(MSG_FM_STEPFREQ, NULL);
    }
}
/**************************************************************************************
*   function     : get FM status.
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
FM_STATE RadioPlayerSeekingState(void)
{
    if (gpRadioplayerRegKey->FmState == FM_State_AutoSearch && gpRadioplayerRegKey->FmSearchMode == FM_SearchModeState_Auto)
        return FM_State_AutoSearch;//auto search.
    else if (gpRadioplayerRegKey->FmState == FM_State_StepStation && gpRadioplayerRegKey->FmSearchMode == FM_SearchModeState_Auto)
        return FM_State_StepStation;//default mode
    else if (gpRadioplayerRegKey->FmState == FM_State_HandSearch && gpRadioplayerRegKey->FmHandSearchSt == 1 && gpRadioplayerRegKey->FmSearchMode == FM_SearchModeState_ByHand)
        return FM_State_HandSearch;//hand search
    else
        return FM_State_HandStepFreq;//step.
}

/**************************************************************************************
*   function     : stop the processing of search station.
                   if now is in auto-search mode,stop in default mode
                   if now is in hand_search mode,stop in signal step mode.
*   input        ��
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
BOOL RadioStopSeeking(void)
{
    switch (RadioPlayerSeekingState())
    {
        case FM_State_AutoSearch:
        {
            gpRadioplayerRegKey->FmState = FM_State_StepStation;
            return TRUE;
        }
        break;

        case FM_State_HandSearch:
        {
            gpRadioplayerRegKey->FmState = FM_State_HandStepFreq;
            return TRUE;
        }
        break;

        case FM_State_HandStepFreq:
            return FALSE;
            break;

        default:
            return FALSE;
            break;
    }
}

/**************************************************************************************
*   function     : FMSearchByHand
*   input        ��direct -- 1(inc), 0(dec)
*   output       ��
*   return       :
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
FM_SEARCH_RESULT FMSearchByHand(UINT16 direct, void *msg)
{
    FmStepChangeFreq(direct, TUNERFREQSTEP);

    if ((gpRadioplayerRegKey->FmFreq == (FmFreqMinVal - TUNERFREQSTEP)))
    {
        PaintCHAndFreq();
        return FM_InvalidStation;
    }

    if (gpRadioplayerRegKey->FmFreq < FmFreqMinVal || gpRadioplayerRegKey->FmFreq > FmFreqMaxVal)
        return FM_SearchFail;//fail

    PaintCHAndFreq();
    DelayMs(20);
    return (FmDriverTable.Tuner_SearchByHand(direct, &(gpRadioplayerRegKey->FmFreq)));
}

/**************************************************************************************
*   function     : FMSearchAuto
*   input        ��
*   output       ��
*   return       : TRUE -- search finish. FALSE -- do not obtain any station.
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
BOOLEAN FMSearchAuto(void *msg)
{
    UINT32 freqpre, freqcur;
    FM_SEARCH_RESULT result = FMSearchByHand(1, 0);

    if (gpRadioplayerRegKey->FmFreq == (FmFreqMaxVal))
    {
        if (result == FM_FoundStation)
        {
            if (gpRadioplayerRegKey->FmSaveNum > (FREQMAXNUMBLE - 1))
            {
                return TRUE;
            }
            else
            {
                freqpre = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum - 1] / 10;
                freqcur = (gpRadioplayerRegKey->FmFreq) / 10;

                if (freqpre != freqcur)
                {
                    gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum++] = gpRadioplayerRegKey->FmFreq;
                    PaintCHAndFreq();
                }
            }
        }

        return TRUE;
    }

#if 0    //close by chad.ma

    if (CheckVbus())       //HJ
    {
        return TRUE;
    }

#endif

    if (TRUE == RadioKeyStopAutoSearch())
    {
        return TRUE;
    }

    switch (result)
    {
        case FM_FoundStation:
        {
            if (gpRadioplayerRegKey->FmSaveNum > (FREQMAXNUMBLE - 1))
                break;
            else
            {
                //every inc at 0.05 frequency unit,
                freqpre = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum - 1] / 10;
                freqcur = gpRadioplayerRegKey->FmFreq / 10;

                if (freqpre != freqcur)
                {
                    gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum++] = gpRadioplayerRegKey->FmFreq;
                    // RockOSSendMsg(MBGUI, AS_FMUI_UPDATEInd, (void *)WM_SetTxt);
                    PaintCHAndFreq();
                }
            }
        }

        case FM_InvalidStation:
        loop1:
            if (gpRadioplayerRegKey->FmState == FM_State_AutoSearch)//FM_State_Searching)
            {
                FM_Process(MSG_FM_AUTOSEARCH, NULL);
                return FALSE;
            }

        case FM_SearchFail:
            goto loop1;

        default:
            break;
    }

    return TRUE;
}

/**************************************************************************************
*   function     : FMHandSearchProcess
*   input        ��
*   output       ��
*   return       : TRUE -- search finish. FALSE -- do not obtain any station.
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
BOOL FMHandSearchProcess(void *msg)
{
#ifdef _WATCH_DOG_
    WatchDogReload();
#endif

    if ((UINT32)msg)
    {
        FmDriverTable.Tuner_MuteControl(1);
    }

    if (FM_FoundStation != FMSearchByHand(gpRadioplayerRegKey->FmSearchDirect, msg))
    {
        if (TRUE == RadioKeyStopAutoSearch())
        {
            gpRadioplayerRegKey->FmHandSearchSt = 0;
            gpRadioplayerRegKey->FmState = FM_State_HandStepFreq;
            return FALSE;
        }

        if (gpRadioplayerRegKey->FmFreq == gbFreqBack)
        {
            gpRadioplayerRegKey->FmHandSearchSt = 0;
            gpRadioplayerRegKey->FmState = FM_State_HandStepFreq;
            return FALSE;
        }

        if (gpRadioplayerRegKey->FmState == FM_State_HandSearch)//FM_State_Searching)
        {
            FM_Process(MSG_FM_HANDSEARCH, NULL);
            return TRUE;
        }
        else
        {
            if ((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal))
            {
                gpRadioplayerRegKey->FmFreq = FmFreqMinVal;
            }

            gpRadioplayerRegKey->FmHandSearchSt = 0;
            gpRadioplayerRegKey->FmState = FM_State_HandStepFreq;
            return FALSE;
        }
    }
    else
    {
        gpRadioplayerRegKey->FmHandSearchSt = 0;
        gpRadioplayerRegKey->FmState = FM_State_HandStepFreq;
        FmDriverTable.Tuner_MuteControl(0);
        return FALSE;
    }
}
/**************************************************************************************
*   function     : FMAutoSearchProcess
*   input        ��
*   output       ��
*   return       : TRUE -- search finish. FALSE -- continue.
*   description  ��
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
BOOL FMAutoSearchProcess(void *msg)
{
#ifdef _WATCH_DOG_
    WatchDogReload();
#endif

    if ((UINT32)msg)
    {
        FmDriverTable.Tuner_MuteControl(1);
    }

    if (FMSearchAuto(msg))//search complete
    {
        //FmFunctionSele = FM_OPT_MODE_PRESET;
        FmFunctionSeleBack = FM_OPT_MODE_PRESET;
        SendMsg(MSG_RADIOWIN_DISPLAY_SETTING); //ywj 09-7-26
        gpRadioplayerRegKey->FmState = FM_State_StepStation;
        FmFreqNum = gpRadioplayerRegKey->FmSaveNum;  //save the searched stations.
        gpRadioplayerRegKey->FmSaveNum = 0;
        gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum];

        if ((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal))
        {
            gpRadioplayerRegKey->FmFreq = FmFreqMinVal;
            gpRadioplayerRegKey->FmFreqArray[gpRadioplayerRegKey->FmSaveNum] = FmFreqMinVal;
        }

        return FALSE;
    }
    else
    {
        gpRadioplayerRegKey->FmAutoSearchState = FALSE;
        return TRUE;
    }
}
/**************************************************************************************
*   function     : FMStepFreqProcess
*   input        ��
*   output       ��
*   return       :
*   description  ��inc or dec one frequency unit.
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
void FMStepFreqProcess(void)
{
    FmStepChangeFreq(gpRadioplayerRegKey->FmSearchDirect, TUNERFREQSTEP);
    gpRadioplayerRegKey->FmFreq = (gpRadioplayerRegKey->FmFreq / 10) * 10;
    gpRadioplayerRegKey->FmState = FM_State_HandStepFreq;
}
/**************************************************************************************
*   function     : FMStepFreqProcess
*   input        ��
*   output       ��
*   return       : true:success false:failure
*   description  ��function switch.
**************************************************************************************/
_ATTR_FMCONTROL_TEXT_
BOOLEAN FM_Process(MSG_ID id, void *msg)
{
    int resetvolumet = 0;

    switch (id)
    {
        case MSG_FM_INIT://fm initialization
        {
            //FMVol_Resume();
            FmModuleInitial();
            DelayMs(100);
            FmDriverTable.Tuner_MuteControl(0);
        }

        return TRUE;

        case MSG_FM_START://start fm play
        {
            if (FmStandbyFlag == TRUE)
            {
                FmStandbyFlag = FALSE;
                SendMsg(MSG_RADIOSUBWIN_DISPLAY_OFF); //HJ
            }

            FmDriverTable.Tuner_SetFrequency(gpRadioplayerRegKey->FmFreq, 1, gpRadioplayerRegKey->FmStereo, gpRadioplayerRegKey->FmArea); //TRUE

            if (((UINT32)msg))
            {
                FmDriverTable.Tuner_MuteControl(0);
            }

            if (gpRadioplayerRegKey->FmStereo == RADIO_STEREO_OPEN)
                FM_Process(MSG_FM_GETSTEROSTATUS, NULL);
        }

        return TRUE;

        case  MSG_RADIO_VOLUMESET:
        {
            if (FmStandbyFlag == FALSE)
            {
                Codec_SetVolumet(RadioOutputVol);
#if 0

                if (RadioOutputVol == 0)
                {
                    FmDriverTable.Tuner_MuteControl(1);
                }
                else
                {
                    FmDriverTable.Tuner_MuteControl(0);
                }

#endif
            }

            if (FmInitFlag == FALSE)
            {
                FmInitFlag = TRUE;
            }

            return TRUE;
        }

        case MSG_FM_HANDSEARCH:
        {
            if (FMHandSearchProcess(msg))
                return TRUE;
            else
            {
                resetvolumet = 1;
            }

            break;
        }

        case MSG_FM_AUTOSEARCH:
        {
            if (FMAutoSearchProcess(msg))
                return TRUE;
            else
                resetvolumet = 1;

            break;
        }

        case MSG_FM_STEROSWITCH://stereo or not set.
        {
            gpRadioplayerRegKey->FmStereo = gSysConfig.RadioConfig.FmStereo;

            if (gpRadioplayerRegKey->FmStereo == RADIO_STEREO_OPEN)
                FmDriverTable.Tuner_SetStereo(FALSE);
            else
                FmDriverTable.Tuner_SetStereo(TRUE);

            resetvolumet = 1;
        }
        break;

        case MSG_FM_STEPFREQ://inc or dec one frequency unit.
        {
            FMStepFreqProcess();
            resetvolumet = 1;
        }
        break;

        case MSG_FM_STEPSTATION:
        {
            resetvolumet = 1;

            if (gpRadioplayerRegKey->FmSearchDirect)
                GetNextStation();
            else
                GetPrevStation();

            //------TODO-------send message to ui,display freqency point.
            SendMsg(MSG_RADIOWIN_DISPLAY_CH);
        }
        break;

        case MSG_FM_GETSTEROSTATUS://get status of stereo
        {
            DelayMs(200);
            FmGetStereoStatus = FmDriverTable.GetStereoStatus();
            SendMsg(MSG_RADIOWIN_DISPLAY_STEREO);
        }

        return TRUE;

        case MSG_FM_DEINIT://FM auti-initialization
        {
            Codec_SetVolumet(0);
            FmDriverTable.Tuner_MuteControl(1);
            FmDriverTable.Tuner_PowerOffDeinit();
            DEBUG("=== MSG_FM_DEINIT ===");
        }

        return TRUE;

        default:
            return FALSE;
    }

    FM_Process(MSG_FM_START, (void *)resetvolumet);
    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name :  void FMInit(void *pArg)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
extern FIRMWARE_INFO_T const ModuleInfo;
_ATTR_FMCONTROL_TEXT_
void FM_DriveInit(UINT32 *DriveID)
{
    uint32 SystemDefaultParaAddr;
    uint8  FlashBuf[512];
    FIRMWARE_INFO_T      *pFirmwareModuleInfo;
    FM_DRIVER_INFO_T     *pSystemFmPara;
    ////////////////////////////////////////////////////////////////////////////
    MDReadData(SysDiskID, CodeLogicAddress, 512, FlashBuf);
    pFirmwareModuleInfo = (FIRMWARE_INFO_T *)FlashBuf;
    SystemDefaultParaAddr  = CodeLogicAddress + sizeof(pFirmwareModuleInfo -> LoadStartBase);
    SystemDefaultParaAddr += sizeof(pFirmwareModuleInfo -> ModuleInfo.ModuleNum);
    SystemDefaultParaAddr += pFirmwareModuleInfo -> ModuleInfo.ModuleNum * sizeof(CODE_INFO_T);
    SystemDefaultParaAddr += sizeof(SYSTEM_DEFAULT_PARA_T);
    MDReadData(SysDiskID, SystemDefaultParaAddr, sizeof(FM_DRIVER_INFO_T), FlashBuf);
    pSystemFmPara = (FM_DRIVER_INFO_T*)FlashBuf;
    ///////////////////////////////////////////////////////////////////////
    *DriveID = pSystemFmPara->FMDriverIndex;
    memcpy((UINT8*)(&FmDriverTable), (UINT8*)(&(pSystemFmPara->FmDriver[*DriveID])), sizeof(FM_DRIVER_TABLE_T));
}

/*
--------------------------------------------------------------------------------
  Function name :  void FMInit(void *pArg)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_FMCONTROL_TEXT_
void FMInit(void *pArg)
{
    uint32 i = 0;
    UINT32 FmDriverID ;
    fm_is_stop = 0;

    FREQ_EnterModule(FREQ_FM);

    AutoPowerOffDisable();
    FM_DriveInit(&FmDriverID);
    ModuleOverlay(MODULE_ID_FM_DRIVER1 + FmDriverID, MODULE_OVERLAY_ALL);
#ifdef _RADIO_
    gFMConfig.FmArea   =  gSysConfig.RadioConfig.FmArea;
    gFMConfig.FmStereo =  gSysConfig.RadioConfig.FmStereo;
    gFMConfig.FmState  =  gSysConfig.RadioConfig.FmState ;
    gFMConfig.FmSaveNum = gSysConfig.RadioConfig.FmSaveNum;
#endif
#ifdef _RECORD_
    FM_RecordQuality    = gSysConfig.RecordConfig.RecordQuality;
    FM_RecordFmt        = gSysConfig.RecordConfig.RecordFmt;
#endif
    FM_Process(MSG_FM_INIT, NULL);

    if (gSysConfig.RadioConfig.FmFreq < FmFreqMinVal || gSysConfig.RadioConfig.FmFreq > FmFreqMaxVal)
    {
        if (gFMConfig.FmArea ==  RADIO_JAPAN)
        {
            gFMConfig.FmFreq = 7600;
        }
        else
        {
            gFMConfig.FmFreq = 8750;
        }
    }
    else
    {
        gFMConfig.FmFreq  = gSysConfig.RadioConfig.FmFreq;
    }

    for (i = 0; i < FREQMAXNUMBLE; i++)
    {
        if (gSysConfig.RadioConfig.FmFreqArray[i] < FmFreqMinVal ||  gSysConfig.RadioConfig.FmFreqArray[i] > FmFreqMaxVal)
        {
            gFMConfig.FmFreqArray[i] = 0;
        }
        else
        {
            gFMConfig.FmFreqArray[i] = gSysConfig.RadioConfig.FmFreqArray[i];
        }
    }

    gpRadioplayerRegKey = &gFMConfig;

    if ((gFMConfig.FmState == FM_State_AutoSearch ) || (gFMConfig.FmState == FM_State_StepStation))
    {
        SendMsg(MSG_RADIOWIN_DISPLAY_CH);
    }

    FM_Codec_Config(FM_RecordQuality, FM_RecordFmt);
    FM_Start(1);

    FmDriverTable.Tuner_SetVolume(15);
    FM_Process(MSG_RADIO_VOLUMESET, NULL);
    FM_Process(MSG_FM_STEROSWITCH, NULL);
}

/*
--------------------------------------------------------------------------------
  Function name :   UINT32 FMService(void)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_FMCONTROL_TEXT_
UINT32 FMService(void)
{
    int ret = 0;
    UINT16 cout;

    if (CheckMsg(MSG_RADIO_VOLUMRESET))
    {
        cout = SysTickCounter - FmFirstEntryCount;

        if (cout >= 200)
        {
            ClearMsg(MSG_RADIO_VOLUMRESET);
            FM_Process(MSG_RADIO_VOLUMESET, NULL);
        }
    }

    if (GetMsg(MSG_RADIO_STERO_UPDATES))
    {
        FM_Process(MSG_FM_STEROSWITCH, NULL);
    }

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name :  void FMDeInit(void)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_FMCONTROL_TEXT_
void FMDeInit(void)
{
    UINT8 i;
    uint32 timeout = 200;
    gSysConfig.RadioConfig.FmArea    = gpRadioplayerRegKey->FmArea;
    gSysConfig.RadioConfig.FmStereo  = gpRadioplayerRegKey->FmStereo;
    gSysConfig.RadioConfig.FmState   = gpRadioplayerRegKey->FmState;
    gSysConfig.RadioConfig.FmSaveNum = gpRadioplayerRegKey->FmSaveNum;
    gSysConfig.RadioConfig.FmFreq    = gpRadioplayerRegKey->FmFreq;

    if (!CheckMsg(MSG_RADIOSUBWIN_REC_OR_DELETE_REC))
        FmFunctionSele = 0;

    for (i = 0; i < FREQMAXNUMBLE; i++)
    {
        gSysConfig.RadioConfig.FmFreqArray[i] = gpRadioplayerRegKey->FmFreqArray[i];
    }

    FM_Process(MSG_FM_DEINIT, NULL);

    FM_Stop(1);

    AutoPowerOffEnable();

    FREQ_ExitModule(FREQ_FM);
}

/*
--------------------------------------------------------------------------------
  Function name : void FmPowerDown(void)
  Author        : wangping
  Description   : fm power down , fm is auto power up when sys reset
  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void FmPowerOnInit(void)
{
    UINT32 FmDriverID;
    DEBUG("FmPowerOnInit");
    ModuleOverlay(MODULE_ID_FM_CONTROL, MODULE_OVERLAY_ALL);
    FM_DriveInit(&FmDriverID);
    ModuleOverlay(MODULE_ID_FM_DRIVER1 + FmDriverID, MODULE_OVERLAY_ALL);
    FmDriverTable.Tuner_MuteControl(1);
    FmDriverTable.Tuner_PowerDown();
}

#endif

/*
********************************************************************************
*
*                         End of FmControl.c
*
********************************************************************************
*/




