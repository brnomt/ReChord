/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SFNand.c
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


#ifdef SPINAND_DRIVER
#include "SFC.h"
#include "SPIFlash.h"
#include "SFNand.h"

/*------------------------------------ Defines -------------------------------*/

//#define SNAND_TEST

#define NAND_PAGE_SIZE              (2*1024)
#define NAND_BLOCK_SIZE             (NAND_PAGE_SIZE*64)
#define NAND_SEC_PER_PAGE           (NAND_PAGE_SIZE/512)
#define NAND_SEC_PER_BLK            (NAND_BLOCK_SIZE/512)

#define STATUS_REG_ADDR             (0xC0)
#define FEATURE_REG_ADDR            (0xB0)
#define PROTECTION_REG_ADDR         (0xA0)

#define STATUS_P_FAIL               (1<<3)
#define STATUS_E_FAIL               (1<<2)
#define STATUS_ECC_FAIL             (1<<5)

/*----------------------------------- Typedefs -------------------------------*/


typedef  struct tagSFNAND_DEV
{
    uint32              capacity;                   //sec unit
    uint16              PageSecs;                   //sec uint
    uint16              BlockSecs;                  //sec uint
    uint8               Manufacturer;
    uint8               DevID;

    uint8               ReadCmd;
    uint8               ProgCmd;
    SFC_DATA_LINES      ReadLines;                  //Read Data Lines               
    SFC_DATA_LINES      ProgLines;                  //Write Data Lines    

}SFNAND_DEV, *pSFNAND_DEV;

/*-------------------------- Forward Declarations ----------------------------*/

/* ------------------------------- Globals ---------------------------------- */


/*-------------------------------- Local Statics: ----------------------------*/
static SFNAND_DEV SFNandDev;

const SFLASH_DRIVER SFNandDrv = 
{
    SNAND_Read,
    SNAND_Write,
    SNAND_Erase,
    &SFNandDev
};

const uint8 SFNandDevCode[] = 
{
    0x12,                   //MT29F1G01AAADD
    0xF1,                   //GD5F1GQ4UAYIG
    0xAA                    //W25N01GV
};

const uint32 SFNandCapacity[] = 
{
    0x40000,                   // 1Gbits
    0x40000,                   // 1Gbits
    0x40000                    // 1Gbits
};


#ifdef SNAND_TEST
//uint32 TestReadBuf[2048];
//uint32 TestWriteBuf[2048];
#endif
/*--------------------------- Local Function Prototypes ----------------------*/

/*------------------------ Function Implement --------------------------------*/

/*
Name:       SNAND_WriteEn
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
static int32 SNAND_WriteEn(void)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;

    sfcmd.d32 = 0; 
    sfcmd.b.cmd = CMD_WRITE_EN;

    ret = SFC_Request(sfcmd.d32, 0, 0, NULL);
    
    return ret;
}

/*
Name:       SNAND_GetFeature
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_GetFeature(uint32 addr, uint32 *status)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;

    sfcmd.d32 = 0; 
    sfcmd.b.cmd = CMD_GET_FEATURE;
    sfcmd.b.datasize = 1;
    sfcmd.b.addrbits = SFC_ADDR_XBITS;

    sfctrl.d32 = 0;
    sfctrl.b.addrbits = 8;
    
    ret = SFC_Request(sfcmd.d32, sfctrl.d32, addr, status);
    
    return ret;
}


/*
Name:       SNAND_SetFeature
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_SetFeature(uint32 addr, uint32 feature)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;

    sfcmd.d32 = 0; 
    sfcmd.b.cmd = CMD_SET_FEATURE;
    sfcmd.b.datasize = 1;
    sfcmd.b.rw = SFC_WRITE;
    sfcmd.b.addrbits = SFC_ADDR_XBITS;

    sfctrl.d32 = 0;
    sfctrl.b.addrbits = 8;
    
    ret = SFC_Request(sfcmd.d32, sfctrl.d32, addr, &feature);
    
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
static int32 SNAND_EnableQE(void)
{
    int32 ret = SFC_OK;
    uint32 feature;
    pSFNAND_DEV pDev = &SFNandDev;

    if (MID_GIGADEV != pDev->Manufacturer)
        return SFC_OK;

    ret = SNAND_GetFeature(FEATURE_REG_ADDR, &feature);
    if (ret != SFC_OK)
        return ret;

    if (feature & 0x01)
        return SFC_OK;

    feature |= 0x1;
    ret = SNAND_SetFeature(FEATURE_REG_ADDR, feature);

    return ret;
}

/*
Name:       SNAND_WaitBusy
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
static int32 SNAND_WaitBusy(int32 timeout, uint8 *pStatus)
{
    int32           i, ret;
    uint32           status;

    for (i=0; i<timeout; i++)
    {
        ret = SNAND_GetFeature(STATUS_REG_ADDR, &status);
        if (ret != SFC_OK)
            return ret;

        /*check the value of the Write in Progress (WIP) bit.*/
        if ((status & 0x01) == 0) 
        {
            if (pStatus)
                *pStatus = (uint8)status;
            return SFC_OK;
        }

        SFC_Delay(1);
    }

    return SFC_BUSY_TIMEOUT;
}


