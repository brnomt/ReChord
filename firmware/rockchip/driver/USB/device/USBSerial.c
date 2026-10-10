/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: USBSerial.c
Desc:

Author: chenfen
Date: 13-07-26
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/
#include "SysInclude.h"
#include "USBConfig.h"

#ifdef USB_SERIAL

#include <stdio.h>
#include <string.h>


/*------------------------------------ Defines -------------------------------*/


#define         USB_CDC_SUBCLASS_ACM			0x02
//  define	    USB protocol
#define         USB_CDC_ACM_PROTO_AT_V25TER		0x01

/* Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define USB_DT_CS_DEVICE		(USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG		(USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING		(USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE		(USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT		(USB_TYPE_CLASS | USB_DT_ENDPOINT)

/*
 * Class-Specific descriptors ... there are a couple dozen of them
 */

#define USB_CDC_HEADER_TYPE		0x00	/* header_desc */
#define USB_CDC_CALL_MANAGEMENT_TYPE	0x01	/* call_mgmt_descriptor */
#define USB_CDC_ACM_TYPE		0x02	/* acm_descriptor */
#define USB_CDC_UNION_TYPE		0x06	/* union_desc */
#define USB_CDC_COUNTRY_TYPE		0x07
#define USB_CDC_NETWORK_TERMINAL_TYPE	0x0a	/* network_terminal_desc */
#define USB_CDC_ETHERNET_TYPE		0x0f	/* ether_desc */
#define USB_CDC_WHCM_TYPE		0x11
#define USB_CDC_MDLM_TYPE		0x12	/* mdlm_desc */
#define USB_CDC_MDLM_DETAIL_TYPE	0x13	/* mdlm_detail_desc */
#define USB_CDC_DMM_TYPE		0x14
#define USB_CDC_OBEX_TYPE		0x15

#define USB_DEVICE_CLASS_CDC_DATA		0x0a    //ACM-CDC


#define USB_CDC_SEND_ENCAPSULATED_COMMAND	0x00
#define USB_CDC_GET_ENCAPSULATED_RESPONSE	0x01
#define USB_CDC_REQ_SET_LINE_CODING		0x20
#define USB_CDC_REQ_GET_LINE_CODING		0x21
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE	0x22
#define USB_CDC_REQ_SEND_BREAK			0x23

//----------------------------------------------------------------------

/*----------------------------------- Typedefs -------------------------------*/

/**
 * States of EP0.
 */
typedef enum tagSRL_XFER_STATE
{
	SRL_XFER_DISCONNECT,		/* no host */
	SRL_XFER_IDLE,
	SRL_XFER_IN_DATA,
	SRL_XFER_OUT_DATA,
	SRL_XFER_CMPL
} SRL_XFER_STATE;


typedef __packed struct _CDC_LINE_CODING
{
    /// Data terminal rate in bits per second.
    uint32 dwDTERate;
    /// Number of stop bits.
    /// \sa "CDC LineCoding StopBits".
    uint8 bCharFormat;
    /// Type of parity checking used.
    /// \sa "CDC LineCoding ParityCheckings".
    uint8 bParityType;
    /// Number of data bits (5, 6, 7, 8 or 16).
    uint8 bDataBits;

} CDC_LINE_CODING; // GCC


typedef struct tagSRL_DATA_REQ
{
    //uint32  BufLen;
    //uint8   *buf;
    uint32  XferLen;
    SRL_COMPLETE complete;
    //uint32  ret;
    //uint8   flag;
    uint8   status;
} SRL_DATA_REQ, *pSRL_DATA_REQ;


typedef struct tagSRL_CIRCULAR_BUF
{
    uint16  start;
    uint16  end;
    uint16  BufLen;
    uint8   *buf;
} SRL_CIRCULAR_BUF, *pSRL_CIRCULAR_BUF;


typedef volatile struct tagSRL_DEVICE
{
    SRL_DATA_REQ    *DataReq[2];
    SRL_CIRCULAR_BUF TxBuf;
    SRL_CIRCULAR_BUF RxBuf;
    uint8*          LineCoding;
    uint8           DataIn;
    uint8           DataOut;
    uint16          BulkMPS;
    USB_DEVICE      *pDev;
    SRL_HANDLE      hanlde;
    uint8           mode;           //  0: test mode, 1: user self define mode
    uint8           connected;
} SRL_DEVICE, *pSRL_DEVICE;


