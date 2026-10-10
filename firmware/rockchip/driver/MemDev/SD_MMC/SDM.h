/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SDM.h
Desc: 

Author: 
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

#ifndef _SDM_H
#define _SDM_H


/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/

#define SDM_BLOCK_SIZE           (512*1024/512)

/* SDM return value */
#define SDM_SUCCESS              (0)                     //操作成功
#define SDM_FALSE                (0x1 << 0)              //操作失败
#define SDM_CARD_NOTPRESENT      (0x1 << 1)              //要操作的卡不在卡槽上
#define SDM_PARAM_ERROR          (0x1 << 2)              //参数有错
#define SDM_RESP_ERROR           (0x1 << 3)              //卡的回复错误
#define SDM_RESP_CRC_ERROR       (0x1 << 4)              //卡的回复CRC校验错误
#define SDM_RESP_TIMEOUT         (0x1 << 5)              //卡的回复timeout
#define SDM_DATA_CRC_ERROR       (0x1 << 6)              //卡的数据CRC错误
#define SDM_DATA_READ_TIMEOUT    (0x1 << 7)              //读卡的数据timeout
#define SDM_END_BIT_ERROR        (0x1 << 8)              //数据结束位错误
#define SDM_START_BIT_ERROR      (0x1 << 9)              //数据起始位错误
#define SDM_BUSY_TIMEOUT         (0x1 << 10)             //busy时间太久了
#define SDM_DMA_BUSY             (0x1 << 11)             //dma busy
#define SDM_ERROR                (0x1 << 12)             //SDMMC host controller error
#define SDM_VOLTAGE_NOT_SUPPORT  (0x1 << 13)             //卡的工作电压不在host的提供范围内，所以无法正常工作
#define SDM_FUNC_NOT_SUPPORT     (0x1 << 14)             //要求进行的操作，卡不支持
#define SDM_UNKNOWABLECARD       (0x1 << 15)             //不认识的卡
#define SDM_CARD_WRITE_PROT      (0x1 << 16)             //卡被写保护
#define SDM_CARD_LOCKED          (0x1 << 17)             //卡被锁住了
#define SDM_CARD_CLOSED          (0x1 << 18)             //卡已经被调用SDM_Close关闭了
#define SDM_SDC_BUSY             (0x1 << 19)             //控制器忙
#define SDM_VSWTCH_ERROR         (0x1 << 20)             //电压切换错误
#define SDM_NO_MEM               (0x1 << 21)             //内存不足
#define SDM_NO_EVENT             (0x1 << 22)             //等待的事件还没来

/* SDM IOCTRL cmd */
#define SDM_IOCTRL_REGISTER_CARD         (0x0)           //注册一张卡
#define SDM_IOCTRL_UNREGISTER_CARD       (0x1)           //注销一张卡
#define SDM_IOCTRL_SET_PASSWORD          (0x2)           //设置密码
#define SDM_IOCTRL_CLEAR_PASSWORD        (0x3)           //清除密码
#define SDM_IOCTRL_FORCE_ERASE_PASSWORD  (0x4)           //强制擦除密码，卡内的所有数据都将丢失
#define SDM_IOCTRL_LOCK_CARD             (0x5)           //锁卡，没有密码的卡锁不会成功
#define SDM_IOCTRL_UNLOCK_CARD           (0x6)           //解锁
#define SDM_IOCTR_GET_CAPABILITY         (0x7)           //获取容量
#define SDM_IOCTR_GET_PSN                (0x8)           //得到卡的Product serial number
#define SDM_IOCTR_IS_CARD_READY          (0x9)           //得到卡是否准备就绪，可用
#define SDM_IOCTR_FLUSH                  (0xA)           //对卡进行flush操作
#define SDM_IOCTR_GET_BOOT_CAPABILITY    (0xB)           //获取Boot partition容量
#define SDM_IOCTR_PARTITION_CONFIG       (0xC)           //配置EMMC 分区信息

#define SDM_INVALID_CARDID    (-1)           //无效的cardId
#define SDM_WIDE_BUS_MODE     (1 << 0)       //for debug
#define SDM_HIGH_SPEED_MODE   (1 << 1)       //for debug 

/*------------------------------ Global Typedefs -----------------------------*/

