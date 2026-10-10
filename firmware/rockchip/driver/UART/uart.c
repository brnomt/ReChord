/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   Uart.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*
*    desc:    ORG.
********************************************************************************
*/

#define  IN_DRIVER_UART

#include "DriverInclude.h"
#include "interrupt.h"

#define _IN_UART_

const uint32 UART_GROUP[6] =
{
    UART0_BASE,
    UART1_BASE,
    UART2_BASE,
    UART3_BASE,
    UART4_BASE,
    UART5_BASE,
};

static int32 UartDevGpioInit(eUART_IOMUX uartChnPort);
static int32 UartDevGpioDeInit(eUART_IOMUX uartChnPort);
static uint32 UARTGetIntType(eUART_CH uartChn);
void BT_UARTInterrupt(uint32 uartChn);


#define UARTPORT(n)          ((UART_REG*)(UART_GROUP[n]))

UINT8 gUart_flag[UART_CH_MAX] = {0};

//#ifdef DRIVERLIB_IRAM
/*
--------------------------------------------------------------------------------
  Function name : void UARTRest(void)
  Author        :
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void UARTRest(eUART_CH uartChn)
{
    UART_REG *  pReg = UARTPORT(uartChn);
    pReg->UART_SRR = UART_RESET | RCVR_FIFO_REST | XMIT_FIFO_RESET;
    pReg->UART_IER = 0;
    pReg->UART_DMASA = 1;
}

/*
--------------------------------------------------------------------------------
  Function name : int32 UARTSetIOP(uint8 useIrDA)
  Author        :
  Description   : set uart role as serial port or IRDA.

  Input         :

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
int32 UARTSetIOP(eUART_CH uartChn,uint8 useIrDA)
{
    //pUART_REG   phwHead   = (pUART_REG)UART1_BASE_ADDR;
    UART_REG *  pReg = UARTPORT(uartChn);
    if ((useIrDA == IRDA_SIR_DISABLED) || (useIrDA == IRDA_SIR_ENSABLED))
    {
        pReg->UART_MCR = useIrDA;
        return (0);
    }

    return (-1);
}

/*
--------------------------------------------------------------------------------
  Function name : UARTSetBaudRate(pUART_REG phead, uint32 baudRate)
  Author        :
  Description   :
                  baudRate:serial initial speed,get by table seek,
  Input         :

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
int32 UARTSetBaudRate(eUART_CH uartChn,uint32 clk, uint32 baudRate)
{
    uint32  DivLatch;
    UART_REG *  pReg = UARTPORT(uartChn);
    //  DLL and THR(RBR) are mapped the same offset, set LCR[7] to access to
    //  DLL register, otherwise the THR(RBR)

    DivLatch = clk / MODE_X_DIV / baudRate;

    pReg->UART_LCR |= LCR_DLA_EN;

    pReg->UART_DLL  = (DivLatch >> 0) & 0xff;
    pReg->UART_DLH  = (DivLatch >> 8) & 0xff;

    pReg->UART_LCR &= (~LCR_DLA_EN);

    return (0);
}

/*
--------------------------------------------------------------------------------
  Function name : int32 UARTSetLcrReg(uint8 byteSize, uint8 parity,uint8 stopBits )
  Author        :
  Description   :

  Input         :

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
int32 UARTSetLcrReg(eUART_CH uartChn,uint8 byteSize, uint8 parity,uint8 stopBits )
{
    uint32 lcr;
    int32  bRet = 0;

    UART_REG *  pReg = UARTPORT(uartChn);
    lcr = (pReg->UART_LCR) & (~(uint32)(UART_DATABIT_MASK));

    //Data length select
    switch ( byteSize )
    {
        case UART_DATA_5B:
            lcr |= LCR_WLS_5;
            break;
        case UART_DATA_6B:
            lcr |= LCR_WLS_6;
            break;
        case UART_DATA_7B:
            lcr |= LCR_WLS_7;
            break;
        case UART_DATA_8B:
            lcr |= LCR_WLS_8;
            break;
        default:
            bRet = -1;
            break;
    }

    //Parity set
    switch ( parity )
    {
        case UART_ODD_PARITY:
        case UART_EVEN_PARITY:
            lcr |= PARITY_ENABLED;
            lcr |= ((parity) << 4);
            break;
        case UART_PARITY_DISABLE:
            lcr &= ~PARITY_ENABLED;
            break;
        default:
            bRet = -1;
            break;
    }

    //StopBits set
    switch ( stopBits )
    {
        case 0:
            lcr |= ONE_STOP_BIT;
            break;
        case 1:
            lcr |= ONE_HALF_OR_TWO_BIT;
            break;
        default:
            bRet = -1;
            break;
    }

    if (bRet == 0)
    {
        pReg->UART_LCR = lcr;
    }

    return(bRet);
}

/*
--------------------------------------------------------------------------------
  Function name : UARTSetFifoEnabledNumb()
  Author        :
  Description   :

  Input         :

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void UARTSetFifoEnabledNumb(eUART_CH uartChn,uint32 param)
{
    UART_REG *  pReg = UARTPORT(uartChn);

    pReg->UART_SFE  = (param & 0x01);
    pReg->UART_SRT  = (param >> 1) & 0x03;
    pReg->UART_STET = (param >> 3) & 0x03;
}

/*
--------------------------------------------------------------------------------
  Function name : void UARTSetIntEnabled(uint32 uartIntNumb)
  Author        :
  Description   : 设置UART口的中断使能打开。

  Input         : uartIntNumb：UART的中断号

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void UARTSetIntEnabled(eUART_CH uartChn,uint32 uartIntNumb)
{
    UART_REG *  pReg = UARTPORT(uartChn);

    pReg->UART_IER |= uartIntNumb;
}

/*
--------------------------------------------------------------------------------
  Function name : void UARTSetIntDisabled(uint32 uartIntNumb)
  Author        :
  Description   : 关中断使能

  Input         : uartIntNumb：UART的中断号

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void UARTSetIntDisabled(eUART_CH uartChn,uint32 uartIntNumb)
{
    UART_REG *  pReg = UARTPORT(uartChn);
    pReg->UART_IER &= (~uartIntNumb);
}

void UartIntIsr(uint32 uartChn)
{
    /*get uart intrutp type*/
    uint32 uartIntType;
    gUart_flag[uartChn] = uartIntType = UARTGetIntType(uartChn);

    if( uartIntType & UART_INT_READ_FIFO_NOT_EMPTY )
    {
        UARTSetIntDisabled(uartChn,UART_IE_RX);
    }

