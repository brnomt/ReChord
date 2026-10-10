/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: OTGREG.h
Desc: 

Author: chenfen@rock-chips.com
Date: 12-06-20
Notes:

$Log: $
 *
 *
*/

#ifndef _USBREG_H
#define _USBREG_H

/*-------------------------------- Includes ----------------------------------*/

/*------------------------------ Global Defines ------------------------------*/
#define       OTG_REG_BASE              0x60080000
#define       USB_REG_BASE              OTG_REG_BASE
#define       USB_DEV_BASE              (USB_REG_BASE+0x800)

#define       USB_EP_FIFO(ep)           (USB_REG_BASE +((ep+1)<<12))

/*------------------------------ Global Typedefs -----------------------------*/
//Global Registers
typedef volatile struct tagCORE_REG 
{
    uint32 gotgctl; 
    uint32 gotgint;
    uint32 gahbcfg;
    uint32 gusbcfg; 
    uint32 grstctl; 
    uint32 gintsts; 
    uint32 gintmsk; 
    uint32 grxstsr; 
    uint32 grxstsp; 
    uint32 grxfsiz; 
    uint32 gnptxfsiz; 
    uint32 gnptxsts; 
    uint32 gi2cctl; 
    uint32 gpvndctl;
    uint32 ggpio; 
    uint32 guid; 
    uint32 gsnpsid;
    uint32 ghwcfg1; 
    uint32 ghwcfg2;
    uint32 ghwcfg3;
    uint32 ghwcfg4;
    uint32 RESERVED1[(0x100-0x54)/4];
    uint32 hptxfsiz;
    uint32 dptxfsiz_dieptxf[15]; 
    uint32 RESERVED2[(0x400-0x140)/4];
} CORE_REG, *pCORE_REG;

typedef  struct _HOST_CHANNEL_REG
{
    uint32 hccharn;
    uint32 hcspltn;
    uint32 hcintn;
    uint32 hcintmaskn;
    uint32 hctsizn;
    uint32 hcdman;
    uint32 reserved[2];
}HOST_CHANNEL_REG, *pHCHN_REG;

//Host Mode Register Structures
typedef volatile struct tagHOST_REG 
{
    uint32 hcfg;		  
    uint32 hfir;		  
    uint32 hfnum; 
    uint32 reserved40C;
    uint32 hptxsts;	  
    uint32 haint;	  
    uint32 haintmsk;
    uint32 reserved1[(0x440-0x41c)/4];
    uint32 hprt;
    uint32 reserved2[(0x500-0x444)/4];
    HOST_CHANNEL_REG hchn[4];
    uint32 reserved3[(0x800-0x580)/4];
} HOST_REG, *pHOST_REG;

//Device IN ep reg
typedef volatile struct tagIN_EP_REG 
{
    /** Device IN Endpoint Control Register. <i>Offset:900h +
     * (ep_num * 20h) + 00h</i> */
    uint32 diepctl;
    /** Reserved. <i>Offset:900h + (ep_num * 20h) + 04h</i> */
    uint32 reserved04;  
    /** Device IN Endpoint Interrupt Register. <i>Offset:900h +
    * (ep_num * 20h) + 08h</i> */
    uint32 diepint; 
    /** Reserved. <i>Offset:900h + (ep_num * 20h) + 0Ch</i> */
    uint32 reserved0C;  
    /** Device IN Endpoint Transfer Size
    * Register. <i>Offset:900h + (ep_num * 20h) + 10h</i> */
    uint32 dieptsiz; 
    /** Device IN Endpoint DMA Address Register. <i>Offset:900h +
    * (ep_num * 20h) + 14h</i> */
    uint32 diepdma; 
    /** Device IN Endpoint Transmit FIFO Status Register. <i>Offset:900h +
    * (ep_num * 20h) + 18h</i> */
    uint32 dtxfsts;
    /** Reserved. <i>Offset:900h + (ep_num * 20h) + 1Ch - 900h +
    * (ep_num * 20h) + 1Ch</i>*/
    uint32 reserved18;  
}IN_EP_REG, *pIN_EP_REG;

typedef volatile struct tagOUT_EP_REG 
{
    /** Device OUT Endpoint Control Register. <i>Offset:B00h +
     * (ep_num * 20h) + 00h</i> */
    uint32 doepctl; 
    /** Device OUT Endpoint Frame number Register.  <i>Offset:
     * B00h + (ep_num * 20h) + 04h</i> */ 
    uint32 doepfn; 
    /** Device OUT Endpoint Interrupt Register. <i>Offset:B00h +
     * (ep_num * 20h) + 08h</i> */
    uint32 doepint; 
    /** Reserved. <i>Offset:B00h + (ep_num * 20h) + 0Ch</i> */
    uint32 reserved0C;  
    /** Device OUT Endpoint Transfer Size Register. <i>Offset:
     * B00h + (ep_num * 20h) + 10h</i> */
    uint32 doeptsiz; 
    /** Device OUT Endpoint DMA Address Register. <i>Offset:B00h
     * + (ep_num * 20h) + 14h</i> */
    uint32 doepdma; 
    /** Reserved. <i>Offset:B00h + (ep_num * 20h) + 18h - B00h +
     * (ep_num * 20h) + 1Ch</i> */
    uint32 unused[2];
}OUT_EP_REG, *pOUT_EP_REG;

/**
 * This union represents the bit fields in the Device EP Control
 * Register.  Read the register into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.
 */
