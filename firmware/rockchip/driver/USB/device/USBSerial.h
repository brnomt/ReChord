/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: MemDev.h
Desc: 

Author: 
Date: 13-07-26
Notes:

$Log: $
 *
 *
*/

#ifndef _USBSERIAL_H
#define _USBSERIAL_H

/*-------------------------------- Includes ----------------------------------*/
#include    <stdio.h>
#include    <string.h>
#include    "SysConfig.h"

/*------------------------------ Global Defines ------------------------------*/
#define SRL_OPEN_R                      (1<<0)
#define SRL_OPEN_W                      (1<<1)

#define SRL_RX_BUF_LEN                  (128)
#define SRL_TX_BUF_LEN                  (128)

#define CDC_NOTIFICATION_EP                 3

#define CDC_DATAOUT_EP                      2
#define CDC_DATAIN_EP                       2

/*------------------------------ Global Typedefs -----------------------------*/

typedef   void* SRL_HANDLE;

typedef  void (*SRL_COMPLETE)(int32);

typedef __packed struct _CDC_HEADER_DESCRIPTOR
{
    /// Size of this descriptor in bytes.
    uint8 bFunctionLength;
    /// Descriptor type (CDCDescriptors_INTERFACE).
    uint8 bDescriptorType;
    /// Descriptor sub-type (CDCDescriptors_HEADER).
    uint8 bDescriptorSubtype;
    /// USB CDC specification release number.
    uint16 bcdCDC;

} CDC_HEADER_DESCRIPTOR;

//------------------------------------------------------------------------------
/// Configuration descriptor list for a device implementing a CDC serial driver.
//------------------------------------------------------------------------------
typedef __packed struct _CDC_CALLMANAGEMENT_DESCRIPTOR
{
    /// Size of this descriptor in bytes.
    uint8 bFunctionLength;
    /// Descriptor type (CDCDescriptors_INTERFACE).
    uint8 bDescriptorType;
    /// Descriptor sub-type (CDCDescriptors_CALLMANAGEMENT).
    uint8 bDescriptorSubtype;
    /// Configuration capabilities ("CDC CallManagement Capabilities").
    uint8 bmCapabilities;
    /// Interface number of the data class interface used for call management
    /// (optional).
    uint8 bDataInterface;
} CDC_CALLMANAGEMENT_DESCRIPTOR;

//------------------------------------------------------------------------------
/// Describes the command supported by the communication interface class
/// with the Abstract Control Model subclass code.
//------------------------------------------------------------------------------
typedef __packed struct _CDC_ACM_DESCRIPTOR
{
    /// Size of this descriptor in bytes.
    uint8 bFunctionLength;
    /// Descriptor type (CDCDescriptors_INTERFACE).
    uint8 bDescriptorType;
    /// Descriptor subtype (CDCDescriptors_ABSTRACTCONTROLMANAGEMENT).
    uint8 bDescriptorSubtype;
    /// Configuration capabilities.
    /// \sa "CDC ACM Capabilities".
    uint8 bmCapabilities;

} CDC_ACM_DESCRIPTOR; // GCC


//------------------------------------------------------------------------------
/// Describes the relationship between a group of interfaces that can
/// be considered to form a functional unit.
//------------------------------------------------------------------------------
typedef __packed struct _CDC_UNION_DESCRIPTOR
{

    /// Size of the descriptor in bytes.
    uint8 bFunctionLength;
    /// Descriptor type (CDCDescriptors_INTERFACE).
    uint8 bDescriptorType;
    /// Descriptor subtype (CDCDescriptors_UNION).
    uint8 bDescriptorSubtype;
    /// Number of the master interface for this union.
    uint8 bMasterInterface;
    /// Number of the first slave interface for this union.
    uint8 bSlaveInterface0;

} CDC_UNION_DESCRIPTOR; // GCC


typedef __packed struct _CDC_SERIAL_CONFIGURATION_DESCRIPTOR
{

    /// Standard configuration descriptor.
    USB_CONFIGURATION_DESCRIPTOR configuration;
    /// Communication interface descriptor.
    USB_INTERFACE_DESCRIPTOR  communication;
    /// CDC header functional descriptor.
    CDC_HEADER_DESCRIPTOR header;
    /// CDC call management functional descriptor.
    CDC_CALLMANAGEMENT_DESCRIPTOR CallManagement;
    /// CDC abstract control management functional descriptor.
    CDC_ACM_DESCRIPTOR AbstractControlManagement;
    /// CDC union functional descriptor (with one slave interface).
    CDC_UNION_DESCRIPTOR union1;
    /// Notification endpoint descriptor.
    USB_ENDPOINT_DESCRIPTOR notification;
    /// Data interface descriptor.
    USB_INTERFACE_DESCRIPTOR data;
    /// Data OUT endpoint descriptor.
    USB_ENDPOINT_DESCRIPTOR DataOut;
    /// Data IN endpoint descriptor.
    USB_ENDPOINT_DESCRIPTOR DataIn;

}CDC_SERIAL_CONFIGURATION_DESCRIPTOR;


/*----------------------------- External Variables ---------------------------*/


/*------------------------- Global Function Prototypes -----------------------*/

extern int32 SrlInit(void *pArg);
extern void SrlDeInit(void);
extern int32 SrlThread(void);
extern int32 SrlWaitConnect(int32 timeout);

extern SRL_HANDLE SrlOpen(uint32 flag);
extern int32 SrlClose(SRL_HANDLE handle);

extern int32 SrlRead(SRL_HANDLE handle, SRL_COMPLETE complete);
extern int32 SrlReadData(SRL_HANDLE handle, uint32 len, void* buf);
extern int32 SrlWrite(SRL_HANDLE handle, uint32 len, void* buf, SRL_COMPLETE complete);


#endif
