/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  Hw_Sdmmc.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _HW_SD_H_
#define _HW_SD_H_

/*
*-------------------------------------------------------------------------------
*
*                  external memory control memap register define
*
*-------------------------------------------------------------------------------
*/
typedef volatile struct {
    
    UINT32 SDCI_CTRL;
    UINT32 SDCI_DCTRL;
    UINT32 SDCI_DLEN;
    UINT32 SDCI_TOKEN;
    UINT32 SDCI_CMD;
    UINT32 SDCI_ARG;	//32bit Register.
    UINT32 SDCI_STAC;
    UINT32 SDCI_STA;
    UINT32 SDCI_RESP0; //32bit Register.
    UINT32 SDCI_RESP1; //8bit Register.
    UINT32 SDCI_TX;		//32bit Register.
    UINT32 SDCI_RX;		//32bit Register.
    UINT32 SDCI_CRC16;
    UINT32 SDCI_ADDR1; //8bit Register.
    UINT32 SDCI_ADDR2; //8bit Register.
   
} SDMMC_t,*gSDMMC_t;

#define SDMMC_BASE                   ((UINT32)0x4000A000)
#define SDMMC                        ((SDMMC_t *) SDMMC_BASE)


/*
********************************************************************************
*
*                         End of hw_SDMMC.h
*
********************************************************************************
*/
#endif