typedef union tagDEPCTL_DATA
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct
	{
		/** Maximum Packet Size
		 * IN/OUT EPn
		 * IN/OUT EP0 - 2 bits
		 *	 2'b00: 64 Bytes
		 *	 2'b01: 32
		 *	 2'b10: 16
		 *	 2'b11: 8 */
		unsigned mps : 11;
#define DWC_DEP0CTL_MPS_64	 0
#define DWC_DEP0CTL_MPS_32	 1
#define DWC_DEP0CTL_MPS_16	 2
#define DWC_DEP0CTL_MPS_8	 3

		/** Next Endpoint
		 * IN EPn/IN EP0
		 * OUT EPn/OUT EP0 - reserved */
		unsigned nextep : 4;

		/** USB Active Endpoint */
		unsigned usbactep : 1;

		/** Endpoint DPID (INTR/Bulk IN and OUT endpoints)
		 * This field contains the PID of the packet going to
		 * be received or transmitted on this endpoint. The
		 * application should program the PID of the first
		 * packet going to be received or transmitted on this
		 * endpoint , after the endpoint is
		 * activated. Application use the SetD1PID and
		 * SetD0PID fields of this register to program either
		 * D0 or D1 PID.
		 *
		 * The encoding for this field is
		 *	 - 0: D0
		 *	 - 1: D1
		 */
		unsigned dpid : 1;

		/** NAK Status */
		unsigned naksts : 1;

		/** Endpoint Type
		 *	2'b00: Control
		 *	2'b01: Isochronous
		 *	2'b10: Bulk
		 *	2'b11: Interrupt */
		unsigned eptype : 2;

		/** Snoop Mode
		 * OUT EPn/OUT EP0
		 * IN EPn/IN EP0 - reserved */
		unsigned snp : 1;

		/** Stall Handshake */
		unsigned stall : 1;

		/** Tx Fifo Number
		 * IN EPn/IN EP0
		 * OUT EPn/OUT EP0 - reserved */
		unsigned txfnum : 4;

		/** Clear NAK */
		unsigned cnak : 1;
		/** Set NAK */
		unsigned snak : 1;
		/** Set DATA0 PID (INTR/Bulk IN and OUT endpoints)
		 * Writing to this field sets the Endpoint DPID (DPID)
		 * field in this register to DATA0. Set Even
		 * (micro)frame (SetEvenFr) (ISO IN and OUT Endpoints)
		 * Writing to this field sets the Even/Odd
		 * (micro)frame (EO_FrNum) field to even (micro)
		 * frame.
		 */
		unsigned setd0pid : 1;
		/** Set DATA1 PID (INTR/Bulk IN and OUT endpoints)
		 * Writing to this field sets the Endpoint DPID (DPID)
		 * field in this register to DATA1 Set Odd
		 * (micro)frame (SetOddFr) (ISO IN and OUT Endpoints)
		 * Writing to this field sets the Even/Odd
		 * (micro)frame (EO_FrNum) field to odd (micro) frame.
		 */
		unsigned setd1pid : 1;

		/** Endpoint Disable */
		unsigned epdis : 1;
		/** Endpoint Enable */
		unsigned epena : 1;
		} b;
}DEPCTL_DATA;

/**
 * This union represents the bit fields in the Device EP Transfer
 * Size Register.  Read the register into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.
 */
typedef union  tagDEPTSIZ_DATA
{
		/** raw register data */
		uint32 d32;
		/** register bits */
		struct 
		{
		/** Transfer size */
		unsigned xfersize : 19;
		/** Packet Count */
		unsigned pktcnt : 10;
		/** Multi Count - Periodic IN endpoints */
		unsigned mc : 2;
		unsigned reserved : 1;
		} b;
}   DEPTSIZ_DATA;

/**
 * This union represents the bit fields in the Device EP 0 Transfer
 * Size Register.  Read the register into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.
 */
typedef union tagDEPTSIZ0_DATA
{
		/** raw register data */
		uint32 d32;
		/** register bits */
		struct 
		{
		/** Transfer size */
		unsigned xfersize : 7;
				/** Reserved */
				unsigned reserved7_18 : 12;
		/** Packet Count */
		unsigned pktcnt : 1;
				/** Reserved */
		unsigned reserved20_28 : 9;
		/**Setup Packet Count (DOEPTSIZ0 Only) */
		unsigned supcnt : 2;
		unsigned reserved31;
		} b;
} DEPTSIZ0_DATA;