/*
Name:       SNAND_Erase
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_Erase(uint32 RowAddr)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint8           status;
    
    /* 1.WRITE ENBALE command*/
    SNAND_WriteEn();

    /*BLOCK ERASE command*/
    sfcmd.d32 = 0; 
    sfcmd.b.cmd = CMD_BLOCK_ERASE;
    sfcmd.b.addrbits = SFC_ADDR_24BITS;

    ret = SFC_Request(sfcmd.d32, 0, RowAddr, NULL);

    /* 4.GET FEATURE command to read the status*/
    ret = SNAND_WaitBusy(20000, &status);       // 20ms
    if (ret != SFC_OK)
        return ret;

    if (STATUS_E_FAIL & status)
        ret = SFC_ERASE_FAIL;
    
    return ret;
}

/*
Name:       SNAND_ProgPage
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_ProgPage(uint32 RowAddr, void *pData, uint32 size)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;
    uint8           status;
    pSFNAND_DEV     pDev = &SFNandDev;

    /* 1.WRITE ENABLE*/
    SNAND_WriteEn();

    /* 2.PROGRAM LOAD DATA*/
    sfcmd.d32 = 0; 
    sfcmd.b.cmd = pDev->ProgCmd;
    sfcmd.b.datasize = size;
    sfcmd.b.rw = SFC_WRITE;
    sfcmd.b.addrbits = SFC_ADDR_XBITS;

    sfctrl.d32 = 0;
    sfctrl.b.addrbits = 16;
    sfctrl.b.datalines = pDev->ProgLines;
    sfctrl.b.enbledma = 1;
    ret = SFC_Request(sfcmd.d32, sfctrl.d32, 0, pData);     
    if (ret != SFC_OK)
        return ret;

    /* 3.PROGRAM EXECUTE*/
    sfcmd.d32 = 0; 
    sfcmd.b.cmd = CMD_PROG_EXEC;
    sfcmd.b.addrbits = SFC_ADDR_24BITS;
    ret = SFC_Request(sfcmd.d32, 0, RowAddr, NULL);
    if (ret != SFC_OK)
        return ret;

    /* 4.GET FEATURE command to read the status*/
    ret = SNAND_WaitBusy(50000, &status);       // 50ms
    if (ret != SFC_OK)
        return ret;

    if (STATUS_P_FAIL & status)
        ret = SFC_PROG_FAIL;

    return ret;
}

/*
Name:       SNAND_ReadPage
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_ReadPage(uint32 RowAddr, uint32 ColAddr, void *pData, uint32 size)
{
    int32           ret, ret1 = SFC_OK;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;
    uint8           status;
    pSFNAND_DEV     pDev = &SFNandDev;

    /* 1.PAGE READ to cache*/
    sfcmd.d32 = 0; 
    sfcmd.b.cmd = CMD_PAGE_READ;
    sfcmd.b.addrbits = SFC_ADDR_24BITS;
    ret = SFC_Request(sfcmd.d32, 0, RowAddr, NULL);
    if (ret != SFC_OK)
        return ret;

    /* 2.GET FEATURES command to read the status*/
    ret = SNAND_WaitBusy(1000, &status);       // 1ms
    if (ret != SFC_OK)
        return ret;

    if (STATUS_ECC_FAIL & status)
        ret1 = SFC_ECC_FAIL;

    /* 3.RANDOM DATA READ*/
    sfcmd.d32 = 0; 
    sfcmd.b.cmd = pDev->ReadCmd;
    sfcmd.b.dummybits = 8;
    sfcmd.b.datasize = size;
    sfcmd.b.addrbits = SFC_ADDR_XBITS;

    sfctrl.d32 = 0;
    sfctrl.b.addrbits = 16;
    sfctrl.b.datalines = pDev->ReadLines;
    sfctrl.b.enbledma = 1;
    ret = SFC_Request(sfcmd.d32, sfctrl.d32, ColAddr, pData);

    return (ret1 == SFC_OK)? ret : ret1;
}

