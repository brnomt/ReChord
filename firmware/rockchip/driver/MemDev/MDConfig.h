/********************************************************************************
*********************************************************************************
            COPYRIGHT (c)   2004 BY ROCK-CHIP FUZHOU
                --  ALL RIGHTS RESERVED  --

File Name:      config.h
Author:         XUESHAN LIN
Created:        1st Dec 2008
Modified:
Revision:       1.00
********************************************************************************
********************************************************************************/
#ifndef _MDCONFIG_H
#define  _MDCONFIG_H

#include    <stdio.h>
#include    <string.h>
#include    "SysConfig.h"

#include    "typedef.h"
#include    "macro.h"

#include    "hw_memap.h"
#include    "Hook.h"
#include    "MemDev.h"
#include    "USBConfig.h"


#include    "SPIFlash/SPIFlash.h"

#include    "SD_MMC/SDConfig.h"


#define UNUSED(x)           ( void )(x)

#if defined(_SDCARD_)
#define     SDC0_DRIVER
#endif

#if defined(_EMMC_)
#define     EMMC_DRIVER
#endif

#if defined(EMMC_DRIVER) || defined(SDC0_DRIVER)
#define     SDMMC_DRIVER
#endif

#if defined(_USB_HOST_)
#define     USBHOST_DRIVER
#endif

#ifdef _SPINOR_
#define   SPINOR_DRIVER
#endif

//#define       SPINAND_DRIVER

#if defined(SPINAND_DRIVER) || defined(SPINOR_DRIVER)
#define SPIFLASH_DRIVER
#endif



//SD����ʼ�����롢���ݶ�
#define     _ATTR_SD_INIT_CODE_         
#define     _ATTR_SD_INIT_DATA_         
#define     _ATTR_SD_INIT_BSS_          

//SD��������������ȡ�������롢���ݶ�
#define     _ATTR_SD_CODE_              
#define     _ATTR_SD_DATA_              
#define     _ATTR_SD_BSS_               
#define     IRAM_SD                     _ATTR_SD_CODE_

//SD��д�������롢���ݶ�
#define     _ATTR_SD_WRITE_CODE_        
#define     _ATTR_SD_WRITE_DATA_        
#define     _ATTR_SD_WRITE_BSS_         


#define     _ATTR_FLASH_INIT_CODE_      //
#define     _ATTR_FLASH_INIT_DATA_      //
#define     _ATTR_FLASH_INIT_BSS_       //

#define     _ATTR_FLASH_CODE_           
#define     _ATTR_FLASH_DATA_           
#define     _ATTR_FLASH_BSS_            

#define     _ATTR_FLASH_WRITE_CODE_      //
#define     _ATTR_FLASH_WRITE_DATA_      //
#define     _ATTR_FLASH_WRITE_BSS_       //

#define     IRAM_FLASH_INIT             _ATTR_FLASH_INIT_CODE_

#endif


