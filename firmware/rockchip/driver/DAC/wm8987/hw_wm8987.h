/*********************************************************************************
*     Copyright (C),2004-2005,  Fuzhou Rockchip Co.,Ltd.
*         All Rights Reserved
*          V1.00
* FileName :  Hw_WM8987.h
* Author :  lzy
* Description:
* History  :
*   <author>  <time>    <version>    <desc>
*    lzy        07/6/29        1.0     ORG
*
$Log: hw_wm8987.h,v $
Revision 1.2  2007/11/14 01:42:26  Huangxinyu
重新调试WM8987后的更新

Revision 1.1  2007/10/15 08:57:16  Huangxinyu
根据RK27提交修改driver


*********************************************************************************/
#if (CODEC_CONFIG == CODEC_WM8987)

#ifndef _WM8987_H
#define _WM8987_H

#define       WM8987_R0           0x00      //Left input volume
#define       WM8987_R1           0x01      //Right input volume
#define       WM8987_R2           0x02      //Lout1 volume
#define       WM8987_R3           0x03      //Rout1 volume 
#define       WM8987_R4           0x04      //Reserved
#define       WM8987_R5           0x05      //ADC&DAC control
#define       WM8987_R6           0x06      //Reserved 
#define       WM8987_R7           0x07      //Audio interface
#define       WM8987_R8           0x08      //Sample rate
#define       WM8987_R9           0x09      //Reserved 
#define       WM8987_R10          0x0a      //Left DAC volume
#define       WM8987_R11          0x0b      //Right DAC volume
#define       WM8987_R12          0x0c      //Bass control
#define       WM8987_R13          0x0d      //Treble control
#define       WM8987_R15          0x0f      //Reset               
#define       WM8987_R16          0x10     // //3D control          
#define       WM8987_R17          0x11      //ALC1                
#define       WM8987_R18          0x12      //ALC2                
#define       WM8987_R19          0x13      //ALC3                
#define       WM8987_R20          0x14      //Noise Gate          
#define       WM8987_R21          0x15      //Left ADC volume     
#define       WM8987_R22          0x16      //Right ADC volume    
#define       WM8987_R23          0x17      //Additional control1 
#define       WM8987_R24          0x18      //Additional control2 
#define       WM8987_R25          0x19      //Pwr Mgmt1            
#define       WM8987_R26          0x1a      //Pwr Mgmt2
#define       WM8987_R27          0x1b      //Additional control3
#define       WM8987_R31          0x1f      //ADC input mode
#define       WM8987_R32          0x20      //ADCL signal path
#define       WM8987_R33          0x21      //ADCR signal path
#define       WM8987_R34          0x22      //Left out mix1
#define       WM8987_R35          0x23      //Left out mix2
#define       WM8987_R36          0x24      //Right out Mix1
#define       WM8987_R37          0x25      //Right out Mix2
#define       WM8987_R38          0x26      //Mono out mix1
#define       WM8987_R39          0x27      //Mono out mix2
#define       WM8987_R40          0x28      //Lout2 volume
#define       WM8987_R41          0x29      //Rout2 volume
#define       WM8987_R42          0x2a      //Monoout volume

//WM8987_R7
#define   WM_MASTER_MODE 0x40
#define   WM_SLAVE_MODE 0x00
#define  WM_16BIT_MODE (0x0<<2)
#define  WM_24BIT_MODE (0x2<<2)
#define  WM_DSP_MODE  0x03
#define     WM_I2S_MODE  0x02

//WM8987_R8
#define   WM_BCKdiv0               (0<<7)
#define   WM_BCKdiv4               (1<<7)
#define   WM_BCKdiv8               (2<<7)
#define   WM_CLKDIV2  0x40

#define  FREQ96kHz    (0x0e<<1)
#define  FREQ48kHz    (0x00<<1)
#define  FREQ441kHz   (0x11<<1)
#define  FREQ32kHz   (0x0c<<1)
#define  FREQ24kHz   (0x1c<<1)
#define  FREQ2205kHz  (0x1b<<1)
#define  FREQ16kHz   (0x0a<<1)
#define  FREQ12kHz   (0x08<<1)
#define  FREQ11025kHz  (0x19<<1)
//#define  FREQ9k6Hz   0x09
#define  FREQ8kHz   (0x06<<1)
#define  WM_USB_MODE  0x01|WM_BCKdiv8

#define   WM_VOL_MUTE  0x00
#define   WM_VOL_0DB  0xff
#define   WM_UPDATE_VOL 0x100

//PM1 WM8987_R25
#define   WM_VMID50k  0x080
#define   WM_VREF   0x40
#define  WM_AINL   0x20
#define  WM_AINR   0x10
#define WM_ADCL   0x08
#define WM_ADCR   0x04
#define WM_MICB   0x02
#define   WM_DIGENB   0x01

//PM2  WM8987_R26
#define   WM_DACL   0x100
#define   WM_DACR   0x80
#define  WM_LOUT1   0x40
#define  WM_ROUT1   0x20
#define  WM_LOUT2   0x10
#define  WM_ROUT2   0x08
#define WM_MONO   0x04
#define   WM_OUT3   0x02

// WM8987_R34  Left out mix1
#define  LD2LO    0x100
#define  LI2LO    0x80
#define  LO0DB    0x20

//WM8987_R36  Right out Mix1
#define  RD2RO    0x100
#define  RI2RO    0x80
#define  RO0DB    0x20

#define  Bass_control  WM8987_R12
#define  Treble_control  WM8987_R13
#define  EQ3D_enhance  WM8987_R16

typedef  enum EQ_Mode
{
    EQ_Flat = 0,
    EQ_Rock = 1,
    EQ_Pop = 2,
    EQ_Jazz = 3,
    EQ_Classcial  = 4,
    EQ_Dance  = 5,
    EQ_Heavy  = 6,
    EQ_Disco = 7,
    EQ_Soft = 8,
    EQ_3D = 9,
    EQ_Hall = 10,
    EQ_End
} EQ_Mode_t;


#define  Bass_Linear   0x00
#define  Bass_Adaptive  0x80

#define Bass_Cut130  0x00
#define Bass_Cut200  0x40

#define   Treble_Cut4k 0x40
#define   Treble_Cut8k 0x00

#define EQ_P9DB 0x01
#define EQ_P75DB 0x02
#define EQ_P6DB 0x03
#define EQ_P45DB 0x04
#define EQ_P3DB 0x05
#define EQ_P15DB 0x06
#define EQ_P0DB 0x07
#define EQ_N15DB 0x08
#define EQ_N3DB 0x09
#define EQ_N45DB 0x0a
#define EQ_N6DB 0x0b
#define EQ_Bypass 0x0f

#define   EQ3D_Mode  0x80
#define   EQ3D_UC2k2  0x00
#define   EQ3D_UC1k5  0x40
#define   EQ3D_LC200  0x00
#define   EQ3D_LC500  0x20
#define EQ3D_enable 0x01
#define EQ3D_disable  0x00
#define EQ3D_depth   0x01


#endif
#endif //CODEC_WM8987

