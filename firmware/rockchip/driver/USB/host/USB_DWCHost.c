/* Copyright (C) 2016 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: USB_DWCHost.c
Desc:

Author: wrm@rock-chips.com
Date: 16-09-05
Notes:

$Log: $
 *
 *
*/
#include "driver/CRU/Hw_cru.h"  /* USB*_SRST, FREQ_* gate IDs */
#include "driver/GRF/grf.h"    /* Grf instance */
#include "driver/GRF/hw_grf.h" /* GRF registers */
#include "freq_enums.h"       /* FREQ_USB */
#include "SysInclude.h"
#include "OsInclude.h"


#ifdef _USB_HOST_
#include "USBConfig.h"


UHC_CHN_INFO gDWCHostInfo;
USB_REQ  gURBCache;
uint8 gUSBHost_InitState = 0;
uint8 gUSBHost_forbidInit = 0;



/*
*------------------------------------------------------------------------------------
*
*                          local function(common) define
*
*------------------------------------------------------------------------------------
*/
#if 1
/*******************************************************************************
** Name:   USBResetPhy
** Input:  void
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostResetPhy(void)
{
    ScuSoftResetCtr(USBGLB_SRST, 1);
    ScuSoftResetCtr(USBOTG_SRST, 1);
    DelayUs(20);
    ScuSoftResetCtr(USBGLB_SRST, 0);
    ScuSoftResetCtr(USBOTG_SRST, 0);

    ScuSoftResetCtr(USBPHY_SRST, 1);  //usbphy_sft_rst
    DelayMs(10);
    ScuSoftResetCtr(USBPHY_SRST, 0);
    DelayMs(1);
}

/*******************************************************************************
** Name:   HostControllerInit
** Input:  bool FullSpeed
** Return: void
** Owner:  wrm
** Date:   2015.9.29
** Time:   15:24:17
*******************************************************************************/
void HostControllerInit(bool FullSpeed)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    GINTMSK_DATA gintmsk;
    GAHBCFG_DATA ahbcfg;
    int count = 0;

    /* Wait for AHB master IDLE state. */
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1ul<<31))!=0)
            break;
        DelayUs(10);
    }

    otg_core->ClkGate.pcgcctl = 0x00;    //Restart the Phy Clock
    //printf ("\nClkGate.pcgcctl Adress=0x%x Value=0x%x\n",
    //          &otg_core->ClkGate.pcgcctl,
    //          otg_core->ClkGate.pcgcctl);

    //Core soft reset
    otg_core->Core.grstctl |= 1<<0;
    for (count=0; count<10000; count++)
    {
        if ((otg_core->Core.grstctl & (1<<0))==0)
            break;
    }

    for (count=0; count<5000; count++)
    {
        if ((otg_core->Core.gintsts & 0x1)!=0)
            break;

        DelayMs(1);
    }

    otg_core->Host.hprt |= (0x01<<12);          //power on the port
    otg_core->Core.gintsts=0xffffffff;
    otg_core->Core.gotgint=0xffffffff;


    gintmsk.d32 = 0;
    gintmsk.b.disconnint = 1;
    gintmsk.b.hchint = 1;
    gintmsk.b.prtint = 1;
    gintmsk.b.rxflvl = 1;
    otg_core->Core.gintmsk = gintmsk.d32;

    ahbcfg.d32 = otg_core->Core.gahbcfg;
    ahbcfg.b.glblintrmsk = 1;
    ahbcfg.b.nptxfemplvl = DWC_GAHBCFG_TXFEMPTYLVL_EMPTY;
    otg_core->Core.gahbcfg = ahbcfg.d32;      //Global Interrupt Mask, operates in Slave mode
}


/*******************************************************************************
** Name:   HostPortIsr
** Input:  void
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostPortIsr(void)
{
    HPRT0_DATA hprt;
    HPRT0_DATA clean_hprt;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;

    hprt.d32 = otg_core->Host.hprt;
    clean_hprt.d32 = hprt.d32;

    /* Clear appropriate bits in HPRT0 to clear the interrupt bit in
     * GINTSTS */
    clean_hprt.b.prtena = 0;
    clean_hprt.b.prtconndet = 0;
    clean_hprt.b.prtenchng = 0;
    clean_hprt.b.prtovrcurrchng = 0;

    /* Port Connect Detected
     * Set flag and clear if detected */
    if(hprt.b.prtconndet)
    {
        clean_hprt.b.prtconndet = 1;
    }
    if (hprt.b.prtenchng)
    {
        clean_hprt.b.prtenchng = 1;
    }
    if(hprt.b.prtovrcurrchng)
    {
        /** Overcurrent Change Interrupt */
        clean_hprt.b.prtovrcurrchng = 1;
        //disable the port and power off
        clean_hprt.b.prtpwr = 0;
    }

    /* Clear Port Interrupts */
    otg_core->Host.hprt = clean_hprt.d32;
}

/*******************************************************************************
** Name:   HostRxData
** Input:  int8 chn, uint32 *pBuf, uint32 len
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostRxData(int8 chn, uint32 *pBuf, uint32 len)
{
    uint32 * FIFO = (uint32 *)HC_CHN_FIFO(chn);
    int i = 0;

    while(len--)
    {
        *pBuf++ = *FIFO;
    }
}


/*******************************************************************************
** Name:   HostRxPkt
** Input:  UHC_CHN_INFO *hc, uint32 bytes
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostRxPkt(UHC_CHN_INFO *hc, uint32 bytes)
{
    uint32 dword = (bytes + 3) >>2;

    /* xferbuf must be DWORD aligned. */
    HostRxData(hc->chn, (uint32 *)hc->XferBuf, dword);

    hc->XferCnt += bytes;
    hc->XferBuf += bytes;

}


/*******************************************************************************
** Name:   HostGetChn
** Input:  int8 chn, UHC_CHN_INFO *pUsbHC
** Return: UHC_CHN_INFO*
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
UHC_CHN_INFO* HostGetChn(int8 chn, UHC_CHN_INFO * pUsbHC)
{
    if (chn >= MAX_HOST_CHN)
        return NULL;

    return pUsbHC;
}


/*******************************************************************************
** Name:   HostRxflvlIsr
** Input:  UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostRxflvlIsr(UHC_CHN_INFO * pUsbHC)
{
    int8 chn;
    GRXSTSH_DATA grxsts;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;

    otg_core->Core.gintmsk |= (1<<4);

    grxsts.d32 = otg_core->Core.grxstsp;

    chn = grxsts.b.chnum;

    switch (grxsts.b.pktsts)
    {
        case DWC_GRXSTS_PKTSTS_IN:
        /* Read the data into the host buffer.    */
        if (grxsts.b.bcnt > 0)
        {
            HostRxPkt(HostGetChn(chn, pUsbHC), grxsts.b.bcnt);
        }

        case DWC_GRXSTS_PKTSTS_IN_XFER_COMP:
        case DWC_GRXSTS_PKTSTS_DATA_TOGGLE_ERR:
        case DWC_GRXSTS_PKTSTS_CH_HALTED:
        /* Handled in interrupt, just ignore data */
            break;
        default:
            otg_core->Core.gintmsk &= ~(1<<4);
            break;
    }
}


