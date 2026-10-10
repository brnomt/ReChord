/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SFC.c
Desc:

Author: chenfen
Date: 14-02-25
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/

#include "SysConfig.h"
#include "MDConfig.h"
#ifdef SPIFLASH_DRIVER
#include "DriverInclude.h"
#include "SFC.h"
#include "Hw_cru.h"
/*------------------------------------ Defines -------------------------------*/
//#define     SFC_BASE_ADDR           0x10208000


#define DMA_INT                     (1 << 7)      //dma interrupt
#define NSPIERR_INT                 (1 << 6)      //Nspi error interrupt
#define AHBERR_INT                  (1 << 5)      //Ahb bus error interrupt
#define FINISH_INT                  (1 << 4)      //Transfer finish interrupt
#define TXEMPTY_INT                 (1 << 3)      //Tx fifo empty interrupt
#define TXOF_INT                    (1 << 2)      //Tx fifo overflow interrupt
#define RXUF_INT                    (1 << 1)      //Rx fifo underflow interrupt
#define RXFULL_INT                  (1 << 0)      //Rx fifo full interrupt

/* SFC_FSR Register*/
#define SFC_RXFULL                  (1 << 3)      //rx fifo full
#define SFC_RXEMPTY                 (1 << 2)      //rx fifo empty
#define SFC_TXEMPTY                 (1 << 1)      //tx fifo empty
#define SFC_TXFULL                  (1 << 0)      //tx fifo full

/* SFC_RCVR Register */
#define SFC_RESET                   (1 << 0)     //controller reset

/* SFC_SR Register */
#define SFC_BUSY                    (1 << 0)     //sfc busy flag. When busy , don��t try to set the control register

/* SFC_DMA_TRIGGER Register */
#define SFC_DMA_START               (1 << 0)     //Dma start trigger signal. Auto cleared after write


/*----------------------------------- Typedefs -------------------------------*/
typedef union tagSFCFTLR_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
        /*Tx fifo threshold level
        ��TX FIFO�е����ݸ���С���ڸ�ˮ�ߵ�ʱ�򣬻ᴥ��
        tx fifo empty interrupt*/
        unsigned txthresholdlevel : 8;
        /*Rx fifo threshold level
        ��RX FIFO�е�����������ڸ�ˮ�ߵ�ʱ�򣬻ᴥ��rx
        fifo full interrupt*/
        unsigned rxthresholdlevel : 8;

        unsigned reserved31_16 : 16;
    } b;
}SFCFTLR_DATA;

typedef union tagSFCFSR_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
        /*tx fifo empty*/
        unsigned txempty : 1;
        /*tx fifo full*/
        unsigned txfull :  1;
        /*rx fifo empty*/
        unsigned rxempty : 1;
        /*Tx fifo empty interrupt mask*/
        unsigned rxfull :  1;

        unsigned reserved7_4 : 4;
        /*tx fifo waterlevel:
        0x0: fifo is full
        0x1:left 1 entry
        ��
        0x10: left 16 entry,fifo is empty*/
        unsigned txlevel : 5;

        unsigned reserved15_13 : 3;
        /*rx fifo waterlevel:
        0x0: fifo is empty
        0x1: one entry is taken
        ��
        0x10: 16 entry is taken fifo is full*/
        unsigned rxlevel : 5;

        unsigned reserved31_21 : 11;
    } b;
}SFCFSR_DATA;

//SFC Registers
typedef volatile struct tagSFC_STRUCT
{
    uint32 SFC_CTRL;
    uint32 SFC_IMR;
    uint32 SFC_ICLR;
    uint32 SFC_FTLR;
    uint32 SFC_RCVR;
    uint32 SFC_AX;
    uint32 SFC_ABIT;
    uint32 SFC_MASKISR;
    uint32 SFC_FSR;
    uint32 SFC_SR;
    uint32 SFC_RAWISR;
    uint32 reserved1[(0x080 - 0x02c)/4];
    uint32 SFC_DMA_TRIGGER;
    uint32 SFC_DMA_ADDR;
    uint32 reserved2[(0x100 - 0x088)/4];
    uint32 SFC_CMD;
    uint32 SFC_ADDR;
    uint32 SFC_DATA;
}SFC_REG,*pSFC_REG;

typedef volatile struct tagSFC_HOST
{
    uint32  signal;
    //uint8   EnableDMA;
}SFC_HOST, *pSFC_HOST;