//Device Mode Registers Structures
typedef volatile struct tagDEVICE_REG 
{
    /** Device Configuration Register. <i>Offset 800h</i> */
    uint32 dcfg; 
    /** Device Control Register. <i>Offset: 804h</i> */
    uint32 dctl; 
    /** Device Status Register (Read Only). <i>Offset: 808h</i> */
    uint32 dsts; 
    /** Reserved. <i>Offset: 80Ch</i> */
    uint32 unused;    
    /** Device IN Endpoint Common Interrupt Mask
    * Register. <i>Offset: 810h</i> */
    uint32 diepmsk; 
    /** Device OUT Endpoint Common Interrupt Mask
    * Register. <i>Offset: 814h</i> */
    uint32 doepmsk;  
    /** Device All Endpoints Interrupt Register.  <i>Offset: 818h</i> */
    uint32 daint;  
    /** Device All Endpoints Interrupt Mask Register.  <i>Offset:
    * 81Ch</i> */
    uint32 daintmsk; 
    /** Device IN Token Queue Read Register-1 (Read Only).
    * <i>Offset: 820h</i> */
    uint32 dtknqr1;  
    /** Device IN Token Queue Read Register-2 (Read Only).
    * <i>Offset: 824h</i> */ 
    uint32 dtknqr2;  
    /** Device VBUS  discharge Register.  <i>Offset: 828h</i> */
    uint32 dvbusdis;   
    /** Device VBUS Pulse Register.  <i>Offset: 82Ch</i> */
    uint32 dvbuspulse;
    /** Device IN Token Queue Read Register-3 (Read Only). /
    *  Device Thresholding control register (Read/Write)
    * <i>Offset: 830h</i> */ 
    uint32 dtknqr3_dthrctl;  
    /** Device IN Token Queue Read Register-4 (Read Only). /
    *  Device IN EPs empty Inr. Mask Register (Read/Write)
    * <i>Offset: 834h</i> */ 
    uint32 dtknqr4_fifoemptymsk;

    uint32 reserved[(0x900-0x838)/4];

    /* Device Logical IN Endpoint-Specific Registers 900h-AFCh */
    IN_EP_REG in_ep[16];

    /** Device Logical OUT Endpoint-Specific Registers B00h-CFCh */
    OUT_EP_REG out_ep[16];

    uint32 reserved2[(0xe00-0xd00)/4];    
} DEVICE_REG, *pDEVICE_REG; 

typedef volatile struct tagPOWER_CLOCK_CTRL 
{
    uint32 pcgcctl;
    uint32 reserved[(0x1000-0xe04)/4];
}POWER_CLOCK_CTRL, *pPOWER_CLOCK_CTRL;

typedef volatile struct tagFIFO
{
    uint32 DataPort;
    uint32 reserved[(0x1000-0x004)/4];
}FIFO, *pFIFO;

typedef volatile struct tagUSB_OTG_REG 
{
    CORE_REG Core; 
    HOST_REG Host; 
    DEVICE_REG Device;
    POWER_CLOCK_CTRL ClkGate;
    FIFO Fifo[16];
    uint32 reserved[(0x40000-0x11000)/4];
}USB_OTG_REG, *pUSB_OTG_REG;

/**
 * This union represents the bit fields of the Core OTG Control
 * and status Register (GOTGCTL).  Set the bits using the bit
 * fields then write the <i>d32</i> value to the register.
 */
typedef union tagGOTGCTL_DATA                  //*
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct {
		unsigned sesreqscs : 1;
		unsigned sesreq : 1;
		unsigned reserved2_7 : 6;
		unsigned hstnegscs : 1;
		unsigned hnpreq : 1;
		unsigned hstsethnpen : 1;
		unsigned devhnpen : 1;
		unsigned reserved12_15 : 4;
		unsigned conidsts : 1;
		unsigned dbnctime : 1;
		unsigned asesvld : 1;
		unsigned bsesvld : 1;
		unsigned currmod : 1;
		unsigned reserved21_31 : 11;
	} b;
} GOTGCTL_DATA;

/**
 * This union represents the bit fields of the Core OTG Interrupt Register
 * (GOTGINT).  Set/clear the bits using the bit fields then write the <i>d32</i>
 * value to the register.
 */
typedef union tagGOTGINT_DATA                  //*
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct
	{
		unsigned reserved0_1 : 2;

		/** Session End Detected */
		unsigned sesenddet : 1;

		unsigned reserved3_7 : 5;

		/** Session Request Success status Change */
		unsigned sesreqsucstschng : 1;
		/** Host Negotiation Success status Change */
		unsigned hstnegsucstschng : 1;

		unsigned reserver10_16 : 7;

		/** Host Negotiation Detected */
		unsigned hstnegdet : 1;
		/** A-Device Timeout Change */
		unsigned adevtoutchng : 1;
		/** Debounce Done */
		unsigned dbncedone : 1;

		unsigned reserved31_20 : 12;

	} b;
} GOTGINT_DATA;


/**
 * This union represents the bit fields of the Core AHB Configuration
 * Register (GAHBCFG).  Set/clear the bits using the bit fields then
 * write the <i>d32</i> value to the register.
 */
typedef union tagGAHBCFG_DATA                  //*
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct
	{
		unsigned glblintrmsk : 1;
#define DWC_GAHBCFG_GLBINT_ENABLE		1

		unsigned hbstlen : 4;
#define DWC_GAHBCFG_INT_DMA_BURST_SINGLE	0
#define DWC_GAHBCFG_INT_DMA_BURST_INCR		1
#define DWC_GAHBCFG_INT_DMA_BURST_INCR4		3
#define DWC_GAHBCFG_INT_DMA_BURST_INCR8		5
#define DWC_GAHBCFG_INT_DMA_BURST_INCR16	7

		unsigned dmaen : 1;
#define DWC_GAHBCFG_DMAENABLE			1
		unsigned reserved : 1;
		unsigned nptxfemplvl : 1;
		unsigned ptxfemplvl : 1;
#define DWC_GAHBCFG_TXFEMPTYLVL_EMPTY		1
#define DWC_GAHBCFG_TXFEMPTYLVL_HALFEMPTY	0
		unsigned reserved9_31 : 23;
	} b;
} GAHBCFG_DATA;

/**
 * This union represents the bit fields of the Core USB Configuration
 * Register (GUSBCFG).  Set the bits using the bit fields then write
 * the <i>d32</i> value to the register.
 */
