/******************************************************************/
/*   Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved.  */
/*******************************************************************
File :  hw_fm5800.h
Desc :  fm type / structure/variables/functions interfaces declaration

Author : huangxinyu
Date : 2007-08-21
Notes :

$Log: hw_fm5800.h,v $
Revision 1.3  2007/10/08 02:38:41  Lingzhaojun


*********************************************************************/

#ifndef _HW_QN8035_H
#define _HW_FM5807_H

#undef  EXT
#ifdef _IN_HWQN8035_
#define EXT
#else
#define EXT extern
#endif

#define RADIO_CHINA                 0           //China       
#define RADIO_EUROPE                1           //Europe           
#define RADIO_JAPAN                 2           //Japan                
#define RADIO_USA                   3           //America

#define RUNER_AREA_JA               0x0002      // 1为ja 0为ch   us 
#define RUNER_SEEK_DN               0x01        //
#define RUNER_SEEK_UP               0x03        //
#define TUNER_MONO                  0x20        /* Stereo indication, 0: stereo reception, 1: mono  5800*/
#define TUNER_MUTE                  0x40        //  mute  5800

#define USFMSTART                   870
#define USFMSTOP                    1080
#define JPFMSTART                   760
#define JPFMSTOP                    900
#define USFMDET                     (USFMSTOP-USFMSTART)
#define JPFMDET                     (JPFMSTOP-JPFMSTART)
#define QND_FSTEP_100KHZ            1

//#define RDA5807E  /*支持RDA5807E模组，注释掉默认支持RDA5807P模组*/
//#define USBAND_STARTFRE                0  //87.0
#define USBAND_STARTFRE             0   //87.5
#define USBAND_LENGTH               (210-USBAND_STARTFRE)
#define I2C_DEV0_ADDRESS            0x20
#define I2C_TIMEOUT_TIME            10
#define I2C_TIMEOUT_COUNT           20
#define MS_DELAY_CONST              40
#define QND_SINE_WAVE_CLOCK         0x00    //inject sine-wave clock  
#define QND_DIGITAL_CLOCK           0x80    //inject digital clock,default is inject digital clock
                                    
//*24MHZ                            
#define QND_XTAL_DIV0               0xDC
#define QND_XTAL_DIV1               0x02
#define QND_XTAL_DIV2               0x54

//32.768K
//#define QND_XTAL_DIV0               0x01
//#define QND_XTAL_DIV1               0x08
//#define QND_XTAL_DIV2               0x5C
                                    
#define CID1                        0x05
#define CID2                        0x06
#define QND_INPUT_CLOCK             0x00    //using external clock or oscillator as qn8035's clock 
#define SMP_HLD_THRD                0x19
#define SMSTART                     0x34
#define SNCSTART                    0x35
#define HCCSTART                    0x36
#define CHIPSUBID_QN8035A1          0x02
#define NCO_COMP_VAL                0x46
#define ICPREF                      0x0f
#define CHIPSUBID_QN8035A0          0x01
#define IMR                         0x40
#define CCA_CH_DIS                  0x01
#define QND_XTAL_DIV1_855           0x88
#define QND_XTAL_DIV2_855           0x3B
#define RXCCA_MASK                  0x03
#define RX_CCA                      0x02
#define CCA_PILOT_SNR_FILTER        20// 25 
#define CCA_PILOT_READ_DELAY        60
#define CCA_PILOT_READ_COUNT        10
#define CCA_PILOT_READ_COUNT_DELAY  2
#define CCA_PILOT_TH                1 //  3
                                    
//enum {
    // Bit[15-8] of the word: RSSI Threshold
    // Bit[7-0] of the word: SNR Threshold
    // e.g. 0x1E06 => RSSI_TH = 0x1E, SNR_TH = 0x06
    // notice: the rang of RSSI is 0x0A ~ 0x3F
    // notice: the rang of SNR is 0x00 ~ 0x3F
   
