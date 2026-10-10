#include "SysInclude.h"

#ifdef _USB_HOST_
#include "usb_msc_host.h"

MSC_HOST gMscHost;
uint32   CBWTag;
SCSI_CMD SCSICmd;
uint8    usb_read_reload;


/*
*-------------------------------------------------------------------------------------
*
*                            Local function(common) define
*
*-------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name:   MscHostGetPipe
** Input:  MSC_HOST *pMscHost, HOST_INTERFACE* intf
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostGetPipe(MSC_HOST *pMscHost, HOST_INTERFACE* intf)
{
    int32 i, ret = -HOST_ERR;
    USB_ENDPOINT_DESCRIPTOR *ep = &intf->EpDesc[0];
    uint8 epnum = intf->IfDesc.bNumEndpoints;

    for (i=0; i<epnum; i++, ep++)
    {
        if (USBEpType(ep) == USB_ENDPOINT_XFER_BULK)
        {
            if (USBEpDirIn(ep))
            {
                if (!pMscHost->BulkIn)
                    pMscHost->BulkIn = USBEpNum(ep);
            }
            else
            {
                if (!pMscHost->BulkOut)
                    pMscHost->BulkOut = USBEpNum(ep);
            }
        }
    }
    if (pMscHost->BulkIn && pMscHost->BulkOut)
    {
        pMscHost->SendCtrlPipe = HostCreatePipe(intf->pDev, 0, 0);
        pMscHost->RecvCtrlPipe = HostCreatePipe(intf->pDev, 0, 1);
        pMscHost->SendBulkPipe = HostCreatePipe(intf->pDev, pMscHost->BulkOut, 0);
        pMscHost->RecvBulkPipe = HostCreatePipe(intf->pDev, pMscHost->BulkIn, 1);
        pMscHost->pDev = intf->pDev;
        ret = HOST_OK;
    }
    return ret;
}

/*******************************************************************************
** Name:   MscHostGetMaxLun
** Input:  uint8 *pLun
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostGetMaxLun(uint8 *pLun)
{
    MSC_HOST *pMscHost = &gMscHost;

    return  HostCtrlMsg(pMscHost->pDev, pMscHost->RecvCtrlPipe, 0xA1, USB_BBB_GET_MAX_LUN,
                      0, 0, pLun, 1, USB_CNTL_TIMEOUT*5);
}

/*******************************************************************************
** Name:   MscHostnptxfemp
** Input:  void
** Return: void
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
void MscHostnptxfemp(int32 ret)
{
    Hostnptxfemp(ret);
}


/*******************************************************************************
** Name:   MscHostBulkOnlyXfer
** Input:  SCSI_CMD *sc
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostBulkOnlyXfer(SCSI_CMD *sc)
{
    int32 ret, ret1, retry, count=0;
    HOST_CBW cbw;
    HOST_CSW csw;
    MSC_HOST *pMscHost = &gMscHost;
    //GINTMSK_DATA gintmsk;

    /* COMMAND phase */
    cbw.dCBWSignature = CBWSIGNATURE;
    cbw.dCBWTag = CBWTag++;
    cbw.dCBWDataTransLen = sc->datalen;
    cbw.bCBWFlags = (sc->dirin)? CBWFLAGS_IN : CBWFLAGS_OUT;
    cbw.bCBWLUN = sc->lun;
    cbw.bCDBLength = sc->cmdlen;

    /* copy the command data into the CBW command data buffer */
    memset(cbw.CBWCDB, 0, sizeof(cbw.CBWCDB));
    memcpy(cbw.CBWCDB, sc->cmd, sc->cmdlen);
REPEAT:
    ret = HostBulkMsg(pMscHost->pDev, pMscHost->SendBulkPipe, &cbw, UMASS_BBB_CBW_SIZE, USB_CNTL_TIMEOUT*5);//USB_CNTL_TIMEOUT*5
    if (ret < 0)
    {
        MscHostnptxfemp(ret);
        if (ret == -HOST_NO_DEV)
        {
            printf ("MscHostBulkOnlyXfer error(no device)\n");
            return ret;
        }
        if (count < 10) //10
        {
            count++;
            goto REPEAT;
        }
        printf ("MscHostBulkOnlyXfer error(timeout)\n");
        return ret;
    }

    DelayUs(1);

    /* DATA phase + error handling */
    /* no data, go immediately to the STATUS phase */
    if (sc->datalen == 0)
        goto STATUS;

    ret = HostBulkMsg(pMscHost->pDev, (sc->dirin)? pMscHost->RecvBulkPipe : pMscHost->SendBulkPipe,
                        sc->pdata, sc->datalen, USB_CNTL_TIMEOUT*5);
    if (ret < 0)
    {
        if (-HOST_STALL == ret)
        {
            ret1 = HostCleanEpStall(pMscHost->pDev, pMscHost->SendCtrlPipe, (sc->dirin)? (pMscHost->BulkIn|0x80) : pMscHost->BulkOut);
            if (ret1 < 0)
            {
                printf ("MscHostBulkOnlyXfer error 1\n");
                return ret1;
            }
        }
        else
        {
            printf ("MscHostBulkOnlyXfer error 2\n");
            return ret;
        }
    }

    /* STATUS phase + error handling */
