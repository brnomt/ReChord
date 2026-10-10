/******************************************************************/

#include "SysInclude.h"

#ifdef _RADIO_

#include "hw_fm5807.h"

#if (RKNANOD_CHIP_TYPE == RKNANOD_N)
#define FM5807_I2C_IOPORT  I2C_CH0_PB
#define FM5807_I2C_CHANNEL  I2C_CH0
#elif (RKNANOD_CHIP_TYPE == RKNANOD_M)
#define FM5807_I2C_IOPORT  I2C_CH0_PA
#define FM5807_I2C_CHANNEL  I2C_CH0
#else
#define FM5807_I2C_IOPORT  I2C_CH1_PA
#define FM5807_I2C_CHANNEL  I2C_CH1
#endif

/**************************************************************************************
*   Name    : FmChanToFreq
*   Function: CH turn to frequency
*   Input   : ch   (channel)
*   Return  : frequency(frequency*10)
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
UINT32 FM5807_FmChanToFreq(UINT8 channel)
{
    UINT16 channelSpacing;
    UINT16 bottomOfBand;
    UINT16 frequency;
    if ((RDA5800REG_IMG[3] & 0x0002) == 0x0000)
        bottomOfBand = 870;
    else
        bottomOfBand = 760;
    if ((RDA5800REG_IMG[3] & 0x0001) == 0x0000)
        channelSpacing = 1;
    else if ((RDA5800REG_IMG[3] & 0x0001) == 0x0001)
        channelSpacing = 2;
    else
        channelSpacing = 1;
    frequency = (bottomOfBand + channelSpacing * channel);
    return (frequency*10);
}


/**************************************************************************************
*   Name    : FmFreqToChan
*   Function: frequency turn to CH
*   Input   : frequency(frequency)
*   Return  : ch  (channel)
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
UINT8 FmFreqToChan(UINT32 frequency)
{
    UINT16 channelSpacing;
    UINT16 bottomOfBand;
    UINT8 channel;
    frequency = frequency / 10;

    if ((RDA5800REG_IMG[3] & 0x000C) == 0x0000)
        bottomOfBand = 870;
    else if ( ((RDA5800REG_IMG[3] & 0x000C) == 0x0004) ||((RDA5800REG_IMG[3] & 0x000C) == 0x0008))
        bottomOfBand = 760;
    else if ((RDA5800REG_IMG[3] & 0x000C) == 0x000C)
        bottomOfBand = 650;


    if ((RDA5800REG_IMG[3] & 0x0003) == 0x0000)
        channelSpacing = 1;
    else if ((RDA5800REG_IMG[3] & 0x0003) == 0x0001)
        channelSpacing = 2;
    else
        channelSpacing = 1;
    channel = (UINT8)((frequency - bottomOfBand) / channelSpacing);
    return (channel);
}


/**************************************************************************************
*   Name    : Tuner_ReadData
*   Function: Read data form FM chip (TEA5800).
*   Input   : pdata -- the pointer from which read out data.
*             size --  the size want to read.
*   Output  : null
*   Return  : TRUE --  Read success
*             FALSE -- Read failure
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
BOOL FM5807_Tuner_ReadData(UINT8 *pdata, UINT8 size)
{
    BOOL ret;
    I2C_Init(FM5807_I2C_IOPORT,FM5800, 400);//210
    ret = I2C_Read(FM5807_I2C_CHANNEL,0, pdata, size, DirectMode);
    I2C_Deinit(FM5807_I2C_IOPORT);
    return ret;
}


/**************************************************************************************
*   Name    : Tuner_SendCommand
*   Function: send data to FM chip (TEA5800).
*   Input   : pdata -- the pointer of would send data.
*             size  -- the size of would send data.
*   Output  : null
*   Return  : TRUE  -- send seccess
*             FALSE -- send failure
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
BOOL FM5807_Tuner_SendCommand(UINT8 *pdata, UINT8 size)
{
    BOOL ret;
    I2C_Init(FM5807_I2C_IOPORT,FM5800, 400);
    ret = I2C_Write(FM5807_I2C_CHANNEL,0, pdata, size, DirectMode);
    I2C_Deinit(FM5807_I2C_IOPORT);

#if(RKNANOD_CHIP_TYPE == RKNANOD_N)
    #ifdef _UART_DEBUG_
        #if(DEBUG_UART_PORT == 1)
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_UART1A_TX);
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_UART1A_RX);
        #endif
    #endif
#endif
    return ret;
}

/**************************************************************************************
*   Name    : FmInitReady
*   Function: initial W/R registers
*   Input   : NULL
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FmInitReady(void)
{
    UINT8 ReadData8[2];
    FM5807_Tuner_ReadData(ReadData8, 2);
}

/**************************************************************************************
*   Name    : FmInitPowerOn
*   Function: power on
*   Input   : area  0:china 1:japan
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_FmInitPowerOn(UINT8 area)
{
    int i, j;
    UINT8 writeData8[2]={0x00,0x02}, xbTemp;

    FM5807_Tuner_SendCommand(writeData8, 2);
    DelayMs(10);
    for (xbTemp = 0; xbTemp < 60; xbTemp++)
        RDA5800REG_IMG[xbTemp] = cwFM5807Default[xbTemp];
    if (area == 1)
        RDA5800REG_IMG[3] |= RUNER_AREA_JA;
    else
        RDA5800REG_IMG[3] &= (~RUNER_AREA_JA);
}

/**************************************************************************************
*   Name    : FmAreaSwitch
*   Function: change area
*   Input   : area  0:china/us     1:japan
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FmAreaSwitch(UINT8 area)
{
    if (area == 1)
        RDA5800REG_IMG[3] |= RUNER_AREA_JA;
    else
        RDA5800REG_IMG[3] &= (~RUNER_AREA_JA);
}

/**************************************************************************************
*   Name    : FmInitData
*   Function: fm registers initial
*   Input   : NULL
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FmInitData(void)
{
    int i, j;
    UINT8 writeData8[86], xbTemp;
    for (xbTemp = 0; xbTemp < 24; xbTemp++)
    {
        writeData8[xbTemp*2] = (RDA5800REG_IMG[xbTemp+2] >> 8);
        writeData8[xbTemp*2+1] = (RDA5800REG_IMG[xbTemp+2]);
    }
    FM5807_Tuner_SendCommand(writeData8, 48);
}

/**************************************************************************************
*   Name    : FmInit5800
*   Function: search initial
*   Input   : NULL
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FmInit5800(void)
{
  /*  int i, j;
    UINT8 writeData8[2], xbTemp;
    writeData8[0] = 0xd8 & (~0x40);  //0x02   0xd881
    writeData8[1] = 0x81;  //0x81;
    FM5807_Tuner_SendCommand(writeData8, 2);
    */
}

