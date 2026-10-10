/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SDCtrl.c
Desc:

Author: chenfen
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/

#include "SDConfig.h"

#if defined(SDMMC_DRIVER)

/*------------------------------------ Defines -------------------------------*/

//#define pSDCReg(n)        ((n == 0) ? ((pSDC_REG_T)SDC0_ADDR) : ( (n==1) ? ((pSDC_REG_T)SDC1_ADDR):(((pSDC_REG_T)SDC2_ADDR)) ) )
//#define pSDCFIFOADDR(n)   ((n == 0) ? ((volatile uint32 *)SDC0_FIFO_ADDR) : ( (n==1)? ((volatile uint32 *)SDC1_FIFO_ADDR): ((volatile uint32 *)SDC2_FIFO_ADDR) ) )
//#define pSDC_DMACB(n)     ((n == SDC0)? SDC0_DMACB : ((n == SDC1)? SDC1_DMACB : SDC2_DMACB));
//#define pSDC_ISR(n)       ((n ==SDC0)? SDC0_ISR : ((n == SDC1)? SDC1_ISR : SDC2_ISR)

#define pSDCReg(n)        ((n == 0) ? ((pSDC_REG_T)SDC0_ADDR) : ((pSDC_REG_T)SDC1_ADDR))
#define pSDCFIFOADDR(n)   ((n == 0) ? ((volatile uint32 *)SDC0_FIFO_ADDR) : ((volatile uint32 *)SDC1_FIFO_ADDR))
#define pSDC_DMACB(n)     ((n == SDC0)? SDC0_DMACB : SDC1_DMACB )
#define pSDC_ISR(n)       ((n ==SDC0)? SDC0_ISR : SDC1_ISR )

//#define pSDCFIFOADDR(n)    ((volatile uint32 *)SDC0_FIFO_ADDR)
//#define pSDCReg(n)         ((pSDC_REG_T)SDC0_ADDR)
//#define pSDC_DMACB(n)      SDC0_DMACB
//#define pSDC_ISR(n)        SDC0_ISR


//#define SDC_Start(reg, cmd) reg->SDMMC_CMD = cmd;
#define SDC_EnanbleDMA(reg) reg->SDMMC_CTRL |= ENABLE_DMA
#define SDC_DisableDMA(reg) reg->SDMMC_CTRL &= ~ENABLE_DMA

#define SDC_WriteData(fifo, buf, len)       \
while(len--)                                \
{                                           \
    *fifo = *buf++;                         \
}

#define SDC_ReadData(buf, fifo, len)        \
while(len--)                                \
{                                           \
    *buf++ = *fifo;                         \
}

#define SDC_WaitBitSet(cond)                \
do                                          \
{                                           \
    if (cond)                               \
        break;                              \
    SDA_Delay(1);                           \
}while(1)

/*----------------------------------- Typedefs -------------------------------*/

/* SDMMC Host Controller register struct */
typedef volatile struct TagSDC_REG
{
    volatile uint32 SDMMC_CTRL;        //SDMMC Control register
    volatile uint32 SDMMC_PWREN;       //Power enable register
    volatile uint32 SDMMC_CLKDIV;      //Clock divider register
    volatile uint32 SDMMC_CLKSRC;      //Clock source register
    volatile uint32 SDMMC_CLKENA;      //Clock enable register
    volatile uint32 SDMMC_TMOUT;       //Time out register
    volatile uint32 SDMMC_CTYPE;       //Card type register
    volatile uint32 SDMMC_BLKSIZ;      //Block size register
    volatile uint32 SDMMC_BYTCNT;      //Byte count register
    volatile uint32 SDMMC_INTMASK;     //Interrupt mask register
    volatile uint32 SDMMC_CMDARG;      //Command argument register
    volatile uint32 SDMMC_CMD;         //Command register
    volatile uint32 SDMMC_RESP0;       //Response 0 register
    volatile uint32 SDMMC_RESP1;       //Response 1 register
    volatile uint32 SDMMC_RESP2;       //Response 2 register
    volatile uint32 SDMMC_RESP3;       //Response 3 register
    volatile uint32 SDMMC_MINTSTS;     //Masked interrupt status register
    volatile uint32 SDMMC_RINISTS;     //Raw interrupt status register
    volatile uint32 SDMMC_STATUS;      //Status register
    volatile uint32 SDMMC_FIFOTH;      //FIFO threshold register
    volatile uint32 SDMMC_CDETECT;     //Card detect register
    volatile uint32 SDMMC_WRTPRT;      //Write protect register
    volatile uint32 SDMMC_GPIO;        //GPIO register
    volatile uint32 SDMMC_TCBCNT;      //Transferred CIU card byte count
    volatile uint32 SDMMC_TBBCNT;      //Transferred host/DMA to/from BIU_FIFO byte count
    volatile uint32 SDMMC_DEBNCE;      //Card detect debounce register
    volatile uint32 SDMMC_USRID;        //User ID register
    volatile uint32 SDMMC_VERID;        //Synopsys version ID register
    volatile uint32 SDMMC_HCON;         //Hardware configuration register
    volatile uint32 SDMMC_UHS_REG;      //UHS-1 register
    volatile uint32 SDMMC_RST_n;        //Hardware reset register
    volatile uint32 SDMMC_CARDTHRCTL;   //Card Read Threshold Enable
    volatile uint32 SDMMC_BACK_END_POWER; //Back-end Power
}SDC_REG_T,*pSDC_REG_T;