/*******************************************************************************
** Name:   HostHaltChn
** Input:  UHC_CHN_INFO * pUsbHC
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
int32 HostHaltChn(UHC_CHN_INFO* hc, uint32 status)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    HCCHAR_DATA hcchar;
    GNPTXSTS_DATA txsts;
    int32 ret = 1;

    if (hc->HaltPending)
    {
        return ret;
    }

    hc->status = status;
    hcchar.d32 = pChnReg->hccharn;
    if (hcchar.b.chen == 0)
    {
        ret = 0;
        return ret;
    }

    txsts.d32 = otg_core->Core.gnptxsts;
    hcchar.b.chen = (txsts.b.nptxqspcavail)? 1 : 0;

    hcchar.b.chdis = 1;
    pChnReg->hccharn = hcchar.d32;

    if (hcchar.b.chen)
    {
        hc->HaltPending = 1;
        hc->HaltOnQueue = 0;
    }
    else
    {
        hc->HaltOnQueue = 1;
    }

    if (hc->HaltOnQueue)
    {
        GINTMSK_DATA gintmsk;
        gintmsk.d32 = otg_core->Core.gintmsk;
        gintmsk.b.nptxfemp = 1;
        otg_core->Core.gintmsk = gintmsk.d32;
    }

    return ret;
}

/*******************************************************************************
** Name:   HostDoPing
** Input:  UHC_CHN_INFO * hc
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostDoPing(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCTSIZ_DATA hctsiz;
    HCCHAR_DATA hcchar;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    hctsiz.d32 = 0;
    hctsiz.b.dopng = 1;
    hctsiz.b.pktcnt = 1;
    pChnReg->hctsizn =  hctsiz.d32;

    hcchar.d32 = pChnReg->hccharn;
    hcchar.b.chen = 1;
    hcchar.b.chdis = 0;

    pChnReg->hccharn = hcchar.d32;
}

/*******************************************************************************
** Name:   HostTxData
** Input:  int8 chn, uint32 *pBuf, uint32 len
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostTxData(int8 chn, uint32 *pBuf, uint32 len)
{
    uint32 *FIFO = (uint32 *)HC_CHN_FIFO(chn);
    //printf ("TX:chn=%d len=%d\n",chn,len);
    //printf ("TX:pBuf[0]=0x%x\n",pBuf[0]);
    //printf ("TX:pBuf[1]=0x%x\n",pBuf[1]);
    //printf ("TX:FIFO Adress=0x%x\n",FIFO);

    while(len--)
    {
        *FIFO = *pBuf++;
    }
    //printf ("TX Data Complete\n");
}


/*******************************************************************************
** Name:   HostTxPkt
** Input:  UHC_CHN_INFO * hc
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostTxPkt(UHC_CHN_INFO *hc)
{
    uint32 remain;
    uint32 byte;
    uint32 dword;

    remain = hc->XferLen - hc->XferCnt;
    byte =  (remain > hc->MaxPkt) ? hc->MaxPkt : remain;

    dword = (byte+3)>>2;

    /* xferbuf must be DWORD aligned. */
    HostTxData(hc->chn, (uint32 *)hc->XferBuf, dword);

    hc->XferCnt += byte;
    hc->XferBuf += byte;
}


/*******************************************************************************
** Name:   HostStartXfer
** Input:  UHC_CHN_INFO * hc
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostStartXfer(UHC_CHN_INFO *hc)
{
    USB_OTG_REG *otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCCHAR_DATA hcchar;
    HCTSIZ_DATA hctsiz;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    uint16  NumPkt;

    hctsiz.d32 = 0;
    if (hc->DoPing)
    {
        HostDoPing(hc);
        hc->XferStarted = 1;
        return;
    }
    if (hc->XferLen > 0)
    {
        NumPkt = (hc->XferLen + hc->MaxPkt- 1) / hc->MaxPkt;
    }
    else
    {
        /* Need 1 packet for transfer length of 0. */
        NumPkt = 1;
    }

    if (hc->EpIsIn)
    {
        /* Always program an integral # of max packets for IN transfers. */
        hc->XferLen = NumPkt * hc->MaxPkt;
    }

    hctsiz.b.xfersize = hc->XferLen;

    hc->StartPktCnt = NumPkt;
    hctsiz.b.pktcnt = NumPkt;;
    hctsiz.b.pid = hc->PidStart;

    pChnReg->hctsizn = hctsiz.d32;

    hcchar.d32 = pChnReg->hccharn;
    /* Set host channel enable after all other setup is complete. */
    hcchar.b.chen = 1;
    hcchar.b.chdis = 0;
    pChnReg->hccharn = hcchar.d32;

    hc->XferStarted = 1;

    if (!hc->EpIsIn && hc->XferLen > 0)
    {
        /* Load OUT packet into the appropriate Tx FIFO. */
        HostTxPkt(hc);
    }
}


/*******************************************************************************
** Name:   HostContinueXfer
** Input:  UHC_CHN_INFO * hc
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
int32 HostContinueXfer(UHC_CHN_INFO *hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    if (hc->PidStart == DWC_OTG_HC_PID_SETUP)
    {
        /* SETUPs are queued only once since they can't be NAKed. */
        return 0;
    }
    else if (hc->EpIsIn)
    {
        HCCHAR_DATA hcchar;
        HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

        hcchar.d32 = pChnReg->hccharn;
        hcchar.b.chen = 1;
        hcchar.b.chdis = 0;
        pChnReg->hccharn = hcchar.d32;

        return 1;
    }
    else
    {
        /* OUT transfers. */
        if (hc->XferCnt < hc->XferLen)
        {
            HostTxPkt(hc);
            return 1;
        }
        else
        {
            return 0;
        }
    }
}


/*******************************************************************************
** Name:   HostProcessXfer
** Input:  UHC_CHN_INFO * pUsbHC
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
int32 HostProcessXfer(UHC_CHN_INFO * pUsbHC)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    int32 i = 0;
    int32 ret = 0;
    GINTMSK_DATA gintmsk;
    GNPTXSTS_DATA txsts;
    UHC_CHN_INFO* hc = pUsbHC;

    for(i=0; i<MAX_HOST_CHN; i++, hc++)
    {
        if (hc->valid && hc->urb && !hc->urb->completed)
        {
            break;
        }
    }

    if (i >= MAX_HOST_CHN)
    {
        goto OUT;
    }

    txsts.d32 = otg_core->Core.gnptxsts;
    if (txsts.b.nptxqspcavail == 0)
    {
        ret = 1;
    }
    else if (hc->HaltPending)
    {
        ret = 0;
    }
    else if (hc->HaltOnQueue)
    {
        HostHaltChn(hc, hc->status);
        ret = 0;
    }
    else if (hc->DoPing)
    {
        if (!hc->XferStarted)
        {
            HostStartXfer(hc);
        }
        ret = 0;
    }
    else if (!hc->EpIsIn || hc->PidStart == DWC_OTG_HC_PID_SETUP)
    {
        if (txsts.b.nptxfspcavail*4 >= hc->MaxPkt)
        {
            if (!hc->XferStarted)
            {
                HostStartXfer(hc);
                ret = 1;
            }
            else
            {
                ret = HostContinueXfer(hc);
            }
        }
        else
        {
            ret = -1;
        }
    }
    else
    {

        if (!hc->XferStarted)
        {
            HostStartXfer(hc);
            ret = 1;
        }
        else
        {
            ret = HostContinueXfer(hc);
        }
    }
OUT:
    /*
    * Ensure NP Tx FIFO empty interrupt is disabled when
    * there are no non-periodic transfers to process.
    */
    //printf ("wrm_5_3 ret=%d\n",ret);
    gintmsk.d32 = otg_core->Core.gintmsk;
    gintmsk.b.nptxfemp = (ret != 0);
    otg_core->Core.gintmsk = gintmsk.d32;
    return ret;
}