//    if( uartIntType & UART_INT_WRITE_FIFO_EMPTY )
//    {
//        UARTSetIntDisabled(uartChn,UART_IE_TX);
//    }

}

void UartIntIsr5(void)
{
    UartIntIsr(UART_CH5);
}

void UartIntIsr4(void)
{
    UartIntIsr(UART_CH4);
}

void UartIntIsr3(void)
{
    UartIntIsr(UART_CH3);
}

void UartIntIsr2(void)
{
    UartIntIsr(UART_CH2);
}

void UartIntIsr1(void)
{
#ifdef _BLUETOOTH_
    BT_UARTInterrupt(UART_CH1);
#else
    UartIntIsr(UART_CH1);
#endif
}

void UartIntIsr0(void)
{
    UartIntIsr(UART_CH0);
}
/*
--------------------------------------------------------------------------------
  Function name : UARTWriteByte(uint8 ch)
  Author        :
  Description   : 串口写一个字节

  Input         : byte:输入的字节值
                  uartTimeOut:等待延时时间

  Return        : 0:OK -1:fail

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
rk_size_t UARTWriteByte0(eUART_CH uartChn,uint8 * pdata, uint32 cnt)
{
    rk_size_t dwRealSize;
    UART_REG *  pReg = UARTPORT(uartChn);

    dwRealSize = 0;
    while (cnt--)
    {
        if (!(pReg->UART_USR & UART_TRANSMIT_FIFO_NOT_FULL))
        {
            break;
        }

        pReg->UART_THR = *pdata++;
        dwRealSize++;
    }

    return dwRealSize;

}

_ATTR_DRIVERLIB_CODE_
rk_size_t UARTWriteByte(eUART_CH uartChn,uint8 * pdata, uint32 cnt)
{
    rk_size_t realSize;
    rk_size_t needtoSend;
    uint32 uartIntType;
    int timeOut;
    UART_REG *  pReg = UARTPORT(uartChn);

    if(pdata == NULL || cnt == 0)
        return ERROR;

    timeOut = UART_TIMEOUT_MS;
    needtoSend = cnt;

    realSize = UARTWriteByte0(uartChn,pdata,cnt);

    while(realSize == 0)
    {
        DelayMs(5);
        realSize = UARTWriteByte0(uartChn,pdata,cnt);
    }

    if (realSize == needtoSend)
    {
        return cnt;
    }
    else
    {
        pdata       += realSize;
        needtoSend  -= realSize;
    }

    UARTSetIntEnabled(uartChn,UART_IE_TX);
    do
    {
        while (!gUart_flag[uartChn] && timeOut > 0)
        {
            timeOut--;
            __WFI();
        }

        if(timeOut)
        {
            if (gUart_flag[uartChn] & UART_INT_WRITE_FIFO_EMPTY)
            {
                int size = 0;
                size = UARTWriteByte0(uartChn,pdata, needtoSend);

                pdata       += size;
                needtoSend  -= size;
                realSize    += size;
                gUart_flag[uartChn] = 0;
            }
        }
        else
        {
            UARTSetIntDisabled(uartChn,UART_IE_TX);
            return TIMEOUT;
        }
    }while (needtoSend);

    UARTSetIntDisabled(uartChn,UART_IE_TX);
    return realSize;
}

/*
--------------------------------------------------------------------------------
  Function name :  UARTReadByte0(eUART_CH uartPort,uint8 *pdata, uint32 cnt)
  Author        :
  Description   : 串口读取cnt个字节

  Input         : uartPort - uart channel port
                  pdata - the buffer which read data
                  cnt - want to read byte number

  Return        : real read data size (bytes)

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
rk_size_t UARTReadByte0(eUART_CH uartChn,uint8 *pdata, uint32 cnt)
{
    rk_size_t dwRealSize;
    int timeOut;

    UART_REG *  pReg = UARTPORT(uartChn);

    dwRealSize = 0;
    //timeOut = UART_TIMEOUT_MS;

    while (cnt--)
    {
        if (!(pReg->UART_USR & UART_RECEIVE_FIFO_NOT_EMPTY))
        {
            break;
        }

/*
        while(!(pReg->UART_USR & UART_RECEIVE_FIFO_NOT_EMPTY))
        {
            if(timeOut == 0)
                return TIMEOUT;
            timeOut--;
        }
*/
        *pdata++ = (uint8 )pReg->UART_RBR;
        dwRealSize++;
    }

    return dwRealSize;
}