STATUS:
    retry = 0;
AGAIN:
    ret1 = HostBulkMsg(pMscHost->pDev, pMscHost->RecvBulkPipe, &csw, UMASS_BBB_CSW_SIZE, USB_CNTL_TIMEOUT*5);
    if (ret1 < 0)
    {
        if (-HOST_STALL == ret1)
        {
            ret1 = HostCleanEpStall(pMscHost->pDev, pMscHost->SendCtrlPipe, (pMscHost->BulkIn|0x80));
            if (ret1 >= 0 && (retry++ < 1))
            {
                goto AGAIN;
            }
        }
        else
        {
            printf ("MscHostBulkOnlyXfer error 3\n");
            return ret1;
        }
    }
    else
    {
        // Check Csw
        sc->status = csw.bCSWStatus;
    }

    return (ret >= 0)? HOST_OK : ret;
}


/*******************************************************************************
** Name:   MscHostInquiry
** Input:  uint8 lun, void *pInquiry
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostInquiry(uint8 lun, void *pInquiry)
{
    int32 ret;
    SCSI_CMD *sc = &SCSICmd;
    int retries = 3;

    sc->lun = lun;
    sc->dirin = 1;
    memset(&sc->cmd[0], 0, 12);
    sc->cmd[0]=SCSI_INQUIRY_CMD;
    sc->cmd[1]=sc->lun<<5;
    sc->cmd[4]=36;
    sc->cmdlen=0x06;
    sc->datalen=36;
    sc->pdata = (uint8*)pInquiry;

    do
    {
        ret = MscHostBulkOnlyXfer(sc);
        if(HOST_OK == ret)
        {
            return ret;
        }
    } while(retries--);

    return ret;
}


/*******************************************************************************
** Name:   MscHostSwap32
** Input:  uint32 input
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
uint32 MscHostSwap32(uint32 input)
{
    return ((input>>24)|(input<<24)|(input>>8&0xff00)|(input<<8&0xff0000));
}

/*******************************************************************************
** Name:   MscHostRequestSense
** Input:  uint8 lun
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostRequestSense(uint8 lun)
{
    int32 ret;
    uint8 *ptr;
    SCSI_CMD *sc = &SCSICmd;

    sc->lun = lun;
    sc->dirin = 1;
    ptr=(uint8 *)sc->pdata;
    memset(&sc->cmd[0], 0, 12);

    sc->cmd[0]=SCSI_REQ_SENSE;
    sc->cmd[1]=sc->lun<<5;
    sc->cmd[4]=18;
    sc->datalen=18;
    sc->pdata= (uint8*)&sc->sense[0];
    sc->cmdlen=0x0a;

    ret = MscHostBulkOnlyXfer(sc);
    sc->pdata = (uint8 *)ptr;

    return ret;
}


/*******************************************************************************
** Name:   MscHostReadFormatCapacity
** Input:  uint8 lun, uint32 *pCapacity
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostReadFormatCapacity(uint8 lun, uint32 *pCapacity)
{
    int32 ret;
    SCSI_CMD *sc = &SCSICmd;
    int retries = 3;
    uint32 cap[4];

    do
    {
        sc->lun = lun;
        sc->dirin = 1;
        memset(&sc->cmd[0], 0, 12);
        sc->cmd[0]=SCSI_RD_FORMAT_CAPAC;
        sc->cmd[1]=sc->lun<<5;
        sc->cmd[8]=0xfc;        //参见 mass_storage_ufi.pdf
        sc->cmdlen=0x0a;
        sc->datalen=0x0c;
        sc->pdata = (uint8*)&cap[0];

        ret = MscHostBulkOnlyXfer(sc);

        if(HOST_OK == ret)
        {
            if ((MscHostSwap32(cap[2]) & 0xffff)==512)
                *pCapacity = MscHostSwap32(cap[1]);
            printf ("\nMscHostReadFormatCapacity: %dMB(%lfG)\n",
                    *pCapacity/2/1024,*pCapacity/2.0/1024/1024);
            return ret;
        }
        else if (-HOST_STALL == ret)
        {
            MscHostRequestSense(lun);
            DelayMs(1);
        }
        else
        {
            return ret;
        }
    } while(retries--);

    return ret;
}

/*******************************************************************************
** Name:   MscHostReadCapacity
** Input:  uint8 lun, uint32 *pCapacity
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostReadCapacity (uint8 lun, uint32 *pCapacity)
{
    int32 ret;
    SCSI_CMD *sc = &SCSICmd;
    int retries = 3;
    uint32 cap[2];

    do
    {
        sc->lun = lun;
        sc->dirin = 1;
        memset(&sc->cmd[0], 0, 12);
        sc->cmd[0]=SCSI_RD_CAPAC;
        sc->cmd[1]=sc->lun<<5;
        sc->cmdlen=0x0a;
        sc->datalen=8;
        sc->pdata = (uint8*)&cap[0];

        ret = MscHostBulkOnlyXfer(sc);
        if(HOST_OK == ret)
        {
            *pCapacity = MscHostSwap32(cap[0]);
            //printf ("*pCapacity=0x%x\n",*pCapacity);
            printf ("\nMscHostReadCapacity: %dMB(%lfG)\n",
                    *pCapacity/2/1024,*pCapacity/2.0/1024/1024);
            return ret;
        }
        else if (-HOST_STALL == ret)
        {
            MscHostRequestSense(lun);
            DelayMs(1);
        }
        else
        {
            return ret;
        }
    } while(retries--);

    return ret;
}

/*******************************************************************************
** Name:   MscHostTestUnitReady
** Input:  uint8 lun
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostTestUnitReady(uint8 lun)
{
    int32 ret;
    SCSI_CMD *sc = &SCSICmd;
    int retries = 10;

    do
    {
        sc->lun = lun;
        sc->dirin = 0;
        memset(&sc->cmd[0], 0, 12);
        sc->cmd[0]   = SCSI_TST_U_RDY;
        sc->cmd[1]   = sc->lun<<5;
        sc->cmdlen   = 0x06;
        sc->datalen  = 0;

        ret = MscHostBulkOnlyXfer(sc);
        if(HOST_OK == ret)
        {
            return ret;
        }
        else if (-HOST_STALL == ret)
        {
            MscHostRequestSense(lun);
            DelayMs(1);
        }
        else
        {
            return ret;
        }
    } while(retries--);

    return ret;
}


/*******************************************************************************
** Name:   MscHostGetInfo
** Input:  MSC_HOST *pMscHost, uint8 lun
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostGetInfo(MSC_HOST *pMscHost, uint8 lun)
{
    int32 ret;
    uint8 TmpBuf[64];

    ret = MscHostInquiry(lun, TmpBuf);
    if (ret < 0)
        return ret;

    if((TmpBuf[0]&0x1f) == 5)//cdrom
    {
        return -HOST_ERR;
    }

    ret = MscHostReadFormatCapacity(lun, &pMscHost->capacity[lun]);
    if (ret < 0)
    {
        printf ("\nUSB Host MscHostReadFormatCapacity fail\n");
        //return ret;
    }
    #if 1
    ret = MscHostReadCapacity(lun, &pMscHost->capacity[lun]);
    if (ret < 0)
    {
        printf ("\nUSB Host MscHostReadCapacity fail\n");
        //return ret;
    }
    //pMsc->capacity[lun]++;
    #endif
    ret = MscHostTestUnitReady(lun);
    if (ret < 0)
        return ret;

    return HOST_OK;
}


/*******************************************************************************
** Name:   MscHostGetPipe
** Input:  uint8 lun, uint32 start, uint32 blks, void *pBuf
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostRead10(uint8 lun, uint32 start, uint32 blks, void *pBuf)
{
    MSC_HOST *pMscHost = &gMscHost;
    SCSI_CMD *sc = &SCSICmd;
    if (!pMscHost->valid)
    {
        printf ("MscHostRead10 error\n");
        return -HOST_ERR;
    }
    sc->lun = pMscHost->LunMap[lun];
    sc->dirin = 1;
    memset(&sc->cmd[0], 0, 12);
    sc->cmd[0]=SCSI_READ10;
    sc->cmd[1]=sc->lun<<5;
    sc->cmd[2]=((uint8) (start>>24))&0xff;
    sc->cmd[3]=((uint8) (start>>16))&0xff;
    sc->cmd[4]=((uint8) (start>>8))&0xff;
    sc->cmd[5]=((uint8) (start))&0xff;
    sc->cmd[7]=((uint8) (blks>>8))&0xff;
    sc->cmd[8]=(uint8) blks & 0xff;
    sc->cmdlen=0x0a;
    sc->datalen=blks<<9;
    sc->pdata = (uint8*)pBuf;
    return MscHostBulkOnlyXfer(sc);

}

/*******************************************************************************
** Name:   MscHostWrite10
** Input:  uint8 lun, uint32 start, uint32 blks, void *pBuf
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
uint32 MscHostWrite10(uint8 lun, uint32 start, uint32 blks, void *pBuf)
{
    MSC_HOST *pMscHost = &gMscHost;
    SCSI_CMD *sc = &SCSICmd;

    if (!pMscHost->valid)
        return -HOST_ERR;

    sc->lun = pMscHost->LunMap[lun];
    sc->dirin = 0;
    memset(&sc->cmd[0], 0, 12);
    sc->cmd[0]=SCSI_WRITE10;
    sc->cmd[1]=sc->lun<<5;
    sc->cmd[2]=((uint8) (start>>24))&0xff;
    sc->cmd[3]=((uint8) (start>>16))&0xff;
    sc->cmd[4]=((uint8) (start>>8))&0xff;
    sc->cmd[5]=((uint8) (start))&0xff;
    sc->cmd[7]=((uint8) (blks>>8))&0xff;
    sc->cmd[8]=(uint8) blks & 0xff;
    sc->cmdlen=0x0a;
    sc->datalen=blks<<9;
    sc->pdata = (uint8*)pBuf;

    return MscHostBulkOnlyXfer(sc);
}

/*******************************************************************************
** Name:   MscHostProbe
** Input:  HOST_INTERFACE* intf
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHostProbe(HOST_INTERFACE* intf)
{
    int32 i, n, ret;
    MSC_HOST *pMscHost = &gMscHost;
    uint8 LunNum;

    //CBWTag = 0;
    CBWTag = 0;
    memset (pMscHost, 0, sizeof(MSC_HOST));
    ret =  MscHostGetPipe(pMscHost, intf);
    if (ret < 0)
        return ret;
    ret = MscHostGetMaxLun(&LunNum);
    if (ret < 0)
    {
        if (-HOST_STALL == ret)
        {
            HostCleanEpStall(pMscHost->pDev, pMscHost->SendCtrlPipe, 0);
            LunNum = 1;
        }
        else
            return ret;
    }
    else
    {
        LunNum++;
    }

    if (LunNum > MAX_LUN_NUM)
    {
        LunNum = MAX_LUN_NUM;
    }

    for (i=0, n=0; i<LunNum; i++)
    {
        ret = MscHostGetInfo(pMscHost, i);
        if (ret >= 0)
        {
            pMscHost->LunMap[n++] = i;
        }
    }

    if (n > 0)
    {
        ret = HOST_OK;
        pMscHost->LunNum = n;
        pMscHost->valid = 1;
    }
    else
    {
        ret = -HOST_ERR;
    }

    return ret;
}

#if 1
/*
*-------------------------------------------------------------------------------------
*
*                            API function(common) define
*
*-------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name:   MscHost
** Input:  HOST_INTERFACE* interfaceDes
** Return: int32
** Owner:  wrm
** Date:   2015.7.10
** Time:   10:29:11
*******************************************************************************/
int32 MscHost(HOST_INTERFACE* interfaceDes)
{
    int ret = 0;
    memset (&gMscHost, 0 , sizeof(MSC_HOST));
    memset (&SCSICmd, 0 , sizeof(SCSI_CMD));

    //在枚举后可以正常热插拔
    ret = MscHostProbe(interfaceDes);

    printf ("USB Msc Device Enum OK\n");
    return ret;
}


