/*
********************************************************************************************
*
*  Copyright (c): 2014 - 2014 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: Driver\SPI\SpiDevice.h
* Owner: chad.ma
* Date: 2016.03.16
* Time:
* Desc: SPI DEVICE CLASS
* History:
*    <author>	 <date> 	  <time>	 <version>	   <Desc>
*    Aaron.sun     2016.03.16      1.0
********************************************************************************************
*/

#ifndef __DRIVER_SPI_SPIDEVICE_H__
#define __DRIVER_SPI_SPIDEVICE_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/



typedef rk_err_t (* pSpiRxTx_complete)(void* buffer);      //tx callback funciton


typedef struct _SPI_DEV_ARG
{
    uint32 Portid;
    uint32 SpiRate;
    uint32 CtrlMode;

}SPI_DEV_ARG;

#define SPI_DEVICE_MAX      2
#define SPI_DMA_CHANNAL     2

#define SPI_TX_MODE              ((UINT32)(0x01))
#define SPI_RX_MODE              ((UINT32)(0x02))


#define SPI_CTL_MODE (SPI_MASTER_MODE | TRANSMIT_RECEIVE | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | LITTLE_ENDIAN_MODE | CS_2_SCLK_OUT_1_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_LOW\
                            | SERIAL_CLOCK_PHASE_START | DATA_FRAME_8BIT)

#define SPI_CTL_TXRX_MASTER ( SPI_MASTER_MODE | TRANSMIT_RECEIVE | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | LITTLE_ENDIAN_MODE | CS_2_SCLK_OUT_1_2_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_HIGH\
                            | SERIAL_CLOCK_PHASE_MIDDLE | DATA_FRAME_8BIT)

#define SPI_CTL_TXRX_SLAVE_TEST ( SPI_SLAVE_MODE | TRANSMIT_RECEIVE | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | BIG_ENDIAN_MODE | CS_2_SCLK_OUT_1_2_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_LOW \
                            | SERIAL_CLOCK_PHASE_MIDDLE | DATA_FRAME_8BIT)



#define SPI_BUS_CLK (12 * 1000 * 1000)


DMA_CFGX SPI0ControlDmaCfg_TX  = {DMA_CTLL_SPI0_8_MTX, DMA_CFGL_SPI0_8_MTX, DMA_CFGH_SPI0_8_MTX,0};
DMA_CFGX SPI1ControlDmaCfg_TX  = {DMA_CTLL_SPI1_8_MTX, DMA_CFGL_SPI1_8_MTX, DMA_CFGH_SPI1_8_MTX,0};

DMA_CFGX SPI0ControlDmaCfg_RX  = {DMA_CTLL_SPI0_8_MRX, DMA_CFGL_SPI0_8_MRX, DMA_CFGH_SPI0_8_MRX,0};
DMA_CFGX SPI1ControlDmaCfg_RX  = {DMA_CTLL_SPI1_8_MRX, DMA_CFGL_SPI1_8_MRX, DMA_CFGH_SPI1_8_MRX,0};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t SpiDev_Realse_Channel(HDC dev,  uint32 Ch);
extern rk_err_t SpiDev_SetChannel(HDC dev, uint32 Ch, uint32 SpiRate, uint32 CtrlMode);
extern rk_size_t SpiDev_Read_Write(HDC dev, uint8 * ReadBuf, uint8 * WriteBuf, uint32 Size, uint32 Mode, pSpiRxTx_complete pfRxTxComplete);
extern rk_size_t SpiDev_Write(HDC dev, uint8 * pBuf,
                                uint32 size, uint32 Mode,
                                eSPI_TRANSFER_MODE_t  transf_mode,
                                pSpiRxTx_complete pfTxComplete);

extern rk_size_t SpiDev_Read(HDC dev, uint8 * pBuf, uint32 size, uint32 Mode, pSpiRxTx_complete pfRxComplete);

extern rk_size_t SpiDev_DMA_Write(HDC dev, uint8* buffer,
                                 uint32 size,uint32 mode,
                                 eSPI_TRANSFER_MODE_t  transf_mode,
                                 pSpiRxTx_complete Tx_complete);
extern HDC SpiDev_Create(uint32 DevID, void * arg);



#endif


