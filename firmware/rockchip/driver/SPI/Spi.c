/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:   spi.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-15          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_SPI_


#include "DriverInclude.h"
uint32 g_spimFreq;

_ATTR_DRIVER_DATA_ pFunc SpiDmaCallBack = NULL;

_ATTR_DRIVERLIB_CODE_
pSPI_REG SpiGetCH(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    switch (spiChNumb)
    {
        case SPI_CH0:
            spiReg = (pSPI_REG)SPI0_BASE;
            break;
        case SPI_CH1:
            spiReg = (pSPI_REG)SPI1_BASE;
            break;
        default:
            break;
    }

    return spiReg;
}

_ATTR_DRIVERLIB_CODE_
void SPIClearAllInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    //spiReg->SPI_ICR = 0;
    spiReg->SPI_ICR = 0xFF;
}

_ATTR_DRIVERLIB_CODE_
void SpiWaitIdle(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }

    while(spiReg->SPI_SR & 0X01);
}

_ATTR_DRIVERLIB_CODE_
uint32 SpiGetInt(eSPI_ch_t spiChNumb)
{
    uint32 flag;
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return 0;
    }
    flag = spiReg->SPI_ISR;

    //spiReg->SPI_ICR = 0;

    return flag;
}

_ATTR_DRIVERLIB_CODE_
uint32 SpiGetRxFIFOaddr(eSPI_ch_t spiChNumb)
{
    pSPI_REG SpiReg;

    switch (spiChNumb)
    {
        case SPI_CH0:
            SpiReg = (pSPI_REG)SPI0_BASE;
            break;
        case SPI_CH1:
            SpiReg = (pSPI_REG)SPI1_BASE;
        default:
            break;
    }

    return (uint32)(&(SpiReg->SPI_RXDR));
}

_ATTR_DRIVERLIB_CODE_
uint32 SpiGetTxFIFOaddr(eSPI_ch_t spiChNumb)
{
    pSPI_REG SpiReg;

    switch (spiChNumb)
    {
        case SPI_CH0:
            SpiReg = (pSPI_REG)SPI0_BASE;
            break;
        case SPI_CH1:
            SpiReg = (pSPI_REG)SPI1_BASE;
            break;
        default:
            break;
    }

    return (uint32)(&(SpiReg->SPI_TXDR));
}

_ATTR_DRIVERLIB_CODE_
void SpiEnableTxInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_IMR |= SPI_TX_FIFO_EMPTY;

}

_ATTR_DRIVERLIB_CODE_
void SpiDisalbeTxInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_IMR &= ~SPI_TX_FIFO_EMPTY;
}

_ATTR_DRIVERLIB_CODE_
void SpiEnableRxInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_IMR |= SPI_RX_FIFO_FULL;// | (1 << 3) | (1 << 2);
}

_ATTR_DRIVERLIB_CODE_
void SpiDisalbeRxInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_IMR &= ~SPI_RX_FIFO_FULL;
}


_ATTR_DRIVERLIB_CODE_
uint32 SpiEnableChannel(eSPI_ch_t spiChNumb,uint32 ch)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return 0;
    }
    spiReg->SPI_SER = ((uint32)(0x01) << ch);
}

_ATTR_DRIVERLIB_CODE_
uint32 SpiDisableChannel(eSPI_ch_t spiChNumb,uint32 ch)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return 0;
    }
    spiReg->SPI_SER  &= ~((uint32)(0x01) << ch);
}

_ATTR_DRIVERLIB_CODE_
void SPI_GPIO_Init(eSPI_IO_MUX spiChIoMux)
{
    switch(spiChIoMux)
    {
        case SPI_CH0_PA:
            Grf_SPI0_Sel(SPI_SEL_A);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin0, IOMUX_GPIO0C0_SPI0A_TX);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin2, IOMUX_GPIO0C2_SPI0A_CLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin3, IOMUX_GPIO0C3_SPI0A_CS);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin1, IOMUX_GPIO0C1_SPI0A_RX);
            break;

        case SPI_CH0_PB:
            Grf_SPI0_Sel(SPI_SEL_B);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin2, IOMUX_GPIO2B2_SPI0B_TX);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin1, IOMUX_GPIO2B1_SPI0B_CLK);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin0, IOMUX_GPIO2B0_SPI0B_CS);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin3, IOMUX_GPIO2B3_SPI0B_RX);
            break;

        case SPI_CH1_PA:
            Grf_SPI1_Sel(SPI_SEL_A);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin0, IOMUX_GPIO1B0_SPI1A_TX);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin6, IOMUX_GPIO1A6_SPI1A_CLK);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin5, IOMUX_GPIO1A5_SPI1A_CS);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin7, IOMUX_GPIO1A7_SPI1A_RX);
            break;

        case SPI_CH1_PB:
            Grf_SPI1_Sel(SPI_SEL_B);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin1, IOMUX_GPIO2C1_SPI1B_TX);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin0, IOMUX_GPIO2C0_SPI1B_CLK);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin6, IOMUX_GPIO2B6_SPI1B_CS);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin7, IOMUX_GPIO2B7_SPI1B_RX);
            break;

        default:
            break;
    }
}

