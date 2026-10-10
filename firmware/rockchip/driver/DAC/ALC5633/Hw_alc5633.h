
#ifndef _Hw_ALC5633_H
#define _Hw_ALC5633_H
//8.1.  Reg-00h: Software Reset
#define RT5633_RESET                        0X00
#define RT5633_SPK_OUT_VOL                  0X02            //SPEAKER OUT VOLUME
#define RT5633_SPK_HP_MIXER_CTRL            0X03            //SPEAKER/HP OUTPUT MIXER CONTROL
#define RT5633_HP_OUT_VOL                   0X04            //HEADPHONE OUTPUT VOLUME
#define RT5633_AUXOUT_VOL                   0X06            //AUXOUT OUTPUT VOLUME
#define RT5633_LINE_IN_1_VOL                0X08            //LINE IN 1 VOLUME
#define RT5633_LINE_IN_2_VOL                0X0A            //LINE IN 2 VOLUME
#define RT5633_DAC_CTRL                     0X0C            //DAC CONTROL
#define RT5633_DAC_DIG_VOL                  0X0E            //DAC DIGITAL VOLUME CONTROL
#define RT5633_MIC_CTRL_1                   0X10            //MICPHONE INPUT CONTROL
#define RT5633_ADC_CTRL                     0X12            //ADC CONTROL
#define RT5633_REC_MIXER_CTRL               0X14            //RECORD MIXER CONTROL
#define RT5633_ADC_DIG_VOL                  0X16            //ADC DIGITAL VOL CONTROL
#define RT5633_HPMIXER_CTRL                 0X18            //HP MIXER CONTROL
#define RT5633_AUXMIXER_CTRL                0X1A            //AUX MIXER CONTROL
#define RT5633_SPKMIXER_CTRL                0X1C            //SPEAKER MIXER CONTROL
#define RT5633_SPK_AMP_CTRL                 0X1E            //SPEAKER AMPLIFIER CONTROL
#define RT5633_MIC_CTRL_2                   0X22            //MICROPHONE CONTROL 2
#define RT5633_SDP_CTRL                     0X34            //STEREO I2S SERIAL DATA PORT CONTROL
#define RT5633_STEREO_AD_DA_CLK_CTRL        0X38            //STEREO AD/DA CLOCK CONTROL
#define RT5633_PWR_MANAG_ADD1               0X3A            //POWER MANAGMENT ADDITION 1
#define RT5633_PWR_MANAG_ADD2               0X3B            //POWER MANAGMENT ADDITION 2
#define RT5633_PWR_MANAG_ADD3               0X3C            //POWER MANAGMENT ADDITION 3
#define RT5633_PWR_MANAG_ADD4               0X3E            //POWER MANAGMENT ADDITION 4
#define RT5633_GEN_PUR_CTRL_1               0X40            //GENERAL PURPOSE CONTROL REGISTER
#define RT5633_GBL_CLK_CTRL                 0X42            //GLOBAL CLOCK CONTROL
#define RT5633_PLL_CTRL                     0X44            //PLL CONTROL
#define RT5633_DIG_BEEP_IRQ_CTRL            0X48            //DIGITAL BEEP GEN AND IRQ CONTROL
#define RT5633_INT_ST_STICKY_CTRL           0X4A            //INTERNAL STATUS AND STICKY CONTROL
#define RT5633_GPIO_CTRL_1                  0X4C            //GPIO CONTROL 1
#define RT5633_GPIO_CTRL_2                  0X4D            //GPIO CONTROL 2
#define RT5633_GEN_PUR_CTRL_2               0X52            //GENERAL PURPOSE CONTROL REGISTER
#define RT5633_DEPOP_CTRL_1                 0X54            //DEPOP MODE CONTROL 1
#define RT5633_DEPOP_CTRL_2                 0X56            //DEPOP MODE CONTROL 2
#define RT5633_JACK_DET_CTRL                0X5A            //JACK DETECT CONTROL REGISTER
#define RT5633_ZC_SM_CTRL_1                 0X5C            //ZERO CROSS AND SOFT VOLUME CONTROL 1
#define RT5633_ZC_SM_CTRL_2                 0X5E            //ZERO CROSS AND SOFT VOLUME CONTROL 2
#define RT5633_ALC_CTRL_1                   0X64            //ALC CONTROL 1
#define RT5633_ALC_CTRL_2                   0X65            //ALC CONTROL 2
#define RT5633_ALC_CTRL_3                   0X66            //ALC CONTROL 3
#define RT5633_PSEUDO_SPATL_CTRL            0X68            //PSEUDO STEREO AND SPATIAL EFFECT CONTROL
#define RT5633_PRI_REG_ADD                  0X6A            //INDEX ADDRESS
#define RT5633_PRI_REG_DATA                 0X6C            //INDEX DATA
#define RT5633_EQ_CTRL_1                    0X6E            //EQ CONTROL 1
#define RT5633_EQ_CTRL_2                    0X70            //EQ CONTROL 2
#define RT5633_VERSION                      0x7A            //VERSION ID
#define RT5633_VENDOR_ID                    0x7C            //VENDOR ID1


