/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SFNor.c
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
#ifdef SPINOR_DRIVER
#include "SFC.h"
#include "SPIFlash.h"
#include "SFNor.h"

/*------------------------------------ Defines -------------------------------*/

//#define SNOR_TEST

#define NOR_PAGE_SIZE            256
#define NOR_BLOCK_SIZE          (64*1024)
#define NOR_SECS_BLK            (NOR_BLOCK_SIZE/512)
#define NOR_SECS_PAGE           4

/*----------------------------------- Typedefs -------------------------------*/
typedef int32 (*SNOR_WRITE_STATUS)(uint32 RegIndex, uint8 status);

typedef  struct tagSFNOR_DEV
{
    uint32          capacity;
    uint8           Manufacturer;
    uint8           MemType;
    uint16          PageSize;
    uint32          BlockSize;

    uint8           ReadCmd;
    uint8           ProgCmd;

    SNOR_READ_MODE  ReadMode;
    SNOR_ADDR_MODE  AddrMode;
    SNOR_IO_MODE    IOMode;

    SFC_DATA_LINES ReadLines;
    SFC_DATA_LINES ProgLines;

    SNOR_WRITE_STATUS WriteStatus;
}SFNOR_DEV, *pSFNOR_DEV;

/*-------------------------- Forward Declarations ----------------------------*/

/* ------------------------------- Globals ---------------------------------- */

/*-------------------------------- Local Statics: ----------------------------*/
static int32 SNOR_WaitBusy(int32 timeout);

/*--------------------------- Local Function Prototypes ----------------------*/

static SFNOR_DEV SFNorDev;

const SFLASH_DRIVER SFNorDrv =
{
    SNOR_Read,
    SNOR_Write,
    SNOR_EraseBlk,
    &SFNorDev,
};


const uint8 SFNorDevCode[] =
{
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x17,
    0x18,
    0x19
};

const uint32 SFNorCapacity[] =
{
    0x20000,        //128k-byte
    0x40000,        //256k-byte
    0x80000,        //512k-byte
    0x100000,       // 1M-byte
    0x200000,       // 2M-byte
    0x400000,       // 4M-byte
    0x800000,       // 8M-byte
    0x1000000,      //16M-byte
    0x2000000       // 32M-byte
};

/*------------------------ Function Implement --------------------------------*/

/*
Name:       SNOR_WriteEn
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_WriteEn(void)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_WRITE_EN;

    ret = SFC_Request(sfcmd.d32, 0, 0, NULL);

    return ret;
}

/*
Name:       SNOR_ReadStatus
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_ReadStatus(uint32 RegIndex, uint8 *status)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint8           ReadStatCmd[] = {CMD_READ_STATUS, CMD_READ_STATUS2, CMD_READ_STATUS3};

    sfcmd.d32 = 0;
    sfcmd.b.cmd = ReadStatCmd[RegIndex];
    sfcmd.b.datasize = 1;

    ret = SFC_Request(sfcmd.d32, 0, 0, status);

    return ret;
}

/*
Name:       SNOR_WriteStatus
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_WriteStatus2(uint32 RegIndex, uint8 status)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint8           status2[2];
    uint8           ReadIndex;

    status2[RegIndex] = status;
    ReadIndex = (RegIndex==0)? 1:0;
    ret = SNOR_ReadStatus(ReadIndex, &status2[ReadIndex]);
    if (ret != SFC_OK)
        return ret;

    SNOR_WriteEn();

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_WRITE_STATUS;
    sfcmd.b.datasize = 2;
    sfcmd.b.rw = SFC_WRITE;

    ret = SFC_Request(sfcmd.d32, 0, 0, &status2[0]);
    if (ret != SFC_OK)
        return ret;

    ret = SNOR_WaitBusy(10000);    // 10ms

    return ret;
}

/*
Name:       SNOR_ReadStatus
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_WriteStatus(uint32 RegIndex, uint8 status)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint8           WriteStatCmd[] = {CMD_WRITE_STATUS, CMD_WRITE_STATUS2, CMD_WRITE_STATUS3};

    SNOR_WriteEn();

    sfcmd.d32 = 0;
    sfcmd.b.cmd = WriteStatCmd[RegIndex];
    sfcmd.b.datasize = 1;
    sfcmd.b.rw = SFC_WRITE;

    ret = SFC_Request(sfcmd.d32, 0, 0, &status);
    if (ret != SFC_OK)
        return ret;

    ret = SNOR_WaitBusy(10000);    // 10ms

    return ret;
}


/*
Name:       SNOR_WaitBusy
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_WaitBusy(int32 timeout)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint32           i,status;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_READ_STATUS;
    sfcmd.b.datasize = 1;

    for (i=0; i<timeout; i++)
    {
        ret = SFC_Request(sfcmd.d32, 0, 0, &status);
        if (ret != SFC_OK)
            return ret;

        /*check the value of the Write in Progress (WIP) bit.The Write in Progress
        (WIP) bit is 1 during the self-timed Block Erase cycle, and is 0 when it is completed*/
        if ((status & 0x01) == 0)
            return SFC_OK;

        SFC_Delay(1);
    }

    return SFC_BUSY_TIMEOUT;

}