/*-------------------------- Forward Declarations ----------------------------*/
extern void InterruptInit(void);
extern void EnableSFCIntr(void);

/* ------------------------------- Globals ---------------------------------- */


/*-------------------------------- Local Statics: ----------------------------*/

/*--------------------------- Local Function Prototypes ----------------------*/
static SFC_HOST SFCHost;

/*------------------------ Function Implement --------------------------------*/

/*
Name:       SFC_HandleIRQ
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
void SFC_HandleIRQ(void)
{
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;
    pSFC_HOST pSFC = &SFCHost;
    volatile uint32  value;

    value = pReg->SFC_RAWISR;
    if (value & FINISH_INT)
    {
        pSFC->signal |= FINISH_INT;
    }

    pReg->SFC_ICLR = value;
}

/*
Name:       SFC_Delay
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
void SFC_Delay(uint32 us)
{
    DelayUs(us);
}

/*
Name:       SFC_SetClk
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static __inline void SFC_SetClk(void)
{
    //pCRU_REG cruReg=(pCRU_REG)CRU_BASE;

    /*sclk = 12M, from 24Mhz. sfc_div_con=0,��Ϊ�ڲ���2��Ƶ. */
    SetSFCFreq(XIN12M,24*1000*1000);
    //cruReg->CRU_CLKSEL_CON[10] = (((0x1<<15)|(0x1F<<8))<<16)|(0x1<<15)|(0<<8);
}

