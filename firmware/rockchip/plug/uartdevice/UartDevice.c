/*
********************************************************************************************
*
*                Copyright (c): 2014 - 2014 + 5, Aaron.sun
*                             All rights reserved.
*
* FileName: ..\Driver\Uart\UartDevice.c
* Owner: Aaron.sun
* Date: 2014.2.14
* Time: 13:51:36
* Desc: Uart Device Class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.2.14     13:51:36   1.0
********************************************************************************************
*/

//#include "BspConfig.h"
#define __DRIVER_UART_UARTDEVICE_C__
#ifdef __DRIVER_UART_UARTDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "SysInclude.h"

#if 1//def _UART_DEBUG_
#include "device.h"
#include "UartDevice.h"
#include "DriverInclude.h"
#include "interrupt.h"

#ifdef USE_USBSERIAL
#include "USBSerial.h"
#endif



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _DRIVER_UART_UARTDEVICE_READ_  
#define _DRIVER_UART_UARTDEVICE_WRITE_ 
#define _DRIVER_UART_UARTDEVICE_INIT_  
#define _DRIVER_UART_UARTDEVICE_SHELL_  


typedef  struct _TX_ITEM
{
    uint8* buffer;
    uint32 size;
    uint32 mode;
    pUartTx_complete UartTx_complete;

}TX_ITEM;


typedef  struct _RX_ITEM
{
    uint8* buffer;
    uint32 size;

}RX_ITEM;


typedef  struct _UART_DEVICE_CLASS
{
    uint32 devID;
    uint32 dwBitRate;
    uint32 dwBitWidth;
    uint32           dwTxNeedTransLen;
    uint32           dwRxNeedTransLen;
    uint32           dwTxStatus;
    uint32           dwRxStatus;
    TX_ITEM          stTxItem;
    RX_ITEM          stRxItem;

}UART_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static UART_DEVICE_CLASS * gpstUartDevISR[UART_CH_MAX];
UART_DEVICE_CLASS stUartDev;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
uint8 SerialConnected = 0;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
void UartIntIsr0(void);
rk_err_t UartDevResume(HDC dev);
rk_err_t UartDevSuspend(HDC dev);
rk_err_t UartDevDelete(HDC dev);
rk_err_t UartDevDeInit(UART_DEVICE_CLASS * pstUartDev);
rk_err_t UartDevInit(UART_DEVICE_CLASS * pstUartDev);
void UartIntIsr(uint32 UartDevID);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*******************************************************************************
** Name: UartDevRead
** Input:DEVICE_CLASS* dev,uint32 pos, void* buffer, uint32 size,uint8 mode,pRx_indicate Rx_indicate
** Return: rk_size_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:48:03
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_READ_
READ API rk_size_t UartDev_Read(HDC dev, void* buffer, uint32 size)
{
    rk_size_t realsize;

    UART_DEVICE_CLASS * pstUartDev =  &stUartDev;
    if (pstUartDev == NULL)
    {
        return RK_ERROR;
    }

    pstUartDev->dwRxStatus = DEVICE_STATUS_SYNC_PIO_READ;

    pstUartDev->stRxItem.buffer = (uint8*) buffer;
    pstUartDev->stRxItem.size   = size;
    pstUartDev->dwRxNeedTransLen = size;

    if(pstUartDev->devID == 0)
    {
        realsize = UARTReadByte(DEBUG_UART_PORT,pstUartDev->stRxItem.buffer, pstUartDev->dwRxNeedTransLen);
    }
    else if((pstUartDev->devID == 1) && SerialConnected)
    {
        #ifdef USE_USBSERIAL
        realsize = SrlRead(pstUartDev->dwRxNeedTransLen, pstUartDev->stRxItem.buffer);
        #endif
    }
    else
    {
        pstUartDev->dwRxStatus = DEVICE_STATUS_IDLE;
        return 0;
    }

    if (realsize == pstUartDev->dwRxNeedTransLen)
    {
        pstUartDev->dwRxStatus = DEVICE_STATUS_IDLE;
        return size;
    }
    else
    {
        pstUartDev->stRxItem.buffer += realsize;
        pstUartDev->dwRxNeedTransLen -= realsize;
    }
    //UARTSetIntEnabled(DEBUG_UART_PORT,UART_IE_RX);

    return realsize;
}

#if 0

