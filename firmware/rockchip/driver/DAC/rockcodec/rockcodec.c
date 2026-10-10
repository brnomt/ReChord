/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��   Rockcodec.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo          2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "DriverInclude.h"


#include "rockcodec.h"
#include "codec.h"

#ifdef _MUSIC_
#include "AudioControl.h"
#endif

#ifdef _RADIO_
#include "FMControl.h"
#endif
#define ReadAcodecReg(addr)                     (*(volatile uint32 *)(addr))
#define WriteAcodecReg(addr, data)              (*(volatile uint32 *)(addr) = data)
#define SetAcodecRegData(addr, databit)         WriteAcodecReg(addr, ReadAcodecReg(addr)|(databit))
#define ClrAcodecRegData(addr, databit)         WriteAcodecReg(addr, ReadAcodecReg(addr)&(~databit))

#define SetAcodecRegBit(addr,bit)               WriteAcodecReg(addr,(ReadAcodecReg(addr)|(1<<bit)))
#define ClrAcodecRegBit(addr,bit)               WriteAcodecReg(addr,(ReadAcodecReg(addr)&(~(1<<bit))))
#define GetAcodecRegBit(addr,bit)               (ReadAcodecReg(addr)&(1<<bit))
#define MaskAcodecRegBits(addr, y, z)           WriteAcodecReg((addr), ((ReadAcodecReg(addr))&(~(y)))|(z))

ACodecI2S_mode_t Acodec_I2S_MODE;
eACodecPll_Target_Freq pll_old_target;
CodecMode_en_t Codecmode_Bak;
CodecFS_en_t CodecFS_Bak;
eACodecI2sDATA_WIDTH_t CodecDataWidth_Bak;
uint32 Acodec_output_sel;
uint32 Acodec_suspend_En;

_ATTR_SYS_BSS_ uint32 HP_DET_TICKCount;
_ATTR_SYS_BSS_ uint8  HP_DET_Status;
_ATTR_SYS_BSS_ uint8  HP_DET_Status_check;
_ATTR_SYS_BSS_ uint32 ACodec_Get_Over_Current_TICKCount;
_ATTR_SYS_BSS_ uint32 Volume_Bak;

typedef struct tagAPLL_APP
{
    uint32 F_source;    //KHz
    uint32 F_target;    //KHz
    uint32 PLL_POSDIV_L3;
    uint32 PLL_POSDIV_H8;
    uint32 PLL_PREDIV_BIT;
    uint32 PLL_OUTDIV;
} ACodecPLL_APP, *pACodecPLL_APP;

/*
***************************************************
PLL_OUTDIV configer
bit       7 6               5 4
div5     0 0               0 0   div3
div3     1 0               1 0   div2
div10    0 1               0 1   div1
div6     1 1               1 1   div1
***************************************************
*/
static const ACodecPLL_APP ACodecpllTable_61440[8] =
{
    {2048  , 61440 , 0  , 45, 2,  6},
    {3072  , 61440 , 0  , 45, 3,  6},
    {4096  , 61440 , 0  , 45, 4,  6},
    {6000  , 61440 , 0  , 64, 10, 5},
    {6144  , 61440 , 0  , 45, 6,  6},
    {12000 , 61440 , 0  , 96, 25, 6},
    {12288 , 61440 , 0  , 45, 12, 6},
    {24000 , 61440 , 0  , 48, 25, 10}, //PLL_OUTDIV = 6

};

static const ACodecPLL_APP ACodecpllTable_56448[8] =
{
    {2048  , 56448 , 3  , 41, 2,  6},
    {3072  , 56448 , 1  , 55, 4,  6},
    {4096  , 56448 , 3  , 41, 4,  6},
    {6000  , 56448 , 0  , 147, 25, 5},
    {6144  , 56448 , 1  , 55, 8,  6},
    {12000 , 56448 , 4  , 73, 25, 5},
    {12288 , 56448 , 1  , 55, 16, 6},
    {24000 , 56448 , 6  , 36, 25, 3}, //PLL_OUTDIV = 5

};

static const ACodecPLL_APP ACodecpllTable_40960[8] =
{
    {2048  , 40960 , 4   , 22, 1,  9},
    {3072  , 40960 , 0   , 45, 3,  9},
    {4096  , 40960 , 4   , 22, 2,  9},
    {6000  , 40960 , 0   , 192, 25, 9},
    {6144  , 40960 , 0   , 40, 6,  9},
    {12000 , 40960 , 0   , 96, 25, 9},
    {12288 , 40960 , 0   , 45, 12, 9},
    {24000 , 40960 , 0   , 96, 50, 8}, //PLL_OUTDIV = 9

};

/*******************************************************************************
** Name: ACodec_PLL_Set
** Input:pll_target
** Return: void
*******************************************************************************/
_ATTR_DRIVER_CODE_
void ACodec_PLL_Set(eF_SOURCE_ID F_SOURCE, eACodecPll_Target_Freq pll_target)
{
    uint32 MHz, POSDIV_L3, POSDIV_H8, PREDIV_BIT, OUTDIV;
    uint32 config, i;

    if (F_SOURCE >= F_SOURCE_MAX)
    {
        return;
    }

    if (pll_old_target == pll_target)
    {
        return;  //���PLLһ���Ͳ���Ҫ�������ã�ֻ��Ҫ����SCK_DIV����
    }

    pll_old_target = pll_target;

    switch (pll_target)
    {
        case Pll_Target_Freq_40960:
            POSDIV_L3  = ACodecpllTable_40960[F_SOURCE].PLL_POSDIV_L3;
            POSDIV_H8  = ACodecpllTable_40960[F_SOURCE].PLL_POSDIV_H8;
            PREDIV_BIT = ACodecpllTable_40960[F_SOURCE].PLL_PREDIV_BIT;
            OUTDIV     = ACodecpllTable_40960[F_SOURCE].PLL_OUTDIV;
            break;

        case Pll_Target_Freq_56448:
            POSDIV_L3  = ACodecpllTable_56448[F_SOURCE].PLL_POSDIV_L3;
            POSDIV_H8  = ACodecpllTable_56448[F_SOURCE].PLL_POSDIV_H8;
            PREDIV_BIT = ACodecpllTable_56448[F_SOURCE].PLL_PREDIV_BIT;
            OUTDIV     = ACodecpllTable_56448[F_SOURCE].PLL_OUTDIV;
            break;

        case Pll_Target_Freq_61440:
            POSDIV_L3  = ACodecpllTable_61440[F_SOURCE].PLL_POSDIV_L3;
            POSDIV_H8  = ACodecpllTable_61440[F_SOURCE].PLL_POSDIV_H8;
            PREDIV_BIT = ACodecpllTable_61440[F_SOURCE].PLL_PREDIV_BIT;
            OUTDIV     = ACodecpllTable_61440[F_SOURCE].PLL_OUTDIV;
            break;

        default:
            break;
    }

    //disable out vco
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG0, PLL_OUTDIV_ENABLE, PLL_OUTDIV_DISABLE);

    //PLL RESET pwd down
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG5, APLL_RESET, APLL_RESET);
    //MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,PLL_PW_DOWN,PLL_PW_DOWN);

    //Select 24M as the source clock of PLL
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG0, (0x03 << 3), PLL_CLKIN_SEL_S_CLOCK);

    WriteAcodecReg(ACODEC_BASE + ACODEC_PLLCFG1, POSDIV_L3);
    WriteAcodecReg(ACODEC_BASE + ACODEC_PLLCFG2, POSDIV_H8);
    WriteAcodecReg(ACODEC_BASE + ACODEC_PLLCFG3, PREDIV_BIT);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG4, (0xf << 4), (OUTDIV << 4));

    //PLL power on
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG5, PLL_PW_DOWN, PLL_PW_UP);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG5, APLL_RESET, APLL_RELEASE_RESET);

    DelayMs(5);
    //enable out vco
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG0, PLL_OUTDIV_ENABLE, PLL_OUTDIV_ENABLE);

}