/*******************************************************************************
** Name:   HostUpdateXfer
** Input:  UHC_CHN_INFO* hc, uint32 ChnStatus, int32 *ShortRead
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
int32 HostGetActualLen(UHC_CHN_INFO* hc, uint32 ChnStatus, int32 *ShortRead)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCTSIZ_DATA hctsiz;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    uint32     length;

    hctsiz.d32 = pChnReg->hctsizn;

    if (ChnStatus == HC_XFER_COMPL)
    {
        if (hc->EpIsIn) //ep_is_in
        {
            length = hc->XferLen - hctsiz.b.xfersize;
            if (ShortRead != NULL)
            {
                *ShortRead = (hctsiz.b.xfersize != 0);
            }
        }
        else
        {
            length = hc->XferLen;
        }
    }
    else
    {
        /*
         * Must use the hctsiz.pktcnt field to determine how much data
         * has been transferred. This field reflects the number of
         * packets that have been transferred via the USB. This is
         * always an integral number of packets if the transfer was
         * halted before its normal completion. (Can't use the
         * hctsiz.xfersize field because that reflects the number of
         * bytes transferred via the AHB, not the USB).
         */
        length = (hc->StartPktCnt - hctsiz.b.pktcnt) * hc->MaxPkt;
    }

    return length;

}


/*******************************************************************************
** Name:   HostUpdateXfer
** Input:  UHC_CHN_INFO* hc, USB_REQ *urb, uint32 ChnStatus
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
int32 HostUpdateXfer(UHC_CHN_INFO* hc, USB_REQ *urb, uint32 ChnStatus)
{
    int32 XferDone = 0;

    if (HC_XFER_COMPL == ChnStatus)
    {
        int32 ShortRead = 0;

        urb->ActualLen += HostGetActualLen(hc, ChnStatus, &ShortRead);
        if (ShortRead || (urb->ActualLen >= urb->BufLen))
        {
           XferDone = 1;
        }
    }
    else
    {
        urb->ActualLen += HostGetActualLen(hc, ChnStatus, NULL);
    }

    return XferDone;
}

/*******************************************************************************
** Name:   HostCompleteUrb
** Input:  USB_REQ *urb, int32 status
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostCompleteUrb(USB_REQ *urb, int32 status)
{
    urb->status = status;
    urb->completed = 1;
}

/*******************************************************************************
** Name:   HostInitChnReg
** Input:  UHC_CHN_INFO *hc
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostInitChnReg(UHC_CHN_INFO *hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg;
    HCCHAR_DATA hcchar;
    HCINTMSK_DATA hcintmaskn;
    HOST_REG *pHostReg = &otg_core->Host;

    pChnReg = &pHostReg->hchn[hc->chn];

    /* Clear old interrupt conditions for this host channel. */
    hcintmaskn.d32 = 0xFFFFFFFF;
    hcintmaskn.b.reserved = 0;
    pChnReg->hcintn = hcintmaskn.d32;

    /* Enable channel interrupts required for this transfer. */
    hcintmaskn.d32 = 0;
    hcintmaskn.b.chhltd = 1;

    switch (hc->EpType)
    {
        case DWC_OTG_EP_TYPE_CONTROL:
        case DWC_OTG_EP_TYPE_BULK:
            hcintmaskn.b.xfercompl = 1;
            hcintmaskn.b.stall = 1;
            hcintmaskn.b.xacterr = 1;
            hcintmaskn.b.datatglerr = 1;
            if (hc->EpIsIn)
            {
                hcintmaskn.b.bblerr = 1;
            }
            else
            {
                hcintmaskn.b.nak = 1;
                hcintmaskn.b.nyet = 1;
                if (hc->DoPing)
                {
                    hcintmaskn.b.ack = 1;
                }
            }
            if (hc->ErrorState)
            {
                hcintmaskn.b.ack = 1;
            }
            break;
        default:
            break;
    }
    //printf ("HostInitChnReg:pChnReg->hcintmaskn=0x%x\n",hcintmaskn.d32);

    pChnReg->hcintmaskn = hcintmaskn.d32;
    pHostReg->haintmsk |= (0x1ul<<hc->chn);
    /* Make sure host channel interrupts are enabled. */
    //gintmsk.b.hchint = 1;

    hcchar.d32 = 0;
    hcchar.b.devaddr = hc->DevAddr;
    hcchar.b.epnum = hc->EpNum;
    hcchar.b.epdir = hc->EpIsIn;
    hcchar.b.lspddev = (hc->speed == DWC_OTG_EP_SPEED_LOW); //HIGH SPEED
    hcchar.b.eptype = hc->EpType;
    hcchar.b.mps = hc->MaxPkt;

    pChnReg->hccharn = hcchar.d32;
    //printf ("HostInitChnReg:pChnReg->hccharn=0x%x\n",pChnReg->hccharn);

}


/*******************************************************************************
** Name:   HostInitChn
** Input:  UHC_CHN_INFO * pUsbHC
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostInitChn(UHC_CHN_INFO *hc, USB_REQ *urb)
{
    HOST_PIPE pipe = urb->pipe;

    hc->DevAddr = pipe.b.devaddr;
    hc->EpNum  = pipe.b.epnum;
    hc->MaxPkt = pipe.b.mps;
    hc->EpType = pipe.b.eptype;
    hc->EpIsIn = pipe.b.epdir;

    hc->speed = urb->pDev->speed;

    hc->XferStarted = 0;
    hc->status = HC_XFER_IDLE;
    hc->ErrorState = (urb->ErrCnt > 0);
    hc->HaltPending = 0;
    hc->HaltOnQueue = 0;
    hc->valid = urb->valid;

    hc->DoPing = urb->PingState;
    hc->PidStart = (urb->DataToggle)? DWC_OTG_HC_PID_DATA1 : DWC_OTG_HC_PID_DATA0;

    hc->XferBuf = (uint8*)((uint32)urb->TransBuf + urb->ActualLen);
    hc->XferLen = urb->BufLen - urb->ActualLen;
    hc->XferCnt = 0;
#if 0
    printf ("*****************************\n");
    printf ("\nHostInitChn:hc->DevAddr=%d\n",hc->DevAddr);
    printf ("\nHostInitChn:hc->EpNum=%d\n",hc->EpNum);
    printf ("\nHostInitChn:hc->MaxPkt=%d\n",hc->MaxPkt);
    printf ("\nHostInitChn:hc->EpType=%d\n",hc->EpType);
    printf ("\nHostInitChn:hc->EpIsIn=%d\n",hc->EpIsIn);
    printf ("\nHostInitChn:hc->speed=%d\n",hc->speed);
    printf ("\nHostInitChn:hc->ErrorState=%d\n",hc->ErrorState);
    printf ("\nHostInitChn:hc->DoPing=%d\n",hc->DoPing);
    printf ("\nHostInitChn:hc->PidStart=%d\n",hc->PidStart);
    printf ("\nHostInitChn:urb->BufLen=%d urb->ActualLen=%d\n",
            urb->BufLen,
            urb->ActualLen);
    printf ("\nHostInitChn:hc->XferLen=%d\n",hc->XferLen);
    printf ("\nHostInitChn:hc->valid=%d\n",hc->valid);

    printf ("\nHostInitChn:urb->CtrlPhase=%d\n",urb->CtrlPhase);

    printf ("*****************************\n");
#endif
    switch (hc->EpType)
    {
        case DWC_OTG_EP_TYPE_CONTROL:
            switch (urb->CtrlPhase)
            {
                case DWC_OTG_CONTROL_SETUP:
                    hc->DoPing = 0;
                    hc->EpIsIn = 0;
                    hc->PidStart = DWC_OTG_HC_PID_SETUP;
                    hc->XferBuf = (uint8*)urb->SetupPkt;
                    hc->XferLen = 8;
                    break;

                 case DWC_OTG_CONTROL_DATA:
                    hc->PidStart = (urb->DataToggle)? DWC_OTG_HC_PID_DATA1 : DWC_OTG_HC_PID_DATA0;
                    break;

                 case DWC_OTG_CONTROL_STATUS:
                    /*
                    * Direction is opposite of data direction or IN if no
                    * data.
                    */
                    if (urb->BufLen == 0)
                        hc->EpIsIn = 1;
                    else
                        hc->EpIsIn = (pipe.b.epdir != 1);

                    if (hc->EpIsIn)
                        hc->DoPing = 0;

                    hc->PidStart = DWC_OTG_HC_PID_DATA1;
                    hc->XferLen = 0;
                    hc->XferBuf = (uint8*)urb->StatusBuf;
                    break;
                default:
                    break;
            }
            break;

        case DWC_OTG_EP_TYPE_BULK:
            break;
        default:
            break;
    }

    hc->urb = urb;
    HostInitChnReg(hc);
}


