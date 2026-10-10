/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   FsConfig.h
*
* Description:
*
* History:      <author>          <time>        <version>
*                               2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _USBCONFIG_H
#define _USBCONFIG_H

/*
*-------------------------------------------------------------------------------
*
*                           FileSystem Configer
*
*-------------------------------------------------------------------------------
*/

/*
*-------------------------------------------------------------------------------
*
*                           Section define
*
*-------------------------------------------------------------------------------
*/


#include <stdio.h>
#include <string.h>
#include "SysConfig.h"
#include "typedef.h"
#include "macro.h"

#include "hw_nvic.h"

#include "hw_memap.h"
#include "Hook.h"


//#define     USB_DRM_EN
#define     DMA_BULKIN_EN
#define     DMA_BULKOUT_EN

//section defines
//usb ui
#define     _ATTR_USB_UI_CODE_              
#define     _ATTR_USB_UI_DATA_              
#define     _ATTR_USB_UI_BSS_               

//usb audio
#define     _ATTR_USB_AUDIO_CODE_           
#define     _ATTR_USB_AUDIO_DATA_           
#define     _ATTR_USB_AUDIO_BSS_            

//usb msc
#define     _ATTR_USB_MSC_CODE_             
#define     _ATTR_USB_MSC_DATA_             
#define     _ATTR_USB_MSC_BSS_              

//usb msc
#define     _ATTR_USB_SRL_CODE_             
#define     _ATTR_USB_SRL_DATA_             
#define     _ATTR_USB_SRL_BSS_              

//usb control
#define     _ATTR_USBCONTROL_CODE_          
#define     _ATTR_USBCONTROL_DATA_          
#define     _ATTR_USBCONTROL_BSS_           

//usb driver
#define     _ATTR_USB_DRIVER_CODE_          
#define     _ATTR_USB_DRIVER_DATA_          
#define     _ATTR_USB_DRIVER_BSS_           

//USB module compile switch.
#ifdef _USB_
#define USB_MSC

#define USB_IF_TEST                 //Ҫ��USB��֤������Ҫ�򿪴˶���, �򿪴˶���ֻҪUSB��������,�Ͳ��˳�USB
//#define USB_PHY_TEST                //USB PHY���ԣ�������ͼ������
#define USB_SUSPEND_EN           1
#endif

#ifdef  USB_DEBUG
#define UDEBUG(...)  printf(__VA_ARGS__)
#else
#define UDEBUG(...)
#endif

#define USB_DISPLAY

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/
#include "OsInclude.h"

#include "FsConfig.h"
#include "fsinclude.h"

#include "USBReg.h"
#include "USBComm.h"
#include "chap9.h"

#include "USBDevice.h"

#include "storage.h"
#include "USBSerial.h"

#include "MDConfig.h"
//#include "MtpUsb.h"

#ifdef _USB_HOST_
#include "USB_DWCHost.h"
#include "usb_msc_host.h"
#endif


/*
********************************************************************************
*
*                         End of FsConfig.h
*
********************************************************************************
*/
#endif