/**************************************************************************************
*   Name    : FmTune
*   Function: assign value to fm registers.
*   Input   : frequency
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FmTune(UINT32 frequency)
{
    UINT8  writeData8[4];
    int temp = 0;
    temp = FmFreqToChan(frequency);
    RDA5800REG_IMG[3] &= 0x003f;
    RDA5800REG_IMG[3] |= (((int)(temp)&0x3) <<6);
    RDA5800REG_IMG[3] |= ((((int)(temp)>>2)<<8) );

    writeData8[0] = (RDA5800REG_IMG[2] >> 8) & ~TUNER_MUTE; //0x02   0xd081
    writeData8[1] = (RDA5800REG_IMG[2]);
    writeData8[2] = (RDA5800REG_IMG[3] >> 8); //0x03
    writeData8[3] = (RDA5800REG_IMG[3]);
    FM5807_Tuner_SendCommand(writeData8, 4);

    /*
    DelayMs(160);
    do
    {
        DelayMs(5);
        FM5807_Tuner_ReadData(writeData8, 4);
    }
    while ((writeData8[0] & 0x40 == 0));
    printf("RSSI = %d, fmreq = %d\n", writeData8[2], frequency);
   */

}

/**************************************************************************************
*   Name    : FmSingleSeek
*   Function: fm search registers initial.
*   Input   : seek Direction
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FmSingleSeek(UINT8  seekDirection)
{
    /*
        UINT8 readData8[4];
        UINT8 writeData8[8];
        //set seek bit
        if (seekDirection == 0)
            writeData8[0] = ((RDA5800REG_IMG[2] >> 8) | RUNER_SEEK_DN); // seek down0x01
        else
            writeData8[0] = ((RDA5800REG_IMG[2] >> 8) | RUNER_SEEK_UP);  // seek up0x03

        writeData8[0] |= 0x01;
        writeData8[1] = RDA5800REG_IMG[2];
        FM5807_Tuner_SendCommand(writeData8, 2);
    */
}

