#ifdef _USB_HOST_
#ifndef USB_DWCHOST_H
#define USB_DWCHOST_H
#include "driver/USB/chap9.h"
#include "driver/USB/USBComm.h"   /* USB_CTRL_REQ type */   /* USB_*_DESCRIPTOR types */

#define     HOST_OK                     0
#define     HOST_ERR                    1
#define     HOST_STALL                  2
#define     HOST_NAK                    3
#define     HOST_NYET                   4
#define     HOST_XERR                   5
#define     HOST_TGLERR                 6
#define     HOST_BABBLE                 7
#define     HOST_NO_QUEUE               8
#define     HOST_NO_SPACE               9
#define     HOST_NO_DATA                10
#define     HOST_TIMEOUT                11
#define     HOST_NO_READY               12
#define     HOST_NO_DEV                 13

#define     USB_SPEED_LS                0x00
#define     USB_SPEED_FS                0x01
#define     USB_SPEED_HS                0x02


#define    USB_CNTL_TIMEOUT            100    /* 100ms timeout */
#define    USB_CTRL_GET_TIMEOUT        5000
#define    USB_CTRL_SET_TIMEOUT        5000

#define    USB_MAXCONFIG               1
#define    USB_MAXINTERFACES           1

#define    MAX_USB_DEV                 1
#define    MAX_USB_DRV                 1
#define    MAX_HOST_EP                 4
#define    MAX_USB_REQ                 1

#define    USBPipeEpNum(pipe)          ((pipe>>11) & 0xf)
#define    USBPipeDir(pipe)            ((pipe>>15) & 0x1)
#define    USBIsCtrlPipe(pipe)         (!((pipe>>16) & 0x1))
#define    USBPipeType(pipe)           ((pipe>>16) & 0x3)
#define    USBPipeDev(pipe)            ((pipe>>18) & 0x7f)


#define    USBEpNum(ep)    (ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)
#define    USBEpType(ep)   (ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
#define    USBEpDirIn(ep)  ((ep->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)

#define    HostCleanEpStall(pDev, pipe, wIndex)    \
           HostMscClearFeature(pDev, pipe, USB_RECIP_ENDPOINT, USB_ENDPOINT_HALT, wIndex)

#define    USBGetToggle(dev, ep, out)   (((dev)->toggle[out] >> (ep)) & 1)
#define    USBDoToggle (dev, ep, out)   ((dev)->toggle[out] ^= (1 << (ep)))

#define    USBSetToggle(dev, ep, out, bit) \
           ((dev)->toggle[out] = ((dev)->toggle[out] & ~(1 << (ep))) | \
           ((bit) << (ep)))

#define     MAX_HOST_CHN                       1
#define     HC_CHN_FIFO(chn)                   (USB_REG_BASE +((chn+1)<<12))

#define     DWC_OTG_HC_PID_DATA0               0
#define     DWC_OTG_HC_PID_DATA2               1
#define     DWC_OTG_HC_PID_DATA1               2
#define     DWC_OTG_HC_PID_MDATA               3
#define     DWC_OTG_HC_PID_SETUP               3

#define     DWC_OTG_EP_TYPE_CONTROL            0
#define     DWC_OTG_EP_TYPE_ISOC               1
#define     DWC_OTG_EP_TYPE_BULK               2
#define     DWC_OTG_EP_TYPE_INTR               3

#define     DWC_OTG_EP_SPEED_LOW               0
#define     DWC_OTG_EP_SPEED_FULL              1
#define     DWC_OTG_EP_SPEED_HIGH              2

#define     HOST_DISCONNECT                    1
#define     HOST_PORT_INT                      2

#define     HOST_FORBID_INIT                   1



typedef union tagHOST_PIPE
{
    /** raw register data */
    uint32 d32;

        /** register bits */
    struct
    {
        /** Maximum packet size in bytes */
        unsigned mps : 11;

        /** Endpoint number */
        unsigned epnum : 4;

        /** 0: OUT, 1: IN */
        unsigned epdir : 1;

        /** 0: Control, 1: Isoc, 2: Bulk, 3: Intr */
        unsigned eptype : 2;

        /** Device address */
        unsigned devaddr : 7;

        //unsigned pid : 2;
        unsigned reserved25_31 : 7;

    } b;
} HOST_PIPE;


typedef    struct tagHOST_INTERFACE
{
    USB_INTERFACE_DESCRIPTOR        IfDesc;
    USB_ENDPOINT_DESCRIPTOR         EpDesc[4];
    struct tagHOST_DEV  *pDev;
}HOST_INTERFACE, *pHOST_INTERFACE;


typedef    struct tagHOST_DEV
{
    uint8               valid;
    uint8               DevNum;
    uint8               speed;
    uint8               state;
    uint16              toggle[2];  /*one bit for each endpoint   [0] = OUT, [1] = IN*/
    USB_ENDPOINT_DESCRIPTOR        ep0;
    USB_DEVICE_DESCRIPTOR          DevDesc;
    USB_CONFIGURATION_DESCRIPTOR   CfgDesc;
    HOST_INTERFACE       intf;

    USB_ENDPOINT_DESCRIPTOR        *EpIn[16];
    USB_ENDPOINT_DESCRIPTOR        *EpOut[16];

}HOST_DEV, *pHOST_DEV;