/*控制器传输信息*/
typedef struct tagSDC_TRANS_INFO
{
    uint32     TransLen;                   //已经发送或接收的数据长度
    uint32     NeedLen;                  //需要传输的数据长度
    uint32    *pBuf;                    //中断数据接收或发送数据用到的buf地址                                        //指针用32 bit的uint32指针，就可以满足SDMMC FIFO要求的32 bits对齐的要求了，                                    //这样就算你的数据没有4字节对齐，也会因为用了uint32指针，每次向FIFO操作是4字节对齐的。
}SDC_TRANS_INFO, *pSDC_TRANS_INFO;


/* SDMMC Host Controller Information */
typedef struct tagSDMMC_CTRL
{
    uint32              LowPower;
    pSDC_REG_T          pReg;           //SDMMC Host Controller Register, only for debug
    HOST_BUS_WIDTH      BusWidth;       //SDMMC Host Controller Bus Width，
                                      //控制器支持的总线宽度是由硬件设计的IO共用情况决定的，
                                      //插在该控制器上的卡设置总线宽度时受到该宽度的限制
    HOST_BUS_WIDTH      ConfigWidth;
    SDC_TRANS_INFO      TransInfo;      //控制器中断要用到的信息
    pEVENT              event;          //SDMMC Host Controller对应的事件
    uint32              CardFreq;       //卡的工作频率,单位KHz，这个频率只有SDM层有权修改
    bool                UpdatedFreq;  //表示是否需要更新卡的频率，用于由于AHB频率改变而导致需要更新卡的频率,TRUE:要更新，FALSE:不用更新

    bool                present;          //记录目前插拔的状态
    bool                changed;          //用于指示卡是否被插拔过

} SDMMC_CTRL, *pSDMMC_CTRL;

/*-------------------------- Forward Declarations ----------------------------*/
extern uint16 eMMC_ID;


/* ------------------------------- Globals ---------------------------------- */

_ATTR_SYS_BSS_ SDMMC_CTRL gSDMMCCtrl[MAX_SDC_NUM];

/*-------------------------------- Local Statics: ----------------------------*/

/*--------------------------- Local Function Prototypes ----------------------*/


/*------------------------ Function Implement --------------------------------*/

