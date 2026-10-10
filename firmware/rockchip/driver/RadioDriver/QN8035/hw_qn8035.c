/******************************************************************/

#include "SysInclude.h"

#ifdef _RADIO_

#define _IN_HWQN8035_

#include "hw_qn8035.h"

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
_ATTR_FMDRIVER_QN8035_TEXT_
BOOL Qn8035_Tuner_ReadData(UINT8 reg,UINT8 *pdata, UINT8 size)//2ping add  寄存器数据模式
{
    BOOL ret;
    
    I2C_Init(0,FM5800, 200);//210
    ret = I2C_Read(0,reg, pdata, 1, NormalMode);
    I2C_Deinit(0);

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
_ATTR_FMDRIVER_QN8035_TEXT_
BOOL Qn8035_Tuner_SendCommand(UINT8 reg,UINT8 pdata, UINT8 size)//2ping add  寄存器数据模式
{
    BOOL ret;
    
    I2C_Init(0,FM5800, 200);
    ret = I2C_Write(0,reg, &pdata, 1, NormalMode);
    I2C_Deinit(0);
    
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
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_SetBit(uint8 reg, uint8 bitMask, uint8 data_val)//2 ping add  寄存器数据模式
{
    uint8 temp;
    
    Qn8035_Tuner_ReadData(reg,&temp,1);
    temp &= (uint8)(~bitMask);
    temp |= data_val & bitMask;
    Qn8035_Tuner_SendCommand(reg, temp,1);
    
}

/**************************************************************************************
*   Name    : Qn8035_set_ch
*   Other    : 
*   Function: 
*   Input   : 
*   Return  : 
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
uint8 Qn8035_set_ch(uint16 freq) 
{
    uint8 temp;
    uint16 freqbak;
    
    freqbak = freq;
    freq = FREQ2CHREG(freq);
    
    //writing lower 8 bits of CCA channel start index
    Qn8035_Tuner_SendCommand(CH_START, (uint8)freq,1);
    
    //writing lower 8 bits of CCA channel stop index
    Qn8035_Tuner_SendCommand(CH_STOP, (uint8)freq,1);

    //writing lower 8 bits of channel index
    if(freqbak == 8550)
    {

        Qn8035_Tuner_SendCommand(XTAL_DIV1, QND_XTAL_DIV1_855,1);
        Qn8035_Tuner_SendCommand(XTAL_DIV2, QND_XTAL_DIV2_855,1);
        Qn8035_Tuner_SendCommand(NCO_COMP_VAL, 0x69,1);
        freqbak = 8570;
    }
    else
    {
        Qn8035_Tuner_SendCommand(XTAL_DIV1, qnd_div1,1);
        Qn8035_Tuner_SendCommand(XTAL_DIV2, qnd_div2,1);
        Qn8035_Tuner_SendCommand(NCO_COMP_VAL, qnd_nco,1);
    }   
    
    freqbak = FREQ2CHREG(freqbak);

    //Manually set RX Channel index
    Qn8035_Tuner_SetBit(SYSTEM1, CCA_CH_DIS, CCA_CH_DIS); 
    Qn8035_Tuner_SendCommand(CH, (uint8)freqbak,1);

    //writing higher bits of CCA channel start,stop and step index
    temp = (uint8) ((freqbak >> 8) & CH_CH);
    temp |= ((uint8)(freqbak >> 6) & CH_CH_START);
    temp |= ((uint8) (freqbak >> 4) & CH_CH_STOP);
    temp |= 0x40;//(step << 6);
    Qn8035_Tuner_SendCommand(CH_STEP, temp,1);
    
    return 1;
}

/**************************************************************************************
*   Name    : Qn8035_set_ch
*   Other    : 
*   Function: 
*   Input   : 
*   Return  :
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
uint16 Qn8035_get_ch(void) 
{
    uint8 tCh;
    uint8  tStep; 
    uint16 ch = 0;
    uint16 frq=0;
    
    Qn8035_Tuner_ReadData(CH_STEP,&tStep,1);
    tStep &= CH_CH;
    ch  =  tStep ;
     
    Qn8035_Tuner_ReadData(CH,&tCh,1);
    ch = (ch<<8)+tCh;
    
    frq = CHREG2FREQ(ch);
    
}

/**************************************************************************************
*   Name    : Qn8035_set_initdata
*   Function: fm registers initial
*   Input   : NULL
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_set_initdata(uint8 areaset)
{
    Qn8035_Tuner_SendCommand(0x00, 0x81,1); 
    DelayMs(10);

    //qnd_ChipID = QND_ReadReg(CID1) & 0x03;
    Qn8035_Tuner_ReadData(CID1,&qnd_ChipID,1);
    qnd_ChipID = qnd_ChipID&0x03;
    
    // qnd_IsQN8035B = QND_ReadReg(0x58) & 0x1f;
    Qn8035_Tuner_ReadData(0x58,&qnd_IsQN8035B,1);
    qnd_IsQN8035B = qnd_IsQN8035B&0x1f;
    
    /*********User sets chip working clock **********/
    if(qnd_IsQN8035B == 0x13)
    {
        Qn8035_Tuner_SendCommand(0x58,0x13,1);
        
        //Following is where change the input clock type.as crystal or oscillator.
        Qn8035_Tuner_SetBit(0x58,0x80,QND_INPUT_CLOCK);            
    }
    
    //Following is where change the input clock wave type,as sine-wave or square-wave.
    Qn8035_Tuner_SetBit(0x01,0x80,0x00);
    
    //Following is where change the input clock frequency.
    Qn8035_Tuner_SendCommand(XTAL_DIV0, QND_XTAL_DIV0,1);
    Qn8035_Tuner_SendCommand(XTAL_DIV1, QND_XTAL_DIV1,1);
    Qn8035_Tuner_SendCommand(XTAL_DIV2, QND_XTAL_DIV2,1);    
    DelayMs(10);
    
    /********User sets chip working clock end ********/
    Qn8035_Tuner_SendCommand(0x54, 0x47,1);//mod PLL setting
    Qn8035_Tuner_SendCommand(SMP_HLD_THRD, 0xc4,1);//select SNR as filter3,SM step is 2db    
    Qn8035_Tuner_SetBit(0x40,0x70,0x70);
    Qn8035_Tuner_SendCommand(0x33, 0x9c,1);//set HCC Hystersis to 5db
    Qn8035_Tuner_SendCommand(0x2d, 0xd6,1);//notch filter threshold adjusting
    Qn8035_Tuner_SendCommand(0x43, 0x10,1);//notch filter threshold enable    
    Qn8035_Tuner_SetBit(SMSTART,0x7f,SMSTART_VAL);
    Qn8035_Tuner_SetBit(SNCSTART,0x7f,SNCSTART_VAL);
    Qn8035_Tuner_SetBit(HCCSTART,0x7f,HCCSTART_VAL);
    
    if(qnd_ChipID == CHIPSUBID_QN8035A1)
    {
        Qn8035_Tuner_SetBit(0x47,0x0c,0x08); 
    }   
    
    //these variables are used in Qn8035_set_ch() function.
    Qn8035_Tuner_ReadData(XTAL_DIV1,&qnd_div1,1);
    Qn8035_Tuner_ReadData(XTAL_DIV2,&qnd_div2,1);
    Qn8035_Tuner_ReadData(NCO_COMP_VAL,&qnd_nco,1);

}

/**************************************************************************************
*   Name    : Tuner_Enable
*   Function: Enable FM chip (TEA5800).
*   Input   : area  1:japan 0: china/us
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_Enable(UINT8 area)//ping add 
{
    //FMarea=area;
    Qn8035_set_initdata(area);
}

/**************************************************************************************
*   Name    : Tuner_Disable
*   Function: Disable FM chip (TEA5807)
*   Input   : NULL
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_Disable(void)
{
    Qn8035_Tuner_SetBit(REG_DAC, 0x08, 0x00);    //make sure that Power down control by FSM control
    Qn8035_Tuner_SetBit(SYSTEM1, R_TXRX_MASK, STNBY);  
}

/**************************************************************************************
*   Name    : Tuner_PowerOnInit
*   Function: FM chip (TEA5807) power on initialization.
*   Input   : NULL
*   Output  : null
*   Return  : NULL
*   explain ：frist close FM chips before power on. 
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_PowerOnInit(void)
{
    Qn8035_Tuner_Disable();
}

/**************************************************************************************
*   Name    : Tuner_SetStereo
*   Function: Fm Radio Stereo Setting
*   输入参数：bStereo  0 ST  1MO
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_SetStereo(BOOL bStereo)//ping add fm3
{

}

/**************************************************************************************
*   Name    : Tuner_SetMute
*   Function: Fm Radio Mute Setting
*   Input   :
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_SetMute(BOOL  mute)
{
    if (mute)
    {
        Qn8035_Tuner_SetBit(0x14,0x80,0x80);
        Qn8035_Tuner_SetBit(0x4a, 0x30, 0x30);        
    }
    else
    {        
        Qn8035_Tuner_SetBit(0x4a, 0x30, 0x10);
        Qn8035_Tuner_SetBit(0x14,0x80,0x00);
    }
    
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
_ATTR_FMDRIVER_QN8035_TEXT_
BOOLEAN Qn8035_GetStereoStatus(void)
{
    uint8 result=1;
    uint8 stereoCount,readCnt,snr;

    Qn8035_Tuner_ReadData(SNR,&snr,1);
    
    if(snr> 25) 
    {
        result=0;
    }
    for(readCnt=10;readCnt>0;readCnt--)
    {
        Qn8035_Tuner_ReadData(STATUS1,&stereoCount,1);
        stereoCount+=((stereoCount&ST_MO_RX)? 0:1);
        
        if(stereoCount >= 6) 
            result=0;
    }
    
    return result;
}

/**************************************************************************************
*   Name    : Tuner_PowerOffDeinit
*   Function: Fm Radio Power Off Deinit
*   Input   : NULL
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_PowerOffDeinit(void)////ping add fm4
{
    Qn8035_Tuner_Disable();
}

/**************************************************************************************
*   Name    : Tuner_SetFrequency
*   Function: set FM chip(TEA5800) frequency.
*   Input   : n10KHz -- frequency(unit:10KHz)
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_SetFrequency(UINT32 n10KHz, UINT8 HILO, BOOL ForceMono,UINT16 Area)//ping add fm2
{
    UINT8  writeData8[4];
    UINT16 Freq = 0;
    uint8 result;
    uint8 reg;
    uint8 imrFlag=0;
   
    Freq =(UINT16)n10KHz;
    
    Qn8035_Tuner_SendCommand(0x4a,  0x30 ,1); 
    Qn8035_Tuner_SendCommand(REG_REF,0x7a,1);
    
    /********** QNF_RXInit ****************/
    Qn8035_Tuner_SetBit(0x1b,0x08,0x00);  //Let NFILT adjust freely
    Qn8035_Tuner_SetBit(0x2c,0x3f,0x12);
    Qn8035_Tuner_SetBit(0x1d,0x40,0x00);
    Qn8035_Tuner_SetBit(0x41,0x0f,0x0a);
    Qn8035_Tuner_SendCommand(0x45,0x50,1);
    Qn8035_Tuner_SetBit(0x3e,0x80,0x80);
    Qn8035_Tuner_SetBit(0x41,0xe0,0xc0);
    if(qnd_ChipID == CHIPSUBID_QN8035A0)
    {
        Qn8035_Tuner_SetBit(0x42,0x10,0x10);
    }
    /********** End of QNF_RXInit ****************/

    if(qnd_IsQN8035B == 0x13)
    {
        if((Freq==7630)||(Freq==8580)||(Freq==9340)||(Freq==9390)||(Freq==9530)||(Freq==9980)||(Freq==10480))
        {
            imrFlag = 1;
        }
    }   
    else if((qnd_ChipID == CHIPSUBID_QN8035A0)||(qnd_ChipID == CHIPSUBID_QN8035A1))
    {
        if((Freq==6910)||(Freq==7290)||(Freq==8430))
        {
            imrFlag = 1;
        }        
        else if(qnd_ChipID == CHIPSUBID_QN8035A1)
        {
            if((Freq==7860) || (Freq==10710))
            {
                imrFlag = 1;
            }
        }       
    }   
    
    if(imrFlag)
    {
        Qn8035_Tuner_SetBit(CCA, IMR, IMR);
    }
    else
    {
        Qn8035_Tuner_SetBit(CCA, IMR, 0x00);  
    } 
    Qn8035_set_ch(Freq);
    
    //enable CCA mode with user write into frequency
    Qn8035_Tuner_SendCommand(0x00, 0x13,1);
  
    //Auto tuning
    Qn8035_Tuner_SendCommand(0x4f, 0x80,1);
    Qn8035_Tuner_ReadData(0x4f,&reg,1);
    reg >>= 1;
    Qn8035_Tuner_SendCommand(0x4f, reg,1);
  
    DelayMs(200);
    Qn8035_Tuner_SendCommand(0x4a,  0x10,1); 
    
    ///decrease reference PLL charge pump current.
    //Qn8035_Tuner_SendCommand(REG_REF,0x70,1);
    Qn8035_Tuner_SetBit(REG_REF,ICPREF,0x00);
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
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_MuteControl(bool active)//ping add fm7
{
    Qn8035_Tuner_SetMute(active);
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
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_PowerDown(void)//ping add fm6
{
    Qn8035_Tuner_PowerOffDeinit();
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
_ATTR_FMDRIVER_QN8035_TEXT_
void QND_RXSetTH(UINT8 th) 
{   
    UINT8 rssiTH;
    UINT8 snrTH;                                  
    UINT16 rssi_snr_TH;
    UINT16 rssi_snr_TH_tbl [10];
    #if 0
    UINT16 rssi_snr_TH_tbl [10] = { CCA_SENSITIVITY_LEVEL_0,CCA_SENSITIVITY_LEVEL_1,
                                    CCA_SENSITIVITY_LEVEL_2,CCA_SENSITIVITY_LEVEL_3,
                                    CCA_SENSITIVITY_LEVEL_4,CCA_SENSITIVITY_LEVEL_5,
                                    CCA_SENSITIVITY_LEVEL_6,CCA_SENSITIVITY_LEVEL_7,
                                    CCA_SENSITIVITY_LEVEL_8,CCA_SENSITIVITY_LEVEL_9                                    
                                  };
    #endif

    rssi_snr_TH =CCA_SENSITIVITY_LEVEL_6;// rssi_snr_TH_tbl[th];
    rssiTH = (UINT8) (rssi_snr_TH >> 8);
    snrTH = (UINT8) (rssi_snr_TH & 0xff);
    
    Qn8035_Tuner_SendCommand(0x4f, 0x00,1);//enable auto tunning in CCA mode
    Qn8035_Tuner_SetBit(REG_REF,ICPREF,0x0a);
    Qn8035_Tuner_SetBit(GAIN_SEL,0x08,0x08);//NFILT program is enabled 
    
    //selection filter:filter3
    Qn8035_Tuner_SetBit(CCA1,0x30,0x30);
    
    //Enable the channel condition filter3 adaptation,Let ccfilter3 adjust freely  
    Qn8035_Tuner_SetBit(SYSTEM_CTL2,0x40,0x00);
    Qn8035_Tuner_SendCommand(PLT1,0x00,1);  
    Qn8035_Tuner_SendCommand(CCA_CNT1,0x00,1);
    Qn8035_Tuner_SetBit(CCA_CNT2,0x3f,0x03);
    
    //selection the time of CCA FSM wait SNR calculator to settle:20ms
    //0x00:     20ms(default)
    //0x40:     40ms
    //0x80:     60ms
    //0xC0:     100m
    Qn8035_Tuner_SetBit(CCA_SNR_TH_1 , 0xc0, 0x00);  
    
    //selection the time of CCA FSM wait RF front end and AGC to settle:20ms
    //0x00:     10ms
    //0x40:     20ms(default)
    //0x80:     40ms
    //0xC0:     60ms                            
    Qn8035_Tuner_SetBit(CCA_SNR_TH_2, 0xc0, 0x40);  
    
    Qn8035_Tuner_SetBit(CCA, 0x3f, rssiTH);  //setting RSSI threshold for CCA
    Qn8035_Tuner_SetBit(CCA_SNR_TH_1 , 0x3f, snrTH); //setting SNR threshold for CCA
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
_ATTR_FMDRIVER_QN8035_TEXT_
uint8 QND_RXValidCH(uint16 freq, uint8 step,uint8 db,uint8 Initing) 
{

    uint8 regValue;
    uint8 timeOut;
    uint8 isValidChannelFlag;
    uint8 snr;
    uint8 readCnt,stereoCount=0;
    
    step = step;
    db   = db;
    Initing = Initing;
    
    QND_RXSetTH(9); 
    Qn8035_set_ch(freq);
    
    #if USING_INDUCTOR
    //Auto tuning
    Qn8035_Tuner_SendCommand(0x4f, 0x80,1);
    Qn8035_Tuner_ReadData(0x4f,&regValue,1);
    regValue = (regValue >> 1);
    Qn8035_Tuner_SendCommand(0x4f, regValue,1);
    #endif 
    
    timeOut = 20;  // time out is 100ms
    Qn8035_Tuner_SetBit(SYSTEM1,RXCCA_MASK,RX_CCA);
    while(1)
    {
        DelayMs(5);
        //if it seeks a potential channel, the loop will be quited     
        Qn8035_Tuner_ReadData(SYSTEM1,&regValue,1);
        if((regValue & CHSC) == 0) 
        {
            break;
        }

        //if it was time out,chip would not normally work.
        if((timeOut--) == 0) 
        {
            return -1;
        }
    }
    
    //reading out the rxcca_fail flag of RXCCA status
    Qn8035_Tuner_ReadData(STATUS1,&isValidChannelFlag,1);
    isValidChannelFlag=isValidChannelFlag& RXCCA_FAIL;
    isValidChannelFlag=(isValidChannelFlag ? 0:1);  
    Qn8035_get_ch();  
    
    if(isValidChannelFlag)    
    {
        Qn8035_Tuner_ReadData(SNR,&snr,1);
        DelayMs(60);//QND_Delay(CCA_PILOT_READ_DELAY);    
        
        if(snr <= CCA_PILOT_SNR_FILTER)  
        {
            isValidChannelFlag = 0;  
            for(readCnt=0;readCnt<CCA_PILOT_READ_COUNT;readCnt++)
            {
                DelayMs(CCA_PILOT_READ_COUNT_DELAY);  
                Qn8035_Tuner_ReadData(STATUS1,&stereoCount,1);
                
                stereoCount+= ((stereoCount & ST_MO_RX) ? 0:1);
                if(stereoCount >= CCA_PILOT_TH) 
                {
                    isValidChannelFlag = 1;
                    break;
                }
            }           
        }
    }
    if(isValidChannelFlag)
    {
        return 0;   
    }
    else
    {
        return 1;
    }       
}

/**************************************************************************************
*   Name    : FMSearchByHand
*   Function: search FM station by handle.
*   Input   : direct -- 1(increment), 0(decrease)
*   Output  : FM_FoundStation 
*   Return  : result
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
UINT16 Qn8035_Tuner_SearchByHand(UINT16 direct, UINT32 *FmFreq)//ping add fm5
{
    uint8 result;
    uint16 startFreq,endFreq;
    
    startFreq=*FmFreq;
    result=QND_RXValidCH(startFreq,QND_FSTEP_100KHZ,8,TRUE);

    if (result == 1)
    {
      return 0;   //FM_SearchFail; found no valid station.
    }
    else if(result == 0)
    {
       return 2;   //found a valid station
    }
    else
    {
        return 1;
    }
}


/**************************************************************************************
*   Name    : Tuner_SetInitArea
*   Function: set FM chip(TEA5767) frequency.
*   Input   :
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_SetInitArea(UINT8 area)//ping add fm1
{
    FMarea = area;
    
    if(RADIO_JAPAN == area)
    {
        Qn8035_set_initdata(1);
    }
    else
    {
        Qn8035_set_initdata(0);
    }
}

/**************************************************************************************
*   Name    : Qn5807_Tuner_Vol_Set
*   Function: set FM chip(TEA5807) gain.
*   Input   :
*   Output  : null
*   Return  : NULL
*   explain :
**************************************************************************************/
_ATTR_FMDRIVERL_QN8035_DATA_  uint8 Qn8035_Vol_Table[MAX_VOLUME + 3]=
{
    //+:-6DB
    ((0x05 << 3) + 0x00), // 2,   // 00     // 101, 000  -48 + -6  = -54
    ((0x01 << 3) + 0x00), // 2,   // 01     // 001, 000  -43 + -6  = -49
    ((0x00 << 3) + 0x01), // 2,   // 02     // 000, 001  -36 + -6  = -42
    ((0x00 << 3) + 0x02), // 2,   // 03     // 000, 010  -30 + -6  = -36 
    ((0x01 << 3) + 0x03), // 2,   // 04     // 001, 011  -25 + -6  = -31 
    ((0x02 << 3) + 0x04), // 2,   // 05     // 010, 100  -20 + -6  = -26 
    ((0x04 << 3) + 0x05), // 2,   // 06     // 100, 101  -16 + -6  = -22 
    ((0x00 << 3) + 0x05), // 2,   // 07     // 000, 101  -12 + -6  = -18
    ((0x03 << 3) + 0x06), // 2,   // 08     // 011, 110  -9  + -6  = -15
    ((0x00 << 3) + 0x06), // 2,   // 09     // 000, 110  -6  + -6  = -12
    ((0x04 << 3) + 0x07), // 2,   // 10     // 100, 111  -4  + -6  = -10
    ((0x02 << 3) + 0x07), // 2,   // 11     // 010, 111  -2  + -6  = -8
    ((0x00 << 3) + 0x07), // 2,   // 12     // 000, 111  0db + -6  = -6   
                                               
    //+: 0DB                                   
    ((0x05 << 3) + 0x07), // 0,   // 13     // 101, 111  -5  + 0  = -5
    ((0x04 << 3) + 0x07), // 0,   // 14     // 100, 111  -4  + 0  = -4
    ((0x03 << 3) + 0x07), // 0,   // 15     // 011, 111  -3  + 0  = -3
    ((0x02 << 3) + 0x07), // 0,   // 16     // 010, 111  -2  + 0  = -2
    ((0x01 << 3) + 0x07), // 0,   // 17     // 001, 111  -1  + 0  = -1
    ((0x00 << 3) + 0x07), // 0,   // 18     // 000, 111  0db + 0  = 0
                                               
    //+: +6DB                                  
    ((0x05 << 3) + 0x07), // 1,   // 19     // 101, 111  -5  + 6  = 1
    ((0x04 << 3) + 0x07), // 1,   // 20     // 100, 111  -4  + 6  = 2
    ((0x03 << 3) + 0x07), // 1,   // 21     // 011, 111  -3  + 6  = 3
    ((0x02 << 3) + 0x07), // 1,   // 22     // 010, 111  -2  + 6  = 4
    ((0x01 << 3) + 0x07), // 1,   // 23     // 001, 111  -1  + 6  = 5
    ((0x00 << 3) + 0x07), // 1,   // 24     // 000, 111  0db + 6  = 6
                                               
    //+: +14DB                                 
    ((0x01 << 3) + 0x06), // 3,   // 25     // 001, 110  -7  + 14 = 7
    ((0x00 << 3) + 0x06), // 3,   // 26     // 000, 110  -6  + 14 = 8
    ((0x05 << 3) + 0x07), // 3,   // 27     // 101, 111  -5  + 14 = 9
    ((0x04 << 3) + 0x07), // 3,   // 28     // 100, 111  -4  + 14 = 10
    ((0x03 << 3) + 0x07), // 3,   // 29     // 011, 111  -3  + 14 = 11
    ((0x02 << 3) + 0x07), // 3,   // 30     // 010, 111  -2  + 14 = 12
    ((0x01 << 3) + 0x07), // 3,   // 31     // 001, 111  -1  + 14 = 13
    ((0x00 << 3) + 0x07), // 3,   // 32     // 000, 111  0db + 14 = 14 
};

_ATTR_FMDRIVER_QN8035_TEXT_
void Qn8035_Tuner_Vol_Set(UINT8 gain)
{
    Qn8035_Tuner_SetBit(0x14,0x3f,Qn8035_Vol_Table[gain]);
}

//#endif
#endif

