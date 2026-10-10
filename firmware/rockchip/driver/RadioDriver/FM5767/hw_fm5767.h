/******************************************************************/
/*   Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved.  */
/*******************************************************************
File :  hw_fm5767.h
Desc :  fm type / structure/variables/functions interfaces declaration

Author : huangxinyu
Date : 2007-08-21
Notes :

$Log: hw_fm5767.h,v $
Revision 1.3  2007/10/08 02:38:41  Lingzhaojun
添加版本自动注释脚本

*********************************************************************/

#ifndef _HW_FM5767_H
#define _HW_FM5767_H


//------------------------------------------------------------------------------
#define    TUNER_HIGHT_IF           0x3e
#define    YUNER_LOW_IF             0x3e

#define    TUNER_ADD_W              0xc0            /* TEA5767HN I2C slave address write DEVID */
#define    TUNER_ADD_R              0xc1            /* TEA5767HN I2C slave address write DEVID */

#define    TUNER_REFFREQ            32768           /* tuner reference frequency, 32.768Khz    */

//------------------------------------------------------------------------------
/* bit mask value for the 1st data in write mode    */
#define    TUNER_MUTE               0x80            /* 1: mute enabled, 0: not mute                     */
#define    TUNER_SM                 0x40            /* search mode, 1:search, 0:not search mode         */
#define    TUNER_PLLHIGH            0x3f            /* PLL[13:8] value                                  */

/* bit mask value for the 2nd data in write mode    */
#define    TUNER_PLLLOW             0xff            /* PLL[7:0] value                                   */

/* bit mask value for the 3rd data in write mode    */
#define    TUNER_SUD                0x80            /* search up/down, 1: search up, 0: search down     */
#define    TUNER_SSL                0x20            /* search stop level, datasheet pp.11, Table 12     */
#define    TUNER_HLSI               0x10            /* Hight/Low side injection, 1: high, 0: low        */
#define    TUNER_MS                 0x08            /* Mono/Stereo, 1: mono, 0:stereo                   */
#define    TUNER_ML                 0x04            /* Mute Left, 1: left ch. muted, forced mono        */
#define    TUNER_MR                 0x02            /* Mute right, 1: right ch. muted, forced mono      */
#define    TUNER_SWP1               0x01            /* Software programmable port1, 1: port1 is high    */

/* bit mask value for the 4th data in write mode    */
#define    TUNER_SWP2               0x80            /* Software programmable port2, 1: port2 is high    */
#define    TUNER_STBY               0x40            /* Standby, 1: forced to standby state              */
#define    TUNER_BL                 0x20            /* Band Limit, 1: Japan FM band, 0: US/Europe FM    */
#define    TUNER_XTAL               0x10            /* Select xtal, 1: 32.768kHz, 0: 13MHz              */
#define    TUNER_SMUTE              0x08            /* Soft Mute, 1: mute on, 0: mute off               */
#define    TUNER_HCC                0x04            /* High Cut Control, 1: on, 0: off                  */
#define    TUNER_SNC                0x02            /* Stereo noise cancelling, 1: on, 0: off           */
#define    TUNER_SI                 0x01            /* Search indicator, 1: SWPORT1 is ready flag       */

/* bit mask value for the 5th data in write mode    */
#define    TUNER_PLLREF             0x80            /* 1: 6.5MHz reference, 0: not enabled              */
#define    TUNER_DTC                0x40            /* 1: de-emphasis time constants is 75us, 0: 50us   */

/* bit mask value for the 1st data in read mode     */
#define    TUNER_RF                 0x80            /* Ready Flag, 1: a station has been found          */
#define    TUNER_BLF                0x40            /* Band Limit Flag, 1: band limit has been reached  */

/* bit mask value for the 3rd data in read mode     */
#define    TUNER_STEREO             0x80            /* Stereo indication, 1: stereo reception, 0: mono  */

/* bit mask value for the 4th data in read mode     */
#define    TUNER_LEV                0xf0            /* Level ADC output                                 */
#define    TUNER_CI                 0x0e            /* Chip identification, these bit must be set to 0  */
//------------------------------------------------------------------------------
#define    RFF                      0x10
#define    RFNE                     0x08
#define    TFE                      0x04
#define    TFNF                     0x02
#define    ACTIVITY                 0x01
#define    ENABLE                   0

#define     FM_VS_LLIMIT            0x31
#define     FM_VS_HLIMIT            0x3e

#define     FM_SENSI                0x3//6                /*sensitivity threshold from low to high is 0 ~ 7 */
#define     FM_VS_WLIMIT            2

extern void FM5767_Tuner_SetInitArea(UINT8 area);
extern void FM5767_Tuner_SetFrequency(UINT32 n10KHz, UINT8 HILO, BOOL ForceMono,UINT16 Area);              
extern void FM5767_Tuner_SetStereo(BOOL bStereo);
extern void FM5767_Tuner_PowerOffDeinit(void);
extern UINT16 FM5767_Tuner_SearchByHand(UINT16 direct, UINT32 *FmFreq);
extern void FM5767_Tuner_PowerDown(void);
extern void FM5767_Tuner_MuteControl(bool active);               
extern BOOLEAN FM5767_GetStereoStatus(void);


_ATTR_FMDRIVER_FM5767_BSS_ UINT8   IFReadDataH[5];    //IF upper offset point 
_ATTR_FMDRIVER_FM5767_BSS_ UINT8  IFReadDataL[5];    // IF lower offset point 
_ATTR_FMDRIVER_FM5767_BSS_ UINT8   TEA5767ControlData[5];

#endif  /* _HW_FM_H */
