/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SFNand.h
Desc: 

Author: 
Date: 14-02-25
Notes:

$Log: $
 *
 *
*/

#ifndef _SFNAND_H
#define _SFNAND_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/


/*------------------------------ Global Typedefs -----------------------------*/


/*----------------------------- External Variables ---------------------------*/


/*------------------------- Global Function Prototypes -----------------------*/

extern int32 SNAND_Init(uint8* pFlashID, SFLASH_DRIVER **pDrv);

extern int32 SNAND_SetDLines(SFC_DATA_LINES lines);

extern int32 SNAND_ReadID(uint8* data);

extern int32 SNAND_Read(uint32 sec, uint32 nSec, void *pData);

extern int32 SNAND_Write(uint32 sec, uint32 nSec, void *pData);

extern int32 SNAND_ReadPage(uint32 RowAddr, uint32 ColAddr, void *pData, uint32 size);

extern int32 SNAND_ProgPage(uint32 RowAddr, void *pData, uint32 size);

extern int32 SNAND_Erase(uint32 RowAddr);

extern int32 SNAND_GetFeature(uint32 addr, uint32 *status);

extern int32 SNAND_SetFeature(uint32 addr, uint32 feature);

extern uint32 SNAND_GetCapacity(void);

#endif
