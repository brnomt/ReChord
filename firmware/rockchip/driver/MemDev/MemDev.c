/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: MemDev.c
Desc:

Author: chenfen
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/
#include "MDConfig.h"
#include "SDConfig.h"
#include "USBConfig.h"


/*------------------------------------ Defines -------------------------------*/

/*----------------------------------- Typedefs -------------------------------*/


//memory operation function
typedef uint32 (*MEM_INIT)(void);
typedef uint32 (*MEM_READ)(uint8 lun, uint32 SecAdrr, uint32 nSec, void *pBuf);
typedef uint32 (*MEM_WRITE)(uint8 lun, uint32 SecAdrr, uint32 nSec, void *pBuf);
//typedef uint32 (*MEM_IOCTRL)(uint32 cmd, uint32 arg, void *pBuf);

typedef void (*MEM_DEINIT)(void);
typedef uint32 (*MEN_GETDEVINFO)(uint8 lun, pMEMDEV_INFO pDevInfo);
typedef bool (*MEM_CHECKEVENT)(uint32 event);

typedef uint32 (*MEM_PROBE)(void);

typedef void (*MEM_SYSWPSET)(uint32 magic);

typedef int32 (*MEM_GETPSN)(void *pSn);


typedef struct tagMEM_DEVICE
{
    MEM_INIT init;
    MEM_READ read;
    MEM_WRITE write;
    MEN_GETDEVINFO GetInfo;
    MEM_CHECKEVENT CheckEvent;
    MEM_PROBE probe;
    MEM_DEINIT DeInit;
    MEM_SYSWPSET SysWPSet;
    MEM_GETPSN GetProductSN;
    //char*  name;
    uint16 major;
    uint8 removable;
    uint8 reserved;
    //uint32 priv;

}MEM_DEVICE, *pMEM_DEVICE;


/*-------------------------- Forward Declarations ----------------------------*/
/* ------------------------------- Globals ---------------------------------- */
_ATTR_SYS_BSS_ uint32   SysProgDiskCapacity;            //ϵͳ����������
_ATTR_SYS_BSS_ uint32   SysProgRawDiskCapacity;         //ϵͳ����������
_ATTR_SYS_BSS_ uint32   SysDataDiskCapacity;            //ϵͳ����������
_ATTR_SYS_BSS_ uint32   SysUserDisk2Capacity;           //ϵͳ����������

_ATTR_SYS_BSS_  uint16  UpgradeTimes;
_ATTR_SYS_BSS_  uint16  LoaderVer;
_ATTR_SYS_BSS_  uint8   NandIOMuxRef;         //Nand��LCD IO �л�����
_ATTR_SYS_BSS_  uint8   ValidSysDisk;



static MEM_DEVICE const gMemDevTab[] =
{
    #if defined(SPIFLASH_DRIVER)
    {
        SFlashInit,
        MDSFlashRead,
        MDSFlashWrite,
        MDSFlashGetInfo,
        NULL,
        NULL,
        NULL,
        SFlashSysProtSet,
        SFlashGetProductSn,
        MEM_DEV_SPINOR,
        0,
        0
    },
    #endif
    #if defined(EMMC_DRIVER)
    {
        EmmcInit,
        EmmcRead,
        EmmcWrite,
        EmmcGetInfo,
        NULL,
        NULL,
        EmmcDeInit,
        EmmcSysProtSet,
        EmmcGetProductSn,
        MEM_DEV_EMMC,
        0,
        0
    },
    #endif
    #if defined(SDC0_DRIVER)
    {
        SDCardInit,
        SDCardRead,
        SDCardWrite,
        SDCardGetInfo,
        SDCardChkEvent,
        SDCardProbe,
        SDCardDeInit,
        NULL,
        NULL,
        MEM_DEV_SD,
        1,
        0
    },
    #endif
    #if defined(USBHOST_DRIVER)
    {
        USBMSCHost_Init,
        USBMSCHost_Read,
        USBMSCHost_Write,
        USBMSCHost_GetInfo,

        USBMSCHost_ChkEvent,
        USBMSCHost_Probe,
        USBMSCHost_DeInit,
        NULL,
        NULL,
        MEM_DEV_USBHOST,
        1,
        0
    }
    #endif
};

#define MEM_DEV_NUM  (sizeof(gMemDevTab)/sizeof(MEM_DEVICE))

