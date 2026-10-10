/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: mmc.h
Desc: 

Author: 
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

#ifndef _MMC_H
#define _MMC_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/


/*------------------------------ Global Typedefs -----------------------------*/


/*----------------------------- External Variables ---------------------------*/

/*------------------------- Global Function Prototypes -----------------------*/
extern int32 MMC_Init(pSDM_CARD_INFO pCard);
extern int32 MMC_PartitionConfig(pSDM_CARD_INFO         pCard, uint8 value);

#endif