//8.2.  Reg-02h: Speaker Output Control
#define SPK_RT_MUTE                     (0x1<<15)   //MUTE LEFTCONTROL BIT
#define SPK_RN_PIN_SEL_MASK             (0x3<<13)   //SPO_RN PIN SELECT MASK
#define SPK_RN_PIN_SEL_RN               (0x0<<13)   //SPO_RN PIN SELECT RN
#define SPK_RN_PIN_SEL_RP               (0x1<<13)   //SPO_RN PIN SELECT RP
#define SPK_RN_PIN_SEL_LN               (0x2<<13)   //SPO_RN PIN SELECT LN
#define SPK_RN_PIN_SEL_VMID             (0x3<<13)   //SPO_RN PIN SELECT VMID

//8.3.  Reg-03h: Speaker/HP Output Mixer Control
#define UM_DACL_TO_SPOL_MIXER           (0x1<<8)        //UN-MUTE DAC LEFT TO SPOL_MIXER
#define UM_DACR_TO_SPOR_MIXER           (0x1<<6)        //UN-MUTE DAC RIGHT TO SPOR_MIXER
#define UM_DAC_TO_HPO_MIXER             (0x1<<5)        //UN-MUTE DAC TO HPOUT MIXER
#define UM_HPVOL_TO_HPO_MIXER           (0x1<<4)        //UN-MUTE HP VOL TO HPOUT MIXER

//8.4.  Reg-04h: Headphone Output Volume Control
#define HP_L_MUTE                       (0x1<<15)   //MUTE LEFTCONTROL BIT
#define HP_L_VOL_SEL_MASK               (0x1<<14)   //HP left channel volume input select MASK
#define HP_L_VOL_SEL_VMID               (0x0<<14)   //HP left channel volume input select VMID
#define HP_L_VOL_SEL_HPMIX_L            (0x1<<14)   //HP left channel volume input select HPMIXER LEFT
#define HP_R_MUTE                       (0x1<<7)    //MUTE RIGHT CONTROL BIT
#define HP_R_VOL_SEL_MASK               (0x1<<6)    //HP right channel volume input select MASK
#define HP_R_VOL_SEL_VMID               (0x0<< 6)   //HP right channel volume input select VMID
#define HP_R_VOL_SEL_HPMIX_R            (0x1<< 6)   //HP right channel volume input select HPMIXER RIGHT

//8.5.  Reg-06h: AUX Output Control
#define AUXOUT_L_MUTE                   (0x1<<15)   //MUTE LEFTCONTROL BIT
#define AUXOUT_MODE_SEL_MASK            (0x1<<14)   //AUXOUT OUTPUT MODE MASK
#define AUXOUT_MODE_SEL_MONO            (0x0<<14)   //AUXOUT OUTPUT MODE SELECT MONO
#define AUXOUT_MODE_SEL_STEREO          (0x1<<14)   //AUXOUT OUTPUT MODE SELECT STEREO
#define AUXOUT_R_MUTE                   (0x1<<7)    //MUTE RIGHT CONTROL BIT

//8.6.  Reg-08h: LINE1 Input Volume Control

//8.7.  Reg-0Ah: LINE2 Input Volume Control

//8.8.  Reg-0Ch: DAC Control

#define DAC_L_MUTE                      (0x1<<15)   //MUTE LEFTCONTROL BIT
#define DAC_L_UN_MUTE                   (0x0<<15)   //UN_MUTE LEFTCONTROL BIT
#define DAC_FOR_I2SMIXER_MUTE           (0x1<<14)   //Mute Control for I2S Data to Digital Mixer of DAC
#define DAC_FOR_ADCMIXER_ENABLE         (0x1<<13)   //Enable Control for ADC Data to Digital Mixer of DAC n
#define DAC_R_MUTE                      (0x1<<7)    //MUTE RIGHT CONTROL BIT
#define DAC_R_UN_MUTE                   (0x0<<7)    //UN_MUTE RIGHT CONTROL BIT
//8.9.  Reg-0Eh: DAC Digital Volume Control

//8.10. Reg-10h: Microphone Input Control
#define MIC_1_MODE_SEL_MASK             (0x1<<15)      //MIC 1 INPUT MODE MASK
#define MIC_1_MODE_SEL_SE               (0x0<<15)      //MIC 1 INPUT MODE SEL SINGLE END
#define MIC_1_MODE_SEL_DIFF             (0x1<<15)      //MIC 1 INPUT MODE SEL DIFF

#define MIC_2_MODE_SEL_MASK             (0x1<<7)       //MIC 2 INPUT MODE MASK
#define MIC_2_MODE_SEL_SE               (0x0<<7)       //MIC 2 INPUT MODE SEL SINGLE END
#define MIC_2_MODE_SEL_DIFF             (0x1<<7)       //MIC 2 INPUT MODE SEL DIFF

