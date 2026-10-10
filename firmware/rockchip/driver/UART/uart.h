/******************************************************************/
/*   Copyright (C) 2008 ROCK-CHIPS FUZHOU . All Rights Reserved.  */
/*******************************************************************
File    :  api_uart.h
Desc    :  UART interface parmeters.

Author  : lhh
Date    : 2008-11-10
Modified:
Revision:           1.00
$Log: api_uart.h,v $
*********************************************************************/

#ifndef _UART_H_
#define _UART_H_
#include "driver/DMA/Dma.h"   /* eDMA_CHN */

#undef  EXT
#ifdef  _IN_UART_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------
  
                        Macro define  
  
--------------------------------------------------------------------------------
*/
///uartWorkStatusFlag
#define   UART0_TX_WORK                     (1)
#define   UART0_RX_WORK                     (1<<1)
#define   UART1_TX_WORK                     (1<<2)
#define   UART1_RX_WORK                     (1<<3)
#define   UART2_TX_WORK                     (1<<5)
#define   UART2_RX_WORK                     (1<<6)
#define   UART3_TX_WORK                     (1<<7)
#define   UART3_RX_WORK                     (1<<8)
#define   UART_ERR_RX                       (1<<4)

#define   UART_IE_THRE                      (1<<7)
#define   UART_IE_TX                        (1<<1)
#define   UART_IE_RX                        (1)

#define   UART_IF_MASK                      (0x000F)
#define   UART_IF_THR_EMPTY                 (0x0002)
#define   UART_IF_REC_DATA                  (0x0004)

/*
--------------------------------------------------------------------------------
  
                        Struct Define
  
--------------------------------------------------------------------------------
*/
typedef enum UART_ch
{
    UART_CH0,
    UART_CH1,
    UART_CH2,
    UART_CH3,
    UART_CH4,
    UART_CH5,
    
    UART_CH_MAX
}eUART_CH;

typedef enum UART_IO_MUX
{
    UART_CH0_PA,   //0
    UART_CH0_PB,   //used same to usb dm/dp
    UART_CH0_RES,  //reserve 
    
    UART_CH1_PA,   // 3
    UART_CH1_PB,
    UART_CH1_RES,   //not use
    
    UART_CH2_PA,   // 6
    UART_CH2_PB,
    UART_CH2_PC,
    
    UART_CH3_PA,
    UART_CH3_RES0,   //reserve 
    UART_CH3_RES1,   //reserve 
    
    UART_CH4_PA,
    UART_CH4_RES0,   //reserve 
    UART_CH4_RES1,   //reserve 
    
    UART_CH5_PA,
    UART_CH5_RES0,   //reserve 
    UART_CH5_RES1,   //reserve 
    
    UART_IO_MAX    
}eUART_IOMUX;

typedef enum UART_BaudRate
{
    UART_BR_110    = 110,
    UART_BR_300    = 300,
    UART_BR_600    = 600,
    UART_BR_1200   = 1200,
    UART_BR_2400   = 2400,
    UART_BR_4800   = 4800,
    UART_BR_9600   = 9600,
    UART_BR_14400  = 14400,
    UART_BR_19200  = 19200,
    UART_BR_38400  = 38400,
    UART_BR_57600  = 57600,
    UART_BR_115200 = 115200,
    UART_BR_230400 = 230400,
    UART_BR_380400 = 380400,
    UART_BR_460800 = 460800,
    UART_BR_921600 = 921600
}eUART_BaudRate;

typedef enum UART_dataLen
{
    UART_DATA_5B = 5,
    UART_DATA_6B,
    UART_DATA_7B,
    UART_DATA_8B
}eUART_dataLen;

typedef enum UART_stopBit
{
    UART_ONE_STOPBIT,
    UART_ONE_AND_HALF_OR_TWO_STOPBIT
}eUART_stopBit;

typedef enum UART_parity_en
{
    UART_ODD_PARITY,
    UART_EVEN_PARITY,
    UART_PARITY_DISABLE
}eUART_parity_en;

//结构体定义
typedef enum _UART_INT_TYPE
{
    UART_INT_READ_FIFO_NOT_EMPTY = 2,
    UART_INT_WRITE_FIFO_EMPTY  = 4,
    UART_INT_UART_ERR = 8,
    UART_INT_NUM
    
}UART_INT_TYPE, * P_UART_INT_TYPE;

//#define UART_CONF               (CRU_BASE + 0x800 + 0xFC)      //uart select config
//#define UART1_EN                ((uint32)0)
//#define UART2_EN                ((uint32)1)
//#define UART3_EN                ((uint32)2)
//#define UART4_EN                ((uint32)3)
//#define UART5_EN                ((uint32)4)


#define UART_TIMEOUT_MS         200
/*
--------------------------------------------------------------------------------
  
                        Variable Define
  
--------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/ 

extern void  UARTRest(eUART_CH uartChn);
extern int32 UARTSetIOP(eUART_CH uartChn,uint8 useIrDA);
extern int32 UARTSetBaudRate(eUART_CH uartChn,uint32 clk, uint32 baudRate);
extern int32 UARTSetLcrReg(eUART_CH uartChn,uint8 byteSize, uint8 parity,uint8 stopBits );
extern void  UARTSetFifoEnabledNumb(eUART_CH uartChn,uint32 param);
extern void  UARTSetIntEnabled(eUART_CH uartChn,uint32 uartIntNumb);
extern void  UARTSetIntDisabled(eUART_CH uartChn,uint32 uartIntNumb);
extern rk_size_t UARTWriteByte(eUART_CH uartChn,uint8 * pdata, uint32 cnt);
extern rk_size_t  UARTReadByte(eUART_CH uartChn,uint8 *pdata, uint32 cnt);
extern int32 UARTInit(eUART_IOMUX uartPort,eUART_BaudRate baudRate,eUART_dataLen dataBit,eUART_stopBit stopBit,eUART_parity_en parity);


extern int32 UartDmaWrite(eDMA_CHN dmaPort,
                        eUART_IOMUX uartPort,
                        uint8 *pdata, 
                        uint32 size, 
                        pFunc Callback);
extern int32 UartDmaRead(eDMA_CHN dmaPort,
                        eUART_IOMUX uartPort,
                        uint8 *pdata, 
                        uint32 size, 
                        pFunc Callback);

extern void UartEnableFlowControl(eUART_CH uartChn);
extern uint32 UartGetCTSState(eUART_CH uartChn);
extern bool UartCTSStateChanged(eUART_CH uartChn);
extern void UartSetAutoRTS(eUART_CH uartChn);
/*
********************************************************************************
*
*                         End of uart.h
*
********************************************************************************
*/
#endif

