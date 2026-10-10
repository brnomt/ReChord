/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: USBComm.c
Desc:

Author: chenfen@rock-chips.com
Date: 12-06-20
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/

#include "USBConfig.h"
#include "Hw_cru.h"
#ifdef  _USB_

/*------------------------------------ Defines -------------------------------*/

/*----------------------------------- Typedefs -------------------------------*/


/*-------------------------- Forward Declarations ----------------------------*/

//extern UDC_DRIVER UDCDriver;

/* ------------------------------- Globals ---------------------------------- */

//字符串描述符,LangId
_ATTR_USB_DRIVER_DATA_
uint8 USBLangId[]=
{
    0x04,
    0x03,

    0x09, 0x04      //English--0x0409
    //0x04, 0x08           //Chinese--0x0804
};

// string descriptor
_ATTR_USB_DRIVER_DATA_
uint8 StringProduct[] =
{
	18,
	0x03,

	'R', 0x00,
	'O', 0x00,
	'C', 0x00,
	'K', 0x00,
	' ', 0x00,
	'M', 0x00,
	'P', 0x00,
	'3', 0x00
};

// string descriptor
_ATTR_USB_DRIVER_DATA_
uint8 StringManufacture[] =
{
	18,
	0x03,

	'R', 0x00,
	'o', 0x00,
	'c', 0x00,
	'k', 0x00,
	'C', 0x00,
	'h', 0x00,
	'i', 0x00,
	'p', 0x00
};

// string number
_ATTR_USB_DRIVER_DATA_
uint8 StringSerialNumbel[] =
{
	18,
	0x03,

	'U', 0x00,
	'S', 0x00,
	'B', 0x00,
	'V', 0x00,
	'1', 0x00,
	'.', 0x00,
	'0', 0x00,
	'0', 0x00
};


//高速设备限制描述符
_ATTR_USB_DRIVER_DATA_
HS_DEVICE_QUALIFIER HS_Device_Qualifier=
{
	sizeof(HS_DEVICE_QUALIFIER),		//length of HS Device Descriptor
	0x06,							    //HS Device Qualifier Type
	0x0200,					            // USB 2.0 version
	USB_DEVICE_CLASS_STORAGE,		    //Device class
	USB_SUBCLASS_CODE_SCSI,		        // Device SubClass
	USB_PROTOCOL_CODE_BULK, 		    //Device Protocol Code
	FS_BULK_RX_SIZE,				    //Maximum Packet SIze for other speed
	0x01,							    //Number of Other speed configurations
	0x00							    //Reserved
};

_ATTR_USB_DRIVER_DATA_
USB_DEVICE_DESCRIPTOR HSDeviceDescr =
{
    sizeof(USB_DEVICE_DESCRIPTOR),              //descriptor's size 18(1B)
    USB_DT_DEVICE,                              //descriptor's type 01(1B)
    0x0200,                                      //USB plan distorbution number (2B)
    0,                                          //1type code (point by USB)(1B),0x00
    0, 0,                                       //child type and protocal (usb alloc)(2B)
    EP0_PACKET_SIZE,                            //endpoint 0 max package length(1B)
    0x071b,
    0x3203,
    0x0200,								        // device serial number
    USB_STRING_MANUFACTURER,
    USB_STRING_PRODUCT,
    USB_STRING_SERIAL,                          //producter,produce,device serial number index(3B)
    1                                           //feasible configuration parameter(1B)
};

/*-------------------------------- Local Statics: ----------------------------*/

/*--------------------------- Local Function Prototypes ----------------------*/


/*------------------------ Function Implement --------------------------------*/