//8.11.  Reg-12h: ADC Control
#define ADC_L_MUTE                       (0x0<<15)  //MUTE LEFTCONTROL BIT
#define ADC_FOR_I2SDATA_MUTE             (0x0<<14)  //Mute Control for ADC Data to I2S
#define ADC_R_MUTE                       (0x0<<7)   //MUTE RIGHT CONTROL BIT


//8.12.  Reg-14h: REC Mixer Control
#define REC_Mu_hpmixL_to_recmixL        (0x1<<14)   //HP Left Mixer to REC Left Mixer (RECMIXL)
#define REC_Mu_auxmixL_to_recmixL       (0x1<<13)   //AUX Left Mixer to REC Left Mixer (RECMIXL)
#define REC_Mu_spkmixL_to_recmixL       (0x1<<12)   //Speaker Mixer to REC Left Mixer  (RECMIXL)
#define REC_Mu_lin1_L_vol_to_recmixL    (0x1<<11)   //Line In 1 Left Volume to REC Left Mixer (RECMIXL)
#define REC_Mu_lin2_L_vol_to_recmixL    (0x1<<10)   //Line In 2 Left Volume to REC Left Mixer (RECMIXL)
#define REC_Mu_bst1_to_recmixL          (0x1<<9)    //MIC1 Boost Gain to REC Left Mixer (RECMIXL)
#define REC_Mu_bst2_to_recmixL          (0x1<<8)    //MIC2 Boost Gain to REC Left Mixer (RECMIXL)
#define REC_Mu_hpmixR_to_recmixR        (0x1<<6)    //HP Right Mixer to REC Right Mixer (RECMIXR)
#define REC_Mu_auxmixR_to_recmixR       (0x1<<5)    //AUX Right Mixer to REC Right Mixer (RECMIXR)
#define REC_Mu_spkmixR_to_recmixR       (0x1<<4)    //Speaker Mixer to REC Right Mixer (RECMIXR)
#define REC_Mu_lin1_R_vol_to_recmixR    (0x1<<3)    //Line In 1 Right Volume to REC Right Mixer (RECMIXR)
#define REC_Mu_lin2_R_vol_to_recmixR    (0x1<<2)    //Line In 2 Right Volume to REC Right Mixer
#define REC_Mu_bst1_to_recmixR          (0x1<<1)    //MIC1 Boost Gain to REC Right Mixer
#define REC_Mu_bst2_to_recmixR          (0x1<<0)    //MIC2 Boost Gain to REC Right Mixer

//8.13.  Reg-16h: ADC Digital Volume Control

//8.14. Reg-18h:Headphone Mixer Control
#define HP_Mu_recmixl_to_hpmix      (0x0<<13)       //REC Left Mixer to Headphone Left Mixer (HPMIXL)
#define HP_Mu_micvol1_to_hpmixl     (0x0<<12)       //MIC 1 volume to Headphone Left Mixer  (HPMIXL)
#define HP_Mu_micvol2_to_hpmixl     (0x0<<11)       //MIC 2 volume to Headphone Left Mixer (HPMIXL)
#define HP_Mu_lin1_l_vol_to_hpmixl  (0x0<<10)       //Line In 1 Left volume to Headphone Left Mixer (HPMIXL)
#define HP_Mu_lin2_l_vol_to_hpmix   (0x0<<9)        //Line In 2 Left volume to Headphone Left Mixer (HPMIXL)
#define HP_Mu_dacl_to_hpmixl        (0x0<<8)        //DAC Left to Headphone Left Mixer (HPMIXL)
#define HP_Mu_recmixr_to_hpmixr     (0x0<<5)        //REC Right Mixer to Headphone Right Mixer (HPMIXR)
#define HP_Mu_micvol1_to_hpmixr     (0x0<<4)        //MIC 1 volume to Headphone Right Mixer (HPMIXR)
#define HP_Mu_micvol2_to_hpmixr     (0x0<<3)        //MIC 2 volume to Headphone Right Mixer (HPMIXR)
#define HP_Mu_lin1_r_vol_to_hpmixr  (0x0<<2)        //Line In 1 Right volume to Headphone Right Mixer (HPMIXR)
#define HP_Mu_lin2_r_vol_to_hpmixr  (0x0<<1)        //Line In 2 Right volume to Headphone Right Mixer
#define HP_Mu_dacr_to_hpmixr        (0x0<<0)        //DAC Right to Headphone Right Mixer (HPMIXR)

//8.15.  Reg-1Ah: AUX Mixer Control