/*******************************************************************************
** Name: ACodec_I2s_SetSampleRate
** Input:
** Return: void
*******************************************************************************/
_ATTR_DRIVER_CODE_
void ACodec_I2s_SetSampleRate(CodecFS_en_t CodecFS)
{
    uint32 config;
    uint32 DACSRT_TIME = 0;
    uint32 Sck_div, FS_value;

    DEBUG("ACodec_I2s_SetSampleRate");

    switch (CodecFS)
    {
        case FS_8000Hz:
        case FS_11025Hz:
        case FS_12KHz:
            DACSRT_TIME = 0;
            Sck_div     = 16;
            break;

        case FS_16KHz:
        case FS_22050Hz:
        case FS_24KHz:
            DACSRT_TIME = 1;
            Sck_div     = 8;
            break;

        case FS_32KHz:
        case FS_44100Hz:
        case FS_48KHz:
            DACSRT_TIME = 2;
            Sck_div     = 4;
            break;

        case FS_64KHz:
        case FS_88200Hz:
        case FS_96KHz:
            DACSRT_TIME = 3;
            Sck_div     = 2;
            break;

        case FS_128KHz:
        case FS_1764KHz:
        case FS_192KHz:
            DACSRT_TIME = 4;
            Sck_div     = 1;
            break;

        default:
            break;
    }

    WriteAcodecReg(ACODEC_BASE + ACODEC_DACSR, DACSRT_TIME);
    WriteAcodecReg(ACODEC_BASE + ACODEC_ADCSR, DACSRT_TIME);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_I2SCKM, (0xF << 4), ((Sck_div - 1) << 4));
}


#if (CODEC_CONFIG == CODEC_ROCKC)
/*******************************************************************************
** Name: Volume Table
** Input:void
** Return: void
*******************************************************************************/
_ATTR_DRIVER_DATA_
SRUCT_CODEC_CONFIG ACodec_LineOutVol_General[MAX_VOLUME + 1] =
{
//+0dB            //+4dB     //+7db       //+12dB   //BASS
    0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254, //  0
    0, 140, 15, 1,  0, 165,   0, 157,      0, 124,   0, 124, //  1
    0, 135, 15, 1,  0, 153,   0, 145,      0, 132,   0, 132, //  2
    0, 123, 15, 1,  0, 148,   0, 140,      0, 127,   0, 127, //  3
    0, 123, 11, 1,  0, 145,   0, 137,      0, 124,   0, 124, //  4
    0, 123, 7,  1,  0, 143,   0, 135,      0, 122,   0, 122, //  5
    0, 123, 4,  1,  0, 144,   0, 136,      0, 123,   0, 123, //  6
    0, 123, 2,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  7
    0, 123, 0,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  8
    0, 117, 0,  1,  0, 134,   0, 126,      0, 113,   0, 113, //  9
    0, 111, 0,  1,  0, 127,   0, 119,      0, 106,   0, 106, //  10
    0, 107, 0,  1,  0, 122,   0, 114,      0, 101,   0, 101, //  11
    0, 101, 0,  1,  0, 116,   0, 108,      0, 95,    0, 95,  //  12
    0, 96,  0,  1,  0, 110,   0, 102,      0, 89,    0, 89,  //  13
    0, 91,  0,  1,  0, 103,   0, 95,       0, 82,    0, 82,  //  14
    0, 87,  0,  1,  0, 98,    0, 90,       0, 77,    0, 77,  //  15
    0, 81,  0,  1,  0, 93,    0, 85,       0, 72,    0, 72,  //  16
    0, 77,  0,  1,  0, 84,    0, 76,       0, 63,    0, 63,  //  17
    0, 72,  0,  1,  0, 80,    0, 72,       0, 59,    0, 59,  //  18
    0, 67,  0,  1,  0, 77,    0, 69,       0, 56,    0, 56,  //  19
    0, 62,  0,  1,  0, 69,    0, 61,       0, 48,    0, 48,  //  20
    0, 57,  0,  1,  0, 61,    0, 53,       0, 40,    0, 40,  //  21
    0, 52,  0,  1,  0, 56,    0, 48,       0, 35,    0, 35,  //  22
    0, 47,  0,  1,  0, 50,    0, 42,       0, 29,    0, 29,  //  23
    0, 43,  0,  1,  0, 45,    0, 37,       0, 24,    0, 24,  //  24
    0, 38,  0,  1,  0, 40,    0, 32,       0, 19,    0, 19,  //  25
    0, 34,  0,  1,  0, 33,    0, 25,       0, 12,    0, 12,  //  26
    0, 28,  0,  1,  0, 28,    0, 20,       0, 7,     0, 7,   //  27
    0, 24,  0,  1,  0, 22,    0, 14,       0, 1,     0, 1,    //  28
    0, 19,  0,  1,  0, 17,    0, 9,        1, 4,     1, 4,    //  29
    0, 14,  0,  1,  0, 14,    0, 6,        1, 1,     1, 1,    //  30
    0, 9,   0,  1,  0, 17,    0, 9,        1, 4,     1, 4,    //  31
    0, 4,   0,  1,  0, 14,    0, 6,        1, 1,     1, 1,    //  32
};


// HP_AMPVol;EQ_POP_HEAVY_HP_AMPVol;EQ_JAZZ_UNIQUE_HP_AMPVol;EQ_USER_HP_AMPVol;EQ_BASS_HP_AMPVol
// Description : increase the HP amplitude from 3dB to 9dB
//               0: 0 dB;1: 3 dB;2: 6 dB;3: 9 dB

// Dac_DigVol;EQ_POP_HEAVY_Dac_DigVol;EQ_JAZZ_UNIQUE_Dac_DigVol;EQ_USER_Dac_DigVol;EQ_BASS_Dac_DigVol
// Description : digital volume of DAC channel
//               0.375db/step: 0 dB - (-95) dB
//               0x00: 0dB; 0xff : -95dB

// HP_ANTIPOPVol
// Description : decrease the HP amplitude from 0dB to -15dB
//               1 db/step: 0 dB - (-15) dB
//               0x00: 0dB; 0xf : -15dB

_ATTR_DRIVER_DATA_
SRUCT_CODEC_CONFIG ACodec_HPoutVol_General[MAX_VOLUME + 1] =
{
//+0dB            //+4dB     //+7db       //+12dB   //BASS
    0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254, //  0
    0, 144, 15, 1,  0, 134,   0, 125,      0, 112,   0, 112, //  1
    0, 140, 15, 1,  0, 130,   0, 121,      0, 108,   0, 108, //  2
    0, 137, 15, 1,  0, 127,   0, 118,      0, 105,   0, 105, //  3
    0, 135, 12, 1,  0, 125,   0, 116,      0, 103,   0, 103, //  4
    0, 135, 9,  1,  0, 125,   0, 116,      0, 103,   0, 103, //  5
    0, 135, 6,  1,  0, 125,   0, 116,      0, 103,   0, 103, //  6
    0, 135, 3,  1,  0, 125,   0, 116,      0, 103,   0, 103, //  7
    0, 135, 0,  1,  0, 125,   0, 116,      0, 103,   0, 103, //  8
    0, 132, 0,  1,  0, 122,   0, 113,      0, 100,   0, 100, //  9
    0, 129, 0,  1,  0, 119,   0, 110,      0, 97,    0, 97, //  10
    0, 122, 0,  1,  0, 112,   0, 103,      0, 90,    0, 90, //  11
    0, 115, 0,  1,  0, 105,   0, 96,       0, 83,    0, 83,  //  12
    0, 110, 0,  1,  0, 100,   0, 91,       0, 78,    0, 78,  //  13
    0, 104, 0,  1,  0, 94,    0, 85,       0, 72,    0, 72,  //  14
    0, 98,  0,  1,  0, 88,    0, 79,       0, 66,    0, 66,  //  15
    0, 92,  0,  1,  0, 82,    0, 73,       0, 60,    0, 60,  //  16
    0, 87,  0,  1,  0, 77,    0, 68,       0, 55,    0, 55,  //  17
    0, 81,  0,  1,  0, 71,    0, 62,       0, 49,    0, 49,  //  18
    0, 75,  0,  1,  0, 65,    0, 56,       0, 43,    0, 43,  //  19
    0, 70,  0,  1,  0, 60,    0, 51,       0, 38,    0, 38,  //  20
    0, 64,  0,  1,  0, 54,    0, 46,       0, 32,    0, 32,  //  21
    0, 58,  0,  1,  0, 48,    0, 40,       0, 26,    0, 26,  //  22
    0, 52,  0,  1,  0, 42,    0, 34,       0, 20,    0, 20,  //  23
    0, 47,  0,  1,  0, 37,    0, 29,       0, 15,    0, 15,  //  24
    0, 41,  0,  1,  0, 30,    0, 23,       0, 7,     0, 7,  //  25
    0, 35,  0,  1,  0, 23,    0, 17,       0, 2,     0, 2,  //  26
    0, 30,  0,  1,  0, 18,    0, 12,       1, 3,     1, 3,   //  27
    0, 24,  0,  1,  0, 14,    0, 6,        2, 8,     2, 5,    //  28
    0, 19,  0,  1,  0, 10,    0, 2,        2, 5,     2, 2,    //  29
    0, 16,  0,  1,  0, 7,     1, 7,        2, 2,     2, 0,    //  30
    0, 14,  0,  1,  0, 2,     1, 4,        2, 0,     3, 4,    //  31
    0, 13,  0,  1,  1, 4,     1, 2,        3, 7,     3, 2,    //  32

};


