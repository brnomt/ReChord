/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: USBDevice.c
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
#include "driver/DMA/Hw_dma.h"   /* DMA_CFGX, DMA_CTLL_USB_* */
#include "driver/DMA/Dma.h"      /* eDMA_CHN, DMA_FALSE, DMA_SUCCESS */

#ifdef _USB_

#include "USBConfig.h"

/*------------------------------------ Defines -------------------------------*/

#define MAX_UDC_DEVICES              3

#define USB_DMA

/*----------------------------------- Typedefs -------------------------------*/


/*-------------------------- Forward Declarations ----------------------------*/
extern int32 USBGetDescriptor(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev);

/* ------------------------------- Globals ---------------------------------- */


_ATTR_USB_DRIVER_BSS_ UDC_DRIVER UDCDriver;

/*-------------------------------- Local Statics: ----------------------------*/

_ATTR_USB_DRIVER_BSS_ uint32 USBEpBuf[512/4];

/*--------------------------- Local Function Prototypes ----------------------*/


/*------------------------ Function Implement --------------------------------*/

/*
Name:       USBEnableEp
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
void USBEnableEp(uint8 epnum, USB_ENDPOINT_DESCRIPTOR *desc)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if (epnum & 0x80)
    {
        epnum = 0x7F & epnum;
        dev_regs->daintmsk |= (1<<epnum);
        dev_regs->in_ep[epnum].diepctl = (1<<28)|(1<<27)|(desc->bmAttributes<<18)|(1<<15)|desc->wMaxPacketSize;
        //if (3==epnum)
        //    dev_regs->in_ep[epnum].diepctl |= (1<<22);  //这个配置,还要进一步debug
        dev_regs->dctl |= 1<<8;
    }
    else
    {
        dev_regs->daintmsk |= ((1<<epnum)<<16);
        dev_regs->out_ep[epnum].doeptsiz = (1ul << 19) | desc->wMaxPacketSize;
        dev_regs->out_ep[epnum].doepctl = (1ul<<31)|(1<<28)|(1<<26)|(desc->bmAttributes<<18)|(1<<15)|desc->wMaxPacketSize;
        dev_regs->out_ep[epnum].doepint = 0xFF;
    }
}


/*
Name:       USBWriteEp0
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
uint32 USBWriteEp0(uint16 len, void* buf)
{
    uint32 i, j = 0;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    uint32 *fifo = (uint32 *)USB_EP_FIFO(0);

REPEAT:
    if (((otg_core->Core.gnptxsts & 0xFFFF) >= (len+3)/4) && (((otg_core->Core.gnptxsts>>16) & 0xFF) > 0))
    {
        otg_core->Device.in_ep[0].dieptsiz = len | (1<<19);
        otg_core->Device.in_ep[0].diepctl = (1ul<<26) | (1ul<<31);   //Endpoint Enable, Clear NAK

        if (!((uint32)buf & 0x3))
        {
            uint32 *ptr = (uint32*)buf;
            for (i = 0; i < len; i += 4)
            {
                *fifo = *ptr++;
            }
        }
        else
        {
            uint8 *ptr = (uint8*)buf;
            for (i = 0; i < len; i += 4, ptr += 4)
            {
                *fifo = (ptr[3]<<24) | ( ptr[2]<<16) | (ptr[1]<<8) | ptr[0];
            }
        }
        return (len);
    }
    else
    {
        if (j > 10)
        {
            printf ("Write EP0 fail\n");
            return (0);
        }
        j++;
        DelayUs(1);
        goto REPEAT;

    }
}

/*
Name:       USBReadEp
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
uint32 USBReadEp(uint8 epnum, uint16 len, void * buf)
{
    uint32 i;
    uint32 *fifo = (uint32 *)USB_EP_FIFO(epnum);

    if (!((uint32)buf & 0x3))
    {
        #ifdef USB_DMA
        if (len >= 512)
        {
            uint32 timeout = 0;
            eDMA_CHN channel;
            DMA_CFGX DmaCfg = {DMA_CTLL_USB_RX, DMA_CFGL_USB_RX, DMA_CFGH_USB_RX,0};

            channel = DmaGetChannel();
            if ((channel != DMA_FALSE)/* && (channel < DMA_CHN2)*/)
            {
                DmaStart((uint32)(channel), (uint32)fifo, (uint32)buf, len>>2, &DmaCfg, NULL);
                while (DMA_SUCCESS != DmaGetState(channel))
                {
                    USBDelayUS(1);
                    if (++timeout > len * 1000)
                    {
                        //printf("USB DMA Read Error: len = %d!\n", len);
                        break;
                    }
                }
            }
        }
        else
        #endif
        {
            uint32 *ptr = (uint32*)buf;
            for (i=0; i<len; i+=4)
            {
                *ptr++ = *fifo;
            }
        }

    }
    else
    {
        uint32 *ptr = &USBEpBuf[0];
        for (i=0; i<len; i+=4)
        {
            *ptr++ = *fifo;
        }

        memcpy(buf, USBEpBuf, len);
    }

    if (0==epnum)
    {
        DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
        dev_regs->out_ep[0].doeptsiz = 0x40 | (1<<19) | (1<<29);
    }

    return len;
}

