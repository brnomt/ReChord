/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  Adc.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _ADC_H_
#define _ADC_H_

#undef  EXT
#ifdef  _IN_ADC_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*  
*                           Macro define
*  
*-------------------------------------------------------------------------------
*/
//define adc sample channel
#define ADC_CHANEL_BATTERY              0       //ADC channel 4
#define ADC_CHANEL_KEY                  1       //ADC channel 2
#define ADC_MIC                         0       //ADC channel 0
#define ADC_CHANEL_FM0                  0       //ADC channel 0
#define ADC_CHANEL_FM1                  1       //ADC channel 1
#define ADC_CHANEL_TEMP                 1       //ADC channel 1
#define ADC_CHANEL_3                    3
#define ADC_CHANEL_5                    5
#define ADC_CHANEL_6                    6
#define ADC_CHANEL_7                    7

#define ADC_CHANEL_MAX                  8

#define ADC_CH0_FINISH                  ((uint32)1 << 0)
#define ADC_CH1_FINISH                  ((uint32)1 << 1)
#define ADC_CH2_FINISH                  ((uint32)1 << 2)
#define ADC_CH3_FINISH                  ((uint32)1 << 3)
#define ADC_CH4_FINISH                  ((uint32)1 << 4)
#define ADC_CH5_FINISH                  ((uint32)1 << 5)
#define ADC_CH6_FINISH                  ((uint32)1 << 6)
#define ADC_CH7_FINISH                  ((uint32)1 << 7)

#define ADC_CH0_PEND                    ((uint32)1 << 8)    //channel suspend flag
#define ADC_CH1_PEND                    ((uint32)1 << 9)    //channel suspend flag
#define ADC_CH2_PEND                    ((uint32)1 << 10)    //channel suspend flag
#define ADC_CH3_PEND                    ((uint32)1 << 11)    //channel suspend flag
#define ADC_CH4_PEND                    ((uint32)1 << 12)    //channel suspend flag
#define ADC_CH2_PEND                    ((uint32)1 << 13)    //channel suspend flag
#define ADC_CH3_PEND                    ((uint32)1 << 14)    //channel suspend flag
#define ADC_CH4_PEND                    ((uint32)1 << 15)    //channel suspend flag

#define ADC_FINISH                      ((uint32)1 << 16)    //sampling finish flag

#define ADC_PENDING_SIZE                32

/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/
EXT UINT32          AdcFlag;
EXT UINT16          AdcData[ADC_CHANEL_MAX];
EXT UINT8           AdcSamplingCounter;
EXT UINT8           AdcPendCounter;   //ad suspend counter,if suspend too much,that maybe is ad crash, it need to clear supend flag and reset,
EXT UINT8           AdcSamplingCh;    //current sampling channel.
#if 1
EXT UINT8           *pAdcPendIn;
EXT UINT8           *pAdcPendout;
EXT UINT32          AdcPendCnt;
EXT UINT8           AdcPendBuf[ADC_PENDING_SIZE];
#else
EXT UINT8           AdcPendCh;        //suspend channel.
#endif

#ifdef _IN_ADC_
EXT _ATTR_DRIVER_DATA_  UINT8   AdcSamplingChConfig[4] = {ADC_CHANEL_KEY, ADC_CHANEL_KEY, ADC_CHANEL_KEY, ADC_CHANEL_BATTERY}; 
#else
EXT _ATTR_DRIVER_DATA_  UINT8   AdcSamplingChConfig[4];  
#endif

/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/
extern void AdcInit(void);
extern void AdcStart(uint8 StartCh);
extern __irq void AdcIsr(void);
extern uint32 CheckAdcState(uint8 ch);
extern uint32 GetAdcData(uint8 ch);

/*
********************************************************************************
*
*                         End of Adc.h
*
********************************************************************************
*/
#endif

