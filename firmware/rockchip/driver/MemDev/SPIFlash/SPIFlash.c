/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SFComm.c
Desc:

Author: chenfen
Date: 14-02-25
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/

#include "SysConfig.h"
#include "MDConfig.h"

#ifdef SPIFLASH_DRIVER

#include "SFC.h"
#include "SPIFlash.h"

/*------------------------------------ Defines -------------------------------*/

#define SPINOR_IDB_NUM               1      //ID Block �ĸ����� Boot ��д�ĸ���

#define SPINAND_IDB_NUM              2

/*----------------------------------- Typedefs -------------------------------*/


/*-------------------------- Forward Declarations ----------------------------*/

/* ------------------------------- Globals ---------------------------------- */

/*-------------------------------- Local Statics: ----------------------------*/
_ATTR_FLASH_BSS_ static SFLASH_DRIVER *pSFlashDrv;

_ATTR_FLASH_BSS_ static uint32 SPI_IDB_SIZE;

_ATTR_FLASH_BSS_ static uint32 SPI_FW_OFFSET;       //�̼���ƫ�Ƶ�ַ, sec uint

_ATTR_FLASH_BSS_  uint32    SFlashSysProtMagic;

/*--------------------------- Local Function Prototypes ----------------------*/

/*------------------------ Function Implement --------------------------------*/

/*
Name:       SFlashSysProtSet
Desc:       ϵͳ��д������
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
void SFlashSysProtSet(uint32 magic)
{
    SFlashSysProtMagic=magic;
}

/*
Name:       NandProtStatusGet
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
 _ATTR_FLASH_CODE_
uint32 SFlashProtStatusGet(void)
{
    return (SFlashSysProtMagic);
}

/*
Name:       NandSysProtChk
Desc:       ��ڲ���:��Ҫ��д��������ַ
            ���ڲ���:0=��ϵͳ������, 1=ϵͳ������
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashSysProtChk(uint32 SecAddr)
{
    uint32 protect=0;

    #ifdef SYS_PROTECT
    if (SFlashProtStatusGet() != FLASH_PROT_MAGIC)
    {
        if (SecAddr < SFlashGetCapacity(0))    //ϵͳ������
            protect=1;
    }
    #endif
    return (protect);
}


/*
Name:       SFlashRead
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashRead(uint32 sec, uint32 nSec, void *pData)
{

    if (pSFlashDrv)
        return pSFlashDrv->read(sec, nSec, pData);
    else
        return ERROR;
}


/*
Name:       SFlashWrite
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashWrite(uint32 sec, uint32 nSec, void *pData)
{

    if (pSFlashDrv)
        return pSFlashDrv->write(sec, nSec, pData);
    else
        return ERROR;
}



/*
Name:       SFlashRead
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashErase(uint32 SecAddr)
{

    if (pSFlashDrv)
        return pSFlashDrv->erase(SecAddr);
    else
        return ERROR;
}

/*
Name:       SPINorGetCapacity
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashGetCapacity(uint8 lun)
{

    if (0 == lun)
    {
        return SysProgDiskCapacity;
    }
    else if (1 == lun)
    {
        return SysDataDiskCapacity;
    }
    else if (0xFF == lun)
    {
        SFLASH_INFO *pInfo;

        if (!pSFlashDrv)
            return 0;

        pInfo = (SFLASH_INFO *)pSFlashDrv->pDev;
        return pInfo->capacity;
    }
    else
    {
        return 0;
    }
}


/*
Name:       SPIFlashReadID
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_INIT_CODE_
static int32 SFlashReadID(uint8* data, bool bNand)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_READ_JEDECID;
    sfcmd.b.datasize = 3;               //ID �ֽ�����2-3B, ��ͬ���̲�һ��

    sfctrl.d32 = 0;
    if (bNand)
    {
        sfcmd.b.addrbits = SFC_ADDR_XBITS;
        sfctrl.b.addrbits = 8;
    }
    ret = SFC_Request(sfcmd.d32, sfctrl.d32, 0, data);

    return ret;
}

/*
Name:       SFlashSysCodeChk
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_INIT_CODE_
uint32 SFlashSysCodeChk(void)
{
    uint8 src[512];
    uint8 dst[512];
    uint32 i, addr;
    PFIRMWARE_HEADER pFWHead1;
    SFLASH_INFO *spec;
    unsigned char FwSign[8] = {'R','K','n','a','n','o','F','W'};

    spec = (SFLASH_INFO*)pSFlashDrv->pDev;
    addr = SPI_FW_OFFSET;
    SFlashRead(addr, 1, src);

    pFWHead1 = (PFIRMWARE_HEADER)src;
    if (0 != memcmp(pFWHead1->FwSign, FwSign, 8))
    {
        return ERROR;
    }

    if(OK != SFlashRead(addr+(pFWHead1->FwEndOffset>>9), 1, dst))
    {
        return ERROR;
    }

    if (0 != memcmp(src, dst, 512))
        return ERROR;

    //SysProgRawDiskCapacity = (((pFWHead1->FwEndOffset +512 /*16*1024*/ + FW_ALIGN_SIZE - 1)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE)>>9;
    SysProgRawDiskCapacity = ((((pFWHead1->FwEndOffset + 16*1024 + FW_ALIGN_SIZE)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE)>>9);
    i = SysProgRawDiskCapacity & (spec->BlockSize-1);
    if (i)
    {
        printf("fw error: no align 64KB!\n");
        SysProgRawDiskCapacity = SysProgRawDiskCapacity - i + spec->BlockSize;
    }

    #ifdef FW_UPDATE
    SysProgDiskCapacity = SysProgRawDiskCapacity<<1;
    #else
    SysProgDiskCapacity = SysProgRawDiskCapacity;
    #endif

    return OK;
}

