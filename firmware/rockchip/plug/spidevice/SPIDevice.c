/*
********************************************************************************************
*
*                Copyright (c): 2014 - 2014 + 5, Aaron.sun
*                             All rights reserved.
*
* FileName: ..\Driver\Uart\UartDevice.c
* Owner: Chad.ma
* Date: 2016.03.16
* Time: 13:51:36
* Desc: SPI Device Class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2016.03.16   1.0
********************************************************************************************
*/

#define __DRIVER_SPI_SPIDEVICE_C__
#ifdef __DRIVER_SPI_SPIDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "SysInclude.h"

#include "device.h"
#include "SPIDevice.h"
#include "DriverInclude.h"
#include "interrupt.h"
#include "spi.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

typedef enum _SPI_DEV_STATUS
{
    SPI_DEV_IDLE,
    SPI_DEV_LOCKED,
    SPI_DEV_NUM

} SPI_DEV_STATUS;

typedef  struct _RX_TX_ITEM
{
    uint8* buffer;
    uint32 size;
    uint32 NeedTransLen;

} RX_TX_ITEM;

typedef struct _SPI_CONFIG
{
    uint32 SpiRate;
    uint32 CtrlMode;

} SPI_CONFIG;


typedef  struct _SPI_DEVICE_CLASS
{
    DEVICE_CLASS stSpiDevice;
    uint32 CurCh_portid;
    SPI_CONFIG stConfig[SPI_MAX_CH];

    RX_TX_ITEM stRxItem;
    RX_TX_ITEM stTxItem;

    uint32 dwRxTxStatus;
    uint32 mode;
    pSpiRxTx_complete SpiRxTx_complete;

} SPI_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SPI_DEVICE_CLASS * gpstSpiDevISR[SPI_DEVICE_MAX];
SPI_DEVICE_CLASS stSpiDev;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