// FmRegVol
// Description : FM VOL control from 0dB to 15dB
//               0x00: 0dB; 0xf : 15dB

// ADCDigVol
// Description : digital volume of ADC channel
//               0.375db/step: 0 dB - (-95) dB
//               0x00: 0dB; 0xff : -95dB

// ADCMuxVol
// Description : change the gain of MUX block, the value changed from -18dB to 27dB, 3db/step.
//                  0000: -18db
//                  1111: 27db

// HP_AMPVol;
// Description : increase the HP amplitude from 3dB to 9dB
//               0: 0 dB;1: 3 dB;2: 6 dB;3: 9 dB

// Dac_DigVol;
// Description : digital volume of DAC channel
//               0.375db/step: 0 dB - (-95) dB
//               0x00: 0dB; 0xff : -95dB

// HP_ANTIPOPVol
// Description : decrease the HP amplitude from 0dB to -15dB
//               1 db/step: 0 dB - (-15) dB
//               0x00: 0dB; 0xf : -15dB
_ATTR_DRIVER_DATA_
FM_SRUCT_CODEC_CONFIG FmCodecConfig_General[MAX_VOLUME + 1] =
{
    15, 0, 12,   0, 254, 15,  //  0
    15, 0, 12,   0, 144, 15,  //  1
    15, 0, 12,   0, 140, 15,  //  2
    15, 0, 12,   0, 137, 15,  //  3
    15, 0, 12,   0, 135, 12,  //  4
    15, 0, 12,   0, 135, 9,    //  5
    15, 0, 12,   0, 135, 6,    //  6
    15, 0, 12,   0, 135, 3,    //  7
    15, 0, 12,   0, 135, 0,    //  8
    15, 0, 12,   0, 132, 0,    //  9
    15, 0, 12,   0, 129, 0,    //  10
    15, 0, 12,   0, 122, 0,    //  11
    15, 0, 12,   0, 115, 0,     //  12
    15, 0, 12,   0, 110, 0,     //  13
    15, 0, 12,   0, 104, 0,     //  14
    15, 0, 12,   0, 98,  0,     //  15
    15, 0, 12,   0, 92,  0,     //  16
    15, 0, 12,   0, 87,  0,     //  17
    15, 0, 12,   0, 81,  0,     //  18
    15, 0, 12,   0, 75,  0,     //  19
    15, 0, 12,   0, 70,  0,     //  20
    15, 0, 12,   0, 64,  0,     //  21
    15, 0, 12,   0, 58,  0,     //  22
    15, 0, 12,   0, 52,  0,     //  23
    15, 0, 12,   0, 47,  0,     //  24
    15, 0, 12,   0, 41,  0,     //  25
    15, 0, 12,   0, 35,  0,     //  26
    15, 0, 12,   0, 30,  0,     //  27
    15, 0, 12,   0, 24,  0,      //  28
    15, 0, 12,   0, 19,  0,      //  29
    15, 0, 12,   0, 16,  0,      //  30
    15, 0, 12,   0, 14,   0,     //  31
    15, 0, 12,   0, 13,   0,     //  32

};

/*******************************************************************************
** Name: ACodec_Soft_Reset
** Input:
** Return: void
*******************************************************************************/
_ATTR_DRIVER_CODE_
void ACodec_Soft_Reset(void)
{
    uint32 timeout = 200000;
    WriteAcodecReg(ACODEC_BASE + ACODEC_SRST, SOFT_RESET);

    while (ReadAcodecReg(ACODEC_BASE + ACODEC_SRST))
    {
        DelayUs(1);

        if (--timeout == 0)
        {
            break;
        }
    }
}

/*******************************************************************************
** Name: ACodec_get_over_current_value
** Input:
** Return:
*******************************************************************************/
_ATTR_DRIVER_CODE_
uint8 ACodec_get_over_current_value(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG2);
    return config;
}
//
///*******************************************************************************
//** Name: ACodec_I2s_SetSampleRate
//** Input:
//** Return: void
//*******************************************************************************/
//_ATTR_DRIVER_CODE_
//void ACodec_I2s_SetSampleRate(CodecFS_en_t CodecFS)
//{
//    uint32 config;
//    uint32 DACSRT_TIME = 0;
//    uint32 Sck_div,FS_value;
//
//    switch (CodecFS)
//    {
//        case FS_8000Hz:
//        case FS_11025Hz:
//        case FS_12KHz:
//            DACSRT_TIME = 0;
//            Sck_div     = 16;
//            break;
//
//        case FS_16KHz:
//        case FS_22050Hz:
//        case FS_24KHz:
//            DACSRT_TIME = 1;
//            Sck_div     = 8;
//            break;
//
//        case FS_32KHz:
//        case FS_44100Hz:
//        case FS_48KHz:
//            DACSRT_TIME = 2;
//            Sck_div     = 4;
//            break;
//
//        case FS_64KHz:
//        case FS_88200Hz:
//        case FS_96KHz:
//            DACSRT_TIME = 3;
//            Sck_div     = 2;
//            break;
//
//        case FS_128KHz:
//        case FS_1764KHz:
//        case FS_192KHz:
//            DACSRT_TIME = 4;
//            Sck_div     = 1;
//            break;
//        default:
//            break;
//    }
//    WriteAcodecReg(ACODEC_BASE+ACODEC_DACSR,DACSRT_TIME);
//    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCSR,DACSRT_TIME);
//    MaskAcodecRegBits(ACODEC_BASE+ACODEC_I2SCKM,(0xF << 4),((Sck_div-1) << 4));
//}

/*******************************************************************************
** Name: ACodec_I2S_Init
** Input:
** Return: void
*******************************************************************************/
_ATTR_DRIVER_CODE_
void ACodec_I2S_Init(eACodecI2sFormat_t BUS_FORMAT,
                     eACodecI2sDATA_WIDTH_t I2S_Data_width,
                     ACodecI2S_BUS_MODE_t I2S_Bus_mode,
                     ACodecI2S_mode_t I2S_mode,
                     ACodecI2S_RxTx_t TxRxMode)
{
    uint32 timeout = 20000;
    uint32 cofig_data = 0;

    Acodec_I2S_MODE = I2S_mode;

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_I2SCKM, (I2S_MST_MASTER | SCK_EN_ENABLE), I2S_mode | SCK_EN_ENABLE);

    if (ACodec_I2S_TX & TxRxMode)
    {
        //I2S tx config
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCR3, 0); //RCNVT_TX = 0
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCR2, I2S_Data_width); //24bit/16bit
        cofig_data = LSB_TX_MSB | EXRL_TX_NORMAL | PBM_TX_BUS_MODE_DELAY0 | BUS_FORMAT | I2S_Bus_mode;
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCR1, cofig_data);
        cofig_data = TXRL_P_NORMAL | SCKD_TX_64_DIV;
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCR0, cofig_data);

        //enable ADC and i2s tx clk
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_CLKE, ADC_CKE_ENABLE | I2STX_CKE_ENABLE,
                          ADC_CKE_ENABLE | I2STX_CKE_ENABLE);
        //enbale i2s_tx
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_DIGEN, I2STX_EN_ENABLE , I2STX_EN_ENABLE);
        // tx transfer start
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCMD, TX_START);
    }

    if (ACodec_I2S_RX & TxRxMode)
    {
        //i2s rx config
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2SRXCR2, I2S_Data_width);
        cofig_data = LSB_RX_MSB | EXRL_RX_NORMAL | PBM_RX_BUS_MODE_DELAY0 | BUS_FORMAT | I2S_Bus_mode;
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2SRXCR1, cofig_data);
        cofig_data = RXRL_P_NORMAL | SCKD_RX_64_DIV;
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2SRXCR0, cofig_data);

        //enable dac clk ;enbale i2s_rx clk
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_CLKE, DAC_CKE_ENABLE | I2SRX_CKE_ENABLE,
                          DAC_CKE_ENABLE | I2SRX_CKE_ENABLE);
        //enbale i2s_rx
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_DIGEN, I2SRX_EN_ENABLE, I2SRX_EN_ENABLE);

        WriteAcodecReg(ACODEC_BASE + ACODEC_I2SRXCMD, RX_START);
    }