#define AUX_Mu_recmixl_to_auxmixl       (0x1<<13)       //REC Left Mixer to AUX Left Mixer  (AUXMIXL)
#define AUX_Mu_micvol1_to_auxmixl       (0x1<<12)       //MIC1 Volume to AUX Left Mixer (AUXMIXL)
#define AUX_Mu_mu_micvol2_to_auxmixl    (0x1<<11)       //MIC2 Volume to AUX Left Mixer (AUXMIXL)
#define AUX_Mu_lin1_l_vol_to_auxmixl    (0x1<<10)       //Line In 1 Left Volume to AUX Left Mixer (AUXMIXL)
#define AUX_Mu_lin2_l_vol_to_auxmixl    (0x1<<9)        //Line In 2 Left Volume to AUX Left Mixer (AUXMIXL)
#define AUX_Mu_dacl_to_auxmixl          (0x1<<8)        //DAC Left Channel to AUX Left Mixer (AUXMIXL)
#define AUX_Mu_recmixr_to_auxmixr       (0x1<<5)        //REC Right Mixer to AUX Right Mixer (AUXMIXR)
#define AUX_Mu_micvol1_to_auxmixr       (0x1<<4)        //MIC1 Volume to AUX Right Mixer (AUXMIXR)
#define AUX_Mu_mu_micvol2_to_auxmixr    (0x1<<3)        //MIC2 Volume to AUX Right Mixer (AUXMIXR)
#define AUX_Mu_lin1_r_vol_to_auxmixr    (0x1<<2)        //Line In 1 Right Volume to AUX Right Mixe (AUXMIXR)
#define AUX_Mu_lin2_r_vol_to_auxmixr    (0x1<<1)        //Line In 2 Right Volume to AUX Right Mixer (AUXMIXR)
#define AUX_Mu_dacr_to_auxmixr          (0x1<<0)        //DAC Right Channel to AUX Right Mixer (AUXMIXR)

//8.16.  Reg-1Ch: Speaker Mixer Control
#define SPK_VOL_INPUT_SEL_MASK          (0x3<<10)       //Speaker volume input select mask
#define SPK_VOL_INPUT_SEL_VMID          (0x0<<10)       //Speaker volume input select VMID
#define SPK_VOL_INPUT_SEL_SPKMIXER      (0x2<<10)       //Speaker volume input select Speaker Mixer
#define SPK_VOL_INPUT_SEL_AUXMIXER      (0x3<<10)       //Speaker volume input select Aux MIxer
#define M_MIC1_TO_SPKMIXER              (0x1<<7)        //Mute Mic1 to SPKMixer
#define M_MIC2_TO_SPKMIXER              (0x1<<6)        //Mute Mic2 to SPKMixer
#define M_LINEIN1_L_TO_SPKMIXER         (0x1<<5)        //Mute LEFT LINE IN 1 TO SPKMixer
#define M_LINEIN1_R_TO_SPKMIXER         (0x1<<4)        //Mute RIGHT LINE IN 1 TO SPKMixer
#define M_LINEIN2_L_TO_SPKMIXER         (0x1<<3)        //Mute LEFT LINE IN 1 TO SPKMixer
#define M_LINEIN2_R_TO_SPKMIXER         (0x1<<2)        //Mute RIGHT LINE IN 1 TO SPKMixer
#define M_DAC_L_TO_SPKMIXER             (0x1<<1)        //Mute LEFT DAC to SPKMixer
#define M_DAC_R_TO_SPKMIXER             (0x1<<0)        //Mute RIGHT DAC to SPKMixer


//8.17.  Reg-1Eh: Speaker Amplifier Control
#define SPK_AMP_MODE_SEL_MASK           (0x1<<15)       //Speaker Amplifier Mode Control Mask
#define SPK_AMP_MODE_SEL_AB             (0x0<<15)       //Speaker Amplifier MODE SELECT CLASS AB
#define SPK_AMP_MODE_SEL_D              (0x1<<15)       //Speaker Amplifier MODE SELECT CLASS D
#define SPK_CLASS_AB_SEL_MASK           (0x1<<14)       //Class AB Amplifier Dirving Control Mask
#define SPK_CLASS_AB_SEL_STRONG         (0x0<<14)       //Class AB Amplifier Dirving select strong AMP
#define SPK_CLASS_AB_SEL_WEAK           (0x1<<14)       //Class AB Amplifier Dirving select weak AMP

//8.18.  Reg-22h: Microphone Control
#define MIC_BIAS_90_PRECNET_AVDD    1
#define MIC_BIAS_75_PRECNET_AVDD    2

#define MIC1_BOOST_CTRL_MASK        (0xf<<12)
#define MIC1_BOOST_CTRL_BYPASS      (0x0<<12)
#define MIC1_BOOST_CTRL_20DB        (0x1<<12)
#define MIC1_BOOST_CTRL_24DB        (0x2<<12)
#define MIC1_BOOST_CTRL_30DB        (0x3<<12)
#define MIC1_BOOST_CTRL_35DB        (0x4<<12)
#define MIC1_BOOST_CTRL_40DB        (0x5<<12)
#define MIC1_BOOST_CTRL_44DB        (0x6<<12)
#define MIC1_BOOST_CTRL_50DB        (0x7<<12)
#define MIC1_BOOST_CTRL_52DB        (0x8<<12)