/*
Name:       USBWriteEp
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
uint32 USBWriteEp(uint8 epnum, uint16 len, void * buf)
{
    uint32 j = 0;
    USB_OTG_REG * otg_core = (USB_OTG_REG *)USB_REG_BASE;


REPEAT:
    if ((otg_core->Core.gnptxsts & 0xFFFF) >= ((len+3)>>2))
    {
        uint32 i;
        DEPCTL_DATA diepctl;
        uint32 *fifo = (uint32 *)USB_EP_FIFO(epnum);
        IN_EP_REG * in_ep = (IN_EP_REG *)&otg_core->Device.in_ep[epnum];

        in_ep->dieptsiz = len | (1<<19);

        diepctl.d32 = in_ep->diepctl;
        in_ep->diepctl = ((diepctl.b.dpid==0)? (1<<28):(1<<29))|(diepctl.b.eptype<<18)|(1<<15)|diepctl.b.mps;
        in_ep->diepctl |= ((1ul<<31)|(1<<26));

        if (!((uint32)buf & 0x3))
        {
            #ifdef USB_DMA
            if (len >= 512)
            {
                uint32 timeout = 0;
                eDMA_CHN channel;
                DMA_CFGX DmaCfg = {DMA_CTLL_USB_TX, DMA_CFGL_USB_TX, DMA_CFGH_USB_TX,0};

                channel = DmaGetChannel();
                if ((channel != DMA_FALSE)/* && (channel < DMA_CHN2)*/)
                {
                    DmaStart((uint32)(channel), (uint32)buf, (uint32)fifo, len>>2, &DmaCfg, NULL);
                    while (DMA_SUCCESS != DmaGetState(channel))
                    {
                        USBDelayUS(1);
                        if (++timeout > len * 1000)
                        {
                            //printf("USB DMA write Error: len = %d!\n", len);
                            break;
                        }
                    }
                }
            }
            else
            #endif
            {
                uint32 *ptr = (uint32*)buf;

                /*{
                    extern __align(4) uint8  EpInData[512];
                    if(ptr == (uint32 *)EpInData)
                    {
                       USBDEBUG("REDO WRITE EP OK");
                    }
                }*/

                for (i=0; i<len; i+=4)
                {
                    *fifo = *ptr++;
                }
            }

        }
        else
        {
            uint8 *ptr = (uint8*)buf;

            for (i=0; i<len; i+=4)
            {
                *fifo = (ptr[3]<<24) | ( ptr[2]<<16) | (ptr[1]<<8) | ptr[0];
                ptr += 4;
            }
        }

        return (len);
    }
    else
    {
        if (j > 10)
        {
            USBDEBUG("WRITE EP FAIL");
            return (0);
        }
        j++;
        DelayUs(1);
        goto REPEAT;
    }
}