/*
Name:       SDC_StartCmd
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
static int32 SDC_StartCmd(pSDC_REG_T        pReg, uint32 cmd)
{
    int32 timeOut = 50000;

    pReg->SDMMC_CMD = cmd;
    while ((pReg->SDMMC_CMD & START_CMD) && (timeOut > 0))
    {
        timeOut--;
    }
    if(timeOut == 0)
    {
        return SDC_SDC_ERROR;
    }

    return SDC_SUCCESS;
}

/*
Name:       SDC_SetClock
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
static int32 SDC_SetClock(SDMMC_PORT_E SDCPort, bool enable)
{
    volatile uint32 value;
    pSDC_REG_T      pReg = pSDCReg(SDCPort);

    value = (TRUE == enable)? (CCLK_ENABLE|CCLK_LOW_POWER): 0;
    pReg->SDMMC_CLKENA = value;

    if (SDC_SUCCESS != SDC_StartCmd(pReg, (START_CMD | UPDATE_CLOCK | WAIT_PREV)))
    {
        return SDC_SDC_ERROR;
    }

    return SDC_SUCCESS;
}

/*
Name:       SDC_WaitBusy
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
int32 SDC_WaitBusy(SDMMC_PORT_E SDCPort)
{
    int32           ret = SDC_SUCCESS;
    pSDC_REG_T      pReg = pSDCReg(SDCPort);

    #if EN_SD_BUSY_INT
    if (SDC_SUCCESS != SDOAM_GetEvent(gSDMMCCtrl[SDCPort].event, BDONE_EVENT, 250000 * 4))
    {
        ret = SDC_BUSY_TIMEOUT;
    }
    pReg->SDMMC_INTMASK &= ~BDONE_INT;
    #else
    //wait busy
    int32 timeout = 0;
    volatile uint32 value;

    while ((value = pReg->SDMMC_STATUS) & DATA_BUSY)
    {
        SDA_Delay(1);
        timeout++;
        if (timeout > 1000000 * 5)
        {
            DEBUG("\nSDC_WaitBusy: Time out!");
            ret = SDC_BUSY_TIMEOUT;
            break;
        }
    }
    #endif
    return ret;
}

/*
Name:       SDC_HandleIRQ
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
static void SDC_HandleIRQ(SDMMC_PORT_E SDCPort)
{
    volatile uint32  value = 0;
    pSDC_REG_T       pReg = pSDCReg(SDCPort);
    pSDMMC_CTRL      pSDC = &gSDMMCCtrl[SDCPort];
    #if !(EN_SD_DMA)
    uint32*          pBuf = 0;
    uint32 i = 0;
    volatile uint32 *pFIFO = pSDCFIFOADDR(SDCPort);
    pSDC_TRANS_INFO      pTransInfo = &gSDMMCCtrl[SDCPort].TransInfo;
    #endif


    value = pReg->SDMMC_MINTSTS;
    #if EN_SD_DMA
    if (value & (RXDR_INT | TXDR_INT))
    {
        //Assert(0, "_SDCISTHandle:dma mode, but rx tx int\n", value);
    }
    #else
    if (value & RXDR_INT)
    {
        //Assert((pTransInfo->NeedLen > 0), "_SDCISTHandle:rx len <= 0\n", pTransInfo->NeedLen);

        pBuf = &(pTransInfo->pBuf[pTransInfo->TransLen]);
        i = (RX_WMARK+1);
        SDC_ReadData(pBuf, pFIFO, i);
        pTransInfo->TransLen += (RX_WMARK+1);

        pReg->SDMMC_RINISTS = RXDR_INT;
    }
    if (value & TXDR_INT)
    {
        ///Assert((pTransInfo->NeedLen > 0), "_SDCISTHandle:tx len <= 0\n", pSDC->TransInfo.NeedLen);

        //fill data
        pBuf = &(pTransInfo->pBuf[pTransInfo->TransLen]);
        i = pTransInfo->NeedLen-pTransInfo->TransLen;
        (i>(FIFO_DEPTH-TX_WMARK))? (i=(FIFO_DEPTH-TX_WMARK), pTransInfo->TransLen+=(FIFO_DEPTH-TX_WMARK)) : (pTransInfo->TransLen=pTransInfo->NeedLen);

        SDC_WriteData(pFIFO, pBuf, i);

        pReg->SDMMC_RINISTS = TXDR_INT;
    }
    #endif
    #if 0
    if (value & SDIO_INT)
    {
    }
    #endif
    if (value & CD_INT)
    {
        SDA_SetEvent(pSDC->event, CD_EVENT);
        pReg->SDMMC_RINISTS = CD_INT;
    }
    if (value & DTO_INT)
    {
        SDA_SetEvent(pSDC->event, DTO_EVENT);
        pReg->SDMMC_RINISTS = DTO_INT;
        #if EN_SD_BUSY_INT
        if (pSDC->EnBusyInt)
        {
            pSDC->EnBusyInt = 0;
            pReg->SDMMC_INTMASK |= BDONE_INT; //在数据传输完成中断中打开，避免在传输过程中被多次中断
        }
        #endif
    }

    #if EN_SD_BUSY_INT
    if (value & BDONE_INT)
    {
        SDA_SetEvent(pSDC->event, BDONE_EVENT);
        pReg->SDMMC_RINISTS = BDONE_INT;
    }
    #endif

    if (value & SBE_INT)
    {
        SDA_SetEvent(pSDC->event, DTO_EVENT);
        pReg->SDMMC_INTMASK &= ~SBE_INT;  //暂时关闭，在中断外部清中断
        pReg->SDMMC_RINISTS = SBE_INT;
    }

    #if 0
    if (value & CDT_INT)
    {
        if (pReg->SDMMC_CDETECT & NO_CARD_DETECT)
        {
            //卡的拔插，如果不用发消息，而用一个全局变量的话(假设gCardState)，会出现这样的问题，
            //已经识别好可用的卡，在使用过程中，我快速拔出，然后快速再插入，这时查询gCardState
            //的程序如果来不及响应，就会认为卡没有被动过，不会做任何动作，所以卡也就不能再使用了
            //而如果用消息的话，就会收到卡被拔出，然后又被插入，所以会重新再初始化一下，卡又可用了
            //SDM_NotifyAll((int32)nSDCPort, 0);
        }
        else
        {
            //SDM_NotifyAll((int32)nSDCPort, 1);
        }
        pReg->SDMMC_RINISTS = CDT_INT;
    }
    #endif

    if (value & FRUN_INT)
    {
        //Assert(0, "_SDCISTHandle:overrun or underrun\n", value);
        pReg->SDMMC_RINISTS = FRUN_INT;
    }
    if (value & HLE_INT)
    {
        //Assert(0, "_SDCISTHandle:hardware locked write error\n", value);
        pReg->SDMMC_RINISTS = HLE_INT;
    }

    return;
}


/*
Name:       SDC_HandleIRQ
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
void SDC0_ISR(void)
{
    SDC_HandleIRQ(SDC0);
}

/*
Name:       SDC_HandleIRQ
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
void SDC1_ISR(void)
{
    SDC_HandleIRQ(SDC1);
}

/*
Name:       SDC0_DMACB
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
static void SDC0_DMACB(void)
{
    gSDMMCCtrl[SDC0].TransInfo.TransLen = gSDMMCCtrl[SDC0].TransInfo.NeedLen;
}

/*
Name:       SDC1_DMACB
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
static void SDC1_DMACB(void)
{
    gSDMMCCtrl[SDC1].TransInfo.TransLen = gSDMMCCtrl[SDC1].TransInfo.NeedLen;
}

/*
Name:       SDC_ResetFIFO
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
static int32 SDC_ResetFIFO(pSDC_REG_T        pReg)
{
    volatile uint32 value = 0;
    int32 timeOut = 0;

    value = pReg->SDMMC_STATUS;
    if (!(value & FIFO_EMPTY))
    {
        pReg->SDMMC_CTRL |= FIFO_RESET;
        timeOut = 5000;
        while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET)) && (timeOut > 0))
        {
            timeOut--;
        }

        if(timeOut == 0)
            return SDC_SDC_ERROR;
    }

    return SDC_SUCCESS;
}


/*
Name:       SDC_PrepareWrite
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_WRITE_CODE_
static int32 SDC_PrepareWrite(SDMMC_PORT_E        SDCPort, void *pDataBuf, uint32 DataLen)
{
	#if EN_SD_DMA
    pSDC_REG_T       pReg = pSDCReg(SDCPort);
 	#endif
	volatile uint32 *pFIFO = pSDCFIFOADDR(SDCPort);
    //指针用32 bit的uint32指针，就可以满足SDMMC FIFO要求的32 bits对齐的要求了，这样就算你的数据没有4字节对齐，
    //也会因为用了uint32指针，每次向FIFO操作是4字节对齐的。
    uint32          *pBuf = (uint32 *)pDataBuf;
    //下面对于dataLen的操作，考虑到SDMMC控制器要求32bit对齐的要求，所以看起来对dataLen的操作比较麻烦
    //SDMMC控制器要求32bit对齐，如:要传输13 byte个数据，实际传入FIFO(写时)或从FIFO传出(读时)的数据必须要16个,
    uint32           count = (DataLen >> 2) + ((DataLen & 0x3) ? 1:0);   //用32bit指针来传，因此数据长度要除4
    uint32           len;
    pSDC_TRANS_INFO   pTransInfo = &gSDMMCCtrl[SDCPort].TransInfo;

    pTransInfo->NeedLen = count;
    pTransInfo->pBuf = (uint32 *)pDataBuf;

    #if EN_SD_DMA
    //写之前clean cache
    //SDA_CleanCache(pDataBuf, dataLen);
    if (count <= FIFO_DEPTH)
    {
        len = count;
        SDC_WriteData(pFIFO, pBuf, len);
        pTransInfo->TransLen = count;
    }
    else
    {
        pTransInfo->TransLen = 0;
        if(!SDA_DMAStart(SDCPort, (uint32)pFIFO, (uint32)pBuf, count, 1, pSDC_DMACB(SDCPort)))
        {
            //Assert(0, "_PrepareForWriteData:DMA busy\n", 0);
            return SDC_DMA_BUSY;
        }
        SDC_EnanbleDMA(pReg);
    }
    #else
    //if write, fill FIFO to full before start
    count = (count > FIFO_DEPTH)? FIFO_DEPTH : count;
    len = count;
    SDC_WriteData(pFIFO, pBuf, len);
    pTransInfo->TransLen = count;
    #endif

    #if EN_SD_BUSY_INT
    //pReg->SDMMC_INTMASK |= BDONE_INT;
    //在数据传输完成中断中打开，避免在传输过程中被多次中断
    //pSDC->EnBusyInt = 1;
    #endif

    return SDC_SUCCESS;
}

/*
Name:       SDC_PrepareRead
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
static int32 SDC_PrepareRead(SDMMC_PORT_E         SDCPort, void *pDataBuf, uint32 DataLen)
{
    pSDC_TRANS_INFO      pTransInfo = &gSDMMCCtrl[SDCPort].TransInfo;

    #if EN_SD_DMA
    uint32          count = 0;
    pSDC_REG_T      pReg = pSDCReg(SDCPort);

    //下面对于dataLen的操作，考虑到SDMMC控制器要求32bit对齐的要求，所以看起来对dataLen的操作比较麻烦
    //SDMMC控制器要求32bit对齐，如:要传输13 byte个数据，实际传入FIFO(写时)或从FIFO传出(读时)的数据必须要16个
    count = (DataLen >> 2) + ((DataLen & 0x3) ? 1:0);
    if (count > (RX_WMARK+1))  //当总的数据长度小于等于RX_WMARK+1时，不会向DMA发起请求(这个跟datasheet说的不大一样)
    {
        if(!SDA_DMAStart(SDCPort, (uint32)pDataBuf, (uint32)pSDCFIFOADDR(SDCPort), (DataLen >> 2), 0, pSDC_DMACB(SDCPort)))
        {
            //Assert(0, "_PrepareForReadData:DMA busy\n", 0);
            return SDC_DMA_BUSY;
        }
        SDC_EnanbleDMA(pReg);
    }
    #endif

    pTransInfo->NeedLen = (DataLen >> 2);
    pTransInfo->TransLen = 0;
    pTransInfo->pBuf = (uint32 *)pDataBuf;

    return SDC_SUCCESS;
}

/*
Name:       SDC_ReadRemain
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
static int32 SDC_ReadRemain(SDMMC_PORT_E           SDCPort, uint32 OriginalLen, void *pDataBuf)
{
    volatile uint32  value = 0;
    volatile uint32 *pFIFO = pSDCFIFOADDR(SDCPort);
    pSDC_REG_T       pReg = pSDCReg(SDCPort);
    uint32           i = 0;
    //指针用32 bit的uint32指针，就可以满足SDMMC FIFO要求的32 bits对齐的要求了，这样就算你的数据没有4字节对齐，
    //也会因为用了uint32指针，每次向FIFO操作是4字节对齐的。
    uint32          *pBuf = (uint32 *)pDataBuf;
    uint8           *pByteBuf = (uint8 *)pDataBuf;
    uint32           lastData = 0;
    //下面对于dataLen的操作，考虑到SDMMC控制器要求32bit对齐的要求，所以看起来对dataLen的操作比较麻烦
    //SDMMC控制器要求32bit对齐，如:要传输13 byte个数据，实际传入FIFO(写时)或从FIFO传出(读时)的数据必须要16个
    uint32           count = (OriginalLen >> 2) + ((OriginalLen & 0x3) ? 1:0);   //用32bit指针来传，因此数据长度要除4
    pSDC_TRANS_INFO      pTransInfo = &gSDMMCCtrl[SDCPort].TransInfo;

    value = pReg->SDMMC_STATUS;
    if (value & FIFO_EMPTY)
    {
        return SDC_SUCCESS;
    }

    #if EN_SD_DMA
    //DMA传输时，只有dataLen/4小于等于RX_WMARK+1,或者dataLen没4字节对齐的，才会有剩余数据
    if (count <= (RX_WMARK+1))
    {
        i = 0;
        while ((i<(OriginalLen >> 2))&&(!(value & FIFO_EMPTY)))
        {
            pBuf[i++] = *pFIFO;
            value = pReg->SDMMC_STATUS;
        }
        pTransInfo->TransLen += i;
    }
    #else
    //中断传输时，当dataLen/4小于等于RX_WMARK+1,或者dataLen没4字节对齐的，或者最后剩余的数据达不到RX_WMARK+1都会有剩余数据
    while ((pTransInfo->TransLen < pTransInfo->NeedLen) && (!(value & FIFO_EMPTY)))
    {
        pBuf[pTransInfo->TransLen++] = *pFIFO;
        value = pReg->SDMMC_STATUS;
    }
    #endif

    if (count > (OriginalLen >> 2))
    {
        if(value & FIFO_EMPTY)
        {
            return SDC_SDC_ERROR;
        }
        lastData = *pFIFO;
        //填充剩余的1-3个字节
        for (i=0; i<(OriginalLen & 0x3); i++)
        {
            pByteBuf[(OriginalLen & 0xFFFFFFFC) + i] = (uint8)((lastData >> (i << 3)) & 0xFF);  //只考虑CPU为小端，little-endian
        }
    }

    return SDC_SUCCESS;
}


/*
Name:       SDC_UpdateFreq
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
int32 SDC_UpdateFreq(SDMMC_PORT_E SDCPort, uint32 FreqKHz)
{
    uint32          SrcClkDiv;
    uint32          ClkInDiv;
    uint32          SrcFreq;
    uint32          ClkInFreq;
    uint32          SrcClkSel = 1;               //默认选择PLL时钟
    int32           ret = SDC_SUCCESS;
    pSDC_REG_T      pReg = pSDCReg(SDCPort);
    pSDMMC_CTRL     pSDC = &gSDMMCCtrl[SDCPort];

    if (FreqKHz == 0 || FreqKHz > MAX_CLKIN_FREQ)//频率不能为0，否则后面会出现除数为0
    {
        return SDC_PARAM_ERROR;
    }

    SrcFreq = SDA_GetSrcClkFreq(SDCPort, 24000, &SrcClkSel);
    //DEBUG("SDC SrcFreq=%dKHz, FreqKHz=%dKHz\n", SrcFreq, FreqKHz);
    ret = SDC_SetClock(SDCPort, FALSE);
    if(ret != SDC_SUCCESS)
    {
        return ret;
    }

    //先保证SDMMC控制器工作cclk_in不超过52MHz，否则后面设置寄存器会跑飞掉
    SrcClkDiv = (SrcFreq+FreqKHz-1)/FreqKHz;
    if ((SrcClkDiv & 0x1) && (SrcClkDiv != 1)) //SrcClkDiv 不能为奇数, 否则影响clk占空比
    {
        SrcClkDiv++;
    }

    if(SrcClkDiv > MAX_SCU_CLK_DIV)
    {
        SrcClkDiv = MAX_SCU_CLK_DIV;
    }

    ClkInFreq = SrcFreq/SrcClkDiv;
    ClkInDiv = (ClkInFreq+FreqKHz-1)/FreqKHz;
    if(ClkInDiv > 510)
    {
        return SDC_SDC_ERROR;
    }

    if (((ClkInDiv & 0x1) == 1) && (ClkInDiv != 1))
    {
        ClkInDiv++;  //除了1分频，保证是偶数倍
    }

    pReg->SDMMC_CLKDIV = (ClkInDiv >> 1);
    if (SDC_SUCCESS != SDC_StartCmd(pReg, (START_CMD | UPDATE_CLOCK | WAIT_PREV)))
    {
        return SDC_SDC_ERROR;
    }

    SDA_SetSrcClkDiv(SDCPort, SrcClkDiv, SrcClkSel);

    pSDC->CardFreq = FreqKHz;
    pSDC->UpdatedFreq = FALSE;

    return SDC_SetClock(SDCPort, TRUE);
}

/*
Name:       SDC_SetPower
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
int32 SDC_SetPower(SDMMC_PORT_E SDCPort, bool enable)
{
    pSDC_REG_T   pReg = pSDCReg(SDCPort);

    //SDA_SetPower(SDCPort, enable);
    if (enable)
    {
        SDA_SetSrcClk(SDCPort, enable);
        pReg->SDMMC_PWREN = 1;   // power enable
    }
    else
    {

        pReg->SDMMC_PWREN = 0;   // power disable
        SDA_SetSrcClk(SDCPort, enable);
    }

    return SDC_SUCCESS;
}

/*
Name:       SDC_GetBusWidth
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
int32  SDC_GetBusWidth(SDMMC_PORT_E SDCPort, HOST_BUS_WIDTH *pWidth)
{
    *pWidth = gSDMMCCtrl[SDCPort].ConfigWidth;
    return SDC_SUCCESS;
}

/*
Name:       SDC_SetBusWidth
Desc:       改变SDMMC 控制器的数据线宽度
            应该先发送完ACMD6后，紧跟调用该函数改变控制器的数据线宽度
Param:
Return:
Global:
Note:       这个函数只改变SDMMC控制器内部的数据线宽度设置，对于卡还是
            需要发送ACMD6来进行设置的。
            应该先发送完ACMD6后，紧跟调用该函数改变控制器的数据线宽度
            数据传输过程中不建议改变bus width
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
int32 SDC_SetBusWidth(SDMMC_PORT_E SDCPort, HOST_BUS_WIDTH width)
{
    uint32       value = 0;
    pSDC_REG_T   pReg = pSDCReg(SDCPort);

    switch (width)
    {
        case BUS_WIDTH_1_BIT:
            value = BUS_1_BIT;
            break;
        case BUS_WIDTH_4_BIT:
            value = BUS_4_BIT;
            break;
        case BUS_WIDTH_8_BIT:    //现在IC不支持8bit
            value = BUS_8_BIT;
            break;
        default:
            return SDC_PARAM_ERROR;
    }

    SDA_SetIOMux(SDCPort, width);
    pReg->SDMMC_CTYPE = value;
    gSDMMCCtrl[SDCPort].BusWidth = width;

    return SDC_SUCCESS;
}

/*
Name:       SDC_Reset
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
int32 SDC_Reset(SDMMC_PORT_E SDCPort)
{
    volatile uint32  value = 0;
    pSDC_REG_T       pReg = pSDCReg(SDCPort);
    int32            timeOut = 0;
    uint32           SrcFreq = 24000;//SDA_GetSrcClkFreq();

    /* reset SDMMC IP */
    SDA_SetSrcClk(SDCPort, TRUE);
    SDA_Delay(10);
    /* reset */

    pReg->SDMMC_CTRL = (FIFO_RESET | SDC_RESET | DMA_RESET);
    timeOut = 1000;
    while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET | SDC_RESET | DMA_RESET)) && (timeOut > 0))
    {
        SDA_Delay(1);
        timeOut--;
    }

    if(timeOut == 0)
    {
        SDA_SetSrcClk(SDCPort, FALSE);
        return SDC_SDC_ERROR;
    }
    /* config FIFO */
    pReg->SDMMC_FIFOTH = (SD_MSIZE_16 | (RX_WMARK << RX_WMARK_SHIFT) | (TX_WMARK << TX_WMARK_SHIFT));
    pReg->SDMMC_CTYPE = BUS_1_BIT;
    pReg->SDMMC_CLKSRC = CLK_DIV_SRC_0;
    /* config debounce */
    pReg->SDMMC_DEBNCE = (DEBOUNCE_TIME*SrcFreq)&0xFFFFFF;
    pReg->SDMMC_TMOUT = 0xFFFFFF40;
    /* config interrupt */
    pReg->SDMMC_RINISTS = 0xFFFFFFFF;
    value = 0;

    #if !(EN_SD_DMA)
    value |= (RXDR_INT | TXDR_INT);
    #endif

    #if EN_SD_INT
    value |= (SBE_INT | FRUN_INT | DTO_INT | CD_INT /*| VSWTCH_INT*/);
    //value |= (SBE_INT | FRUN_INT /* | DTO_INT | CD_INT| VSWTCH_INT*/);
    #else
    value |= (FRUN_INT);
    #endif

    //if (SD_CONTROLLER_DET == gSDCDevice[cardId].pConfig->DetMode)
    //{
    //    value |= CDT_INT;
    //}

    pReg->SDMMC_INTMASK = value;

    SDA_RegISR(SDCPort, pSDC_ISR(SDCPort));
    SDA_EnableIRQ(SDCPort);

    pReg->SDMMC_CTRL = ENABLE_INT;

    return SDC_SUCCESS;
}