/*
Name:       SFC_IOMUX
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static __inline void SFC_IOMUX(void)
{
    //sfc_d1 sfc_d0,sfc_clk,sfc_csn0
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7,IOMUX_GPIO0A7_SFC_D1);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0,IOMUX_GPIO0B0_SFC_D0);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1,IOMUX_GPIO0B1_SFC_CLK);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2,IOMUX_GPIO0B2_SFC_CS);
    DelayUs(1);
}

/*
Name:       SFC_Reset
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static void SFC_Reset(pSFC_REG pReg)
{
    int32 timeout = 10000;

    pReg->SFC_RCVR = SFC_RESET;
    while ((pReg->SFC_RCVR == SFC_RESET) && (timeout > 0))
    {
        SFC_Delay(1);
        timeout--;
    }
    pReg->SFC_ICLR = 0xFFFFFFFF;
}

/*
Name:       SFC_WaitSignal
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static __inline int32 SFC_WaitSignal(int32 timeout, uint32 signal)
{
    pSFC_HOST pSFC = &SFCHost;

    while (!(pSFC->signal &  signal))
    {
        if (timeout == 0)
            return SFC_WAIT_TIMEOUT;

        timeout--;
        SFC_Delay(1);
    }

    //need disable interrupt
    pSFC->signal &= ~signal;
    //need enable interrupt

    return SFC_OK;
}

/*
Name:       SFC_Init
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/

int32 SFC_Init(void)
{
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;
    pSFC_HOST   pSFC = &SFCHost;

    ScuClockGateCtr(CLK_SFC_GATE,1);
    ScuClockGateCtr(HCLK_SFC_GATE,1);
    ScuSoftResetCtr(SFC_SRST, 0);
    SFC_SetClk();
    SFC_IOMUX();

    memset((void*)pSFC, 0, sizeof(SFC_HOST));
    SFC_Reset(pReg);

    pReg->SFC_CTRL = 0;             //Data bits:1,Addr bits:1,Cmd bits:1, SPI mode 0

    #if (SFC_EN_INT == 1)
    InterruptInit();
    EnableSFCIntr();
    #endif

    //pReg->SFC_FTLR = (SFC_RX_WMARK << SFC_RX_WMARK_SHIFT) | (SFC_TX_WMARK << SFC_TX_WMARK_SHIFT)
    //pReg->SFC_ABIT = 0;
    //pReg->SFC_IMR = 0xFFFFFFFF;     //interrupt mask

    return OK;
}

/*
Name:       SFC_Request
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
int32 SFC_Request(uint32 sfcmd, uint32 sfctrl, uint32 addr, void *data)
{
    pSFC_REG        pReg = (pSFC_REG)SFC_BASE;
    int32           ret = SFC_OK;
    SFCCMD_DATA     cmd;
    volatile  int32 timeout = 0;

    if (!(pReg->SFC_FSR & SFC_TXEMPTY) || !(pReg->SFC_FSR & SFC_RXEMPTY) || (pReg->SFC_SR & SFC_BUSY))
    {
        SFC_Reset(pReg);
    }

    cmd.d32 = sfcmd;
    if (SFC_ADDR_XBITS == cmd.b.addrbits)
    {
        SFCCTRL_DATA ctrl;
        ctrl.d32 = sfctrl;
        if (!ctrl.b.addrbits)
            return SFC_PARAM_ERR;
        pReg->SFC_ABIT = ctrl.b.addrbits-1;         //�������ڲ����Զ���1
    }

    pReg->SFC_CTRL = sfctrl;
    pReg->SFC_CMD = sfcmd;
    if (cmd.b.addrbits)
        pReg->SFC_ADDR = addr;

    if (cmd.b.datasize)
    {
        #if (SFC_EN_DMA == 1)
        if (SFC_ENABLE_DMA & sfctrl)
        {
            pReg->SFC_ICLR = 0xFFFFFFFF;
            pReg->SFC_DMA_ADDR = (uint32)data;
            pReg->SFC_DMA_TRIGGER = SFC_DMA_START;
            timeout = cmd.b.datasize*10;

            #if (SFC_EN_INT == 1)
            pReg->SFC_IMR &= ~FINISH_INT;    //not mask Transfer finish interrupt
            ret = SFC_WaitSignal(timeout, FINISH_INT);
            pReg->SFC_IMR |= FINISH_INT;
            #else
            while ((!(pReg->SFC_RAWISR & FINISH_INT)) && (timeout-- > 0))
            {
                SFC_Delay(1);
            }
            pReg->SFC_ICLR = 0xFFFFFFFF;
            if (timeout <= 0)
                ret = SFC_WAIT_TIMEOUT;
            #endif
        }
        else
        #endif
        {
            uint32                  i, words;
            volatile SFCFSR_DATA    fifostat;
            uint32                  *pData = (uint32*)data;

            if (SFC_WRITE == cmd.b.rw) //write data
            {
                words  = (cmd.b.datasize+3)>>2;
                while (words)
                {

                    fifostat.d32 = pReg->SFC_FSR;
                    if (fifostat.b.txlevel > 0)
                    {
                        uint32 count = MIN(words, fifostat.b.txlevel);
                        for (i=0; i<count; i++)
                        {
                            pReg->SFC_DATA = *pData++;
                            words--;
                        }
                        if (0 == words)
                            break;
                        timeout = 0;
                    }
                    else
                    {
                        SFC_Delay(1);
                        if (timeout++ > 10000)
                        {
                            ret = SFC_TX_TIMEOUT;
                            break;
                        }
                    }
                }

            }
            else    //read data
            {
                uint32 bytes = cmd.b.datasize & 0x3;

                words = cmd.b.datasize>>2;
                while (words)
                {

                    fifostat.d32 = pReg->SFC_FSR;
                    if (fifostat.b.rxlevel > 0)
                    {
                        uint32 count = MIN(words, fifostat.b.rxlevel);
                        for (i=0; i<count; i++)
                        {
                            *pData++ = pReg->SFC_DATA;
                            words--;
                        }
                        if (0 == words)
                            break;
                        timeout = 0;
                    }
                    else
                    {
                        SFC_Delay(1);
                        if (timeout++ > 10000)
                        {
                            ret = SFC_RX_TIMEOUT;
                            break;
                        }
                    }
                }

                timeout = 0;
                while (bytes)
                {
                    fifostat.d32 = pReg->SFC_FSR;
                    if (fifostat.b.rxlevel > 0)
                    {
                        uint8 *pData1 = (uint8 *)pData;
                        words = pReg->SFC_DATA;
                        //memcpy(pData, &words, bytes);
                        for (i=0; i<bytes; i++)
                        {
                            pData1[i] = (uint8)((words>>(i*8)) & 0xFF);
                        }
                        break;
                    }
                    else
                    {
                        SFC_Delay(1);
                        if (timeout++ > 10000)
                        {
                            ret = SFC_RX_TIMEOUT;
                            break;
                        }
                    }
                }
            }
        }
    }

    timeout = 0;    //wait  cmd or data send complete
    while (!(pReg->SFC_FSR & SFC_TXEMPTY))
    {
        DelayUs(1);
        if (timeout++ > 100000)         //wait 100ms
        {
            ret = SFC_TX_TIMEOUT;
            break;
        }
    }

    SFC_Delay(2); //CS# High Time (read/write) >100ns
    return ret;
}

#endif