/*
Name:       SNOR_Erase
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
int32 SNOR_Erase(uint32 addr, NOR_ERASE_TYPE EraseType)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint8           EraseCmd[] = {CMD_SECTOR_ERASE, CMD_BLK64K_ERASE, CMD_CHIP_ERASE};
    int32           timeout[] = {10, 100, 20000};   //ms

    if (EraseType > ERASE_CHIP)
        return SFC_PARAM_ERR;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = EraseCmd[EraseType];
    sfcmd.b.addrbits = (EraseType != ERASE_CHIP)? SFC_ADDR_24BITS : SFC_ADDR_0BITS;

    SNOR_WriteEn();

    ret = SFC_Request(sfcmd.d32, 0, addr, NULL);
    if (ret != SFC_OK)
        return ret;

    ret = SNOR_WaitBusy(timeout[EraseType]*1000);

    return ret;
}

/*
Name:       SNOR_Erase
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
int32 SNOR_EraseBlk(uint32 addr)
{
    return SNOR_Erase(addr, ERASE_BLOCK64K);
}

/*
Name:       SNOR_ProgPage
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_ProgPage(uint32 addr, void *pData, uint32 size)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;
    pSFNOR_DEV      pDev = &SFNorDev;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = pDev->ProgCmd;
    sfcmd.b.addrbits = SFC_ADDR_24BITS;
    sfcmd.b.datasize = size;
    sfcmd.b.rw = SFC_WRITE;

    sfctrl.d32 = 0;
    sfctrl.b.datalines = pDev->ProgLines;
    sfctrl.b.enbledma = 1;
    if (pDev->ProgCmd == CMD_PAGE_PROG_A4)
    {
        sfctrl.b.addrlines = SFC_4BITS_LINE;;
    }

    SNOR_WriteEn();

    ret = SFC_Request(sfcmd.d32, sfctrl.d32, addr, pData);
    if (ret != SFC_OK)
        return ret;

    ret = SNOR_WaitBusy(10000);                 //10ms

    return ret;
}

/*
Name:       SNOR_Prog
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
int32 SNOR_Prog(uint32 addr, void *pData, uint32 size)
{
    int32           ret = SFC_OK;
    uint32          PageSize, len;
    uint8           *pBuf =  (uint8*)pData;

    PageSize = NOR_PAGE_SIZE;
    while (size)
    {
        len = MIN(PageSize, size);
        ret = SNOR_ProgPage(addr, pBuf, len);
        if (ret != SFC_OK)
            return ret;

        size -= len;
        addr += len;
        pBuf += len;
    }

    return ret;
}

/*
Name:       SNAND_EnableQE
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_EnableQE(void)
{
    int32 ret = SFC_OK;
    uint8 status;
    pSFNOR_DEV pDev = &SFNorDev;

    if (pDev->Manufacturer == MID_GIGADEV
        || pDev->Manufacturer == MID_WINBOND)
    {
        ret = SNOR_ReadStatus(1, &status);
        if (ret != SFC_OK)
            return ret;

        if (status & 0x2)   //is QE bit set
            return SFC_OK;

        status |= 0x2;

        return pDev->WriteStatus(1, status);
    }

    return ret;
}

/*
Name:       SNOR_SetIOMode
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_SetIOMode(SNOR_IO_MODE mode)
{
    pSFNOR_DEV pDev = &SFNorDev;

    pDev->IOMode = mode;

    return SFC_OK;

}

/*
Name:       SNOR_EnabeFast
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_SetReadMode(SNOR_READ_MODE mode)
{
    pSFNOR_DEV pDev = &SFNorDev;

    pDev->ReadMode = mode;

    return SFC_OK;
}

/*
Name:       SNOR_SetAddrMode
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_SetAddrMode(SNOR_ADDR_MODE mode)
{
    pSFNOR_DEV pDev = &SFNorDev;

    pDev->AddrMode = mode;

    return SFC_OK;

}

/*
Name:       SNOR_SetDLines
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
static int32 SNOR_SetDLines(SFC_DATA_LINES lines)
{
    int32           ret;
    pSFNOR_DEV      pDev = &SFNorDev;
    uint8           ReadCmd[] = {CMD_FAST_READ_X1, CMD_FAST_READ_X2, CMD_FAST_READ_X4/*CMD_FAST_READ_A4*/};

    if (pDev->ReadMode != READ_MODE_FAST) //����ģʽ��ʹ��Fast read mode
        return SFC_ERROR;

    if (lines == DATA_LINES_X4)
    {
        ret = SNOR_EnableQE();
        if (ret != SFC_OK)
            return ret;
    }

    pDev->ReadLines = lines;
    pDev->ReadCmd = ReadCmd[lines];

    if (pDev->Manufacturer == MID_GIGADEV || pDev->Manufacturer == MID_WINBOND
        || pDev->Manufacturer == MID_MACRONIX)
    {

        pDev->ProgLines = (lines != DATA_LINES_X2)? lines : DATA_LINES_X1;  //��֧�����߱��
        if (lines == DATA_LINES_X1)
            pDev->ProgCmd = CMD_PAGE_PROG;
        else
        {
            if (pDev->Manufacturer == MID_GIGADEV || pDev->Manufacturer == MID_WINBOND)
                pDev->ProgCmd = CMD_PAGE_PROG_X4;
            else
                pDev->ProgCmd = CMD_PAGE_PROG_A4;   //MID_MACRONIX
        }
    }

    return SFC_OK;
}