/*
Name:       MemBoot
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
int32 SDC_Init(SDMMC_PORT_E SDCPort, SDMMC_PORT_FUNCTION func)
{
    pSDMMC_CTRL      pSDC = &gSDMMCCtrl[SDCPort];

    memset(pSDC, 0, sizeof(SDMMC_CTRL));

    SDA_Init(SDCPort, func);

    pSDC->BusWidth = BUS_WIDTH_1_BIT;
    pSDC->ConfigWidth = (HOST_BUS_WIDTH)((func==SD_CARD_FUNC)? CONFIG_SD_BUS_WIDTH : CONFIG_EMMC_BUS_WIDTH);

    pSDC->pReg   = pSDCReg(SDCPort);
    pSDC->event = SDA_CreateEvent((uint8)SDCPort);
    //pSDC->mutex  = SDOAM_CreateMutex((uint8)nSDCPort);

    /* enable SDMMC port */
    SDA_SetIOMux(SDCPort, BUS_WIDTH_1_BIT);

    SDC_Reset(SDCPort);

    if(SDC_CheckCard(SDCPort, MD_EVENT_PRESENT))
    {
        ;//SDM_NotifyAll((int32)SDCPort, 1);
    }
    else
    {
        SDC_SetClock(SDCPort, FALSE);
        SDA_SetSrcClk(SDCPort, FALSE);
    }

    return SDC_SUCCESS;

}

