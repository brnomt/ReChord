/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SDAdapt.h
Desc: 

Author: 
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

#ifndef _SDADAPT_H
#define _SDADAPT_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/

#define NO_TIMEOOUT        (0xFFFFFFFF)

/* for debug */
#define CD_EVENT                (0x1 << 0)
#define DTO_EVENT               (0x1 << 1)
#define DMA_EVENT               (0x1 << 2)
#define VSWTCH_EVENT            (0x1 << 3) 
#define BDONE_EVENT             (0x1 << 4)
#define ERR_EVENT               (0x1 << 5)

/*------------------------------ Global Typedefs -----------------------------*/
typedef uint32*   pEVENT;
typedef uint32*   pMUTEX;

typedef struct TagSDOAM_EVENT
{
    uint32 event;
}SDOAM_EVENT_T;

/*----------------------------- External Variables ---------------------------*/


/*------------------------- Global Function Prototypes -----------------------*/
extern void SDA_Delay(uint32 us);
extern pEVENT SDA_CreateEvent(uint8 nSDCPort);
extern void SDA_SetEvent(pEVENT handle, uint32 event);
extern int32 SDOAM_GetEvent(pEVENT handle, uint32 event, uint32 timeout);
extern bool SDA_DMAStart(SDMMC_PORT_E nSDCPort, uint32 DstAddr, uint32 SrcAddr, uint32 size, bool rw, pFunc CallBack);
extern bool SDA_DMAStop(SDMMC_PORT_E nSDCPort);
extern uint32 SDA_GetSrcClkFreq(SDMMC_PORT_E nSDCPort, uint32 FreqKHz, uint32 *SrcSel);
extern int32 SDA_SetSrcClkDiv(SDMMC_PORT_E nSDCPort, uint32 div, uint32 SrcSel);
extern void SDA_SetSrcClk(SDMMC_PORT_E nSDCPort, bool enable);
extern bool SDA_RegISR(SDMMC_PORT_E nSDCPort, pFunc Routine);
extern void SDA_EnableIRQ(SDMMC_PORT_E nSDCPort);
extern bool SDA_SetIOMux(SDMMC_PORT_E SDCPort, HOST_BUS_WIDTH width);
extern void SDA_Init(SDMMC_PORT_E nSDCPort, SDMMC_PORT_FUNCTION func);


#endif