typedef union tagGUSBCFG_DATA                  //*
{
        /** raw register data */
        uint32 d32;
        /** register bits */
        struct
        {
		unsigned toutcal : 3;
		unsigned phyif : 1;
		unsigned ulpiutmisel : 1;
		unsigned fsintf : 1;
		unsigned physel : 1;
		unsigned ddrsel : 1;
		unsigned srpcap : 1;
		unsigned hnpcap : 1;
		unsigned usbtrdtim : 4;
		unsigned reserved14 : 1;
		unsigned phylpwrclksel : 1;
		unsigned otgi2csel : 1;
		unsigned ulpifsls : 1;
		unsigned ulpiautores : 1;
		unsigned ulpiclksusm : 1;
		unsigned ulpiextvbusdrv : 1;
		unsigned ulpiextvbusindicator : 1;
		unsigned termseldlpulse : 1;
		unsigned reserved23_28 : 6;
		unsigned forcehstmode: 1;
		unsigned forcedevmode: 1;
		unsigned cortxpkt: 1;
        } b;
} GUSBCFG_DATA;

/**
 * This union represents the bit fields of the Core Reset Register
 * (GRSTCTL).  Set/clear the bits using the bit fields then write the
 * <i>d32</i> value to the register.
 */
typedef union tagGRSTCTL_DATA                  //*
{
        /** raw register data */
        uint32 d32;
        /** register bits */
        struct
        {
		/** Core Soft Reset (CSftRst) (Device and Host)
		 *
		 * The application can flush the control logic in the
		 * entire core using this bit. This bit resets the
		 * pipelines in the AHB Clock domain as well as the
		 * PHY Clock domain.
		 *
		 * The state machines are reset to an IDLE state, the
		 * control bits in the CSRs are cleared, all the
		 * transmit FIFOs and the receive FIFO are flushed.
		 *
		 * The status mask bits that control the generation of
		 * the interrupt, are cleared, to clear the
		 * interrupt. The interrupt status bits are not
		 * cleared, so the application can get the status of
		 * any events that occurred in the core after it has
		 * set this bit.
		 *
		 * Any transactions on the AHB are terminated as soon
		 * as possible following the protocol. Any
		 * transactions on the USB are terminated immediately.
		 *
		 * The configuration settings in the CSRs are
		 * unchanged, so the software doesn't have to
		 * reprogram these registers (Device
		 * Configuration/Host Configuration/Core System
		 * Configuration/Core PHY Configuration).
		 *
		 * The application can write to this bit, any time it
		 * wants to reset the core. This is a self clearing
		 * bit and the core clears this bit after all the
		 * necessary logic is reset in the core, which may
		 * take several clocks, depending on the current state
		 * of the core.
		 */
		unsigned csftrst : 1;
		/** Hclk Soft Reset
		 *
		 * The application uses this bit to reset the control logic in
		 * the AHB clock domain. Only AHB clock domain pipelines are
		 * reset.
		 */
		unsigned hsftrst : 1;
		/** Host Frame Counter Reset (Host Only)<br>
		 *
		 * The application can reset the (micro)frame number
		 * counter inside the core, using this bit. When the
		 * (micro)frame counter is reset, the subsequent SOF
		 * sent out by the core, will have a (micro)frame
		 * number of 0.
		 */
		unsigned frmcntrrst : 1;
		/** In Token Sequence Learning Queue Flush
		 * (INTknQFlsh) (Device Only)
		 */
		unsigned intknqflsh : 1;
		/** RxFIFO Flush (RxFFlsh) (Device and Host)
		 *
		 * The application can flush the entire Receive FIFO
		 * using this bit.	<p>The application must first
		 * ensure that the core is not in the middle of a
		 * transaction.	 <p>The application should write into
		 * this bit, only after making sure that neither the
		 * DMA engine is reading from the RxFIFO nor the MAC
		 * is writing the data in to the FIFO.	<p>The
		 * application should wait until the bit is cleared
		 * before performing any other operations. This bit
		 * will takes 8 clocks (slowest of PHY or AHB clock)
		 * to clear.
		 */
		unsigned rxfflsh : 1;
		/** TxFIFO Flush (TxFFlsh) (Device and Host).
		 *
		 * This bit is used to selectively flush a single or
		 * all transmit FIFOs.	The application must first
		 * ensure that the core is not in the middle of a
		 * transaction.	 <p>The application should write into
		 * this bit, only after making sure that neither the
		 * DMA engine is writing into the TxFIFO nor the MAC
		 * is reading the data out of the FIFO.	 <p>The
		 * application should wait until the core clears this
		 * bit, before performing any operations. This bit
		 * will takes 8 clocks (slowest of PHY or AHB clock)
		 * to clear.
		 */
		unsigned txfflsh : 1;

		/** TxFIFO Number (TxFNum) (Device and Host).
		 *
		 * This is the FIFO number which needs to be flushed,
		 * using the TxFIFO Flush bit. This field should not
		 * be changed until the TxFIFO Flush bit is cleared by
		 * the core.
		 *	 - 0x0 : Non Periodic TxFIFO Flush
		 *	 - 0x1 : Periodic TxFIFO #1 Flush in device mode
		 *	   or Periodic TxFIFO in host mode
		 *	 - 0x2 : Periodic TxFIFO #2 Flush in device mode.
		 *	 - ...
		 *	 - 0xF : Periodic TxFIFO #15 Flush in device mode
		 *	 - 0x10: Flush all the Transmit NonPeriodic and
		 *	   Transmit Periodic FIFOs in the core
		 */
		unsigned txfnum : 5;
		/** Reserved */
		unsigned reserved11_29 : 19;
		/** DMA Request Signal.	 Indicated DMA request is in
		 * probress.  Used for debug purpose. */
		unsigned dmareq : 1;
		/** AHB Master Idle.  Indicates the AHB Master State
		 * Machine is in IDLE condition. */
		unsigned ahbidle : 1;
        } b;
} GRSTCTL_DATA;