#define MIC2_BOOST_CTRL_MASK        (0xf<< 8)
#define MIC2_BOOST_CTRL_BYPASS      (0x0<< 8)
#define MIC2_BOOST_CTRL_20DB        (0x1<< 8)
#define MIC2_BOOST_CTRL_24DB        (0x2<< 8)
#define MIC2_BOOST_CTRL_30DB        (0x3<< 8)
#define MIC2_BOOST_CTRL_35DB        (0x4<< 8)
#define MIC2_BOOST_CTRL_40DB        (0x5<< 8)
#define MIC2_BOOST_CTRL_44DB        (0x6<< 8)
#define MIC2_BOOST_CTRL_50DB        (0x7<< 8)
#define MIC2_BOOST_CTRL_52DB        (0x8<< 8)

#define MICBIAS1_VOLT_CTRL_MASK     (0x1<< 7)
#define MICBIAS1_VOLT_CTRL_90P      (0x0<< 7)
#define MICBIAS1_VOLT_CTRL_75P      (0x1<< 7)

#define MICBIAS1_S_C_DET_MASK           (0x1<< 6)
#define MICBIAS1_S_C_DET_DIS            (0x0<< 6)
#define MICBIAS1_S_C_DET_ENA            (0x1<< 6)

#define MICBIAS1_SHORT_CURR_DET_MASK    (0x3<< 4)
#define MICBIAS1_SHORT_CURR_DET_600UA   (0x0<< 4)
#define MICBIAS1_SHORT_CURR_DET_1500UA  (0x1<< 4)
#define MICBIAS1_SHORT_CURR_DET_2000UA  (0x2<< 4)

//8.19.  Reg-34h: I2S Audio Serial Data Port Control
#define SDP_MODE_SEL_MASK               (0x1<<15)       //Main I2S interface select MASK
#define SDP_MODE_SEL_MASTER             (0x0<<15)       //Main I2S interface select MASTER MODE
#define SDP_MODE_SEL_SLAVE              (0x1<<15)       //Main I2S interface select SLAVE MODE

#define SDP_CP_ADC_MASK                 (0x3<<13)       //
#define SDP_CP_ADC_NORMAL               (0x0<<13)       //Independent
#define SDP_CP_ADC_R_TO_ADC_L           (0x1<<13)       //COPY ADC RIGHT CHANNEL DIGITAL DATA TO LEFT CHANNEL
#define SDP_CP_ADC_L_TO_ADC_R           (0x2<<13)       //COPY ADC LEFT CHANNEL DIGITAL DATA TO RIGHT CHANNEL


#define SDP_ADC_CPS_SEL_MASK            (0x3<<10)       //ADC Compress select Mask
#define SDP_ADC_CPS_SEL_OFF             (0x0<<10)       //ADC Compress select OFF
#define SDP_ADC_CPS_SEL_U_LAW           (0x1<<10)       //ADC Compress select u_law
#define SDP_ADC_CPS_SEL_A_LAW           (0x2<<10)       //ADC Compress select a_law

#define SDP_DAC_CPS_SEL_MASK            (0x3<< 8)       //DAC Compress select Mask
#define SDP_DAC_CPS_SEL_OFF             (0x0<< 8)       //DAC Compress select OFF
#define SDP_DAC_CPS_SEL_U_LAW           (0x1<< 8)       //DAC Compress select u_law
#define SDP_DAC_CPS_SEL_A_LAW           (0x2<< 8)       //DAC Compress select a_law

#define SDP_I2S_BCLK_POL_NORMAL         (0x0<<7)        //0:Normal
#define SDP_I2S_BCLK_POL_INV            (0x1<<7)        //1//:Invert

#define SDP_DAC_R_NORMAL                (0x0<<6)        //0:Normal 1:Invert
#define SDP_DAC_R_INV                   (0x1<<6)        //0:Normal 1:Invert

#define SDP_ADC_DATA_L_R_NORMAL         (0x0<<5)        //0:ADC Digital Data Left/Right Channel Swap NORMAL
#define SDP_ADC_DATA_L_R_SWAP           (0x1<<5)        //1:ADC Digital Data Left/Right Channel Swap SEAP

#define SDP_DAC_DATA_L_R_NORMAL         (0x0<<5)        //0:DAC Digital Data Left/Right Channel Swap NORMAL
#define SDP_DAC_DATA_L_R_SWAP           (0x1<<4)        //1:DAC Digital Data Left/Right Channel Swap SEAP

//Data Length Slection
#define SDP_I2S_DL_MASK                 (0x3<<2)        //Stereo Serial Data Length mask
#define SDP_I2S_DL_16                   (0x0<<2)        //16 bits
#define SDP_I2S_DL_20                   (0x1<<2)        //20 bits
#define SDP_I2S_DL_24                   (0x2<<2)        //24 bits
#define SDP_I2S_DL_8                    (0x3<<2)        //8  bits

