/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name?那o   alc5633.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo          2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/
#include "DriverInclude.h"

#if (CODEC_CONFIG == CODEC_ALC5633)

#include "Hw_alc5633.h"

#define  ALC5633_Codec_IICAdress   (0x1C<<1)
#define  ALC5633_Codec_IICSpeed    400

struct rt5633_init_reg{
	UINT8 reg;
	UINT16 val;
};
static struct rt5633_init_reg init_list[] = {
	//{RT5633_SPK_OUT_VOL		    , 0x8000},//speaker output volume is 0db by default
	{RT5633_SPK_HP_MIXER_CTRL	, 0x0030},//HP from HP_VOL
	{RT5633_HP_OUT_VOL 		    , 0xc0c0},//HP output volume is 0 db by default
	//{RT5633_AUXOUT_VOL		    , 0x0010},//Auxout volume is 0db by default
	//{RT5633_REC_MIXER_CTRL		, 0x7d7d},//ADC Record Mixer Control
	//{RT5633_MIC_CTRL_2		    , 0x5500},//boost 20db
	//{RT5633_AUXMIXER_CTRL		, 0x3e3e},//"AUX Mixer Control"
	//{RT5633_SPKMIXER_CTRL		, 0x08fc},//"SPK Mixer Control"
	//{RT5633_ZC_SM_CTRL_1		, 0x0001},//Disable Zero Cross
	//{RT5633_ZC_SM_CTRL_2		, 0x3000},//Disable Zero cross
	//{RT5633_MIC_CTRL_1       	, 0x8808}, //set mic1 to differnetial mode
	{RT5633_DEPOP_CTRL_2		, 0x8000},
	//{RT5633_SPK_AMP_CTRL		, 0x0000}, //set to class AB
	//{RT5633_GEN_PUR_CTRL_1		, 0x8C00}, //set spkratio to auto
	//{RT5633_PRI_REG_ADD		    , 0x0056},
	//{RT5633_PRI_REG_DATA		, 0x303f},
//JD setting
   	//{RT5633_ZC_SM_CTRL_1    	, 0x04b0},
   	//{RT5633_ZC_SM_CTRL_2        , 0x3000},
	{RT5633_JACK_DET_CTRL       , 0x0000},
};

static int is_mute;

static void Codec5633CommandSet(UINT8 uCmd, UINT16 uData);

//_ATTR_DRIVER_CODE_
static void Codec5633CommandSet(UINT8 uCmd, UINT16 uData)
{
    uint8 cmd,data[2];
    int ret;
    DEBUG("uCmd = 0x%x",uCmd);
    DEBUG("uData = 0x%x",uData);
    //data = (uint8)(uData & 0xff);
    data[0] = (0xff00 & uData) >> 8;
	data[1] = (0xff & uData);
    I2C_Init(I2C_CH1_PA,ALC5633_Codec_IICAdress, ALC5633_Codec_IICSpeed);

    ret =  I2C_Write(I2C_CH1,uCmd,data,2, NormalMode);
    if(ret <= 0) //ack not recived,try again.
    {
        if(ret == TIMEOUT)
            DEBUG("i2c write time out ! \n");

        if(ret == ERROR)
            DEBUG("i2c write error happen (noACK)! \n" );

        if(ret == 0)
            DEBUG("i2c write not success! \n");

   	    //DelayMs(5);
        //ret =  I2C_Write(I2C_CH1,uCmd,data,2, NormalMode);
        //printf("send data ret %d \n",ret);
    }
    I2C_Deinit(I2C_CH1_PA);
}

//_ATTR_DRIVER_CODE_
void Codec5633CommandRead(UINT8 uCmd, UINT8 *uData)
{
    uint8 cmd,data;
    int  ret;

    //cmd  = (uCmd << 1);

    I2C_Init(I2C_CH1_PA,ALC5633_Codec_IICAdress, ALC5633_Codec_IICSpeed);

    ret = I2C_Read(I2C_CH1,uCmd, uData, 2, NormalMode);

    printf("\n read data ret %d \n",ret);
    I2C_Deinit(I2C_CH1_PA);
}