#if 0

    if (ACodec_I2S_TXRX == TxRxMode)
    {
        //i2s rx config
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2SRXCR2, I2S_Data_width);
        cofig_data = LSB_RX_MSB | EXRL_RX_NORMAL | PBM_RX_BUS_MODE_DELAY0 | BUS_FORMAT | I2S_Bus_mode;
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2SRXCR1, cofig_data);
        cofig_data = RXRL_P_NORMAL | SCKD_RX_64_DIV;
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2SRXCR0, cofig_data);

        //I2S tx config
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCR3, 0); //RCNVT_TX = 0
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCR2, I2S_Data_width); //24bit/16bit
        cofig_data = LSB_TX_MSB | EXRL_TX_NORMAL | PBM_TX_BUS_MODE_DELAY0 | BUS_FORMAT | I2S_Bus_mode;
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCR1, cofig_data);
        cofig_data = TXRL_P_NORMAL | SCKD_TX_64_DIV;
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCR0, cofig_data);

        //enable ADC and i2s tx clk
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_CLKE, ADC_CKE_ENABLE | I2STX_CKE_ENABLE | DAC_CKE_ENABLE | I2SRX_CKE_ENABLE,
                          ADC_CKE_ENABLE | I2STX_CKE_ENABLE | DAC_CKE_ENABLE | I2SRX_CKE_ENABLE);
        //enbale i2s_tx
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_DIGEN, I2STX_EN_ENABLE | I2SRX_EN_ENABLE , I2STX_EN_ENABLE | I2SRX_EN_ENABLE);

        // tx transfer start
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCMD, TX_START);
        WriteAcodecReg(ACODEC_BASE + ACODEC_I2SRXCMD, RX_START);
    }

#endif
}

/*******************************************************************************
** Name: ACodec_I2S_Deinit
** Input:
** Return: void
*******************************************************************************/
_ATTR_DRIVER_CODE_
void ACodec_I2S_Deinit(void)
{
    //Disable  i2s tx clk
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_CLKE, ADC_CKE_ENABLE  | I2STX_CKE_ENABLE  | DAC_CKE_ENABLE  | I2SRX_CKE_ENABLE,
                      ADC_CKE_DISABLE | I2STX_CKE_DISABLE | DAC_CKE_DISABLE | I2SRX_CKE_DISABLE);
    //Disable i2s_txrx
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_DIGEN, I2STX_EN_ENABLE | I2SRX_EN_ENABLE , I2STX_EN_DISABLE | I2SRX_EN_DISABLE);

    // txrx transfer stop
    WriteAcodecReg(ACODEC_BASE + ACODEC_I2STXCMD, TX_STOP | RX_STOP | RX_CLEAR | TX_CLEAR);
    WriteAcodecReg(ACODEC_BASE + ACODEC_I2SRXCMD, RX_STOP | TX_STOP);
}

/*******************************************************************************
** Name: ACodec_hp_init
** Input:
** Return: void
*******************************************************************************/
_ATTR_DRIVER_CODE_
void ACodec_hp_init(void)
{
    //DAC to HP
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACPOPD, ATPCE_DISABLE | SMTPO_ON | HPSEL_HP2DAC | ANTIPOP_ENABLE);

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG1, (0xf << 3), (0x00 << 3)); //clear HP amplitude to 0db
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG2, (0xf << 3), (0x00 << 3)); //clear LO amplitude to 0db
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG3, 0xf | HP_ANTIPOP_ENABLE, 0x0 | HP_ANTIPOP_DISABLE); //clear Hp antipop gain
#if 0
    WriteAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG3, 0xb); // 1 0 1 1  clr hp vgnd pwd
    DelayMs(5);
    WriteAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG3, 0x9); // 1 0 0 1 clr hp buf pwd
    DelayMs(5);
    WriteAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG3, 0x1); // 0 0 0 1 clr hp ostg pwd
    DelayMs(1);
#endif
    WriteAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG3, 0x0); // 0 0 0 0 clr hp short out pwd

}

/*******************************************************************************
** Name: ACodec_hp_DeInit
** Input:
** Return:
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
void ACodec_hp_DeInit(void)
{
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG1, (0xf << 3), (0x00 << 3)); //clear HP amplitude to 0db
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG2, (0xf << 3), (0x00 << 3)); //clear LO amplitude to 0db
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG3, 0xf | HP_ANTIPOP_ENABLE, 0x0 | HP_ANTIPOP_DISABLE); //clear Hp antipop gain
#if 0
    WriteAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG3, 0x9); // 1 0 1 1  clr hp vgnd pwd
    DelayMs(5);
    WriteAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG3, 0xb); // 1 0 0 1 clr hp buf pwd
    DelayMs(5);
#endif
    WriteAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG3, 0xf); // 0 0 0 0 clr hp short out pwd
}

/*******************************************************************************
** Name: ACodec_lineout_init
** Input:
** Return: void
*******************************************************************************/
_ATTR_DRIVER_CODE_
void ACodec_lineout_init(void)
{
    //DAC to LineOut
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACPOPD, ATPCE_DISABLE | SMTPO_ON | HPSEL_LINE2DAC | ANTIPOP_ENABLE);

    //clear LO amplitude to 0db
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG0, (0x3  << 2) | LO_VAG_RISE_SLOW_ENABLE,
                      (0x00 << 2) | LO_VAG_RISE_SLOW_ENABLE);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG2, (0xf << 3), (0x00 << 3)); //clear LO amplitude to 0db

    //clear Lineout antipop gain
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG5, 0xf | LO_ANTIPOP_ENABLE, 0x0 | LO_ANTIPOP_DISABLE);

    //lineout power on
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG5, PWD_LO_BUF_DOWN | PWD_LO_OSTG_DOWN, PWD_LO_BUF_ON | PWD_LO_OSTG_ON);
}

/*******************************************************************************
** Name: ACodec_dac_lineout_Deinit
** Input:
** Return: void
*******************************************************************************/
_ATTR_DRIVER_CODE_
void ACodec_lineout_Deinit(void)
{
    //clear LO amplitude to 0db
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG0, (0x3  << 2) | LO_VAG_RISE_SLOW_ENABLE,
                      (0x00 << 2) | LO_VAG_RISE_SLOW_ENABLE);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG2, (0xf << 3), (0x00 << 3)); //clear LO amplitude to 0db

    //clear Lineout antipop gain
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG5, 0xf | LO_ANTIPOP_ENABLE, 0x0 | LO_ANTIPOP_DISABLE);

    //lineout power down
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG5, PWD_LO_BUF_DOWN | PWD_LO_OSTG_DOWN, PWD_LO_BUF_DOWN | PWD_LO_OSTG_DOWN);
}

void ACodec_dac_init(void)
{
    int    i = 0;
    uint32 temp, temp1;
    uint32 config = 0;

    //enbale dac
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACPOPD, ATPCE_DISABLE | SMTPO_ON);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_DIGEN, DAC_EN_ENABLE, DAC_EN_ENABLE);

    //DAC config
    config |= DACBYPS_DISABLE; //DACBYPS DISABLE
    config |= DACFADE_AS_DACCZDT; //DACFADE = 1
    config |= DACCZDT_1; //volume adjusts only when audio waveform crosses zero or volume-control time-limit condition meets
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_VCTL, DACBYPS_ENABLE | DACFADE_AS_DACCZDT | DACCZDT_1, config);
    WriteAcodecReg(ACODEC_BASE + ACODEC_VCTIME, 2);

    //DAC mute
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACMUTE, DACMTE_DISABLE); //DAC mute mast Disable to set vol
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACVCTLL, 0xff);
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACVCTLR, 0xff);

    for (i = 0; i < 2550; i++)  //wait for vol --> 0xff
    {
        temp  = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTL);
        temp1 = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTR);

        //DEBUG("temp = %d, temp1 =%d", temp,temp1);
        if ((0xff == temp) && (0xff == temp1))
        {
            break;
        }

        DelayMs(1);
    }

    WriteAcodecReg(ACODEC_BASE + ACODEC_DACMUTE, DACMTE_ENABLE);

    for (i = 0; i < 2550; i++) //wair for mute
    {
        temp  = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTL);
        temp1 = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTR);

        //DEBUG("temp = %d, temp1 =%d", temp,temp1);
        if ((255 == temp) && (temp1 == 255))
        {
            break;
        }

        DelayMs(1);
    }

    //DAC power on
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACACFG0, 0x0);
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACACFG1, 0x0);

    WriteAcodecReg(ACODEC_BASE + ACODEC_DACACFG2, PWD_DACBIAS_ON | PWD_DACL_ON | PWD_DACR_ON); // power down dac ibias/l/r

    for (i = 0; i < 100; i++)
    {
        DelayMs(1);
        temp = ReadAcodecReg(ACODEC_BASE + ACODEC_DACST);

        if (0 == (temp & 0x01))
        {
            break;
        }
    }
}
void ACodec_dac_Deinit(void)
{
    uint32 i = 0;
    uint32 antipop_bit_tmp = 0;
    uint32 temp, temp1;

    //DAC mute
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACMUTE, DACMTE_DISABLE); //DAC mute mast Disable to set vol
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACVCTLL, 0xff);
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACVCTLR, 0xff);

    for (i = 0; i < 2550; i++)  //wait for vol --> 0xff
    {
        temp  = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTL);
        temp1 = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTR);

        if ((0xff == temp) && (0xff == temp1))
        {
            break;
        }

        DelayMs(1);
    }

    WriteAcodecReg(ACODEC_BASE + ACODEC_DACMUTE, DACMTE_ENABLE);

    for (i = 0; i < 2550; i++) //wair for mute
    {
        temp  = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTL);
        temp1 = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTR);

        if ((255 == temp) && (temp1 == 255))
        {
            break;
        }

        DelayMs(1);
    }

    //Disable dac
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_DIGEN, DAC_EN_ENABLE, DAC_EN_ENABLE);

    //DAC powerDown
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACACFG2, PWD_DACBIAS_DOWN | PWD_DACL_DOWN | PWD_DACR_DOWN); // power down dac ibias/l/r

    for (i = 0; i < 100; i++)
    {
        DelayMs(1);
        temp = ReadAcodecReg(ACODEC_BASE + ACODEC_DACST);

        if (1 == (temp & 0x01))
        {
            break;
        }
    }

}