/*
Name:       USBResetToggle
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
void USBResetToggle(uint8 dir, uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if (dir)
    {
        dev_regs->in_ep[epnum].diepctl  |= 1<<28;
    }
    else
    {
        dev_regs->out_ep[epnum].doepctl |= 1<<28;
    }
}

/*
Name:       USBStartEp0Xfer
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
int32 USBStartEp0Xfer(USB_EP0_REQ *req)
{
    pUDC_DRIVER pUDC = &UDCDriver;

    req->XferLen = 0;

    if (req->dir)
    {
        uint16 len = MIN(EP0_PACKET_SIZE, req->NeedLen);
        USBWriteEp0(len, req->buf);
        req->XferLen += len;
        if(req->NeedLen > req->XferLen)
        {
            pUDC->ep0state = EP0_IN_DATA_PHASE;
        }
    }
    else
    {
        pUDC->ep0state = EP0_OUT_DATA_PHASE;
    }

    return 0;
}


/*
Name:       USBStallEp0
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
void USBStallEp(uint8 dir)
{
    //write_XDATA(USB_CSR0, CSR0_OUT_SEVD | CSR0_SEND_STALL);   //send stall
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if (dir == 1)
        dev_regs->in_ep[0].diepctl |= 1<<21;   //send IN0 stall handshack
    else
        dev_regs->out_ep[0].doepctl |= 1<<21;  //send OUT0 stall handshack

}

/*
Name:       USBClearNak
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
void USBClearNak(uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if (epnum & 0x80)
        dev_regs->in_ep[epnum].diepctl |= (1ul<<26) | (1ul<<31); //Active ep, Clr Nak, endpoint enable
    else
        dev_regs->out_ep[epnum].doepctl |= (1ul<<26) | (1ul<<31); //Active ep, Clr Nak, endpoint enable

}

/*
Name:       USBSetNak
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
void USBSetNak(uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if (epnum & 0x80)
        dev_regs->in_ep[epnum].diepctl |= (1ul<<27) ; // set Nak
    else
        dev_regs->out_ep[epnum].doepctl |= (1ul<<27) ; // set Nak
}

/*
Name:       USBInEp0Ack
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
void USBInEp0Ack(void)
{
    USBWriteEp0(0, NULL);
}

/*
Name:       USBGetEpSts
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
uint8 USBGetEpSts(uint8 dir, uint8 epnum)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    uint8 status;

    if (dir!=0)
    {
        status = (dev_regs->in_ep[epnum].diepctl>>21) & 0x1;
    }
    else
    {
        status = (dev_regs->out_ep[epnum].doepctl>>21) & 0x1;  //Out endpoint send stall handshack
    }

    return status;
}


/*
Name:       USBSetEpSts
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
void USBSetEpSts(uint8 dir, uint8 epnum, uint8 status)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if (dir!=0)
    {
        if (status != 0)
        {
            volatile uint32 value = dev_regs->in_ep[epnum].diepctl;
            (value & (1ul<<31))? (value |= (1<<30 | 1<<21)) : (value |= (1<<21));
            dev_regs->in_ep[epnum].diepctl = value;  //In endpoint send stall handshack
        }
        else
            dev_regs->in_ep[epnum].diepctl &= ~(1<<21);
    }
    else
    {
        if (status != 0)
            dev_regs->out_ep[epnum].doepctl |= 1<<21;  //Out endpoint send stall handshack
        else
            dev_regs->out_ep[epnum].doepctl &= ~(1<<21);
    }
}

/*
Name:       USBGetStatus
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
static void USBGetStatus(pUDC_DRIVER pUDC)
{
    uint8 bRecipient;
    uint16 status;
    pUSB_CTRL_REQUEST ctrl = &pUDC->CtrlRequest;

    bRecipient = ctrl->bRequestType & USB_RECIP_MASK;

    if (bRecipient == USB_RECIP_ENDPOINT)
    {

        uint8 dir = ctrl->wIndex & (uint8)USB_ENDPOINT_DIR_MASK;
        uint8 EpNum = (uint8)(ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK);

        status = USBGetEpSts(dir , EpNum);
        USBWriteEp0(2, &status);
    }
    else if (bRecipient == USB_RECIP_DEVICE)
    {
        status = (uint16)(pUDC->RemoteWakeup<<1);
        USBWriteEp0(2, &status);
    }
}

/*
Name:       USBCleanFeature
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
static void USBCleanFeature(pUDC_DRIVER pUDC)
{
    uint8 endp,dir;
    uint8 bRecipient;
    pUSB_CTRL_REQUEST ctrl = &pUDC->CtrlRequest;
    pUSB_DEVICE  pDev = pUDC->pDev;

    bRecipient = ctrl->bRequestType & USB_RECIP_MASK;

    if (bRecipient == USB_RECIP_DEVICE              //清除远程唤醒功能
        && ctrl->wValue == USB_DEVICE_REMOTE_WAKEUP)
    {
        pUDC->RemoteWakeup = 0;
        USBInEp0Ack();
    }
    else if (bRecipient == USB_RECIP_ENDPOINT       //清除端点stall
        && ctrl->wValue == USB_ENDPOINT_HALT)
    {
        endp = (uint8)(ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK);
        dir = ctrl->wIndex & (uint8)USB_ENDPOINT_DIR_MASK;
//        USBDEBUG("CLEAR EP%d STALL", endp);
        USBSetEpSts(dir, endp, 0);     // clear RX/TX stall for OUT/IN on EPn.
        USBInEp0Ack();
        if (endp > 0)
        {
            USBResetToggle(dir, endp);
            pDev->request(endp, UDC_EV_CLEAN_EP, 0, pDev);
        }
    }
    else
        USBStallEp(0);
}


/*
Name:       USBCleanFeature
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
static void USBSetFeature(pUDC_DRIVER pUDC)
{
    uint8 bRecipient;
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    pUSB_CTRL_REQUEST ctrl = &pUDC->CtrlRequest;

    bRecipient = ctrl->bRequestType & USB_RECIP_MASK;
    USBInEp0Ack();

    if (bRecipient == USB_RECIP_DEVICE)
    {
        if (ctrl->wValue == USB_DEVICE_REMOTE_WAKEUP)
        {
            pUDC->RemoteWakeup = 1;
        }
        else if (ctrl->wValue == USB_DEVICE_TEST_MODE)
        {
            switch (ctrl->wIndex>>8)
            {
                case 0x01:  //test J
                    dev_regs->dctl = ((dev_regs->dctl|(1<<4))&(~(3<<5)));
                    break;
                case 0x02:  //test K
                    dev_regs->dctl = ((dev_regs->dctl|(1<<5))&(~(1<<6))&(~(1<<4)));
                    break;
                case 0x03:  //test SE0
                    dev_regs->dctl = ((dev_regs->dctl|(3<<4))&(~(1<<6)));
                    break;
                case 0x04:  //test packet
                    dev_regs->dctl = ((dev_regs->dctl|(1<<6))&(~(3<<4)));
                    break;
                case 0x05:  //Test Force Enable
                    Delay100cyc(12);        //延时10us @100MHz
                    dev_regs->dctl=(dev_regs->dctl & (~(0x07<<4))) | (((ctrl->wIndex>>8)%8)<<4);
                    break;
                default:
                    break;
            }
        }
    }
    else if (bRecipient == USB_RECIP_ENDPOINT       //清除端点stall
        && ctrl->wValue == USB_ENDPOINT_HALT)
    {
        uint8 endp,dir;

        endp = (uint8)(ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK);
        dir = ctrl->wIndex & (uint8)USB_ENDPOINT_DIR_MASK;
        USBSetEpSts(dir, endp, 1);
    }

}

/*
Name:       USBSetAddress
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
static void USBSetAddress(pUDC_DRIVER pUDC)
{
    pUSB_CTRL_REQUEST ctrl = &pUDC->CtrlRequest;
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->dcfg = (dev_regs->dcfg & (~0x07f0)) | (ctrl->wValue << 4);  //reset device addr
    USBDEBUG("SET ADDR = %d", ctrl->wValue);
    USBInEp0Ack();
}

/*
Name:       USBDeviceSetup
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
void USBDeviceSetup(pUDC_DRIVER pUDC)
{
    int32 ret = 0;
    uint8 type, req;
    pUSB_DEVICE  pDev = pUDC->pDev;
    pUSB_CTRL_REQUEST ctrl = &pUDC->CtrlRequest;

    type = ctrl->bRequestType & USB_TYPE_MASK;
    req = ctrl->bRequest & USB_REQ_MASK;

    /* handle non-standard (class/vendor) requests in the fun driver */
    if (type == USB_TYPE_CLASS || type == USB_TYPE_VENDOR)
    {
        pDev->setup(ctrl, pDev);
        return;
    }
    else if (type == USB_TYPE_STANDARD)
    {
        //// --- Standard Request handling --- ////
        switch (req)
        {
            case USB_REQ_GET_STATUS:
                USBDEBUG("GET STATUS");
                USBGetStatus(pUDC);
                break;

            case USB_REQ_CLEAR_FEATURE:
                USBCleanFeature(pUDC);
                break;

            case USB_REQ_SET_FEATURE:
                USBDEBUG("SET FEATURE");
                USBSetFeature(pUDC);
                break;

            case USB_REQ_SET_ADDRESS:
                USBSetAddress(pUDC);
                break;

            case USB_REQ_GET_DESCRIPTOR:
                ret = pDev->setup(ctrl, pDev);
                if (ret < 0)
                    USBGetDescriptor(ctrl, pDev);
                break;

            case USB_REQ_SET_CONFIGURATION:
                pUDC->connected = 1;
            case USB_REQ_GET_CONFIGURATION:
            case USB_REQ_GET_INTERFACE:
            case USB_REQ_SET_INTERFACE:
                pDev->setup(ctrl, pDev);
                break;

            case USB_REQ_SYNCH_FRAME:
                break;

            default:
                /* Call the fun Driver's setup functions */
                pDev->setup(ctrl, pDev);
                break;
        }
    }
    else
    {
        USBStallEp(0);
    }
}