/**
 * This union represents the bit fields of the Core Interrupt Mask
 * Register (GINTMSK).  Set/clear the bits using the bit fields then
 * write the <i>d32</i> value to the register.
 */
typedef union tagGINTMSK_DATA                  //*
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
		unsigned curmod : 1;
		unsigned modemis : 1;
		unsigned otgint : 1;
		unsigned sof : 1;
		unsigned rxflvl : 1;
		unsigned nptxfemp : 1;
		unsigned ginnakeff : 1;
		unsigned goutnakeff : 1;
		unsigned reserved8 : 1;
		unsigned i2cint : 1;
		unsigned erlysusp : 1;
		unsigned usbsusp : 1;
		unsigned usbrst : 1;
		unsigned enumdone : 1;
		unsigned isooutdrop : 1;
		unsigned eopf : 1;
		unsigned reserved16_20 : 5;
		unsigned incompip : 1;
		unsigned reserved22_23 : 2;
		unsigned prtint : 1;
		unsigned hchint : 1;
		unsigned ptxfemp : 1;
		unsigned reserved27 : 1;
		unsigned conidstschng : 1;
		unsigned disconnint : 1;
		unsigned sessreqint : 1;
		unsigned wkupint : 1;
    } b;
} GINTMSK_DATA;
/**
 * This union represents the bit fields of the Core Interrupt Register
 * (GINTSTS).  Set/clear the bits using the bit fields then write the
 * <i>d32</i> value to the register.
 */
typedef union tagGINTSTS_DATA                  //*
{
	/** raw register data */
	uint32 d32;
#define DWC_SOF_INTR_MASK 0x0008
	/** register bits */
	struct
	{
#define DWC_DEVICE_MODE 0	
#define DWC_HOST_MODE 1
		unsigned curmod : 1;
		unsigned modemis : 1;
		unsigned otgint : 1;
		unsigned sof : 1;
		unsigned rxflvl : 1;
		unsigned nptxfemp : 1;
		unsigned ginnakeff : 1;
		unsigned goutnakeff : 1;
		unsigned reserved8 : 1;
		unsigned i2cint : 1;
		unsigned erlysusp : 1;
		unsigned usbsusp : 1;
		unsigned usbrst : 1;
		unsigned enumdone : 1;
		unsigned isooutdrop : 1;
		unsigned eopf : 1;
		unsigned reserved16_20 : 5;
		unsigned incompip : 1;
		unsigned reserved22_23 : 2;
		unsigned prtint : 1;
		unsigned hchint : 1;
		unsigned ptxfemp : 1;
		unsigned reserved27 : 1;
		unsigned conidstschng : 1;
		unsigned disconnint : 1;
		unsigned sessreqint : 1;
		unsigned wkupint : 1;
	} b;
} GINTSTS_DATA;

/**
 * This union represents the bit fields in the Host Receive status Read and
 * Pop Registers (GRXSTSR, GRXSTSP) Read the register into the <i>d32</i>
 * element then read out the bits using the <i>b</i>it elements.
 */
typedef union tagGRXSTSH_DATA
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct {
		unsigned chnum : 4;
		unsigned bcnt : 11;
		unsigned dpid : 2;

		unsigned pktsts : 4;
#define DWC_GRXSTS_PKTSTS_IN			  0x2
#define DWC_GRXSTS_PKTSTS_IN_XFER_COMP	  0x3
#define DWC_GRXSTS_PKTSTS_DATA_TOGGLE_ERR 0x5
#define DWC_GRXSTS_PKTSTS_CH_HALTED		  0x7

		unsigned reserved : 11;
	} b;
} GRXSTSH_DATA;

/**
 * This union represents the bit fields in the FIFO Size Registers (HPTXFSIZ,
 * GNPTXFSIZ, DPTXFSIZn). Read the register into the <i>d32</i> element then
 * read out the bits using the <i>b</i>it elements.
 */
typedef union tagFIFOSIZE_DATA
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct {
		unsigned startaddr : 16;
		unsigned depth : 16;
	} b;
} FIFOSIZE_DATA;

/**
 * This union represents the bit fields in the Non-Periodic Transmit
 * FIFO/Queue status Register (GNPTXSTS). Read the register into the
 * <i>d32</i> element then read out the bits using the <i>b</i>it
 * elements.
 */
typedef union tagGNPTXSTS_DATA
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct {
		unsigned nptxfspcavail : 16;
		unsigned nptxqspcavail : 8;
		/** Top of the Non-Periodic Transmit Request Queue
		 *	- bit 24 - Terminate (Last entry for the selected
		 *	  channel/EP)
		 *	- bits 26:25 - Token Type
		 *	  - 2'b00 - IN/OUT
		 *	  - 2'b01 - Zero Length OUT
		 *	  - 2'b10 - PING/Complete Split
		 *	  - 2'b11 - Channel Halt
		 *	- bits 30:27 - Channel/EP Number
		 */
		unsigned nptxqtop_terminate : 1;
		unsigned nptxqtop_token : 2;
		unsigned nptxqtop_chnep : 4;
		unsigned reserved : 1;
        } b;
} GNPTXSTS_DATA;