/*******************************************************************************
** Name:   USBMSCHost_Write
** Input:  uint8 lun, uint32 LBA, uint32 nSec, void *pBuf
** Return: uint32
** Owner:  wrm
** Date:   2015.5.25
** Time:   10:29:11
*******************************************************************************/
uint32 USBMSCHost_Write (uint8 lun, uint32 LBA, uint32 nSec, void *pBuf)
{
    uint32 ret;
    uint32 zheng = 0,yu = 0;
    int32  i = 0;
    lun = 0;

#if 1
    lun = gMscHost.LunMap[0];
    if ((lun!=0)&&(lun!=1)&&(lun!=2))
    {
        printf ("\nFirst Write lun=%d\n",lun);
        lun = 0;
    }
    //printf ("\nWrite lun=%d\n",lun);
#endif
    //USBMSCHost Write...
    if (nSec > 2)
    {
        zheng = nSec / 2;
        yu = nSec % 2;
        for (i=0; i<zheng;i++)
        {
            ret = MscHostWrite10(lun, LBA+i*2, 2, pBuf);
            if (ret < 0)
            {
                return RK_ERROR;
            }
            pBuf = (uint8*)pBuf+512*2;
        }
        if (yu > 0)
        {
            ret = MscHostWrite10(lun, LBA+i*2, yu, pBuf);
            if (ret < 0)
            {
                return RK_ERROR;
            }
        }
    }
    else
    {
        ret = MscHostWrite10(lun, LBA, nSec, pBuf);
        if (ret < 0)
        {
            return RK_ERROR;
        }
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name:   USBMSCHost_Read
** Input:  uint8 lun, uint32 LBA, uint32 nSec, void *pBuf
** Return: uint32
** Owner:  wrm
** Date:   2015.5.25
** Time:   10:29:11
*******************************************************************************/
uint32 USBMSCHost_Read (uint8 lun, uint32 LBA, uint32 nSec, void *pBuf)
{
    int32  ret;
    uint32 zheng = 0,yu = 0;
    int32  i = 0;
    lun = 0;

    if (usb_read_reload == 1)
        printf ("\nUSBMSCHost_Read reload\n");

    usb_read_reload = 1;
#if 1
    lun = gMscHost.LunMap[0];
    if ((lun!=0)&&(lun!=1)&&(lun!=2))
    {
        printf ("\nFirst Read lun=%d\n",lun);
        lun = 0;
    }
    //printf ("\nLBA(WinHex)=%d",LBA*512-63*512);
    //printf ("\nHost_Read: lun=%d, LBA=0x%x(%d) nSec=%d\n",lun,LBA,LBA*512,nSec);
#endif
    //USBMSCHost Read...
    if (nSec > 10)
    {
        zheng = nSec / 10;
        yu = nSec % 10;
        for (i=0; i<zheng;i++)
        {
            ret = MscHostRead10(lun, LBA+i*10, 10, pBuf);
            if (ret < 0)
            {

                printf ("USBMSCHost_Read error 0\n");
                usb_read_reload = 0;
                return RK_ERROR;
            }
            pBuf = (uint8*)pBuf+512*10;
        }
        if (yu > 0)
        {
            ret = MscHostRead10(lun, LBA+i*10, yu, pBuf);
            if (ret < 0)
            {

                printf ("USBMSCHost_Read error 1\n");
                usb_read_reload = 0;
                return RK_ERROR;
            }
        }
    }
    else
    {
        ret = MscHostRead10(lun, LBA, nSec, pBuf);
        if (ret < 0)
        {

            printf ("USBMSCHost_Read error 2\n");
            usb_read_reload = 0;
            return RK_ERROR;
        }
    }
    usb_read_reload = 0;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name:   USBMSCHost_GetInfo
** Input:  uint8 lun, pMEMDEV_INFO pDevInfo
** Return: uint32
** Owner:  wrm
** Date:   2015.5.25
** Time:   10:29:11
*******************************************************************************/
uint32 USBMSCHost_GetInfo(uint8 lun, pMEMDEV_INFO pDevInfo)
{
    MSC_HOST *pMscHost = &gMscHost;

    //pDevInfo->Manufacturer = 0;
    pDevInfo->BlockSize = 128;  //固定成64K
    pDevInfo->PageSize = 288;
    pDevInfo->capacity = pMscHost->capacity[0]; //单位Sec
    printf ("pDevInfo->capacity=0x%x\n",pDevInfo->capacity);

    return OK;
}

/*******************************************************************************
** Name:   USBMSCHost_Init
** Input:  void
** Return: uint32
** Owner:  wrm
** Date:   2015.5.25
** Time:   10:29:11
*******************************************************************************/
uint32 USBMSCHost_Init(void)
{
    return OK;
}

/*******************************************************************************
** Name:   USBMSCHost_DeInit
** Input:  void
** Return: uint32
** Owner:  wrm
** Date:   2015.5.25
** Time:   10:29:11
*******************************************************************************/
uint32 USBMSCHost_DeInit(void)
{
    return OK;
}

/*******************************************************************************
** Name:   USBMSCHost_ChkEvent
** Input:  event
** Return: bool
** Owner:  wrm
** Date:   2015.5.25
** Time:   10:29:11
*******************************************************************************/
bool USBMSCHost_ChkEvent(uint32 event)
{
    return TRUE;
}

/*******************************************************************************
** Name:   USBMSCHost_Probe
** Input:  void
** Return: uint32
** Owner:  wrm
** Date:   2015.5.25
** Time:   10:29:11
*******************************************************************************/
uint32 USBMSCHost_Probe(void)
{
    return OK;
}


#endif

#endif