rk_size_t  UARTReadByte(eUART_CH uartChn,uint8 *pdata, uint32 cnt)
{
    rk_size_t realSize;
    uint32 uartIntType;
    UART_REG *  pReg = UARTPORT(uartChn);

    realSize = UARTReadByte0(uartChn,pdata,cnt);
    return realSize;
}

int32 UARTIntInit0(eUART_CH uartChn,
                  eUART_BaudRate baudRate,
                  eUART_dataLen dataBit,
                  eUART_stopBit stopBit,
                  eUART_parity_en parity)
{
    UART_REG *  pReg = UARTPORT(uartChn);

    UARTRest(uartChn);

    UARTSetIOP(uartChn,IRDA_SIR_DISABLED);

    UARTSetFifoEnabledNumb(uartChn,SHADOW_FIFI_ENABLED
                           | RCVR_TRIGGER_ONE
                           | TX_TRIGGER_EMPTY);

    UARTSetLcrReg(uartChn,dataBit,parity,stopBit);

    UARTSetBaudRate(uartChn,24 * 1000 * 1000, baudRate);
    pReg->UART_SRR = RCVR_FIFO_REST | XMIT_FIFO_RESET;

    return (0);
}

_ATTR_DRIVERLIB_CODE_
int32 UARTInit(eUART_IOMUX uartPort,
               eUART_BaudRate baudRate,
               eUART_dataLen dataBit,
               eUART_stopBit stopBit,
               eUART_parity_en parity)
{
    int uartChn;
    int ret;

    uartChn = uartPort / 3;
//    UartDevChannelSel((eUART_CH)uartChn);
    ret = UartDevGpioInit(uartPort);
    if( ret != OK )
    {
        printf("UART not SUPPORT this port. \n");
        return ERROR;
    }

    //open rst uart ip
    switch( uartChn )
    {
        case UART_CH0:
            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);

            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            SetUartFreq(UART_CH0,XIN24M,24*1000*1000);
            break;

        case UART_CH1:
            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);

            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            SetUartFreq(UART_CH1,XIN24M,24*1000*1000);
            break;

        case UART_CH2:
            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);

            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            SetUartFreq(UART_CH2,XIN24M,24*1000*1000);
            break;

        case UART_CH3:
            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);

            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            SetUartFreq(UART_CH3,XIN24M,24*1000*1000);
            break;

        case UART_CH4:
            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);

            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            SetUartFreq(UART_CH4,XIN24M,24*1000*1000);
            break;

        case UART_CH5:
            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);

            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            SetUartFreq(UART_CH5,XIN24M,24*1000*1000);
            break;
    }

    UARTIntInit0((eUART_CH)uartChn, baudRate, dataBit, stopBit, parity);

    gUart_flag[uartChn] = 0;

    switch ( uartChn )
    {
        case UART_CH0:
            IntRegister(INT_ID_UART0 ,UartIntIsr0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_CH1:
            IntRegister(INT_ID_UART1 ,UartIntIsr1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_CH2:
            IntRegister(INT_ID_UART2 ,UartIntIsr2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_CH3:
            IntRegister(INT_ID_UART3 ,UartIntIsr3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_CH4:
            IntRegister(INT_ID_UART4 ,UartIntIsr4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_CH5:
            IntRegister(INT_ID_UART5 ,UartIntIsr5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;

        default:
            break;
    }

    return (0);
}

_ATTR_DRIVERLIB_CODE_
int32 UARTDeInit(eUART_IOMUX uartPort)
{
    int uartChn;
    int ret;

    uartChn = uartPort / 3;

    ret = UartDevGpioDeInit(uartPort);

    if( ret != OK )
    {
        printf("UART not SUPPORT this port. \n");
        return ERROR;
    }

    //open rst uart ip
    switch( uartChn )
    {
        case UART_CH0:
            ScuSoftResetCtr(UART0_SRST, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 0);
            ScuClockGateCtr(CLK_UART0_GATE, 0);
            break;

        case UART_CH1:
            ScuSoftResetCtr(UART1_SRST, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 0);
            ScuClockGateCtr(CLK_UART1_GATE, 0);
            break;

        case UART_CH2:
            ScuSoftResetCtr(UART2_SRST, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 0);
            ScuClockGateCtr(CLK_UART2_GATE, 0);
            break;

        case UART_CH3:
            ScuSoftResetCtr(UART3_SRST, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 0);
            ScuClockGateCtr(CLK_UART3_GATE, 0);
            break;

        case UART_CH4:
            ScuSoftResetCtr(UART4_SRST, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 0);
            ScuClockGateCtr(CLK_UART4_GATE, 0);
            break;

        case UART_CH5:
            ScuSoftResetCtr(UART5_SRST, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 0);
            ScuClockGateCtr(CLK_UART5_GATE, 0);
            break;
    }

    gUart_flag[uartChn] = 0;

    switch ( uartChn )
    {
        case UART_CH0:
            IntPendingClear(INT_ID_UART0);
            IntDisable(INT_ID_UART0);
            IntUnregister(INT_ID_UART0);
            break;

        case UART_CH1:
            IntPendingClear(INT_ID_UART1);
            IntDisable(INT_ID_UART1);
            IntUnregister(INT_ID_UART1);
            break;

        case UART_CH2:
            IntPendingClear(INT_ID_UART2);
            IntDisable(INT_ID_UART2);
            IntUnregister(INT_ID_UART2);
            break;

        case UART_CH3:
            IntPendingClear(INT_ID_UART3);
            IntDisable(INT_ID_UART3);
            IntUnregister(INT_ID_UART3);
            break;

        case UART_CH4:
            IntPendingClear(INT_ID_UART4);
            IntDisable(INT_ID_UART4);
            IntUnregister(INT_ID_UART4);
            break;

        case UART_CH5:
            IntPendingClear(INT_ID_UART5);
            IntDisable(INT_ID_UART5);
            IntUnregister(INT_ID_UART5);
            break;

        default:
            break;
    }

    return (0);
}

/*
*********************************************************************************************************
*                                     UARTGetIntType(void)
*
* Description : 判断串口中断类型.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint32 UARTGetIntType(eUART_CH uartChn)
{
    uint32 dwUartIntStaus;
    uint event;
    UART_REG *  pReg = UARTPORT(uartChn);

    dwUartIntStaus = pReg->UART_IIR & UART_IF_MASK;
    event = 0;

    if (dwUartIntStaus & UART_IF_REC_DATA)
    {
        event |= UART_INT_READ_FIFO_NOT_EMPTY;
    }

    if (dwUartIntStaus & UART_IF_THR_EMPTY)
    {
        event |= UART_INT_WRITE_FIFO_EMPTY;
    }

    return event;
}


int32 UartDmaWrite(eDMA_CHN dmaPort,
                        eUART_IOMUX uartPort,
                        uint8 *pdata,
                        uint32 size,
                        pFunc Callback)
{
    UART_REG *  pReg ;
    uint32 dstAddr;
    DMA_CFGX DmaCfg;
    int uartChn;
    int dmaChn;

    uartChn = uartPort / 3;
    pReg = UARTPORT(uartChn);
    dstAddr = (uint32)&(pReg->UART_THR);

    UARTSetIntEnabled(uartChn,UART_IE_TX);

    switch( uartChn)
    {
        case UART_CH0:
        case UART_CH2:
            DmaCfg.CTLL = DMA_CTLL_M2UART_02;
            DmaCfg.CFGL = DMA_CFGL_M2UART_02;
            DmaCfg.CFGH = DMA_CFGH_M2UART_02;
            DmaCfg.pLLP = 0;
            break;

        case UART_CH1:
            DmaCfg.CTLL = DMA_CTLL_M2UART_1;
            DmaCfg.CFGL = DMA_CFGL_M2UART_1;
            DmaCfg.CFGH = DMA_CFGH_M2UART_1;
            DmaCfg.pLLP = 0;
            break;

        case UART_CH3:

            break;

        case UART_CH4:

            break;

        case UART_CH5:

            break;
    }

    DmaStart((uint32)(dmaPort), (UINT32)(pdata),dstAddr,size, &DmaCfg, Callback);
    return dmaChn;
}

int32 UartDmaRead(eDMA_CHN dmaPort,
                        eUART_IOMUX uartPort,
                        uint8 *pdata,
                        uint32 size,
                        pFunc Callback)
{
    UART_REG *  pReg ;
    uint32 srcAddr;
    DMA_CFGX DmaCfg;
    int uartChn;
    int dmaChn;

    uartChn = uartPort / 3;
    pReg = UARTPORT(uartChn);
    srcAddr = (uint32)&(pReg->UART_RBR);

    switch( uartPort)
    {
        case UART_CH0:
        case UART_CH2:
            DmaCfg.CTLL = DMA_CTLL_UART_022M;
            DmaCfg.CFGL = DMA_CFGL_UART_022M;
            DmaCfg.CFGH = DMA_CFGH_UART_022M;
            DmaCfg.pLLP = 0;

            break;

        case UART_CH1:
            DmaCfg.CTLL = DMA_CTLL_UART_12M;
            DmaCfg.CFGL = DMA_CFGL_UART_12M;
            DmaCfg.CFGH = DMA_CFGH_UART_12M;
            DmaCfg.pLLP = 0;

            break;

        case UART_CH3:
            break;

        case UART_CH4:
            break;

        case UART_CH5:
            break;
    }

    DmaStart((uint32)(dmaPort),srcAddr,(UINT32)(pdata),size, &DmaCfg, Callback);
    return dmaChn;

}

int32 UartDevGpioInit(eUART_IOMUX uartChnPort)
{
    int ret = OK;
    if( uartChnPort > UART_IO_MAX)
        return ERROR;

    switch( uartChnPort )
    {
        case UART_CH0_PA:
            Grf_UART0_Sel(UART_SEL_A);
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_UART0A_TX);
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_UART0A_RX);
            break;

        case UART_CH0_PB:
            Grf_UART0_Sel(UART_SEL_B);
            break;

        case UART_CH1_PA:
            Grf_UART1_Sel(UART_SEL_A);
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_UART1A_TX);
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_UART1A_RX);
            // rts/cts
            //Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin6,IOMUX_GPIO2B6_UART1A_RTS);
            //Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin7,IOMUX_GPIO2B7_UART1A_CTS);
            break;

        case UART_CH1_PB:
            Grf_UART1_Sel(UART_SEL_B);
            Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin2,IOMUX_GPIO1A2_UART1B_TX);
            Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin1,IOMUX_GPIO1A1_UART1B_RX);
            // rts/cts
            //Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin3,IOMUX_GPIO1A4_UART1B_RTS);
            //Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin4,IOMUX_GPIO1A3_UART1B_CTS);
            break;

        case UART_CH2_PA:
            Grf_UART2_Sel(UART_SEL_A);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin6,IOMUX_GPIO0B6_UART2A_TX);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin5,IOMUX_GPIO0B5_UART2A_RX);

            // rts/cts
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin3,IOMUX_GPIO0B3_UART2A_RTS);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin4,IOMUX_GPIO0B4_UART2A_CTS);
            break;

        case UART_CH2_PB:
            Grf_UART2_Sel(UART_SEL_B);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin5,IOMUX_GPIO0C5_UART2B_TX);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin4,IOMUX_GPIO0C4_UART2B_RX);

            // rts/cts
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin6,IOMUX_GPIO0C6_UART2B_RTS);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin7,IOMUX_GPIO0C7_UART2B_CTS);
            break;

        case UART_CH2_PC:
            Grf_UART2_Sel(UART_SEL_C);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_UART2C_TX);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_UART2C_RX);

            // rts/cts
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_UART2C_RTS);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_UART2C_CTS);
            break;

        case UART_CH3_PA:
            Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin5,IOMUX_GPIO1A5_UART3_TX);
            Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin6,IOMUX_GPIO1A6_UART3_RX);
            break;

        case UART_CH4_PA:
            Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin7,IOMUX_GPIO1A7_UART4_TX);
            Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin0,IOMUX_GPIO1B0_UART4_RX);
            break;

        case UART_CH5_PA:
            Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_UART5_TX);
            Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin2,IOMUX_GPIO1B2_UART5_RX);
            break;

        default:
            ret = ERROR;
            break;
    }

    return ret;
}

int32 UartDevGpioDeInit(eUART_IOMUX uartChnPort)
{
    int ret = OK;
    if( uartChnPort > UART_IO_MAX)
        return ERROR;

    switch( uartChnPort )
    {
        case UART_CH0_PA:
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin5, IOMUX_GPIO2B5_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin4, IOMUX_GPIO2B4_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin5, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin4, GPIO_IN);
            break;

        case UART_CH0_PB:
            break;

        case UART_CH1_PA:
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin0, IOMUX_GPIO2C0_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin1, IOMUX_GPIO2C1_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortC_Pin0, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortC_Pin1, GPIO_IN);
            break;

        case UART_CH1_PB:
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin2, IOMUX_GPIO1A2_IO);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin1, IOMUX_GPIO1A1_IO);
            Gpio_SetPinDirection(GPIO_CH1, GPIOPortA_Pin2, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH1, GPIOPortA_Pin1, GPIO_IN);
            break;

        case UART_CH2_PA:
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin6, IOMUX_GPIO0B6_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin5, IOMUX_GPIO0B5_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin3, IOMUX_GPIO0B3_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin4, IOMUX_GPIO0B4_IO);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortB_Pin6, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortB_Pin5, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortB_Pin3, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortB_Pin4, GPIO_IN);
            break;

        case UART_CH2_PB:
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin4, IOMUX_GPIO0C4_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin5, IOMUX_GPIO0C5_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin6, IOMUX_GPIO0C6_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin7, IOMUX_GPIO0C7_IO);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortC_Pin4, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortC_Pin5, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortC_Pin6, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortC_Pin7, GPIO_IN);
            break;

        case UART_CH2_PC:
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin1, IOMUX_GPIO0A1_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin2, IOMUX_GPIO0A2_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin4, IOMUX_GPIO0A3_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin3, IOMUX_GPIO0A4_IO);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin1, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin2, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin4, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin3, GPIO_IN);
            break;

        case UART_CH3_PA:
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin5, IOMUX_GPIO1A5_IO);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin6, IOMUX_GPIO1A6_IO);
            Gpio_SetPinDirection(GPIO_CH1, GPIOPortA_Pin5, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH1, GPIOPortA_Pin6, GPIO_IN);
            break;

        case UART_CH4_PA:
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin7, IOMUX_GPIO1A7_IO);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin0, IOMUX_GPIO1B0_IO);
            Gpio_SetPinDirection(GPIO_CH1, GPIOPortA_Pin7, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH1, GPIOPortB_Pin0, GPIO_IN);
            break;

        case UART_CH5_PA:
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin1, IOMUX_GPIO1B1_IO);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin2, IOMUX_GPIO1B2_IO);
            Gpio_SetPinDirection(GPIO_CH1, GPIOPortB_Pin1, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH1, GPIOPortB_Pin2, GPIO_IN);
            break;

        default:
            ret = ERROR;
            break;
    }

    return ret;
}

//#endif

void UartEnableFlowControl(eUART_CH uartChn)
{
    UART_REG *  pReg ;

    pReg = UARTPORT(uartChn);
    pReg->UART_MCR = AUTO_FLOW_ENSABLED;
    pReg->UART_FCR = 0x1;
}

/*0 = cts_n input is de-asserted (logic 1)
1 = cts_n input is asserted (logic 0)
*/
uint32 UartGetCTSState(eUART_CH uartChn)
{
    UART_REG *  pReg ;
    uint32 cts_status;

    pReg = UARTPORT(uartChn);
    cts_status = (pReg->UART_MSR & (1<<4)) >> 4;

    return cts_status;
}

bool UartCTSStateChanged(eUART_CH uartChn)
{
    UART_REG *  pReg ;
    bool ctsChanged;

    pReg = UARTPORT(uartChn);
    ctsChanged = pReg->UART_MSR & 0x01;

    return ctsChanged;
}


void UartSetAutoRTS(eUART_CH uartChn)
{
    UART_REG *  pReg ;
    pReg = UARTPORT(uartChn);

    pReg->UART_FCR = 0<<4 | 1<<0;
    pReg->UART_MCR = IRDA_SIR_DISABLED <<6 | AUTO_FLOW_ENSABLED | 1<<1; //request to send
}


void UartSetAutoCTS(eUART_CH uartChn)
{
    UART_REG *  pReg ;
    pReg = UARTPORT(uartChn);

    pReg->UART_FCR = 0<<4 | 1<<0;
    pReg->UART_MCR = IRDA_SIR_DISABLED <<6 | AUTO_FLOW_ENSABLED; //request to send
}

#ifdef _BLUETOOTH_

#define DEFAULT_INPUT_BUFFER_SIZE                                128
#define DEFAULT_OUTPUT_BUFFER_SIZE                               64
#define XOFF_LIMIT                                               32
#define XON_LIMIT                                                128

typedef struct _tagUartContext_t
{
//   unsigned char          ID;
//   unsigned long          UartBase;
//   unsigned char          RxBuffer[DEFAULT_INPUT_BUFFER_SIZE];
//
//   int                    RxBufferSize;
//   volatile int           RxBytesFree;
//   int                    RxInIndex;
//   int                    RxOutIndex;
//   int                    XOffLimit;
//   int                    XOnLimit;
#ifdef _A2DP_SINK_
     #if BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H5
     unsigned char          TxBuffer[1024*2];
     #else
     unsigned char          TxBuffer[1024];
     #endif
#else
     #if BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H5
     unsigned char          TxBuffer[672*2];
     #else
     unsigned char          TxBuffer[672];
     #endif
#endif
     unsigned char          *TxBufferP;

     int                    TxBufferSize;
//   volatile int           TxBytesFree;
//   int                    TxInIndex;
     int                      TxOutIndex;
//   unsigned char          Flags;
//   HCILL_Action_Request_t HCILL_Action;
//   Byte_t                 HCILL_Byte;
     unsigned char          isInTx;
     unsigned char          unused[3];
   void (* recv)(uint8 data);
   void (* tx_complete)(void);

} UartContext_t;
_ATTR_DRIVER_BSS_ static UartContext_t  UartContext;

_ATTR_DRIVER_CODE_
int BT_UARTInit(uint32 uartChn, uint32 clk,eUART_BaudRate baudRate,eUART_dataLen dataBit,eUART_stopBit stopBit,eUART_parity_en parity)
{
    UARTInit(uartChn,baudRate,dataBit,stopBit, parity );
    UARTSetIntEnabled(uartChn/3, UART_IE_RX );//| UART_IE_THRE);
  //  UartReg->UART_IIR = 0;

    memset(&UartContext, 0 , sizeof(UartContext_t));
}

_ATTR_DRIVER_CODE_
void BT_Recv_Register(void (*func)(uint data))
{
    UartContext.recv = func;
}

_ATTR_DRIVER_CODE_
void BT_Tx_Complete_Register(void (*func)(void))
{
    UartContext.tx_complete = func;
}

_ATTR_DRIVER_CODE_
uint8 get_send_state()
{
    return UartContext.isInTx;
}


_ATTR_DRIVER_CODE_
void BT_UARTSend(uint32 uartChn, char * data, int len)
{
    //DEBUG("Enter BT_UARTSend\n");
    memcpy(UartContext.TxBuffer, data, len);
   // while(UartContext.isInTx);

    UartContext.isInTx = 1;
    //UartContext.TxBufferP = data;
    UartContext.TxBufferP = UartContext.TxBuffer;
    UartContext.TxBufferSize = len;
    UartContext.TxOutIndex = 0;
    UARTSetIntEnabled(uartChn/3, UART_IE_TX);

    //UartContext.TxOutIndex++;
    //UartReg->UART_THR = UartContext.TxBuffer[0];
    //UartReg->UART_IIR = UartReg->UART_IIR | UART_IF_THR_EMPTY;

    //DEBUG("Leaving BT_UARTSend\n");
}

_ATTR_DRIVER_CODE_
void BT_UARTInt(void)
{
    BT_UARTInterrupt(BT_HCI_UART_ID);
}

_ATTR_DRIVER_CODE_
void BT_UARTInterrupt(uint32 uartChn)
{
    uint32 intstatus;
    uint8  recvdata;

    UART_REG *UartReg = UARTPORT(uartChn);

    intstatus = UartReg->UART_IIR & UART_IF_MASK;
    //UartReg->UART_IIR = 0x01;

    switch(intstatus)
    {
        case UART_IF_REC_DATA:
            if(UartContext.recv != NULL)
            {
                while ((UartReg->UART_USR & UART_RECEIVE_FIFO_NOT_EMPTY) == UART_RECEIVE_FIFO_NOT_EMPTY)
                {
                    recvdata = (uint8)UartReg->UART_RBR;
                    UartContext.recv(recvdata);
                }
            }
            break;

        case UART_IF_THR_EMPTY:
            while ((UartReg->UART_USR & UART_TRANSMIT_FIFO_NOT_FULL) == UART_TRANSMIT_FIFO_NOT_FULL)
            {
                if(UartContext.TxBufferSize != UartContext.TxOutIndex)
                {
                    UartReg->UART_THR = UartContext.TxBufferP[UartContext.TxOutIndex];
                    UartContext.TxOutIndex++;
                    if(UartContext.TxBufferSize == UartContext.TxOutIndex)
                    {
                        return;
                    }
                }
                else
                {
                    UARTSetIntDisabled(uartChn, UART_IE_TX);
                    UartContext.isInTx = 0;
                    if(UartContext.tx_complete != NULL)
                    {
                        UartContext.tx_complete();
                    }
                    break;
                }
            }
            break;

        default:
            break;
    }

}

#endif

/*
********************************************************************************
*
*                         End of Uart.c
*
********************************************************************************
*/