typedef volatile struct tagSRL_INHANDLE
{
    SRL_DATA_REQ    DataReq[2];
    uint32          flag;
    uint8           valid;
    pSRL_DEVICE     pSrl;
} SRL_INHANDLE, *pSRL_INHANDLE;

/*-------------------------- Forward Declarations ----------------------------*/

static int32 SrlSetup(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev);
static void SrlSuspend(USB_DEVICE *pDev);
static void SrlResume(USB_DEVICE *pDev);
static void SrlDisconnect(USB_DEVICE *pDev);
static void SrlReqest(uint8 epnum, uint32 event, uint32 param, USB_DEVICE *pDev);


/* ------------------------------- Globals ---------------------------------- */


/*-------------------------------- Local Statics: ----------------------------*/
/// Standard USB configuration descriptor for the CDC serial driver
_ATTR_USB_SRL_CODE_
CDC_SERIAL_CONFIGURATION_DESCRIPTOR HSSerialConfigs =
{
    // Standard configuration descriptor
    {
        sizeof(USB_CONFIGURATION_DESCRIPTOR),
        USB_DT_CONFIG,
        sizeof(CDC_SERIAL_CONFIGURATION_DESCRIPTOR),
        2, // There are two interfaces in this configuration
        1, // This is configuration #1
        0, // No string descriptor for this configuration
        0x80,                                       //位图,总线供电&远程唤醒(1B)
        0x32,                                            //最大消耗电流*2mA(1B)
    },
    // Communication class interface standard descriptor
    {
        sizeof(USB_INTERFACE_DESCRIPTOR),
        USB_DT_INTERFACE,
        0, // This is interface #0
        0, // This is alternate setting #0 for this interface
        1, // This interface uses 1 endpoint
        USB_DEVICE_CLASS_COMMUNICATIONS,
        USB_CDC_SUBCLASS_ACM,
        USB_CDC_ACM_PROTO_AT_V25TER,
        0  // No string descriptor for this interface
    },
    // Class-specific header functional descriptor
    {
        sizeof(CDC_HEADER_DESCRIPTOR),
        USB_DT_CS_INTERFACE,
        USB_CDC_HEADER_TYPE,
        0x0110
    },
    // Class-specific call management functional descriptor
    {
        sizeof(CDC_CALLMANAGEMENT_DESCRIPTOR),
        USB_DT_CS_INTERFACE,
        USB_CDC_CALL_MANAGEMENT_TYPE,
        0,
        0 // No associated data interface
    },
    // Class-specific abstract control management functional descriptor
    {
        sizeof(CDC_ACM_DESCRIPTOR),
        USB_DT_CS_INTERFACE,
        USB_CDC_ACM_TYPE,
        2       // bmCapabilities 支持Set_Line_Coding、Set_Control_Line_State、Get_Line_Coding请求和Serial_State通知
    },
    // Class-specific union functional descriptor with one slave interface
    {
        sizeof(CDC_UNION_DESCRIPTOR),
        USB_DT_CS_INTERFACE,
        USB_CDC_UNION_TYPE,
        0,  /* index of control interface */
        1,  /* index of data interface */

    },
    // Notification endpoint standard descriptor
    {
        sizeof(USB_ENDPOINT_DESCRIPTOR),
        USB_DT_ENDPOINT,
        CDC_NOTIFICATION_EP|0x80,
        USB_EPTYPE_INTR,
        HS_INT_MAXPACKET,//HS_BULK_RX_SIZE,
        10 // Endpoint is polled every 10ms
    },
    // Data class interface standard descriptor
    {
        sizeof(USB_INTERFACE_DESCRIPTOR),
        USB_DT_INTERFACE,
        1, // This is interface #1
        0, // This is alternate setting #0 for this interface
        2, // This interface uses 2 endpoints
        USB_DEVICE_CLASS_CDC_DATA,
        0,
        0,
        0  // No string descriptor for this interface
    },
    // Bulk-IN endpoint descriptor
    {
        sizeof(USB_ENDPOINT_DESCRIPTOR),
        USB_DT_ENDPOINT,
        CDC_DATAIN_EP|0x80,
        USB_EPTYPE_BULK,
        HS_BULK_TX_SIZE,
        0 // Must be 0 for full-speed bulk endpoints
    },
     // Bulk-OUT endpoint standard descriptor
    {
        sizeof(USB_ENDPOINT_DESCRIPTOR),
        USB_DT_ENDPOINT,
        CDC_DATAOUT_EP,
        USB_EPTYPE_BULK,
        HS_BULK_RX_SIZE,
        0 // Must be 0 for full-speed bulk endpoints
    }
};