void ACodec_adc_init(CodecIn_sel_t InMode)
{
    uint32 config = 0;
    uint32 ADCSRT_NUM;
    uint32 timeout = 200000;

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_DIGEN, ADC_EN_ENABLE , ADC_EN_ENABLE);

    config |= ADCBYPS_DISABLE; //ADCBYPS DISABLE
    config |= ADCFADE_AS_ADCCZDT; //ADCFADE = 1
    config |= ADCCZDT_1; //volume adjusts only when audio waveform crosses zero or volume-control time-limit condition meets
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_VCTL, ADCBYPS_ENABLE | ADCFADE_AS_ADCCZDT | ADCCZDT_1, config);

    //MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG0,HALF_ADC_BUF_ENABLE|HALF_VAG_BUF_ENABLE,HALF_ADC_BUF_DISABLE|HALF_VAG_BUF_DISABLE);
    //MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG1,BOOST_ADC_BUF_ENABLE|BOOST_VAG_BUF_ENABLE|REF_ADC_SEL_15V,
    //                                            BOOST_ADC_BUF_ENABLE|BOOST_VAG_BUF_ENABLE|REF_ADC_SEL_15V);

    if ((InMode == Codecin_Sel_LINE1) || (InMode == Codecin_Sel_LINE2))
    {
        WriteAcodecReg(ACODEC_BASE + ACODEC_ALC0,   0x00);
        WriteAcodecReg(ACODEC_BASE + ACODEC_ALC1,   0x00);
        WriteAcodecReg(ACODEC_BASE + ACODEC_ALC2,   0x00);
        WriteAcodecReg(ACODEC_BASE + ACODEC_ADCNG,  0x00);
        WriteAcodecReg(ACODEC_BASE + ACODEC_ADCNGST, 0x00);
        WriteAcodecReg(ACODEC_BASE + ACODEC_ADCHPF, 0x00);
    }
    else //MIC
    {
        //set ALC & HFP
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_ALC0, ALCL_ENABLE  | ALCR_ENABLE,
                          ALCL_DISABLE | ALCR_DISABLE);
        WriteAcodecReg(ACODEC_BASE + ACODEC_ALC1,   0x00);
        WriteAcodecReg(ACODEC_BASE + ACODEC_ALC2,   0x00);
        WriteAcodecReg(ACODEC_BASE + ACODEC_ADCNG,  0x00);
        WriteAcodecReg(ACODEC_BASE + ACODEC_ADCNGST, 0x00);
        //WriteAcodecReg(ACODEC_BASE+ACODEC_ADCHPF, 0x00);
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCHPF, 0xf0, 0xd0); // HFP enable, 60Hz filter
    }

    //MaskAcodecRegBits(ACODEC_BASE+ACODEC_ADCCFG0,(0x07<<0)|ADC_DITH_OFF_ENABLE,
    //                                             (0x00<<0)|ADC_DITH_OFF_ENABLE);

    //MaskAcodecRegBits(ACODEC_BASE+ACODEC_ADCCFG1,ADC_ATTN_ALLIBIAS_ENABLE|ADC_ATTN_OPBIAS_ENABLE|ADC_DLY_INC_ENABLE|ADC_OVERLAP_INC_ENABLE|ADC_BOOST_OPAMP_ENABLE|ADC_BOOST_VAGOP_ENABLE,
    //                                             ADC_ATTN_ALLIBIAS_DISABLE|ADC_ATTN_OPBIAS_DISABLE|ADC_DLY_INC_DISABLE|ADC_OVERLAP_INC_DISABLE|ADC_BOOST_OPAMP_DISABLE|ADC_BOOST_VAGOP_DISABLE);

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCNG, NGCHL_BOTH_CH | NGEN_ENABLE | NGBOOST_BOOST,
                      NGCHL_BOTH_CH | NGEN_ENABLE | NGBOOST_BOOST);

    if (InMode == Codecin_Sel_MIC1_MONO)
    {
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCCFG1, ADC_MUTE_L_ENABLE,
                          ADC_MUTE_L_DISABLE);

        MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCCFG0, ADC_L_PWD_DOWN,
                          ADC_L_PWD_ON  );
    }
    else if (InMode == Codecin_Sel_MIC2_MONO)
    {
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCCFG1, ADC_MUTE_R_ENABLE,
                          ADC_MUTE_R_DISABLE);

        MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCCFG0, ADC_R_PWD_DOWN,
                          ADC_R_PWD_ON);
    }
    else
    {
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCCFG1, ADC_MUTE_L_ENABLE  | ADC_MUTE_R_ENABLE,
                          ADC_MUTE_L_DISABLE | ADC_MUTE_R_DISABLE);

        MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCCFG0, ADC_L_PWD_DOWN | ADC_R_PWD_DOWN,
                          ADC_L_PWD_ON   | ADC_R_PWD_ON);
    }
}

void ACodec_adc_DeInit(void)
{
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCCFG0, ADC_MUTE_L_ENABLE | ADC_MUTE_R_ENABLE,
                      ADC_MUTE_L_ENABLE | ADC_MUTE_R_ENABLE);

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_DIGEN, ADC_EN_ENABLE , ADC_EN_DISABLE );

    //clear register
    WriteAcodecReg(ACODEC_BASE + ACODEC_ALC0,   0x00);
    WriteAcodecReg(ACODEC_BASE + ACODEC_ALC1,   0x00);
    WriteAcodecReg(ACODEC_BASE + ACODEC_ALC2,   0x00);
    WriteAcodecReg(ACODEC_BASE + ACODEC_ADCNG,  0x00);
    WriteAcodecReg(ACODEC_BASE + ACODEC_ADCNGST, 0x00);
    WriteAcodecReg(ACODEC_BASE + ACODEC_ADCHPF, 0x00);

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_ADCCFG0, ADC_R_PWD_DOWN | ADC_R_PWD_DOWN,
                      ADC_R_PWD_DOWN | ADC_R_PWD_DOWN);
}


void ACodec_linein_init(CodecIn_sel_t InMode)
{
    if (InMode == Codecin_Sel_LINE1)
    {
        //linein1 to adc
        WriteAcodecReg(ACODEC_BASE + ACODEC_LICFG1, MUX_L_IN_SEL_LINE_1 | MUX_R_IN_SEL_LINE_1);

    }
    else //if (InMode == Codecin_Sel_LINE2)
    {
        //linein2 to adc
        WriteAcodecReg(ACODEC_BASE + ACODEC_LICFG1, MUX_L_IN_SEL_LINE_2 | MUX_R_IN_SEL_LINE_2);
    }

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG3, MUX_CHOP_EN_ENABLE, MUX_CHOP_EN_DISABLE);

    //linein mux power on
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG4, MUX_L_PD_DOWN | MUX_R_PD_DOWN, MUX_L_PD_ON | MUX_R_PD_ON);
}

void ACodec_Linein_Deinit(void)
{
    //linein mux power on
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG4, MUX_L_PD_DOWN | MUX_R_PD_DOWN, MUX_L_PD_DOWN | MUX_R_PD_DOWN);

    //clear register
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG3, MUX_CHOP_EN_ENABLE, MUX_CHOP_EN_DISABLE);
}

