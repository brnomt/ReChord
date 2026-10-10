/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: eMMC.c
Desc:

Author: chenfen
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/

#include "SDConfig.h"
#if defined(EMMC_DRIVER)

#include "gpio.h"
#include "Hw_cru.h"
#include "hw_grf.h"
#include "eMMC.h"

/*------------------------------------ Defines -------------------------------*/

#define EMMC_IDB_NUM    5

#define  EMMC_BOOT_OFFSET       (64)                //IDB ��32Kƫ�ƿ�ʼ

#define EMMC_IDB_SIZE          (3*1024*1024/512)       //��0��ַ��ʼ,����0��ַ������32KB+IDB Size(512KB*5)
#define EMMC_BOOT_SIZE         EMMC_IDB_SIZE           //BOOT������С

#define EMMC_OTP_OFFSET         EMMC_IDB_SIZE       // 3MB~4MB �ռ�Ԥ����OTP ʹ��

#define  EMMC_SYS_OFFSET        (4*1024*1024/512)      //�̼���4Mƫ�ƿ�ʼ


#define EmmcCtl                  ((SDC_REG_T *)(EMMC_BASE)

/*----------------------------------- Typedefs -------------------------------*/


/*-------------------------- Forward Declarations ----------------------------*/



/* ------------------------------- Globals ---------------------------------- */


/*-------------------------------- Local Statics: ----------------------------*/
_ATTR_SD_BSS_  uint32    EmmcSysProtMagic;

_ATTR_SD_BSS_ SDM_CARD_INFO EMMCInfo;

/*--------------------------- Local Function Prototypes ----------------------*/

void EmmcTest(void);

/*------------------------ Function Implement --------------------------------*/