/*******************************************************************************
** Name: UartIntIsr
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_READ_
void UartIntIsr(uint32 UartDevID)
{
    uint32 uartIntType, realsize;

//    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    uartIntType = UARTGetIntType(0);

    if (gpstUartDevISR[UartDevID] != NULL)
    {
        if (uartIntType & UART_INT_READ_FIFO_NOT_EMPTY)
        {

            if (gpstUartDevISR[UartDevID]->dwRxStatus == DEVICE_STATUS_SYNC_PIO_READ)
            {

                if (gpstUartDevISR[UartDevID]->dwRxNeedTransLen)
                {

                    realsize = UARTReadByte(0,gpstUartDevISR[UartDevID]->stRxItem.buffer, gpstUartDevISR[UartDevID]->dwRxNeedTransLen);

                    gpstUartDevISR[UartDevID]->stRxItem.buffer += realsize;
                    gpstUartDevISR[UartDevID]->dwRxNeedTransLen -= realsize;


                    if (gpstUartDevISR[UartDevID]->dwRxNeedTransLen == 0)
                    {
                        if (gpstUartDevISR[UartDevID]->dwRxStatus == DEVICE_STATUS_SYNC_PIO_READ)
                        {
                        }
                        gpstUartDevISR[UartDevID]->dwRxStatus = DEVICE_STATUS_IDLE;
                        UARTSetIntDisabled(0,UART_IE_RX);
                    }
                }
            }
        }

    }


    if (uartIntType & UART_INT_WRITE_FIFO_EMPTY)
    {

        if ((gpstUartDevISR[UartDevID]->dwTxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                || (gpstUartDevISR[UartDevID]->dwTxStatus == DEVICE_STATUS_SYNC_PIO_WRITE))
        {

            if (gpstUartDevISR[UartDevID]->dwTxNeedTransLen)
            {

                realsize = UARTWriteByte(0,gpstUartDevISR[UartDevID]->stTxItem.buffer, gpstUartDevISR[UartDevID]->dwTxNeedTransLen);

                gpstUartDevISR[UartDevID]->stTxItem.buffer += realsize;
                gpstUartDevISR[UartDevID]->dwTxNeedTransLen -= realsize;


                if (gpstUartDevISR[UartDevID]->dwTxNeedTransLen == 0)
                {

                    if (gpstUartDevISR[UartDevID]->dwTxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                    {
                        if (gpstUartDevISR[UartDevID]->stTxItem.UartTx_complete != NULL)
                        {
                            gpstUartDevISR[UartDevID]->stTxItem.UartTx_complete(gpstUartDevISR[UartDevID]->stTxItem.buffer);
                        }
                        UARTSetIntDisabled(0,UART_IE_TX);
                    }

                    else if (gpstUartDevISR[UartDevID]->dwTxStatus == DEVICE_STATUS_SYNC_PIO_WRITE)
                    {
                    }

                    gpstUartDevISR[UartDevID]->dwTxStatus = DEVICE_STATUS_IDLE;

                }
            }

        }

    }
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UartIntIsr0
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.4.11
** Time: 15:55:17
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_READ_
READ FUN void UartIntIsr0(void)
{
    UartIntIsr(0);
}

#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UartDevWrite
** Input:DEVICE_CLASS* dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_size_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:46:51
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_WRITE_
WRITE API rk_size_t UartDev_Write(HDC dev,const void* buffer,
                    uint32 size, uint8 mode, pUartTx_complete Tx_complete)
{

    rk_size_t realsize;

    UART_DEVICE_CLASS * pstUartDev = &stUartDev;
    if (pstUartDev == NULL)
    {
        return RK_ERROR;
    }


    if (mode == SYNC_MODE)
    {
//        xSemaphoreTake(pstUartDev->osUartWriteReqSem, portMAX_DELAY);

//        portENTER_CRITICAL();

        pstUartDev->dwTxStatus = DEVICE_STATUS_SYNC_PIO_WRITE;

        pstUartDev->stTxItem.buffer = (uint8*) buffer;
        pstUartDev->stTxItem.size   = size;
        pstUartDev->dwTxNeedTransLen = size;
//        portEXIT_CRITICAL();

        if(pstUartDev->devID == 0)
        {
            realsize = UARTWriteByte0(DEBUG_UART_PORT,pstUartDev->stTxItem.buffer, pstUartDev->dwTxNeedTransLen);
        }
        else if(pstUartDev->devID == 1)
        {
            #ifdef USE_USBSERIAL
            realsize = SrlWrite(pstUartDev->dwTxNeedTransLen, pstUartDev->stTxItem.buffer);
            #endif
        }

        if (realsize == pstUartDev->dwTxNeedTransLen)
        {
            pstUartDev->dwTxStatus = DEVICE_STATUS_IDLE;
//            xSemaphoreGive(pstUartDev->osUartWriteReqSem);
            return size;
        }
        else
        {
            pstUartDev->stTxItem.buffer += realsize;
            pstUartDev->dwTxNeedTransLen -= realsize;
        }

        while(pstUartDev->dwTxNeedTransLen)
        {
            if(pstUartDev->devID == 0)
            {
                realsize = UARTWriteByte0(DEBUG_UART_PORT,pstUartDev->stTxItem.buffer, pstUartDev->dwTxNeedTransLen);
            }
            else if((pstUartDev->devID == 1) && SerialConnected)
            {
                #ifdef USE_USBSERIAL
                realsize = SrlWrite(pstUartDev->dwTxNeedTransLen, pstUartDev->stTxItem.buffer);
                SrlBulkInPkt();
                #endif
            }
            else
            {
                return 0;
            }

            pstUartDev->stTxItem.buffer += realsize;
            pstUartDev->dwTxNeedTransLen -= realsize;
        }

        //UARTSetIntEnabled(DEBUG_UART_PORT,UART_IE_TX);
        return size;
    }
    else if (mode == ASYNC_MODE)
    {

        pstUartDev->dwTxStatus = DEVICE_STATUS_ASYNC_PIO_WRITE;
        pstUartDev->stTxItem.buffer = (uint8*) buffer;

        pstUartDev->stTxItem.size   = size;
        pstUartDev->stTxItem.UartTx_complete = Tx_complete;
        pstUartDev->dwTxNeedTransLen = size;

        realsize = UARTWriteByte0(0,pstUartDev->stTxItem.buffer, pstUartDev->dwTxNeedTransLen);

        if (realsize == pstUartDev->dwTxNeedTransLen)
        {
            pstUartDev->dwTxStatus = DEVICE_STATUS_IDLE;
            return size;
        }
        else
        {
            pstUartDev->stTxItem.buffer += realsize;
            pstUartDev->dwTxNeedTransLen -= realsize;
        }
        //UARTSetIntEnabled(0,UART_IE_TX);
        return realsize;

    }
    return NULL;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UartDevCreate
** Input:void
** Return: DEVICE_CLASS *
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:40:31
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
INIT API HDC UartDev_Create(uint32 DevID, void *arg)
{
    DEVICE_CLASS * pstDev;
    UART_DEVICE_CLASS * pstUartDev;
    UART_DEV_ARG * pstUartArg = (UART_DEV_ARG *)arg;

    if (arg == NULL)
    {
        return (HDC)(RK_PARA_ERR);
    }

    //pstUartDev =  pvPortMalloc(sizeof(UART_DEVICE_CLASS));
    pstUartDev = &stUartDev;

    if (pstUartDev == NULL)
    {
        return pstUartDev;
    }

    pstDev = (DEVICE_CLASS *)pstUartDev;

    pstDev->suspend = UartDevSuspend;
    pstDev->resume  = UartDevResume;

    pstUartDev->dwRxStatus = DEVICE_STATUS_IDLE;
    pstUartDev->dwTxStatus = DEVICE_STATUS_IDLE;
    pstUartDev->dwBitRate = pstUartArg->dwBitRate;
    pstUartDev->dwBitWidth = pstUartArg->dwBitWidth;
    pstUartDev->devID = DevID;

    gpstUartDevISR[DevID] = pstUartDev;
//    UartDevGpioInit(DevID);
    UartDevInit(pstUartDev);

    return pstDev;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UartDevResume
** Input:DEVICE_CLASS *
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:57:19
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDevResume(HDC dev)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UartDevSuspend
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:56:05
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDevSuspend(HDC dev)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UartDevDelete
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:41:18
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDevDelete(HDC dev)
{
    UART_DEVICE_CLASS * pstUartDev = (UART_DEVICE_CLASS *) dev;

    return RK_SUCCESS;
}



/*******************************************************************************
** Name: UartDevDeInit
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:38:30
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDevDeInit(UART_DEVICE_CLASS * pstUartDev)
{
    if (pstUartDev->devID == 0)
    {

        return RK_SUCCESS;
    }
    else
    {
        return RK_SUCCESS;
    }

}

/*******************************************************************************
** Name: UartDevInit
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:44:46
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
rk_err_t UartDevInit(UART_DEVICE_CLASS * pstUartDev)
{

    if (pstUartDev->devID == 0)
    {

        return RK_SUCCESS;
    }
    else
    {
        return RK_SUCCESS;
    }

}

#endif
#endif