void ACodec_Mic_init(CodecIn_sel_t InMode, CodecMic_Mode_t MIC_Mode, CodecMicBias_sel_t Mic_Bias)
{
    uint32 config, Mic_Bias_old;
    uint32 MicModeL, MicModeR;

    if (MIC_Mode == CodecMIC_Normal_Mode)
    {
        MicModeL = MIC_L_DIFF_EN_DISABLE;
        MicModeR = MIC_R_DIFF_EN_DISABLE;
    }
    else
    {
        MicModeL = MIC_L_DIFF_EN_ENABLE;
        MicModeR = MIC_R_DIFF_EN_ENABLE;
    }

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG0, MICBIAS_SEL_MASK, Mic_Bias);
    DelayMs(50);

    if (InMode & Codecin_Sel_MIC1_MONO)
    {
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG1, (0x03 << 2) | MIC_L_DIFF_EN_ENABLE, MUX_L_IN_SEL_MIC | MicModeL);
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG4, MIC_L_PD_DOWN  | MUX_L_PD_DOWN, MIC_L_PD_ON  | MUX_L_PD_ON);
    }

    if (InMode & Codecin_Sel_MIC2_MONO)
    {
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG1, (0x03 << 0) | MIC_R_DIFF_EN_ENABLE, MUX_R_IN_SEL_MIC | MicModeR);
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG4, MIC_R_PD_DOWN | MUX_R_PD_DOWN, MIC_R_PD_ON | MUX_R_PD_ON);
    }
}

void ACodec_Mic_Deinit(void)
{
    WriteAcodecReg(ACODEC_BASE + ACODEC_LICFG4, MIC_R_PD_DOWN | MIC_L_PD_DOWN | MUX_R_PD_DOWN  | MUX_L_PD_DOWN);
}

/*******************************************************************************
** Name: ACodec_Set_ADC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Set_ADC_Mode(CodecIn_sel_t In_Mode)
{
    uint32 config = 0;
    uint32 ADCSRT_NUM;
    uint32 timeout = 200000;

    ACodec_adc_init(In_Mode);

    if ((In_Mode == Codecin_Sel_LINE1) || (In_Mode == Codecin_Sel_LINE2))
    {
        ACodec_linein_init(In_Mode);
    }
    else //if ((In_Mode & Codecin_Sel_MIC1_MONO) || (In_Mode & Codecin_Sel_MIC2_MONO))
    {
        ACodec_Mic_init(In_Mode, CodecMIC_Diff_Mode, MicBias_sel_30V);
    }
}


/*******************************************************************************
** Name: ACodec_Exit_ADC_Mode
** Input:
** Return: void
*******************************************************************************/
void ACodec_Exit_ADC_Mode(void)
{
    //if ((In_Mode == Codecin_Sel_LINE1) || (In_Mode == Codecin_Sel_LINE2))
    {
        ACodec_Linein_Deinit();
    }
    //else //if ((In_Mode & Codecin_Sel_MIC1_MONO) || (In_Mode & Codecin_Sel_MIC2_MONO))
    {
        ACodec_Mic_Deinit();
    }

    ACodec_adc_DeInit();
}

/*******************************************************************************
** Name: ACodec_Set_DAC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Set_DAC_Mode(CodecOut_sel_t OUT_Mode)
{
    ACodec_dac_init();

    if (OUT_Mode & CodecOut_Sel_HP)
    {
        ACodec_hp_init();
    }
    else //if (OUT_Mode & CodecOut_Sel_LINE)
    {
        ACodec_lineout_init();
    }
}

/*******************************************************************************
** Name: ACodec_Exit_DAC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Exit_DAC_Mode()
{
    //if (Acodec_output_sel == CodecOut_Sel_HP)
    {
        ACodec_hp_DeInit();
    }
    //else //if (Acodec_output_sel == CodecOut_Sel_LINE)
    {
        ACodec_lineout_Deinit();
    }

    ACodec_dac_Deinit();
}

/*******************************************************************************
** Name: ACodec_ADC2DAC_MIX
** Input:CodecMIX_Mode_t MIX_en 0-ENABLE;1-DISABLE
** Return: void
*******************************************************************************/
void ACodec_ADC2DAC_MIX(CodecMIX_Mode_t MIX_en)
{
    if (MIX_en == CodecMIX_ENABLE)
    {
        WriteAcodecReg(ACODEC_BASE + ACODEC_MIXCTRL, MIXE_ENABLE); //MIX
    }
    else
    {
        WriteAcodecReg(ACODEC_BASE + ACODEC_MIXCTRL, MIXE_DISABLE); //NO MIX
    }
}

/*******************************************************************************
** Name: ACodec_Set_DAC_DigVol
** Input:vol 0.375db/step: (0-0xff)0dB - -95dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_DAC_DigVol(uint8 vol)
{
    uint32 i, temp, temp1;

    WriteAcodecReg(ACODEC_BASE + ACODEC_DACMUTE, DACMTE_DISABLE);

    WriteAcodecReg(ACODEC_BASE + ACODEC_DACVCTLL, vol); //DAC left channel volume
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACVCTLR, vol); //DAC right channel volume

    //DEBUG("xxxxxxxxxxxxxxxxxxxxx");
    for (i = 0; i < 2550; i++)
    {
        temp  = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTL);
        temp1 = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTR);

        if ((vol == temp) && (vol == temp1))
        {
            break;
        }

        DelayMs(1);
    }

    //DEBUG("YYYYYYYYYYYYYYYYYYYYYYY i = %d",i);
    return vol;
}

/*******************************************************************************
** Name: ACodec_Set_ADC_DigVol
** Input:vol 0.375db/step: (0-0xff)0dB - -95dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_ADC_DigVol(uint8 vol)
{
    uint32 config;
    WriteAcodecReg(ACODEC_BASE + ACODEC_ADCVCTLL, vol); //ADC left channel volume
    WriteAcodecReg(ACODEC_BASE + ACODEC_ADCVCTLR, vol); //ADC right channel volume
    return vol;

}

/*******************************************************************************
** Name: ACodec_Set_ADCMUX_Vol
** Input:   0-15  -18dB to 27dB, 3db/step.
** Return:
*******************************************************************************/
uint8 ACodec_Set_ADCMUX_Vol(uint8 vol)
{
    uint32 vol_l, vol_r;

    if (vol > 15)
        return 0;

    vol_r = vol << 0;
    vol_l = vol << 4;
    //DEBUG("ACodec_Set_ADCMUX_Vol = %d",vol);

    WriteAcodecReg(ACODEC_BASE + ACODEC_LICFG2, vol_r | vol_l);
    return vol;

}

/*******************************************************************************
** Name: ACodec_Set_MIC_AnaVol
** Input:   0-3
         2'b00: 0dB
         2'b01: 10dB
         2'b10: 20dB
         2'b11: 30dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_MIC_AnaVol(uint8 vol)
{
    uint8 vol_l, vol_r;
    uint32 config;

    if (vol > 3)
        return 0;

    vol_r = vol << 0;
    vol_l = vol << 2;

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_LICFG0, 0xF, vol_r | vol_l);
    return vol;
}


/*******************************************************************************
** Name: ACodec_Set_HP_Gain
** Input:vol 1db/step: (0-0xff)0dB - -15dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_HP_Gain(uint8 vol)
{
    uint32 i, antipop_bit_tmp;

    WriteAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG4, vol | HP_ANTIPOP_ENABLE);
    return vol;
}

/*******************************************************************************
** Name: ACodec_Set_LO_Gain
** Input:vol 1db/step: (0-0xff)0dB - -15dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_LO_Gain(uint8 vol)
{
    WriteAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG5, vol | LO_ANTIPOP_ENABLE);

    return vol;
}

/*******************************************************************************
** Name: ACodec_Set_HP_AMP
** Input:vol
** Return: void
*******************************************************************************/
void ACodec_Set_HP_AMP(eACodecHp_AMP vol)
{
    uint32 config;

    if (vol != 0)
    {
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG4, HP_ANTIPOP_ENABLE, HP_ANTIPOP_DISABLE);
    }

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG1, (0x3 << 3), vol << 3);
}

