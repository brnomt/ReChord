/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SpiNor.h
Desc: 

Author: 
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

#ifndef _SPINOR_H
#define _SPINOR_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/


/*------------------------------ Global Typedefs -----------------------------*/


/*----------------------------- External Variables ---------------------------*/


/*------------------------- Global Function Prototypes -----------------------*/

extern uint32 SPINorInit(void);
extern uint32 SPINorDeInit(void);

extern uint32 SPINorRead(uint32 Addr, uint8 *pData, uint32 size);
extern uint32 SPINorWrite(uint32 Addr, uint8 *pData, uint32 size);

extern uint32 MDSPINorRead(uint8 lun, uint32 sec, uint32 nSec, void *pData);
extern uint32 MDSPINorWrite(uint8 lun, uint32 sec, uint32 nSec, void *pData);
extern uint32 MDSPIGetInfo(uint8 lun, pMEMDEV_INFO pDevInfo);
extern bool   MDSPIChkEvent(uint32 event);

#endif