/*
Name:       SFlashInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_INIT_CODE_
uint32 SFlashInit(void)
{
    int32 ret = ERROR;
    uint8 data[3];
    uint8 bSpiNand;
    uint32 i;
    SFLASH_INFO *spec;
    uint32 SPI_IDB_NUM;

    pSFlashDrv = NULL;
    SPI_IDB_SIZE = 0;
    SFC_Init();

    if (SFC_OK != SFlashReadID(data, FALSE))
        return ERROR;

    DEBUG("SPI FLASH ID:%x %x %x\n", data[0], data[1], data[2]);
    //��ʹ��data[0]�ж�, �ǿ��Ǽ���nand, ��nandʱdata[0]=0xFF(IOĬ������������)
    if ((0xFF==data[2] && 0xFF==data[1]) || (0x00==data[2] && 0x00==data[1]))
    {
        return ERROR;
    }

    //bSpiNand = 0;
    bSpiNand = (0xFF==data[0])? 1 : 0;

    #ifdef SPINAND_DRIVER
    if (bSpiNand)
    {
        ret = SNAND_Init(&data[1], &pSFlashDrv);
    }
    else
    #endif
    {
        #ifdef SPINOR_DRIVER
        ret = SNOR_Init(data, &pSFlashDrv);
        #endif
    }

    if (ret != SFC_OK)
        return ret;

    spec = (SFLASH_INFO*)pSFlashDrv->pDev;
    SPI_IDB_NUM = (bSpiNand == 1)? SPINAND_IDB_NUM : SPINOR_IDB_NUM;
    SPI_IDB_SIZE = SPI_IDB_NUM*spec->BlockSize;
    SPI_FW_OFFSET = SPI_IDB_SIZE + OTP_PARTITION_SECS;

    {
		uint8 DataBuf[1024];
        pIDSEC0 IdSec0;
        pIDSEC1 IdSec1;
		for (i=0; i<SPI_IDB_NUM; i++)	//��2��BLOCK�����ID PAGE
		{
            if (OK != SFlashRead((i*spec->BlockSize), 2, DataBuf))
            {
                continue;
            }

            IdSec0 = (pIDSEC0)DataBuf;
            if (IdSec0->magic == 0xFCDC8C3B)   //0x0ff0aa55 + rc4
			{
                IdSec1=(pIDSEC1)(DataBuf+512);      //IdSec1 NOT RC4
                if (IdSec1->ChipTag==CHIP_SIGN)
                {
                    //spec->PageSize=IdSec1->pageSize;
                    //spec->BlockSize=IdSec1->blockSize;

                    //LoaderVer    = IdSec1->LoaderVer;
                    //UpgradeTimes = IdSec1->UpgradeTimes;

                    SysProgDiskCapacity = (uint32)(IdSec1->SysProgDiskCapacity)<<11;
                    DEBUG("SysProgDiskCapacity = %d",SysProgDiskCapacity);
                    SysProgRawDiskCapacity = SysProgDiskCapacity>>1;
                    //SysDataDiskCapacity = SysUserDisk2Capacity=0;

                    //if ((SysProgDiskCapacity) <= spec->capacity) //�̼���������NOR����, �̼�����ڿ���
                    ret = OK;

                    break;
                }
			}
		}
    }

    if (OK == ret)
    {
        ret = SFlashSysCodeChk();
    }

    return ret;
}

/*
Name:       SPINorInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashDeInit(void)
{
    return OK;
}


/*
Name:       SFlashReadIDB
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashReadIDB(uint32 sec, uint32 nSec, uint8 *pData)
{
    if (sec >= SPI_IDB_SIZE)
    {
        sec = sec & (SPI_IDB_SIZE-1);
    }

    return SFlashRead(sec, nSec, pData);
}

#ifdef IN_LOADER

/*
Name:       SFlashWriteIDB
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashWriteIDB(uint32 sec, uint32 nSec, uint8 *pData)
{
    if (sec >= SPI_IDB_SIZE)
        return OK;

    return SFlashWrite(sec, nSec, pData);
}
#endif

#if defined(OTP_DATA_ENABLE)
/*
Name:       SFlashReadOTP
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashReadOTP(uint32 ByteAddr, uint32 ByteSize, void *pData)
{
    uint32 sec = ByteAddr>>9;
    uint32 nSec = ByteSize>>9;

    sec += SPI_IDB_SIZE;
    return SFlashRead(sec, nSec, pData);
}

/*
Name:       SFlashWriteOTP
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 SFlashWriteOTP(uint32 ByteAddr, uint32 ByteSize, void *pData)
{
    uint32 sec = ByteAddr>>9;
    uint32 nSec = ByteSize>>9;

    if ((sec + nSec) > OTP_PARTITION_SECS)
        return ERROR;

    sec += SPI_IDB_SIZE;

    return SFlashWrite(sec, nSec, pData);
}

#endif

/*
Name:       MDSFlashRead
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 MDSFlashRead(uint8 lun, uint32 sec, uint32 nSec, void *pData)
{
    SFLASH_INFO *spec;

    if (lun > 1)
    {
        DEBUG("error!\n");
        return ERROR;
    }

    spec = (SFLASH_INFO*)pSFlashDrv->pDev;
    sec = sec + SPI_FW_OFFSET + ((lun>0)? SysProgDiskCapacity:0);
    if (sec+nSec > spec->capacity)
        return ERROR;

    return SFlashRead(sec, nSec, pData);
}

/*
Name:       MDSFlashWrite
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 MDSFlashWrite(uint8 lun, uint32 sec, uint32 nSec, void *pData)
{
    uint32 ret;
    uint32 len, BlkOffset;
    SFLASH_INFO *spec;

    if (lun > 1)
    {
        DEBUG("error!\n");
        return ERROR;
    }

    #ifdef SYS_PROTECT
    if (lun < 1)
    {
        if (SFlashSysProtChk(sec))
        {
            DEBUG("SFlashSysProt!\n");
            return ERROR;
        }
    }
    #endif

    spec = (SFLASH_INFO*)pSFlashDrv->pDev;
    sec = sec + SPI_FW_OFFSET + ((lun>0)? SysProgDiskCapacity:0);
    if (sec+nSec > spec->capacity)
        return ERROR;

    //sec += SPI_IDB_SIZE;

    return SFlashWrite(sec, nSec, pData);
}


/*
Name:       MDGetInfo
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 MDSFlashGetInfo(uint8 lun, pMEMDEV_INFO pDevInfo)
{
    SFLASH_INFO *spec;

    if (!pSFlashDrv)
        return ERROR;

    spec = (SFLASH_INFO*)pSFlashDrv->pDev;

    pDevInfo->BlockSize = spec->BlockSize;
    pDevInfo->PageSize = spec->PageSize;
    pDevInfo->capacity = spec->capacity;

    return OK;
}

/*
Name:       SFlashGetProductSn
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
int32 SFlashGetProductSn(void *pSn)
{
    int32 ret = -1;

    uint8 DataBuf[512];
    PRKNANO_IDB_SEC3 pIDBSec3;

    DEBUG("SFlashGetProductSn in");
    if (SDM_SUCCESS != SFlashReadIDB(3, 1, DataBuf))//����IDB��3������.
    {
        return ret;
    }

    pIDBSec3 = (PRKNANO_IDB_SEC3)DataBuf;

    if (pIDBSec3->usSNSize)
    {
        memcpy(pSn, pIDBSec3->sn, pIDBSec3->usSNSize);
    }
    ret = pIDBSec3->usSNSize;
    DEBUG("SFlashGetProductSn Out");

    return ret;
}

/*
Name:       SFlashGetBluetoothMac
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
int32 SFlashGetBluetoothMac(void *pBTMac)
{
    int32 ret = -1;

    uint8 DataBuf[512];
    PRKNANO_IDB_SEC3 pIDBSec3;

    DEBUG("SFlashGetBluetoothMac in");
    if (SDM_SUCCESS != SFlashReadIDB(3, 1, DataBuf))//����IDB��3������.
    {
        return ret;
    }

    pIDBSec3 = (PRKNANO_IDB_SEC3)DataBuf;
    if (pIDBSec3->blueToothSize)
    {
        memcpy(pBTMac, pIDBSec3->blueToothAddr, pIDBSec3->blueToothSize);
    }
    ret = pIDBSec3->blueToothSize;

    DEBUG("SFlashGetProductSn success Out");
    return ret;
}


#ifdef IN_LOADER

/*
Name:       MDSFlashErase
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 MDSFlashErase(uint32 ChipSel, uint32 BlkStart, uint32 BlkNum, uint32 mod)
{

    uint32 StartAddr, EndAddr;
    uint32 ret = OK;
    SFLASH_INFO *spec;

    UNUSED(mod);
    spec = (SFLASH_INFO*)pSFlashDrv->pDev;

    StartAddr = BlkStart*spec->BlockSize;
    EndAddr = StartAddr+BlkNum*spec->BlockSize;

    if (ChipSel > 0 || EndAddr > SPI_IDB_SIZE) //ֻ�ܲ���IDB block
    {
        return OK;
    }

    for ( ; StartAddr < EndAddr; StartAddr+= spec->BlockSize)
    {
        if (OK != SFlashErase(StartAddr))
            ret = ERROR;
    }

    return ret;
}
#endif

#endif