/*******************************************************************************
** Name: ACodec_Set_LO_AMP
** Input:vol
** Return: void
*******************************************************************************/
void ACodec_Set_LO_AMP(eACodecLo_AMP vol)
{
    uint32 config;

    if (vol != 0)
    {
        MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG4, LO_ANTIPOP_DISABLE, LO_ANTIPOP_ENABLE);
    }

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_HPLOCFG0, (0x3 << 2), vol << 2);
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_PowerOnInitial(void)
  Author        : yangwenjie
  Description   : Codec power on initial

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
//_ATTR_SYS_INIT_CODE_
void Codec_PowerOnInitial(void)
{
    ScuClockGateCtr(CLK_ACODEC_GATE, 1);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 1);     //PCLK ACODEC gating open
    ScuSoftResetCtr(ACODEC_SRST0, 0);

    DEBUG("RK-ACODEC Codec_PowerOnInitial");

    Acodec_I2S_MODE    = ACodec_I2S_MASTER_MODE;
    pll_old_target     = Pll_Target_Freq_NULL;

    HP_DET_Status      = 0xff;
    HP_DET_Status_check = 0;
    HP_DET_TICKCount    = 0;

    Acodec_output_sel  = CodecOut_Sel_HP;

    ACodec_Soft_Reset();

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG5, PLL_PW_DOWN, PLL_PW_UP);
    DelayMs(5);

    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG1, 0x7);
    DelayMs(5);

    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_DOWN | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    DelayMs(200);
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    DelayMs(200);
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
    DelayMs(500);
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_ON);

    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG1, 0x1);
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DeIitial(void)
  Author        : yangwenjie
  Description   : close Codec

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_DeInitial(void)
{
    Codec_Suspend();
}

_ATTR_DRIVER_CODE_
void Codec_Suspend(void)
{
    //ACODEC I2S DeInit
    ACodec_I2S_Deinit();

    // ADC & linein & MIC DeInit
    ACodec_Linein_Deinit();
    ACodec_Mic_Deinit();
    ACodec_adc_DeInit();

    //DAC & LineOut & HPout DeInit
    ACodec_hp_DeInit();
    ACodec_lineout_Deinit();
    ACodec_dac_Deinit();

    //ACODEC Power Down
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
    DelayMs(50);
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    DelayMs(50);
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_DOWN | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    DelayMs(50);

    //ACodec clock Power Down
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG0, PLL_OUTDIV_ENABLE, PLL_OUTDIV_DISABLE);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG5, APLL_RESET, APLL_RESET);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG5, PLL_PW_DOWN, PLL_PW_DOWN);

    //ScuSoftResetCtr(ACODEC_SRST0, 1);
    ScuClockGateCtr(CLK_ACODEC_GATE, 0);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 0);     //PCLK ACODEC gating open
}