uint32 gSpiSendOver;
_ATTR_DRIVERLIB_CODE_
void SpiDevIntIsr(uint32 DevID)
{
    uint32 spiIntType, realsize;
    spiIntType = SpiGetInt(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID);
    printf("\n spiInttype = %d \n", spiIntType);

    if (gpstSpiDevISR[DevID] != NULL)
    {
        if (spiIntType & SPI_RX_FIFO_FULL)
        {
            printf("SPI_RX_FIFO_FULL int\n");
            SpiDisalbeRxInt(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID);
#if 0

            if ( (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_SYNC_PIO_READ)
                 || (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_ASYNC_PIO_READ))
            {
                if (gpstSpiDevISR[DevID]->stRxItem.NeedTransLen)
                {
                    realsize = SPIReadFIFO(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID,
                                           gpstSpiDevISR[DevID]->stRxItem.buffer,
                                           gpstSpiDevISR[DevID]->stRxItem.NeedTransLen);
                    gpstSpiDevISR[DevID]->stRxItem.buffer += realsize;
                    gpstSpiDevISR[DevID]->stRxItem.NeedTransLen -= realsize;

                    if (gpstSpiDevISR[DevID]->stRxItem.NeedTransLen == 0)
                    {
                        SpiDisalbeRxInt(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID);
                    }
                }
            }

#endif
        }

        if (spiIntType & SPI_TX_FIFO_OVERFLOW)
        {
            printf("SPI_TX_FIFO_OVERFLOW int trigger\n");
            SpiDisalbeTxInt(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID);
        }

        if (spiIntType & SPI_TX_FIFO_EMPTY)
        {
            printf("SPI_TX_FIFO_EMPTY int trigger\n");
            SpiDisalbeTxInt(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID);
#if 0

            if ((gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                || (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_SYNC_PIO_WRITE))
            {
                if (gpstSpiDevISR[DevID]->stTxItem.NeedTransLen)
                {
                    realsize = SPIWriteFIFO(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID, gpstSpiDevISR[DevID]->stTxItem.buffer, gpstSpiDevISR[DevID]->stTxItem.NeedTransLen);
                    gpstSpiDevISR[DevID]->stTxItem.buffer += realsize;
                    gpstSpiDevISR[DevID]->stTxItem.NeedTransLen -= realsize;

                    if (gpstSpiDevISR[DevID]->stTxItem.NeedTransLen == 0)
                    {
                        if (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                        {
                            gpstSpiDevISR[DevID]->dwRxTxStatus = DEVICE_STATUS_IDLE;
                        }
                        else if (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_SYNC_PIO_WRITE)
                        {
                            gpstSpiDevISR[DevID]->dwRxTxStatus = DEVICE_STATUS_IDLE;
                        }

                        SpiDisalbeTxInt(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID);
                        gSpiSendOver = 1;
                        printf("int isr over");
                    }
                }
            }

#endif
        }
    }
}

_ATTR_DRIVERLIB_CODE_
void SpiDevIntIsr0(void)
{
    SpiDevIntIsr(0);
}

_ATTR_DRIVERLIB_CODE_
void SpiDevIntIsr1(void)
{
    SpiDevIntIsr(1);
}

_ATTR_DRIVERLIB_CODE_
rk_size_t SpiDev_Write(HDC dev, uint8* buffer,
                          uint32 size, uint32 mode,
                          eSPI_TRANSFER_MODE_t  transf_mode,
                          pSpiRxTx_complete Tx_complete)
{
    rk_size_t realsize;
    SPI_DEVICE_CLASS * pstSpiDev = &stSpiDev;
    uint32 spi_ch;

    gSpiSendOver = 0;
    spi_ch = ((DEVICE_CLASS*)pstSpiDev)->DevID;

    if (pstSpiDev == NULL)
    {
        return RK_ERROR;
    }

    if (spi_ch > SPI_MAX_CH)
    {
        return RK_ERROR;
    }

    if (mode == SYNC_MODE)
    {
        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_SYNC_PIO_WRITE;

        pstSpiDev->stTxItem.buffer = (uint8*) buffer;
        pstSpiDev->stTxItem.size   = size;
        pstSpiDev->stTxItem.NeedTransLen = size;

        SpiEnableChannel(spi_ch, 0);

        SPIPioWriteEnable(spi_ch,transf_mode);
        //DEBUG();
        //Delay100cyc(100);
        realsize = SPIWriteFIFO0(spi_ch,
                                pstSpiDev->stTxItem.buffer,
                                pstSpiDev->stTxItem.NeedTransLen);


        if (realsize == pstSpiDev->stTxItem.NeedTransLen)
        {
            pstSpiDev->dwRxTxStatus = DEVICE_STATUS_IDLE;
            SpiWaitIdle(spi_ch);
            //DEBUG();
            SpiDisableChannel(spi_ch, spi_ch);
            return size;
        }
        else
        {
            pstSpiDev->stTxItem.buffer += realsize;
            pstSpiDev->stTxItem.NeedTransLen -= realsize;
        }

//        SpiGetInt(((DEVICE_CLASS*)pstSpiDev)->DevID);
//        SPIClearAllInt(((DEVICE_CLASS*)pstSpiDev)->DevID);
//
//        SpiEnableTxInt(((DEVICE_CLASS*)pstSpiDev)->DevID);

        while (pstSpiDev->stTxItem.NeedTransLen)
        {
            SPIPioWriteEnable(spi_ch,transf_mode);
            realsize = SPIWriteFIFO0(spi_ch,
                                    pstSpiDev->stTxItem.buffer,
                                    pstSpiDev->stTxItem.NeedTransLen);


            pstSpiDev->stTxItem.buffer += realsize;
            pstSpiDev->stTxItem.NeedTransLen -= realsize;
        }

        SpiDisableChannel(spi_ch, spi_ch);
        return size;
    }
    return NULL;
}

_ATTR_DRIVERLIB_CODE_
rk_size_t SpiDev_Read(HDC dev, uint8* buffer, uint32 size, uint32 Mode, pSpiRxTx_complete pfRxComplete)
{
    rk_size_t realsize;
    SPI_DEVICE_CLASS * pstSpiDev = &stSpiDev;
    uint32 spi_ch;
    uint32 spi_portid;

    spi_ch = ((DEVICE_CLASS*)pstSpiDev)->DevID;

    if (pstSpiDev == NULL)
    {
        return RK_ERROR;
    }

    if (spi_ch > SPI_MAX_CH)
    {
        return RK_ERROR;
    }

    if (Mode == SYNC_MODE)
    {
        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_SYNC_PIO_READ;
        pstSpiDev->stRxItem.buffer = (uint8*) buffer;
        pstSpiDev->stRxItem.size   = size;
        pstSpiDev->stRxItem.NeedTransLen = size;
        SpiEnableChannel(spi_ch, 0);
        SPIPioReadEnable(spi_ch, pstSpiDev->stRxItem.NeedTransLen);
        realsize = SPIReadFIFO0(spi_ch,
                                pstSpiDev->stRxItem.buffer,
                                pstSpiDev->stRxItem.NeedTransLen);

        if (realsize == pstSpiDev->stRxItem.NeedTransLen)
        {
            pstSpiDev->dwRxTxStatus = DEVICE_STATUS_IDLE;
            SpiWaitIdle(spi_ch);
            SpiDisableChannel(spi_ch, spi_ch);
            return size;
        }
        else
        {
            pstSpiDev->stRxItem.buffer += realsize;
            pstSpiDev->stRxItem.NeedTransLen -= realsize;
        }

        SpiGetInt(spi_ch);
        SpiEnableRxInt(spi_ch);

        while (pstSpiDev->stRxItem.NeedTransLen)
        {
            SPIPioReadEnable(spi_ch, pstSpiDev->stRxItem.NeedTransLen);
            realsize = SPIReadFIFO0(spi_ch,
                                    pstSpiDev->stTxItem.buffer,
                                    pstSpiDev->stTxItem.NeedTransLen);


            pstSpiDev->stRxItem.buffer += realsize;
            pstSpiDev->stRxItem.NeedTransLen -= realsize;
        }

        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_IDLE;
        SpiWaitIdle(spi_ch);
        SpiDisableChannel(spi_ch, spi_ch);
        return size;
    }
    else if (Mode == ASYNC_MODE)
    {
    }

    return NULL;
}

//------------------------------------------------------------------------------
_ATTR_DRIVERLIB_CODE_
void SPI_DMAinit()
{
    //open  clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DmaEnableInt(SPI_DMA_CHANNAL);
}

_ATTR_DRIVERLIB_CODE_
static
void Spi_DMAWrite_Cb()
{
    DmaDisableInt(SPI_DMA_CHANNAL);
    printf("## Spi_DMAWrite_Cb in ##\n");
}

_ATTR_DRIVERLIB_CODE_
rk_size_t SpiDev_DMA_Write(HDC dev, uint8* buffer,
                                 uint32 size,uint32 mode,
                                 eSPI_TRANSFER_MODE_t  transf_mode,
                                 pSpiRxTx_complete Tx_complete)
{
    rk_size_t realsize;
    SPI_DEVICE_CLASS * pstSpiDev = &stSpiDev;

    uint32 spi_ch;
    uint32 spi_port;
    uint32 TX_FIFO_ADDR;
    gSpiSendOver = 0;

    spi_ch = ((DEVICE_CLASS*)pstSpiDev)->DevID;
    TX_FIFO_ADDR = SpiGetTxFIFOaddr(spi_ch);

    spi_port = pstSpiDev->CurCh_portid;
    DEBUG("SPI CHANNAL = %d,port id = %d",spi_ch,spi_port);

    if (pstSpiDev == NULL)
    {
        return RK_ERROR;
    }

    if (spi_ch > SPI_MAX_CH)
    {
        return RK_ERROR;
    }

//    //open  clk
//    ScuClockGateCtr(HCLK_DMA_GATE, 1);
//
//    //open rst ip
//    ScuSoftResetCtr(SYSDMA_SRST, 1);
//    DelayMs(1);
//    ScuSoftResetCtr(SYSDMA_SRST, 0);
//    IntPendingClear(INT_ID_DMA);
//    IntEnable(INT_ID_DMA);
//
//    DmaEnableInt(SPI_DMA_CHANNAL);

    if(SYNC_MODE == mode)
    {
        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_SYNC_DMA_WRITE;

        pstSpiDev->stTxItem.buffer = (uint8*) buffer;
        pstSpiDev->stTxItem.size   = size;
        pstSpiDev->stTxItem.NeedTransLen = size;

        SpiEnableChannel(spi_ch, spi_port);
        SPIDmaWriteEnable(spi_ch,transf_mode);

        if(SPI_CH0 == spi_ch)
        {
            DmaStart(SPI_DMA_CHANNAL, (uint32)buffer,
                    TX_FIFO_ADDR,size,
                    &SPI0ControlDmaCfg_TX, Spi_DMAWrite_Cb);
        }
        else
        {
            DmaStart(SPI_DMA_CHANNAL, (uint32)buffer,
                    TX_FIFO_ADDR,size,
                    &SPI1ControlDmaCfg_TX, Spi_DMAWrite_Cb);
        }



        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_IDLE;
        SpiWaitIdle(((DEVICE_CLASS*)pstSpiDev)->DevID);
        SpiDisableChannel(((DEVICE_CLASS*)pstSpiDev)->DevID, 0);
        return size;
    }
}

_ATTR_DRIVERLIB_CODE_
static
void Spi_DMARead_Cb()
{
    DmaDisableInt(SPI_DMA_CHANNAL);
    printf("## Spi_DMARead_Cb in ##\n");
}

_ATTR_DRIVERLIB_CODE_
rk_size_t SpiDev_DMA_Read(HDC dev, uint8* buffer,
                                 uint32 size,uint32 mode,
                                 eSPI_TRANSFER_MODE_t  transf_mode,
                                 pSpiRxTx_complete Rx_complete)
{
    rk_size_t realsize;
    SPI_DEVICE_CLASS * pstSpiDev = &stSpiDev;

    uint32 spi_ch;
    uint32 RX_FIFO_ADDR;
    gSpiSendOver = 0;

    spi_ch = ((DEVICE_CLASS*)pstSpiDev)->DevID;
    RX_FIFO_ADDR = SpiGetRxFIFOaddr(spi_ch);
    DEBUG("SPI CHANNAL = %d",spi_ch);

    if (pstSpiDev == NULL)
    {
        return RK_ERROR;
    }

    if (spi_ch > SPI_MAX_CH)
    {
        return RK_ERROR;
    }

    if(SYNC_MODE == mode)
    {
        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_SYNC_DMA_READ;

        pstSpiDev->stRxItem.buffer = (uint8*) buffer;
        pstSpiDev->stRxItem.size   = size;
        pstSpiDev->stRxItem.NeedTransLen = size;

        SpiEnableChannel(spi_ch, 0);
        SPIDmaReadEnable(spi_ch,size);

        if(SPI_CH0 == spi_ch)
        {
            DmaStart(SPI_DMA_CHANNAL, RX_FIFO_ADDR,
                    (uint32)buffer,size,
                    &SPI0ControlDmaCfg_RX, Spi_DMARead_Cb);
        }
        else
        {
            DmaStart(SPI_DMA_CHANNAL, RX_FIFO_ADDR,
                    (uint32)buffer,size,
                    &SPI1ControlDmaCfg_RX, Spi_DMARead_Cb);
        }



        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_IDLE;
        SpiWaitIdle(((DEVICE_CLASS*)pstSpiDev)->DevID);
        SpiDisableChannel(((DEVICE_CLASS*)pstSpiDev)->DevID, 0);
        return size;
    }
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
INIT FUN rk_err_t SpiDevResume(HDC dev)
{
    return RK_SUCCESS;
}
_ATTR_DRIVERLIB_CODE_
INIT FUN rk_err_t SpiDevSuspend(HDC dev)
{
    return RK_SUCCESS;
}

_ATTR_DRIVERLIB_CODE_
INIT FUN rk_err_t SpiDevInit(SPI_DEVICE_CLASS * pstSpiDev)
{
    uint32 spi_ch, spi_port;
    spi_ch = ((DEVICE_CLASS*)pstSpiDev)->DevID;
    spi_port = pstSpiDev->CurCh_portid;

    DEBUG("########## SPI_ID = %d, spi_ch = %d #########",spi_ch,spi_port);

    /*spi clockgate init*/
    //SPI_HWInit(spi_id);
    //DEBUG("1. HWINIT OVER");

    /*spi gpio init*/
    SPI_GPIO_Init(spi_port);
    DEBUG("1. GPIO INIT OVER");
    /*spi peremeter setting*/

    SPI_HWInit(spi_ch);
    DEBUG("2. HWINIT OVER");

    SPIInit(spi_ch, 0, pstSpiDev->stConfig[spi_ch].SpiRate,
            pstSpiDev->stConfig[spi_ch].CtrlMode);
    DEBUG("3. SPI baudrate set");

//    SPI_HWInit(spi_id);
//    DEBUG("3. HWINIT OVER");

    /*regester spi interupt*/
    if (spi_ch == SPI_CH0)
    {
        IntRegister(INT_ID_SPI0,SpiDevIntIsr0);
        IntPendingClear(INT_ID_SPI0);
        IntEnable(INT_ID_SPI0);
    }
    else if (spi_ch == SPI_CH0)
    {
        IntRegister(INT_ID_SPI1,SpiDevIntIsr1);
        IntPendingClear(INT_ID_SPI1);
        IntEnable(INT_ID_SPI1);
    }

    return RK_SUCCESS;
}


_ATTR_DRIVERLIB_CODE_
INIT API HDC SpiDev_Create(uint32 DevID, void *arg)
{
    DEVICE_CLASS * pstDev;
    SPI_DEVICE_CLASS * pstSpiDev;
    SPI_DEV_ARG * pstSpiArg = (SPI_DEV_ARG *)arg;

    if (arg == NULL)
    {
        return (HDC)(RK_PARA_ERR);
    }

    if (pstSpiDev == NULL)
    {
        return pstSpiDev;
    }

    pstSpiDev = &stSpiDev;

    if (pstSpiDev == NULL)
    {
        return pstSpiDev;
    }

    pstDev = (DEVICE_CLASS *)pstSpiDev;
    pstDev->suspend = SpiDevSuspend;
    pstDev->resume  = SpiDevResume;
    pstDev->DevID = DevID;
    pstSpiDev->CurCh_portid = pstSpiArg->Portid;
    pstSpiDev->stConfig[pstDev->DevID].SpiRate = pstSpiArg->SpiRate;
    pstSpiDev->stConfig[pstDev->DevID].CtrlMode = pstSpiArg->CtrlMode;
    //pstSpiDev->CurCh = -1;
    gpstSpiDevISR[DevID] = pstSpiDev;
    SpiDevInit(pstSpiDev);
    return pstDev;
}


////////////////////////////////////////////////////////////////////////////////

//test example
_ATTR_DRIVERLIB_CODE_
void SpiLcd_init()
{
    SPI_DEV_ARG stSpiArg;
    stSpiArg.Portid = SPI_CH0_PA;
    stSpiArg.CtrlMode = SPI_CTL_TXRX_MASTER;//SPI_CTL_MODE;
    stSpiArg.SpiRate = 12000 * 1000;
    SpiDev_Create(SPI_CH0, &stSpiArg);
}

_ATTR_DRIVERLIB_CODE_
void SpiLcd_Write(uint8 * dataBuf, uint32 data_size)
{
    #if 0   //pipo mode
    SpiDev_Write(NULL, dataBuf, data_size, SYNC_MODE,SPI_XFM_TX, NULL);
    #else   // dma mode
    SPI_DMAinit();
    SpiDev_DMA_Write(NULL,dataBuf,data_size,SYNC_MODE,SPI_TRANSMIT_ONLY,NULL);
    #endif
}

////////////////////////////////////////////////////////////////////////////////
#endif