//PCM Data Format Selection
#define SDP_I2S_DF_MASK                 (0x3<<0)            //main i2s Data Format mask
#define SDP_I2S_DF_I2S                  (0x0<<0)            //I2S FORMAT
#define SDP_I2S_DF_RIGHT                (0x1<<0)            //Left JUSTIFIED
#define SDP_I2S_DF_LEFT                 (0x2<<0)            //PCM format A
#define SDP_I2S_DF_PCM                  (0x3<<0)            //PCM format B

//8.20.  Reg-38h: Stereo ADC/DAC Clock Control
#define SEL_I2S_PRE_DIV1_MASK   (0x7<<13)
#define SEL_I2S_PRE_DIV1_1      (0x0<<13)    //I2S Pre-Divider
#define SEL_I2S_PRE_DIV1_2      (0x1<<13)
#define SEL_I2S_PRE_DIV1_4      (0x2<<13)
#define SEL_I2S_PRE_DIV1_8      (0x3<<13)
#define SEL_I2S_PRE_DIV1_16     (0x4<<13)
#define SEL_I2S_PRE_DIV1_32     (0x4<<13)

#define SEL_I2S_BCLK_MSL_32     (0x0<<12)   //Master Mode Clock Relative of BCLK and LRCK 64Fs
#define SEL_I2S_BCLK_MSL_16     (0x1<<12)   //32 FS

#define SEL_DAC_OSR_MASK        (0x3<<8)    //Stereo ADC Over Sample Rate Select
#define SEL_DAC_OSR_128FS       (0x0<<8)
#define SEL_DAC_OSR_64FS        (0x1<<8)
#define SEL_DAC_OSR_32FS        (0x2<<8)
#define SEL_DAC_OSR_16FS        (0x3<<8)

#define SEL_FILTER_CLK_256FS        (0x0<<7)    //Stereo ADC/DAC Filter Clock Select
#define SEL_FILTER_CLK_384FS        (0x1<<7)

//8.21.  Reg-3Ah: Power Management Control 1
//Power managment addition 1 (0x3A),0:Disable,1:Enable
#define PWR_MAIN_I2S_EN                 (0x1<<15)
#define PWR_CLASS_D                     (0x1<<13)
#define PWR_ADC_L_CLK                   (0x1<<12)
#define PWR_ADC_R_CLK                   (0x1<<11)
#define PWR_DAC_L_CLK                   (0x1<<10)
#define PWR_DAC_R_CLK                   (0x1<< 9)
#define PWR_DAC_REF                     (0x1<< 8)
#define PWR_DAC_L_TO_MIXER              (0x1<< 7)
#define PWR_DAC_R_TO_MIXER              (0x1<< 6)
#define PWR_CLASS_AB                    (0x1<< 5)

//8.22.  Reg-3Bh: Power Management Control 2
#define PWR_HPMIXER_L                   (0x1<<15)
#define PWR_HPMIXER_R                   (0x1<<14)
#define PWR_AUXMIXER_L                  (0x1<<13)
#define PWR_AUXMIXER_R                  (0x1<<12)
#define PWR_RECMIXER_L                  (0x1<<11)
#define PWR_RECMIXER_R                  (0x1<<10)
#define PWR_LINEIN1_L                   (0x1<< 9)
#define PWR_LINEIN1_R                   (0x1<< 8)
#define PWR_LINEIN2_L                   (0x1<< 7)
#define PWR_LINEIN2_R                   (0x1<< 6)
#define PWR_MIC1_BOOT                   (0x1<< 5)
#define PWR_MIC2_BOOT                   (0x1<< 4)
#define PWR_MICBIAS1                    (0x1<< 3)
#define PWR_MICBIAS2                    (0x1<< 2)
#define PWR_PLL                         (0x1<< 1)
#define PWR_SPKMIXER                    (0x1<< 0)

//8.23.  Reg-3Ch: Power Management Control 3
//Power managment addition 3(0x3C),0:Disable,1:Enable
#define PWR_VREF                (0x1<<15)
#define PWR_DIS_FAST_VREF       (0x1<<14)
#define PWR_MAIN_BIAS           (0x1<<13)
#define PWR_HP                  (0x1<< 3)
#define PWR_VCM_HP              (0x1<< 2)   //Power On Headphone Common-mode Buffer
#define PWR_HP_OUT              (0x1<< 1)   //Change Output Mode for Power on Depop
#define PWR_HP_AMP              (0x1<< 0)   //Improve HP Amp Driving
//8.24.  Reg-3Eh: Power Management Control 4
#define PWR_SPK_L_VOL               (0x1<<15)
#define PWR_SPK_R_VOL               (0x1<<14)
#define PWR_HP_L_VOL                (0x1<<11)
#define PWR_HP_R_VOL                (0x1<<10)
#define PWR_AUXOUT_L_VOL            (0x1<< 9)
#define PWR_AUXOUT_R_VOL            (0x1<< 8)