/**
 * This union represents the bit fields in the User HW Config1
 * Register.  Read the register into the <i>d32</i> element then read
 * out the bits using the <i>b</i>it elements.
 */
typedef union tagHWCFG1_DATA
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct {
		unsigned ep_dir0 : 2;
		unsigned ep_dir1 : 2;
		unsigned ep_dir2 : 2;
		unsigned ep_dir3 : 2;
		unsigned ep_dir4 : 2;
		unsigned ep_dir5 : 2;
		unsigned ep_dir6 : 2;
		unsigned ep_dir7 : 2;
		unsigned ep_dir8 : 2;
		unsigned ep_dir9 : 2;
		unsigned ep_dir10 : 2;
		unsigned ep_dir11 : 2;
		unsigned ep_dir12 : 2;
		unsigned ep_dir13 : 2;
		unsigned ep_dir14 : 2;
		unsigned ep_dir15 : 2;
	} b;
} HWCFG1_DATA;

/**
 * This union represents the bit fields in the User HW Config2
 * Register.  Read the register into the <i>d32</i> element then read
 * out the bits using the <i>b</i>it elements.
 */
typedef union tagHWCFG2_DATA
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct {
		/* GHWCFG2 */
		unsigned op_mode : 3;
#define DWC_HWCFG2_OP_MODE_HNP_SRP_CAPABLE_OTG 0
#define DWC_HWCFG2_OP_MODE_SRP_ONLY_CAPABLE_OTG 1
#define DWC_HWCFG2_OP_MODE_NO_HNP_SRP_CAPABLE_OTG 2
#define DWC_HWCFG2_OP_MODE_SRP_CAPABLE_DEVICE 3
#define DWC_HWCFG2_OP_MODE_NO_SRP_CAPABLE_DEVICE 4
#define DWC_HWCFG2_OP_MODE_SRP_CAPABLE_HOST 5
#define DWC_HWCFG2_OP_MODE_NO_SRP_CAPABLE_HOST 6

		unsigned architecture : 2;
		unsigned point2point : 1;
		unsigned hs_phy_type : 2;
#define DWC_HWCFG2_HS_PHY_TYPE_NOT_SUPPORTED 0
#define DWC_HWCFG2_HS_PHY_TYPE_UTMI 1
#define DWC_HWCFG2_HS_PHY_TYPE_ULPI 2
#define DWC_HWCFG2_HS_PHY_TYPE_UTMI_ULPI 3

		unsigned fs_phy_type : 2;
		unsigned num_dev_ep : 4;
		unsigned num_host_chan : 4;
		unsigned perio_ep_supported : 1;
		unsigned dynamic_fifo : 1;
		unsigned rx_status_q_depth : 2;
		unsigned nonperio_tx_q_depth : 2;
		unsigned host_perio_tx_q_depth : 2;
		unsigned dev_token_q_depth : 5;
		unsigned reserved31 : 1;
	} b;
} HWCFG2_DATA;

/**
 * This union represents the bit fields in the User HW Config3
 * Register.  Read the register into the <i>d32</i> element then read
 * out the bits using the <i>b</i>it elements.
 */
typedef union tagHWCFG3_DATA                   //*
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct {
		/* GHWCFG3 */
		unsigned xfer_size_cntr_width : 4;
		unsigned packet_size_cntr_width : 3;
		unsigned otg_func : 1;
		unsigned i2c : 1;
		unsigned vendor_ctrl_if : 1;
		unsigned optional_features : 1;
		unsigned synch_reset_type : 1;
		unsigned reserved15_12 : 4;
		unsigned dfifo_depth : 16;
	} b;
} HWCFG3_DATA;

/**
 * This union represents the bit fields in the User HW Config4
 * Register.  Read the register into the <i>d32</i> element then read
 * out the bits using the <i>b</i>it elements.
 */
typedef union tagHWCFG4_DATA                   //*
{
        /** raw register data */
        uint32 d32;
        /** register bits */
	struct {
		unsigned num_dev_perio_in_ep : 4;
		unsigned power_optimiz : 1;
		unsigned min_ahb_freq : 9;
		unsigned utmi_phy_data_width : 2;
		unsigned num_dev_mode_ctrl_ep : 4;
		unsigned iddig_filt_en : 1;
		unsigned vbus_valid_filt_en : 1;
		unsigned a_valid_filt_en : 1;
		unsigned b_valid_filt_en : 1;
		unsigned session_end_filt_en : 1;
		unsigned ded_fifo_en : 1;
		unsigned num_in_eps : 4;
		unsigned reserved31_30 : 2;

        } b;
} HWCFG4_DATA;

/**
 * This union represents the bit fields in the Host Configuration Register.
 * Read the register into the <i>d32</i> member then set/clear the bits using
 * the <i>b</i>it elements. Write the <i>d32</i> member to the hcfg register.
 */
typedef union tagHCFG_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits */
	struct {
		/** FS/LS Phy Clock Select */
		unsigned fslspclksel : 2;
#define DWC_HCFG_30_60_MHZ 0
#define DWC_HCFG_48_MHZ	   1
#define DWC_HCFG_6_MHZ	   2

		/** FS/LS Only Support */
		unsigned fslssupp : 1;
		} b;
} HCFG_DATA;