/*
Name:       USBGetString
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
static int32 USBGetString(uint8 index, uint32 size, void *string)
{
    int32 len = 0;

    switch (index)
    {
        case 0:
            len = MIN(USBLangId[0], size);
            memcpy(string, (void*)USBLangId, len);
            break;

        #if USB_STRING_MANUFACTURER > 0
        case USB_STRING_MANUFACTURER:
            len = MIN(StringManufacture[0], size);
            memcpy(string, (void*)StringManufacture, len);
        #endif

        #if USB_STRING_PRODUCT > 0
        case USB_STRING_PRODUCT:
            len = MIN(StringProduct[0], size);
            memcpy(string, (void*)StringProduct, len);
            break;
        #endif

        #if USB_STRING_SERIAL > 0
        case USB_STRING_SERIAL:
            len = MIN(StringSerialNumbel[0], size);
            memcpy(string, (void*)StringSerialNumbel, len);
            break;
        #endif

        #if USB_STRING_CONFIG > 0
        case USB_STRING_CONFIG:
            len = MIN(StringConfig[0], size);
            memcpy(string, (void*)StringConfig, len);
            break;
        #endif

        #if USB_STRING_INTERFACE > 0
        case USB_STRING_INTERFACE:
            len = MIN(StringInterface[0], size);
            memcpy(string, (void*)StringInterface, len);
            break;
        #endif

        case USB_STRING_MS_OS:
            printf ("\nMicrosoft OS string Descriptor\n");
            USBStallEp(1);
            len = -1;
            #if 1 // No Support
            //len = 4;
            //memcpy(string, (void*)USBD_STATUS_STALL_PID, 4);
            #else //Support
            len = MIN(MSOSString[0], size);
            memcpy(string, (void*)MSOSString, len);
            #endif
            break;

        default:
            break;
    }

    return len;
}

/*
Name:       USBGetDescriptor
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
int32 USBGetDescriptor(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    uint32 len = 0;
    uint8 bDescriptor = (ctrl->wValue >> 8) & 0xff;
    uint8 speed = pDev->speed;
    USB_EP0_REQ *ep0req = USBGetEp0Req();

    switch (bDescriptor)
    {
        case USB_DT_DEVICE:
        {
            //获取设备描述符
            USB_DEVICE_DESCRIPTOR *ptr = (USB_DEVICE_DESCRIPTOR*)&HSDeviceDescr;

            len = MIN(sizeof(USB_DEVICE_DESCRIPTOR),ctrl->wLength);
            if (speed == USB_SPEED_FULL)
            {
                ptr->bcdUSB = 0x0110;
            }

            //ptr->bNumConfigurations = USBCountConfigs(speed);
            USBDEBUG("GET DES = %d", len);
            memcpy(ep0req->buf, (uint8*)ptr, len);
	        break;
        }
        case USB_DT_DEVICE_QUALIFIER:
            len = MIN(sizeof(HS_DEVICE_QUALIFIER), ctrl->wLength);
            memcpy(ep0req->buf, (uint8*)&HS_Device_Qualifier, len);
            USBDEBUG("GET QUALIFIER = %d", len);
            break;

        case USB_DT_OTHER_SPEED_CONFIG:
            goto GET_CONFIG;

        case USB_DT_CONFIG:
        GET_CONFIG:
            {
                USB_CONFIGURATION_DESCRIPTOR *ptr = (USB_CONFIGURATION_DESCRIPTOR *)ep0req->buf;

                if (bDescriptor == USB_DT_OTHER_SPEED_CONFIG)
	            {
	                speed = (USB_SPEED_FULL + USB_SPEED_HIGH) - speed;
	            }

                len = MIN(pDev->ConfigsLen, ctrl->wLength);
                memcpy(ep0req->buf, ((speed==USB_SPEED_HIGH)? pDev->HSConfigs:pDev->FSConfigs), len);
                ptr->bDescriptorType = bDescriptor;
                USBDEBUG("GET CONFIG = %d", len);
   	            break;
			}
        case USB_DT_STRING:
        {
            len = USBGetString(ctrl->wValue & 0x00ff, ctrl->wLength, ep0req->buf);
            if (len != -1)
            {
                if (len == 0)
                {
                    USBInEp0Ack();
                }
                USBDEBUG("GET STRING = %d", len);
            }
            else
            {
                len = 0;
                printf ("\nunknow String Descriptor:Stall\n");
            }
            break;
        }
        default:
            USBStallEp(1);
            break;
    }

    if (len)
    {
        ep0req->dir = 1;
        ep0req->NeedLen = len;
        USBStartEp0Xfer(ep0req);
    }

    return len;
}


/*
Name:       USBResetPhy
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_CODE_
void USBResetPhy(void)
{
    //USBOTG PHY software reset request
    ScuSoftResetCtr(USBPHY_SRST,1);
    ScuSoftResetCtr(USBGLB_SRST,1);
    //CRUReg->CRU_SOFTRST_CON[0] = (((0x1<<2)|(0x1<<1))<<16)|(0x1<<2)|(0x1<<1); //USBOTG PHY software reset request
    DelayMs(10);                    //delay 10ms @48MHz
    ScuSoftResetCtr(USBPHY_SRST,0);
    ScuSoftResetCtr(USBGLB_SRST,0);
    //CRUReg->CRU_SOFTRST_CON[0] = (((0x1<<2)|(0x1<<1))<<16)|(0x0<<2)|(0x0<<1);

    DelayUs(2000);                  //delay 2ms @48MHz
}

/*
Name:       USBDelayUS
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_CODE_
void USBDelayUS(uint32 us)
{
    DelayUs(us);
}

/*
Name:       USBDelayMS
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_CODE_
void USBDelayMS(uint32 ms)
{
    DelayUs(1000*ms);
}

/*
Name:       USBOtgMode
Desc:
Param:
Return:    1: Host mode
           0: Device mode

Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
uint8 USBOtgMode(void)
{
    return OTGReg->Core.gintsts & 0x1;
}

#endif