//8.25.  Reg-40h: General Purpose Control Register 1
#define GPCR_CLSD_AUTO_RATO_EN  (0x1<<15)   //Speaker Amplifier Auto Ratio Gain (AC+DC) Control

#define GPCR_DAC_HPF_BYPASS     (0x0<<11)   // Stereo DAC High Pass Filter Control
#define GPCR_DAC_HPF_NORMAL     (0x1<<11)   //

#define GPCR_ADC_HPF_BYPASS     (0x0<<10)   // Stereo ADC High Pass Filter Control
#define GPCR_ADC_HPF_NORMAL     (0x1<<10)   //

#define ADC_WIND_FILT_COFF_MASK (0x3F<<2)       //Select ADC Wind Filter Clock type

#define ADC_WIND_FILT_CTRL_MASK         (0x3<<0)        //ADC WIND FILTER CTRL MASK
#define ADC_WIND_FILT_CTRL_DISABLE      (0x0<<0)        //ADC WIND FILTER Disable
#define ADC_WIND_FILT_CTRL_1S_HPF       (0x1<<0)        //ADC WIND FILTER CTRL 1 Order HPF
#define ADC_WIND_FILT_CTRL_2S_HPF       (0x3<<0)        //ADC WIND FILTER CTRL 2 Order HPF

//8.26.  Reg-42h: Internal Clock Control
#define SYSCLK_SOUR_SEL_MASK            (0x3<<14)
#define SYSCLK_SOUR_SEL_MCLK            (0x0<<14)       //system Clock source from MCLK
#define SYSCLK_SOUR_SEL_PLL             (0x1<<14)       //system Clock source from PLL

#define PLLCLK_SOUR_SEL_MCLK            (0x0<<13)       //PLL clock source from MCLK
#define PLLCLK_SOUR_SEL_BITCLK          (0x1<<13)       //PLL clock source from BITCLK

#define PLLCLK_PRE_DIV1                 (0x0<<11)           //DIV 1
#define PLLCLK_PRE_DIV2                 (0x1<<11)           //DIV 2

//8.27.  Reg-44h: PLL Function Control
#define PLL_CTRL_M_VAL(m)               ((m)&0xf)           //M code for analog PLL
#define PLL_CTRL_K_VAL(k)               (((k)&0x7)<<4)      //K code for analog PLL
#define PLL_CTRL_N_VAL(n)               (((n)&0xff)<<8)     //N code for analog PLL
#define PLL_M_BYPASS_EN                 (0x0<<7)        //NO BYPASS


//8.28.  Reg-48h: Digital Beep Gen. and IRQ Control

//8.29.  Reg-4Ah: Internal Status and Sticky Control

//8.30.  Reg-4Ch: GPIO Control 1

//8.31.  Reg-4Dh: GPIO Control 2

//8.32.  Reg-52h: General Purpose Control Register 2

//8.33.  Reg-54h: Depop control 1
#define PW_SOFT_GEN                     (0x1<<15)
#define EN_SOFT_FOR_S_M_DEPOP           (0x1<<14)
#define EN_DEPOP_2                      (0x1<<7)
#define EN_HP_L_M_UM_DEPOP              (0x1<<1)
#define EN_HP_R_M_UM_DEPOP              (0x1<<0)

//8.34.  Reg-56h: Depop control 2

//8.35.  Reg-5Ah: Jack Detect Control Register
#define JD_USE_MASK                 (0x7<<13)               //JD Pin select
#define JD_USE_JD2                  (0x5<<13)               //select JD2 from Line1_L
#define JD_USE_JD1                  (0x4<<13)               //select JD1 from Line1_R
#define JD_OFF                      (0x0<<13)               //off

#define JD_HP_EN                    (0x1<<11)               //JD trigger enable for HP

#define JD_HP_TRI_MASK              (0x1<<10)               //Trigger mask
#define JD_HP_TRI_HI                (0x1<<10)               //high trigger
#define JD_HP_TRI_LO                (0x0<<10)               //low trigger


#define JD_SPK_ENA_MASK             (0x3<<8)                //Enable Jack Detect Trigger for SPK
#define JD_SPK_DIS                  (0x0<<8)                //Disable JD for Speaker
#define JD_SPK_ENA_SPK_LP_LN        (0x1<<8)                //Enable Jack Detect Trigger for SPK_LP/LN
#define JD_SPK_ENA_SPK_LP_RP        (0x2<<8)                //Enable Jack Detect Trigger for SPK_LP/RP

#define JD_SPK_TRI_MASK             (0x1<<7)                //Trigger mask
#define JD_SPK_TRI_HI               (0x1<<7)                //high trigger
#define JD_SPK_TRI_LO               (0x0<<7)                //low trigger

