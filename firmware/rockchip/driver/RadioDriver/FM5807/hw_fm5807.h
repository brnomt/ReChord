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

#ifndef _HW_FM5807_H
#define _HW_FM5807_H

#undef  EXT
#ifdef _IN_HWFM5807_
#define EXT
#else
#define EXT extern
#endif

#define    RUNER_AREA_JA            0x0002      // 1Ϊja 0Ϊch   us 
#define    RUNER_SEEK_DN            0x01      //
#define    RUNER_SEEK_UP            0x03      //
#define    TUNER_MONO               0x20            /* Stereo indication, 0: stereo reception, 1: mono  5800*/
#define    TUNER_MUTE               0x40               //  mute  5800

//------------------------------------------------------------------------------
#define    TUNER_HIGHT_IF           0x3e
#define    YUNER_LOW_IF             0x3e

#define    TUNER_ADD_W              0xc0            /* TEA5767HN I2C slave address write DEVID */
#define    TUNER_ADD_R              0xc1            /* TEA5767HN I2C slave address write DEVID */

#define    TUNER_REFFREQ            32768           /* tuner reference frequency, 32.768Khz    */

//------------------------------------------------------------------------------
/* bit mask value for the 1st data in write mode    */
//#define    TUNER_MUTE               0x80            /* 1: mute enabled, 0: not mute                     */
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

#define    TUNER_SEEK                0x01  

extern void FM5807_Tuner_SetInitArea(UINT8 area);
extern void FM5807_Tuner_SetFrequency(UINT32 n10KHz, UINT8 HILO, BOOL ForceMono,UINT16 Area);              
extern void FM5807_Tuner_SetStereo(BOOL bStereo);
extern void FM5807_Tuner_PowerOffDeinit(void);
extern UINT16 FM5807_Tuner_SearchByHand(UINT16 direct, UINT32 *FmFreq);
extern void FM5807_Tuner_PowerDown(void);
extern void FM5807_Tuner_MuteControl(bool active);               
extern BOOLEAN FM5807_GetStereoStatus(void);
extern void FM5807_Tuner_Vol_Set(UINT8 gain);


_ATTR_FMDRIVER_FM5807_BSS_ UINT8   TEA5800ControlData[5];
_ATTR_FMDRIVER_FM5807_BSS_ UINT16  RDA5800REG_IMG[60];
_ATTR_FMDRIVER_FM5807_BSS_ UINT8   READCHAN;
//UINT16  cwFMDefault[] =
_ATTR_FMDRIVERL_FM5807_DATA_
UINT16	cwFM5807Default[]={									//init data table
0x0000,
0x0000,

//0xC0D1, //0xC451, //02H:  //24M crystal : 0xC0, 0xD1, 32.768k crystal: 0xC0, 0x01;
0xc081,//0x02		//d881-->9881 for first init don't need voice output,0xb881 no stereo

0x1a10,			//97100KHZ
0x0600,			//no interruput	
0x9cad, //05h    0x88, 0xAF
0x0000,
0x46c6,
0x5096,
0x0000,
0x0468, //0x0a
0x650f,
0x0000,
0x0000,
0x0000,
0x0000, 
0xf027,//4c08//0x4808,//0x10
0x9000,
0xf483,
0x7001,
0x40f0,
0x6180, //0x15
0x01D6,
0x10bb,
0x55a4,
0xe848,
0x5080,//0x1a
0x0000,
0x37b6,
0x420c,
0x079b,
0x4c1d, 
0x8810,//0x20
0x45c0,
0x0923,
0x98a4,
0x0500,
0x1928, //0x25
0x2d04,
0x002f,
0x17af,
0xd34e,
0x348c,//0x2a
0xa004,
0x7eac,
0x0014,
0x0CFF,
0x3C1E,
0x0200, //30H:
0x16F4,
0xBD00,
0x830C,
0xB9FC,
0x8287, //35H:
0x0000,
0x0000,
0x028B,
0x8895,
0x8D9F, //3AH:
0x92AA,
};


#define     RADIO_CHINA         0                       //China       
#define     RADIO_EUROPE        1                       //Europe           
#define     RADIO_JAPAN         2                       //Japan                
#define     RADIO_USA           3                       //America

#endif  /* _HW_FM_H */