typedef    struct tagUSB_REQ
{
    uint8               valid;
    HOST_DEV            *pDev;
    HOST_PIPE           pipe;
    USB_CTRL_REQ    *SetupPkt;
    uint8               CtrlPhase;
    void                *TransBuf;
    uint32              BufLen;
    uint32              ActualLen;
    uint32              StatusBuf[1];
    uint8               DataToggle;
    /** Ping state if 1. */
    uint8               PingState;
    uint8                 ErrCnt;
    int32               status;
    uint8               completed;
}USB_REQ, *pUSB_REQ;


typedef    volatile struct tagUHC_CHN_INFO
{
    uint8           chn;
    uint8           valid;

    /** Device to access            */
    uint8           DevAddr;
    /** EP to access                */
    uint8           EpNum;
    /** EP direction. 0: OUT, 1: IN */
    uint8           EpIsIn;
    /**
     * EP speed.
     * One of the following values:
     *    - DWC_OTG_EP_SPEED_LOW
     *    - DWC_OTG_EP_SPEED_FULL
     *    - DWC_OTG_EP_SPEED_HIGH
     */
    uint8           speed;
    /**
     * Endpoint type.
     * One of the following values:
     *    - DWC_OTG_EP_TYPE_CONTROL: 0
     *    - DWC_OTG_EP_TYPE_ISOC: 1
     *    - DWC_OTG_EP_TYPE_BULK: 2
     *    - DWC_OTG_EP_TYPE_INTR: 3
     */
    uint8           EpType;
    /** Max packet size in bytes */
    uint16          MaxPkt;
    /**
     * PID for initial transaction.
     * 0: DATA0,<br>
     * 1: DATA2,<br>
     * 2: DATA1,<br>
     * 3: MDATA (non-Control EP),
     *      SETUP (Control EP)
     */
    uint8           PidStart;

    uint8           *XferBuf;              //中断数据接收或发送数据用到的buf地址
    uint32          XferLen;               //需要传输的数据长度
    uint32          XferCnt;               //已经发送或接收的数据长度
    /** Packet count at start of transfer.*/
    uint16          StartPktCnt;
    /**
    * Flag to indicate whether the transfer has been started. Set to 1 if
    * it has been started, 0 otherwise.
    */
    uint8           XferStarted;
    /**
     * Set to 1 to indicate that a PING request should be issued on this
     * channel. If 0, process normally.
     */
    uint8           DoPing;
    /**
     * Set to 1 to indicate that the error count for this transaction is
     * non-zero. Set to 0 if the error count is 0.
     */
    uint8           ErrorState;

    uint32          status;
    /**
    * Set to 1 if the host channel has been halted, but the core is not
    * finished flushing queued requests. Otherwise 0.
    */
    uint8           HaltPending;
    uint8           HaltOnQueue;

    USB_REQ         *urb;

    /**wrm add**/
    uint8           DisStatus;
    uint8           ConnectStatus;

}UHC_CHN_INFO, *pUHC_CHN_INFO;

/* Macro used to clear one channel interrupt */
#define CLEAN_HC_INT(_hc_regs_, _intr_) \
do { \
    HCINT_DATA hcint_clear; \
    hcint_clear.d32 = 0;    \
    hcint_clear.b._intr_ = 1; \
    _hc_regs_->hcintn = hcint_clear.d32; \
} while (0)

#define DISABLE_HC_INT(_hc_regs_, _intr_) \
do { \
    HCINTMSK_DATA hcintmsk; \
    hcintmsk.d32 = _hc_regs_->hcintmaskn;  \
    hcintmsk.b._intr_ = 0; \
    _hc_regs_->hcintmaskn =  hcintmsk.d32; \
} while (0)


typedef enum HC_XFER_STATUS
{
    HC_XFER_IDLE,
    HC_XFER_COMPL,
    HC_XFER_URB_COMPL,
    HC_XFER_ACK,
    HC_XFER_NAK,
    HC_XFER_NYET,
    HC_XFER_XERR,
    HC_XFER_STALL,
    HC_XFER_BABBLE,
    HC_XFER_TIMEOUT,
} HC_XFER_STATUS_e;

typedef enum DWC_OTG_CTRL_PHASE {
    DWC_OTG_CONTROL_SETUP,
    DWC_OTG_CONTROL_DATA,
    DWC_OTG_CONTROL_STATUS
} DWC_OTG_CTRL_PHASE_e;

extern void   HostConnect(bool FullSpeed);
extern uint32 HostIntType();
extern int32  HostBulkMsg(HOST_DEV *pDev, uint32 pipe, void * data, int len, int timeout);
extern void   Hostnptxfemp (int32 ret);
extern int32  HostCtrlMsg ( HOST_DEV *pHDev,
                              uint32 pipe,
                              uint8 bmRequestType,
                              uint8 bRequest,
                              uint16 wValue,
                              uint16 wIndex,
                              void *data,
                              uint32 size,
                              int timeout);
extern int32 HostMscClearFeature(HOST_DEV *pDev, uint32 pipe, uint8 type, uint16 wValue, uint16 wIndex);
extern int32 HostSetAddr(HOST_DEV *pHDev);
extern int32 HostPortInit(HOST_DEV *pHDev);
extern int32 HostNewDev(HOST_DEV *pHDev);
extern void  HostRegDeinit(void);

extern uint32 USBHostStart(void);
extern uint32 USBHostReInit(void);
extern uint8  USBHostSetParams(uint8 type, uint8 value);

#endif

#endif