_ATTR_DRIVERLIB_CODE_
void SPI_GPIO_DeInit(eSPI_IO_MUX spiChIoMux)
{
    switch(spiChIoMux)
    {
        case SPI_CH0_PA:
            ScuClockGateCtr(CLK_SPI0_GATE, 0);
            ScuClockGateCtr(PCLK_SPI0_GATE, 0);
            Grf_SPI0_Sel(SPI_SEL_A);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin0, IOMUX_GPIO0C0_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin2, IOMUX_GPIO0C2_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin3, IOMUX_GPIO0C3_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin1, IOMUX_GPIO0C1_IO);
            break;

        case SPI_CH0_PB:
            ScuClockGateCtr(CLK_SPI0_GATE, 0);
            ScuClockGateCtr(PCLK_SPI0_GATE, 0);
            Grf_SPI0_Sel(SPI_SEL_B);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin2, IOMUX_GPIO2B2_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin1, IOMUX_GPIO2B1_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin0, IOMUX_GPIO2B0_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin3, IOMUX_GPIO2B3_IO);
            break;

        case SPI_CH1_PA:
            ScuClockGateCtr(CLK_SPI1_GATE, 0);
            ScuClockGateCtr(PCLK_SPI1_GATE, 0);
            Grf_SPI1_Sel(SPI_SEL_A);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin0, IOMUX_GPIO1B0_IO);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin6, IOMUX_GPIO1A6_IO);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin5, IOMUX_GPIO1A5_IO);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin7, IOMUX_GPIO1A7_IO);
            break;

        case SPI_CH1_PB:
            ScuClockGateCtr(CLK_SPI1_GATE, 0);
            ScuClockGateCtr(PCLK_SPI1_GATE, 0);
            Grf_SPI1_Sel(SPI_SEL_B);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin1, IOMUX_GPIO2C1_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin0, IOMUX_GPIO2C0_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin6, IOMUX_GPIO2B6_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin7, IOMUX_GPIO2B7_IO);
            break;

        default:
            break;
    }
}
/*
--------------------------------------------------------------------------------
  Function name : int32 SPIWrite(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size)
  Author        :
  Description   :

  Input         : baudRate:   unit:HZ
                  ch£º        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:
--------------------------------------------------------------------------------*/
_ATTR_DRIVERLIB_CODE_
void SPIInit(eSPI_ch_t spiChNumb,eSPI_cs_t slaveNumb, uint32 baudRate, uint32 CtrMode)
{
    int8 ret=0;
    uint32 APBnKHz;
    pSPI_REG spiReg;
    uint32 clk;
    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }

    g_spimFreq = baudRate;

    if (spiReg->SPI_SER)
    {
        return;
    }

    clk  = 24;  //FOR TEST

    //clk = GetSpiFreq();
    //if ((baudRate > 12*1000*1000) && (clk > 24))
    if ((baudRate > 12*1000*1000)&& (clk > 24))
    {
        //Need Configer spi clk to other
        //clk = baudRate * 2;
        //SetSPIFreq(spiChNumb,PLL_MUX_CLK,clk);
        spiReg->SPI_BAUDR = clk * 1000 * 1000 / baudRate;
    }
    else
    {
        //Need Configer spi clk to 24M
        //SetSPIFreq(spiChNumb,XIN24M,24 * 1000 * 1000);

        spiReg->SPI_BAUDR = 24 * 1000 * 1000 / baudRate;
    }

    spiReg->SPI_CTRLR0 = CtrMode;
    //spiReg->SPI_SER    = ((uint32)(0x01) << slaveNumb);

}
/*
--------------------------------------------------------------------------------
  Function name : int32 SPIWrite(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size)
  Author        :
  Description   :

  Input         : baudRate:   unit:HZ
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void SPIDeInit(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_ENR = 0;
    spiReg->SPI_SER = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : int32 SPIPioWriteEnable(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size)
  Author        :
  Description   :

  Input         : baudRate:   ¨°?Hz?a¦Ì£¤??
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
rk_err_t SPIPioWriteEnable(eSPI_ch_t spiChNumb,eSPI_TRANSFER_MODE_t  transfer_mode)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return ERROR;
    }
    spiReg->SPI_ENR = SPI_DISABLE;
    //spiReg->SPI_CTRLR0 &= ~(TRANSMIT_RECEIVE_MASK);
    //spiReg->SPI_CTRLR0 |= TRANSMIT_RECEIVE;
    spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(3ul<<18) | TRANSMIT_ONLY;
    spiReg->SPI_CTRLR1 = 0x1f;

    spiReg->SPI_ENR = SPI_ENABLE;

    return OK;
}

_ATTR_DRIVERLIB_CODE_
rk_size_t SPIWriteFIFO0(eSPI_ch_t spiChNumb,uint8 *pdata, uint32 Cnt)
{
    rk_size_t dwRealSize;
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
        return ERROR;
    }

    //DEBUG(" 00 FIFO Contain data : %d",spiReg->SPI_TXFTLR);
    dwRealSize = 0;
    while (Cnt--)
    {
        if ((spiReg->SPI_SR & TRANSMIT_FIFO_FULL) != TRANSMIT_FIFO_FULL)
        {
             spiReg->SPI_TXDR[0] = *pdata++;
             //DelayMs(5);
             dwRealSize++;
        }
        else
            break;
    }

    //DEBUG(" 11 FIFO Contain data : %d",spiReg->SPI_TXFTLR);

    return dwRealSize;
}

_ATTR_DRIVERLIB_CODE_
rk_size_t SPIWriteFIFO(eSPI_ch_t spiChNumb,uint8 *pdata, uint32 size)
{
     int32  totalsize;
     pSPI_REG spiReg;

     if(spiChNumb == SPI_CH0)
     {
         spiReg = (pSPI_REG) SPI0_BASE;
     }
     else if(spiChNumb == SPI_CH1)
     {
         spiReg = (pSPI_REG) SPI1_BASE;
     }
     else
     {
        return ERROR;
     }
     totalsize = size;

     while (size)
     {
         if ((spiReg->SPI_SR & TRANSMIT_FIFO_FULL) != TRANSMIT_FIFO_FULL)
         {
             spiReg->SPI_TXDR[0] = *pdata++;
             size--;
         }
         else
         {
             break;
         }
     }

    return (totalsize - size);
}


/*
--------------------------------------------------------------------------------
  Function name : int32 SPIRead(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size)
  Author        :
  Description   :

  Input         : baudRate:   unit:HZ
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/

_ATTR_DRIVERLIB_CODE_
rk_err_t SPIPioReadEnable(eSPI_ch_t spiChNumb,uint32 size)
{
     pSPI_REG spiReg;

     if(spiChNumb == SPI_CH0)
     {
         spiReg = (pSPI_REG) SPI0_BASE;
     }
     else if(spiChNumb == SPI_CH1)
     {
         spiReg = (pSPI_REG) SPI1_BASE;
     }
     else
     {
        return ERROR;
     }
    spiReg->SPI_ENR = SPI_DISABLE;
    //spiReg->SPI_CTRLR0 &= ~(TRANSMIT_RECEIVE_MASK);
    //spiReg->SPI_CTRLR0 |= TRANSMIT_RECEIVE;
    spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(3ul<<18) | RECEIVE_ONLY;
    spiReg->SPI_CTRLR1 = size - 1;

 //   spiReg->SPI_TXFTLR = 1;
    spiReg->SPI_RXFTLR = 0;

 //   SpiEnableRxInt(spiChNumb);

    spiReg->SPI_ENR = SPI_ENABLE;

    return OK;
}

_ATTR_DRIVERLIB_CODE_
rk_size_t SPIReadFIFO0(eSPI_ch_t spiChNumb,uint8 *pdata, uint32 size)
{
    pSPI_REG spiReg;
    rk_size_t dwRealSize;

     if(spiChNumb == SPI_CH0)
     {
         spiReg = (pSPI_REG) SPI0_BASE;
     }
     else if(spiChNumb == SPI_CH1)
     {
         spiReg = (pSPI_REG) SPI1_BASE;
     }
     else
     {
        return ERROR;
     }
    while (size--)
    {
        if ((spiReg->SPI_SR & RECEIVE_FIFO_EMPTY) != RECEIVE_FIFO_EMPTY)
        {
            *pdata++ = (uint8)(spiReg->SPI_RXDR[0] & 0xFF);
            dwRealSize++;
        }
        else
        {
            break;
        }
    }

    return dwRealSize;
}

_ATTR_DRIVERLIB_CODE_
rk_size_t SPIReadFIFO(eSPI_ch_t spiChNumb,uint8 *pdata, uint32 size)
{

    int32  totalsize;
    pSPI_REG spiReg;

    totalsize = size;

     if(spiChNumb == SPI_CH0)
     {
         spiReg = (pSPI_REG) SPI0_BASE;
     }
     else if(spiChNumb == SPI_CH1)
     {
         spiReg = (pSPI_REG) SPI1_BASE;
     }
     else
     {
        return ERROR;
     }
    while (size)
    {
        //spiReg->SPI_TXDR[0] = 0xFF;     //send clock
        //DelayUs(5);
        if ((spiReg->SPI_SR & RECEIVE_FIFO_EMPTY) != RECEIVE_FIFO_EMPTY)
        {
            *pdata++ = (uint8)(spiReg->SPI_RXDR[0] & 0xFF);
            size--;
        }
        else
        {
            break;
        }
    }

    return (totalsize - size);


}



/*
--------------------------------------------------------------------------------
  Function name : rk_err_t SPIDmaWriteEnable(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size, pFunc CallBack)
  Author        :
  Description   :

  Input         : baudRate:   unit:HZ
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:        After finish spi transfer,need call SPIDeInit function to release bus.
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
rk_err_t SPIDmaWriteEnable(eSPI_ch_t spiChNumb,eSPI_TRANSFER_MODE_t  transfer_mode)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return ERROR;
    }
    spiReg->SPI_ENR = SPI_DISABLE;

    if(transfer_mode & SPI_TRANSMIT_ONLY)
    {
        //transmit only
        //spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(3ul<<18) | TRANSMIT_ONLY;
        spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 | TRANSMIT_ONLY;
    }

    if(transfer_mode & SPI_RECEIVE_ONLY)
    {
        //RECEIVE_ONLY
        //spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(3ul<<18) | RECEIVE_ONLY;
        spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 | RECEIVE_ONLY;

    }
    spiReg->SPI_CTRLR1 = 0x1f;

    spiReg->SPI_DMACR = TRANSMIT_DMA_ENABLE;// | RECEIVE_DMA_ENABLE;
    spiReg->SPI_ENR = SPI_ENABLE;

    return OK;
}

/*
--------------------------------------------------------------------------------
  Function name : int32 SPIDmaReadEnable(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size, pFunc CallBack)
  Author        :
  Description   :

  Input         : baudRate:   unit:HZ
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:        After finish spi transfer,need call SPIDeInit function to release bus.
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
rk_err_t  SPIDmaReadEnable(eSPI_ch_t spiChNumb,uint32 size)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return ERROR;
    }
    spiReg->SPI_ENR = SPI_DISABLE;

    spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 | RECEIVE_ONLY;

    spiReg->SPI_CTRLR1 = size - 1;
    spiReg->SPI_DMACR   = RECEIVE_DMA_ENABLE; //| TRANSMIT_DMA_ENABLE;
    spiReg->SPI_ENR = SPI_ENABLE;

    return OK;
}

////////////////////////////////////////////////////////////////////////////////
/*
--------------------------------------------------------------------------------
  Function name :
  Author        :
  Description   :
  Input         :
  Return        :
  History:     <author>         <time>         <version>
  desc:         Chad.ma
  Note:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void SPI_HWInit(eSPI_ch_t nCh)
{
    switch(nCh)
    {
        case SPI_CH0:
            //ScuClockGateCtr(CLK_SPI0_GATE, 0);
            //ScuClockGateCtr(PCLK_SPI0_GATE, 0);
            //DelayMs(1);
            ScuClockGateCtr(CLK_SPI0_GATE, 1);
            ScuClockGateCtr(PCLK_SPI0_GATE, 1);

            ScuSoftResetCtr(SPI0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(SPI0_SRST, 0);

            SetSPIFreq(SPI_CH0,XIN24M,24000000);
            break;

        case SPI_CH1:
            ScuClockGateCtr(CLK_SPI1_GATE, 0);
            ScuClockGateCtr(PCLK_SPI1_GATE, 0);
            DelayMs(1);
            ScuClockGateCtr(CLK_SPI1_GATE, 1);
            ScuClockGateCtr(PCLK_SPI1_GATE, 1);

            ScuSoftResetCtr(SPI1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(SPI1_SRST, 0);

            SetSPIFreq(SPI_CH1,XIN24M,24000000);
            break;

        default:
            break;
    }
}
/*
********************************************************************************
*
*                         End of spi.c
*
********************************************************************************
*/