//_ATTR_DRIVER_CODE_
void Codec_DACMute(void)
{
    DEBUG("ALC 5633 Codec_DACMute");
    Codec5633CommandSet(RT5633_DAC_CTRL,DAC_L_MUTE|DAC_R_MUTE);       // DAC soft mute
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  ?芍?谷“o“a:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
UINT16 Codec5633CommandWrodRead(UINT8 RegAddr)
{
    UINT16 temp1;
    UINT8 temp[2];
    //I2C_Init(ALC5633_Codec_IICAdress, ALC5633_Codec_IICSpeed);
    //I2C_Read(RegAddr,temp,2, NormalMode);
    temp1 = temp[0];
    temp1<<=8;
    temp1 |= temp[1];
    return temp1;
}
static int Codec5633CommandSet_Mask(UINT8 uCmd, UINT16 uData,UINT16 mask)
{
	UINT16 reg_val;
	int ret = 0;

	if (!mask)
		return 0;

	if (mask != 0xffff)
    {
		reg_val = Codec5633CommandWrodRead(uCmd);
		reg_val &= ~mask;
		reg_val |= (uData & mask);
		Codec5633CommandSet(uCmd, reg_val);
	}
    else
	{
		Codec5633CommandSet(uCmd, uData);
	}

	return ret;
}
//HP power on depop
static void hp_depop_mode(void)
{
        Codec5633CommandSet_Mask(RT5633_PWR_MANAG_ADD3,PWR_MAIN_BIAS|PWR_VREF,PWR_VREF|PWR_MAIN_BIAS);
        Codec5633CommandSet_Mask(RT5633_PWR_MANAG_ADD4,PWR_HP_L_VOL|PWR_HP_R_VOL,PWR_HP_L_VOL|PWR_HP_R_VOL);
        Codec5633CommandSet_Mask(RT5633_PWR_MANAG_ADD3,PWR_HP_AMP,PWR_HP_AMP);
        Codec5633CommandSet_Mask(RT5633_DEPOP_CTRL_1,PW_SOFT_GEN|EN_DEPOP_2,PW_SOFT_GEN|EN_DEPOP_2);

        //Codec5633CommandSet_Mask(RT5633_PWR_MANAG_ADD3,PWR_HP_DIS_DEPOP|PWR_HP_AMP_DRI,PWR_HP_DIS_DEPOP|PWR_HP_AMP_DRI);
        Codec5633CommandSet_Mask(RT5633_DEPOP_CTRL_1,0,EN_DEPOP_2);
}
//HP mute/unmute depop
static void hp_mute_unmute_depop(unsigned int Mute)
{
	if(Mute)
	{
		Codec5633CommandSet_Mask(RT5633_DEPOP_CTRL_1,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP
												   ,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
		Codec5633CommandSet_Mask(RT5633_HP_OUT_VOL, 0x8080, 0x8080);
		DelayMs(80);
		Codec5633CommandSet_Mask(RT5633_DEPOP_CTRL_1,0,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
	}
	else
	{
		Codec5633CommandSet_Mask(RT5633_DEPOP_CTRL_1,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP
												   ,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
		Codec5633CommandSet_Mask(RT5633_HP_OUT_VOL, 0x0000, 0x8080);
		DelayMs(80);
		Codec5633CommandSet_Mask(RT5633_DEPOP_CTRL_1,0,PW_SOFT_GEN|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
	}

}
//_ATTR_DRIVER_CODE_
void Codec_DACUnMute(void)
{
    DEBUG("ALC 5633 Codec_DACUnMute");
    Codec5633CommandSet(RT5633_DAC_CTRL,DAC_L_UN_MUTE|DAC_R_UN_MUTE); ; // DAC soft unmute
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_PowerOnInitial(void)
  Author        : yangwenjie
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void Codec_PowerOnInitial(void)
{
    UINT8 uData[16] = {0,};
    UINT16 i;
    DEBUG();
    //I2C_Init(I2C_CH1_PA,ALC5633_Codec_IICAdress, ALC5633_Codec_IICSpeed);
    DEBUG("ALC5633 Codec_PowerOnInitial");
    Codec5633CommandSet(RT5633_RESET, 0x00);   //reset

    ///////////////////////////////////////////////////////////
    //power
    //Codec5633CommandSet(RT5633_PWR_MANAG_ADD3,PWR_VREF | PWR_MAIN_BIAS | PWR_HP | PWR_HP_AMP);   //power on
    Codec5633CommandSet(RT5633_PWR_MANAG_ADD3,PWR_VREF |PWR_DIS_FAST_VREF| PWR_MAIN_BIAS | PWR_HP |PWR_VCM_HP | PWR_HP_OUT| PWR_HP_AMP);   //power on

    Codec5633CommandSet(RT5633_PWR_MANAG_ADD1,(PWR_MAIN_I2S_EN |
                                               PWR_ADC_L_CLK | PWR_ADC_R_CLK |
                                               PWR_DAC_L_CLK | PWR_DAC_R_CLK | PWR_DAC_REF |
                                               PWR_DAC_L_TO_MIXER | PWR_DAC_R_TO_MIXER));                       //power I2S and DAC

    Codec5633CommandSet(RT5633_PWR_MANAG_ADD2,(PWR_PLL |
                                               PWR_HPMIXER_L | PWR_HPMIXER_R |
                                               PWR_RECMIXER_L|PWR_RECMIXER_R |
                                               PWR_LINEIN1_L |PWR_LINEIN1_R));                         //POWER PLL

    Codec5633CommandSet(RT5633_PWR_MANAG_ADD4, (PWR_HP_L_VOL | PWR_HP_R_VOL));  //power hp vol

    // input set
    Codec5633CommandSet(RT5633_REC_MIXER_CTRL, 0xf7f7);                 //Line 1 Input
    Codec5633CommandSet(RT5633_ADC_CTRL,0x0000);                        //ADC unmute
    Codec5633CommandSet(RT5633_LINE_IN_1_VOL,0x0000);                   // Line 1 Vol 0db
    Codec5633CommandSet(RT5633_ADC_DIG_VOL, 0x0000);                    // ADC vol 0db

    // output set
    Codec5633CommandSet(RT5633_DAC_CTRL,0x0000);                         // DAC unmute
    Codec5633CommandSet(RT5633_DAC_DIG_VOL,0x0000);                      // 0db

    Codec5633CommandSet(RT5633_HPMIXER_CTRL, 0x3e3e );                    // HpMix RecMix output
    Codec5633CommandSet(RT5633_SPK_HP_MIXER_CTRL,UM_HPVOL_TO_HPO_MIXER|UM_DAC_TO_HPO_MIXER/*0x0010*/); // dac hp_mix vol to hpout

    // Headphone Output Volume Control
    Codec5633CommandSet(RT5633_HP_OUT_VOL,HP_L_VOL_SEL_HPMIX_L | HP_R_VOL_SEL_HPMIX_R);                     // HPMIX output

//test 5633 reg read.
//    DelayMs(10);
//    Codec5633CommandRead(RT5633_HP_OUT_VOL,uData);
//    dumpMemoryCharA(&uData[0], 2, 1);

    //4 Codec is Master mode
    //Codec5633CommandSet(RT5633_SDP_CTRL,  (SDP_MODE_SEL_MASTER|SDP_DAC_CPS_SEL_OFF|
    //                                        SDP_DAC_DATA_L_R_SWAP|
    //                                        SDP_I2S_DL_24|SDP_I2S_DF_I2S));
    //4 Codec is Slave mode
    Codec5633CommandSet(RT5633_SDP_CTRL,  (SDP_MODE_SEL_SLAVE|SDP_DAC_CPS_SEL_OFF|
                                            SDP_DAC_DATA_L_R_NORMAL|
                                            SDP_I2S_DL_16|SDP_I2S_DF_I2S));   //codec is slave

    Codec5633CommandSet(RT5633_STEREO_AD_DA_CLK_CTRL, /*0x0000*/SEL_I2S_PRE_DIV1_1 |
                                                                SEL_I2S_BCLK_MSL_32 |
                                                                SEL_DAC_OSR_128FS |
                                                                SEL_FILTER_CLK_256FS);
//PLL from where
#define PLL_FR_BCLK (1<< 13)
#define PLL_FR_MCLK (0<< 13)
Codec5633CommandSet(RT5633_GBL_CLK_CTRL,SYSCLK_SEL_MCLK | PLL_FR_MCLK /*|1<< 11*/);

    //Codec5633CommandSet(RT5633_PLL_CTRL,0x4d4c);                        //44.1kHz

    // HP dePOP
    Codec5633CommandSet(RT5633_DEPOP_CTRL_1,PW_SOFT_GEN|EN_DEPOP_2|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
    Codec5633CommandSet(RT5633_DEPOP_CTRL_2, 0x8000);
    Codec5633CommandSet(RT5633_DEPOP_CTRL_1, 0x0000);


    Codec5633CommandSet(RT5633_EQ_CTRL_1,0X0000);
//add just for test:generate digital beep.
//    Codec5633CommandSet(RT5633_DIG_BEEP_IRQ_CTRL, 0x1f00);

}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   : “|““??Codec??DD?∟?“o?

  Input         : Codecmode?那o??DD??那“o?

  Return        : ?T

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  ?芍?谷“o“a:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVER_CODE_
void Codec_SetMode(CodecMode_en_t Codecmode, uint32 datawidth)
{
    switch (Codecmode)
    {
        case Codec_DACoutHP:
        {
            //setup left and right MIXER
            //hp_depop_mode();
            //hp_mute_unmute_depop(0);
        }
        break;
#if 0
        case Codec_FMin:
        {
            CodecCommandSet(WM8987_R34, LI2LO | LO0DB);
            CodecCommandSet(WM8987_R37, RI2RO | RO0DB);

            CodecCommandSet(WM8987_R25, WM_VMID50k | WM_VREF);
            CodecCommandSet(WM8987_R26, WM_LOUT2 | WM_ROUT2 | WM_OUT3);
        }
        break;

        case Codec_FMADC:
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

        case Codec_LineADC:
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
#endif
    }

}
void Codec_DeInitial(void)
{
		Codec5633CommandSet_Mask(RT5633_SPK_OUT_VOL, 0x8080, 0x8080);	//mute speaker volume
		Codec5633CommandSet_Mask(RT5633_HP_OUT_VOL, 0x8080, 0x8080);	//mute hp volume
		Codec5633CommandSet(RT5633_PWR_MANAG_ADD1, 0x0000);
		Codec5633CommandSet(RT5633_PWR_MANAG_ADD2, 0x0000);
		Codec5633CommandSet(RT5633_PWR_MANAG_ADD3, 0x0000);
		Codec5633CommandSet(RT5633_PWR_MANAG_ADD4, 0x0000);
}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetSampleRate(CodecFS_en_t CodecFS)
  Author        : yangwenjie
  Description   : “|““??Codec2“|?“∩?“o

  Input         : CodecFS?那o2“|?“∩?|足?“o

  Return        : ?T

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  ?芍?谷“o“a:
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVER_CODE_
void Codec_SetSampleRate(CodecFS_en_t CodecFS)
{
    #if 0
    DEBUG("CodecFS = %d",CodecFS);
    switch (CodecFS)
    {
        case FS_8000Hz:
            Codec5633CommandSet(RT5633_PLL_CTRL,0x0040);      //12MHZ,8KHZ
            DEBUG("8KHZ");
            break;
        case FS_11025Hz:
            Codec5633CommandSet(RT5633_PLL_CTRL,0x123f);      //12MHZ,11.025K
            DEBUG("11.025KHZ");
            break;
        case FS_12KHz:
            Codec5633CommandSet(RT5633_PLL_CTRL,0x0020);      //12MHZ,12K
            DEBUG("12KHZ");
            break;
        case FS_16KHz:
            Codec5633CommandSet(RT5633_PLL_CTRL,0x276d);      //12MHZ,16K
            DEBUG("16KHZ");
            break;
        case FS_22050Hz:
            Codec5633CommandSet(RT5633_PLL_CTRL,0x2e4f);      //12MHZ,22.05K
            DEBUG("22.05KHZ");
            break;
        case FS_24KHz:
            Codec5633CommandSet(RT5633_PLL_CTRL,0x294c);      //12MHZ,24K
            DEBUG("24KHZ");
            break;
        case FS_32KHz:
            Codec5633CommandSet(RT5633_PLL_CTRL,0x456b);      //12MHZ,32K
            DEBUG("32KHZ");
            break;
        case FS_48KHz:
            Codec5633CommandSet(RT5633_PLL_CTRL,0x291c);      //12MHZ,48K
            DEBUG("48KHZ");
            break;
        case FS_44100Hz:
        default:
            DEBUG("44.1KHZ");
            Codec5633CommandSet(RT5633_PLL_CTRL,0x4d4c);      //12MHZ,44.1KHZ
            break;
    }
    #endif
}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetVolumet(unsigned int Volume)
  Author        : yangwenjie
  Description   :

  Input         :

  Return        : ?T

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
              volume = 0 mean mute,
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVER_CODE_
void Codec_SetVolumet(unsigned int Volume)
{
    UINT32 Vol_L,Vol_R ;

    if (Volume > MAX_VOLUME)
        Volume = MAX_VOLUME;

   Volume = MAX_VOLUME - Volume;    //-46.5dB, with 1.5dB/step to 0db

    if(Volume == MAX_VOLUME)
    {
        Codec_DACMute();
        is_mute = TRUE;
    }
    else
    {
        if(is_mute)
        {
            is_mute = FALSE;
            Codec_DACUnMute();
        }

        Vol_L = (Volume << 8);
        Vol_R = (Volume << 0);

        DEBUG("5633 Codec_SetVolumet VOL_L_R = %d",Volume);
        Codec5633CommandSet(RT5633_HP_OUT_VOL,  Vol_L |Vol_R|HP_L_VOL_SEL_HPMIX_L|HP_R_VOL_SEL_HPMIX_R);    // set output 1 volume
    }

}
void Codec_ExitMode(CodecMode_en_t Codecmode)
{

}

void Codec_Suspend(void)
{
}

void Codec_Resume(void)
{
}

#endif