/*
Name:       USBCtrlInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_CODE_
static void USBCtrlInit(bool FullSpeed)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    int count = 0;

    /* Wait for AHB master IDLE state. */
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1ul<<31))!=0)
            break;
    }

        //core soft reset
    otg_core->Core.grstctl |= 1<<0;               //Core soft reset
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1<<0))==0)
            break;
    }

    otg_core->ClkGate.pcgcctl = 0x0F;             /* Restart the Phy Clock */

    DelayMs(10);
    otg_core->ClkGate.pcgcctl = 0x00;             /* Restart the Phy Clock */


    //Non-Periodic TxFIFO Empty Level interrupt indicates that the IN Endpoint TxFIFO is completely empty
    otg_core->Core.gahbcfg |= 1<<7;

    /* Do device intialization*/

    //usb_write_reg32(USB_PCGCCTL, 0);

    /* High speed PHY running at high speed */
    if (!FullSpeed)
    {
        otg_core->Device.dcfg &= ~0x03;
    }
    else
    {
        //#ifdef FORCE_FS
        otg_core->Device.dcfg |= 0x01;   //Force FS
        //#endif
    }

    /* Reset Device Address */
    otg_core->Device.dcfg &= ~0x07F0;

    /* Flush the FIFOs */
    otg_core->Core.grstctl |= ( 0x10<<6) | (1<<5);     //Flush all Txfifo
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1<<5))==0)
            break;
    }

    otg_core->Core.grstctl |= 1<<4;              //Flush all Rxfifo
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1<<4))==0)
            break;
    }

    /* Flush the Learning Queue. */
    otg_core->Core.grstctl |= 1<<3;

    /* Clear all pending Device Interrupts */
    otg_core->Device.in_ep[0].diepctl = (1<<27)|(1<<30);        //IN0 SetNAK & endpoint disable
    otg_core->Device.in_ep[0].dieptsiz = 0;
    otg_core->Device.in_ep[0].diepint = 0xFF;

    otg_core->Device.out_ep[0].doepctl = (1<<27)|(1<<30);        //OUT0 SetNAK & endpoint disable
    otg_core->Device.out_ep[0].doeptsiz = 0;
    otg_core->Device.out_ep[0].doepint = 0xFF;

    otg_core->Device.diepmsk = 0x2F;              //device IN interrutp mask
    otg_core->Device.doepmsk = 0x0F;              //device OUT interrutp mask
    otg_core->Device.daint = 0xFFFFFFFF;          //clear all pending intrrupt
    otg_core->Device.daintmsk = 0x00010001;         //device all ep interrtup mask(IN0 & OUT0)
    otg_core->Core.gintsts = 0xFFFFFFFF;
    otg_core->Core.gotgint = 0xFFFFFFFF;

    otg_core->Core.gintmsk=(1<<4)|/*(1<<5)|*/(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<18)|(1<<19)|(1ul<<30)|(1ul<<31);
    otg_core->Core.gahbcfg |= 0x01;        //Global interrupt mask

}

