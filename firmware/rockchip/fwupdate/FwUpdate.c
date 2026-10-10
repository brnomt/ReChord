/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: FwUpdate.c
Desc:

Author: chenfen
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/

#include "SysConfig.h"
#include "MDConfig.h"
#include "ModuleOverlay.h"
#include "FwUpdate.h"

#ifdef FW_UPDATE

/*----------------------------------- Typedefs -------------------------------*/
#define _ATTR_FW_UPGRADE_CODE_    
#define _ATTR_FW_UPGRADE_DATA_    
#define _ATTR_FW_UPGRADE_BSS_     

/*------------------------------------ Defines -------------------------------*/
//#define  _FW_CRC_

#define FW_CHECK_LEN        (8*1024)

#ifdef _SPINOR_
#define FW_BUF_LEN          (FW_CHECK_LEN*8)
#else
#define FW_BUF_LEN          (FW_CHECK_LEN*1)
#endif

#define UPDATE_FILE_NANE    "RKNANOFWIMG"


/*-------------------------- Forward Declarations ----------------------------*/
static int32 FwRecovery(uint32 DstAddr, uint32 SrcAddr, uint32 FwSize);

/* ------------------------------- Globals ---------------------------------- */

/*-------------------------------- Local Statics: ----------------------------*/
_ATTR_FW_UPGRADE_BSS_ uint32 FwUpdateBuf[FW_BUF_LEN/4];
_ATTR_FW_UPGRADE_BSS_ uint32 FwCheckBuf[FW_CHECK_LEN/4];