/*******************************************************************************
** Name:   HostReleaseChn
** Input:  UHC_CHN_INFO* hc
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostReleaseChn(UHC_CHN_INFO * hc)
{
    USB_OTG_REG *otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_REG *pHostReg = &otg_core->Host;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ  *urb = hc->urb;

    if (HC_XFER_XERR==hc->status)
    {
        if (urb->ErrCnt >= 3)
        {
            HostCompleteUrb(urb, -HOST_XERR);
        }
    }

    hc->HaltPending = 0;
    hc->XferStarted = 0;
    pChnReg->hcintmaskn = 0;
    pChnReg->hcintn = 0xFFFFFFFF;
    pHostReg->haintmsk &= ~(0x1ul<<hc->chn);

    if (!urb->completed)
    {
        HostInitChn(hc, hc->urb);
        HostProcessXfer(hc);
    }
}


/*******************************************************************************
** Name:   HostCompleteXfer
** Input:  UHC_CHN_INFO* hc, HOST_CHANNEL_REG *pChnReg, uint32 status
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostCompleteXfer(UHC_CHN_INFO* hc, HOST_CHANNEL_REG *pChnReg, uint32 status)
{
    HCINT_DATA hcint;
    int32 ret;

    hcint.d32 = pChnReg->hcintn;
    if (hcint.b.nyet)
    {
        //printf ("HostCompleteXfer_1\n");
        /*
         * Got a NYET on the last transaction of the transfer. This
         * means that the endpoint should be in the PING state at the
         * beginning of the next transfer.
         */
        hc->urb->PingState = 1;
        CLEAN_HC_INT(pChnReg, nyet);
    }

    if (hc->EpIsIn) //ep_is_in
     {
        /*
         * IN transfers in Slave mode require an explicit disable to
         * halt the channel. (In DMA mode, this call simply releases
         * the channel.)
         */
        ret = HostHaltChn(hc, status);
        //printf ("HostCompleteXfer_2 ret=%d\n",ret);
        if (ret == 0)
            HostReleaseChn(hc);
    }
    else
    {
        //printf ("HostCompleteXfer_3\n");
        /*
         * The channel is automatically disabled by the core for OUT
         * transfers in Slave mode.
         */
        hc->status = status;
        HostReleaseChn(hc);
    }
}


/*******************************************************************************
** Name:   HostSaveToggle
** Input:  void
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostSaveToggle(UHC_CHN_INFO* hc, USB_REQ *urb)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCTSIZ_DATA hctsiz;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    hctsiz.d32 = pChnReg->hctsizn;
    //printf ("HostSaveToggle Data chn=%d\n",hc->chn);
    //printf ("HostSaveToggle Data pid=%d\n",hctsiz.b.pid);
    if (hctsiz.b.pid == DWC_HCTSIZ_DATA0)
    {
        urb->DataToggle = 0;
    }
    else//(hctsiz.b.pid == DWC_HCTSIZ_DATA1)
    {
        urb->DataToggle = 1;
    }
}


/*******************************************************************************
** Name:   HostXfercompIsr
** Input:  void
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostXfercompIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg = &otg_core->Host.hchn[hc->chn];
    USB_REQ   *urb    = hc->urb;
    uint32 status = HC_XFER_COMPL;

    DISABLE_HC_INT(pChnReg, xfercompl);
    //printf ("HostXfercompIs EpType=%d\n",hc->EpType);
    switch (hc->EpType) /** 0: Control, 1: Isoc, 2: Bulk, 3: Intr */
    {
        case DWC_OTG_EP_TYPE_CONTROL:
            //printf ("HostXfercompIs CtrlPhase=%d\n",urb->CtrlPhase);
            switch (urb->CtrlPhase)
            {
                case DWC_OTG_CONTROL_SETUP:
                    if (urb->BufLen > 0)
                        urb->CtrlPhase = DWC_OTG_CONTROL_DATA;
                    else
                        urb->CtrlPhase = DWC_OTG_CONTROL_STATUS;;
                    break;
                case DWC_OTG_CONTROL_DATA:
                    HostUpdateXfer(hc, urb, HC_XFER_COMPL);
                    urb->CtrlPhase = DWC_OTG_CONTROL_STATUS;
                    break;
                case DWC_OTG_CONTROL_STATUS:
                    HostCompleteUrb(urb, 0);
                    status = HC_XFER_URB_COMPL;
                    break;
                default:
                    break;
            }

            HostCompleteXfer(hc, pChnReg, status);
            break;
        case DWC_OTG_EP_TYPE_BULK:
            HostUpdateXfer(hc, urb, HC_XFER_COMPL);
            HostSaveToggle(hc, urb);
            HostCompleteUrb(urb, 0);
            status = HC_XFER_URB_COMPL;
            HostCompleteXfer(hc, pChnReg, status);
            break;

        default:
            break;
    }
}

/*******************************************************************************
** Name:   HostChhltdIsr
** Input:  UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostChhltdIsr(UHC_CHN_INFO* hc)
{
    HostReleaseChn(hc);
}


/*******************************************************************************
** Name:   HostStallIsr
** Input:  UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostStallIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    HostCompleteUrb(hc->urb, -HOST_STALL);

    if (hc->EpType == DWC_OTG_EP_TYPE_BULK)
    {
        /*
        * USB protocol requires resetting the data toggle for bulk
        * and interrupt endpoints when a CLEAR_FEATURE(ENDPOINT_HALT)
        * setup command is issued to the endpoint. Anticipate the
        * CLEAR_FEATURE command since a STALL has occurred and reset
        * the data toggle now.
        */
        hc->urb->DataToggle = 0;
    }

    HostHaltChn(hc, HC_XFER_STALL);                //Disable Channel

    DISABLE_HC_INT(pChnReg, stall);
}

/*******************************************************************************
** Name:   HostNakIsr
** Input:  UHC_CHN_INFO * hc
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostNakIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ     *urb = hc->urb;

    switch (hc->EpType) /** 0: Control, 1: Isoc, 2: Bulk, 3: Intr */
    {
        case DWC_OTG_EP_TYPE_CONTROL:
        case DWC_OTG_EP_TYPE_BULK:
            //Reset Error Count
            urb->ErrCnt = 0;

            if (!urb->PingState)
            {
                HostUpdateXfer(hc, urb, HC_XFER_NAK);
                HostSaveToggle(hc, urb);

                if (USB_SPEED_HS==hc->speed && !hc->EpIsIn)
                {
                    urb->PingState = 1;
                }
            }

            HostHaltChn(hc, HC_XFER_NAK);
            break;
        default:
            break;
    }

    CLEAN_HC_INT(pChnReg, nak);
    DISABLE_HC_INT(pChnReg, nak);
}