/*
Name:       USBConnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_CODE_
int32 USBConnect(bool FullSpeed)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    //DISABLE_USB_INT;
    dev_regs->dctl |= 0x02;         //soft disconnect
    //USBResetPhy();
    USBCtrlInit(FullSpeed);
    dev_regs->dctl &= ~0x02;        //soft connect
    //DelayMs(1000);
    //ENABLE_USB_INT;

    DisableIntMaster();
    UsbIntEnable();
    EnableIntMaster();

    return 0;
}

/*
Name:       USBDisconnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
void USBDisconnect(void)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    dev_regs->dctl |= 0x02;         //soft disconnect
}

/*
Name:       USBReConnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
int32 USBReConnect(uint32 param)
{
    IntRegister(INT_ID_USBC, USBDeviceIsr);

    if (1 == param)
    {
        USBConnect(0);
    }
    else
    {
        //ENABLE_USB_INT;
        DisableIntMaster();
        UsbIntEnable();
        EnableIntMaster();
    }
    return 0;
}

/*
Name:       USBWaitConnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
int32 USBWaitConnect(int32 timeout)
{
    timeout *= 1000;

    while (CheckVbus() && !UDCDriver.connected && timeout--)
    {
        //每隔一秒喂一次狗
        if ((timeout%1000000) == 0)
        {
            #ifdef _WATCH_DOG_
            WatchDogReload();
            #endif
        }
        USBDelayUS(1);
    }
    DEBUG("connect = %d", UDCDriver.connected);
    return (UDCDriver.connected)?  1 : 0;
}

/*
Name:       USBIsConnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
uint8 USBIsConnect(void)
{
    return (UDCDriver.connected);
}

/*
Name:       USBIsConnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
uint8 USBIsSuspend(void)
{
    return (UDCDriver.suspend);
}

/*
Name:       USBDriverProbe
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
int32 USBDriverProbe(USB_DEVICE *pDev)
{
    pUDC_DRIVER pUDC = &UDCDriver;

    if (NULL == pUDC->pDev)
    {
        pUDC->pDev = pDev;
        pUDC->ClassTypeMask |= pDev->ClassType;
        pUDC->DevNum++;
    }
    else
    {
        pUSB_DEVICE next;

        if (pUDC->DevNum >= MAX_UDC_DEVICES)
            return -1;

        next = pUDC->pDev;
        while (next->next)
        {
            next = next->next;
        }

        next->next = pDev;
        pDev->prev = next;
        pUDC->ClassTypeMask |= pDev->ClassType;
        pUDC->DevNum++;
    }

    return 0;
}

/*
Name:       USBDriverInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
int32 USBDriverInit(void)
{
    pUDC_DRIVER pUDC = &UDCDriver;

    memset(pUDC, 0, sizeof(UDC_DRIVER));

    pUDC->ep0req.buf = (uint8*)USBEpBuf;
    return 0;
}

/*
Name:       GetUDCDriver
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_DRIVER_CODE_
pUSB_EP0_REQ USBGetEp0Req(void)
{
    return &UDCDriver.ep0req;
}

#endif