_ATTR_USB_SRL_CODE_
USB_DEVICE_DESCRIPTOR HSSerialDescr =
{
    sizeof(USB_DEVICE_DESCRIPTOR),              //descriptor's size 18(1B)
    USB_DT_DEVICE,                              //descriptor's type 01(1B)
    0x0200,                                      //USB plan distorbution number (2B)
    0x02,                                        //bDeviceClass字段。本设备必须在设备描述符中指定设备的类型，
                                                //否则，由于在配置集合中有两个接口，就会被系统认为是一个USB
                                                //复合设备，从而导致设备工作不正常。0x02为通信设备类的类代码。
    0, 0,                                       //child type and protocal (usb alloc)(2B)
    EP0_PACKET_SIZE,                            //endpoint 0 max package length(1B)
    0x8888,
    0x0007,
    0x0200,                                     // device serial number
    USB_STRING_MANUFACTURER,
    USB_STRING_PRODUCT,
    USB_STRING_SERIAL,                          //producter,produce,device serial number index(3B)
    1                                           //feasible configuration parameter(1B)
};

_ATTR_USB_SRL_BSS_ SRL_DEVICE SerialDevice;

_ATTR_USB_SRL_DATA_
static USB_DEVICE SerialDriver =
{
    USB_SPEED_HIGH,
    USB_CLASS_TYPE_SERIAL,
    sizeof(CDC_SERIAL_CONFIGURATION_DESCRIPTOR),
    &HSSerialConfigs,
    &HSSerialConfigs,
    SrlSetup,
    SrlSuspend,
    SrlResume,
    SrlDisconnect,
    SrlReqest,
    &SerialDevice,
    NULL,
    NULL
};

//LineCoding数组，用来保存波特率、停止位等串口属性。
//初始化波特率为9600，1停止位，无校验，8数据位。
_ATTR_USB_SRL_DATA_
static uint8 LineCoding[7]={0x80,0x25,0x00,0x00,0x00,0x00,0x08};

_ATTR_USB_SRL_BSS_
SRL_INHANDLE   SrlInHandle/*[2]*/;

_ATTR_USB_SRL_BSS_
static uint32 SrlTmpBuf[128/4];

_ATTR_USB_SRL_BSS_
static uint32 SrlTestBuf[128/4];

_ATTR_USB_SRL_BSS_
static uint32 SrlRxBuf[SRL_RX_BUF_LEN/4];

_ATTR_USB_SRL_BSS_
static uint32 SrlTxBuf[SRL_TX_BUF_LEN/4];

/*--------------------------- Local Function Prototypes ----------------------*/


/*------------------------ Function Implement --------------------------------*/