/*******************************************************************************
** Name:   HostAckIsr
** Input:  UHC_CHN_INFO * hc
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostAckIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ     *urb = hc->urb;

    //Reset Error Count
    urb->ErrCnt = 0;

    if (urb->PingState)
    {
        urb->PingState = 0;
        HostHaltChn(hc, HC_XFER_ACK);
    }

    DISABLE_HC_INT(pChnReg, ack);
}

/*******************************************************************************
** Name:   HostNyetIsr
** Input:  UHC_CHN_INFO * hc
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostNyetIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ     *urb = hc->urb;

    printf("NYET\n");
    //Reset Error Count
    urb->ErrCnt = 0;
    urb->PingState = 1;

    HostUpdateXfer(hc, urb, HC_XFER_NYET);
    HostSaveToggle(hc, urb);

    /*
    * Halt the channel and re-start the transfer so the PING
    * protocol will start.
    */
    HostHaltChn(hc, HC_XFER_NYET);

    DISABLE_HC_INT(pChnReg, nyet);
}

/*******************************************************************************
** Name:   HostXacterrIsr
** Input:  UHC_CHN_INFO * hc
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostXacterrIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];
    USB_REQ     *urb = hc->urb;

    //Reset Error Count
    urb->ErrCnt++;
    if (!urb->PingState)
    {
        HostUpdateXfer(hc, urb, HC_XFER_XERR);
        HostSaveToggle(hc, urb);

        if (USB_SPEED_HS==hc->speed && !hc->EpIsIn)
        {
            urb->PingState = 1;
        }
    }
    HostHaltChn(hc, HC_XFER_XERR);

    DISABLE_HC_INT(pChnReg, xacterr);
}

/*******************************************************************************
** Name:   HostBabbleIsr
** Input:  UHC_CHN_INFO * hc
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostBabbleIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    HostCompleteUrb(hc->urb, -HOST_BABBLE);
    HostHaltChn(hc, HC_XFER_BABBLE);

    DISABLE_HC_INT(pChnReg, bblerr);
}

/*******************************************************************************
** Name:   HostTglerrIsr
** Input:  UHC_CHN_INFO * hc
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostTglerrIsr(UHC_CHN_INFO* hc)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[hc->chn];

    if (hc->EpIsIn) //ep_is_in
    {
        hc->urb->ErrCnt = 0;
    }
    else
    {
        //printf("Data Toggle Error on OUT transfer,""channel %d\n", _hc->hc_num);
    }

    DISABLE_HC_INT(pChnReg, datatglerr);
}


/*******************************************************************************
** Name:   int8 chn
** Input:  UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostChnIsr(int8 chn, UHC_CHN_INFO * pUsbHC)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    HCINT_DATA hcint;
    HCINTMSK_DATA hcintmsk;
    HOST_CHANNEL_REG *pChnReg  = &otg_core->Host.hchn[chn];
    UHC_CHN_INFO* hc = HostGetChn(chn, pUsbHC);

    hcint.d32 = pChnReg->hcintn;
    hcintmsk.d32 = pChnReg->hcintmaskn;

    hcint.d32 = hcint.d32 & hcintmsk.d32;

    if ((hcint.b.chhltd) && (hcint.d32 != 0x2))
    {
        hcint.b.chhltd = 0;
    }

    if (hcint.b.xfercomp)
    {
        //printf ("Xfer TX Complete\n");
        HostXfercompIsr(hc);
        /*
         * If NYET occurred at same time as Xfer Complete, the NYET is
         * handled by the Xfer Complete interrupt handler. Don't want
         * to call the NYET interrupt handler in this case.
         */
        hcint.b.nyet = 0;
    }
    if (hcint.b.chhltd)
    {
        //printf ("Host ReleaseChn\n");
        HostChhltdIsr(hc);
    }
    if (hcint.b.ahberr)
    {
        ;
    }
    if (hcint.b.stall)
    {
        //printf ("Host Stall\n");
        HostStallIsr(hc);
    }
    if (hcint.b.nak)
    {
        //printf ("Host Nak\n");
        HostNakIsr(hc);
    }
    if (hcint.b.ack)
    {
        //printf ("Host Ack\n");
        HostAckIsr(hc);
    }
    if (hcint.b.nyet)
    {
        //printf ("Host Nyet\n");
        HostNyetIsr(hc);
    }
    if (hcint.b.xacterr)
    {
        //printf ("Host Xacterr\n");
        HostXacterrIsr(hc);
    }
    if ( hcint.b.bblerr)
    {
        //printf ("Host Babblerr\n");
        HostBabbleIsr(hc);
    }

    if (hcint.b.frmovrun)
    {
        /**
        * Handles a host channel frame overrun interrupt. This handler may be called
        * in either DMA mode or Slave mode.
        */
    }
    if (hcint.b.datatglerr)
    {
        //printf ("Host TglerrIsr\n");
        HostTglerrIsr(hc);
    }
}


/*******************************************************************************
** Name:   HostAllocUrb
** Input:
** Return: int32
** Owner:  wrm
** Date:   2015.10.8
** Time:   14:37:09
*******************************************************************************/
USB_REQ * HostAllocUrb(void)
{
    USB_REQ *urb = &gURBCache;

    //printf ("HostAllocUrb:urb->valid=%d\n",urb->valid);
    if (!urb->valid)
    {
        memset(urb, 0, sizeof(USB_REQ));

        urb->valid = 1;
        urb->completed = 0;
        return urb;
    }

    return NULL;
}

/*******************************************************************************
** Name:   HostFreeChn
** Input:  UHC_CHN_INFO * hc
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
void HostFreeChn(UHC_CHN_INFO * hc)
{
    if (hc->valid)
    {
        uint32 i;
        if(hc->XferStarted)
        {
            if (hc->HaltPending || hc->HaltOnQueue)
                DelayUs(1);
            else
                HostHaltChn(hc, HC_XFER_TIMEOUT);
        }

        for (i=0; i<1000; i++)
        {
            if (!hc->XferStarted)
                break;

            DelayUs(1);
        }

        hc->valid = 0;
    }
}


/*******************************************************************************
** Name:   HostStartWaitXfer
** Input:  USB_REQ *urb, int32 timeout
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
int32 HostStartWaitXfer(USB_REQ *urb, int32 timeout, UHC_CHN_INFO * pUsbHC)
{
    int32 i;
    UHC_CHN_INFO *hc = pUsbHC;

    timeout *= 1000;

    HostInitChn(hc, urb);

    HostProcessXfer(pUsbHC);
    for (i=0; i<timeout; i++)
    {
        if (pUsbHC->ConnectStatus == 0) //usbdevice disconnect
        {
            HostCompleteUrb(urb, -HOST_NO_DEV);
            break;
        }
        if (urb->completed)
            break;

        DelayUs(1);
    }
    if (i >= timeout)
    {
        HostCompleteUrb(urb, -HOST_TIMEOUT);
    }
    HostFreeChn(hc);
    if(urb->status < 0)
    {
    }
    return (urb->status<0)? urb->status : urb->ActualLen;
}


/*******************************************************************************
** Name:   HostFreeUrb
** Input:  USB_REQ *urb
** Return: int32
** Owner:  wrm
** Date:   2015.10.8
** Time:   14:37:09
*******************************************************************************/
void HostFreeUrb(USB_REQ *urb)
{
    urb->valid = 0;
}