/*
Name:       SNAND_Read
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_Read(uint32 sec, uint32 nSec, void *pData)
{
    int32           ret  = SFC_OK;
    uint32      len, offset;
    pSFNAND_DEV pDev = &SFNandDev;
    uint8       *pBuf =  (uint8*)pData;

    if ((sec+nSec) > pDev->capacity)
        return SFC_PARAM_ERR;
    
    offset = sec & (pDev->PageSecs-1);
    if (offset)
    {
        len = MIN(pDev->PageSecs-offset, nSec);
        ret = SNAND_ReadPage(sec/pDev->PageSecs, offset<<9, pData, len<<9);
        if (ret != SFC_OK)
            return ret;

        nSec -= len;
        sec += len;
        pBuf += len;
    }

    while (nSec)
    {
        len = MIN(pDev->PageSecs, nSec);
        ret = SNAND_ReadPage(sec/pDev->PageSecs, 0, pData, len<<9);
        if (ret != SFC_OK)
            return ret;

        nSec -= len;
        sec += len;
        pBuf += len;
    }

    return ret;
}

/*
Name:       SNAND_Write
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_Write(uint32 sec, uint32 nSec, void *pData)
{
    int32       ret = SFC_OK;
    uint32      len, BlockSecs, PageSecs;
    pSFNAND_DEV pDev = &SFNandDev;
    uint8       *pBuf =  (uint8*)pData;

    if ((sec+nSec) > pDev->capacity)
        return SFC_PARAM_ERR;


    BlockSecs = pDev->BlockSecs;        //BlockSize is 4K Bytes
    PageSecs = pDev->PageSecs; 
    
    while (nSec)
    {
        if (!(sec & (BlockSecs-1)))  
        {
            ret = SNAND_Erase(sec/PageSecs); 
            if (ret != SFC_OK)
                return ret;    
        }

        len = MIN(PageSecs, nSec);
        ret = SNAND_ProgPage(sec/PageSecs, pData, len<<9);
        if (ret != SFC_OK)
            return ret;            

        nSec -= len;
        sec += len;
        pBuf += len;
    }

    return ret;


}

/*
Name:       SNAND_ReadID
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_ReadID(uint8* data)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;

    sfcmd.d32 = 0; 
    sfcmd.b.cmd = CMD_READ_JEDECID;
    sfcmd.b.datasize = 3;               //ID �ֽ�����2-3B, ��ͬ���̲�һ��
    sfcmd.b.addrbits = SFC_ADDR_XBITS;
    //sfcmd.b.dummybits = 8;
    sfctrl.d32 = 0;
    sfctrl.b.addrbits = 8;
    //sfctrl.b.scic = 1;
    
    ret = SFC_Request(sfcmd.d32, sfctrl.d32, 0, data);
    
    return ret;
}


/*
Name:       SNAND_SetDLines
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_SetDLines(SFC_DATA_LINES lines)
{
    int32 ret;
    pSFNAND_DEV pDev = &SFNandDev;
    uint8           ReadCmd[] = {CMD_FAST_READ_X1, CMD_FAST_READ_X2, CMD_FAST_READ_X4/*CMD_FAST_READ_A4*/};

    //if (pDev->ReadMode != READ_MODE_FAST) //����ģʽ��ʹ��Fast read mode
    //    return SFC_ERROR;

    if (lines == DATA_LINES_X4)
    {
        ret = SNAND_EnableQE();
        if (ret != SFC_OK)
            return ret;
    }

    pDev->ReadLines = lines;
    pDev->ReadCmd = ReadCmd[lines];

    if (pDev->Manufacturer == MID_GIGADEV || pDev->Manufacturer == MID_WINBOND)
    {
        pDev->ProgLines = (lines != DATA_LINES_X2)? lines : DATA_LINES_X1;  //��֧�����߱��
        if (lines == DATA_LINES_X1)
            pDev->ProgCmd = CMD_PAGE_PROG;
        else
        {
            pDev->ProgCmd = CMD_PAGE_PROG_X4;
        }
    }

    return SFC_OK;
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
uint32 SNAND_GetCapacity(void)
{
    pSFNAND_DEV pDev = &SFNandDev;
    
    return pDev->capacity;
}

/*
Name:       SNAND_Reset
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_Reset(void)
{
    int32 ret;

    SFCCMD_DATA         sfcmd;
    /*reset spi nand*/
    sfcmd.d32 = 0; 
    sfcmd.b.cmd = CMD_RESET_NAND;
    ret = SFC_Request(sfcmd.d32, 0, 0, NULL);
    if (ret != SFC_OK)
        return ret;  

    DelayUs(2000);
    return ret;
}

