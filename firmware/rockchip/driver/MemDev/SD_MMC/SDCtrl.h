/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SDCtrl.h
Desc: 

Author: 
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

#ifndef _SDCTRL_H
#define _SDCTRL_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/

#define SDC0_ADDR         (SDMMC_BASE)
#define SDC1_ADDR         (EMMC_BASE)
//#define SDC2_ADDR         (EMMC_BASE_ADDR)

#define SDC0_FIFO_ADDR    (SDC0_ADDR + 0x200)
#define SDC1_FIFO_ADDR    (SDC1_ADDR + 0x200)
//#define SDC2_FIFO_ADDR    (EMMC_BASE_ADDR+ 0x200)

#define FIFO_DEPTH        (0x20)       //FIFO depth = 32 word
#define RX_WMARK_SHIFT    (16)
#define TX_WMARK_SHIFT    (0)

/***************************************************************/
//可配置的参数
/***************************************************************/
/* FIFO watermark */
#define RX_WMARK          (FIFO_DEPTH/2-1)      //RX watermark level set to 127
#define TX_WMARK          (FIFO_DEPTH/2)       //TX watermark level set to  128

/***************************************************************/
//不可配置的参数
/***************************************************************/
/* SDMMC Control Register */
#define ENABLE_DMA        (1 << 5)     //Enable DMA transfer mode
#define ENABLE_INT        (1 << 4)     //Enable interrupt
#define DMA_RESET        (1 << 2)     //FIFO reset
#define FIFO_RESET        (1 << 1)     //FIFO reset
#define SDC_RESET         (1 << 0)     //controller reset

/* Power Enable Register */
#define POWER_ENABLE      (1 << 0)     //Power enable

/* SDMMC Clock source Register */
#define CLK_DIV_SRC_0     (0x0)        //clock divider 0 selected
#define CLK_DIV_SRC_1     (0x1)        //clock divider 1 selected
#define CLK_DIV_SRC_2     (0x2)        //clock divider 2 selected
#define CLK_DIV_SRC_3     (0x3)        //clock divider 3 selected

/* Clock Enable Register */
#define CCLK_LOW_POWER    (1 << 16)    //Low-power control for SD/MMC card clock
#define NO_CCLK_LOW_POWER (0 << 16)    //low-power mode disabled
#define CCLK_ENABLE       (1 << 0)     //clock enable control for SD/MMC card clock
#define CCLK_DISABLE      (0 << 0)     //clock disabled

/* Card Type Register */
#define BUS_1_BIT         (0x0)
#define BUS_4_BIT         (0x1)
#define BUS_8_BIT         (0x10000)

/* interrupt mask bit */
#if !(EN_SD_BUSY_INT)
#define SDIO_INT          (1 << 16)    //SDIO interrupt
#else
#define SDIO_INT          (1 << 24)    //SDIO interrupt            
#endif
#define BDONE_INT          (1 << 16)   //busy Done interrupt

#define EBE_INT           (1 << 15)    //End Bit Error(read)/Write no CRC
#define ACD_INT           (1 << 14)    //Auto Command Done
#define SBE_INT           (1 << 13)    //Start Bit Error
#define HLE_INT           (1 << 12)    //Hardware Locked Write Error
#define FRUN_INT          (1 << 11)    //FIFO Underrun/Overrun Error
#define HTO_INT           (1 << 10)    //Data Starvation by Host Timeout
#define VSWTCH_INT        (1 << 10)    //Volt Switch interrupt
#define DRTO_INT          (1 << 9)     //Data Read TimeOut
#define RTO_INT           (1 << 8)     //Response TimeOut
#define DCRC_INT          (1 << 7)     //Data CRC Error
#define RCRC_INT          (1 << 6)     //Response CRC Error
#define RXDR_INT          (1 << 5)     //Receive FIFO Data Request
#define TXDR_INT          (1 << 4)     //Transmit FIFO Data Request
#define DTO_INT           (1 << 3)     //Data Transfer Over
#define CD_INT            (1 << 2)     //Command Done
#define RE_INT            (1 << 1)     //Response Error
#define CDT_INT           (1 << 0)     //Card Detect

/* Command Register */
#define START_CMD           (0x1U << 31) //start command
#define USE_HOLD_REG        (1 << 29)
#define VOLT_SWITCH         (1 << 28)           
#define BOOT_MODE           (1 << 27)
#define DISABLE_BOOT        (1 << 26)
#define EXPECT_BOOT_ACK     (1 << 25)
#define ENABLE_BOOT         (1 << 24)
#define CCS_EXPECTED        (1 << 23)
#define READ_CEATA          (1 << 22)