/**************************************************************************************
*   Name    : FmSeek
*   Function: fm search
*   Input   : seek Direction
*   Return  : current frequency
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
UINT16 FM5807_FmSeek(UINT8 seekDirection, UINT32 FmFreq)
{
#define TUNE_FIND_TUNE 0X00
#define TUNE_CHECK_FD  0X01
#define TUNE_CHECK_FW  0X02


    UINT8 readData8[4];
    //UINT8 writeData8[8];
    BOOL fStopSeek = 0;
    BOOL falseStation = 0;
    UINT16 channelflag = 0;
    UINT8  seekover = 0;
    UINT8 TuneState, LastRSSI;
    UINT8 Sensitivity;
    TuneState = TUNE_FIND_TUNE;

    if (gSysConfig.RadioConfig.ScanSensitivity==0) // 0: high, 1: low
    {
        //Sensitivity = 40;// Hign Scan Sensitivity
        Sensitivity = 66; // High Scan Sensitivity
    }
    else
    {
        //Sensitivity = 30;// Low Scan Sensitivity
        Sensitivity = 50; // Low Scan Sensitivity
    }

    //FREQ_EnterModule(FREQ_AUDIO_INIT);

TUNE_CHECK:
    FmTune(FmFreq);
    DelayMs(160);
    do
    {
        seekover++;
        DelayMs(5);
        FM5807_Tuner_ReadData(readData8, 4);
        //printf("seekover = %d\n", seekover);
    }
    while ((readData8[0] & 0x40 == 0) && (seekover <= 8));

    //printf("RSSI = %d, fmreq = %d\n", readData8[2], FmFreq);

    if (readData8[2] < Sensitivity)
    {
        switch (TuneState)
        {
            case TUNE_FIND_TUNE:
                falseStation = 1;
                break;

            case TUNE_CHECK_FD:
                FmFreq -= 20;
                TuneState = TUNE_CHECK_FW;
                goto TUNE_CHECK;

            case TUNE_CHECK_FW:
                if (LastRSSI & 0X01)
                {
                    falseStation = 0;
                }
                else
                {
                    falseStation = 1;
                }
                break;
        }

    }
    else
    {
        switch (TuneState)
        {
            case TUNE_FIND_TUNE:
                LastRSSI = readData8[2];
                FmFreq += 10;
                TuneState = TUNE_CHECK_FD;
                goto TUNE_CHECK;

            case TUNE_CHECK_FD:
                if (LastRSSI <= readData8[2])
                {
                    falseStation = 1;
                    break;
                }
                else
                {
                    FmFreq -= 20;
                    TuneState = TUNE_CHECK_FW;
                    goto TUNE_CHECK;
                }


            case TUNE_CHECK_FW:
                if (LastRSSI <= readData8[2])
                {
                    falseStation = 1;
                }
                else
                {
                    if (LastRSSI & 0X01)
                    {
                        falseStation = 0;
                    }
                    else
                    {
                        falseStation = 1;
                    }
                }

                break;

        }
    }

    if (falseStation == 0)
    {
        channelflag |= 0x0001;
        return channelflag;
    }
    else
    {
        return channelflag;
    }
    //FREQ_ExitModule(FREQ_AUDIO_INIT);

#undef TUNE_FIND_TUNE
#undef TUNE_CHECK_FD
#undef TUNE_CHECK_FW
}

/**************************************************************************************
*   Name    : Tuner_Enable
*   Function: Enable FM chip (TEA5800).
*   Input   : area  1:japan 0: china/us
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_Enable(UINT8 area)
{
    FM5807_FmInitPowerOn(area);
    FmInitData();
}

/**************************************************************************************
*   Name    : Tuner_Disable
*   Function: Disable FM chip (TEA5807)
*   Input   : NULL
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_Disable(void)
{
    UINT8 writeData8[2];
    writeData8[0] = 0xd8;  //0x02   0xd881
    writeData8[1] = 0x80;  //0x81;
    FM5807_Tuner_SendCommand(writeData8, 2);
    //DelayMs_nops(100);
}

/**************************************************************************************
*   Name    : Tuner_PowerOnInit
*   Function: FM chip (TEA5807) power on initialization.
*   Input   : NULL
*   Output  : null
*   Return  : NULL
*   explain :frist close FM chips before power on.
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_PowerOnInit(void)
{
    FM5807_Tuner_Disable();
}

/**************************************************************************************
*   Name    : Tuner_SetStereo
*   Function: Fm Radio Stereo Setting
*   ÊäÈë²ÎÊý£ºbStereo  0 ST  1MO
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_SetStereo(BOOL bStereo)
{
    UINT8 writeData8[1];
    UINT8 ReadData8[1];

    if (bStereo)
    {
        writeData8[0] = ((RDA5800REG_IMG[2] >> 8) & (~TUNER_MONO));
    }
    else
    {
        writeData8[0] = ((RDA5800REG_IMG[2] >> 8) | TUNER_MONO);
    }
    FM5807_Tuner_SendCommand(writeData8, 1);
}

/**************************************************************************************
*   Name    : Tuner_SetMute
*   Function: Fm Radio Mute Setting
*   Input   :
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_SetMute(BOOL  mute)
{
    UINT8 writeData8[2];

    writeData8[0] = RDA5800REG_IMG[2] >> 8;
    writeData8[1] = RDA5800REG_IMG[2] & 0xff;

    if (mute == 0)
    {
        writeData8[0] |= 0x40;
    }
    else
    {
        writeData8[0] &= ~0x40;
    }

    FM5807_Tuner_SendCommand(writeData8, 2);
}

/**************************************************************************************
*   Name    : GetStereoStatus
*   Function: get stereo status
*   Input   : NULL
*   Output  : null
*   Return  : TRUE  -- stereo status
*             FALSE -- non stero status
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
BOOLEAN FM5807_GetStereoStatus(void)
{
    UINT8 readData8[1];
    FM5807_Tuner_ReadData(readData8, 0);
    return (readData8[0]&0x04);
}

/**************************************************************************************
*   Name    : Tuner_PowerOffDeinit
*   Function: Fm Radio Power Off Deinit
*   Input   : NULL
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_PowerOffDeinit(void)
{
    FM5807_Tuner_Disable();

}

/**************************************************************************************
*   Name    : Tuner_SetFrequency
*   Function: set FM chip(TEA5800) frequency.
*   Input   : n10KHz -- frequency(unit:10KHz)
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_SetFrequency(UINT32 n10KHz, UINT8 HILO, BOOL ForceMono,UINT16 Area)
{
    FmTune(n10KHz);
}


_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_MuteControl(bool active)
{
    FM5807_Tuner_SetMute(active);
}

_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_PowerDown(void)
{
    FM5807_Tuner_PowerOffDeinit();
}

/**************************************************************************************
*   Name    : FMSearchByHand
*   Function: search FM station by handle.
*   Input   : direct -- 1(increment), 0(decrease)
*   Output  : FM_FoundStation
*   Return  : result
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
UINT16 FM5807_Tuner_SearchByHand(UINT16 direct, UINT32 *FmFreq)
{
    UINT16 channelflag = 0;

    channelflag = FM5807_FmSeek(direct, *FmFreq);

    PaintCHAndFreq();

    if ((channelflag&0x0001) == 1)
        return 2;   //found a valid station
    else
        return 0;   //FM_SearchFail; found no valid station.
}


/**************************************************************************************
*   Name    : Tuner_SetInitArea
*   Function: set FM chip(TEA5767) frequency.
*   Input   :
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_SetInitArea(UINT8 area)
{
    if (RADIO_JAPAN==area)
        FM5807_Tuner_Enable(1);
    else
        FM5807_Tuner_Enable(0);
}

_ATTR_FMDRIVER_FM5807_TEXT_
void FM5807_Tuner_Vol_Set(UINT8 gain)
{
    UINT8 writeData8[8];

    writeData8[0] = RDA5800REG_IMG[2] >> 8;
    writeData8[1] = RDA5800REG_IMG[2] & 0xff;

    writeData8[2] = RDA5800REG_IMG[3] >> 8;
    writeData8[3] = RDA5800REG_IMG[3] & 0xff;

    writeData8[4] = RDA5800REG_IMG[4] >> 8;
    writeData8[5] = RDA5800REG_IMG[4] & 0xff;

    writeData8[6] = RDA5800REG_IMG[5] >> 8;
    writeData8[7] = RDA5800REG_IMG[5] & 0xff;
    writeData8[7] &= 0xF0;
    //writeData8[7] |= (Fm5807_Vol_Table[gain] & 0x0F);
    writeData8[7] |= (gain & 0x0F);
    writeData8[3] &= ~0x10; // for audio break

    RDA5800REG_IMG[5] = (RDA5800REG_IMG[5] & 0xfff0) + (writeData8[7] & 0x0f);
    FM5807_Tuner_SendCommand(writeData8, 8);
}

//#endif
#endif
