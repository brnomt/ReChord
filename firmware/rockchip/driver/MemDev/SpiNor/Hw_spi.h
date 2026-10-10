/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   hw_spi.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-15          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_SPI_H_
#define _HW_SPI_H_

//SPIM_CTRLR0  SPIS_CTRLR0
#define SPI_MASTER_MODE             (0<<20)
#define SPI_SLAVE_MODE              (1<<20)

#define TRANSMIT_RECEIVE            (0<<18)
#define TRANSMIT_ONLY               (1<<18)
#define RECEIVE_ONLY                (2<<18)

#define MOTOROLA_SPI                (0<<16)
#define TEXAS_INSTRUMENTS_SSP       (1<<16)
#define NATIONAL_SEMI_MICROWIRE     (2<<16)

#define RXD_SAMPLE_NO_DELAY         (0<<14)
#define RXD_SAMPLE_1CK_DELAY        (1<<14)
#define RXD_SAMPLE_2CK_DELAY        (2<<14)
#define RXD_SAMPLE_3CK_DELAY        (3<<14)

#define APB_BYTE_WR                 (1<<13)
#define APB_HALFWORD_WR             (0<<13)

#define MSB_FBIT                    (0<<12)
#define LSB_FBIT                    (1<<12)

#define LITTLE_ENDIAN_MODE          (0<<11)
#define BIG_ENDIAN_MODE             (1<<11)

#define CS_2_SCLK_OUT_1_2_CK        (0<<10)
#define CS_2_SCLK_OUT_1_CK          (1<<10)

#define CS_KEEP_LOW                 (0<<8)
#define CS_KEEP_1_2_CK              (1<<8)
#define CS_KEEP_1_CK                (2<<8)

#define SERIAL_CLOCK_POLARITY_LOW   (0<<7)
#define SERIAL_CLOCK_POLARITY_HIGH  (1<<7)

#define SERIAL_CLOCK_PHASE_MIDDLE   (0<<6)
#define SERIAL_CLOCK_PHASE_START    (1<<6)

#define DATA_FRAME_4BIT             (0)
#define DATA_FRAME_8BIT             (1)
#define DATA_FRAME_16BIT            (2)

///SPIM_SR  SPIS_SR
#define RECEIVE_FIFO_FULL           (1<<4)
#define RECEIVE_FIFO_EMPTY          (1<<3)
#define TRANSMIT_FIFO_EMPTY         (1<<2)
#define TRANSMIT_FIFO_FULL          (1<<1)
#define SPI_BUSY_FLAG               (1)

//SPIM_DMACR SPIS_DMACR
#define TRANSMIT_DMA_ENABLE         (1<<1)
#define RECEIVE_DMA_ENABLE          (1)

//SPI Registers
typedef volatile struct tagSPI_STRUCT
{
    uint32 SPI_CTRLR0;
    uint32 SPI_CTRLR1;
    uint32 SPI_ENR;
    uint32 SPI_SER;
    uint32 SPI_BAUDR;
    uint32 SPI_TXFTLR;
    uint32 SPI_RXFTLR;
    uint32 SPI_TXFLR;
    uint32 SPI_RXFLR;
    uint32 SPI_SR;
    uint32 SPI_IPR;
    uint32 SPI_IMR;
    uint32 SPI_ISR;
    uint32 SPI_RISR;
    uint32 SPI_ICR;
    uint32 SPI_DMACR;
    uint32 SPI_DMATDLR;
    uint32 SPI_DMARDLR;
    uint32 reserved[(0x400 - 0x0048)/4];
    uint32 SPI_TXDR[(0x800 - 0x400)/4];
    uint32 SPI_RXDR[(0xC00 - 0x800)/4];
}SPI_REG,*pSPI_REG;


#define SPI_MASTER_BASE_ADDR           	((UINT32)0x40040000)
#define SPICtl            		        ((SPI_REG *) SPI_MASTER_BASE_ADDR)
//SPI_REG SPICtl;

/*
********************************************************************************
*
*                         End of hw_spi.h
*
********************************************************************************
*/

#endif