/*******************************************************************************
** Name:   HostResetPort
** Input:  HOST_DEV *pDev
** Return: int32
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
int32 HostResetPort(HOST_DEV *pHDev)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    volatile uint32 status;
    uint32 i, j;
    int32 ret=-HOST_ERR;
    uint8 change;
    uint8 state;

    //for(i=0; i<3; i++)
    for(i=0; i<1; i++)
    {
        //reset the port
        status = otg_core->Host.hprt&(~0x0f);
        status |= (0x01<<8);
        otg_core->Host.hprt = status;

        DelayMs(50);  //hi-speed 50ms,F/L speed 10ms
        DelayMs(10);  //datasheet 建议在加10ms清除

        status = otg_core->Host.hprt&(~0x0f);
        status &= ~(0x01<<8);
        otg_core->Host.hprt = status;

        for (j=0; j<1000; j++)
        {
            if (otg_core->Host.hprt & 0x04) //wait for port enable
                break;
            DelayMs(1);
        }
        printf ("HostResetPort:j=%d\n",j);
        if (j>=50)
            return ret;

        ret = HOST_OK;
        status=(otg_core->Host.hprt >> 17) & 0x03;
        switch (status)
        {
            case 0:     //high speed
                printf ("HostResetPort: high speed\n");
                pHDev->speed = USB_SPEED_HS ;
                break;
            case 1:     //full speed
                printf ("HostResetPort: full speed\n");
                pHDev->speed = USB_SPEED_FS;
                break;
            case 2:     //low speed
                printf ("HostResetPort: low speed\n");
                pHDev->speed = USB_SPEED_LS;
                break;
            case 3:
            default:
                ret = -HOST_ERR;
                break;
        }
    }
    return ret;
}


/*******************************************************************************
** Name:   HostPortInit
** Input:  HOST_DEV *pDev, uint16 wMaxPacketSize
** Return: void
** Owner:  wrm
** Date:   2014.10.8
** Time:   14:37:09
*******************************************************************************/
void HostEp0ReInit(HOST_DEV *pHDev, uint16 wMaxPacketSize)
{
    USB_ENDPOINT_DESCRIPTOR *ep0 = &pHDev->ep0;

    ep0->bmAttributes = USB_ENDPOINT_XFER_CONTROL;
    ep0->wMaxPacketSize = wMaxPacketSize;

    pHDev->EpOut[0] = ep0;
    pHDev->EpIn[0] = ep0;
}

/*******************************************************************************
** Name: HostCreatePipe
** Input:HOST_DEV * dev, uint8 epnum, uint8 epdir
** Return: int32
** Owner:wrm
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
uint32 HostCreatePipe(HOST_DEV * pHDev, uint8 epnum, uint8 epdir)
{
    USB_ENDPOINT_DESCRIPTOR *Ep;
    uint32 pipe;
    Ep = pHDev->EpOut[epnum];
    if (epdir)
        Ep = pHDev->EpIn[epnum];
    else
        Ep = pHDev->EpOut[epnum];

    pipe = (pHDev->DevNum<<18) |(Ep->bmAttributes<<16)|(epdir << 15)
            |(epnum << 11)| Ep->wMaxPacketSize;

    return pipe;
}


/*******************************************************************************
** Name:   HostGetDesc
** Input:  HOST_DEV *pHDev, uint8 type, uint8 index, void *buf, int size
** Return: int32
** Owner:  wrm
** Date:   2015.10.8
** Time:   14:37:09
*******************************************************************************/
int32 HostGetDesc(HOST_DEV *pHDev, uint8 type, uint8 index, void *buf, int size)
{
    int32 i, ret;

    for (i = 0; i < 3; i++)
    {
        ret = HostCtrlMsg(pHDev, HostCreatePipe(pHDev, 0, 1),  USB_DIR_IN, USB_REQ_GET_DESCRIPTOR,
                    (type << 8) + index, 0, buf, size, USB_CTRL_GET_TIMEOUT);
        if (ret <= 0)
        {
            continue;
        }
        if (ret>1 && ((uint8 *)buf)[1] != type)
        {
            ret = -HOST_ERR;
            continue;
        }
        break;
    }
    return ret;
}

/*******************************************************************************
** Name:   HostParseInterface
** Input:  HOST_DEV *pHDev, uint8* buf, uint32 size
** Return: void
** Owner:  wrm
** Date:   2015.10.8
** Time:   14:37:09
*******************************************************************************/
int32 HostParseInterface(HOST_DEV *pHDev, uint8* buf, uint32 size)
{
    int32 i, numep, ret = -HOST_ERR;
    USB_ENDPOINT_DESCRIPTOR *ep;
    uint8 epnum;
    uint8 epdir; /** 0: OUT, 1: IN */
    USB_INTERFACE_DESCRIPTOR* ifdesc;
    HOST_INTERFACE *intf = &pHDev->intf;

    ifdesc = &intf->IfDesc;
    memcpy(ifdesc, buf, USB_DT_INTERFACE_SIZE);
    numep = ifdesc->bNumEndpoints;
    if (numep > MAX_HOST_EP)
    {
        numep = MAX_HOST_EP;
        ifdesc->bNumEndpoints = numep;
    }

    buf += ifdesc->bLength;
    size -= ifdesc->bLength;

    if (numep > 0)
    {
        for (i=0; i<numep && size>0; i++)
        {
            ep = (USB_ENDPOINT_DESCRIPTOR*)buf;
            if (ep->bDescriptorType != USB_DT_ENDPOINT)
                return -HOST_ERR;

            epdir = ep->bEndpointAddress >> 7;
            epnum = ep->bEndpointAddress & 0x0F;

            ep = &intf->EpDesc[i];
            memcpy(ep, buf, USB_DT_ENDPOINT_SIZE);
            (epdir)? (pHDev->EpIn[epnum]=ep) : (pHDev->EpOut[epnum]=ep);

            buf += USB_DT_ENDPOINT_SIZE;
            size -=USB_DT_ENDPOINT_SIZE;
        }
        intf->pDev = pHDev;
        ret = HOST_OK;
    }

    return ret;
}


/*******************************************************************************
** Name:   HostParseConfig
** Input:  HOST_DEV *pHDev, uint8* buf, uint32 size
** Return: void
** Owner:  wrm
** Date:   2015.10.8
** Time:   14:37:09
*******************************************************************************/
int32 HostParseConfig(HOST_DEV *pHDev, uint8* buf, uint32 size)
{
    int32 ret = -HOST_ERR;
    uint32 nintf;
    USB_DESC_HEADER *header;
    USB_CONFIGURATION_DESCRIPTOR *config = &pHDev->CfgDesc;

    memcpy(config, buf, USB_DT_CONFIG_SIZE);
    #if 1
    nintf = config->bNumInterfaces;
    if (nintf > USB_MAXINTERFACES)
    {
        nintf = USB_MAXINTERFACES;
        config->bNumInterfaces = nintf;
    }
    #endif
    buf += config->bLength;
    size -= config->bLength;

    header = (USB_DESC_HEADER *)buf;

    if (USB_DT_INTERFACE == header->bDescriptorType)
    {
        ret = HostParseInterface(pHDev, buf, size);
        if (ret < 0)
            return ret;
    }

    return ret;
}

/*******************************************************************************
** Name:   HostSetConfig
** Input:  HOST_DEV *pHDev, uint16 config
** Return: void
** Owner:  wrm
** Date:   2014.10.8
** Time:   14:37:09
*******************************************************************************/
int32 HostSetConfig(HOST_DEV *pHDev, uint16 config)
{
    int32 ret;

    ret = HostCtrlMsg(pHDev, HostCreatePipe(pHDev, 0, 0),
             0, USB_REQ_SET_CONFIGURATION, config, 0,
            NULL, 0, USB_CTRL_SET_TIMEOUT);

    if (ret < 0)
        return ret;

    pHDev->state = USB_STATE_CONFIGURED;
    return HOST_OK;
}



#endif
/*
*------------------------------------------------------------------------------------
*
*                          API function(common) define
*
*------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name:   HostConnect
** Input:  bool FullSpeed
** Return: void
** Owner:  wrm
** Date:   2015.9.29
** Time:   15:33:44
*******************************************************************************/
void HostConnect(bool FullSpeed)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG *)USB_REG_BASE;
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
    GINTMSK_DATA gintmsk;
    GAHBCFG_DATA ahbcfg;
#if 1
    memset (&gURBCache, 0 , sizeof(USB_REQ));
    memset (&gDWCHostInfo, 0 , sizeof(gDWCHostInfo));