/*
Name:       SrlCleanBuf
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static int32 SrlCleanXferBuf(SRL_DEVICE *pSrl)
{
    pSRL_CIRCULAR_BUF pXBuf = &pSrl->TxBuf;

    pXBuf->end = pXBuf->start = 0;

    pXBuf = &pSrl->RxBuf;
    pXBuf->end = pXBuf->start = 0;
}


/*
Name:       SrlGetTxData
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static int32 SrlIsTxDataAvail(SRL_DEVICE *pSrl)
{
    pSRL_CIRCULAR_BUF pTBuf = &pSrl->TxBuf;

    return (pTBuf->end != pTBuf->start)? 1:0;
}


/*
Name:       SrlGetTxData
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static int32 SrlGetTxData(SRL_DEVICE *pSrl, void **buf)
{
    uint32 size = 0;

    pSRL_CIRCULAR_BUF pTBuf = &pSrl->TxBuf;

    if (pTBuf->end > pTBuf->start)
    {
        size = MIN(pSrl->BulkMPS, pTBuf->end-pTBuf->start);
        *buf = (void*)(pTBuf->buf+pTBuf->start);
    }
    else if (pTBuf->end < pTBuf->start)
    {
        size = pTBuf->BufLen - pTBuf->start;
        if (size < pSrl->BulkMPS)
        {
            uint16 len;
            uint8 *tmp = (uint8 *)SrlTmpBuf;
            memcpy(tmp, pTBuf->buf+pTBuf->start, size);
            len = MIN((pSrl->BulkMPS-size), pTBuf->end);
            memcpy(tmp+size, pTBuf->buf, len);
            size += len;
            *buf = (void*)tmp;
        }
		else
        {
            size = pSrl->BulkMPS;
            *buf = (void*)(pTBuf->buf+pTBuf->start);
        }
    }

    return size;
}


/*
Name:       SrlBulkInPkt
Desc:       批量数据传输IN包处理
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlBulkInPkt(SRL_DEVICE *pSrl)
{
    SRL_DATA_REQ *req =  pSrl->DataReq[0];

    if (req && SRL_XFER_IN_DATA == req->status)
    {
        uint16 byte;
        void *buf;

        byte = SrlGetTxData(pSrl, &buf);
        if (byte)
        {
            pSRL_CIRCULAR_BUF pTBuf = &pSrl->TxBuf;

            USBWriteEp(pSrl->DataIn, byte, buf);
            pTBuf->start += byte;
            req->XferLen += byte;

            if (pTBuf->start >= pTBuf->BufLen)
                pTBuf->start -= pTBuf->BufLen;

            if (pTBuf->start == pTBuf->end)
            {
                req->status = SRL_XFER_CMPL;
                pTBuf->start = pTBuf->end = 0;
            }
        }
    }
}

/*
Name:       FsgBulkOutPkt
Desc:       批量数据传输OUT包处理
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlBulkOutPkt(SRL_DEVICE *pSrl, uint16 len)
{
    SRL_DATA_REQ *req =  pSrl->DataReq[1];

    if (req && SRL_XFER_OUT_DATA==req->status)
    {
        uint32 byte;
        pSRL_CIRCULAR_BUF pRBuf = &pSrl->RxBuf;

        byte = pRBuf->BufLen-pRBuf->end;
        if (byte >= len)
        {
            USBReadEp(pSrl->DataOut, len, pRBuf->buf+pRBuf->end);
        }
        else
        {
            uint8 tmpbuf[512];
            USBReadEp(pSrl->DataOut, len, tmpbuf);
            memcpy(pRBuf->buf+pRBuf->end, tmpbuf, byte);
            memcpy(pRBuf->buf, tmpbuf+byte, len-byte);
        }

        pRBuf->end += len;
        if (pRBuf->end >= pRBuf->BufLen)
            pRBuf->end -= pRBuf->BufLen;

        req->XferLen += len;
    }
    else
    {
        uint8 dummy[512];
        USBReadEp(pSrl->DataOut, len, dummy);
    }
}

/*
Name:       SrlBulkOutHandle
Desc:       批量数据传输BulkOnly命令解释
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlBulkOutHandle(SRL_DEVICE *pSrl)
{
    SRL_DATA_REQ *req =  pSrl->DataReq[1];

    if (req && SRL_XFER_OUT_DATA==req->status)
    {
        req->status = SRL_XFER_CMPL;
        //USBSetNak(pSrl->DataOut);
    }
}

/*
Name:       SrlGetDescriptor
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static int32 SrlGetDescriptor(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    uint8 bDescriptor = (ctrl->wValue >> 8) & 0xff;

    if  (bDescriptor == USB_DT_DEVICE)
    {
        //获取设备描述符
        USB_DEVICE_DESCRIPTOR *ptr = (USB_DEVICE_DESCRIPTOR*)&HSSerialDescr;
        USB_EP0_REQ *ep0req = USBGetEp0Req();
        uint32 len = 0;

        len = MIN(sizeof(USB_DEVICE_DESCRIPTOR), ctrl->wLength);
        if (pDev->speed == USB_SPEED_FULL)
        {
            ptr->bcdUSB=0x0110;
        }

        memcpy(ep0req->buf, (uint8*)ptr, len);

        ep0req->dir = 1;
        ep0req->NeedLen = len;
        USBStartEp0Xfer(ep0req);
        return 0;
    }

    return -1;
}

/*
Name:       SrlSetConfig
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlSetConfig(SRL_DEVICE *pSrl, uint8 config)
{
    if (config)
    {
        USBEnableEp(pSrl->DataIn|0x80, &HSSerialConfigs.DataIn);
        USBEnableEp(pSrl->DataOut, &HSSerialConfigs.DataOut);
        USBEnableEp(CDC_NOTIFICATION_EP|0x80, &HSSerialConfigs.notification);
    }
    USBInEp0Ack();
}

/*
Name:       SrlStandardReq
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static int32 SrlStandardReq(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    SRL_DEVICE *pSrl = (SRL_DEVICE *)pDev->pFunDev;
    int32 ret = 0;

    switch (ctrl->bRequest)
    {
        case USB_REQ_GET_DESCRIPTOR:
            ret = SrlGetDescriptor(ctrl, pDev);
            break;
        /* One config, two speeds */
        case USB_REQ_SET_CONFIGURATION:
            SrlSetConfig(pSrl, ctrl->wValue & 0x1);
            break;

        default:
            break;
    }

    return ret;
}


