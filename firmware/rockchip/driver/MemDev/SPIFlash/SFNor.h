/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SFNOR.h
Desc: 

Author: 
Date: 14-02-25
Notes:

$Log: $
 *
 *
*/

#ifndef _SFNOR_H
#define _SFNOR_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/


/*------------------------------ Global Typedefs -----------------------------*/

typedef enum
{
    ERASE_SECTOR = 0,
    ERASE_BLOCK64K,
    //ERASE_BLOCK32K,
    ERASE_CHIP
} NOR_ERASE_TYPE;


typedef enum
{
    IO_MODE_SPI = 0,
    IO_MODE_QPI
} SNOR_IO_MODE;

typedef enum
{
    READ_MODE_NOMAL = 0,
    READ_MODE_FAST
} SNOR_READ_MODE;

typedef enum
{
    ADDR_MODE_3BYTE = 0,
    ADDR_MODE_4BYTE
} SNOR_ADDR_MODE;


/*----------------------------- External Variables ---------------------------*/

/*------------------------- Global Function Prototypes -----------------------*/

extern int32 SNOR_Init(uint8* pFlashID, SFLASH_DRIVER **pDrv);

extern uint32 SNOR_GetCapacity(void);

extern int32 SNOR_ReadID(uint8* data);

extern int32 SNOR_Read(uint32 sec, uint32 nSec, void *pData);

extern int32 SNOR_Write(uint32 sec, uint32 nSec, void *pData);

extern int32 SNOR_Prog(uint32 addr, void *pData, uint32 size);

extern int32 SNOR_Erase(uint32 addr, NOR_ERASE_TYPE EraseType);

extern int32 SNOR_EraseBlk(uint32 addr);

#endif