/*
Name:       SNOR_ReadData
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
int32 SNOR_ReadData(uint32 addr, void *pData, uint32 size)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;
    pSFNOR_DEV      pDev = &SFNorDev;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = pDev->ReadCmd;
    sfcmd.b.datasize = size;
    sfcmd.b.addrbits = SFC_ADDR_24BITS;

    sfctrl.d32 = 0;
    sfctrl.b.datalines = pDev->ReadLines;
    sfctrl.b.enbledma = 1;

    if (pDev->ReadCmd == CMD_FAST_READ_X1 || pDev->ReadCmd == CMD_FAST_READ_X4
        || pDev->ReadCmd == CMD_FAST_READ_X2)
    {
        sfcmd.b.dummybits = 8;
    }
    else if (pDev->ReadCmd == CMD_FAST_READ_A4)
    {
        sfcmd.b.addrbits = SFC_ADDR_32BITS;
        addr = (addr<<8) | 0xFF;                //Set M[7:0] = 0xFF
        sfcmd.b.dummybits = 4;
        sfctrl.b.addrlines = SFC_4BITS_LINE;
    }
    else
    {
        ;
    }

    ret = SFC_Request(sfcmd.d32, sfctrl.d32, addr, pData);

    return ret;
}

/*
Name:       SNOR_Read
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
int32 SNOR_Read(uint32 sec, uint32 nSec, void *pData)
{
    int32       ret = SFC_OK;
    uint32      addr, size, len;
    pSFNOR_DEV  pDev = &SFNorDev;
    uint8       *pBuf =  (uint8*)pData;

    if ((sec+nSec) > pDev->capacity)
        return SFC_PARAM_ERR;

    addr = sec<<9;
    size = nSec<<9;
    while(size)
    {
        len = MIN(size, SFC_MAX_IOSIZE);
        ret = SNOR_ReadData(addr, pBuf, len);
        if (ret != SFC_OK)
            break;

        size -= len;
        addr += len;
        pBuf += len;
    }

    return ret;
}

/*
Name:       SNOR_Write
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
int32 SNOR_Write(uint32 sec, uint32 nSec, void *pData)
{
    int32       ret = SFC_OK;
    uint32      addr, size, len ,BlockSize;
    pSFNOR_DEV  pDev = &SFNorDev;
    uint8       *pBuf =  (uint8*)pData;

    if ((sec+nSec) > pDev->capacity)
        return SFC_PARAM_ERR;

    addr = sec<<9;
    size = nSec<<9;
    BlockSize = pDev->BlockSize*512;        //BlockSize is 64K Bytes

    while (size)
    {
        if (!(addr & (BlockSize-1)))
        {
            ret = SNOR_Erase(addr, ERASE_BLOCK64K);
            if (ret != SFC_OK)
                return ret;
        }

        len = MIN(BlockSize, size);
        ret = SNOR_Prog(addr, pBuf, size);
        if (ret != SFC_OK)
            return ret;

        size -= len;
        addr += len;
        pBuf += len;
    }

    return ret;
}

/*
Name:       SNOR_ReadID
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
int32 SNOR_ReadID(uint8* data)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_READ_JEDECID;
    sfcmd.b.datasize = 3;

    ret = SFC_Request(sfcmd.d32, 0, 0, data);

    return ret;
}

/*
Name:       SNOR_GetCapacity
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
uint32 SNOR_GetCapacity(void)
{
    pSFNOR_DEV pDev = &SFNorDev;

    return pDev->capacity;
}

/*
Name:       SNOR_Init
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
int32 SNOR_Init(uint8* pFlashID, SFLASH_DRIVER **pDrv)
{
    int32 i;
    pSFNOR_DEV pDev = &SFNorDev;
    uint8 IDByte[3];
    uint8 *data;

    memset(pDev, 0, sizeof(SFNOR_DEV));
    //SFC_Init();

    if (pFlashID)
    {
        data = pFlashID;
    }
    else
    {
        data = IDByte;
        SNOR_ReadID(data);
    }

    if ((0xFF==data[0] && 0xFF==data[1]) || (0x00==data[0] && 0x00==data[1]))
    {
        return SFC_ERROR;
    }

    for(i=0; i<sizeof(SFNorDevCode); i++)
    {
        if (data[2] == SFNorDevCode[i])
        {
            pDev->capacity = SFNorCapacity[i]>>9;
            break;
        }
    }

    if (i >= sizeof(SFNorDevCode))
        return SFC_ERROR;

    pDev->Manufacturer = data[0];
    pDev->MemType = data[1];
    pDev->BlockSize = NOR_SECS_BLK;
    pDev->PageSize = NOR_SECS_PAGE;

    pDev->ReadCmd = CMD_READ_DATA;
    pDev->ProgCmd = CMD_PAGE_PROG;

    pDev->WriteStatus = SNOR_WriteStatus2;      //��ͬ������, д״̬�Ĵ����ķ�ʽ���ܲ�һ��

    if (pDrv)
        *pDrv = (SFLASH_DRIVER *)&SFNorDrv;

    return SFC_OK;
}


#ifdef SNOR_TEST

#define SNOR_READ_ADDR 0x03051000
extern uint32 TestBin[];

/*
Name:       SNAND_UpdateFw
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
void SNOR_UpdateFw(void)
{
    int32 ret;
    uint32 *pFirmware, *pReadBuf;
    uint32  size;
    uint32 i, j, addr;
    //pSFNAND_DEV pDev = &SFNandDev;
    //uint8 status = 0;

    pFirmware = (uint32 *)TestBin;
    size = 20*1024;
    pReadBuf = (uint32 *)SNOR_READ_ADDR;

    addr = 0;
    for (i=0; i<(size>>9); i+= 4)
    {
        ret = SNOR_Write(addr+i, 4, pFirmware);
        if (ret != SFC_OK)
            while(1);

        ret = SNOR_Read(addr+i, 4, pReadBuf);
        if (ret != SFC_OK)
            while(1);

        for (j=0; j<512; j++)
        {
            if (pReadBuf[j] != pFirmware[j])
            {
                while(1);
            }
        }
        pFirmware += 512;
    }

    while(1);
}


/*
Name:       SNOR_Test
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
void SNOR_Test(void)
{
    int32 ret;
    uint32 i, j, addr;
    uint32 *pReadBuf;
    uint32 *pWriteBuf;
    //uint8  status;
    //pSFNOR_DEV pDev = &SFNorDev;

    ret = SNOR_Init(NULL, NULL);
    if (ret != SFC_OK)
        while(1);

    SNOR_UpdateFw();

    SNOR_SetReadMode(READ_MODE_FAST);

    //ret = SNOR_SetDLines(DATA_LINES_X4);
    //if (ret != SFC_OK)
    //    while(1);

    pReadBuf = (uint32 *)(SDRAM_ADDR+0x100000);//INIT_CODE_ADDR;
    pWriteBuf = pReadBuf+512;

    memset(pReadBuf, 0, 2048);
    memset(pWriteBuf, 0xFF, 2048);


    ret = SNOR_Erase(0, ERASE_SECTOR);
    if (ret != SFC_OK)
        while(1);

    ret = SNOR_Read(0, 4, pReadBuf);
    if (ret != SFC_OK)
        while(1);

    for (j=0; j<512; j++)
    {
        if (pReadBuf[j] != 0xFFFFFFFF)
        {
            while(1);
        }
    }

    for (i=0; i<512; i++)
        pWriteBuf[i] = i;

    addr = 0;
    for (i=0; i<128; i+= 4)
    {
        ret = SNOR_Write(addr+i, 4, pWriteBuf);
        if (ret != SFC_OK)
        	while(1);

        ret = SNOR_Read(addr+i, 4, pReadBuf);
        if (ret != SFC_OK)
        	while(1);

        for (j=0; j<512; j++)
        {
            if (pReadBuf[j] != pWriteBuf[j])
            {
                SNOR_SetDLines(DATA_LINES_X1);
                SNOR_Read(addr+i, 1, pReadBuf);
                while(1);

            }
        }
    }

    while(1);
}
#endif

#endif