/* Card Operation Function */
typedef struct tagCARD_OPERATIONS
{
    //int32 (*open)(int32 cardId, uint32 mode);
    int (*read)(int32 CardId, uint32 Addr, uint32 BlkCnt, void *pBuf);
    int (*write)(int32 CardId, uint32 Addr, uint32 BlkCnt, void *pBuf);
    //int32 (*ioctrl)(int32 CardId, uint32 cmd, void *param);
    //int32 (*release)(int32 cardId);
}CARD_OPERATIONS, *pCARD_OPERATIONS;

/* Card Information */
typedef struct tagSDM_CARD_INFO
{
    int32            CardId;           //cardId，-1被认为是无效id
    uint32           type;             //Card type
    uint16           rca;              //Relative Card Address
    uint32           TranSpeed;       //卡的最大数据传输速度，也就是卡的最大工作频率，单位KHz
    uint32           capability;       //Card capability,单位block数，每个block为512字节
    uint32           BootSize;         //boot partition size,单位sector(512B)
    bool             WriteProt;        //Mechanical write Protect switch state,
    bool             bPassword;        //用于指示卡是否是有密码的卡, TRUE:have password, FALSE:no password              
    CARD_SPEC_VER_E  SpecVer;          //SD Specification Version
    uint16           EraseSize;        //this size is given as minimum number of write blocks that can be erased in a single erase command
    //uint32           WorkMode;         //for debug, record card work mode
    //int32            error;            //for debug，出错信息保存在这边
    /* operation function */
    struct tagCARD_OPERATIONS fops;    //SDM operation functions
    SDMMC_PORT_E     SDCPort;           //这张卡, 所在的控制器
    uint32           WorkMode; 

    void            *priv;

}SDM_CARD_INFO,*pSDM_CARD_INFO;

/*----------------------------- External Variables ---------------------------*/

/*------------------------- Global Function Prototypes -----------------------*/


/* Card type */
/*
#define UNKNOW_CARD             (0)         //无法识别或不支持的卡，不可用
#define SDIO                    (0x1 << 1)
#define SDHC                    (0x1 << 2)  //Ver2.00 High Capacity SD Memory Card
#define SD20                    (0x1 << 3)  //Ver2.00 Standard Capacity SD Memory Card
#define SD1X                    (0x1 << 4)  //Ver1.xx Standard Capacity SD Memory Card
#define MMC4                    (0x1 << 5)  //Ver4.xx MMC
#define MMC                     (0x1 << 6)  //Ver3.xx MMC
#define eMMC2G                  (0x1 << 7)  //Ver4.2 larter eMMC and densities greater than 2GB
#define SDXC                    (0x1 << 8)
#define UHS1                    (0x1 << 9)
*/

extern uint32 SDM_Init(int32 CardId, SDM_CARD_INFO *pCard, SDMMC_PORT_FUNCTION func);
extern void  SDM_DeInit(int32 CardId);
extern int32 SDM_Open(int32 CardId, uint32 type);
extern int32 SDM_Close(int32 CardId);
extern int32 SDM_Read(int32 CardId, uint32 Addr, uint32 BlkCnt, void *pBuf);
extern int32 SDM_Write(int32 CardId, uint32 Addr, uint32 BlkCnt, void *pBuf);
extern int32 SDM_IOCtrl(int32 CardId, uint32 cmd, uint32 param);
extern uint32 SDM_GetCapability(int32 CardId);
extern pSDM_CARD_INFO SDM_GetCardInfo(int32 CardId);
extern uint32 SDM_Probe(int32 CardId);

extern uint32 SDCardInit(void);
extern bool SDCardCheck(SDMMC_PORT_E SDCPort);

extern uint32 SDCardRead(uint8 lun, uint32 sec, uint32 nSec, void *pBuf);
extern uint32 SDCardWrite(uint8 lun, uint32 sec, uint32 nSec, void *pBuf);
extern uint32 SDCardGetInfo(uint8 lun, pMEMDEV_INFO pDevInfo);
extern bool SDCardChkEvent(uint32 event);
extern uint32 SDCardProbe(void);
extern uint32 SDCardInit(void);
extern uint32 SDCardDeInit(void);
#endif
