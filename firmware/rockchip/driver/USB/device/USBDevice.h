/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: USBDevice.h
Desc: 

Author: 
Date: 13-07-26
Notes:

$Log: $
 *
 *
*/

#ifndef _USBDEVICE_H
#define _USBDEVICE_H

/*-------------------------------- Includes ----------------------------------*/

#include    "SysConfig.h"

/*------------------------------ Global Defines ------------------------------*/

// USB支持类别定义
#ifdef USB_MSC
#define         USB_CLASS_TYPE_MSC          (1 << 0)
#else
#define         USB_CLASS_TYPE_MSC           0
#endif

#ifdef USB_MTP
#define         USB_CLASS_TYPE_MTP          (1 << 1)
#else
#define         USB_CLASS_TYPE_MTP           0
#endif

#ifdef USB_AUDIO
#define         USB_CLASS_TYPE_AUDIO        (1 << 2)
#else
#define         USB_CLASS_TYPE_AUDIO         0
#endif

#ifdef USB_SERIAL
#define         USB_CLASS_TYPE_SERIAL       (1 << 3)
#else
#define         USB_CLASS_TYPE_SERIAL        0
#endif

#define         USB_CLASS_TYPE_UPGRADE      (1 << 4)


#define UDC_EV_OUT_PKT_RCV                              (1<<0)
#define UDC_EV_OUT_XFER_COMPL                           (1<<1)
#define UDC_EV_IN_XFER_COMPL                            (1<<2)
#define UDC_EV_SETUP_DONE                               (1<<3)
#define UDC_EV_ENUM_DONE                                (1<<4)
#define UDC_EV_CLEAN_EP                                 (1<<5)
#define UDC_EV_IN_XFER_REDO                             (1<<6)
#define UDC_EV_IN_XFER_TIMEOUT                          (1<<7)

#define EP0_PACKET_SIZE    		                        64


// defined USB device classes
#define USB_DEVICE_CLASS_RESERVED           			0x00
#define USB_DEVICE_CLASS_AUDIO              			0x01	//音频设备
#define USB_DEVICE_CLASS_COMMUNICATIONS     		    0x02	//通讯设备
#define USB_DEVICE_CLASS_HUMAN_INTERFACE    		    0x03	//人机接口
#define USB_DEVICE_CLASS_MONITOR            			0x04	//显示器
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE 	        0x05	//物理接口
#define USB_DEVICE_CLASS_POWER              			0x06	//电源
#define USB_DEVICE_CLASS_PRINTER            			0x07	//打印机
#define USB_DEVICE_CLASS_STORAGE            			0x08	//海量存储
#define USB_DEVICE_CLASS_HUB                			0x09	//HUB
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC    		    0xFF	//

//define	USB subclass
#define USB_SUBCLASS_CODE_RBC                           0x01
#define USB_SUBCLASS_CODE_SFF8020I                      0x02
#define USB_SUBCLASS_CODE_QIC157                        0x03
#define USB_SUBCLASS_CODE_UFI                           0x04
#define USB_SUBCLASS_CODE_SFF8070I                      0x05
#define USB_SUBCLASS_CODE_SCSI                          0x06

//define	USB protocol
#define USB_PROTOCOL_CODE_CBI0                          0x00
#define USB_PROTOCOL_CODE_CBI1                          0x01
#define USB_PROTOCOL_CODE_BULK                          0x50

#define FS_BULK_RX_SIZE                                 64
#define FS_BULK_TX_SIZE                                 64
#define HS_BULK_RX_SIZE                                 512
#define HS_BULK_TX_SIZE                                 512

#define HS_INT_MAXPACKET                                0x08

/*------------------------------ Global Typedefs -----------------------------*/

/**
 * States of EP0.
 */
typedef enum tagEP0_STATE
{
	EP0_DISCONNECT,		/* no host */
	EP0_IDLE,
	EP0_IN_DATA_PHASE,
	EP0_OUT_DATA_PHASE,
	EP0_IN_STATUS_PHASE,
	EP0_OUT_STATUS_PHASE,
	EP0_STALL
} EP0_STATE;


//设备请求结构
typedef struct _USB_CTRL_REQUEST
{
    uint8   bRequestType;
    uint8   bRequest;   
    uint16  wValue;
    uint16  wIndex;
    uint16  wLength;
} USB_CTRL_REQUEST, *pUSB_CTRL_REQUEST;


typedef  void (*EP0_COMPLETE)(void*);

typedef struct tagUSB_EP0_REQ
{
    uint16  NeedLen;
    uint16  XferLen;
    uint8   *buf;
    EP0_COMPLETE complete;
    //uint32  ret; 
    uint8   dir;        //0: host to device, 1: device to host
} USB_EP0_REQ, *pUSB_EP0_REQ;


typedef	struct tagUSB_DEVICE
{
    uint8             speed;
    uint16            ClassType;
    uint8             ConfigsLen;
    void               *HSConfigs;
    void               *FSConfigs;
    int32              (*setup)(USB_CTRL_REQUEST *, void *);
    void               (*suspend)(void *);
    void               (*resume)(void *);
    void               (*disconnect)(void *);
    void               (*request)(uint8, uint32, uint32, void *);
    void              *pFunDev;
    struct tagUSB_DEVICE    *next;
    struct tagUSB_DEVICE    *prev;
    
}USB_DEVICE, *pUSB_DEVICE;

typedef struct tagUDC_DRIVER
{
    uint8 ep0state;
    uint8 connected;
    uint8 suspend;
    uint8 RemoteWakeup;
    uint8 DevNum;
    uint32 ClassTypeMask;
    USB_EP0_REQ ep0req;
    USB_CTRL_REQUEST CtrlRequest;
    pUSB_DEVICE pDev;

}UDC_DRIVER, *pUDC_DRIVER;


/*----------------------------- External Variables ---------------------------*/



/*------------------------- Global Function Prototypes -----------------------*/

extern void USBEnableEp(uint8 epnum, USB_ENDPOINT_DESCRIPTOR *desc);
extern uint32 USBWriteEp0(uint16 len, void* buf);
extern uint32 USBReadEp(uint8 epnum, uint16 len, void * buf);
extern uint32 USBWriteEp(uint8 epnum, uint16 len, void * buf);
extern void USBResetToggle(uint8 dir, uint8 epnum);
extern int32 USBStartEp0Xfer(USB_EP0_REQ *req);
extern void USBStallEp(uint8 epnum);
extern void USBClearNak(uint8 epnum);
extern void USBSetNak(uint8 epnum);
extern void USBInEp0Ack(void);
extern uint8 USBGetEpSts(uint8 dir, uint8 epnum);
extern void USBSetEpSts(uint8 dir, uint8 epnum, uint8 status);

extern int32 USBDriverInit(void);
extern int32 USBConnect(bool FullSpeed);
extern int32 USBDriverProbe(USB_DEVICE *pDev);
extern int32 USBReConnect(uint32 param);
extern void USBDisconnect(void);
extern int32 USBWaitConnect(int32 timeout);

extern void USBDeviceSetup(pUDC_DRIVER pUDC);
extern pUSB_EP0_REQ USBGetEp0Req(void);
extern void USBDeviceIsr(void);

#endif
