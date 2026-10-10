/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                                    (c) Copyright 2013, RockChip.Ltd
*                                          All Rights Reserved
*File    : device.h
* By      : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/
#ifndef _DEVICE_H_
#define _DEVICE_H_

/*
--------------------------------------------------------------------------------
  
                        Macro define  
  
--------------------------------------------------------------------------------
*/
#define DEV_NAME_MAX_SIZE           32
#define DEV_MAX_NUM                 128

//DEVICE_CLASS->flag 
#define RK_CREATED                  0x05

//DEVICE_CLASS->mode
#define READ_ONLY                   ((uint32)(1<<30))
#define WRITE_ONLY                  ((uint32)(1<<29))
#define CTRL_ONLY                   ((uint32)(1<<28))

#define READ_WRITE                  (READ_ONLY | WRITE_ONLY)
#define READ_CTRL                   (READ_ONLY | CTRL_ONLY)
#define WRITE_CTRL                  (WRITE_ONLY | CTRL_ONLY)
#define READ_WRITE_CTRL             (READ_ONLY | WRITE_ONLY | CTRL_ONLY)
#define NOT_CARE                    0

//DEVICE_CLASS->read(write) mode
#define SYNC_MODE               0x00 //SYNC
#define ASYNC_MODE              0x01 //ASYNC physical DEVICE INT and logical device complete(event)

typedef enum _DEVICE_STATUS
{
    DEVICE_STATUS_IDLE = 0,
    DEVICE_STATUS_ASYNC_PIO_READ,
    DEVICE_STATUS_ASYNC_DMA_READ,
    DEVICE_STATUS_SYNC_PIO_READ,
    DEVICE_STATUS_SYNC_DMA_READ,
    DEVICE_STATUS_ASYNC_PIO_WRITE,
    DEVICE_STATUS_ASYNC_DMA_WRITE,
    DEVICE_STATUS_SYNC_PIO_WRITE,
    DEVICE_STATUS_SYNC_DMA_WRITE,
    DEVICE_STATUS_IO_CONTROL,
    DEVICE_STATUS_NUM
    
}DEVICE_STATUS;


/*
--------------------------------------------------------------------------------
  
                        Struct Define
  
--------------------------------------------------------------------------------
*/

typedef void * HDC;


typedef struct _DEVICE_CLASS
{
    struct _DEVICE_CLASS * next;

    uint32 Cnt;
    uint32 SuspendCnt;
    uint32  DevClassID;
    uint32  DevID;
    
    rk_err_t  (*suspend)(HDC dev);
    rk_err_t  (*resume) (HDC dev);
         
}DEVICE_CLASS;

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

HDC RKDev_Open(uint8 DevClassID, uint8 DevID, uint32 DevMode);
rk_err_t RKDev_Close(HDC dev);
rk_err_t RKDev_Create(uint32 DevClassID, uint32 DevID, void * arg);
rk_err_t  RKDev_Suspend(HDC* dev);
rk_err_t  RKDev_Resume(HDC* dev);
rk_err_t  RKDev_Delete(uint32 DevClassID, uint32 DevID);
HDC RK_CompetenceToDev(HDC dev, uint32 Mode);
HDC RK_DevToCompetence(HDC dev, uint32 Mode);

#endif