#endif

    UsbHostIntDisable();
    dev_regs->dctl |= 0x02;                   //soft disconnec
    //printf ("\nHOST CON1 is0x%08x\n",Grf->GRF_UOC_CON1);
    //printf ("\nHOST CON2 is0x%08x\n",Grf->GRF_UOC_CON2);

    Grf->GRF_UOC_CON2 |= 0x00060006;          //DP, DM 下拉15K电阻
    Grf->GRF_UOC_CON1 = (0x00030001 << 9);    //选择为host

    //printf ("\nHOST CON1 is0x%08x\n",Grf->GRF_UOC_CON1);
    //printf ("\nHOST CON2 is0x%08x\n",Grf->GRF_UOC_CON2);

    HostResetPhy();
    HostControllerInit(FullSpeed);
    UsbHostIntEnable();
    //初始化之后进入USB Host 中断
}

/*******************************************************************************
** Name:   HostIntType
** Input:  UHC_CHN_INFO * pUsbHC
** Return: void
** Owner:  wrm
** Date:   2015.9.24
** Time:   16:21:56
*******************************************************************************/
uint32 HostIntType()
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    uint32 intType = 0;
    GINTSTS_DATA gintsts;
    GINTMSK_DATA gintmsk;
    GNPTXSTS_DATA txsts;
    HPRT0_DATA hprt;
    UHC_CHN_INFO *pUsbHC = &gDWCHostInfo;

    gintsts.d32 = otg_core->Core.gintsts;
    hprt.d32 = otg_core->Host.hprt;
    /* Check if HOST Mode */
    if (DWC_HOST_MODE == gintsts.b.curmod)
    {
        gintsts.d32 &= otg_core->Core.gintmsk;
        if (!gintsts.d32)
        {
            printf ("\nHost Mode error exit\n");
            return 0;
        }
    }
    else
    {
        return 0;
    }

    if(gintsts.b.disconnint)
    {
        printf ("\nHost disconnint\n");
        HostConnect(0);
        pUsbHC->ConnectStatus = 0;
        return HOST_DISCONNECT;
    }

    if(gintsts.b.prtint)
    {
        //printf ("Host Port Int\n");
        if (hprt.b.prtconnsts == 1)
        {
            printf ("\nHost Port connect Int\n");
            HostPortIsr();
            pUsbHC->ConnectStatus = 1;
        }
    }
    if (gintsts.b.rxflvl)
    {
        //printf ("RxFIFO Non-Empty\n");
        HostRxflvlIsr(pUsbHC);
    }

    if (gintsts.b.nptxfemp)
    {
        //printf ("Non-Periodic TxFIFO Empty\n");
        HostProcessXfer(pUsbHC);
    }

    if (gintsts.b.hchint)
    {
        //printf ("Host Channel Interrupt\n");
        int8 chn;
        uint32 haint = otg_core->Host.haint & otg_core->Host.haintmsk;

        for(chn=0; chn<MAX_HOST_CHN; chn++)
        {
            if(haint & (1<<chn))
            {
                //printf ("Judge Host Interrupt Type\n");
                HostChnIsr(chn,pUsbHC);
            }
        }
    }

    //UsbIntEnable();
    otg_core->Core.gintsts = gintsts.d32;//write clear

    return intType;
}

/*******************************************************************************
** Name:   HostBulkMsg
** Input:  HOST_DEV *pDev, uint32 pipe, void * data, int len, int timeout
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   14:28:54
*******************************************************************************/
int32 HostBulkMsg(HOST_DEV *pDev, uint32 pipe, void * data, int len, int timeout)
{
    int32 ret;
    USB_REQ * urb = NULL;
    UHC_CHN_INFO *pUsbHC = &gDWCHostInfo;
    uint8 epdir = USBPipeDir(pipe);
    uint8 epnum = USBPipeEpNum(pipe);

    urb = HostAllocUrb();

    urb->pipe.d32 = pipe;
    urb->TransBuf = data;
    urb->BufLen = len;

    urb->pDev = pDev;
    urb->DataToggle = USBGetToggle(pDev, epnum, epdir);

    ret =  HostStartWaitXfer(urb, timeout, pUsbHC);

    USBSetToggle(pDev, epnum, epdir, urb->DataToggle);

    HostFreeUrb(urb);

    return ret;
}

/*******************************************************************************
** Name:   Hostnptxfemp
** Input:  void
** Return: void
** Owner:  wrm
** Date:   2014.9.24
** Time:   16:21:56
*******************************************************************************/
void Hostnptxfemp (int32 ret)
{
    GINTMSK_DATA gintmsk;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    gintmsk.d32 = otg_core->Core.gintmsk;
    gintmsk.b.nptxfemp = (ret != 0);
    otg_core->Core.gintmsk = gintmsk.d32;
}

/*******************************************************************************
** Name:   HostCtrlMsg
** Input:
** Return: int32
** Owner:  wrm
** Date:   2015.10.8
** Time:   14:37:09
*******************************************************************************/
int32 HostCtrlMsg (HOST_DEV *pHDev, uint32 pipe, uint8 bmRequestType, uint8 bRequest, uint16  wValue,
                                 uint16 wIndex, void *data, uint32 size, int timeout)
{
    int32 ret;
    USB_CTRL_REQUEST CtrlReq;
    UHC_CHN_INFO *pUsbHC = &gDWCHostInfo;
    USB_REQ * urb = NULL;

    CtrlReq.bRequestType = bmRequestType;
    CtrlReq.bRequest = bRequest;
    CtrlReq.wValue = wValue;
    CtrlReq.wIndex = wIndex;
    CtrlReq.wLength = size;

    urb = HostAllocUrb();
    urb->pipe.d32 = pipe;
    urb->SetupPkt = &CtrlReq;
    urb->TransBuf = data;
    urb->BufLen = size;

    urb->DataToggle = 1;
    urb->CtrlPhase = DWC_OTG_CONTROL_SETUP;
    urb->pDev = pHDev;

    ret =  HostStartWaitXfer(urb, timeout,pUsbHC);

    HostFreeUrb(urb);

    return ret;
}


/*******************************************************************************
** Name:   HostMscClearFeature
** Input:  HOST_DEV *pDev, uint32 pipe, uint8 type, uint16 wValue, uint16 wIndex
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 HostMscClearFeature(HOST_DEV *pDev, uint32 pipe, uint8 type, uint16 wValue, uint16 wIndex)
{
    int32 ret;
    ret = HostCtrlMsg(pDev, pipe, type, USB_REQ_CLEAR_FEATURE,
                      wValue, wIndex, NULL, 0, USB_CTRL_SET_TIMEOUT);
    if (ret < 0)
        return ret;

    // toggle is reset on clear
    USBSetToggle(pDev, USBPipeEpNum(pipe), USBPipeDir(pipe), 0);

    return ret;
}

/*******************************************************************************
** Name:   HostSetAddr
** Input:  HOST_DEV *pHDev
** Return: void
** Owner:  wrm
** Date:   2015.10.8
** Time:   14:37:09
*******************************************************************************/
int32 HostSetAddr(HOST_DEV *pHDev)
{
    int32 ret, i;
    uint8 devnum = 1;

    for(i=0; i<2; i++)
    {
        ret = HostCtrlMsg(pHDev, HostCreatePipe(pHDev, 0, 0),
                 0, USB_REQ_SET_ADDRESS, devnum, 0,
                NULL, 0, USB_CTRL_SET_TIMEOUT);

        if (ret >= 0)
        {
            pHDev->DevNum = devnum;
            pHDev->state = USB_STATE_ADDRESS;
            break;
        }
        DelayMs(200);
    }

    return ret;
}