#define CCA_SENSITIVITY_LEVEL_0     0x1406 
#define CCA_SENSITIVITY_LEVEL_1     0x1407    //if using the pilot as CCA,reference this item.
#define CCA_SENSITIVITY_LEVEL_2     0x1408 
#define CCA_SENSITIVITY_LEVEL_3     0x1409 
#define CCA_SENSITIVITY_LEVEL_4     0x140a   //if not using the pilot as CCA,reference this item.
#define CCA_SENSITIVITY_LEVEL_5     0x140b        
#define CCA_SENSITIVITY_LEVEL_6     0x140c 
#define CCA_SENSITIVITY_LEVEL_7     0x140d 
#define CCA_SENSITIVITY_LEVEL_8     0x140e 
#define CCA_SENSITIVITY_LEVEL_9     0x140f
//};

#define GAIN_SEL                    0x1b
#define SYSTEM_CTL1                 0x1c
#define SYSTEM_CTL2                 0x1d
#define CCA_CNT1                    0x37

#define XTAL_DIV0                   0x15
#define XTAL_DIV1                   0x16
#define XTAL_DIV2                   0x17
#define SMSTART_VAL                 10
#define HCCSTART_VAL                18//ping 20
#define SNCSTART_VAL                51
#define CCA                         0x01
#define FREQ2CHREG(freq)            ((freq-6000)/5)
#define CHREG2FREQ(ch)              (ch*5+6000)
#define CH                          0x07
#define CH_START                    0x08
#define CH_STOP                     0x09
#define CH_STEP                     0x0A
#define CH_CH                       0x03
#define CH_CH_START                 0x0c
#define CH_CH_STOP                  0x30
#define REG_REF                     0x49
#define PILOT_CCA                   0 
#define SYSTEM1                     0x00
#define CHSC                        0x02
#define STATUS1                     0x04
#define RXCCA_FAIL                  0x08
#define SNR                         0x02
#define ST_MO_RX                    0x01
#define CCA_SNR_TH_1                0x39
#define CCA_SNR_TH_2                0x3A
#define CCA1                        0x27
#define CCA_CNT2                    0x38
#define PLT1                        0X2F
#define R_TXRX_MASK                 0x30
#define USING_INDUCTOR              0//1//0
#define STNBY                       0x20
#define REG_DAC                     0x4C

//////////////////////////////////////////////////////////////////////////////////////////
extern void Qn8035_Tuner_SetInitArea(UINT8 area);
extern void Qn8035_Tuner_SetFrequency(UINT32 n10KHz, UINT8 HILO, BOOL ForceMono,UINT16 Area);              
extern void Qn8035_Tuner_SetStereo(BOOL bStereo);
extern void Qn8035_Tuner_PowerOffDeinit(void);
extern UINT16 Qn8035_Tuner_SearchByHand(UINT16 direct, UINT32 *FmFreq);
extern void Qn8035_Tuner_PowerDown(void);
extern void Qn8035_Tuner_MuteControl(bool active);               
extern BOOLEAN Qn8035_GetStereoStatus(void);
extern void Qn8035_Tuner_Vol_Set(UINT8 gain);

_ATTR_FMDRIVER_QN8035_BSS_ UINT8   FMarea;  //ping add
_ATTR_FMDRIVER_QN8035_BSS_ UINT8  qnd_PreNoiseFloor;
_ATTR_FMDRIVER_QN8035_BSS_ UINT8 qnd_NoiseFloor;
_ATTR_FMDRIVER_QN8035_BSS_ UINT8 qnd_ChipID;
_ATTR_FMDRIVER_QN8035_BSS_ UINT8 qnd_IsQN8035B;
_ATTR_FMDRIVER_QN8035_BSS_ UINT8 qnd_div1;
_ATTR_FMDRIVER_QN8035_BSS_ UINT8 qnd_div2;
_ATTR_FMDRIVER_QN8035_BSS_ UINT8 qnd_nco;

#endif  /* _HW_FM_H */

