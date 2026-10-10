/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   Rockcodec.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo          2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/
#include "DriverInclude.h"
#if (CODEC_CONFIG == CODEC_WM8987)

#include "hw_wm8987.h"

#define  Codec_IICAdress   (0x1a<<1)
#define  Codec_IICSpeed    100//200

/*
--------------------------------------------------------------------------------
  Function name : void CodecCommandSet(UINT32 uCmd, UINT32 uData)
  Author        : yangwenjie
  Description   : 通过I2C向Codec发送命令

  Input         : uCmd:   输出到codec的命令
                  uData:  输入到codec的数据

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
static void CodecCommandSet(UINT8 uCmd, UINT16 uData)
{
    uint8 cmd,data;
    uint32 ret;

    cmd  = ((uCmd << 1) | (uData >> 8) & 0x01);
    data = (uint8)(uData & 0xff);

    //I2C_Init(0,Codec_IICAdress, Codec_IICSpeed);

    ret = I2C_Write(0,cmd,&data,1, NormalMode);

    if(ret <= 0) //ack not recived,try again.
    {
        if(ret == TIMEOUT)
            printf("i2c write time out ! \n");

        if(ret == ERROR)
            printf("i2c write error happen (noACK)! \n" );

        if(ret == 0)
            printf("i2c write not success! \n" );

//   		DelayMs(5);
//        ret = I2C_Write(0,cmd,&data,1, NormalMode);
//        printf("send data %d \n",cmd);
    }

   // I2C_Deinit(0);
}

/*
--------------------------------------------------------------------------------
  Function name : void CodecCommandRead(UINT8 uCmd, UINT8 *uData)
  Author        : yangwenjie
  Description   : 通过I2C向Codec发送命令

  Input         : uCmd:   输出到codec的命令
                  uData:  输入到codec的数据

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void CodecCommandRead(UINT8 uCmd, UINT8 *uData)
{
    uint8 cmd,data;

    cmd  = (uCmd << 1);

    I2C_Init(0,Codec_IICAdress, Codec_IICSpeed);

    I2C_Read(0,uCmd, uData, 16, DirectMode);

    I2C_Deinit(0);
}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_PowerOnInitial(void)
  Author        : yangwenjie
  Description   : Codec上电初始化

  Input         : 无

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void Codec_PowerOnInitial(void)
{
    CodecCommandSet(WM8987_R15, 0x012f);                        //software reset wm8750

    CodecCommandSet(WM8987_R27, 0x0040);                        // set VREF to output 40K

    //setup output and DAC mute
    CodecCommandSet(WM8987_R10, WM_VOL_MUTE);                   // left DAC mute//声音控制
    CodecCommandSet(WM8987_R11, WM_VOL_MUTE | WM_UPDATE_VOL);   // right DAC mute
    CodecCommandSet(WM8987_R5,  0x0008);                        //Mute

    DelayMs(10);
    CodecCommandSet(WM8987_R25, WM_VMID50k | WM_VREF);          // setup Vmid and Vref, Vmid =

    DelayMs(10);
    CodecCommandSet(WM8987_R26, 0x0000);

    //setup digital audio  interface
    CodecCommandSet(WM8987_R7,  WM_MASTER_MODE|WM_I2S_MODE);     //WM_MASTER_MODE | WM_I2S_MODE);   //Audio inteface Data length and Data formate,16Bit
    CodecCommandSet(WM8987_R8,  FREQ441kHz | WM_USB_MODE);      //ADC and DAC sample frequence 44.118Khz,USB mode

    //setup left and right MIXER
    CodecCommandSet(WM8987_R34, LD2LO | LO0DB);
    CodecCommandSet(WM8987_R37, RD2RO | RO0DB);

    //setup no used register
    CodecCommandSet(WM8987_R35, 0x0070);                        // Right DAC to left MIXER: mute
    CodecCommandSet(WM8987_R36, 0x0070);                        //
    CodecCommandSet(WM8987_R38, 0x0070);                        //Mono out Mix Default: Mono Mixer no input, Lmixsel volume=0
    CodecCommandSet(WM8987_R39, 0x0070);                        //Mono out Mix Default: Mono Mixer no input, Rmixsel volume=0

    CodecCommandSet(WM8987_R2,  0x0100);                        // set R  output 1 mute

    CodecCommandSet(WM8987_R40, 0x0080);                        // set output 2 mute
    CodecCommandSet(WM8987_R41, 0x0180);                        // set output 2 mute

    CodecCommandSet(WM8987_R24, 0x0004);                        // set output 3 ref

    CodecCommandSet(EQ3D_enhance, EQ3D_disable);
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DeInitial(void)
  Author        : yangwenjie
  Description   : 关闭Codec

  Input         : 无

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_DeInitial(void)
{
    CodecCommandSet(WM8987_R25,    WM_VREF);
    CodecCommandSet(WM8987_R26,    0x00);
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   : 设置Codec运行方式

  Input         : Codecmode：运行模式

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetMode1(CodecMode_en_t Codecmode)
{

    switch (Codecmode)
    {
        case Codec_DACoutHP:
        {
//            CodecCommandSet(WM8987_R7,  WM_SLAVE_MODE | WM_I2S_MODE );
            CodecCommandSet(WM8987_R8,  FREQ441kHz | WM_USB_MODE);

            CodecCommandSet(WM8987_R34, LD2LO | LO0DB);
            CodecCommandSet(WM8987_R37, RD2RO | RO0DB);

            CodecCommandSet(WM8987_R10, WM_VOL_0DB);
            CodecCommandSet(WM8987_R11, WM_VOL_0DB | WM_UPDATE_VOL);

            CodecCommandSet(WM8987_R25, WM_VMID50k | WM_VREF);
            CodecCommandSet(WM8987_R26, WM_DACL | WM_DACR | WM_LOUT2 | WM_ROUT2 | WM_OUT3);
        }
        break;

        case Codec_Line1in:
        {
            CodecCommandSet(WM8987_R34, LI2LO | LO0DB);
            CodecCommandSet(WM8987_R37, RI2RO | RO0DB);

            CodecCommandSet(WM8987_R25, WM_VMID50k | WM_VREF);
            CodecCommandSet(WM8987_R26, WM_LOUT2 | WM_ROUT2 | WM_OUT3);
        }
        break;

        case Codec_Line1ADC:
        {
            CodecCommandSet(WM8987_R0,  0x003f);    //Left channel PGA  0dB
            CodecCommandSet(WM8987_R1,  0x013f);    //Left channel PGA   0dB

            CodecCommandSet(WM8987_R21, 0x00c3);    //Left digital ADC 0dB
            CodecCommandSet(WM8987_R22, 0x01c3);    //Right  digital ADC 0dB

            CodecCommandSet(WM8987_R31, 0x0000);

            CodecCommandSet(WM8987_R32, 0x0000);    //Left ADC select = Linput0 MICBoost = 0dB
            CodecCommandSet(WM8987_R33, 0x0000);    //Left ADC select = Linput3 MICBoost = 0dB

            CodecCommandSet(WM8987_R34, LI2LO | LO0DB);
            CodecCommandSet(WM8987_R37, RI2RO | RO0DB);

            CodecCommandSet(WM8987_R25, WM_VMID50k | WM_VREF | WM_ADCL | WM_ADCR);
            CodecCommandSet(WM8987_R26, WM_LOUT2 | WM_ROUT2 | WM_OUT3);
        }
        break;

        case Codec_Line2ADC:
        {
            CodecCommandSet(WM8987_R0,  0x0057);    //Left channel PGA  0dB
            CodecCommandSet(WM8987_R1,  0x0157);    //Left channel PGA   0dB

            CodecCommandSet(WM8987_R21, 0x00c3);    //Left digital ADC 0dB
            CodecCommandSet(WM8987_R22, 0x01c3);    //Right  digital ADC 0dB

            CodecCommandSet(WM8987_R31, 0x0000);

            CodecCommandSet(WM8987_R32, 0x0000);    //Left ADC select = Linput0 MICBoost = 0dB
            CodecCommandSet(WM8987_R33, 0x0000);    //Left ADC select = Linput0 MICBoost = 0dB

            CodecCommandSet(WM8987_R34, LO0DB);
            CodecCommandSet(WM8987_R37, RO0DB);

            CodecCommandSet(WM8987_R25, WM_VMID50k | WM_VREF | WM_ADCL | WM_ADCR);
            CodecCommandSet(WM8987_R26, 0x0000);
        }
        break;

        case Codec_MICAdc:
        {
            CodecCommandSet(WM8987_R0,  0x0117);    //Left channel PGA  0dB
            CodecCommandSet(WM8987_R1,  0x0117);    //Left channel PGA   0dB

            CodecCommandSet(WM8987_R21, 0x01c3);    //Left digital ADC 0dB
            CodecCommandSet(WM8987_R22, 0x01c3);    //Right  digital ADC 0dB

            CodecCommandSet(WM8987_R10, 0x01ff);    //Left digital DAC 0dB
            CodecCommandSet(WM8987_R11, 0x01ff);    //Right  digital DAC 0dB

            CodecCommandSet(WM8987_R31, 0x0040);    // adc use mono mix, select left adc

            CodecCommandSet(WM8987_R32, 0x00a0);    //Left ADC select = Linput3 MICBoost = 20dB
            CodecCommandSet(WM8987_R33, 0x0080);    //Left ADC select = Linput3 MICBoost = 00dB

            CodecCommandSet(WM8987_R25, WM_VMID50k | WM_VREF | WM_AINL | WM_ADCL | WM_MICB);   //Left ADC and Left PGA power on,Right ADC and Right PGA power off
            CodecCommandSet(WM8987_R26, 0x00);
        }
        break;

        case Codec_Standby:
        {
            Codec_DACMute();
            CodecCommandSet(WM8987_R25, WM_VMID50k | WM_VREF);
            CodecCommandSet(WM8987_R26, 0x00);
        }
        break;

    }
    Codec_DACUnMute();

}

_ATTR_DRIVER_CODE_
void Codec_SetMode(CodecMode_en_t Codecmode,uint32 datawidth)
{

}


/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   : 设置Codec运行方式

  Input         : Codecmode：运行模式

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_ExitMode(CodecMode_en_t Codecmode)
{

}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetSampleRate(CodecFS_en_t CodecFS)
  Author        : yangwenjie
  Description   : 设置Codec采样率

  Input         : CodecFS：采样频率

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetSampleRate(CodecFS_en_t CodecFS)
{
    switch (CodecFS)
    {
        case FS_8000Hz:
            CodecCommandSet(WM8987_R8,    FREQ8kHz | WM_USB_MODE);    //12MHZ,8KHZ
            break;
        case FS_11025Hz:
            CodecCommandSet(WM8987_R8,    FREQ11025kHz | WM_USB_MODE);    //12MHZ,11.025K
            break;
        case FS_12KHz:
            CodecCommandSet(WM8987_R8,    FREQ12kHz | WM_USB_MODE);    //12MHZ,12K
            break;
        case FS_16KHz:
            CodecCommandSet(WM8987_R8,    FREQ16kHz | WM_USB_MODE);    //12MHZ,16K
            break;
        case FS_22050Hz:
            CodecCommandSet(WM8987_R8,     FREQ2205kHz | WM_USB_MODE);    //12MHZ,22.05K
            break;
        case FS_24KHz:
            CodecCommandSet(WM8987_R8,     FREQ24kHz | WM_USB_MODE);    //12MHZ,24K
            break;
        case FS_32KHz:
            CodecCommandSet(WM8987_R8,     FREQ32kHz | WM_USB_MODE);    //12MHZ,32K
            break;
        case FS_48KHz:
            CodecCommandSet(WM8987_R8,     FREQ48kHz | WM_USB_MODE);    //12MHZ,48K
            break;
        case FS_44100Hz:
        default:
            CodecCommandSet(WM8987_R8,     FREQ441kHz | WM_USB_MODE);    //12MHZ,44.1KHZ
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetVolumet(unsigned int Volume)
  Author        : yangwenjie
  Description   : 控制音量

  Input         : Volume：音量大小

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:      volume = 0 mean mute,
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_SetVolumet(unsigned int Volume)
{

    UINT32 Vol,Volume1 = 30;

    if (Volume1 > MAX_VOLUME)
        Volume1 = MAX_VOLUME;


    Vol = 41 + (Volume1 << 1);

    if (Volume1 == 0)
    {
        Vol = 0;
    }

    if (Volume1 <= 7)  Vol = 47 + Volume1;

    CodecCommandSet(WM8987_R40, Vol | 0x0080);    // set output 1 volume
    CodecCommandSet(WM8987_R41, Vol | 0x0180);

}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACMute(void)
  Author        : yangwenjie
  Description   : 设置Codec的声道控制器，关闭声道

  Input         : 无

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_DACMute(void)
{
    CodecCommandSet(WM8987_R5,   0x0008);      // DAC soft mute
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   : 设置Codec的声道控制器，打开声道

  Input         : 无

  Return        : 无

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  注释:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Codec_DACUnMute(void)
{
    CodecCommandSet(WM8987_R5,   0x0000); // DAC soft unmute
}


_ATTR_DRIVER_CODE_
void Codec_Suspend(void)
{
    //todo
}

_ATTR_DRIVER_CODE_
void Codec_Resume(void)
{
    //todo
}
/*
********************************************************************************
*
*                         End of Rockcodec.c
*
********************************************************************************
*/

#endif