_ATTR_SYS_DATA_
MEMDEV_ID UserDiskIDTab[MD_MAX_UDISK]=
{
    UserDisk0ID,
    UserDisk1ID,
    UserDisk2ID
};

_ATTR_SYS_DATA_
MEN_NOTIFIER    MDNotifier[MD_MAX_NOTIFY];

/*-------------------------------- Local Statics: ----------------------------*/

/*--------------------------- Local Function Prototypes ----------------------*/
static uint32 MDNotifyClient(uint16 major, uint32 event, uint32 param);

/*------------------------ Function Implement --------------------------------*/

/*
Name:       MemDevInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_INIT_CODE_
uint32 MemDevInit(void)
{
    uint32 i;
    uint32 ret =OK;
    pMEM_DEVICE pDev = (pMEM_DEVICE)&gMemDevTab[0];

    NandIOMuxRef =0;
    memset(MDNotifier, 0, sizeof(MDNotifier));

    for (i=0; i<MEM_DEV_NUM; i++, pDev++)
    {
        if (OK != pDev->init())
        {
            ret = ERROR;
        }
    }

    return ret;
}


/*
Name:       MDDeInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDDeInitAll(void)
{
    uint32 ret = OK;
    uint32 i;
    pMEM_DEVICE pDev = (pMEM_DEVICE)&gMemDevTab[0];

    for (i=0; i<MEM_DEV_NUM; i++, pDev++)
    {
        if (pDev->DeInit)
        {
            pDev->DeInit();
        }
    }

    return ret;

}

/*
Name:       MDGetDev
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


pMEM_DEVICE MDGetDev(MEMDEV_ID DevID)
{
    uint32 i;
    uint16 major  = MD_MAJOR(DevID);
    pMEM_DEVICE pDev = (pMEM_DEVICE)&gMemDevTab[0];

    for(i=0; i<MEM_DEV_NUM; i++, pDev++)
    {
        if (major == pDev->major)
        {
            return pDev;
        }
    }
    return NULL;
}

#ifdef _LOG_DEBUG_
/*
Name:       test code
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
uint32 MDLockFlag = 0;
void MDLock(void)
{
    if (MDLockFlag == 0)
    {
        MDLockFlag++;
    }
    else
    {
        printf("\nerror:   MDLockFlag  = %d, ", MDLockFlag);
        IsrEnableCheck();
        while(1);
    }
}

void MDUnLock(void)
{
    if (MDLockFlag > 0)
    {
        MDLockFlag--;
    }
    else
    {
        printf("\nerror:   MDLockFlag  = %d, ", MDLockFlag);
        IsrEnableCheck();
        while(1);
    }
}
#endif

/*
Name:       MDRead
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDRead(MEMDEV_ID DevID, uint32 SecAdrr, uint32 nSec, void* pBuf)
{
    pMEM_DEVICE pDev;
    uint32 ret;
    uint32 temp = (uint32)pBuf;

    #ifdef _LOG_DEBUG_
    if ((temp & 0x03) != 0)
    {
        DEBUG("pBuf addr illegale: pBuf = %d", (uint32)pBuf);
        while(1);
    }
    #endif

    pDev = MDGetDev(DevID);
    if (!pDev)
    {
        return ERROR;
    }

    UserIsrDisable();

    #ifdef _LOG_DEBUG_
    MDLock();
    #endif

    ret = pDev->read(MD_MKLUN(DevID), SecAdrr, nSec, pBuf);//MDSDMRead->SDM_Read
    if (OK != ret)
    {
        printf("\n MDRead ERR:LBA=0x%x, nSec=0x%x", SecAdrr, nSec);
    }

    #ifdef _LOG_DEBUG_
    MDUnLock();
    #endif

    UserIsrEnable();

    return ret;
}

/*
Name:       MDWrite
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDWrite(MEMDEV_ID DevID, uint32 SecAdrr, uint32 nSec, void* pBuf)
{
    pMEM_DEVICE pDev;

    uint32 ret;
    uint32 temp = (uint32)pBuf;

    #ifdef _LOG_DEBUG_
    if ((temp & 0x03) != 0)
    {
        DEBUG("pBuf addr illegale: pBuf = %d", (uint32)pBuf);
        while(1);
    }
    #endif

    pDev = MDGetDev(DevID);
    if (!pDev)
    {
        return ERROR;
    }

    UserIsrDisable();

    ret = pDev->write(MD_MKLUN(DevID), SecAdrr, nSec, pBuf);
    if (OK != ret)
    {
        printf("\n MDWrite ERR:LBA=0x%x, nSec=0x%x\n", SecAdrr, nSec);
    }

    UserIsrEnable();

    return ret;

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


uint32 MDGetInfo(MEMDEV_ID DevID, pMEMDEV_INFO pDevInfo)
{
    pMEM_DEVICE pDev;

     pDev = MDGetDev(DevID);
     if (!pDev)
         return ERROR;

     return pDev->GetInfo(MD_MKLUN(DevID), pDevInfo);//MDSDMGetInfo
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


uint32 MDGetCapacity(MEMDEV_ID DevID)
{
    MEMDEV_INFO DevInfo;

    if (OK == MDGetInfo(DevID, &DevInfo))
    {
        return DevInfo.capacity;
    }

    return 0;
}

/*
Name:       MDChkValid
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


bool MDChkValid(MEMDEV_ID DevID)
{
    pMEM_DEVICE pDev;

    pDev = MDGetDev(DevID);
    if (!pDev)
    {
        return FALSE;
    }

    if (pDev->removable)
    {
        return pDev->CheckEvent(MD_EVENT_VAILID);//MDSDMChkEvent
    }

    return TRUE;
}

/*
Name:       MDCheckChanged
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


bool MDChkPresent(MEMDEV_ID DevID)
{
    pMEM_DEVICE pDev;

    pDev = MDGetDev(DevID);
    if (!pDev)
         return FALSE;

    if (pDev->removable)
    {
        return pDev->CheckEvent(MD_EVENT_PRESENT);//MDSDMChkEvent
    }

    return TRUE;
}

/*
Name:       MDProbe
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDProbe(MEMDEV_ID DevID)
{
    pMEM_DEVICE pDev;

    pDev = MDGetDev(DevID);
    if (!pDev)
         return ERROR;

    if (pDev->probe)
    {
        return pDev->probe();//SDMCardProbe
    }

    return ERROR;
}

/*
Name:       MDScanDev
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDScanDev(uint8 AutoInit)
{
    uint32 i, ret;
    pMEM_DEVICE pDev = (pMEM_DEVICE)&gMemDevTab[0];

    for (i=0; i<MEM_DEV_NUM; i++, pDev++)
    {
        if (pDev->removable)
        {
            if (pDev->CheckEvent(MD_EVENT_CHANGE))
            {
                bool present;

                present = pDev->CheckEvent(MD_EVENT_PRESENT);//MDSDMChkEvent

                MDNotifyClient(pDev->major, MD_EVENT_CHANGE, present);

                if (present && AutoInit && !pDev->CheckEvent(MD_EVENT_VAILID))
                {
                    ret = pDev->probe();

                    MDNotifyClient(pDev->major, MD_EVENT_VAILID, (OK ==ret)? 1:0);
                }

            }
        }
    }

    return OK;
}

/*
Name:       MDGetUDiskTab
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDGetUDiskTab(MEMDEV_ID *pUDiskID, uint32 mask)
{
    uint32 i, num=0;

    for(i=0; i<MD_MAX_UDISK; i++)
    {
        if ((1<<i) & mask)
        {
            if (UserDiskIDTab[i])
            {
                num++;
                if (pUDiskID)
                {
                    *pUDiskID++ = UserDiskIDTab[i];
                }
            }
        }
    }

    return num;
}

/*
Name:       MDSyncIO
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDSyncIO(void)
{
    return OK;
}


/*
Name:       MDSysWPSet
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDSysWPSet(MEMDEV_ID DevID, uint32 magic)
{
    #ifdef SYS_PROTECT
    pMEM_DEVICE pDev;

    pDev = MDGetDev(DevID);
    if (!pDev)
         return ERROR;

    if (pDev->SysWPSet)
    {
        pDev->SysWPSet(magic);
    }
    #endif

    return OK;
}



int32 MDGetProductSn(MEMDEV_ID DevID, void *pSn)
{
    pMEM_DEVICE pDev;

    pDev = MDGetDev(DevID);
    if (!pDev)
         return ERROR;

    if (pDev->GetProductSN)
    {
        return pDev->GetProductSN(pSn);
    }
    else
        return ERROR;
}

#if 0
/*
Name:       MDRegDiskID
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/i


uint32 MDRegDiskID(uint32 DiskID)
{
    uint32 i;

    for(i=0; i<MD_MAX_UDISK; i++)
    {
        if (!UserDiskIDTab[i])
        {
            UserDiskIDTab[i] = DiskID;
            return OK;
        }
    }

    return ERROR;
}


/*
Name:       MDUnRegDiskID
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/i


uint32 MDUnRegDiskID(uint32 DiskID)
{
    uint32 i;

    for(i=0; i<MD_MAX_UDISK; i++)
    {
        if (DiskID == UserDiskIDTab[i])
        {
            UserDiskIDTab[i] = 0;
            return OK;
        }
    }

    return ERROR;
}
#endif

/*
Name:       MDRegNotify
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDRegNotify(MEN_NOTIFIER notify)
{
    uint32 i;

    for(i=0; i<MD_MAX_NOTIFY; i++)
    {
        if (!MDNotifier[i])
        {
            MDNotifier[i] = notify;
            return OK;
        }
    }

    return ERROR;
}

/*
Name:       MDUnRegNotify
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


uint32 MDUnRegNotify(MEN_NOTIFIER notify)
{
    uint32 i;

    for(i=0; i<MD_MAX_NOTIFY; i++)
    {
        if (notify == MDNotifier[i])
        {
            MDNotifier[i] = NULL;
            return OK;
        }
    }

    return ERROR;

}

/*
Name:       MDNotifyClient
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/


static uint32 MDNotifyClient(uint16 major, uint32 event, uint32 param)
{
    uint32 i;


    for(i=0; i<MD_MAX_NOTIFY; i++)
    {
        if (MDNotifier[i])
        {
            MDNotifier[i](major, event, param);
        }
    }
    return OK;
}

/*
Name:
Desc:       ���������̴�С����MemDevInit֮�����ϵ���
Param:      �����̴�С
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_INIT_CODE_
void SetDataDiskSize(int nMB)
{
    SysDataDiskCapacity =  (uint32)(nMB)<<11;
    //SysDataDiskCapacity = TS_SD_DATADISKCAPACITY;//1368056;
}

/*
Name:
Desc:       ���õڶ����û��̴�С����MemDevInit֮�����ϵ���
Param:      �����̴�С
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_INIT_CODE_
void SetUserDisk2Size(int nMB)
{
    SysUserDisk2Capacity =  (uint32)(nMB)<<11;
}

#if 0
uint32 BroDiskIDTab[MD_MAX_UDISK];
uint32 BroDiskNum;

void SampleNotify(uint16 major, uint32 event, uint32 param)
{
    uint32 i;

    if (MD_EVENT_CHANGE == event)
    {
        bool present = (bool)param;

        for (i=0; i<BroDiskNum; i++)
        {
            if (major==MD_MAJOR(BroDiskIDTab[i]))
            {
                if (present)
                {
                     //MDScanDev(0) manul init
                    if(!MDChkValid(BroDiskIDTab[i]))
                    {
                        if (OK == MDProbe(BroDiskIDTab[i]))
                        {
                            ;//display disk
                        }
                        else
                        {
                            ; //no display disk
                        }
                    }
                    else
                    {
                        ;//display disk
                    }
                }
                else
                {
                    ;//no display disk
                }
            }
        }

    }
    else if (MD_EVENT_VAILID == event) //MDScanDev(1) auto init
    {
        bool valid = (bool)param;

        for (i=0; i<BroDiskNum; i++)
        {
            if (major==MD_MAJOR(BroDiskIDTab[i]))
            {
                if (valid)
                {
                    ;//display disk
                }
                else
                {
                    ;// no display disk

                }

            }
        }

    }
    else
    {
        ;
    }

}

/*
Name:       MDSample1
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
void MDSample1(void)
{

    uint32 i, num;
    uint32 ret;

    BroDiskNum = MDGetUDiskTab(BroDiskIDTab);
    ret = MDRegNotify(SampleNotify);
    if (OK != ret)
    {
        while(1);
    }


    for (i=0; i<BroDiskNum; i++)
    {
        if (MDChkValid(BroDiskIDTab[i]))
        {
            ; //display disk
        }
    }

    MDUnRegNotify(SampleNotify);

}

#endif