_ATTR_DRIVER_CODE_
void Codec_Resume(void)
{
    ScuClockGateCtr(CLK_ACODEC_GATE, 1);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 1);     //PCLK ACODEC gating open

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG5, PLL_PW_DOWN, PLL_PW_UP);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG5, APLL_RESET, APLL_RELEASE_RESET);
    MaskAcodecRegBits(ACODEC_BASE + ACODEC_PLLCFG0, PLL_OUTDIV_ENABLE, PLL_OUTDIV_ENABLE);

    MaskAcodecRegBits(ACODEC_BASE + ACODEC_RTCFG0, (0x3f << 0), 0x01);
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG1, 0x7);
    DelayMs(5);
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    DelayMs(200);
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
    DelayMs(500);
    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG2, REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_ON);

    WriteAcodecReg(ACODEC_BASE + ACODEC_RTCFG1, 0x1);
    WriteAcodecReg(ACODEC_BASE + ACODEC_DACPOPD, ATPCE_DISABLE | SMTPO_DOWN | ANTIPOP_DISABLE);

    Codec_SetMode(Codecmode_Bak, CodecDataWidth_Bak);
    Codec_SetSampleRate(CodecFS_Bak);
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   :

  Input         : Codecmode��

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetMode(CodecMode_en_t Codecmode, uint32 datawidth)
{
    Codecmode_Bak      = Codecmode;
    CodecDataWidth_Bak = datawidth;

    //set Codec Mode
    switch (Codecmode)
    {
        case Codec_DACoutHP:
        case Codec_DACoutLINE:
            ACodec_I2S_Init(TFS_TX_I2S_MODE, datawidth, IBM_TX_BUS_MODE_NORMAL, I2S_MST_MASTER, ACodec_I2S_RX);
            ACodec_Set_DAC_Mode(Acodec_output_sel);
            break;

        case Codec_Line1ADC:
            ACodec_I2S_Init(TFS_TX_I2S_MODE, datawidth, IBM_TX_BUS_MODE_NORMAL, I2S_MST_MASTER, ACodec_I2S_TXRX);
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE1);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            ACodec_Set_DAC_Mode(Acodec_output_sel);
            break;

        case Codec_Line1in:
            ACodec_I2S_Init(TFS_TX_I2S_MODE, datawidth, IBM_TX_BUS_MODE_NORMAL, I2S_MST_MASTER, ACodec_I2S_TXRX);
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE1);
            ACodec_ADC2DAC_MIX(CodecMIX_ENABLE);
            ACodec_Set_DAC_Mode(Acodec_output_sel);
            break;

        case Codec_Line2ADC:
            ACodec_I2S_Init(TFS_TX_I2S_MODE, datawidth, IBM_TX_BUS_MODE_NORMAL, I2S_MST_MASTER, ACodec_I2S_TXRX);
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE2);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            ACodec_Set_DAC_Mode(Acodec_output_sel);
            break;

        case Codec_Line2in:
            ACodec_I2S_Init(TFS_TX_I2S_MODE, datawidth, IBM_TX_BUS_MODE_NORMAL, I2S_MST_MASTER, ACodec_I2S_TXRX);
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE2);
            ACodec_ADC2DAC_MIX(CodecMIX_ENABLE);
            ACodec_Set_DAC_Mode(Acodec_output_sel);
            break;

        case Codec_MicStero:
            ACodec_I2S_Init(TFS_TX_I2S_MODE, datawidth, IBM_TX_BUS_MODE_NORMAL, I2S_MST_MASTER, ACodec_I2S_TX);
            ACodec_Set_ADC_Mode(Codecin_Sel_MIC_STERO);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;

        case Codec_Mic1Mono:
            ACodec_I2S_Init(TFS_TX_I2S_MODE, datawidth, IBM_TX_BUS_MODE_NORMAL, I2S_MST_MASTER, ACodec_I2S_TX);
            ACodec_Set_ADC_Mode(Codecin_Sel_MIC1_MONO);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;

        case Codec_Mic2Mono:
            ACodec_I2S_Init(TFS_TX_I2S_MODE, datawidth, IBM_TX_BUS_MODE_NORMAL, I2S_MST_MASTER, ACodec_I2S_TX);
            ACodec_Set_ADC_Mode(Codecin_Sel_MIC2_MONO);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;

        default:
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_ExitMode(CodecMode_en_t Codecmode)
{
    switch (Codecmode)
    {
        case Codec_DACoutHP:
        case Codec_DACoutLINE:
            //ACodec_I2S_Deinit();
            ACodec_Exit_DAC_Mode();

            ACodec_I2S_Deinit();
            break;

        case Codec_Line1ADC:
            //ACodec_I2S_Deinit();
            ACodec_Exit_DAC_Mode();
            ACodec_Exit_ADC_Mode();

            ACodec_I2S_Deinit();
            break;

        case Codec_Line1in:
            //ACodec_I2S_Deinit();
            ACodec_Exit_DAC_Mode();
            ACodec_Exit_ADC_Mode();

            ACodec_I2S_Deinit();
            break;

        case Codec_Line2ADC:
            //ACodec_I2S_Deinit();
            ACodec_Exit_DAC_Mode();
            ACodec_Exit_ADC_Mode();

            ACodec_I2S_Deinit();
            break;

        case Codec_MicStero:
        case Codec_Mic1Mono:
        case Codec_Mic2Mono:
            //ACodec_I2S_Deinit();
            ACodec_Exit_ADC_Mode();

            ACodec_I2S_Deinit();
            break;

        default:
            break;
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetSampleRate(CodecFS_en_t CodecFS)
  Author        : yangwenjie
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetSampleRate(CodecFS_en_t CodecFS)
{
    CodecFS_Bak = CodecFS;

    switch (CodecFS)
    {
        case FS_8000Hz:
        case FS_16KHz:
        case FS_32KHz:
        case FS_64KHz:
        case FS_128KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz, Pll_Target_Freq_40960);
            break;

        case FS_12KHz:
        case FS_24KHz:
        case FS_48KHz:
        case FS_96KHz:
        case FS_192KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz, Pll_Target_Freq_61440);
            break;

        case FS_11025Hz:
        case FS_22050Hz:
        case FS_44100Hz:
        case FS_88200Hz:
        case FS_1764KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz, Pll_Target_Freq_56448);
            break;

        default:
            break;

    }

    ACodec_I2s_SetSampleRate(CodecFS);
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetVolumet(unsigned int Volume)
  Author        : yangwenjie
  Description   : codec control volume

  Input         : Volume

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      volume = 0 mean mute,
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetVolumet(unsigned int Volume)
{
    uint32 VolumeMode = EQ_NOR;

    Volume_Bak = Volume;

    if (TRUE == ThreadCheck(pMainThread, &FMThread)) // FM / LineIn (adc mode or line in mode)
    {
        if (Volume == 0)
        {
            ACodec_Set_ADC_DigVol(FmCodecConfig_General[Volume].ADCDigVol);
            ACodec_Set_ADCMUX_Vol(FmCodecConfig_General[Volume].ADCMuxVol);
            ACodec_Set_HP_AMP(FmCodecConfig_General[Volume].HP_AMPVol);
            ACodec_Set_DAC_DigVol(FmCodecConfig_General[Volume].Dac_DigVol);
            ACodec_Set_HP_Gain(FmCodecConfig_General[Volume].HP_ANTIPOPVol);
        }
        else
        {
            ACodec_Set_ADC_DigVol(FmCodecConfig_General[Volume].ADCDigVol);
            ACodec_Set_ADCMUX_Vol(FmCodecConfig_General[Volume].ADCMuxVol);
            ACodec_Set_HP_AMP(FmCodecConfig_General[Volume].HP_AMPVol);
            ACodec_Set_DAC_DigVol(FmCodecConfig_General[Volume].Dac_DigVol);
            ACodec_Set_HP_Gain(FmCodecConfig_General[Volume].HP_ANTIPOPVol);
        }
    }
    else   // Music / Video (hp mode / line out mode)
    {
        if (Volume == 0)
        {
            if (Acodec_output_sel == CodecOut_Sel_LINE)
            {
                ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].Dac_DigVol);
                ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].HP_AMPVol);
            }
            else //if (Acodec_output_sel == CodecOut_Sel_HP)
            {
                ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].Dac_DigVol);
                ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].HP_AMPVol);
            }

            Codec_DACMute();
        }
        else
        {
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                AudioInOut_Type  *pAudio  = &AudioIOBuf;
                RKEffect         *pEffect = &pAudio->EffectCtl;
                VolumeMode = pEffect->Mode;
            }

            Codec_DACUnMute();

            switch (VolumeMode)
            {
                case EQ_NOR:
                {
                    if (Acodec_output_sel == CodecOut_Sel_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].Dac_DigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].HP_AMPVol);
                    }
                    else //if (Acodec_output_sel == CodecOut_Sel_HP)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].Dac_DigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].HP_AMPVol);
                    }
                }
                break;

                case EQ_POP:
                case EQ_HEAVY:
                {
                    if (Acodec_output_sel == CodecOut_Sel_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_POP_HEAVY_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_POP_HEAVY_HP_AMPVol);
                    }
                    else //if (Acodec_output_sel == CodecOut_Sel_HP)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].EQ_POP_HEAVY_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].EQ_POP_HEAVY_HP_AMPVol);
                    }
                }
                break;

                case EQ_JAZZ:
                case EQ_UNIQUE:
                {
                    if (Acodec_output_sel == CodecOut_Sel_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_JAZZ_UNIQUE_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_JAZZ_UNIQUE_HP_AMPVol);
                    }
                    else //if (Acodec_output_sel == CodecOut_Sel_HP)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].EQ_JAZZ_UNIQUE_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].EQ_JAZZ_UNIQUE_HP_AMPVol);
                    }
                }
                break;

                case EQ_USER:
                {
                    if (Acodec_output_sel == CodecOut_Sel_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_USER_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_USER_HP_AMPVol);
                    }
                    else //if (Acodec_output_sel == CodecOut_Sel_HP)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].EQ_USER_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].EQ_USER_HP_AMPVol);
                    }
                }
                break;

                case EQ_BASS:
                {
                    if (Acodec_output_sel == CodecOut_Sel_LINE)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[Volume].EQ_BASS_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[Volume].EQ_BASS_HP_AMPVol);
                    }
                    else //if (Acodec_output_sel == CodecOut_Sel_HP)
                    {
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[Volume].EQ_BASS_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[Volume].HP_ANTIPOPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[Volume].EQ_BASS_HP_AMPVol);
                    }
                }

                break;

                default:
                    break;
            }
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACMute(void)
  Author        : yangwenjie
  Description   : set codec mute

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_DACMute(void)
{
    uint32 i;
    uint32 temp, temp1;

    WriteAcodecReg(ACODEC_BASE + ACODEC_DACMUTE, DACMTE_ENABLE); //DAC mute

    for (i = 0; i < 2550; i++) //wair for mute
    {
        temp  = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTL);
        temp1 = ReadAcodecReg(ACODEC_BASE + ACODEC_DACVSTR);

        if ((255 == temp) && (temp1 == 255))
        {
            break;
        }

        DelayMs(1);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   : set codec exit from mute.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_DACUnMute(void)
{
    uint32 i;
    uint32 temp, temp1;

    WriteAcodecReg(ACODEC_BASE + ACODEC_DACMUTE, DACMTE_DISABLE); //DAC Unmute
}

/*
--------------------------------------------------------------------------------
  Function name : void AudioDmaIsrHandler(void)
  Author        : zs
  Description   : the callback of DMA0 interrupt.
  Input         : null
  Return        : null
  History       : <author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void Codec_HP_Detect(void)
{
    UINT8  RetVal = GPIO_HIGH;
    uint8  HPStatus = 0;
    uint32 temp;

    //check DAC power: if DAC Power off, return
    temp = ReadAcodecReg(ACODEC_BASE + ACODEC_DACACFG2);

    if ((temp & 0x07) != (PWD_DACBIAS_ON | PWD_DACL_ON | PWD_DACR_ON))
    {
        return;
    }

    HPStatus = Gpio_GetPinLevel(GPIO_CH2, GPIOPortB_Pin3);

    if (HPStatus != HP_DET_Status)
    {
        Codec_DACMute();
        HP_DET_Status = HPStatus;
        HP_DET_Status_check = 1;
        HP_DET_TICKCount = SysTickCounter;
        return;
    }

    if (HP_DET_Status_check == 1)
    {
        if (HPStatus == 0)
        {
            //printf("~~~HP DET OK~~~\n");
            //HP
            if ((SysTickCounter - HP_DET_TICKCount) > 30)
            {
                HP_DET_Status_check = 0;

                //check HP power
                temp = ReadAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG3);

                if ((temp & 0x0f) != 0)
                {
                    ACodec_hp_init();
                    ACodec_lineout_Deinit();

                    Acodec_output_sel = CodecOut_Sel_HP;
                    Codec_SetVolumet(Volume_Bak);
                }

                Codec_DACUnMute();
            }
        }
        else
        {
            //Line out
            if ((SysTickCounter - HP_DET_TICKCount) > 30)
            {
                //printf("~~~Line out DET OK~~~\n");
                HP_DET_Status_check = 0;

                //check LO power
                temp = ReadAcodecReg(ACODEC_BASE + ACODEC_HPLOCFG5);

                if ((temp & (PWD_LO_BUF_DOWN | PWD_LO_OSTG_DOWN)) != 0)
                {
                    ACodec_hp_DeInit();
                    ACodec_lineout_init();

                    Acodec_output_sel = CodecOut_Sel_LINE;
                    Codec_SetVolumet(Volume_Bak);
                }

                Codec_DACUnMute();
            }
        }
    }

    return;
}

_ATTR_SYS_CODE_
uint8 Codec_Get_Over_Current(void)
{
    uint8 config = 0;

    if (Acodec_output_sel == CodecOut_Sel_HP)
    {
        config = ACodec_get_over_current_value();
        config = config & 0x03;

        if (config != 0)
        {
            if (Acodec_over_cur_value != config)
            {
                Acodec_over_cur_value = config;
                Acodec_over_cur_count = 0;
                ACodec_Get_Over_Current_TICKCount = SysTickCounter;
            }
            else
            {
                Acodec_over_cur_count++;

                if ((SysTickCounter - ACodec_Get_Over_Current_TICKCount) > 500)
                {
                    SendMsg(MSG_POWER_DOWN);
                    Acodec_over_cur_value = 0;
                }
            }
        }
        else
        {
            Acodec_over_cur_value = 0;
        }
    }

    return config;
}

/*
********************************************************************************
*
*                         End of Rockcodec.c
*
********************************************************************************
*/
#endif

