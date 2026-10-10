/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   FsConfig.h
*
* Description:
*
* History:      <author>          <time>        <version>
*                               2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _FS_CONFIG_H
#define _FS_CONFIG_H

/*
*-------------------------------------------------------------------------------
*
*                           FileSystem Configer
*
*-------------------------------------------------------------------------------
*/
#define     SYS_DATA_DISK_SIZE   200    //unit MB
#define     SYS_USER_DISK2_SIZE  200    //unit MB
#define     FS_FAT_SEC_SIZE      512

//enable ENCODE
#ifndef     ENCODE
#define     ENCODE
#endif


//#define     CD_ROM_EN
//#define     PBA_FLASH_EN

//使能FIND_FILE
#define     FIND_FILE
#define     LONG_DIR_PATH                        //支持长文件名打开

//使能FLASH WRITE
#define     FLASH_WRITE

//enable FLASH disk
#define     FLASH_DISK0          1

#ifdef _MULT_DISK_
#define     FLASH_DISK1          1
#else
#define     FLASH_DISK1          0
#endif

//enable sd card
#ifdef _SDCARD_
#define     SD_CARD_EN          1
#define     SDHC_ENABLE
#else
#define     SD_CARD_EN          0
#endif

#define     MAX_LUN     (FLASH_DISK0+FLASH_DISK1+SD_CARD_EN)  /*最大支持的逻辑设备数*/

//FAT Format definition
#define     FAT32FORMAT
#define     LONG_DIR_PATH

//the vervsion of file system
#define     FS_M_VERSION        83
#define     FS_S_VERSION        11

/*
*-------------------------------------------------------------------------------
*
*                           Section define
*
*-------------------------------------------------------------------------------
*/
//Fat initailization's code and data segment.
#define     _ATTR_FAT_INIT_CODE_        
#define     _ATTR_FAT_INIT_DATA_        
#define     _ATTR_FAT_INIT_BSS_         

//fat basic operation and r/w operation code and data segment,(must be fixed in memory)
#define     _ATTR_FAT_CODE_             
#define     _ATTR_FAT_DATA_             
#define     _ATTR_FAT_BSS_              

//Fat find file code,date segment.
#define     _ATTR_FAT_FIND_CODE_        
#define     _ATTR_FAT_FIND_DATA_        
#define     _ATTR_FAT_FIND_BSS_         

//Fat write operation code,data segment.
#define     _ATTR_FAT_WRITE_CODE_       
#define     _ATTR_FAT_WRITE_DATA_       
#define     _ATTR_FAT_WRITE_BSS_        

//
#define     _ATTR_FS_GET_MEM_CODE_       
#define     _ATTR_FS_GET_MEM_DATA_       
#define     _ATTR_FS_GET_MEM_BSS_        


#define     IRAM_ENCODE_INIT            _ATTR_FAT_INIT_CODE_
#define     IRAM_IDLE                   _ATTR_FAT_INIT_CODE_
#define     DRAM_IDLE                   _ATTR_FAT_INIT_BSS_

#define     IRAM_FAT                    _ATTR_FAT_CODE_
#define     DRAM_FAT                    _ATTR_FAT_BSS_

#define     IRAM_DELETE                 _ATTR_FAT_WRITE_CODE_
#define     IRAM_ENCODE                 _ATTR_FAT_WRITE_CODE_
#define     DRAM_ENCODE                 _ATTR_FAT_WRITE_BSS_

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/

//Flash related configuration.
//#define     _4KB_PAGESIZE                       //support 4K Page Flash
//#define       DUAL_PLANE                      //enable NAND FLASH DUAL PLANE

#define     FLASH_PROTECT_ON()                 // FlashWriteDisable()
#define     FLASH_PROTECT_OFF()                // FlashWriteEnable()

/*
********************************************************************************
*
*                         End of FsConfig.h
*
********************************************************************************
*/
#endif