/*
Name:       SrlEp0Complete
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlEp0Complete(void *ptr)
{
    USB_DEVICE *pDev = (USB_DEVICE *)ptr;
	SRL_DEVICE *pSrl = (SRL_DEVICE*)pDev->pFunDev;
    USB_EP0_REQ *ep0req = USBGetEp0Req();

    memcpy(pSrl->LineCoding, ep0req->buf, sizeof(CDC_LINE_CODING));
}

/*
Name:       SrlClassReq
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlClassReq(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    SRL_DEVICE *pSrl = (SRL_DEVICE*)pDev->pFunDev;

    switch ((ctrl->bRequestType << 8) | ctrl->bRequest)
    {
        /* SET_LINE_CODING ... just read and save what the host sends */
        case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
        | USB_CDC_REQ_SET_LINE_CODING:
        {
            USB_EP0_REQ *ep0req = USBGetEp0Req();
            //memset(ep0req, 0, sizeof(USB_EP0_REQ));
            ep0req->dir = 0;
            ep0req->complete = SrlEp0Complete;
            ep0req->NeedLen = sizeof(CDC_LINE_CODING);
            USBStartEp0Xfer(ep0req);
            break;
        }

        /* GET_LINE_CODING ... return what host sent, or initial value */
        case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
        | USB_CDC_REQ_GET_LINE_CODING:
        {
            USBWriteEp0(sizeof(CDC_LINE_CODING), pSrl->LineCoding);
            break;
        }
        /* SET_CONTROL_LINE_STATE ... save what the host sent */
        case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
        | USB_CDC_REQ_SET_CONTROL_LINE_STATE:
        {
            if (0 == pSrl->connected)
            {
                pSrl->connected = 1;
                SendMsg(MSG_USB_SRL_CONNECT);
            }
            USBInEp0Ack();
            UDEBUG("SET_STATE\n");
            break;
        }
        default:
            break;
    }

}

