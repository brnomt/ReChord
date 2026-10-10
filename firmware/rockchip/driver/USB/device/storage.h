/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: storage.h
Desc: 

Author: 
Date: 13-07-26
Notes:

$Log: $
 *
 *
*/

#ifndef _FSTORAGE_H
#define _FSTORAGE_H

/*-------------------------------- Includes ----------------------------------*/

#include    "SysConfig.h"

/*------------------------------ Global Defines ------------------------------*/


/*------------------------------ Global Typedefs -----------------------------*/

typedef __packed struct _RKNANO_VERSION{
    uint16 dwBootVer; // Loader version
    uint32 dwFirmwareVer; // System version
    uint32 Reserved; 
}RKNANO_VERSION, *PRKNANO_VERSION; //RKNANO device version struct

//≈‰÷√√Ë ˆ∑˚ºØ∫œ√Ë ˆ∑˚Ω·ππ
typedef __packed struct _USB_FSG_CONFIGS_DESCRIPTOR 
{
    USB_CONFIGURATION_DESCRIPTOR Config;
    USB_INTERFACE_DESCRIPTOR Interface;
    USB_ENDPOINT_DESCRIPTOR BulkIn;
    USB_ENDPOINT_DESCRIPTOR BulkOut;
} USB_FSG_CONFIGS_DESCRIPTOR, *PUSB_FSG_CONFIGS_DESCRIPTOR;


/*----------------------------- External Variables ---------------------------*/


/*------------------------- Global Function Prototypes -----------------------*/

extern int32 FsgThread(void);

extern int32 FsgInit(void *pArg);

extern void FsgDeInit(void);

#endif