/*
Name:       SNAND_Init
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
int32 SNAND_Init(uint8* pFlashID, SFLASH_DRIVER **pDrv)
{
    int32 i;
    //int32 ret;
    //SFCCMD_DATA         sfcmd;
    pSFNAND_DEV pDev = &SFNandDev;
    uint8 IDByte[3];
    uint8 *data;

    memset(pDev, 0, sizeof(pSFNAND_DEV));
    //SFC_Init();
    if (pFlashID)
    {

        data = pFlashID;
    }
    else
    {
        SNAND_Reset();          //ĿǰֻҪMICROM ��SPI NAND ����ҪReset
        data = IDByte;
        SNAND_ReadID(data);
    }

    if ((0xFF==data[0] && 0xFF==data[1]) || (0x00==data[0] && 0x00==data[1]))
    {
        return SFC_ERROR;
    }
    
    for(i=0; i<sizeof(SFNandDevCode); i++)
    {
        if (data[1] == SFNandDevCode[i])
        {
            pDev->capacity = SFNandCapacity[i];
            break;
        }
    }

    if (i >= sizeof(SFNandDevCode))
        return SFC_ERROR;

    pDev->Manufacturer = data[0];
    pDev->DevID = data[1];
    //pDev->BlockSize = NAND_BLOCK_SIZE;
    //pDev->PageSize = NAND_PAGE_SIZE;
    pDev->PageSecs = NAND_SEC_PER_PAGE;
    pDev->BlockSecs = NAND_SEC_PER_BLK;

    pDev->ReadLines = pDev->ProgLines = DATA_LINES_X1;
    pDev->ReadCmd = CMD_READ_DATA;
    pDev->ProgCmd = CMD_PAGE_PROG;

    if (pDrv)
        *pDrv = (SFLASH_DRIVER *)&SFNandDrv;
    
    return SFC_OK;

}

#ifdef SNAND_TEST

#define SNAND_READ_ADDR 0x03051000
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
void SNAND_UpdateFw(void)
{
    int32 ret;
    uint32 *pFirmware, *pReadBuf;
    uint32  size;
    uint32 i, j, addr;
    //pSFNAND_DEV pDev = &SFNandDev;
    uint32 status = 0;
    

    pFirmware = (uint32 *)TestBin;
    size = 20*1024;
    pReadBuf = (uint32 *)SNAND_READ_ADDR;

    SNAND_GetFeature(PROTECTION_REG_ADDR, &status);
    status = 0;
    SNAND_SetFeature(PROTECTION_REG_ADDR, status);
    SNAND_GetFeature(PROTECTION_REG_ADDR, &status);

    addr = 0;
    for (i=0; i<(size>>9); i+= 4)
    {
        ret = SNAND_Write(addr+i, 4, pFirmware);
        if (ret != SFC_OK)
            while(1);
            
        memset(pReadBuf, 0, 2048);
        ret = SNAND_Read(addr+i, 4, pReadBuf);
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
Name:       SNAND_Init
Desc:       
Param:      
Return:     
Global: 
Note:   
Author: 
Log:
*/
void SNAND_Test(void)
{
    int32 ret;
    uint32 i, j, addr;
    uint32 *pReadBuf;
    uint32 *pWriteBuf;
    uint32  status;
    pSFNAND_DEV pDev = &SFNandDev;

    ret = SNAND_Init(NULL, NULL);
    if (ret != SFC_OK)
        while(1);

    SNAND_UpdateFw();

    pReadBuf = (uint32 *)SDRAM_ADDR;//INIT_CODE_ADDR;
    pWriteBuf = pReadBuf+512;

    memset(pReadBuf, 0, 2048);
    memset(pWriteBuf, 0xFF, 2048);

   
    ret = SNAND_SetDLines(DATA_LINES_X4);
    if (ret != SFC_OK)
        while(1);

    SNAND_GetFeature(STATUS_REG_ADDR, &status);

    SNAND_GetFeature(FEATURE_REG_ADDR, &status);

    SNAND_GetFeature(PROTECTION_REG_ADDR, &status);

    status = 0;
    SNAND_SetFeature(PROTECTION_REG_ADDR, status);
    
    SNAND_GetFeature(PROTECTION_REG_ADDR, &status);

    addr = 0*pDev->BlockSecs;
    SNAND_Erase(addr);

    
    for (i=0; i<pDev->BlockSecs; i+=4)
    {
        ret = SNAND_Read(addr+i, 4, pReadBuf);
        if (ret != SFC_OK)
            while(1);

        for (j=0; j<512; j++)
        {
            if (pReadBuf[j] != 0xFFFFFFFF)
            {
                while(1);
            }   
        }            
    }

    for (i=0; i<512; i++)
        pWriteBuf[j] = i;

    for (i=0; i<pDev->BlockSecs; i+= 4)
    {
        ret = SNAND_Write(addr+i, 4, pWriteBuf);
        if (ret != SFC_OK)
        	while(1);
    
        ret = SNAND_Read(addr+i, 4, pReadBuf);
        if (ret != SFC_OK)
        	while(1);

        for (j=0; j<512; j++)
        {
            if (pReadBuf[j] != pWriteBuf[j])
            {
                while(1);
            }   
        }  
    }

    while(1);
}
#endif

#endif