/*******************************************************************************
** Name:   HostPortInit
** Input:  HOST_DEV *pHDev
** Return: void
** Owner:  wrm
** Date:   2015.10.8
** Time:   14:37:09
*******************************************************************************/
int32 HostPortInit(HOST_DEV *pHDev)
{
    int32 ret;
    uint16    wMaxPacketSize;
    USB_DEVICE_DESCRIPTOR buf[64/4];
    USB_DEVICE_DESCRIPTOR *pDesc;

    ret = HostResetPort(pHDev);
    if (ret < 0)
        return ret;

    printf ("HostPortInit:speed=%d\n",pHDev->speed);
    switch (pHDev->speed)
    {
        case USB_SPEED_HS:     //high speed
            wMaxPacketSize = 64;
            break;
        case USB_SPEED_FS:     //full speed
            wMaxPacketSize = 64;
            break;
        case USB_SPEED_LS:     //low speed
            wMaxPacketSize = 8;
            break;

        default:
            break;
    }

    HostEp0ReInit(pHDev, wMaxPacketSize);

    DelayMs(10);
    ret = HostGetDesc(pHDev, USB_DT_DEVICE, 0, buf, 64);
    if (ret < 0)
        return ret;

    pDesc = (USB_DEVICE_DESCRIPTOR *)buf;
    if (wMaxPacketSize != pDesc->bMaxPacketSize0)
    {
        HostEp0ReInit(pHDev, pDesc->bMaxPacketSize0);
    }

    #if 0
    ret = UHCResetPort(pDev); // windows 和linux 会进行二次复位, 协议无此要求
    if (ret < 0)
        return ret;
    #endif

    ret = HostSetAddr(pHDev);
    if (ret < 0)
        return ret;

    DelayMs(5);

    ret = HostGetDesc(pHDev, USB_DT_DEVICE, 0, buf, USB_DT_DEVICE_SIZE);
    if (ret < 0)
        return ret;

    memcpy(&pHDev->DevDesc, buf, USB_DT_DEVICE_SIZE);

    return ret;
}

/*******************************************************************************
** Name:   HostNewDev
** Input:  HOST_DEV *pHDev
** Return: void
** Owner:  wrm
** Date:   2015.10.8
** Time:   14:37:09
*******************************************************************************/
int32 HostNewDev(HOST_DEV *pHDev)
{
    int32 ret = -HOST_ERR;
    uint16 length;
    uint32 buf[256/4];
    USB_CONFIGURATION_DESCRIPTOR *config;
    uint32 ncfg = pHDev->DevDesc.bNumConfigurations;

    if (ncfg > USB_MAXCONFIG)
    {
        ncfg = USB_MAXCONFIG;
    }

    if (ncfg < 1)
        return -HOST_ERR;

    config = (USB_CONFIGURATION_DESCRIPTOR *)buf;

    ret = HostGetDesc(pHDev, USB_DT_CONFIG, 0, buf, USB_DT_CONFIG_SIZE);
    if (ret < 0)
        goto FAIL;
    length = config->wTotalLength;
    if (length > 256)
    {
        ret = -HOST_ERR;
        goto FAIL;
    }

    ret = HostGetDesc(pHDev, USB_DT_CONFIG, 0, buf, length);
    if (ret < 0)
        goto FAIL;

    ret = HostParseConfig(pHDev, (uint8*)buf, length);
    if (ret < 0)
        goto FAIL;

    ret = HostSetConfig(pHDev, 1);
    if (ret < 0)
        goto FAIL;

FAIL:

    return ret;
}

/*******************************************************************************
** Name:   HostRegDeinit
** Input:  void
** Return: void
** Owner:  wrm
** Date:   2015.9.29
** Time:   15:33:44
*******************************************************************************/
void HostRegDeinit(void)
{
    UsbHostIntDisable();
    //printf ("\nHOST CON1 is0x%08x\n",Grf->GRF_UOC_CON1);
    //printf ("\nHOST CON2 is0x%08x\n",Grf->GRF_UOC_CON2);
    Grf->GRF_UOC_CON1 = 0x02000000;    //选择为Dev
    Grf->GRF_UOC_CON2 = 0x00060000;    //DP, DM 下拉15K电阻使不能
    //printf ("\nHOST CON1 is0x%08x\n",Grf->GRF_UOC_CON1);
    //printf ("\nHOST CON2 is0x%08x\n",Grf->GRF_UOC_CON2);
    USBResetPhy();
    UsbHostIntEnable();
    return ;
}

/*******************************************************************************
** Name:   HostGetChannelInfo
** Input:  void
** Return: UHC_CHN_INFO*
** Owner:  wrm
** Date:   2015.9.29
** Time:   15:33:44
*******************************************************************************/
UHC_CHN_INFO* HostGetChannelInfo(void)
{
    UHC_CHN_INFO *pUsbHCI = &gDWCHostInfo;
    return pUsbHCI;
}


/*******************************************************************************
** Name:   UsbHostIntIsr
** Input:  void
** Return: void
** Owner:  wrm
** Date:   2015.9.29
** Time:   15:33:44
*******************************************************************************/
_ATTR_SYS_CODE_
void UsbHostIntIsr(void)
{
    HostIntType();
}

/*******************************************************************************
** Name:   USBHostSetParams
** Input:  uint8 type, uint8 value
** Return: uint8
** Owner:  wrm
** Date:   2015.9.29
** Time:   15:33:44
*******************************************************************************/
_ATTR_SYS_CODE_
uint8 USBHostSetParams(uint8 type, uint8 value)
{
    switch (type)
    {
        case HOST_FORBID_INIT:
            gUSBHost_forbidInit = value;
            break;
        default:
            break;
    }

    return value;
}


/*******************************************************************************
** Name:   USBHostStart
** Input:  void
** Return: uint32
** Owner:  wrm
** Date:   2015.9.29
** Time:   15:33:44
*******************************************************************************/
_ATTR_SYS_CODE_
uint32 USBHostStart(void)
{
    if ((gUSBHost_InitState == 0)&&(gUSBHost_forbidInit != 1))
    {
        DEBUG ("USBHostStart");
        FREQ_EnterModule(FREQ_USB);
        gUSBHost_InitState = 1;
        ScuClockGateCtr(CLK_USBPHY_GATE, 1);
        ScuClockGateCtr(HCLK_USBC_GATE, 1);
        ScuSoftResetCtr(USBPHY_SRST, 0);
        ScuSoftResetCtr(USBGLB_SRST, 0);
        ScuSoftResetCtr(USBOTG_SRST, 0);
        DelayUs(5);
        Grf_otgphy_suspend(0);
        DelayUs(200);

        //interrupt init
        IntRegister(INT_ID_USBC, (void*)UsbHostIntIsr);
        Grf_Otgphy_Driver_Confige();

        HostConnect(0);
        gUSBHost_InitState = 1;
    }
}

/*******************************************************************************
** Name:   USBHostReInit
** Input:  void
** Return: uint32
** Owner:  wrm
** Date:   2015.9.29
** Time:   15:33:44
*******************************************************************************/
_ATTR_SYS_CODE_
uint32 USBHostReInit(void)
{
    if (gUSBHost_InitState == 1)
    {
        DEBUG ("USBHostReInit");
        UsbHostIntDisable();
        Grf_otgphy_suspend(1);

        ScuSoftResetCtr(USBPHY_SRST, 1);
        ScuSoftResetCtr(USBGLB_SRST, 1);
        ScuSoftResetCtr(USBOTG_SRST, 1);
        ScuClockGateCtr(CLK_USBPHY_GATE, 0);
        ScuClockGateCtr(HCLK_USBC_GATE, 0);

        IntUnregister(INT_ID_USBC);

        HostRegDeinit();
        gUSBHost_InitState = 0;
        FREQ_ExitModule(FREQ_USB);
    }
}

#endif