/**
 * This union represents the bit fields in the Host Frame Remaing/Number
 * Register.
 */
typedef union tagHFIR_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits */
        struct {
		unsigned frint : 16;
		unsigned reserved : 16;
        } b;
} HFIR_DATA;

/**
 * This union represents the bit fields in the Host Frame Remaing/Number
 * Register.
 */
typedef union tagHFNUM_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits */
        struct {
		unsigned frnum : 16;
#define DWC_HFNUM_MAX_FRNUM 0x3FFF
		unsigned frrem : 16;
        } b;
} HFNUM_DATA;

typedef union tagHPTXSTS_DATA
{
	/** raw register data */
	uint32 d32;

	/** register bits */
	struct {
		unsigned ptxfspcavail : 16;
		unsigned ptxqspcavail : 8;
		/** Top of the Periodic Transmit Request Queue
		 *	- bit 24 - Terminate (last entry for the selected channel)
		 *	- bits 26:25 - Token Type
		 *	  - 2'b00 - Zero length
		 *	  - 2'b01 - Ping
		 *	  - 2'b10 - Disable
		 *	- bits 30:27 - Channel Number
		 *	- bit 31 - Odd/even microframe
		 */
		unsigned ptxqtop_terminate : 1;
		unsigned ptxqtop_token : 2;
		unsigned ptxqtop_chnum : 4;
		unsigned ptxqtop_odd : 1;
	} b;
} HPTXSTS_DATA;

/**
 * This union represents the bit fields in the Host Port Control and status
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements. Write the <i>d32</i> member to the
 * hprt0 register.
 */
typedef union tagHPRT0_DATA
{
        /** raw register data */
        uint32 d32;
        /** register bits */
        struct {
		unsigned prtconnsts : 1;
		unsigned prtconndet : 1;
		unsigned prtena : 1;
		unsigned prtenchng : 1;
		unsigned prtovrcurract : 1;
		unsigned prtovrcurrchng : 1;
		unsigned prtres : 1;
		unsigned prtsusp : 1;
		unsigned prtrst : 1;
		unsigned reserved9 : 1;
		unsigned prtlnsts : 2;
		unsigned prtpwr : 1;
		unsigned prttstctl : 4;
		unsigned prtspd : 2;
#define DWC_HPRT0_PRTSPD_HIGH_SPEED 0
#define DWC_HPRT0_PRTSPD_FULL_SPEED 1
#define DWC_HPRT0_PRTSPD_LOW_SPEED	2
		unsigned reserved19_31 : 13;
        } b;
} HPRT0_DATA;

/**
 * This union represents the bit fields in the Host All Interrupt
 * Register.
 */
typedef union tagHAINT_DATA
{
        /** raw register data */
        uint32 d32;
        /** register bits */
        struct {
		unsigned ch0 : 1;
		unsigned ch1 : 1;
		unsigned ch2 : 1;
		unsigned ch3 : 1;
		unsigned ch4 : 1;
		unsigned ch5 : 1;
		unsigned ch6 : 1;
		unsigned ch7 : 1;
		unsigned ch8 : 1;
		unsigned ch9 : 1;
		unsigned ch10 : 1;
		unsigned ch11 : 1;
		unsigned ch12 : 1;
		unsigned ch13 : 1;
		unsigned ch14 : 1;
		unsigned ch15 : 1;
		unsigned reserved : 16;
	} b;
        struct {
		unsigned chint : 16;
		unsigned reserved : 16;
	} b2;
} HAINT_DATA;

/**
 * This union represents the bit fields in the Host All Interrupt
 * Register.
 */
typedef union tagHAINTMSK_DATA
{
        /** raw register data */
        uint32 d32;
        /** register bits */
        struct {
		unsigned ch0 : 1;
		unsigned ch1 : 1;
		unsigned ch2 : 1;
		unsigned ch3 : 1;
		unsigned ch4 : 1;
		unsigned ch5 : 1;
		unsigned ch6 : 1;
		unsigned ch7 : 1;
		unsigned ch8 : 1;
		unsigned ch9 : 1;
		unsigned ch10 : 1;
		unsigned ch11 : 1;
		unsigned ch12 : 1;
		unsigned ch13 : 1;
		unsigned ch14 : 1;
		unsigned ch15 : 1;
		unsigned reserved : 16;
	} b;
        struct {
		unsigned chint : 16;
		unsigned reserved : 16;
	} b2;
} HAINTMSK_DATA;

/**
 * This union represents the bit fields in the Host Channel Characteristics
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements. Write the <i>d32</i> member to the
 * hcchar register.
 */
typedef union tagHCCHAR_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits */
	struct {
		/** Maximum packet size in bytes */
		unsigned mps : 11;

		/** Endpoint number */
		unsigned epnum : 4;

		/** 0: OUT, 1: IN */
		unsigned epdir : 1;

		unsigned reserved : 1;

		/** 0: Full/high speed device, 1: Low speed device */
		unsigned lspddev : 1;

		/** 0: Control, 1: Isoc, 2: Bulk, 3: Intr */
		unsigned eptype : 2;

		/** Packets per frame for periodic transfers. 0 is reserved. */
		unsigned multicnt : 2;

		/** Device address */
		unsigned devaddr : 7;

		/**
		 * Frame to transmit periodic transaction.
		 * 0: even, 1: odd
		 */
		unsigned oddfrm : 1;

		/** Channel disable */
		unsigned chdis : 1;

		/** Channel enable */
		unsigned chen : 1;
	} b;
} HCCHAR_DATA;

