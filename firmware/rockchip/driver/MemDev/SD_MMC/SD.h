/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SD.h
Desc: 

Author: 
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

#ifndef _SD_H
#define _SD_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/


/*------------------------------ Global Typedefs -----------------------------*/

typedef enum
{
    SD_SM_DEFAULT = 0, //SDR12
    SD_SM_HIGH_SPEED,  //SDR50
    SD_SM_SDR50,
    SD_SM_SDR104,
    SD_SM_DDR50,
    SD_SM_MAX
} SD_SPEED_MODE;

/*----------------------------- External Variables ---------------------------*/

/*------------------------- Global Function Prototypes -----------------------*/
extern int32 SDXX_Init(void *pCardInfo, bool SupportCmd8);



#endif