/*
Name:       SDC_DeInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
int32 SDC_DeInit(SDMMC_PORT_E SDCPort)
{
    return SDC_SUCCESS;
}

/*
Name:       SDC_UpdateClkHook
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_CODE_
void SDC_UpdateClkHook(uint32 SDCPort)
{
    pSDMMC_CTRL     pSDC = &gSDMMCCtrl[SDCPort];
    pSDC->UpdatedFreq = TRUE;
}

/*
Name:       SDC_Request
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
int32 SDC_Request(SDMMC_PORT_E SDCPort,
                        uint32  cmd,
                        uint32  CmdArg,
                        uint32 *RespBuf,
                        uint32  BlkSize,
                        uint32  DataLen,
                        void   *pDataBuf)
{
    volatile uint32 value = 0;
    int32           ret = SDC_SUCCESS;
    pSDMMC_CTRL     pSDC= &gSDMMCCtrl[SDCPort];
    pSDC_REG_T      pReg = pSDC->pReg;

    #ifndef ASYNC_WAIT_IO   //同步等待
    if (cmd & WAIT_PREV)
    {
        if (pReg->SDMMC_STATUS & DATA_BUSY)
        {
            ret = SDC_BUSY_TIMEOUT;
            goto END_REQ;
        }
    }
    #else                   //异步等待
    if (cmd & WAIT_PREV)
    {
        ret = SDC_WaitBusy(SDCPort);
        if (ret != SDC_SUCCESS)
        {
            goto END_REQ;
        }
    }
    #endif

    value = pReg->SDMMC_CMD;
    if(value & START_CMD)
    {
       ret = SDC_SDC_ERROR;
       goto END_REQ;
    }

    #if 1
    if(pSDC->UpdatedFreq)
    {
        ret = SDC_UpdateFreq(SDCPort, pSDC->CardFreq);
        if (ret != SDC_SUCCESS)
        {
            goto END_REQ;
        }
    }
    #endif

    //ensure the card is not busy due to any previous data transfer command
    if (!(cmd & STOP_CMD))
    {
        //清除FIFO
        SDC_ResetFIFO(pReg);
    }

    if (cmd & DATA_EXPECT)
    {
        if((cmd & SD_OP_MASK) == SD_WRITE_OP)
        {
            SDC_PrepareWrite(SDCPort, pDataBuf, DataLen);
        }
        else
        {
            SDC_PrepareRead(SDCPort, pDataBuf, DataLen);
        }

        pReg->SDMMC_BLKSIZ = BlkSize;
        pReg->SDMMC_BYTCNT = DataLen;  //这个寄存器的长度一定要设置为需要的长度，不用考虑SDMMC控制器的32bit对齐
    }

    #if EN_SD_BUSY_INT
    if (cmd & RSP_BUSY)
        pReg->SDMMC_INTMASK |= BDONE_INT;
    #endif

    pReg->SDMMC_CMDARG = CmdArg;
    //set start bit,CMD/CMDARG/BYTCNT/BLKSIZ/CLKDIV/CLKENA/CLKSRC/TMOUT/CTYPE were locked
    if (SDC_SUCCESS != SDC_StartCmd(pReg, (cmd & ~(RSP_BUSY)) | START_CMD | USE_HOLD_REG))
    {
        ret = SDC_SDC_ERROR;
        goto END_REQ;
    }

    #if EN_SD_INT
    {
        SDOAM_GetEvent(pSDC->event, CD_EVENT,  1 * 1000000);
    }
    #else
    SDC_WaitBitSet(pReg->SDMMC_RINISTS & CD_INT);
    pReg->SDMMC_RINISTS = CD_INT;
    #endif

    if (cmd & STOP_CMD)
    {
        //随着STOP命令，可能还会有数据进来(如mutiple read时)，要再清除FIFO
        SDC_ResetFIFO(pReg);
    }

    //check response error, or response crc error, or response timeout
    value = pReg->SDMMC_RINISTS;
    if (value & RTO_INT)
    {
        pReg->SDMMC_RINISTS = 0xFFFFFFFF;  //如果response timeout，数据传输 被中止，清除所有中断
        #if EN_SD_DMA
        value = pReg->SDMMC_CTRL;
        if (value & ENABLE_DMA)
        {
            SDA_DMAStop(SDCPort);
            SDC_DisableDMA(pReg);
        }
        #endif
        ret = SDC_RESP_TIMEOUT;
        goto END_REQ;
    }

    if (cmd & DATA_EXPECT)
    {
        #if EN_SD_INT
        {
            SDOAM_GetEvent(pSDC->event, DTO_EVENT, 5 * 1000000);
        }
        #else
        SDC_WaitBitSet(pReg->SDMMC_RINISTS & (DTO_INT | SBE_INT));
        pReg->SDMMC_RINISTS = DTO_INT;
        #endif

        #if EN_SD_DMA
        value = pReg->SDMMC_CTRL;
        if (value & ENABLE_DMA)
        {
            SDA_DMAStop(SDCPort);
            SDC_DisableDMA(pReg);
        }
        #endif

        value = pReg->SDMMC_RINISTS;
        if((cmd & SD_OP_MASK) == SD_WRITE_OP)
        {
            if (value & DCRC_INT)
            {
                ret = SDC_DATA_CRC_ERROR;
            }
            else if (value & EBE_INT)
            {
                ret = SDC_END_BIT_ERROR;
            }
            else
            {
                #ifndef ASYNC_WAIT_IO   //同步等待
                ret = SDC_WaitBusy(SDCPort);
                if (ret != SDC_SUCCESS)
                {
                    goto END_REQ;
                }
                #endif
                ret = SDC_SUCCESS;
            }
        }
        else
        {
            #if EN_SD_DMA
            //读完成后要flush cache
            //SDA_FlushCache(pDataBuf, DataLen);
            #endif
            if(value & (SBE_INT | EBE_INT | DRTO_INT | DCRC_INT))
            {
                if (value & SBE_INT)
                {
                    //SDC_HandleSBE(nSDCPort);
                    ret = SDC_START_BIT_ERROR;
                }
                else if (value & EBE_INT)
                {
                    if((cmd & SD_CMD_MASK) == SD_CMD14)
                    {
                        ret = SDC_ReadRemain(SDCPort, DataLen, pDataBuf);
                    }
                    else
                    {
                        ret = SDC_END_BIT_ERROR;
                    }
                }
                else if (value & DRTO_INT)
                {
                    ret = SDC_DATA_READ_TIMEOUT;
                }
                else if (value & DCRC_INT)
                {
                    ret = SDC_DATA_CRC_ERROR;
                }
            }
            else
            {
                ret = SDC_ReadRemain(SDCPort, DataLen, pDataBuf);
            }
        }
    }

    value = pReg->SDMMC_RINISTS;
    pReg->SDMMC_RINISTS = 0xFFFFFFFF;

    if((cmd & SD_OP_MASK) == SD_READ_OP)
    {
        uint32 status = pReg->SDMMC_RESP0;
        if (status & (0x1 << 19))
        {
            DEBUG("SDC_Req cmd%d,Arg:0x%x,status:0x%x\n", cmd & 0x3f, CmdArg, status);
        }
    }

    if (ret == SDC_SUCCESS)
    {
        if (cmd & RSP_BUSY) //R1b
        {
            ret = SDC_WaitBusy(SDCPort);
            if (ret != SDC_SUCCESS)
            {
                goto END_REQ;
            }
        }

        //if need, get response
        if ((cmd & R_EXPECT) && (RespBuf != NULL))
        {
            RespBuf[0] = pReg->SDMMC_RESP0;
            if (cmd & LONG_R)
            {
                RespBuf[1] = pReg->SDMMC_RESP1;
                RespBuf[2] = pReg->SDMMC_RESP2;
                RespBuf[3] = pReg->SDMMC_RESP3;
            }
        }

        if(pSDC->CardFreq > 400)
        {
            if (value & RCRC_INT)
            {
                ret = SDC_RESP_CRC_ERROR;
                goto END_REQ;
            }
            if (value & RE_INT)
            {
                ret = SDC_RESP_ERROR;
                goto END_REQ;
            }
        }
    }

END_REQ:
    if (ret != SDC_SUCCESS)
    {
        DEBUG("SDC_Req cmd%d,CmdArg:0x%x,ERR:0x%x\n", cmd & 0x3f, CmdArg, ret);
    }
    return ret;

}

/*
Name:       SDC_CheckCard
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
bool SDC_CheckCard(SDMMC_PORT_E SDCPort, uint32 event)
{
    bool present, ret;
    pSDMMC_CTRL pSDC = &gSDMMCCtrl[SDCPort];

    present = SDA_CheckCard(SDCPort);

    if (present != pSDC->present)
    {
        pSDC->changed = 1;
        pSDC->present = present;
        //SDC_NotifyClient(pSDC, EV_HOTPLUG, present);
    }

    if (event == MD_EVENT_PRESENT)
    {
        ret = present;
    }
    else if (event == MD_EVENT_CHANGE)//MD_EVENT_CHANGE
    {
        ret = pSDC->changed;
        pSDC->changed = 0;
    }
    else
    {
        ret = FALSE;
    }
    return ret;
}


/*
Name:       SDC_ResetEmmc
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
int32 SDC_ResetEmmc(void)
{
    pSDC_REG_T  pReg = pSDCReg(SDM_EMMC_ID); //sdc1 port use for emmc function

    pReg->SDMMC_PWREN = 0;          //目前主控的POWER_EN 连到EMMC的 RST_n
    SDA_Delay(10);
    pReg->SDMMC_PWREN = 1;
    SDA_Delay(500);         //200US 后才能发 CMD1

    return SDC_SUCCESS;
}
#endif