#define UPDATE_CLOCK        (1 << 21)    //update clock register only
#define SEND_INIT           (1 << 15)    //send initialization sequence
#define STOP_CMD            (1 << 14)    //stop abort command
#define NO_WAIT_PREV        (0 << 13)    //not wait previous data transfer complete, send command at once
#define WAIT_PREV           (1 << 13)    //wait previous data transfer complete
#define AUTO_STOP           (1 << 12)    //send auto stop command at end of data transfer
#define BLOCK_TRANS         (0 << 11)    //block data transfer command
#define STREAM_TRANS        (1 << 11)    //stream data transfer command
#define READ_CARD           (0 << 10)    //read from card
#define WRITE_CARD          (1 << 10)    //write to card
#define NOCARE_RW           (0 << 10)    //not care read or write
#define NO_DATA_EXPECT      (0 << 9)     //no data transfer expected
#define DATA_EXPECT         (1 << 9)     //data transfer expected
#define NO_CHECK_R_CRC      (0 << 8)     //do not check response crc
#define CHECK_R_CRC         (1 << 8)     //check response crc
#define NOCARE_R_CRC        CHECK_R_CRC  //not care response crc
#define SHORT_R             (0 << 7)     //short response expected from card
#define LONG_R              (1 << 7)     //long response expected from card
#define NOCARE_R            SHORT_R      //not care response length
#define NO_R_EXPECT         (0 << 6)     //no response expected from card
#define R_EXPECT            (1 << 6)     //response expected from card

/* SDMMC status Register */
#define DATA_BUSY           (1 << 9)     //Card busy
#define FIFO_FULL           (1 << 3)     //FIFO is full status
#define FIFO_EMPTY          (1 << 2)     //FIFO is empty status

/* SDMMC FIFO Register */
#define SD_MSIZE_1        (0x0 << 28)  //DW_DMA_Multiple_Transaction_Size
#define SD_MSIZE_4        (0x1 << 28)
#define SD_MSIZE_8        (0x1 << 28)
#define SD_MSIZE_16       (0x3 << 28)
#define SD_MSIZE_32       (0x4 << 28)
#define SD_MSIZE_64       (0x5 << 28)
#define SD_MSIZE_128      (0x6 << 28)
#define SD_MSIZE_256      (0x7 << 28)


/* Card detect Register */
#define NO_CARD_DETECT    (1 << 0)     //Card detect

/* Write Protect Register */
#define WRITE_PROTECT     (1 << 0)     //write protect, 1 represent write protection

/* SDC return value */
#define SDC_SUCCESS              SDM_SUCCESS             //操作成功
#define SDC_FALSE                SDM_FALSE               //操作失败
#define SDC_CARD_NOTPRESENT      SDM_CARD_NOTPRESENT     //卡不存在或被拔出
#define SDC_PARAM_ERROR          SDM_PARAM_ERROR         //参数错误
#define SDC_RESP_ERROR           SDM_RESP_ERROR          //卡的回复错误
#define SDC_RESP_CRC_ERROR       SDM_RESP_CRC_ERROR      //卡的回复CRC校验错误
#define SDC_RESP_TIMEOUT         SDM_RESP_TIMEOUT        //卡的回复timeout
#define SDC_DATA_CRC_ERROR       SDM_DATA_CRC_ERROR      //卡的数据CRC错误
#define SDC_DATA_READ_TIMEOUT    SDM_DATA_READ_TIMEOUT   //读卡的数据timeout
#define SDC_END_BIT_ERROR        SDM_END_BIT_ERROR       //数据结束位错误
#define SDC_START_BIT_ERROR      SDM_START_BIT_ERROR     //数据起始位错误
#define SDC_BUSY_TIMEOUT         SDM_BUSY_TIMEOUT        //busy时间太久了
#define SDC_DMA_BUSY             SDM_DMA_BUSY            //dma busy
#define SDC_SDC_ERROR            SDM_ERROR               //SDMMC host controller error
#define SDC_SDC_BUSY             SDM_SDC_BUSY            //SDMMC host controller busy
#define SDC_VSWTCH_ERROR         SDM_SDC_BUSY 
#define SDC_NO_EVENT             SDM_NO_EVENT

/*------------------------------ Global Typedefs -----------------------------*/


/*----------------------------- External Variables ---------------------------*/
extern int32 SDC_Init(SDMMC_PORT_E SDCPort, SDMMC_PORT_FUNCTION func);    
extern bool SDC_CheckCard(SDMMC_PORT_E SDCPort, uint32 event);
extern int32 SDC_Reset(SDMMC_PORT_E SDCPort);
extern int32 SDC_SetBusWidth(SDMMC_PORT_E SDCPort, HOST_BUS_WIDTH width);
extern int32  SDC_GetBusWidth(SDMMC_PORT_E SDCPort, HOST_BUS_WIDTH *pWidth);
extern int32 SDC_SetPower(SDMMC_PORT_E SDCPort, bool enable);
extern int32 SDC_UpdateFreq(SDMMC_PORT_E SDCPort, uint32 FreqKHz);
extern void SDC0_ISR(void);
extern int32 SDC_ResetEmmc(void);    
extern  int32 SDC_WaitBusy(SDMMC_PORT_E SDCPort);
extern  void SDC_UpdateClkHook(uint32 SDCPort);

extern int32 SDC_Request(SDMMC_PORT_E SDCPort,
                        uint32  cmd,
                        uint32  CmdArg,
                        uint32 *RespBuf,
                        uint32  BlkSize,
                        uint32  DataLen,
                        void   *pDataBuf);

#define SDC_SendCmd(SDCPort, cmd, CmdArg, RespBuf)  \
                   SDC_Request(SDCPort, cmd, CmdArg, RespBuf, 0, 0, NULL);

/*------------------------- Global Function Prototypes -----------------------*/

#endif