typedef union tagHCSPLT_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits */
	struct {
		/** Port Address */
		unsigned prtaddr : 7;

		/** Hub Address */
		unsigned hubaddr : 7;

		/** Transaction Position */
		unsigned xactpos : 2;
#define DWC_HCSPLIT_XACTPOS_MID 0
#define DWC_HCSPLIT_XACTPOS_END 1
#define DWC_HCSPLIT_XACTPOS_BEGIN 2
#define DWC_HCSPLIT_XACTPOS_ALL 3

		/** Do Complete Split */
		unsigned compsplt : 1;

		/** Reserved */
		unsigned reserved : 14;

		/** Split Enble */
		unsigned spltena : 1;
	} b;
} HCSPLT_DATA;


/**
 * This union represents the bit fields in the Host All Interrupt
 * Register.
 */
typedef union tagHCINT_DATA
{
        /** raw register data */
        uint32 d32;
        /** register bits */
	struct {
		/** Transfer Complete */
		unsigned xfercomp : 1;
		/** Channel Halted */
		unsigned chhltd : 1;
		/** AHB Error */
		unsigned ahberr : 1;
		/** STALL Response Received */
		unsigned stall : 1;
		/** NAK Response Received */
		unsigned nak : 1;
		/** ACK Response Received */
		unsigned ack : 1;
		/** NYET Response Received */
		unsigned nyet : 1;
		/** Transaction Err */
		unsigned xacterr : 1;
		/** Babble Error */
		unsigned bblerr : 1;
		/** Frame Overrun */
		unsigned frmovrun : 1;
		/** Data Toggle Error */
		unsigned datatglerr : 1;
		/** Reserved */
		unsigned reserved : 21;
	} b;
} HCINT_DATA;

/**
 * This union represents the bit fields in the Host Channel Transfer Size
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements. Write the <i>d32</i> member to the
 * hcchar register.
 */
typedef union tagHCTSIZ_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits */
        struct {
		/** Total transfer size in bytes */
		unsigned xfersize : 19;

		/** Data packets to transfer */
		unsigned pktcnt : 10;

		/**
		 * Packet ID for next data packet
		 * 0: DATA0
		 * 1: DATA2
		 * 2: DATA1
		 * 3: MDATA (non-Control), SETUP (Control)
		 */
		unsigned pid : 2;
#define DWC_HCTSIZ_DATA0 0
#define DWC_HCTSIZ_DATA1 2
#define DWC_HCTSIZ_DATA2 1
#define DWC_HCTSIZ_MDATA 3
#define DWC_HCTSIZ_SETUP 3

		/** Do PING protocol when 1 */
		unsigned dopng : 1;
        } b;
} HCTSIZ_DATA;

/**
 * This union represents the bit fields in the Host Channel Interrupt Mask
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements. Write the <i>d32</i> member to the
 * hcintmsk register.
 */
typedef union tagHCINTMSK_DATA
{
        /** raw register data */
        uint32 d32;

        /** register bits */
        struct {
		unsigned xfercompl : 1;
		unsigned chhltd : 1;
		unsigned ahberr : 1;
		unsigned stall : 1;
		unsigned nak : 1;
		unsigned ack : 1;
		unsigned nyet : 1;
		unsigned xacterr : 1;
		unsigned bblerr : 1;
		unsigned frmovrun : 1;
		unsigned datatglerr : 1;
		unsigned reserved : 21;
	} b;
} HCINTMSK_DATA;


/**
 * This union represents the bit fields in the Power and Clock Gating Control
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements.
 */
typedef union tagPCGCCTL_DATA
{
	/** raw register data */
	uint32 d32;

	/** register bits */
	struct {
		/** Stop Pclk */
		unsigned stoppclk : 1;
		/** Gate Hclk */
		unsigned gatehclk : 1;
		/** Power Clamp */
		unsigned pwrclmp : 1;
		/** Reset Power Down Modules */
		unsigned rstpdwnmodule : 1;
		/** PHY Suspended */
		unsigned physuspended : 1;

		unsigned reserved : 27;
	} b;
} PCGCCTL_DATA;

/**
 * This union represents the bit fields in the Device Receive Status Read and 
 * Pop Registers (GRXSTSR, GRXSTSP) Read the register into the <i>d32</i> 
 * element then read out the bits using the <i>b</i>it elements.
 */
typedef union tagDEVICE_GRXSTS_DATA 
{
	/** raw register data */
	uint32 d32;
	/** register bits */
	struct 
	{
		unsigned epnum : 4;
		unsigned bcnt : 11;
		unsigned dpid : 2;
		
#define DWC_STS_DATA_UPDT		0x2				  // OUT Data Packet
#define DWC_STS_XFER_COMP		0x3				  // OUT Data Transfer Complete

#define DWC_DSTS_GOUT_NAK		0x1				  // Global OUT NAK
#define DWC_DSTS_SETUP_COMP		0x4				  // Setup Phase Complete
#define DWC_DSTS_SETUP_UPDT 0x6				  // SETUP Packet
		unsigned pktsts : 4;
		unsigned fn : 4;
		unsigned reserved : 7;
	} b;
} DEVICE_GRXSTS_DATA;


/*----------------------------- External Variables ---------------------------*/

#define     OTGReg  ((pUSB_OTG_REG)OTG_REG_BASE)
//pUSB_OTG_REG    OTGReg;

/*------------------------- Global Function Prototypes -----------------------*/

#endif