/*
Name:       SrlSetup
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static int32 SrlSetup(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    uint8 type = ctrl->bRequestType & USB_TYPE_MASK;
    int32 ret = 0;

	if (type == USB_TYPE_STANDARD)
    {
		ret = SrlStandardReq(ctrl, pDev);
    }
	else if (type == USB_TYPE_CLASS)
	{
		SrlClassReq(ctrl, pDev);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

/*
Name:       SrlReqest
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlReqest(uint8 epnum, uint32 event, uint32 param, USB_DEVICE *pDev)
{
    SRL_DEVICE *pSrl = (SRL_DEVICE*)pDev->pFunDev;

    if (epnum == pSrl->DataOut || (epnum == pSrl->DataIn))
    {
        switch (event)
        {
            case UDC_EV_OUT_PKT_RCV:
                SrlBulkOutPkt(pSrl, (uint16)param);
                break;

            case UDC_EV_OUT_XFER_COMPL:
                SrlBulkOutHandle(pSrl);
                break;

            case UDC_EV_IN_XFER_COMPL:
                SrlBulkInPkt(pSrl);
                break;
            default:
                break;
        }
        return;
    }

    if (event == UDC_EV_ENUM_DONE)
    {
        if ((uint8)param == USB_SPEED_FULL)
        {
            CDC_SERIAL_CONFIGURATION_DESCRIPTOR *ptr = &HSSerialConfigs;

            ptr->DataOut.wMaxPacketSize = FS_BULK_TX_SIZE;
            ptr->DataIn.wMaxPacketSize = FS_BULK_TX_SIZE;
            pSrl->BulkMPS = FS_BULK_TX_SIZE;
        }
    }
}

/*
Name:       SrlResume
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlResume(USB_DEVICE *pDev)
{

}

/*
Name:       SrlSuspend
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlSuspend(USB_DEVICE *pDev)
{

}

/*
Name:       SrlDisconnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlDisconnect(USB_DEVICE *pDev)
{
    SRL_DEVICE *pSrl = (SRL_DEVICE*)pDev->pFunDev;

    pSrl->connected = 0;
}
_ATTR_USB_SRL_BSS_
static char SrlRecCommandBuf[128];

_ATTR_USB_SRL_BSS_
static uint32 SrlRecHead;

_ATTR_USB_SRL_CODE_
static void SerCommandParser(int32 len,void* buffer )
{
	SRL_DEVICE *pSrl = &SerialDevice;

	memcpy(SrlRecCommandBuf+SrlRecHead, buffer, len);
	SrlRecHead += len;
	if ((len ==0)&&(buffer == NULL))
	{
		memset(SrlRecCommandBuf, 20, 128);
		SrlRecHead = 0;
		return;
	}
}

/*
Name:       SrlRxComplete
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
static void SrlRxComplete(int32 len)
{
    //uint32 SrlTestBuf[128/4];
    SRL_DEVICE *pSrl = &SerialDevice;

    if (1 == len)
    {
        int32 ch;

        SrlReadData(pSrl->hanlde, 1, &ch);

        SrlWrite(pSrl->hanlde, 1, &ch, NULL);

		SerCommandParser(1,&ch);
		if (ch == '\r')
        {
            int32 tmp = '\n';
            SrlWrite(pSrl->hanlde, 1, &tmp, NULL);
			SerCommandParser(0,NULL);
        }
    }
    else
    {
        len = SrlReadData(pSrl->hanlde, len, SrlTestBuf);
        SrlWrite(pSrl->hanlde, len, SrlTestBuf, NULL);
		SerCommandParser(len, SrlTestBuf);
    }
}


/*
Name:       SrlReadData
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
int32 SrlReadData(SRL_HANDLE handle, uint32 len, void* buf)
{
    pSRL_INHANDLE   inhandle = (pSRL_INHANDLE)handle;
    SRL_DATA_REQ *req = &inhandle->DataReq[1];
    pSRL_CIRCULAR_BUF pRBuf = &inhandle->pSrl->RxBuf;
    int32 size = 0;

    if (!(inhandle->flag & SRL_OPEN_R))
        return -1;

    if (req->status != SRL_XFER_CMPL)
        return -1;

    if (pRBuf->end > pRBuf->start)
    {
        size = MIN((pRBuf->end-pRBuf->start), len);
        memcpy(buf, pRBuf->buf+pRBuf->start, size);
        pRBuf->start += size;
    }
    else if (pRBuf->end < pRBuf->start)
    {
        size = MIN((pRBuf->BufLen - pRBuf->start), len);
        memcpy(buf, pRBuf->buf+pRBuf->start, size);
        pRBuf->start += size;
        len -= size;
        if (len)
        {
            len = MIN(pRBuf->end, len);
            memcpy((uint8*)buf+size, pRBuf->buf, len);
            pRBuf->start += len;
            size += len;
        }

        if (pRBuf->start > pRBuf->BufLen)
            pRBuf->start -= pRBuf->BufLen;
    }

    return size;
}



/*
Name:       SrlRead
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
int32 SrlRead(SRL_HANDLE handle, SRL_COMPLETE complete)
{
    pSRL_INHANDLE   inhandle = (pSRL_INHANDLE)handle;
    SRL_DATA_REQ *req = &inhandle->DataReq[1];

    if (!(inhandle->flag & SRL_OPEN_R))
        return -1;

    if (req->status != SRL_XFER_IDLE)
        return -1;


    //req->BufLen = len;
    //req->buf = buf;
    req->complete = complete;
    req->XferLen = 0;
    //req->ret = 0;
    req->status = SRL_XFER_OUT_DATA;

    USBClearNak(inhandle->pSrl->DataOut);

    return 0;
}

/*
Name:       SrlWrite
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
int32 SrlWrite(SRL_HANDLE handle, uint32 len, void* buf, SRL_COMPLETE complete)
{
    pSRL_INHANDLE   inhandle = (pSRL_INHANDLE)handle;
    SRL_DATA_REQ *req = &inhandle->DataReq[0];
    SRL_CIRCULAR_BUF *pTBuf = &inhandle->pSrl->TxBuf;
    uint32 size = 0;

    if (!(inhandle->flag & SRL_OPEN_W))
        return -1;

    //DISABLE_USB_INT;
    DisableIntMaster();
    UsbIntDisalbe();
    EnableIntMaster();

    if (pTBuf->end >= pTBuf->start)
    {
        size = MIN ((pTBuf->BufLen-pTBuf->end), len);
        if (size)
        {
            memcpy(pTBuf->buf+pTBuf->end, buf, len);
            len -= size;
        }
        if (len)
        {
            len = MIN(pTBuf->start, len);
            memcpy(pTBuf->buf, (uint8*)buf+size, len);
            size += len;
        }
    }
    else
    {
        size = MIN((pTBuf->start-pTBuf->end), len);
        memcpy(pTBuf->buf+pTBuf->end, (uint8*)buf, size);
    }

    pTBuf->end += size;
    if (pTBuf->end >= pTBuf->BufLen)
        pTBuf->end -= pTBuf->BufLen;

    req->complete = complete;
    //ENABLE_USB_INT;
    DisableIntMaster();
    UsbIntEnable();
    EnableIntMaster();

    return size;
}

/*
Name:       SrlOpen
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
SRL_HANDLE SrlOpen(uint32 flag)
{
    pSRL_INHANDLE inhandle = &SrlInHandle;
    SRL_DATA_REQ    *req = &inhandle->DataReq[0];

    if (inhandle->valid)
        return NULL;

    if (!SerialDevice.connected)
        return NULL;

    memset(&inhandle->DataReq[0], 0, sizeof(inhandle->DataReq));

    inhandle->flag = flag;
    inhandle->pSrl = &SerialDevice;

    if ((inhandle->flag & SRL_OPEN_W))
    {
        req->status = SRL_XFER_IDLE;
        inhandle->pSrl->DataReq[0] = req;
    }

    if ((inhandle->flag & SRL_OPEN_R))
    {
        req++;
        req->status = SRL_XFER_IDLE;
        inhandle->pSrl->DataReq[1] = req;
    }

    inhandle->valid = 1;

    return (void*)inhandle;
}

/*
Name:       SrlClose
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
int32 SrlClose(SRL_HANDLE handle)
{
    pSRL_INHANDLE  inhandle = (pSRL_INHANDLE)handle;

    if (inhandle == NULL && !inhandle->valid)
        return -1;

    inhandle->pSrl->DataReq[0] = NULL;
    inhandle->pSrl->DataReq[1] = NULL;
    SrlCleanXferBuf(inhandle->pSrl);

    inhandle->valid = 0;

    return 0;
}

/*
Name:       SrlInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
int32 SrlInit(void *pArg)
{
    SRL_DEVICE *pSrl = &SerialDevice;

    memset(pSrl, 0, sizeof(SRL_DEVICE));

    pSrl->RxBuf.buf = (uint8*)SrlRxBuf;
    pSrl->RxBuf.BufLen = SRL_RX_BUF_LEN;

    pSrl->TxBuf.buf = (uint8*)SrlTxBuf;
    pSrl->TxBuf.BufLen = SRL_TX_BUF_LEN;

    //pSrl->ep0req.buf = (uint8*)SrlTmpBuf;

    pSrl->DataIn = CDC_DATAIN_EP;
    pSrl->DataOut = CDC_DATAOUT_EP;
    pSrl->BulkMPS = HS_BULK_TX_SIZE;

    pSrl->mode = (uint8)pArg;
    pSrl->LineCoding = (uint8*)LineCoding;
    pSrl->pDev = &SerialDriver;

    SrlInHandle.valid = 0;
    USBDriverProbe(&SerialDriver);
	memset(SrlRecCommandBuf, 20, 128);
	SrlRecHead = 0;


    return 0;
}

/*
Name:       SrlDeInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
void SrlDeInit(void)
{
    SrlClose(&SrlInHandle);
}
void UsbPrintKey(void){
	SRL_DEVICE *pSrl = &SerialDevice;
switch(KeyVal){
		case KEY_VAL_FFD_SHORT_UP:
               SrlWrite(pSrl->hanlde, 21, "KEY_VAL_FFD_SHORT_UP\n", NULL);
			break;
		case KEY_VAL_FFW_SHORT_UP:

               SrlWrite(pSrl->hanlde, 21, "KEY_VAL_FFW_SHORT_UP\n", NULL);
			break;
		case KEY_VAL_PLAY_SHORT_UP:

              SrlWrite(pSrl->hanlde, 22, "KEY_VAL_PLAY_SHORT_UP\n", NULL);
			break;
		case KEY_VAL_ESC_SHORT_UP:

               SrlWrite(pSrl->hanlde, 21, "KEY_VAL_ESC_SHORT_UP\n", NULL);
			break;
		case KEY_VAL_UP_SHORT_UP:

                SrlWrite(pSrl->hanlde, 20, "KEY_VAL_UP_SHORT_UP\n", NULL);
			break;
		case KEY_VAL_DOWN_SHORT_UP:

            SrlWrite(pSrl->hanlde, 22, "KEY_VAL_DOWN_SHORT_UP\n", NULL);
			break;
			}
}


/*
Name:       SrlService
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
int32 SrlThread(void)
{
    SRL_DEVICE *pSrl = &SerialDevice;
    SRL_DATA_REQ *req;

    if ((0 == CheckVbus()) || (0 == USBIsConnect()))
    {
        DEBUG("Vbus = 0");
        //SendMsg(MSG_USB_DISCONNECT);
        return 0;
    }
    #if 0
    if (1 == USBIsSuspend())
    {
        SendMsg(MSG_USB_SUSPEND);
        return 0;
    }
    #endif

    if (GetMsg(MSG_USB_SRL_CONNECT))
    {
        if (0 == pSrl->mode)
        {
            pSrl->hanlde = SrlOpen(SRL_OPEN_R|SRL_OPEN_W);

            if (!pSrl->hanlde)
            {
                UDEBUG("Srl open err\n");
                return 0;
            }

            SrlRead(pSrl->hanlde, SrlRxComplete);
        }
        return 0;
    }

    /*process in data request*/
    req = pSrl->DataReq[0];
    if (req)
    {
        if (req->status == SRL_XFER_CMPL)
        {
            if (req->complete)
            {
                req->complete(req->XferLen);
            }

            req->XferLen = 0;
            req->status = SRL_XFER_IDLE;
        }

        if (req->status == SRL_XFER_IDLE)
        {
            if (SrlIsTxDataAvail(pSrl))
            {
                req->status = SRL_XFER_IN_DATA;
                SrlBulkInPkt(pSrl);
            }
        }
    }

    /*process out data request*/
    req = pSrl->DataReq[1];
    if (req)
    {
        if (req->status == SRL_XFER_CMPL)
        {
            if (req->complete)
            {
                req->complete(req->XferLen);
            }

            req->XferLen = 0;
            req->status = SRL_XFER_OUT_DATA;
            //USBClearNak(pSrl->DataOut);
        }
    }

    return 0;
}