/*
Name:       EmmcGetSysCapacity
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
uint32 EmmcGetSysCapacity(void)
{
    uint32 capacity = SDM_GetCapability(SDM_EMMC_ID);

    if (capacity > EMMC_SYS_OFFSET)
        capacity -= EMMC_SYS_OFFSET;
    else
        capacity = 0;

    return capacity;
}

/*
Name:       EmmcGetCapacity
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
uint32 EmmcGetCapacity(uint8 lun)
{
    uint32 capacity = 0;

    switch (lun)
    {
        case 0:
            capacity = SysProgDiskCapacity;
            break;
        case 1:
            capacity = SysDataDiskCapacity;
            break;
        case 2:
            capacity = SysProgDiskCapacity+SysDataDiskCapacity+SysUserDisk2Capacity;
            capacity = EmmcGetSysCapacity() - capacity;
            break;
        case 3:
            capacity = SysUserDisk2Capacity;
            break;
        case 0xff:
            capacity = EmmcGetSysCapacity();
            break;
        default:
            break;
    }

    return (capacity);
}

/*
Name:       EmmcSysProtSet
Desc:       ϵͳ��д������
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
void EmmcSysProtSet(uint32 magic)
{
    EmmcSysProtMagic=magic;
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
_ATTR_SD_CODE_
uint32 EmmcProtStatusGet(void)
{
    return (EmmcSysProtMagic);
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
_ATTR_SD_CODE_
uint32 EmmcSysProtChk(uint32 SecAddr)
{
    uint32 protect=0;

    #ifdef SYS_PROTECT
    if (EmmcProtStatusGet() != FLASH_PROT_MAGIC)
    {
        if (SecAddr < EmmcGetCapacity(0))    //ϵͳ������
            protect=1;
    }
    #endif
    return (protect);
}

/*
Name:        EmmcGetInfo
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
uint32 EmmcGetInfo(uint8 lun, pMEMDEV_INFO pDevInfo)
{
    //pDevInfo->Manufacturer = 0;
    pDevInfo->BlockSize = SDM_BLOCK_SIZE;  //�̶���512K
    pDevInfo->PageSize = 16;
    pDevInfo->capacity= EmmcGetCapacity(lun);

    return OK;
}


/*
Name:        MDEmmcRead
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
uint32 EmmcRead(uint8 lun, uint32 LBA, uint32 nSec, void *pBuf)
{
    int32 ret = ERROR;
    uint32 offset = LBA;
    //ScuClockGateCtr(CLK_EMMC_GATE, 1);
    ScuClockGateCtr(HCLK_EMMC_GATE, 1);

    switch (lun)
    {
        case 3:
            offset += EmmcGetCapacity(2);
        case 2:
            offset += EmmcGetCapacity(1);
        case 1:
            offset += EmmcGetCapacity(0);
        case 0:

            if ((offset+nSec) > EmmcGetSysCapacity())
                break;

            //printf("eMMc: LBA = %d, nSec = %d\n", LBA, nSec);
            ret = SDM_Read(SDM_EMMC_ID, EMMC_SYS_OFFSET+offset, nSec, pBuf);
            ret = (SDM_SUCCESS == ret)? OK : ERROR;
            break;
        default:
            break;
    }

    //ScuClockGateCtr(CLK_EMMC_GATE, 0);
    ScuClockGateCtr(HCLK_EMMC_GATE, 0);

    return (uint32)ret;
}

/*
Name:        MD_SDWrite
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_WRITE_CODE_
uint32 EmmcWrite(uint8 lun, uint32 LBA, uint32 nSec, void *pBuf)
{
    int32 ret = ERROR;
    uint32 offset = LBA;
    //ScuClockGateCtr(CLK_EMMC_GATE, 1);
    ScuClockGateCtr(HCLK_EMMC_GATE, 1);

    switch (lun)
    {
        case 3:
            offset += EmmcGetCapacity(2);
        case 2:
            offset += EmmcGetCapacity(1);
        case 1:
            offset += EmmcGetCapacity(0);
        case 0:
        #ifdef SYS_PROTECT
            if (EmmcSysProtChk(offset))
                break;
        #endif

            if ((offset+nSec) > EmmcGetSysCapacity())
                break;

            ret = SDM_Write(SDM_EMMC_ID, EMMC_SYS_OFFSET+offset, nSec, pBuf);
            ret = (SDM_SUCCESS == ret)? OK : ERROR;
            break;
        default:
            break;
    }

    //ScuClockGateCtr(CLK_EMMC_GATE, 0);
    ScuClockGateCtr(HCLK_EMMC_GATE, 0);

    return (uint32)ret;
}

void EMMC_GPIO_Init(void)
{
    //Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin3, IOMUX_GPIO1B3_EMMC_RSTN);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin0, IOMUX_GPIO0A0_EMMC_PWREN);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1, IOMUX_GPIO0A1_EMMC_CLK);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2, IOMUX_GPIO0A2_EMMC_CMD);

//    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3, IOMUX_GPIO0A3_EMMC_D0);
//    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4, IOMUX_GPIO0A4_EMMC_D1);
//    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5, IOMUX_GPIO0A5_EMMC_D2);
//    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6, IOMUX_GPIO0A6_EMMC_D3);
//    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7, IOMUX_GPIO0A7_EMMC_D4);
//    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0, IOMUX_GPIO0B0_EMMC_D5);
//    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1, IOMUX_GPIO0B1_EMMC_D6);
//    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2, IOMUX_GPIO0B2_EMMC_D7);

}

/*
Name:       EmmcInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
uint32 EmmcInit(void)
{
    int32 ret = OK;

    //ScuClockGateCtr(CLK_EMMC_GATE, 1);
    ScuClockGateCtr(HCLK_EMMC_GATE, 1);

    if (SDM_SUCCESS != SDM_Init(SDM_EMMC_ID, &EMMCInfo, EMMC_FUNC))
    {
        ret =  ERROR;
        goto END;
    }

    if (SDM_SUCCESS != SDM_Open(SDM_EMMC_ID, eMMC2G))
    {
        ret =  ERROR;
        goto END;
    }

    {
        uint32 i;
        uint8 DataBuf[1024];
        pIDSEC0 IdSec0;
        pIDSEC1 IdSec1;

        for (i=0; i<EMMC_IDB_NUM; i++)  //��2��BLOCK�����ID PAGE
        {
            if (SDM_SUCCESS != EmmcReadIDB((i*SDM_BLOCK_SIZE), 2, DataBuf))
            {
                continue;
            }

            IdSec0 = (pIDSEC0)DataBuf;
            if (IdSec0->magic == 0xFCDC8C3B)   //0x0ff0aa55 + rc4
            {
                IdSec1=(pIDSEC1)(DataBuf+512);      //IdSec1 NOT RC4
                if (IdSec1->ChipTag == CHIP_SIGN)
                {
                    //spec->PageSize=IdSec1->pageSize;
                    //spec->BlockSize=IdSec1->blockSize;

                    LoaderVer    = IdSec1->LoaderVer;
                    UpgradeTimes = IdSec1->UpgradeTimes;

                    SysProgDiskCapacity = (uint32)(IdSec1->SysProgDiskCapacity)<<11;
                    SysProgRawDiskCapacity = SysProgDiskCapacity>>1;
                    SysDataDiskCapacity = SysUserDisk2Capacity=0;

                    if ((SysProgDiskCapacity) <= EmmcGetSysCapacity())
                        ret = OK;

                    break;
                }
            }
        }
    }

END:
    //ScuClockGateCtr(CLK_EMMC_GATE, 0);
    ScuClockGateCtr(HCLK_EMMC_GATE, 0);

    return ret;
}

/*
Name:       EmmcDeInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
uint32 EmmcDeInit(void)
{
    //ScuClockGateCtr(CLK_EMMC_GATE, 1);
    ScuClockGateCtr(HCLK_EMMC_GATE, 1);

    SDM_Close(SDM_EMMC_ID);
    SDC_WaitBusy(0);
    SDC_ResetEmmc();

    ScuSoftResetCtr(SDMMC_SRST,1);
    //CRUReg->CRU_SOFTRST_CON[1] = 1<<1 | 1<<17;
    DelayUs(100);
    ScuSoftResetCtr(SDMMC_SRST,0);
    //CRUReg->CRU_SOFTRST_CON[1] = 0<<1 | 1<<17;

    //ScuClockGateCtr(CLK_EMMC_GATE, 0);
    ScuClockGateCtr(HCLK_EMMC_GATE, 0);

    return OK;

}

/*
Name:       EmmcReadIDB
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 EmmcReadIDB(uint32 sec, uint32 nSec, uint8 *pData)
{
    int32 ret;

    sec += EMMC_BOOT_OFFSET;
    if (sec + nSec > EMMC_BOOT_SIZE)
        return ERROR;

    ScuClockGateCtr(HCLK_EMMC_GATE, 1);

    ret = SDM_Read(SDM_EMMC_ID, sec, nSec, pData);

    ScuClockGateCtr(HCLK_EMMC_GATE, 0);

    return (uint32)ret;
}

/*
Name:       EmmcWriteIDB
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 EmmcWriteIDB(uint32 sec, uint32 nSec, uint8 *pData)
{
    int32 ret;

    sec += EMMC_BOOT_OFFSET;
    if (sec + nSec > EMMC_BOOT_SIZE)
        return ERROR;

    ScuClockGateCtr(HCLK_EMMC_GATE, 1);

    ret = SDM_Write(SDM_EMMC_ID, sec, nSec, pData);

    ScuClockGateCtr(HCLK_EMMC_GATE, 0);

    return (uint32)ret;
}

#if defined(OTP_DATA_ENABLE)
/*
Name:       EmmcReadOTP
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 EmmcReadOTP(uint32 ByteAddr, uint32 ByteSize, uint8 *pData)
{
    uint32 sec = ByteAddr>>9;
    uint32 nSec = ByteSize>>9;

    if ((sec + nSec) > OTP_PARTITION_SECS)
        return ERROR;

    sec += EMMC_OTP_OFFSET;
    if (SDM_SUCCESS != SDM_Read(SDM_EMMC_ID, sec, nSec, pData))
        return ERROR;

    return OK;
}

/*
Name:       EmmcWriteOTP
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
uint32 EmmcWriteOTP(uint32 ByteAddr, uint32 ByteSize, uint8 *pData)
{
    uint32 sec = ByteAddr>>9;
    uint32 nSec = ByteSize>>9;

    if ((sec + nSec) > OTP_PARTITION_SECS)
        return ERROR;

    sec += EMMC_OTP_OFFSET;
    if (SDM_SUCCESS != SDM_Write(SDM_EMMC_ID, sec, nSec, pData))
        return ERROR;

    return OK;
}
#endif

/*
Name:       EmmcGetProductSn
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
int32 EmmcGetProductSn(void *pSn)
{
    int32 ret = -1;

    uint8 DataBuf[512];
    PRKNANO_IDB_SEC3 pIDBSec3;

    //ScuClockGateCtr(CLK_EMMC_GATE, 1);
    ScuClockGateCtr(HCLK_EMMC_GATE, 1);

    if (SDM_SUCCESS != EmmcReadIDB(3, 1, DataBuf))//����IDB��3������.
    {
        goto eMMCGetSNEND;
    }

    pIDBSec3 = (PRKNANO_IDB_SEC3)DataBuf;
    if (pIDBSec3->usSNSize)
    {
        memcpy(pSn, pIDBSec3->sn, pIDBSec3->usSNSize);
    }
    ret = pIDBSec3->usSNSize;

eMMCGetSNEND:
    //ScuClockGateCtr(CLK_EMMC_GATE, 0);
    ScuClockGateCtr(HCLK_EMMC_GATE, 0);

    return ret;
}

/*
Name:       EmmcGetBluetoothMac
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
int32 EmmcGetBluetoothMac(void *pBTMac)
{
    int32 ret = -1;

    uint8 DataBuf[512];
    PRKNANO_IDB_SEC3 pIDBSec3;

    //ScuClockGateCtr(CLK_EMMC_GATE, 1);
    ScuClockGateCtr(HCLK_EMMC_GATE, 1);

    if (SDM_SUCCESS != EmmcReadIDB(3, 1, DataBuf))//����IDB��3������.
    {
        goto eMMCGetSNEND;
    }

    pIDBSec3 = (PRKNANO_IDB_SEC3)DataBuf;
    if (pIDBSec3->blueToothSize)
    {
        memcpy(pBTMac, pIDBSec3->blueToothAddr, pIDBSec3->blueToothSize);
    }
    ret = pIDBSec3->blueToothSize;

eMMCGetSNEND:
    //ScuClockGateCtr(CLK_EMMC_GATE, 0);
    ScuClockGateCtr(HCLK_EMMC_GATE, 0);

    return ret;
}


#if 0
/*extern*/ uint32 ProbeReadBuf[PAGE_SIZE];   //FLASH̽��ʱ�õ�PAGE BUF
/*extern*/ uint32 ProbeWriteBuf[PAGE_SIZE];  //FLASH̽��ʱ�õ�PAGE BUF
//uint8 TestEmmcRet;
/*
Name:       EmmcTest
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FLASH_CODE_
void EmmcTest(void)
{
    uint32 i;

    //TestEmmcRet = 0;
	for (i=0; i<512*4; i++)
	{
	    ProbeWriteBuf[i]=i+1;
	    //ProbeWriteBuf[i] = 0xFFFFFFFF;
	}

    memset(ProbeReadBuf, 0, 512*4);
    if (SDM_SUCCESS != SDM_Read(SDM_EMMC_ID, 0, 4, ProbeReadBuf))
    {
        while(1);
    }

    if (SDM_SUCCESS != SDM_Write(SDM_EMMC_ID, 0, 4, ProbeWriteBuf))
    {
        while(1);
    }

    memset(ProbeReadBuf, 0, 512*4);
    if (SDM_SUCCESS != SDM_Read(SDM_EMMC_ID, 0, 4, ProbeReadBuf))
    {
        while(1);
    }

    for(i=0; i<4*128; i++)
    {
        if (ProbeReadBuf[i] != ProbeWriteBuf[i])
        {
            while(1);
        }
    }
    while(1);
}
#endif

#endif

