/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   i2s.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_I2S_

#include "DriverInclude.h"

#define I2SRSD(n)     ((n-1) << 8)
#define I2STSD(n)     ((n-1) << 0)


uint32 I2sGetInt(I2S_CHannel nCH)
{
    uint32 flag;
    gI2S_t I2sReg;


    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }
    flag = I2sReg->I2S_INTSR;
    return flag;
}
uint32 I2SGetIntType(I2S_CHannel i2sChNumb)
{
    uint32 dwI2sIntStaus;
    uint event;
    gI2S_t i2sReg;

    switch (i2sChNumb)
    {
        case I2S_CH0:
            i2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            i2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    dwI2sIntStaus = I2sGetInt(i2sChNumb);
    event = 0;

    if((dwI2sIntStaus & INTSR_TX_empty_interrupt_active) != 0
        || (dwI2sIntStaus & INTSR_TX_underrun_interrupt_active) != 0)
    {
        event |= TX_interrupt_active;
    }

    if((dwI2sIntStaus & INTSR_RX_full_interrupt_active) != 0
        || (dwI2sIntStaus & INTSR_RX_overrun_interrupt_active) != 0)
    {
        event |= RX_interrupt_active;
    }

    return event;
}

uint32 I2sGetRxFIFOaddr(I2S_CHannel i2sChNumb)
{
    gI2S_t i2sReg;

    switch (i2sChNumb)
    {
        case I2S_CH0:
            i2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            i2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    return (uint32)(&(i2sReg->I2S_RXDR));
}

uint32 I2sGetTxFIFOaddr(I2S_CHannel i2sChNumb)
{
    gI2S_t i2sReg;

    switch (i2sChNumb)
    {
        case I2S_CH0:
            i2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            i2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    return (uint32)(&(i2sReg->I2S_TXDR));
}

gI2S_t I2sGetCH(I2S_CHannel i2sChNumb)
{
    gI2S_t i2sReg;

    switch (i2sChNumb)
    {
        case I2S_CH0:
            i2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            i2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    return i2sReg;
}

/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SDMADisable(I2S_CHannel nCH,I2S_Start_t DMA_TxRx)
{
    gI2S_t I2sReg;

    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(DMA_TxRx == I2S_START_DMA_TX)
    {
        //disable DMA
        I2sReg->I2S_DMACR &= (~DMA_Transmit_enabled);
    }
    else if (DMA_TxRx == I2S_START_DMA_RX)
    {
        //disable DMA
        I2sReg->I2S_DMACR &= (~DMA_Receive_enabled);                         ;
    }
    else if(DMA_TxRx == I2S_START_DMA_RTX)
    {
        //disable DMA
        I2sReg->I2S_DMACR &= ~(DMA_Transmit_enabled | DMA_Receive_enabled);
    }
    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SDMAEnable(I2S_CHannel nCH,I2S_Start_t DMA_TxRx)
{
    gI2S_t I2sReg;

    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(DMA_TxRx == I2S_START_DMA_TX)
    {
        //enable DMA
        I2sReg->I2S_DMACR |= (0x10 << 0)               //transmit data level
                         |(DMA_Transmit_enabled)    //transmit data enable
                         ;
    }
    else if (DMA_TxRx == I2S_START_DMA_RX)
    {
        //enable DMA
        I2sReg->I2S_DMACR |= (0x10<<16)                //receive data level
                         |(DMA_Receive_enabled)     //receive data enable
                         ;
    }
    else if(DMA_TxRx == I2S_START_DMA_RTX)
    {
        //enable DMA
        I2sReg->I2S_DMACR |= (0x10<<16)                //receive data level
                         |(DMA_Receive_enabled)     //receive data enable
                         |(0x10 << 0)               //transmit data level
                         |(DMA_Transmit_enabled)    //transmit data enable
                         ;
    }
    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SIntDisable(I2S_CHannel nCH,I2S_INT_t INT_TxRx)
{
    gI2S_t I2sReg;

    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(INT_TxRx == I2S_INT_TX)
    {
        //enable INT
        I2sReg->I2S_INTCR &= ~(INTCR_TX_empty_interrupt_enable
                                | INTCR_TX_underrun_interrupt_enable);       // TFT = 0xf;
    }
    else if (INT_TxRx == I2S_INT_RX)
    {
        //enable INT
        I2sReg->I2S_INTCR &= ~(INTCR_RX_full_interrupt_enable
                                | INTCR_RX_overrun_interrupt_enable);
    }
    else
    {
        //enable INT
        I2sReg->I2S_INTCR &= ~(INTCR_RX_full_interrupt_enable
                                | INTCR_RX_overrun_interrupt_enable
                                | INTCR_TX_empty_interrupt_enable
                                | INTCR_TX_underrun_interrupt_enable);
    }
       //DEBUG1("I2s_Reg->I2S_INTCR = 0x%x",I2s_Reg->I2S_INTCR);

    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SIntEnable(I2S_CHannel nCH,I2S_INT_t INT_TxRx)
{
    gI2S_t I2sReg;

    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(INT_TxRx == I2S_INT_TX)
    {
        //enable INT
        I2sReg->I2S_INTCR |= (INTCR_TX_empty_interrupt_enable
                                | INTCR_TX_underrun_interrupt_enable
                                |(0x10 << 4));       // TFT = 0xf;
    }
    else if (INT_TxRx == I2S_INT_RX)
    {
        //enable INT
        I2sReg->I2S_INTCR |= (INTCR_RX_full_interrupt_enable
                                | INTCR_RX_overrun_interrupt_enable)
                                ;
    }
    else
    {
        //enable INT
        I2sReg->I2S_INTCR |= (   INTCR_RX_full_interrupt_enable
                                | INTCR_RX_overrun_interrupt_enable
                                | INTCR_TX_empty_interrupt_enable
                                | INTCR_TX_underrun_interrupt_enable);
    }
    return 0;
}
/*----------------------------------------------------------------------
Name      : I2SInit(I2S_mode_t mode, I2S_Start_t TxOrRx)
Desc      : initializatin
Params    : mode：I2S_MASTER_MODE or I2S_SLAVE_MODE master or slave
Return    : 0 ok，-1 fail。
            fs(kHz)\Fmclk(MHz)	    12	    24.576	12.288	8.192	11.2896
                            8	    1500	3072	1536	1024	--
                            11.025	1088*	--	    --	    --	    1024
                            12	    1000	2048	1024	--	    --
                            16	    750	    1536	768	    512	    --
                            22.05	544*	--	    --	    --	    512
                            24	    500	    1024	512	    --	    --
                            32	    375	    768	    384	    256	    --
                            44.1	272*	--	    --	    --	    256
                            48	    250	    512	    256	    --	    --
                            96	    125	    256	    128	    --	    --
                            192	    62.5	128	    64	    --	    --
                            表格 7  Fmclk与Flrck比值表
            Fs=Fmclk/(RMD*RSD);
            Fs=Fmclk/(TMD*TSD);
            Fmclk is the frequency of mclk
            TMD(RMD) is divider rato of mclk to sclk,TMD(RMD) = Fmclk/Fsclk,TMD(RMD) is even from 2 to 64.
            TSD(RSD) is divider rato of sclk to lrck,TSD(RSD) = Fsclk/Flrck,TSD(RSD) is any number from 32 to 287.
            for keep 32 bit transfer,so the max value of TSD/4 and RSD is needed to bigger than 64(2X32bit).
            the range of TMD(RMD) is get from this,so the smallest drivider is 48.

----------------------------------------------------------------------*/
uint32 I2sSetSampleRate(I2S_CHannel nCH,I2sFS_en_t I2S_FS)
{
    uint32 FS_Regcofig = 0,clock_sel;
    //这里配置I2S CLK来源 : select pll clock ; select frac_div output ; select 12M
    ScuClockGateCtr(CLK_ACODEC_GATE, 1);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 1);     //PCLK ACODEC gating open

    switch (I2S_FS)
    {
        case I2S_FS_8000Hz:
        case I2S_FS_16KHz:
        case I2S_FS_32KHz:
        case I2S_FS_64KHz:
        case I2S_FS_128KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_40960);
            break;
        case I2S_FS_12KHz:
        case I2S_FS_24KHz:
        case I2S_FS_48KHz:
        case I2S_FS_96KHz:
        case I2S_FS_192KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_61440);
            break;
        case I2S_FS_11025Hz:
        case I2S_FS_22050Hz:
        case I2S_FS_44100Hz:
        case I2S_FS_88200Hz:
        case I2S_FS_1764KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_56448);
            break;
        default:
            break;
    }


    switch (I2S_FS)
    {
        case I2S_FS_8000Hz:
            FS_Regcofig = CKR_MDIV_Value_4 | I2SRSD(256) | I2STSD(256);
            break;
        case I2S_FS_16KHz:
            FS_Regcofig = CKR_MDIV_Value_2 | I2SRSD(256) | I2STSD(256);
            break;
        case I2S_FS_32KHz:
            FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(256) | I2STSD(256);
            break;
        case I2S_FS_64KHz:
            FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(128) | I2STSD(128);
            break;
        case I2S_FS_128KHz:
            FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(64) | I2STSD(64);
            break;
//-------------------------------------------------------------------------------------//
        case I2S_FS_12KHz:
            FS_Regcofig = CKR_MDIV_Value_4 | I2SRSD(256) | I2STSD(256);
            break;
        case I2S_FS_24KHz:
            FS_Regcofig = CKR_MDIV_Value_2 | I2SRSD(256) | I2STSD(256);
            break;
        case I2S_FS_48KHz:
            FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(256) | I2STSD(256);
            break;
        case I2S_FS_96KHz:
            FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(128) | I2STSD(128);
            break;
        case I2S_FS_192KHz:
            FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(64) | I2STSD(64);
            break;
//-------------------------------------------------------------------------------------//
        case I2S_FS_11025Hz:
            FS_Regcofig = CKR_MDIV_Value_4 | I2SRSD(256) | I2STSD(256);
            break;
        case I2S_FS_22050Hz:
            FS_Regcofig = CKR_MDIV_Value_2 | I2SRSD(256) | I2STSD(256);
            break;
        case I2S_FS_44100Hz:
            FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(256) | I2STSD(256);
            break;
        case I2S_FS_88200Hz:
            FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(128) | I2STSD(128);
            break;
        case I2S_FS_1764KHz:
            FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(64) | I2STSD(64);
            break;

        default:
            break;
    }

    return FS_Regcofig;
}
/*
--------------------------------------------------------------------------------
  Function name : void DataPortMuxSet(eDataPortIOMux_t data)
  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void I2s0Cs(eI2sCs_t data)
{
    if(data == I2S0_EXT)
    {
       Grf_I2S0_Sel(I2S_SEL_APAD);
    }
    else
    {
       Grf_I2S0_Sel(I2S_SEL_ACODEC);
    }
}


void I2s1Cs(eI2sCs_t data)
{
    if(data == I2S1_PA)
    {
       Grf_I2S1_Sel(I2S_SEL_APAD);
    }
    else
    {
       Grf_I2S1_Sel(I2S_SEL_BPAD);
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SStart_D(I2S_CHannel nCH,I2S_Start_t txRx)
{
    gI2S_t I2sReg;

    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    // modify I2S_DMACR to enable DMA
    // modify I2S_XFER to enable i2s
    // enable/disable/clear interrupt by modify I2S_INTCR&I2S_INTSR
    // read I2S_FIFOLR to get FIFO status

    if((txRx == I2S_START_PIO_TX) || (txRx == I2S_START_DMA_TX))
    {
        I2sReg->I2S_XFER |= TX_transfer_start;
    }
    else if ((txRx == I2S_START_PIO_RX) || (txRx == I2S_START_DMA_RX))
    {
        I2sReg->I2S_XFER |= RX_transfer_start;
    }
    else
    {
        I2sReg->I2S_XFER |= (RTX_start | TX_transfer_start | RX_transfer_start);
    }
    return 0;
}
int32 I2SStart(I2S_CHannel nCH,I2S_Start_t txRx)
{
    // modify I2S_DMACR to enable DMA
    // modify I2S_XFER to enable i2s
    // enable/disable/clear interrupt by modify I2S_INTCR&I2S_INTSR
    // read I2S_FIFOLR to get FIFO status
    I2SDMAEnable(nCH,txRx);
    I2SStart_D(nCH,txRx);

    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : I2sStop(void)
  Author        : yangwenjie
  Description   : stop transfer

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void I2SStop_D(I2S_CHannel nCH,I2S_Start_t txRx)
{
    gI2S_t I2sReg;

    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if((txRx == I2S_START_PIO_TX) || (txRx == I2S_START_DMA_TX))
    {
        I2sReg->I2S_XFER  &= ~(TX_transfer_start);
    }
    else if((txRx == I2S_START_PIO_RX) || (txRx == I2S_START_DMA_RX))
    {
        I2sReg->I2S_XFER  &= ~(RX_transfer_start);
    }
    else
    {
        I2sReg->I2S_XFER  = 0x0;
    }
}

void I2SStop(I2S_CHannel nCH,I2S_Start_t txRx)
{
    I2SStop_D(nCH,txRx);
}

uint32 I2S_PIO_Read(I2S_CHannel nCH,uint32* buffer,uint32 size)
{
    gI2S_t I2sReg;
    uint32 dwRealSize, cnt;

    uint32 RX_FIFOLR;
    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    cnt = size > 16? 16 : size;
    dwRealSize = 0;

    while (cnt--)
    {
         *buffer = *(uint32 *)(&(I2sReg->I2S_RXDR));
         buffer++;
         dwRealSize++;
    }

    return dwRealSize;

}

uint32 I2S_PIO_Write(I2S_CHannel nCH,uint32* buffer,uint32 size)
{
    gI2S_t I2sReg;
    uint32 dwRealSize, cnt;

    uint32 TX_FIFOLR;
    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }
    cnt = size > 16? 16 : size;
    //cnt = size;

    dwRealSize = 0;
    while (cnt--)
    {
        /*
        if(I2s_Reg->I2S_FIFOLR & 0x3f >= 0x1e)
        {
            break;
        }
        */

        *(uint32 *)(&(I2sReg->I2S_TXDR))  = *buffer;
         buffer++;
         dwRealSize++;
    }

    return dwRealSize;

}

int32 I2SInit_D(I2S_CHannel nCH,
              I2S_mode_t I2S_mode,
              I2sFS_en_t I2S_FS,
              eI2sFormat_t BUS_FORMAT,
              eI2sDATA_WIDTH_t I2S_Data_width,
              I2S_BUS_MODE_t I2S_Bus_mode)
{
    uint32 config,FS_Regcofig = 0;
    uint32 timeout = 20000;
    gI2S_t I2sReg;

    switch (nCH)
    {
        case I2S_CH0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_CH1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    I2sReg->I2S_XFER = (TX_transfer_stop |RX_transfer_stop);

    I2sReg->I2S_CLR = CLR_TXC_cleard | CLR_RXC_cleard;
    while(I2sReg->I2S_CLR)
    {
	    if (--timeout == 0)
	    {
            break;
	    }
    }

    //24 bit config
    if(I2S_Data_width == I2S_DATA_WIDTH24)
    {

        I2sReg->I2S_TXCR = ((Valid_Data_width32)            	//Valid data width
                             |(BUS_FORMAT << 5)            		//Tranfer format select, 0:I2S, 1:PCM
                             |(TXCR_PCM_no_delay)            	//PCM bus mode
                             |(I2S_Bus_mode << 9)            		//I2S bus mode
                             |(TXCR_FBM_MSB)           		    //First Bit Mode
                             |(TXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(TXCR_HWT_Bit32)           		//halfword word transform
                             |(TXCR_Channel_0_Enable) 		    //channel select register
                             |(0<<17)						    //RCNT
                             );

        I2sReg->I2S_RXCR = ((Valid_Data_width32)            	//Valid data width
                             |(BUS_FORMAT << 5)            		//Tranfer format select, 0:I2S, 1:PCM
                             |(RXCR_PCM_no_delay)            	//PCM bus mode
                             |(I2S_Bus_mode << 9)            		//I2S bus mode
                             |(RXCR_FBM_MSB)           		    //First Bit Mode
                             |(RXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(RXCR_HWT_Bit32) 			        //halfword word transform
                             );
    }
    else
    {
        //16 bit config
        I2sReg->I2S_TXCR = ((Valid_Data_width16)            	//Valid data width
                             |(BUS_FORMAT << 5)            		//Tranfer format select, 0:I2S, 1:PCM
                             |(TXCR_PCM_no_delay)            	//PCM bus mode
                             |(I2S_Bus_mode << 9)            		//I2S bus mode
                             |(TXCR_FBM_MSB)           		    //First Bit Mode
                             |(TXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(TXCR_HWT_Bit16)           		//halfword word transform
                             |(TXCR_Channel_0_Enable) 		    //channel select register
                             |(0<<17)						    //RCNT
                             );

        I2sReg->I2S_RXCR = ((Valid_Data_width16)            	//Valid data width
                             |(BUS_FORMAT << 5)            		//Tranfer format select, 0:I2S, 1:PCM
                             |(RXCR_PCM_no_delay)            	//PCM bus mode
                             |(I2S_Bus_mode << 9)            		//I2S bus mode
                             |(RXCR_FBM_MSB)           		    //First Bit Mode
                             |(RXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(RXCR_HWT_Bit16) 			        //halfword word transform
                             );
    }

    if(I2S_mode == I2S_SLAVE_MODE)
    {
        I2sReg->I2S_CKR  = ((CKR_TSD_Value)                   //Fsclk/Ftxlrck TSD=187 no care
                             |(CKR_RSD_Value)                   //Fsclk/Frxlrck RSD=124 no care
                             |(CKR_MDIV_Value_0)                //MDIV=6M  Fmclk/Ftxsck-1,panda_FPGA:MCLK=12M
                             |(CKR_TLP_normal)                  //tx lrck polarity
                             |(CKR_RLP_normal)                  //rx lrck polarity
                             |(CKR_CKP_posedge)                 //rx/tx sclk polarity
                             |(CKR_MSS_slave)                   //slave mode
                             );

    }
    else
    {
        //get FS Reg config
        FS_Regcofig = I2sSetSampleRate(nCH,I2S_FS);
        I2sReg->I2S_CKR  = ( FS_Regcofig
                             |(CKR_TLP_normal)                  //rx/tx lrck polarity
                             |(CKR_RLP_normal)
                             |(CKR_CKP_posedge)                 //rx/tx sclk polarity
                             |(CKR_MSS_master)                  //master mode
                             );
    }

    I2sReg->I2S_INTCR = ((INTCR_TX_empty_interrupt_disabled)        // tx empty interrupt. 0:disable,1:enable
                         |(INTCR_TX_underrun_interrupt_disabled)    // tx under run int enable.0:disable,1:enable
                         |(0<<2)                                    // write 1 to clear TX underrun int
                         |(0x10<<4)                                    // transmit FIFO threshold
                         |(INTCR_RX_full_interrupt_disabled)        // RX Full int
                         |(INTCR_RX_overrun_interrupt_disabled)     // RX overrun
                         |(0<<18)                                   // write 1 to clear RX overrun
                         |(0x1f<<20)                                // Reveive FIFO threshold
                         );

    return 0;
}

/*----------------------------------------------------------------------
Name      : I2SInit(I2S_mode_t mode, I2S_Start_t TxOrRx)
Desc      : initializatin
Params    : mode：I2S_MASTER_MODE or I2S_SLAVE_MODE master or slave
Return    : 0 ok，-1 fail。
            fs(kHz)\Fmclk(MHz)	    12	    24.576	12.288	8.192	11.2896
                            8	    1500	3072	1536	1024	--
                            11.025	1088*	--	    --	    --	    1024
                            12	    1000	2048	1024	--	    --
                            16	    750	    1536	768	    512	    --
                            22.05	544*	--	    --	    --	    512
                            24	    500	    1024	512	    --	    --
                            32	    375	    768	    384	    256	    --
                            44.1	272*	--	    --	    --	    256
                            48	    250	    512	    256	    --	    --
                            96	    125	    256	    128	    --	    --
                            192	    62.5	128	    64	    --	    --
                            表格 7  Fmclk与Flrck比值表
            Fs=Fmclk/(RMD*RSD);
            Fs=Fmclk/(TMD*TSD);
            Fmclk is the frequency of mclk
            TMD(RMD) is divider rato of mclk to sclk,TMD(RMD) = Fmclk/Fsclk,TMD(RMD) is even from 2 to 64.
            TSD(RSD) is divider rato of sclk to lrck,TSD(RSD) = Fsclk/Flrck,TSD(RSD) is any number from 32 to 287.
            for keep 32 bit transfer,so the max value of TSD/4 and RSD is needed to bigger than 64(2X32bit).
            the range of TMD(RMD) is get from this,so the smallest drivider is 48.

----------------------------------------------------------------------*/
_ATTR_DRIVERLIB_CODE_
int32 I2SInit(I2S_CHannel nCH,
              I2S_PORTs I2S_Port,
              I2S_mode_t I2S_mode,
              I2sFS_en_t I2S_FS,
              eI2sFormat_t BUS_FORMAT,
              eI2sDATA_WIDTH_t I2S_Data_width,
              I2S_BUS_MODE_t I2S_Bus_mode)
{
    switch (I2S_Port)
    {
        case I2S_CH0_ACODEC:
            I2s0Cs(I2S0_ACODEC);
            break;

        case I2S_CH0_PA:
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin7, IOMUX_GPIO0B7_I2S0_CLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin5, IOMUX_GPIO0B5_I2S0_SCLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin3, IOMUX_GPIO0B3_I2S0_SDI);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin6, IOMUX_GPIO0B6_I2S0_LRCK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin4, IOMUX_GPIO0B4_I2S0_SDO);

            I2s0Cs(I2S0_EXT);
            if (I2S_MASTER_MODE == I2S_mode)
            {
                ScuClockGateCtr(CLK_ACODEC_GATE, 1);
                ScuClockGateCtr(PCLK_ACODEC_GATE, 1);

                Acodec_pll_Source_sel(CLK_ACODEC_PLL);
                SetI2SFreq(I2S_CH0,CLK_ACODEC_PLL,0);
                SetI2SFreq(I2S_CH1,CLK_ACODEC_PLL,0);
                SetI2SClkOutFreq(I2S1_CLK);
            }
            break;

        case I2S_CH1_PA:
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin0, IOMUX_GPIO1A0_I2S1A_CLK);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin2, IOMUX_GPIO1A2_I2S1A_SCLK);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin4, IOMUX_GPIO1A4_I2S1A_SDI);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin1, IOMUX_GPIO1A1_I2S1A_LRCK);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin3, IOMUX_GPIO1A3_I2S1A_SDO);

            I2s1Cs(I2S1_PA);
            //if (I2S_MASTER_MODE == I2S_mode)
            {
                ScuClockGateCtr(CLK_ACODEC_GATE, 1);
                ScuClockGateCtr(PCLK_ACODEC_GATE, 1);

                Acodec_pll_Source_sel(CLK_ACODEC_PLL);
                SetI2SFreq(I2S_CH1,CLK_ACODEC_PLL,0);
                SetI2SClkOutFreq(I2S1_CLK);
            }
            break;

        case I2S_CH1_PB:
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin0, IOMUX_GPIO0A0_I2S1B_CLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin2, IOMUX_GPIO0A2_I2S1B_SCLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin4, IOMUX_GPIO0A4_I2S1B_SDI);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin1, IOMUX_GPIO0A1_I2S1B_LRCK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin3, IOMUX_GPIO0A3_I2S1B_SDO);

            I2s1Cs(I2S1_PB);
            //if (I2S_MASTER_MODE == I2S_mode)
            {
                ScuClockGateCtr(CLK_ACODEC_GATE, 1);
                ScuClockGateCtr(PCLK_ACODEC_GATE, 1);

                Acodec_pll_Source_sel(CLK_ACODEC_PLL);
                SetI2SFreq(I2S_CH1,CLK_ACODEC_PLL,0);
                SetI2SClkOutFreq(I2S1_CLK);
            }
            break;

        default:
            break;
    }

    switch (nCH)
    {
        case I2S_CH0:
            ScuClockGateCtr(CLK_I2S1_SRC_GATE, 1);
            ScuClockGateCtr(CLK_I2S1_OUT_GATE, 1);
            ScuClockGateCtr(CLK_I2S1_GATE, 1);

            ScuClockGateCtr(CLK_I2S0_SRC_GATE, 1);
            ScuClockGateCtr(CLK_I2S_GATE, 1);

            ScuClockGateCtr(PCLK_I2S0_GATE, 1);
            ScuSoftResetCtr(I2S0_SRST, 0);
            break;
        case I2S_CH1:
            ScuClockGateCtr(CLK_I2S1_SRC_GATE, 1);
            ScuClockGateCtr(CLK_I2S1_OUT_GATE, 1);
            ScuClockGateCtr(CLK_I2S1_GATE, 1);

            ScuClockGateCtr(PCLK_I2S1_GATE, 1);
            ScuSoftResetCtr(I2S1_SRST, 0);
            break;
        default:
            break;
    }

    I2SInit_D(nCH,I2S_mode,I2S_FS,BUS_FORMAT,I2S_Data_width,I2S_Bus_mode);

    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : I2S_PowerOnInit(I2S_mode_t mode)
  Author        : yangwenjie
  Description   : I2S auti-initial

  Input         : NULL

  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:        if do not use i2s device,for example,using internal codec,
  			   must call this function after power on.
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void I2SDeInit(I2S_CHannel nCH,I2S_PORTs I2S_Port)
{
    switch (nCH)
    {
        case I2S_CH0:
            ScuSoftResetCtr(I2S0_SRST, 1);

            ScuClockGateCtr(CLK_I2S1_SRC_GATE, 0);
            ScuClockGateCtr(CLK_I2S1_OUT_GATE, 0);
            ScuClockGateCtr(CLK_I2S1_GATE, 0);

            ScuClockGateCtr(CLK_I2S0_SRC_GATE, 0);
            ScuClockGateCtr(CLK_I2S_GATE, 0);

            ScuClockGateCtr(PCLK_I2S0_GATE, 1);
            break;

        case I2S_CH1:
            ScuSoftResetCtr(I2S1_SRST, 1);

            ScuClockGateCtr(CLK_I2S1_SRC_GATE, 0);
            ScuClockGateCtr(CLK_I2S1_OUT_GATE, 0);
            ScuClockGateCtr(CLK_I2S1_GATE, 0);

            ScuClockGateCtr(PCLK_I2S1_GATE, 0);
            break;

        default:
            break;
    }

    switch (I2S_Port)
    {
        case I2S_CH0_ACODEC:
            //I2s0Cs(I2S0_ACODEC);
            break;

        case I2S_CH0_PA:
            //I2s0Cs(I2S0_EXT);

            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin7, IOMUX_GPIO0B7_I2S0_CLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin5, IOMUX_GPIO0B5_I2S0_SCLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin3, IOMUX_GPIO0B3_I2S0_SDI);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin6, IOMUX_GPIO0B6_I2S0_LRCK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin4, IOMUX_GPIO0B4_I2S0_SDO);
            break;

        case I2S_CH1_PA:
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin0, IOMUX_GPIO1A0_I2S1A_CLK);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin2, IOMUX_GPIO1A2_I2S1A_SCLK);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin4, IOMUX_GPIO1A4_I2S1A_SDI);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin1, IOMUX_GPIO1A1_I2S1A_LRCK);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin3, IOMUX_GPIO1A3_I2S1A_SDO);
            break;

        case I2S_CH1_PB:
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin0, IOMUX_GPIO0A0_I2S1B_CLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin2, IOMUX_GPIO0A2_I2S1B_SCLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin4, IOMUX_GPIO0A4_I2S1B_SDI);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin1, IOMUX_GPIO0A1_I2S1B_LRCK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin3, IOMUX_GPIO0A3_I2S1B_SDO);
            break;

        default:
            break;
    }
}
/*
********************************************************************************
*
*                         End of i2s.c
*
********************************************************************************
*/