#if 0
SRL_HANDLE tHandle = NULL;


/*
Name:       SrlWaitConnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_SRL_CODE_
int32 SrlWaitConnect(int32 timeout)
{
    timeout *= 1000;
    while (!SerialDevice.connected && timeout--)
    {
        USBDelayUS(1);
    }

    return (SerialDevice.connected)?  0 : -1;
}

/*
Name:       SrlTest
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
int32 SrlTest(void)
{
    int32 ret = 0;
    SRL_DEVICE *pSrl = &SerialDevice;

    //BLOffDisable();
    SetPowerOffTimerDisable();

    //SendMsg(MSG_USB_FUNUSB);
    ModuleOverlay(MODULE_ID_USB_MSC, MODULE_OVERLAY_ALL);

    USBDriverInit();
    SrlInit(NULL);
    USBReConnect(0);

    if (!SrlWaitConnect(5000))//wait 5s
    {
        UDEBUG("Srl no connect\n");
        while(1);
    }

    tHandle = SrlOpen(SRL_OPEN_R|SRL_OPEN_W);
    if (!tHandle)
    {
        UDEBUG("Srl open err\n");
        while(1);
    }

    SrlRead(tHandle, SrlRxComplete);

    do
    {
        ret = SrlThread();
        if (ret < 0);
            break;
        USBDelayMS(1);
    }while(1);

    SrlClose(tHandle);
    //SrlDeInit();

    return ret;
}

#endif
#endif