/*
Name:       FwCheck
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SYS_INIT_CODE_
int32 FwCheck(void)
{
    uint8 TmpBuf1[512];
    uint8 TmpBuf2[512];
    PFIRMWARE_HEADER pFWHead1, pFWHead2;
    unsigned char FwSign[8] = {'R','K','n','a','n','o','F','W'};
    uint32 i, addr, FwSize;
    int32  FW1Valid, FW2Valid;

    ValidSysDisk = 0;
    FW1Valid = FW2Valid = 0;
    pFWHead1 = (PFIRMWARE_HEADER)TmpBuf1;
    pFWHead2 = (PFIRMWARE_HEADER)TmpBuf2;

    addr = 0;
    MDRead(SysDiskID, addr, 1, TmpBuf1);       //�ȼ���һ�ݹ̼�
    //printf("Fw1Sign1 = %s\n", pFWHead1->FwSign);

    if (0 == memcmp(pFWHead1->FwSign, FwSign, 8))
    {
        MDRead(SysDiskID, addr+(pFWHead1->FwEndOffset>>9), 1, TmpBuf2);
        //printf("Fw1Sign2 = %s\n", pFWHead2->FwSign);

        if (0 != memcmp(TmpBuf1, TmpBuf2, 512))
        {
            printf("fw1 compare error!\n");
        }
        else
            FW1Valid = 1;

    }
    else
    {
        printf("fw1 Sign error!\n");
    }

    //printf("SysProgRawDiskCapacity = 0x%x\n", SysProgRawDiskCapacity);

    if (1==FW1Valid)
        //addr += (((pFWHead1->FwEndOffset + 512 /*16*1024*/ + FW_ALIGN_SIZE - 1)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE)>>9;
        addr += ((((pFWHead1->FwEndOffset + 16*1024 + FW_ALIGN_SIZE)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE)>>9);
    else
        addr += SysProgRawDiskCapacity;
    for(i = 0; i < 20, addr > 0; i++) //���ҵڶ��ݹ̼�,�̼����ܱ��
    {
        //DEBUG("i = %d", i);
        MDRead(SysDiskID, addr, 1, TmpBuf2);
        if (0 != memcmp(pFWHead2->FwSign, FwSign, 8))
        {
            (1==FW1Valid)? (addr += (FW_ALIGN_SIZE>>9)) : (addr -= (FW_ALIGN_SIZE>>9));
        }
        else
        {
            MDRead(SysDiskID, addr+(pFWHead2->FwEndOffset>>9), 1, TmpBuf1);
            if (0 != memcmp(TmpBuf2, TmpBuf1, 512))
                printf("\n fw2 compare error! 0x%x\n", addr);
            else
            {
                FW2Valid = 1;
                DEBUG("FW2Valid = 1",);
            }

            break;
        }

    }

    if (i >= 20 || addr <= 0)
    {
        printf("No find fw2!\n");
    }

    if (0==FW1Valid && 0==FW2Valid)    //���ݶ�����
    {
        printf("fw1 && fw2 error!\n");
        return -1;
    }

    if (0==FW2Valid) //�ڶ����г���
    {
        printf("fw2 error!\n");
        //SysProgRawDiskCapacity = (((pFWHead1->FwEndOffset + 512 /*16*1024*/ + FW_ALIGN_SIZE - 1)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE)>>9;    //��M����,��secΪ��λ
        SysProgRawDiskCapacity = ((((pFWHead1->FwEndOffset + 16*1024 + FW_ALIGN_SIZE)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE)>>9);    //��M����,��secΪ��λ
    }
    else
    {
        SysProgRawDiskCapacity = addr;
    }

    if (SysProgDiskCapacity < (SysProgRawDiskCapacity<<1))
    {
        #ifdef _SPINOR_
        SysProgDiskCapacity = (SysProgRawDiskCapacity<<1);
        #else
        SysProgDiskCapacity = (SysProgRawDiskCapacity<<1)+4*2048;   // +4M, ����̼������ļ�ϵͳ����
        #endif
    }

    if (1==FW1Valid && 1==FW2Valid)
    {
        MDRead(SysDiskID, 0, 1, TmpBuf1);
        if (0 != memcmp(TmpBuf1, TmpBuf2, 512))
        {
            if (((pFWHead2->Year<<16)+pFWHead2->Date) > ((pFWHead1->Year<<16)+pFWHead1->Date))  //���ڱȽ��µĹ̼�����Ч��
                FW1Valid = 0;
            else
                FW2Valid = 0;
        }
    }

    if (0 == FW1Valid)
    {
        ValidSysDisk = 1;
    }

    //DEBUG("FailFlag = %d",FailFlag);
    if (1==FW1Valid && 1==FW2Valid)
        return 0;
    else
    {
        printf("fw1valid = %d fw2valid = %d \n",FW1Valid,FW2Valid);
        return -1;
    }
}

/*
Name:       FwWrite
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FW_UPGRADE_CODE_
static int32 FwWrite(uint32 SecAdrr, uint32 nSec, void* pWBuf)
{
    uint8 *pRBuf = (uint8*)FwCheckBuf;
    int32 ret = -1;
    uint32 len, CheckSecs = 0;
    uint8* pData = (uint8*)pWBuf;

    //DEBUG("SysDiskID = 0x%08x",SysDiskID);
    MDSysWPSet(SysDiskID, FLASH_PROT_MAGIC);
REPEAT:
    if (OK != MDWrite(SysDiskID, SecAdrr, nSec, pWBuf))
    {
        DEBUG();
        goto FWRITE_ERR;
    }

    while (nSec)
    {
        len = MIN((FW_CHECK_LEN>>9), nSec);
        MDRead(SysDiskID, SecAdrr+CheckSecs, len, pRBuf);
        if (0 != memcmp(pData+(CheckSecs<<9), pRBuf, len<<9))
        {
            DEBUG("memcmp Error!");
            goto FWRITE_ERR;
        }

        CheckSecs += len;
        nSec -= len;
    }

    ret = 0;

FWRITE_ERR:
    MDSysWPSet(SysDiskID, 0);

    return ret;
}

/*
Name:       FwRecovery
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FW_UPGRADE_CODE_
static int32 FwRecovery(uint32 DstAddr, uint32 SrcAddr, uint32 FwSize)
{
    int32 ret = -1;
    uint32 i;
    uint8 *pUBuf = (uint8 *)&FwUpdateBuf[0];

    DEBUG("FwRecovery Enter: addr = %d", DstAddr);

    memset (pUBuf, 0xFF, FW_BUF_LEN);
    FwWrite(DstAddr, FW_BUF_LEN>>9, pUBuf);                   //������̼�ͷ

    SrcAddr += (FW_BUF_LEN>>9);                                //�ӹ̼���FW_BUF_LENλ�ÿ�ʼ����
    DstAddr += (FW_BUF_LEN>>9);

    for(i = 0; i<(FwSize-FW_BUF_LEN); i+=FW_BUF_LEN)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        //DEBUG("i = %d", i);
        if (OK != MDRead(SysDiskID, SrcAddr, FW_BUF_LEN>>9, pUBuf))
        {
            DEBUG("Error!");
            return -1;
        }
        ret = FwWrite(DstAddr, FW_BUF_LEN>>9, pUBuf);
        if (ret < 0)
        {
            DEBUG("Error!");
            return ret;
        }

        SrcAddr += (FW_BUF_LEN>>9);                                //�ӹ̼���FW_BUF_LENλ�ÿ�ʼ����
        DstAddr += (FW_BUF_LEN>>9);
    }

    SrcAddr -= (FwSize>>9);
    DstAddr -= (FwSize>>9);
    if (OK != MDRead(SysDiskID, SrcAddr, FW_BUF_LEN>>9, pUBuf))
    {
        DEBUG("Error!");
        return -1;
    }

    ret = FwWrite(DstAddr, FW_BUF_LEN>>9, pUBuf);             //��д��һ�ݹ̼�ͷ
    if (ret < 0)
    {
        DEBUG("Error!");
        return ret;
    }

    DEBUG("FwRecovery Exit");
    return 0;
}

/*
Name:       FwUpdate
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FW_UPGRADE_CODE_
int32 FwUpgradeRecovery(void)
{
    uint32 SrcAddr, DstAddr;
    int32  ret;

    DEBUG("Enter");

    //ValidSysDisk: 0--fw1 ok;   1--fw2 ok
    (1 == ValidSysDisk)? (DstAddr=0,SrcAddr=SysProgRawDiskCapacity):(DstAddr=SysProgRawDiskCapacity,SrcAddr=0);

    ret = FwRecovery(DstAddr, SrcAddr, SysProgRawDiskCapacity<<9);
    if (!ret)
    {
        DEBUG("Delete fwfile!");
        FileDelete("\\", UPDATE_FILE_NANE);
    }

    DEBUG("Exit");
    return ret;
}

/*
Name:       gTable_Crc32
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
#ifdef _FW_CRC_
_ATTR_FW_UPGRADE_DATA_
unsigned long gTable_Crc32[256]={
    0x00000000,0x04c10db7,0x09821b6e,0x0d4316d9,
    0x130436dc,0x17c53b6b,0x1a862db2,0x1e472005,
    0x26086db8,0x22c9600f,0x2f8a76d6,0x2b4b7b61,
    0x350c5b64,0x31cd56d3,0x3c8e400a,0x384f4dbd,
    0x4c10db70,0x48d1d6c7,0x4592c01e,0x4153cda9,
    0x5f14edac,0x5bd5e01b,0x5696f6c2,0x5257fb75,
    0x6a18b6c8,0x6ed9bb7f,0x639aada6,0x675ba011,
    0x791c8014,0x7ddd8da3,0x709e9b7a,0x745f96cd,
    0x9821b6e0,0x9ce0bb57,0x91a3ad8e,0x9562a039,
    0x8b25803c,0x8fe48d8b,0x82a79b52,0x866696e5,
    0xbe29db58,0xbae8d6ef,0xb7abc036,0xb36acd81,
    0xad2ded84,0xa9ece033,0xa4aff6ea,0xa06efb5d,
    0xd4316d90,0xd0f06027,0xddb376fe,0xd9727b49,
    0xc7355b4c,0xc3f456fb,0xceb74022,0xca764d95,
    0xf2390028,0xf6f80d9f,0xfbbb1b46,0xff7a16f1,
    0xe13d36f4,0xe5fc3b43,0xe8bf2d9a,0xec7e202d,
    0x34826077,0x30436dc0,0x3d007b19,0x39c176ae,
    0x278656ab,0x23475b1c,0x2e044dc5,0x2ac54072,
    0x128a0dcf,0x164b0078,0x1b0816a1,0x1fc91b16,
    0x018e3b13,0x054f36a4,0x080c207d,0x0ccd2dca,
    0x7892bb07,0x7c53b6b0,0x7110a069,0x75d1adde,
    0x6b968ddb,0x6f57806c,0x621496b5,0x66d59b02,
    0x5e9ad6bf,0x5a5bdb08,0x5718cdd1,0x53d9c066,
    0x4d9ee063,0x495fedd4,0x441cfb0d,0x40ddf6ba,
    0xaca3d697,0xa862db20,0xa521cdf9,0xa1e0c04e,
    0xbfa7e04b,0xbb66edfc,0xb625fb25,0xb2e4f692,
    0x8aabbb2f,0x8e6ab698,0x8329a041,0x87e8adf6,
    0x99af8df3,0x9d6e8044,0x902d969d,0x94ec9b2a,
    0xe0b30de7,0xe4720050,0xe9311689,0xedf01b3e,
    0xf3b73b3b,0xf776368c,0xfa352055,0xfef42de2,
    0xc6bb605f,0xc27a6de8,0xcf397b31,0xcbf87686,
    0xd5bf5683,0xd17e5b34,0xdc3d4ded,0xd8fc405a,
    0x6904c0ee,0x6dc5cd59,0x6086db80,0x6447d637,
    0x7a00f632,0x7ec1fb85,0x7382ed5c,0x7743e0eb,
    0x4f0cad56,0x4bcda0e1,0x468eb638,0x424fbb8f,
    0x5c089b8a,0x58c9963d,0x558a80e4,0x514b8d53,
    0x25141b9e,0x21d51629,0x2c9600f0,0x28570d47,
    0x36102d42,0x32d120f5,0x3f92362c,0x3b533b9b,
    0x031c7626,0x07dd7b91,0x0a9e6d48,0x0e5f60ff,
    0x101840fa,0x14d94d4d,0x199a5b94,0x1d5b5623,
    0xf125760e,0xf5e47bb9,0xf8a76d60,0xfc6660d7,
    0xe22140d2,0xe6e04d65,0xeba35bbc,0xef62560b,
    0xd72d1bb6,0xd3ec1601,0xdeaf00d8,0xda6e0d6f,
    0xc4292d6a,0xc0e820dd,0xcdab3604,0xc96a3bb3,
    0xbd35ad7e,0xb9f4a0c9,0xb4b7b610,0xb076bba7,
    0xae319ba2,0xaaf09615,0xa7b380cc,0xa3728d7b,
    0x9b3dc0c6,0x9ffccd71,0x92bfdba8,0x967ed61f,
    0x8839f61a,0x8cf8fbad,0x81bbed74,0x857ae0c3,
    0x5d86a099,0x5947ad2e,0x5404bbf7,0x50c5b640,
    0x4e829645,0x4a439bf2,0x47008d2b,0x43c1809c,
    0x7b8ecd21,0x7f4fc096,0x720cd64f,0x76cddbf8,
    0x688afbfd,0x6c4bf64a,0x6108e093,0x65c9ed24,
    0x11967be9,0x1557765e,0x18146087,0x1cd56d30,
    0x02924d35,0x06534082,0x0b10565b,0x0fd15bec,
    0x379e1651,0x335f1be6,0x3e1c0d3f,0x3add0088,
    0x249a208d,0x205b2d3a,0x2d183be3,0x29d93654,
    0xc5a71679,0xc1661bce,0xcc250d17,0xc8e400a0,
    0xd6a320a5,0xd2622d12,0xdf213bcb,0xdbe0367c,
    0xe3af7bc1,0xe76e7676,0xea2d60af,0xeeec6d18,
    0xf0ab4d1d,0xf46a40aa,0xf9295673,0xfde85bc4,
    0x89b7cd09,0x8d76c0be,0x8035d667,0x84f4dbd0,
    0x9ab3fbd5,0x9e72f662,0x9331e0bb,0x97f0ed0c,
    0xafbfa0b1,0xab7ead06,0xa63dbbdf,0xa2fcb668,
    0xbcbb966d,0xb87a9bda,0xb5398d03,0xb1f880b4,
};

/*
Name:       FwWrite
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FW_UPGRADE_CODE_
unsigned long CRC_32( unsigned char * aData, unsigned long aSize )
{
    unsigned long i;
    static unsigned long nAccum = 0;

    for ( i = 0; i < aSize; i++ )
        nAccum = ( nAccum << 8 ) ^ gTable_Crc32[( nAccum >> 24 ) ^ *aData++];

    return nAccum;
}
#endif

/*
Name:       FwUpdate
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_FW_UPGRADE_CODE_
int32 FwUpdate(void *arg)
{
    int32 ret = -1;
    HANDLE  hFile;
    uint8 TmpBuf[512];
    PFIRMWARE_HEADER pFWHead;
    unsigned char FwSign[8] = {'R','K','n','a','n','o','F','W'};
    uint8 *pUBuf = (uint8 *)&FwUpdateBuf[0];
    uint32 i, addr2, FwSize = 0;

    DEBUG("FwUpdate Enter");

#ifdef _WATCH_DOG_
    WatchDogReload();
#endif

    hFile = FileOpenA("\\", UPDATE_FILE_NANE, "R");
    if (hFile == NOT_OPEN_FILE)//after open file ok,mount disk.
    {
        DEBUG("FileOpen Error!");
        return ret;
    }

    if (0==FileRead(TmpBuf, 512, hFile))
    {
        DEBUG("FileRead Error!");
        goto UPDATE_ERROR;
    }

    pFWHead = (PFIRMWARE_HEADER)TmpBuf;
    //DEBUG("fwSign : %s",pFWHead->FwSign);

    if (0 != memcmp(pFWHead->FwSign, FwSign, 8))
    {
        DEBUG("Fw head Error!");
        goto UPDATE_ERROR;
    }


    //FwSize = ((pFWHead->FwEndOffset + 16*1024 + 1024*1024 - 1)>>20)<<20;    //��M����
    FwSize = ((pFWHead->FwEndOffset + 16*1024+ FW_ALIGN_SIZE)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE;
    //FwSize = ((pFWHead->FwEndOffset + 512 /*16*1024*/+ FW_ALIGN_SIZE - 1)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE;

    #if 0   //����У�飬��ֹ�̼���ԭ������64K ��С�� 64K ����У�鲻��
    updatesize = FileGetSize(hFile);
    DEBUG("FwSize = %d updatesize = %d",FwSize,updatesize);
    if ((FwSize+4) != updatesize)
    {
        DEBUG("FwSize Error!");
        goto UPDATE_ERROR;
    }
    #endif

    FileSeek(pFWHead->FwEndOffset, SEEK_SET, hFile);
    FileRead(pUBuf, 512, hFile);
    if (0 != memcmp(TmpBuf, pUBuf, 512))
    {
        DEBUG("Fw end Error!");
        goto UPDATE_ERROR;
    }

    #ifdef _FW_CRC_
    {
        uint32 crc1, crc2;
        FileSeek(4, SEEK_END, hFile);
        if (0==FileRead((uint8*)&crc1, 4, hFile))
        {
            DEBUG("Error!");
            goto UPDATE_ERROR;
        }

        FileSeek(0, SEEK_SET, hFile);
        for (i=0; i<FwSize; i+= FW_BUF_LEN)
        {
            if (0==FileRead(pUBuf, FW_BUF_LEN, hFile))
            {
                DEBUG("Error!");
                goto UPDATE_ERROR;
            }
            crc2 = CRC_32( pUBuf, FW_BUF_LEN);
        }

        if (crc1 != crc2)
        {
            DEBUG("Error!");
            goto UPDATE_ERROR;
        }
    }
    #endif


    DEBUG("Fw2 Update Start");

    addr2 = SysProgRawDiskCapacity;                  //��ȡ�ڶ��ݹ̼���ַ
    memset (pUBuf, 0xFF, FW_BUF_LEN);
    ret = FwWrite(addr2, FW_BUF_LEN>>9, pUBuf);         //������ɵĵڶ��ݹ̼�ͷ
    if (ret < 0)
    {
        DEBUG("FwWrite Error!");
        goto UPDATE_ERROR;
    }

    if (FwSize > (SysProgRawDiskCapacity<<9))      //�̼���С�б��
    {
        addr2 = FwSize>>9;                          //�ڶ��ݹ̼��µ���ʼ��ַ
        DEBUG("FwSize = %d",FwSize);
    }

    FileSeek(FW_BUF_LEN, SEEK_SET, hFile);          //�ӹ̼���FW_BUF_LENλ�ÿ�ʼ����
    addr2 += (FW_BUF_LEN>>9);

    for (i = 0; i < (FwSize-FW_BUF_LEN); i+= FW_BUF_LEN)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        //DEBUG("i = %d", i);
        if (0 == FileRead(pUBuf, FW_BUF_LEN, hFile))
        {
            DEBUG("Error!");
            ret = -1;
            goto UPDATE_ERROR;
        }

        ret = FwWrite(addr2, FW_BUF_LEN>>9, pUBuf);
        if (ret < 0)
        {
            DEBUG("Error!");
            goto UPDATE_ERROR;
        }

        addr2 += (FW_BUF_LEN>>9);
    }

    FileSeek(0, SEEK_SET, hFile);                       //��д�ڶ��ݹ̼�ͷ
    if (0 == FileRead(pUBuf, FW_BUF_LEN, hFile))
    {
        DEBUG("Error!");
        ret = -1;
        goto UPDATE_ERROR;
    }

    addr2 -= (FwSize>>9);
    ret = FwWrite(addr2, FW_BUF_LEN>>9, pUBuf);
    if (ret < 0)
    {
        DEBUG("Error!");
        goto UPDATE_ERROR;
    }

    DEBUG("Fw2 Update End");

    ret = FwRecovery(0, addr2, FwSize);               //�õڶ��ݹ̼�ȥ������һ�ݹ̼�


UPDATE_ERROR:
    FileClose(hFile);

    DEBUG("Delete fwfile!");
    FileDelete("\\", UPDATE_FILE_NANE);

    DEBUG("FwUpdate Exit");

    return ret;
}

#endif