#define JD_LINE1_L_SHARE_JD1        (0x1<<3)                //LINE IN1 LEFT PIN SHARE TO JD1
#define JD_LINE1_R_SHARE_JD2        (0x1<<2)                //LINE IN1 RIGHT PIN SHARE TO JD2

#define JD_AUXOUT_EN                (0x1<<1)                //JD trigger enable for AUXOUT

#define JD_AUXOUT_MASK              (0x1<<0)                //Trigger mask
#define JD_AUXOUT_TRI_HI            (0x1<<0)                //high trigger
#define JD_AUXOUT_TRI_LO            (0x0<<0)                //low trigger

//8.36.  Reg-5Ch: Soft Volume Control Register 1

//8.37.  Reg-5Eh: Soft Volume Control Register 2

//8.38.  Reg-64h: ALC Function Control 1
#define ALC_ATTACK_RATE_MASK        (0x1F<<8)               //select ALC attack rate
#define ALC_RECOVERY_RATE_MASK      (0x1F<<0)               //select ALC Recovery rate

//8.39.  Reg-65h: ALC Function Control 2
#define ALC_COM_NOISE_GATE_MASK     (0xF<<0)                //select Compensation gain for Noise gate function

//8.40.  Reg-66h: ALC Function Control 3
#define ALC_ENA_DAC_PATH            (0x0<<15)               //DAC path
#define ALC_ENA_ADC_PATH            (0x1<<15)               //ADC path

#define ALC_ENABLE                  (0x1<<14)               //select ALC path
#define ALC_DISABLE                 (0x0<<14)               //disable


#define ALC_PARA_UPDATE             (0x1<<13)               //update ALC parameter
#define ALC_LIMIT_LEVEL_MASK        (0x1F<<8)               //ALC limit level

#define ALC_NOISE_GATE_FUN_MASK     (0x1<<7)                //ALC noise gate function
#define ALC_NOISE_GATE_FUN_DIS      (0x0<<7)                //disable
#define ALC_NOISE_GATE_FUN_ENA      (0x1<<7)                //enable

#define ALC_NOISE_GATE_H_D_MASK     (0x1<<6)                //ALC noise gate hold data function
#define ALC_NOISE_GATE_H_D_DIS      (0x0<<6)                //disable
#define ALC_NOISE_GATE_H_D_ENA      (0x1<<6)                //enable

//8.41.  Reg-68h: Pseudo Stereo & Spatial Effect Control
#define SPATIAL_CTRL_EN             (0x1<<15)               //enable Spatial effect
#define ALL_PASS_FILTER_EN          (0x1<<14)               //enable all pass filter
#define PSEUDO_STEREO_EN            (0x1<<13)               //enable pseudo stereo block
#define STEREO_EXPENSION_EN         (0x1<<12)               //enable stereo expansion block

#define GAIN1_3D_PARA_MASK          (0x3<<10)               //3D gain1 parameter
#define GAIN1_3D_PARA_1_00          (0x0<<10)               //3D gain1 1.0
#define GAIN1_3D_PARA_1_50          (0x1<<10)               //3D gain1 1.5
#define GAIN1_3D_PARA_2_00          (0x2<<10)               //3D gain1 2.0

#define RATIO1_3D_MASK              (0x3<<8)                //3D ratio1 parameter
#define RATIO1_3D_0_0               (0x0<<8)                //3D ratio1 0.0
#define RATIO1_3D_0_66              (0x1<<8)                //3D ratio1 0.66
#define RATIO1_3D_1_0               (0x2<<8)                //3D ratio1 1.0

#define GAIN2_3D_PARA_MASK          (0x3<<6)                //3D gain2 parameter
#define GAIN2_3D_PARA_1_00          (0x0<<6)                //3D gain2 1.0
#define GAIN2_3D_PARA_1_50          (0x1<<6)                //3D gain2 1.5
#define GAIN2_3D_PARA_2_00          (0x2<<6)                //3D gain2 2.0

#define RATIO2_3D_MASK              (0x3<<4)                //3D ratio2 parameter
#define RATIO2_3D_0_0               (0x0<<4)                //3D ratio2 0.0
#define RATIO2_3D_0_66              (0x1<<4)                //3D ratio2 0.66
#define RATIO2_3D_1_0               (0x2<<4)                //3D ratio2 1.0

#define APF_FUN_SLE_MASK            (0x3<<0)                //select samplerate for all pass filter
#define APF_FUN_SEL_48K             (0x3<<0)                //select 48k
#define APF_FUN_SEL_44_1K           (0x2<<0)                //select 44.1k
#define APF_FUN_SEL_32K             (0x1<<0)                //select 32k
#define APF_FUN_DIS                 (0x0<<0)                //disable

#define SYSCLK_SEL_MCLK             (0x0<<14)               //sysclk source MUX : MCLK
#define SYSCLK_SEL_PLL              (0x1<<14)               //sysclk source MUX : PLL

#endif
