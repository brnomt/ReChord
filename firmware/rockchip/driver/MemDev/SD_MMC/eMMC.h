/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: EMMC.h
Desc: 

Author: 
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

#ifndef _EMMC_H
#define _EMMC_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/

#define     EMMC_PROT_MAGIC        0x444e414e  //NAND

/*------------------------------ Global Typedefs -----------------------------*/


/*----------------------------- External Variables ---------------------------*/
;    


/*------------------------- Global Function Prototypes -----------------------*/
extern uint32 EmmcInit(void);
extern uint32 EmmcDeInit(void);

extern uint32 EmmcBlkErase(uint32 blk);

extern uint32 EmmcRead(uint8 lun, uint32 sec, uint32 nSec, void *pBuf);
extern uint32 EmmcWrite(uint8 lun, uint32 sec, uint32 nSec, void *pBuf);

extern uint32 EmmcReadIDB(uint32 sec, uint32 nSec, uint8 *pData);
extern uint32 EmmcWriteIDB(uint32 sec, uint32 nSec, uint8 *pData);

extern uint32 EmmcReadOTP(uint32 ByteAddr, uint32 ByteSize, uint8 *pData);
extern uint32 EmmcWriteOTP(uint32 ByteAddr, uint32 ByteSize, uint8 *pData);

extern uint32 EmmcGetInfo(uint8 lun, pMEMDEV_INFO pDevInfo);

extern void EmmcSysProtSet(uint32 magic);

extern int32 EmmcGetProductSn(void *pSn);

#endif
