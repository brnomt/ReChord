/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: FlieStorage.c
Desc:

Author: chenfen@rock-chips.com
Date: 13-07-26
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/
#include "USBConfig.h"

#ifdef USB_MSC

#include "ModuleInfoTab.h"
#include "ModuleOverlay.h"
#include "driverlib_def.h"
#include "USBControl.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>


/*------------------------------------ Defines -------------------------------*/

#define BULK_IN_EP                                      0x01
#define BULK_OUT_EP                                     0x01

#define FSG_NUM_ENDPOINTS                               2


#define FSG_CONFIG_DT_LENGTH        sizeof(USB_CONFIGURATION_DESCRIPTOR) \
                                    + sizeof(USB_INTERFACE_DESCRIPTOR) \
                                    + (FSG_NUM_ENDPOINTS * sizeof(USB_ENDPOINT_DESCRIPTOR))

/* Bulk-only class specific requests */
#define USB_BULK_RESET_REQUEST                          0xff
#define USB_BULK_GET_MAX_LUN_REQUEST                    0xfe

#define FSG_STRING_MANUFACTURER                         1
#define FSG_STRING_PRODUCT                              2
#define FSG_STRING_SERIAL                               3
#define FSG_STRING_CONFIG                               0
#define FSG_STRING_INTERFACE                            0

#define USB_BULK_CB_WRAP_LEN                            31
#define USB_BULK_CB_SIG                                 0x43425355  /* Spells out USBC */
#define USB_BULK_IN_FLAG                                0x80

#define USB_BULK_CS_WRAP_LEN                            13
#define USB_BULK_CS_SIG                                 0x53425355  /* Spells out 'USBS' */
#define USB_STATUS_PASS                                 0
#define USB_STATUS_FAIL                                 1
#define USB_STATUS_PHASE_ERROR                          2


/* Maximal number of LUNs supported in mass storage function */
#define FSG_MAX_LUNS                                    4

/* Length of a SCSI Command Data Block */
#define MAX_COMMAND_SIZE                                16

#define         USB_BULK_IN                             1
#define         USB_BULK_OUT                            0

#define     MAX_LUN     (FLASH_DISK0+FLASH_DISK1+SD_CARD_EN)  /*���֧�ֵ��߼��豸��*/

#ifndef _NANDFLASH_
#define     USB_BUF_SIZE       1024*64                //USB BULKONLY BUFFER SIZE
#else
#define     USB_BUF_SIZE       1024*16                //USB BULKONLY BUFFER SIZE
#endif

#define     MAX_CDBLEN                                  0x10        //���CBW����

/*******************************************************************
CSW����״ֵ̬
*******************************************************************/
#define     CSW_GOOD                                    0x00        //����ͨ��
#define     CSW_FAIL                                    0x01        //����ʧ��
#define     CSW_PHASE_ERROR                             0x02        //��������
/*******************************************************************
Bulk Only����׶�
*******************************************************************/
#define     K_CommandPhase                              0x00
#define     K_OutDataPhase                              0x01
#define     K_OutDataPhaseWrite                         0x02
#define     K_OutResponsePhase                          0x03
#define     K_InDataPhase                               0x81
#define     K_InDataPhaseRead                           0x82
#define     K_InResponsePhase                           0x83
#define     K_InCompletionPhase                         0x84
#define     K_XferDirMask                               0x80
#define     K_InCSWPhase                                0x08


/*******************************************************************
Bulk Only���
*******************************************************************/
#define     K_SCSICMD_TEST_UNIT_READY                   0x00
#define     K_SCSICMD_REZERO_UNIT                       0x01
#define     K_SCSICMD_REQUEST_SENSE                     0x03
#define     K_SCSICMD_FORMAT_UNIT                       0x04
#define     K_SCSICMD_INQUIRY                           0x12
#define     K_SCSICMD_MODE_SENSE_06                     0x1a
#define     K_SCSICMD_START_STOP_UNIT                   0x1b
#define     K_SCSICMD_SEND_DIAGNOSTIC                   0x1d
#define     K_SCSICMD_PREVENT_ALLOW_MEDIUM_REMOVAL      0x1e
#define     K_SCSICMD_READ_FORMAT_CAPACITIES            0x23
#define     K_SCSICMD_READ_CAPACITY                     0x25
#define     K_SCSICMD_READ_10                           0x28
#define     K_SCSICMD_WRITE_10                          0x2a
#define     K_SCSICMD_SEEK_10                           0x2b
#define     K_SCSICMD_WRITE_AND_VERIFY_10               0x2e
#define     K_SCSICMD_VERIFY_10                         0x2f
#define     K_SCSICMD_MODE_SELECT_06                    0x15
#define     K_SCSICMD_MODE_SELECT_10                    0x55
#define     K_SCSICMD_MODE_SENSE_10                     0x5a
#define     K_SCSICMD_READ_12                           0xa8
#define     K_SCSICMD_WRITE_12                          0xaa


#define     K_RKCMD_FLUSH_REQUEST                       0xe0

#define     K_RKCMD_LOCAL_FWUPDATE                      0xfc

#define     K_RKCMD_FIRMWAVE_UPGRADE                    0xff

/*******************************************************************
MMC-2���
*******************************************************************/
#define     K_SCSICMD_READ_SUB_CHANNEL                  0x42
#define     K_SCSICMD_READ_TOC                          0x43
#define     K_SCSICMD_GET_CONFIGURATION                 0x46
#define     K_SCSICMD_GET_EVENT                         0x4a


/*******************************************************************
Sense codes
*******************************************************************/
#define     SCSI_SENSE_NO_SENSE                         0x00
#define     SCSI_SENSE_RECOVERED_ERROR                  0x01
#define     SCSI_SENSE_NOT_READY                        0x02
#define     SCSI_SENSE_MEDIUM_ERROR                     0x03
#define     SCSI_SENSE_HARDWARE_ERROR                   0x04
#define     SCSI_SENSE_ILLEGAL_REQUEST                  0x05
#define     SCSI_SENSE_UNIT_ATTENTION                   0x06
#define     SCSI_SENSE_DATA_PROTECT                     0x07
#define     SCSI_SENSE_BLANK_CHECK                      0x08
#define     SCSI_SENSE_UNIQUE                           0x09
#define     SCSI_SENSE_COPY_ABORTED                     0x0A
#define     SCSI_SENSE_ABORTED_COMMAND                  0x0B
#define     SCSI_SENSE_EQUAL                            0x0C
#define     SCSI_SENSE_VOL_OVERFLOW                     0x0D
#define     SCSI_SENSE_MISCOMPARE                       0x0E
#define     SCSI_SENSE_RESERVED                         0x0F
/*******************************************************************
Additional Sense Codes(ASC)
*******************************************************************/
#define     SCSI_ADSENSE_NO_SENSE                       0x00
#define     SCSI_ADSENSE_LUN_NOT_READY                  0x04
#define     SCSI_ADSENSE_ILLEGAL_COMMAND                0x20
#define     SCSI_ADSENSE_ILLEGAL_BLOCK                  0x21
#define     SCSI_ADSENSE_INVALID_PARAMETER              0x26
#define     SCSI_ADSENSE_INVALID_LUN                    0x25
#define     SCSI_ADSENSE_INVALID_CDB                    0x24
#define     SCSI_ADSENSE_MUSIC_AREA                     0xA0
#define     SCSI_ADSENSE_DATA_AREA                      0xA1
#define     SCSI_ADSENSE_VOLUME_OVERFLOW                0xA7
#define     SCSI_ADSENSE_NO_MEDIA_IN_DEVICE             0x3A
#define     SCSI_ADSENSE_FORMAT_ERROR                   0x31
#define     SCSI_ADSENSE_CMDSEQ_ERROR                   0x2C
#define     SCSI_ADSENSE_MEDIUM_CHANGED                 0x28
#define     SCSI_ADSENSE_BUS_RESET                      0x29
#define     SCSI_ADWRITE_PROTECT                        0x27
#define     SCSI_ADSENSE_TRACK_ERROR                    0x14
#define     SCSI_ADSENSE_SAVE_ERROR                     0x39
#define     SCSI_ADSENSE_SEEK_ERROR                     0x15
#define     SCSI_ADSENSE_REC_DATA_NOECC                 0x17
#define     SCSI_ADSENSE_REC_DATA_ECC                   0x18
/*******************************************************************
 Additional sense code qualifier(ASCQ)
*******************************************************************/
#define     SCSI_SENSEQ_FORMAT_IN_PROGRESS              0x04
#define     SCSI_SENSEQ_INIT_COMMAND_REQUIRED           0x02
#define     SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED    0x03
#define     SCSI_SENSEQ_BECOMING_READY                  0x01
#define     SCSI_SENSEQ_FILEMARK_DETECTED               0x01
#define     SCSI_SENSEQ_SETMARK_DETECTED                0x03
#define     SCSI_SENSEQ_END_OF_MEDIA_DETECTED           0x02
#define     SCSI_SENSEQ_BEGINNING_OF_MEDIA_DETECTED     0x04
/*******************************************************************
Mode Sense/Select page constants.
*******************************************************************/
#define     MODE_PAGE_RBC_DEVICE_PARAMETERS             0x06

#define     PAGECTRL_CURRENT                            0x00
#define     PAGECTRL_CHANGEABLE                         0x01
#define     PAGECTRL_DEFAULT                            0x02
#define     PAGECTRL_SAVED                              0x03

/*******************************************************************
Page field for Inquiry vital product data parameters
*******************************************************************/
#define     SUPPORTED_PAGES                             0x00
#define     SERIAL_NUMBER_PAGE                          0x80
#define     DEVICE_INDENIFICATION_PAGE                  0x83

/*----------------------------------- Typedefs -------------------------------*/

// Host Device Disagreement Matrix
enum _HOST_DEV_DISAGREE
{
    CASEOK = 0,
    CASE1,
    CASE2,
    CASE3,
    CASE4,
    CASE5,
    CASE6,
    CASE7,
    CASE8,
    CASE9,
    CASE10,
    CASE11,
    CASE12,
    CASE13,
    CASECBW,
    CASECMDFAIL
};

typedef volatile struct tagUSB_BULK_XFER
{
    uint32  LBA;
    uint32  NendLen;
    uint32  TransLen;
    uint32  BufLen;
    uint32  XferLen;
    uint16  XferCmd;
    uint8   *buf;
    uint8   Redo;
    uint8   SendCsw;
} USB_BULK_XFER, *pUSB_BULK_XFER;

typedef struct tagFSG_LUN
{
    uint8   LunNum;
    uint8   changed[FSG_MAX_LUNS];
    uint32  DiskID[FSG_MAX_LUNS];
} FSG_LUN;

typedef  volatile struct tagFSG_LUN_REQ
{
    uint8   cmd;
    uint16  len;
    uint32  LUN;
    uint32  LBA;
    void    *buf;

} FSG_LUN_REQ, *pFSG_LUN_REQ;


typedef volatile struct tagFSG_DEVICE
{
    FSG_CBW         *cbw;
    FSG_CSW         csw;
    uint16          ActualCBWLen;
    uint8           mode;
    uint8           config;
    uint8           AlterInterface;
    uint8           BulkIn;
    uint8           BulkOut;
    uint8           BulkPhase;
    uint16          BulkMPS;
    USB_BULK_XFER   BulkXfer;
    uint8           connected;
    uint32          CurDisk;
    FSG_LUN         FileDisk;
    FSG_LUN_REQ     DiskReq[2];

    USB_DEVICE      *pDev;
} FSG_DEVICE, *pFSG_DEVICE;

/*-------------------------- Forward Declarations ----------------------------*/

static int32 FsgSetup(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev);
static void FsgSuspend(USB_DEVICE *pDev);
static void FsgResume(USB_DEVICE *pDev);
static void FsgDisconnect(USB_DEVICE *pDev);
static void FsgReqest(uint8 epnum, uint32 event, uint32 param, USB_DEVICE *pDev);


/* ------------------------------- Globals ---------------------------------- */

//extern uint32 ModuleOverlay(uint32 ModuleNum, uint8 Type);
extern void FUSBRKCmdHook(uint32 cmd, uint32 param);
extern void FUSBGetVetsion(PRKNANO_VERSION pVersion);

/*-------------------------------- Local Statics: ----------------------------*/


//��������������
_ATTR_USB_MSC_DATA_
USB_FSG_CONFIGS_DESCRIPTOR HSFsgConfigs =
{
    sizeof(USB_CONFIGURATION_DESCRIPTOR),       //�������Ĵ�С9(1B)
    USB_DT_CONFIG,                              //������������02(1B)
    FSG_CONFIG_DT_LENGTH,
    1,                                          //������֧�ֵĽӿ���(1B)
    1,                                          //��ΪSet configuration��һ������ѡ������ֵ(1B)
    FSG_STRING_CONFIG,                              //�������������ַ���������(1B)
    0x80,                                       //λͼ,���߹���&Զ�̻���(1B)
    200,                                            //������ĵ���*2mA(1B)

//�ӿ�������
    sizeof(USB_INTERFACE_DESCRIPTOR),           //�������Ĵ�С9(1B)
    USB_DT_INTERFACE,                           //������������04(1B)
    0,                                          //�ӿڵı��(1B)
    0,                                          //����Ϊ��һ���ֶοɹ��滻������(1B)
    FSG_NUM_ENDPOINTS,                          //ʹ�õĶ˵���(�˵�0����)(1B)
    USB_DEVICE_CLASS_STORAGE,                   //1���ʹ���(��USB����)(1B),USB_DEVICE_CLASS_STORAGE=Mass Storage
    USB_SUBCLASS_CODE_SCSI,                 //1�����ʹ���(��USB����)(1B),"0x06=Reduced Block Commands(RBC)"
    USB_PROTOCOL_CODE_BULK,                 //1Э�����(��USB����)(1B),"0X50=Mass Storage Class Bulk-Only Transport"
    FSG_STRING_INTERFACE,                       //�ַ�������������(1B)

//�˵�������
    sizeof(USB_ENDPOINT_DESCRIPTOR),
    USB_DT_ENDPOINT,
    BULK_IN_EP|0x80,
    USB_EPTYPE_BULK,
    HS_BULK_TX_SIZE,//HS_BULK_RX_SIZE,
    0,      //bulk trans invailed

//�˵�������
    sizeof(USB_ENDPOINT_DESCRIPTOR),
    USB_DT_ENDPOINT,
    BULK_OUT_EP,
    USB_EPTYPE_BULK,
    HS_BULK_RX_SIZE,//HS_BULK_RX_SIZE,
    0       //bulk trans invailed
};


USB_FSG_CONFIGS_DESCRIPTOR  FSFsgConfigs =
{
    sizeof(USB_CONFIGURATION_DESCRIPTOR),       //�������Ĵ�С9(1B)
    USB_DT_CONFIG,      //������������02(1B)
    FSG_CONFIG_DT_LENGTH,
    1,                                          //������֧�ֵĽӿ���(1B)
    1,                                          //��ΪSet configuration��һ������ѡ������ֵ(1B)
    FSG_STRING_CONFIG,                          //�������������ַ���������(1B)
    0x80,                                       //λͼ,���߹���&Զ�̻���(1B)
    200,                                        //������ĵ���*2mA(1B)

//�ӿ�������
    sizeof(USB_INTERFACE_DESCRIPTOR),           //�������Ĵ�С9(1B)
    USB_DT_INTERFACE,                           //������������04(1B)
    0,                                          //�ӿڵı��(1B)
    0,                                          //����Ϊ��һ���ֶοɹ��滻������(1B)
    FSG_NUM_ENDPOINTS,                         //ʹ�õĶ˵���(�˵�0����)(1B)
    USB_DEVICE_CLASS_STORAGE,                   //1���ʹ���(��USB����)(1B),USB_DEVICE_CLASS_STORAGE=Mass Storage
    USB_SUBCLASS_CODE_SCSI,                 //1�����ʹ���(��USB����)(1B),"0x06=Reduced Block Commands(RBC)"
    USB_PROTOCOL_CODE_BULK,                 //1Э�����(��USB����)(1B),"0X50=Mass Storage Class Bulk-Only Transport"
    FSG_STRING_INTERFACE,                       //�ַ�������������(1B)

//�˵�������
    sizeof(USB_ENDPOINT_DESCRIPTOR),
    USB_DT_ENDPOINT,
    BULK_IN_EP|0x80,
    USB_EPTYPE_BULK,
    FS_BULK_RX_SIZE,
    0,      //bulk trans invailed

//�˵�������
    sizeof(USB_ENDPOINT_DESCRIPTOR),
    USB_DT_ENDPOINT,
    BULK_OUT_EP,
    USB_EPTYPE_BULK,
    FS_BULK_RX_SIZE,
    0       //bulk trans invailed
};

#if 0

//�����ٶ�����������
_ATTR_USB_MSC_DATA_
OTHER_SPEED_CONFIG_DESCRIPTOR Other_Speed_Config_Descriptor=
{
    sizeof(OTHER_SPEED_CONFIG_DESCRIPTOR),  //length of other speed configuration descriptor
    0x07,                                   //Other speed configuration Type
    FSG_CONFIG_DT_LENGTH,               //�����������ݵĳ���(����,�ӿ�,�˵�����ͻ�Ӧ��)(2B)
    1,                                      //������֧�ֵĽӿ���(1B)
    1,                                      //��ΪSet configuration��һ������ѡ������ֵ(1B)
    FSG_STRING_CONFIG,                      //�������������ַ���������(1B)
    0x80,                                   //λͼ,���߹���&Զ�̻���(1B)
    200                                     //������ĵ���*2mA(1B)
};

//�����豸����������
_ATTR_USB_MSC_DATA_
HS_DEVICE_QUALIFIER HS_Device_Qualifier=
{
    sizeof(HS_DEVICE_QUALIFIER),        //length of HS Device Descriptor
    0x06,                               //HS Device Qualifier Type
    0x0200,                             // USB 2.0 version
    USB_DEVICE_CLASS_STORAGE,           //Device class
    USB_SUBCLASS_CODE_SCSI,             // Device SubClass
    USB_PROTOCOL_CODE_BULK,             //Device Protocol Code
    FS_BULK_RX_SIZE,                    //Maximum Packet SIze for other speed
    0x01,                               //Number of Other speed configurations
    0x00                                //Reserved
};
#endif


#if FSG_STRING_CONFIG > 0
_ATTR_USB_MSC_DATA_
uint8 StringConfig[]=
{
    18,
    0x03,

    'C', 0x00,
    'O', 0x00,
    'N', 0x00,
    'F', 0x00,
    'I', 0x00,
    'G', 0x00,
    ' ', 0x00,
    ' ', 0x00
};
#endif

#if FSG_STRING_INTERFACE > 0
_ATTR_USB_MSC_DATA_
uint8 StringInterface[] =
{
    18,
    0x03,

    'I', 0x00,
    'N', 0x00,
    'T', 0x00,
    'E', 0x00,
    'R', 0x00,
    'F', 0x00,
    'A', 0x00,
    'C', 0x00
};
#endif

#ifdef USB_DRM_EN
_ATTR_USB_MSC_DATA_
uint8 InquirySupportedData[] =
{
    0x00,
    SUPPORTED_PAGES,
    0x00,
    0x03,
    SERIAL_NUMBER_PAGE,
    DEVICE_INDENIFICATION_PAGE
};

_ATTR_USB_MSC_DATA_
uint8 InquirySnData[] =
{
    0x00,
    SERIAL_NUMBER_PAGE,
    0x00,
    20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20
};

_ATTR_USB_MSC_DATA_
uint8 InquiryDeviceData[] =
{
    0x00,
    DEVICE_INDENIFICATION_PAGE,
    0x00,
    26,
    0x02,
    0x01,
    0x00,
    22,
    'R', 'o', 'c', 'k', 'c', 'h', 'i', 'p', ' ',
    'U', 's', 'b', ' ', 'M', 'p', '3', ' ',
    ' ', ' ', ' ', ' '
};
#endif

_ATTR_USB_MSC_DATA_
uint8  SCSIDATA_Sense_SD[] =
{
    0x03,0x00,0x00,0x00,    //��3�ֽ�Ϊ0x80��ʾд����
    0x01,0x0a,0x00,0x10,
    0x00,0x00,0x00,0x00
};

_ATTR_USB_MSC_DATA_
uint8  SCSIDATA_Sense_FLASH[] =
{
    0x03,0x00,0x00,0x00,    //��3�ֽ�Ϊ0x80��ʾд����
    0x01,0x0a,0x00,0x10,
    0x00,0x00,0x00,0x00
};

_ATTR_USB_MSC_DATA_
uint8 SCSI_INQUIRY[] =
{
    0x00,               //Peripheral Device Type    0, 5:CD-ROM
    0x80,               //RMB   1
    0x00,               //ISO Version+ECMA Version+ANSI Version 2
    0x00,               //Response Data Format  3
    0x1f,               //Additional Length(31) 4

    0x00,0x00,0x00,         //reserved  5~7

    'R','o','c','k','C','h','i','p',    //Vendor Information        8~15

    'U','S','B',' ','M','P','3',        //Product Idernification    16~31
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',

    '1','.','0','0'         //version : 1.00    //32~35
};

_ATTR_USB_MSC_DATA_
uint8 SCSI_INQUIRY_SD[] =
{
    0x00,               //Peripheral Device Type    0
    0x80,               //RMB   1
    0x00,               //ISO Version+ECMA Version+ANSI Version 2
    0x01,               //Response Data Format  3
    0x1f,               //Additional Length(31) 4

    0x00,0x00,0x00,         //reserved  5~7

    'R','o','c','k','C','h','i','p',    //Vendor Information        8~15

    'U','S','B',' ',' ','S','D',    //Product Idernification    16~31
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',

    '1','.','0','0'         //version : 1.00    //32~35
};

_ATTR_USB_MSC_CODE_
uint8 SCSI_INQUIRY_CDROM[]=
{
    0x05,
    0x00,               //RMB   1
    0x04,               //ISO Version+ECMA Version+ANSI Version 2
    0x00,               //Response Data Format  3
    0x1f,               //Additional Length(31) 4

    0x00,0x00,0x00,         //reserved  5~7

    'R','o','c','k','C','h','i','p',    //Vendor Information        8~15

    'U','S','B',' ','C','D','-',    //Product Idernification    16~31
    'R','O','M',' ',' ',' ',' ',' ',' ',

    '1','.','0','0'         //version : 1.00    //32~35
};


_ATTR_USB_MSC_BSS_ uint8                    SCSIDATA_RequestSense[18];
_ATTR_USB_MSC_BSS_ __align(4) uint8         FsgEpBuf[64];
_ATTR_USB_MSC_BSS_ uint32                   FsgBulkBuf[USB_BUF_SIZE/4];
_ATTR_USB_MSC_BSS_ FSG_DEVICE               FsgDevice;

_ATTR_USB_MSC_DATA_
static USB_DEVICE FsgDriver =
{
    USB_SPEED_HIGH,
    USB_CLASS_TYPE_MSC,
    sizeof(USB_FSG_CONFIGS_DESCRIPTOR),
    &HSFsgConfigs,
    &FSFsgConfigs,
    FsgSetup,
    FsgSuspend,
    FsgResume,
    FsgDisconnect,
    FsgReqest,
    &FsgDevice,
    NULL,
    NULL
};

_ATTR_USB_MSC_DATA_
uint8 USBD_STATUS_STALL_PID[]=
{
    0xc0,
    0x00,
    0x00,
    0x04
};

_ATTR_USB_MSC_DATA_
uint8 MSOSString[] =
{
    0x12,       //blength
    0x03,       //bDescriptorType
    //qwSignature (MSFT100)
    0x4d,0x00,
    0x53,0x00,
    0x46,0x00,
    0x54,0x00,
    0x31,0x00,
    0x30,0x00,
    0x30,0x00,

    0x01,       //bms_VendorCode
    0x01        //bpad
};


//_ATTR_USB_MSC_BSS_ __align(4) uint8  EpInData[512];

/*--------------------------- Local Function Prototypes ----------------------*/


/*------------------------ Function Implement --------------------------------*/

#if 0
_ATTR_USB_MSC_BSS_
uint32 UsbReadBuf[USB_BUF_SIZE/4];
_ATTR_USB_MSC_DATA_ uint32 ErrLBA = 0;
_ATTR_USB_MSC_DATA_ uint32 ErrAddr = 0;

_ATTR_USB_MSC_CODE_
void MscChkWrite(MEMDEV_ID DevID, uint32 SecAdrr, uint32 nSec, void* pBuf)
{
    uint32 i;
    uint32 *WriteBuf = (uint32 *)pBuf;
    MDRead(DevID, SecAdrr, nSec, UsbReadBuf);

    for (i=0; i<128*nSec; i++)
    {
        if (WriteBuf[i] != UsbReadBuf[i])
        {
            ErrLBA = SecAdrr;
            ErrAddr = i;
            break;
        }
    }
}
#endif


#ifdef USB_DRM_EN
extern  UINT8   g_pmid[16];
/*
Name:       FsgModifyDRM9ID
Desc:       �޸�DRM9 IDֵ
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgModifyDRM9ID(void)
{
    UINT32 i;

    InquirySnData[3] = 16;

    for (i = 0;i < 16; i++)
    {
        InquirySnData[i+4] = g_pmid[i];
    }
}
#endif

/*
Name:       FsgCSWHandler
Desc:       CSW����
Param:      HostDevCase=״̬,DeviceTrDataLen=�豸Ҫ���͵����ݳ���
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgCSWHandler(FSG_DEVICE *pFsg, uint8 HostDevCase,uint32 DeviceTrDataLen)
{
    FSG_CSW      *csw = &pFsg->csw;
    FSG_CBW      *cbw = pFsg->cbw;
    /* Store and send the Bulk-only CSW */
    csw->dCSWSignature = USB_BULK_CS_SIG;
    csw->dCSWTag = cbw->dCBWTag;
    csw->dCSWDataResidue = cbw->dCBWDataTransLen - DeviceTrDataLen;
    //csw->bCSWStatus = status;

    switch (HostDevCase)
    {
        case CASEOK:
        case CASE1:     /* Hn=Dn*/
        case CASE6:     /* Hi=Di*/
        case CASE12:    /* Ho=Do*/
            csw->bCSWStatus = CSW_GOOD;
            break;

        case CASE4:     /* Hi>Dn*/
        case CASE5:     /* Hi>Di*/
            USBSetEpSts(1, BULK_IN_EP, 1);
            csw->bCSWStatus = CSW_FAIL; //CSW_GOOD or CSW_FAIL
            pFsg->BulkPhase = K_InCSWPhase;
            break;

        case CASE2:     /* Hn<Di*/
        case CASE3:     /* Hn<Do*/
        case CASE7:     /* Hi<Di*/
        case CASE8:     /* Hi<>Do */
            //MUSBHSFC_SetEndpointStatus(3, 1);       //stall IN endpoint
            USBSetEpSts(1, BULK_IN_EP, 1);
            csw->bCSWStatus = CSW_PHASE_ERROR;
            pFsg->BulkPhase = K_InCSWPhase;
            break;

        case CASE9:     /* Ho>Dn*/
        case CASE11:    /* Ho>Do*/
            USBSetEpSts(0, BULK_OUT_EP, 1);
            csw->bCSWStatus = CSW_FAIL;
            pFsg->BulkPhase = K_InCSWPhase;
            break;
        case CASE10:    /* Ho<>Di */
        case CASE13:    /* Ho<Do*/
            //MUSBHSFC_SetEndpointStatus(2, 1);
            USBSetEpSts(0, BULK_OUT_EP, 1);
            csw->bCSWStatus = CSW_PHASE_ERROR;
            pFsg->BulkPhase = K_InCSWPhase;
            break;
        case CASECMDFAIL:
            csw->bCSWStatus = CSW_FAIL;
            pFsg->BulkPhase = K_InCSWPhase;
            break;
        case CASECBW:   /* invalid CBW */
            //MUSBHSFC_SetEndpointStatus(3, 1);
            USBSetEpSts(1, BULK_IN_EP, 1);
            if (cbw->dCBWDataTransLen && !(cbw->bCBWFlags & 0x80))
                USBSetEpSts(0, BULK_OUT_EP, 1);
            csw->bCSWStatus = CSW_FAIL;
            pFsg->BulkPhase = K_InCSWPhase;
            break;

        default:
            break;
    }
}

#ifdef USB_IF_TEST
/*
Name:       FsgChkCmd
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static int32 FsgChkCmd(FSG_DEVICE *pFsg, uint8 DataDir, uint32 DataLen)
{
    int32 ret = OK;
    FSG_CBW * cbw = pFsg->cbw;
    uint8 CbwDir  = (pFsg->cbw->bCBWFlags&USB_BULK_IN_FLAG)? DATA_DIR_TO_HOST : DATA_DIR_FROM_HOST;

    if (0 == cbw->dCBWDataTransLen)
    {
        //CbwDir = DATA_DIR_NONE;
        if (DataLen)
        {
            FsgCSWHandler(pFsg, DataDir? CASE2: CASE3, 0);
            ret = ERROR;
        }
    }
    else
    {
        if (DATA_DIR_TO_HOST == CbwDir)
        {
            if (cbw->dCBWDataTransLen > DataLen)
            {
                FsgCSWHandler(pFsg, DataLen? CASE4 : CASE5, 0);
                ret = ERROR;
            }
            else if (CbwDir != DataDir || cbw->dCBWDataTransLen < DataLen)
            {
                FsgCSWHandler(pFsg, (CbwDir != DataDir)? CASE8 : CASE7, DataLen);
                ret = ERROR;
            }
        }
        else
        {
            if (cbw->dCBWDataTransLen > DataLen)
            {
                FsgCSWHandler(pFsg, DataLen? CASE9 : CASE11, DataLen);
                ret = ERROR;
            }
            else if (CbwDir != DataDir || cbw->dCBWDataTransLen < DataLen)
            {
                FsgCSWHandler(pFsg, (CbwDir != DataDir)? CASE10 : CASE13, DataLen);
                ret = ERROR;
            }
        }
    }

    return ret;
}
#endif

/*
Name:       FsgSenseData
Desc:       ���� sense
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgSenseData(uint8 SenseKey, uint8 ASC, uint8 ASCQ)
{
    SCSIDATA_RequestSense[0]=0x70;      //information filed is invalid=0x70,information filed is valid=0xf0,
    SCSIDATA_RequestSense[1]=0x00;      //reserve
    SCSIDATA_RequestSense[2]=SenseKey;

    SCSIDATA_RequestSense[3]=0x00;      //information
    SCSIDATA_RequestSense[4]=0x00;
    SCSIDATA_RequestSense[5]=0x00;
    SCSIDATA_RequestSense[6]=0x00;

    SCSIDATA_RequestSense[7]=0x0a;      //Addition Sense Length

    SCSIDATA_RequestSense[8]=0x00;      //reserve
    SCSIDATA_RequestSense[9]=0x00;
    SCSIDATA_RequestSense[10]=0x00;
    SCSIDATA_RequestSense[11]=0x00;

    SCSIDATA_RequestSense[12]=ASC;
    SCSIDATA_RequestSense[13]=ASCQ;

    SCSIDATA_RequestSense[14]=0x00;     //reserve
    SCSIDATA_RequestSense[15]=0x00;
    SCSIDATA_RequestSense[16]=0x00;
    SCSIDATA_RequestSense[17]=0x00;
}

/*
Name:       FsgSendCSW
Desc:       ����CSW
Param:
Return:
Global:
Note:
Author:
Log:
*/

_ATTR_USB_MSC_CODE_
static void FsgSendCSW(FSG_DEVICE *pFsg)
{
    USBWriteEp(BULK_IN_EP, 13, (uint8*)&pFsg->csw);

    pFsg->BulkPhase = K_CommandPhase;  //add by lxs
}

/*
Name:       FsgReqAdd
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static int32  FsgReqAdd(uint8 cmd, uint32 lun, uint32 LBA, uint16 len, void *buf)
{
    FSG_LUN_REQ *req = &FsgDevice.DiskReq[0];

    if (req->cmd)
        return -1;

    req->LUN = lun;
    req->LBA = LBA;
    req->len = len;
    req->buf = buf;
    req->cmd = cmd;

    return 0;
}


/*
Name:       FsgReqNext
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static FSG_LUN_REQ *FsgReqNext(FSG_DEVICE *pFsg)
{
    FSG_LUN_REQ *req = &pFsg->DiskReq[0];

    if (req->cmd)
        return req;
    else
        return NULL;
}


/*
Name:       FsgFlushReq
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgFlushReq(FSG_DEVICE *pFsg)
{
    FSG_LUN_REQ *req = &pFsg->DiskReq[1];

    if (req->cmd == K_SCSICMD_WRITE_10)
    {
        MDWrite(req->LUN, req->LBA, req->len, req->buf);
        //MscChkWrite(req->LUN, req->LBA, req->len, req->buf);
    }
    req->cmd = 0;
}

/*
Name:       FsgFlush
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgFlush(FSG_DEVICE *pFsg)
{
    FsgFlushReq(pFsg);
}


/*
Name:       FsgForceDisconnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgForceDisconnect(FSG_DEVICE *pFsg)
{
    FsgCSWHandler(pFsg, CASEOK, 12);
    FsgSendCSW(pFsg);
    USBDelayMS(100);
    USBDisconnect();

    pFsg->connected = 0;               //�Ͽ�����?
}

/*
Name:       FsgFirmwareUpgrade
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgFirmwareUpgrade(FSG_DEVICE *pFsg)
{
    UINT8 tempbuf[512];
    UINT8  cbLun;
    UINT16  cbLen;
    uint32 Read12LBA;
    uint8 *cmnd = &pFsg->cbw->CBWCDB[0];

    cbLun = cmnd[1];
    Read12LBA = __get_unaligned_be32((cmnd+2));
    cbLen = __get_unaligned_be16((cmnd+6));

    if (cbLun == 0XE0)
    {
        if (Read12LBA == 0xFFFFFFFF)    //GET VERSION.
        {
            if (cbLen == 12)            //�汾����
            {
                RKNANO_VERSION version;

                FUSBGetVetsion(&version);
                FsgCSWHandler(pFsg, CASEOK, 12);
                USBWriteEp(pFsg->BulkIn, 12, (uint8*)&version);
                FsgSendCSW(pFsg);
                return ;
            }
        }
        else if (Read12LBA == 0xFFFFFFFD)
        {
            ;
        }
        else if (Read12LBA == 0xFFFFFFFE)
            //||  Read12LBA == 0xFFFFFFF6)    // Show User Disk, not enum the cdrom
        {
            FsgForceDisconnect(pFsg);
        }

#if 0//def USB_PERMIT
        else if (Read12LBA == 0xFFFFFFFB)  //ֻ�����û���
        {
            DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;
            FsgCSWHandler(pFsg, CASEOK, 12);
            FsgSendCSW(pFsg);
            Delay100cyc(6000);
            dev_regs->dctl |= 0x02;       //soft disconnect
            //UsbConnected = 0;
        }
        else if (Read12LBA == 0xFFFFFFF8) //�����û���
        {
            if (cbLen == 16)
            {
                //FlashValid = 1;
                FsgCSWHandler(CASEOK, 12);
                USBWriteEp(pFsg, BULK_IN_EP, 16, tempbuf);
                FsgSendCSW(pFsg);
                return ;
            }

        }
        else if (Read12LBA == 0xFFFFFFF9)  //�����û���
        {
            if (cbLen == 16)
            {
                //USBPermit = USB_PERMIT_DISABLE;
                //FlashValid = 0;
                FsgCSWHandler(pFsg, CASEOK, 12);
                USBWriteEp(BULK_IN_EP, 16, tempbuf);
                FsgSendCSW(pFsg);
                return ;
            }
        }
#endif

        else if (Read12LBA == 0xFFFFFFFC)  //GETCHIPINFO
        {
            if (cbLen == 16)
            {
                memcpy(tempbuf, "RKNANOC", 8);  // 8 = sizeof("RKNANOC")
                tempbuf[7] = '\0';
                FsgCSWHandler(pFsg, CASEOK, 12);
                USBWriteEp(BULK_IN_EP, 16, tempbuf);
                FsgSendCSW(pFsg);
                return ;
            }
        }
        else if (Read12LBA == 0xFFFFFFF6)       // Show User Disk, not enum the cdrom
        {
            FsgForceDisconnect(pFsg);
        }

        FUSBRKCmdHook(Read12LBA, cbLen);

    }

    pFsg->BulkPhase=K_InCSWPhase;
}
//#endif

/*
Name:       FsgInquiry
Desc:       ����:��ѯ0x12
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgInquiry(FSG_DEVICE *pFsg)
{
    uint32 residue;
    uint8 *cmnd = &pFsg->cbw->CBWCDB[0];

#ifdef USB_IF_TEST
    if (0 == cmnd[4])
    {
        FsgCSWHandler(pFsg, CASE6, 0);
        FsgSendCSW(pFsg);
        return;
    }
#endif

    FsgSenseData(SCSI_SENSE_NO_SENSE, 0, 0);

#ifdef USB_DRM_EN
    if (cmnd[1] & 0x01)   //EVPD
    {
        if (cmnd[2] == SUPPORTED_PAGES)
        {
            residue = MIN(sizeof(InquirySupportedData), cmnd[4]);
            FsgCSWHandler(pFsg, CASE6, residue);
            USBWriteEp(pFsg->BulkIn, residue, InquirySupportedData);
        }
#if 1
        else if (cmnd[2] == SERIAL_NUMBER_PAGE)
        {
            residue = MIN(sizeof(InquirySnData), cmnd[4]);
            FsgCSWHandler(pFsg, CASE6, residue);
            FsgModifyDRM9ID();
            USBWriteEp(pFsg->BulkIn, residue, InquirySnData);
        }
#endif
        else if (cmnd[2] == DEVICE_INDENIFICATION_PAGE)
        {
            residue = MIN(sizeof(InquiryDeviceData), cmnd[4]);
            FsgCSWHandler(pFsg, CASE6, residue);
            USBWriteEp(pFsg->BulkIn, residue, InquiryDeviceData);
        }
        else
        {
            FsgSenseData(SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_INVALID_CDB, 0);
            FsgCSWHandler(pFsg, CASECBW, 0);
        }
    }
    else
#endif
    {
        FsgCSWHandler(pFsg, CASE6, cmnd[4]);   //modify by lxs @2007.03.13, for Mac OS 9.2.2

        if (MD_MAJOR(pFsg->CurDisk) == MEM_DEV_NAND
                || MD_MAJOR(pFsg->CurDisk) == MEM_DEV_EMMC)
        {
            uint8 *pInquiry = ((FUSBShowCdrom()==1)&&(MD_MINOR(pFsg->CurDisk)==1))? SCSI_INQUIRY_CDROM : SCSI_INQUIRY;
            residue= MIN(sizeof(SCSI_INQUIRY), cmnd[4]);
            USBWriteEp(pFsg->BulkIn, residue, pInquiry);
        }
        else if (MD_MAJOR(pFsg->CurDisk) == MEM_DEV_SD)
        {
            FsgCSWHandler(pFsg, CASE6, cmnd[4]);//modify by lxs @2008.01.21, for Mac OS 9.2.2
            USBWriteEp(pFsg->BulkIn, cmnd[4], SCSI_INQUIRY_SD);
        }
    }

    pFsg->BulkPhase = K_InCSWPhase;
}

/*
Name:       FsgReadFormatCapacities
Desc:       ����:���ɸ�ʽ������0x23
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgReadFormatCapacities(FSG_DEVICE *pFsg)
{
    uint8 capacity[12];
    uint32 TotalSecs;
    uint16 BytsPerSec;

    if (MDChkValid(pFsg->CurDisk))
    {
        TotalSecs = MDGetCapacity(pFsg->CurDisk);
        BytsPerSec=512;
    }
    else
    {
        FsgSenseData(SCSI_SENSE_NOT_READY, SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0);
        FsgCSWHandler(pFsg, CASECBW, 0);
        return;
    }

    FsgCSWHandler(pFsg, CASE6,12);
    FsgSenseData(SCSI_SENSE_NO_SENSE,0,0);
    capacity[0]=0;
    capacity[1]=0;
    capacity[2]=0;
    capacity[3]=8;
    capacity[4]=(uint8)((TotalSecs >> 24) & 0x00ff);
    capacity[5]=(uint8)((TotalSecs >> 16) & 0x00ff);
    capacity[6]=(uint8)((TotalSecs >> 8) & 0x00ff);
    capacity[7]=(uint8)(TotalSecs & 0x00ff);
    capacity[8]=0x01;
    capacity[9]=0x00;
    capacity[10]=(uint8)(BytsPerSec >> 8);      //bytes/block
    capacity[11]=(uint8)(BytsPerSec & 0x00ff);

    USBWriteEp(pFsg->BulkIn, 12, capacity);
    pFsg->BulkPhase = K_InCSWPhase;
}


/*
Name:       FsgReadCapacity
Desc:       ����:������0x25
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgReadCapacity(FSG_DEVICE *pFsg)
{
    uint8 capacity[8];
    uint32 TotalSecs;
    uint16 BytsPerSec;

    if (MDChkValid(pFsg->CurDisk))
    {
        TotalSecs = MDGetCapacity(pFsg->CurDisk)-1;
        BytsPerSec = 512;
    }
    else
    {
        //BuildSenseData(SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_INVALID_CDB, 0);
        FsgSenseData(SCSI_SENSE_NOT_READY, SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0);
        FsgCSWHandler(pFsg, CASECBW, 0);
        return;
    }

    FsgCSWHandler(pFsg, CASE6,8);
    FsgSenseData(SCSI_SENSE_NO_SENSE,0,0);

    capacity[0]=(uint8)((TotalSecs >> 24) & 0x00ff);
    capacity[1]=(uint8)((TotalSecs >> 16) & 0x00ff);
    capacity[2]=(uint8)(TotalSecs >> 8) & 0x00ff;
    capacity[3]=(uint8)(TotalSecs & 0x00ff);
    capacity[4]=0x00;
    capacity[5]=0x00;
    capacity[6]=(uint8)(BytsPerSec >> 8);       //bytes/block
    capacity[7]=(uint8)(BytsPerSec & 0x00ff);

    USBWriteEp(pFsg->BulkIn, 8, capacity);
    pFsg->BulkPhase = K_InCSWPhase;
}

/*
Name:       FsgRead10
Desc:       ����:������0x28
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgRead10(FSG_DEVICE *pFsg)
{
    uint16 nSec;
    uint32 byte;
    uint8 *cmnd = &pFsg->cbw->CBWCDB[0];
    pUSB_BULK_XFER Xfer = &pFsg->BulkXfer;

    if (! MDChkValid(pFsg->CurDisk))
    {
        FsgSenseData(SCSI_SENSE_NOT_READY, SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0);
        FsgCSWHandler(pFsg, CASECBW, 0);
        return;
    }

    FsgCSWHandler(pFsg, CASE6, pFsg->cbw->dCBWDataTransLen);
    FsgSenseData(SCSI_SENSE_NO_SENSE, 0, 0);

    nSec = __get_unaligned_be16((cmnd+7));
    byte = (uint32)nSec<<9;//sec to byte

#ifdef USB_IF_TEST
    if (OK != FsgChkCmd(pFsg, DATA_DIR_TO_HOST, byte))
    return;
#endif

    Xfer->LBA = __get_unaligned_be32((cmnd+2));

    Xfer->NendLen = byte;
    Xfer->TransLen = 0;
    Xfer->XferLen = Xfer->BufLen = 0;
    nSec = MIN(nSec, USB_BUF_SIZE/512);

    Xfer->XferCmd = USB_BULK_IN;
    pFsg->BulkPhase = K_InDataPhase;

    FsgReqAdd(K_SCSICMD_READ_10, pFsg->CurDisk, Xfer->LBA, nSec, Xfer->buf);

}

/*
Name:       FsgWrite10
Desc:       ����:д����0x2a
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgWrite10(FSG_DEVICE *pFsg)
{
    uint16 nSec;
    uint32 byte;
    uint8 *cmnd = &pFsg->cbw->CBWCDB[0];
    pUSB_BULK_XFER Xfer = &pFsg->BulkXfer;

    if (!MDChkValid(pFsg->CurDisk))
    {
        FsgSenseData(SCSI_SENSE_NOT_READY, SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0);
        FsgCSWHandler(pFsg, CASECBW, 0);
        return;
    }

    FsgCSWHandler(pFsg, CASE12, pFsg->cbw->dCBWDataTransLen);
    FsgSenseData(SCSI_SENSE_NO_SENSE,0,0);

    nSec = __get_unaligned_be16((cmnd+7));
    byte = (uint32)nSec<<9;//sec to byte

#ifdef USB_IF_TEST
    if (OK != FsgChkCmd(pFsg, DATA_DIR_FROM_HOST, byte))
    return;
#endif

    Xfer->LBA=__get_unaligned_be32((cmnd+2));

    //Xfer->LBA=((uint32)cmnd[2]<<8)+cmnd[3];
    //Xfer->LBA <<= 16;
    //Xfer->LBA +=((uint32)cmnd[4]<<8)+cmnd[5];
    //UDEBUG("Xfer->LBA=0x%x\n", Xfer->LBA);

    Xfer->NendLen = byte;            //sec to byte
    Xfer->TransLen = Xfer->XferLen = 0;
    Xfer->BufLen = MIN(Xfer->NendLen, USB_BUF_SIZE);

    Xfer->XferCmd = USB_BULK_OUT;
    pFsg->BulkPhase = K_OutDataPhase;
    Xfer->SendCsw = 0;
}

/*
Name:       FsgModeSense06
Desc:       ����:��ģʽ0x1a
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgModeSense06(FSG_DEVICE *pFsg)
{
    uint8* pSenseData;

    FsgCSWHandler(pFsg, CASEOK, 0x04);
    if (! MDChkValid(pFsg->CurDisk))
    {
        FsgSenseData(SCSI_SENSE_NOT_READY, SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0);
        FsgCSWHandler(pFsg, CASECBW, 0);
        return;
    }

    pSenseData = (uint8*)((MD_MAJOR(pFsg->CurDisk) == MEM_DEV_SD)? SCSIDATA_Sense_SD : SCSIDATA_Sense_FLASH);
    USBWriteEp(pFsg->BulkIn, 4, pSenseData);

    pFsg->BulkPhase = K_InCSWPhase;
}

/*
Name:       FsgTestUnitReady
Desc:       ����:����׼��0x00
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgTestUnitReady(FSG_DEVICE *pFsg)
{
    char buf[512] = {0};
#ifdef USB_IF_TEST
    if (OK != FsgChkCmd(pFsg, DATA_DIR_NONE, 0))
    {
        //FsgSendCSW(pFsg);
        return;
    }
#endif

    if (!MDChkValid(pFsg->CurDisk))
    {
        FsgSenseData(SCSI_SENSE_NOT_READY, SCSI_ADSENSE_NO_MEDIA_IN_DEVICE, 0);
        FsgCSWHandler(pFsg, CASE1, 0);
        pFsg->csw.bCSWStatus = CSW_FAIL;
    }
    else
    {
        uint8 lun = pFsg->cbw->bCBWLUN;
        FSG_LUN *pDisk = &pFsg->FileDisk;

        if (pDisk->changed[lun])
        {
            pDisk->changed[lun] = 0;
            FsgSenseData(SCSI_SENSE_UNIT_ATTENTION, SCSI_ADSENSE_MEDIUM_CHANGED, 0);
            FsgCSWHandler(pFsg, CASE1,0);
            pFsg->csw.bCSWStatus = CSW_FAIL;
        }
        else
        {
            FsgCSWHandler(pFsg, CASE1,0);
            FsgSenseData(SCSI_SENSE_NO_SENSE, 0, 0);
        }
    }
    FsgSendCSW(pFsg);
}


/*
Name:       FsgRequestSense
Desc:       ����:����0x03
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgRequestSense(FSG_DEVICE *pFsg)
{
    uint8 *cmnd = &pFsg->cbw->CBWCDB[0];
    uint32 residue= MIN (sizeof(SCSIDATA_RequestSense), cmnd[4]);

    FsgCSWHandler(pFsg, CASE6,residue);
    if (0 == cmnd[4])
    {
        FsgSendCSW(pFsg);
        return;
    }

    USBWriteEp(BULK_IN_EP, residue, SCSIDATA_RequestSense);

    pFsg->BulkPhase = K_InCSWPhase;
}

/*
Name:       FsgPreventAllowMediumRemoval
Desc:       ����:��ֹ�����Ƴ�0x1e
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgPreventAllowMediumRemoval(FSG_DEVICE *pFsg)
{
    uint8 *cmnd = &pFsg->cbw->CBWCDB[0];
#if 1
    if ((cmnd[4] & 0x01) == 0x01)
    {//���vista �� mac����U���˲����������⣬���ǻ����˳�U�̺���û�������״̬�ġ�
        //BuildSenseData(SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_INVALID_PARAMETER, SCSI_SENSEQ_INIT_COMMAND_REQUIRED);
        FsgSenseData(SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_INVALID_CDB, 0);
        FsgCSWHandler(pFsg, CASECMDFAIL, pFsg->cbw->dCBWDataTransLen);         //�����Ƴ��豸
        pFsg->csw.bCSWStatus = CSW_FAIL; // comment by hwg, 07-06-30
    }
    else
    {
        FsgSenseData(SCSI_SENSE_NO_SENSE,0,0);
        FsgCSWHandler(pFsg, CASE1, pFsg->cbw->dCBWDataTransLen);          //�����Ƴ��豸
        //MscReQAdd(K_SCSICMD_PREVENT_ALLOW_MEDIUM_REMOVAL, USBDiskID, 0, 0, NULL);
    }
#endif

    FsgSendCSW(pFsg);

}

/*
Name:       FsgStartStopUnit
Desc:       ����:��ͣ����0x1b
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgStartStopUnit(FSG_DEVICE *pFsg)
{
    uint8 *cmnd = &pFsg->cbw->CBWCDB[0];

    FsgSenseData(SCSI_SENSE_NO_SENSE,0,0);
    FsgCSWHandler(pFsg, CASE1, pFsg->cbw->dCBWDataTransLen);          //�����Ƴ��豸

    FsgSendCSW(pFsg);

    if (cmnd[4] & 0x02)
    {
        //////////////////////////
        //PC�˵���USBʱ
        DelayUs(300);
        USBDisconnect();
        //////////////////////////

        pFsg->connected = 0;                           //�豸�Ƴ����Ͽ�����
        //�����Ҽ�"����"����Ͽ�USB���ӣ������������񣬵����������Ƴ�USBʱ��Ŀǰ���ܶϿ�USB
        //ԭ����:���ղ���ϵͳ�����ĵ������
        SendMsg(MSG_PC_DISCONNECT_USB);
    }
}

/*
Name:       FsgVerify10
Desc:       ����:У��0x2f
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgVerify10(FSG_DEVICE *pFsg)
{
    if (pFsg->cbw->dCBWDataTransLen == 0)
    {
        FsgCSWHandler(pFsg, CASE1,0);
        FsgSenseData(SCSI_SENSE_NO_SENSE,0,0);
        FsgSendCSW(pFsg);
    }
    else
    {
        FsgCSWHandler(pFsg, CASE12, pFsg->cbw->dCBWDataTransLen);
        FsgSenseData(SCSI_SENSE_NO_SENSE,0,0);
        pFsg->BulkPhase = K_OutDataPhase;
    }
}

/*
Name:       FsgModeSense10
Desc:       ����:0x5a
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgModeSense10(FSG_DEVICE *pFsg)
{
    uint32 len;
    uint8* pSenseData;

    len = pFsg->cbw->dCBWDataTransLen; //modify by lxs @2007.03.13, for Mac OS 10.3.4
    FsgCSWHandler(pFsg, CASEOK, len);

    pSenseData = (uint8*)((MD_MAJOR(pFsg->CurDisk) == MEM_DEV_SD)? SCSIDATA_Sense_SD : SCSIDATA_Sense_FLASH);

    USBWriteEp(pFsg->BulkIn, len, pSenseData);

    pFsg->BulkPhase = K_InCSWPhase;
}

/*
Name:       FsgCBWValidVerify
Desc:       �������ЧУ��
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static int32 FsgCBWValidVerify(FSG_DEVICE *pFsg)
{
    FSG_CBW       *cbw = pFsg->cbw;

    if (pFsg->ActualCBWLen!= USB_BULK_CB_WRAP_LEN || cbw->dCBWSignature != USB_BULK_CB_SIG)
    {
        /* The Bulk-only spec says we MUST stall the IN endpoint*/
        //USBStallEp(pFsg->BulkIn|0x80);
        FsgCSWHandler(pFsg, CASECBW, 0);
        return -EINVAL;
    }

    /* Is the CBW meaningful? */
    if (cbw->bCBWLUN >= FSG_MAX_LUNS || cbw->bCBWFlags & ~USB_BULK_IN_FLAG ||
        cbw->bCDBLength <= 0 || cbw->bCDBLength > MAX_COMMAND_SIZE)
    {
        FsgCSWHandler(pFsg, CASECBW, 0);
        return -EINVAL;
    }

    pFsg->CurDisk = pFsg->FileDisk.DiskID[cbw->bCBWLUN];

    return OK;
}

/*
Name:       FsgFwCmdHandle
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgFwCmdHandle(FSG_DEVICE *pFsg)
{
    uint8 *cdb = &pFsg->cbw->CBWCDB[0];

    if (0x20 == cdb[2]) //DevInfo Request
    {
        PRODUCT_DEVINFO info;

        FwGetDevInfo(&info);
        USBWriteEp(pFsg->BulkIn, 128, (uint8*)&info);
        FsgCSWHandler(pFsg, CASE6, pFsg->cbw->dCBWDataTransLen);
        FsgSendCSW(pFsg);
    }
    else if (0x04 == cdb[2]) //FW update
    {
        FsgForceDisconnect(pFsg);
    }
    else
    {

    }
}

/*
Name:       FsgScsiCmdHandle
Desc:       �������ݴ���SCSI�������
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgScsiCmdHandle(FSG_DEVICE *pFsg)
{
    uint8 *cmnd = &pFsg->cbw->CBWCDB[0];

    switch (cmnd[0])
    {
        //6-byte command
        case K_SCSICMD_TEST_UNIT_READY:     //0x00
            FsgTestUnitReady(pFsg);
            break;
        case K_SCSICMD_REQUEST_SENSE:           //0x03
            FsgRequestSense(pFsg);
            break;
        case K_SCSICMD_INQUIRY:             //0x12
            FsgInquiry(pFsg);
            break;
        case K_SCSICMD_MODE_SENSE_06:           //0x1a
            FsgModeSense06(pFsg);
            break;
        case K_SCSICMD_PREVENT_ALLOW_MEDIUM_REMOVAL:        //0x1e
            FsgPreventAllowMediumRemoval(pFsg);
            break;
        case K_SCSICMD_START_STOP_UNIT:     //0x1b
            FsgStartStopUnit(pFsg);
            break;

        case K_SCSICMD_MODE_SELECT_06:          //0x15
        case K_SCSICMD_MODE_SELECT_10:          //0x55
            FsgCSWHandler(pFsg, CASECMDFAIL, 0);
            FsgSendCSW(pFsg);
            break;
#if 0
        case K_SCSICMD_MODE_SELECT_06:          //0x15
            SCSICMD_ModeSelect06();
            G_ucBulkPhase = K_OutDataPhase;
            break;
        case K_SCSICMD_REZERO_UNIT:         //0x01
            SCSICMD_RezeroUnit();
            G_ucBulkPhase = K_InResponsePhase;
            break;
        case K_SCSICMD_FORMAT_UNIT:         //0x04
            SCSICMD_FormatUnit();
            G_ucBulkPhase = K_OutDataPhase;
            break;
        case K_SCSICMD_SEND_DIAGNOSTIC:     //0x1d
            SCSICMD_SendDiagnostic();
            G_ucBulkPhase = K_InResponsePhase;
            break;
#endif
            //10-byte command
        case K_SCSICMD_READ_CAPACITY:           //0x25
            FsgReadCapacity(pFsg);
            break;
        case K_SCSICMD_READ_10:             //0x28
            FsgRead10(pFsg);
            break;
        case K_SCSICMD_WRITE_10:                //0x2a
            FsgWrite10(pFsg);
            break;
        case K_SCSICMD_VERIFY_10:               //0x2f
            FsgVerify10(pFsg);
            break;
        case K_SCSICMD_MODE_SENSE_10:           //0x5a
            FsgModeSense10(pFsg);
            break;
#if 0
        case K_SCSICMD_SEEK_10:                 //0x2b
            SCSICMD_Seek10();
            G_ucBulkPhase = K_InResponsePhase;
            break;
        case K_SCSICMD_WRITE_AND_VERIFY_10:  //0x2e
            SCSICMD_WriteAndVerify10();
            G_ucBulkPhase = K_OutDataPhaseWrite;
            break;
        case K_SCSICMD_MODE_SELECT_10:          //0x55
            SCSICMD_ModeSelect10();
            G_ucBulkPhase = K_OutDataPhase;
            break;
        #endif
        //12-byte command
        case K_SCSICMD_READ_FORMAT_CAPACITIES:          //0x23
            FsgReadFormatCapacities(pFsg);
            break;

        case K_RKCMD_FIRMWAVE_UPGRADE:                //0xff
            FsgFirmwareUpgrade(pFsg);
            break;

        #if 0
        case K_RKCMD_LOCAL_FWUPDATE:
            break;
        #endif

        #if 0//def CD_ROM_EN
        case K_SCSICMD_READ_SUB_CHANNEL:        //0x42
            FsgReadSubChannel();
            break;
        case K_SCSICMD_READ_TOC:                //0x43
            FsgReadToc();
            break;
        case K_SCSICMD_USBGetConfiguration:     //0x46
            FsgGetConfiguration();
            break;
        case K_SCSICMD_GET_EVENT:               //0x4a
            FsgGetEvent();
            break;
#endif

        default:
            FsgCSWHandler(pFsg, CASECBW, 0);
            //FsgSenseData(SCSI_SENSE_ILLEGAL_REQUEST,SCSI_ADSENSE_ILLEGAL_COMMAND,0);
            USBDEBUG("error cbw = %d\n", cmnd[0]);
            //pFsg->BulkPhase = K_InCSWPhase;
            break;
    }
}

/*
Name:       FsgUpdateXfer
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgUpdateXfer(FSG_DEVICE *pFsg, uint32 byte)
{
    pUSB_BULK_XFER Xfer = &pFsg->BulkXfer;

    Xfer->XferLen += byte;
    Xfer->TransLen += byte;

    if (Xfer->XferCmd == USB_BULK_OUT)
    {
        if(Xfer->TransLen >= Xfer->NendLen)
        {
            Xfer->SendCsw = 1;
        }
    }

    if (Xfer->XferLen >= Xfer->BufLen)
    {
        uint32 len = Xfer->XferLen>>9;
        if (Xfer->XferCmd == USB_BULK_OUT)
        {
            FsgReqAdd(K_SCSICMD_WRITE_10, pFsg->CurDisk,  Xfer->LBA, len, Xfer->buf);
            //DISABLE_USB_INT;                        //���ж�, д���ڿ��ж�, ��֤buf������ȷ��
            //USBSetNak(pFsg->BulkOut);
            UsbIntDisalbe();

            if (Xfer->TransLen < Xfer->NendLen)
            {
                Xfer->LBA += len;
                Xfer->XferLen = 0;
                Xfer->BufLen = MIN((Xfer->NendLen-Xfer->TransLen), USB_BUF_SIZE);
            }
            //else
            //{
            //    pFsg->BulkPhase = K_InCSWPhase;
            //}
        }
        else        //Xfer->XferCmd == USB_BULK_IN
        {
            if (Xfer->TransLen < Xfer->NendLen)
            {
                Xfer->LBA += len;
                //Xfer->XferLen = 0;
                len = MIN((Xfer->NendLen-Xfer->TransLen), USB_BUF_SIZE);

                FsgReqAdd(K_SCSICMD_READ_10, pFsg->CurDisk,  Xfer->LBA, len>>9, Xfer->buf);
            }
            else
            {
                pFsg->BulkPhase = K_InCSWPhase;
            }
        }
    }
}

/*
Name:       FsgBulkInPkt
Desc:       �������ݴ���IN������
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgBulkInPkt(FSG_DEVICE *pFsg)
{
    if (!pFsg->connected)
        return;

    if (pFsg->BulkPhase == K_InDataPhase)
    {
        uint32 byte;
        pUSB_BULK_XFER Xfer = &pFsg->BulkXfer;

        byte = Xfer->BufLen - Xfer->XferLen;
        //printf("Buflen = %d, xferlen = %d\n", Xfer->BufLen, Xfer->XferLen);

        if (byte)
        {
            byte = (byte > pFsg->BulkMPS) ? pFsg->BulkMPS : byte;
            USBWriteEp(pFsg->BulkIn, byte, Xfer->buf+Xfer->XferLen);
            //memcpy(EpInData, Xfer->buf+Xfer->XferLen, pFsg->BulkMPS); //
            FsgUpdateXfer(pFsg, byte);
            Xfer->Redo = 0;
        }
    }
    else if (pFsg->BulkPhase == K_InCSWPhase)  // return the csw packet
    {
        FsgSendCSW(pFsg);
    }
}


/*
Name:       FsgBulkOutPkt
Desc:       �������ݴ���OUT������
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgBulkOutPkt(FSG_DEVICE *pFsg, uint16 len)
{
    if (pFsg->BulkPhase == K_CommandPhase)        //��������
    {
        pFsg->ActualCBWLen = len;
        if (len != USB_BULK_CB_WRAP_LEN)
        {
            /* The Bulk-only spec says we MUST stall the IN endpoint*/
            //USBStallEp(pFsg->BulkIn|0x80);
            uint8 dummy[512];
            USBReadEp(pFsg->BulkOut, len, dummy);
            return;
        }
        USBReadEp(pFsg->BulkOut, len, pFsg->cbw);
    }
    else if (pFsg->BulkPhase == K_OutDataPhase)//��������
    {
        pUSB_BULK_XFER Xfer = &pFsg->BulkXfer;

        if (Xfer->XferLen >= Xfer->BufLen)
            return;

        USBReadEp(BULK_OUT_EP, len, Xfer->buf+Xfer->XferLen);
        FsgUpdateXfer(pFsg, len);
    }
}

/*
Name:       FsgBulkOutHandle
Desc:       �������ݴ���BulkOnly�������
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgBulkOutHandle(FSG_DEVICE *pFsg)
{
    if (pFsg->BulkPhase == K_CommandPhase)
    {
        if (FsgCBWValidVerify(pFsg) == OK)
        {
            FsgScsiCmdHandle(pFsg);
        }
    }
    else if (pFsg->BulkPhase == K_OutDataPhase)
    {

    }
}

/*
Name:       FsgSetConfig
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgSetConfig(FSG_DEVICE *pFsg, uint8 config)
{
    pFsg->config = config;

    if (config)
    {
        USB_FSG_CONFIGS_DESCRIPTOR *ptr;

        ptr = (pFsg->pDev->speed==USB_SPEED_HIGH)? &HSFsgConfigs: &FSFsgConfigs;

        USBEnableEp(pFsg->BulkOut, &ptr->BulkOut);
        USBEnableEp(pFsg->BulkIn|0x80, &ptr->BulkIn);

        if (0==pFsg->connected)
        {
            pFsg->connected = 1;
            FUSBConnectHook();
        }
    }

    USBInEp0Ack();
}

/*
Name:       FsgStandardReq
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static int32 FsgStandardReq(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    FSG_DEVICE *pFsg = (FSG_DEVICE*)pDev->pFunDev;
    int32 ret = 0;

    switch (ctrl->bRequest)
    {
        case USB_REQ_GET_DESCRIPTOR:
            //FsgGetDescriptor(ctrl, pDev);
            ret = -1;
            break;
        /* One config, two speeds */
        case USB_REQ_SET_CONFIGURATION:
            USBDEBUG("Set Config = %d", ctrl->wValue);
            FsgSetConfig(pFsg, ctrl->wValue & 0x1);
            if (pDev->next)
                pDev->next->setup(ctrl, pDev->next);
            break;

        case USB_REQ_GET_CONFIGURATION:
        {
            uint8 config = pFsg->config;
            USBDEBUG("get Config = %d", pFsg->config);
            USBWriteEp0(1, &config);
            break;
        }
        case USB_REQ_SET_INTERFACE:
            pFsg->AlterInterface = ctrl->wValue & 0x1;
            USBDEBUG("set Interface = %d", ctrl->wValue);
            USBInEp0Ack();
            break;

        case USB_REQ_GET_INTERFACE:
        {
            uint8 intf = pFsg->AlterInterface;
            USBDEBUG("get Interface = %d", pFsg->AlterInterface);
            USBWriteEp0(1, &intf);
            break;
        }
        default:
            break;
    }

    return ret;
}


/*
Name:       FsgClassReq
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgClassReq(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    FSG_DEVICE *pFsg = (FSG_DEVICE*)pDev->pFunDev;
    /* Handle Bulk-only class-specific requests */
    if (ctrl->bRequest == USB_BULK_GET_MAX_LUN_REQUEST)     //0xfe:GET MAX LUN
    {
        uint8 lun = pFsg->FileDisk.LunNum-1;
        USBDEBUG("GET LUN = %d", lun);
        USBWriteEp0(1, &lun);
        return;
    }
    else if (ctrl->bRequest == USB_BULK_RESET_REQUEST)
    {
        USBDEBUG("BULK RESET");
        if (0 == ctrl->wLength)
        {
            pFsg->BulkPhase = K_CommandPhase;
            USBInEp0Ack();
        }
        /*else
        {
        }*/
        return;
    }

    if (pDev->next)
        pDev->next->setup(ctrl, pDev->next);
}

/*
Name:       FsgSetup
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static int32 FsgSetup(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev)
{
    uint8 type = ctrl->bRequestType & USB_TYPE_MASK;
    int32 ret = 0;

    if (type == USB_TYPE_STANDARD)
    {
        ret = FsgStandardReq(ctrl, pDev);
    }

    else if (type == USB_TYPE_CLASS)
    {
        FsgClassReq(ctrl, pDev);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

/*
Name:       FsgReqest
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgReqest(uint8 epnum, uint32 event, uint32 param, USB_DEVICE *pDev)
{
    FSG_DEVICE *pFsg = (FSG_DEVICE *)pDev->pFunDev;

    if (epnum == pFsg->BulkOut || (epnum == pFsg->BulkIn))
    {
        switch (event)
        {
            case UDC_EV_OUT_PKT_RCV:
                FsgBulkOutPkt(pFsg, (uint16)param);
                break;

            case UDC_EV_OUT_XFER_COMPL:
                FsgBulkOutHandle(pFsg);
                break;

            case UDC_EV_IN_XFER_COMPL:
                //USBDEBUG("BULK EP IN OVER");
                FsgBulkInPkt(pFsg);
                break;

            case UDC_EV_IN_XFER_TIMEOUT:
            {

                #if 0
                USB_FSG_CONFIGS_DESCRIPTOR *ptr;

                ptr = (pFsg->pDev->speed==USB_SPEED_HIGH)? &HSFsgConfigs: &FSFsgConfigs;

                pFsg->BulkXfer.Redo = 1;
                USBEnableEp(pFsg->BulkIn|0x80, &ptr->BulkIn);
                USBDEBUG("Ep%d timeout",epnum);

                USBWriteEp(pFsg->BulkIn, pFsg->BulkMPS, EpInData);
                printf("BulkMPS = %d\n", pFsg->BulkMPS);
                #else
                pFsg->BulkXfer.Redo = 1;
                USBDEBUG("Ep%d timeout",epnum);
                USBSetEpSts(1, BULK_IN_EP, 1);
                {
                    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
                    uint32 count;

                   /* Flush the FIFOs */
                    otg_core->Core.grstctl |= ( 0x10<<6) | (1<<5);     //Flush all Txfifo
                    for (count=0; count<10000; count++)
                    {
                        if ((otg_core->Core.grstctl & (1<<5))==0)
                            break;
                    }

                    otg_core->Core.grstctl |= 1<<4;              //Flush all Rxfifo
                    for (count=0; count<10000; count++)
                    {
                        if ((otg_core->Core.grstctl & (1<<4))==0)
                            break;
                    }

                    /* Flush the Learning Queue. */
                    otg_core->Core.grstctl |= 1<<3;

                }
                #endif
            }
            break;

            case UDC_EV_IN_XFER_REDO:
                /*
                            if(pFsg->BulkXfer.Redo)
                            {
                                USBWriteEp(pFsg->BulkIn, pFsg->BulkMPS, EpInData);
                                printf("BulkMPS = %d\n", pFsg->BulkMPS);
                            }
                        */
                break;


        /*
            case UDC_EV_ENUM_DONE:
                if ((uint8)param == USB_SPEED_FULL)
                {
                    pFsg->BulkMPS = FS_BULK_TX_SIZE;
                }
                break;

            case UDC_EV_CLEAN_EP:
                if (pFsg->BulkPhase == K_InCSWPhase)
                    FsgSendCSW(pFsg);

                USBDEBUG("Send CSW");
            */

            default:
                break;
        }
        //return;
    }


    if (event == UDC_EV_ENUM_DONE)
    {
        if ((uint8)param == USB_SPEED_FULL)
        {
            pFsg->BulkMPS = FS_BULK_TX_SIZE;
        }
        else if ((uint8)param == USB_SPEED_HIGH)
        {
            pFsg->BulkMPS = HS_BULK_TX_SIZE;
        }

        //printf("enum ok\n");

    }
    else if (event == UDC_EV_CLEAN_EP)
    {

        if(pFsg->BulkXfer.Redo)
        {
            //USBWriteEp(pFsg->BulkIn, pFsg->BulkMPS, EpInData);
            USBDEBUG("USB REDO");
        }
        else if (pFsg->BulkPhase == K_InCSWPhase)
        {
            FsgSendCSW(pFsg);
            //USBDEBUG("Send CSW");
        }

    }

    if (pDev->next)
        pDev->next->request(epnum, event, param, pDev->next);
}


/*
Name:       FsgResume
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgResume(USB_DEVICE *pDev)
{
    SendMsg(MSG_USB_RESUMED_FUSB);
    if (pDev->next)
        pDev->next->resume(pDev->next);
}

/*
Name:       FsgSuspend
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgSuspend(USB_DEVICE *pDev)
{
    SendMsg(MSG_USB_SUSPEND_FUSB);
    if (pDev->next)
        pDev->next->suspend(pDev->next);
}


/*
Name:       FsgDisconnect
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgDisconnect(USB_DEVICE *pDev)
{
    FSG_DEVICE *pFsg = (FSG_DEVICE *)pDev->pFunDev;

    SendMsg(MSG_USB_DISCONNECT_FUSB);
    pFsg->BulkPhase = K_CommandPhase;
    memset(pFsg->DiskReq, 0, sizeof(FSG_LUN_REQ));
    pFsg->connected = 0;
    if (pDev->next)
        pDev->next->disconnect(pDev->next);
}

/*
Name:       FsgMDNotify
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgMDNotify(uint16 major, uint32 event, uint32 param)
{
    FSG_LUN *pDisk = &FsgDevice.FileDisk;

    if (MD_EVENT_CHANGE == event /*&& 0==param*/)
    {
        uint32 i;

        for (i=0; i<pDisk->LunNum; i++)
        {
            if (major == MD_MAJOR(pDisk->DiskID[i]))
            {
                pDisk->changed[i] = 1;
            }
        }
    }
}

/*
Name:       FsgFindLun
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
static void FsgFindLun(FSG_DEVICE *pFsg)
{
    FSG_LUN *pDisk = &pFsg->FileDisk;

    MDScanDev(1);
    pDisk->LunNum = MDGetUDiskTab(&pDisk->DiskID[0], 0x0F);

    #ifdef _MULT_DISK_
    if (bShowFlash1 == 0)
    {
       uint32 i,j;

       for(i=0; i<pDisk->LunNum; i++)
        {
            if (UserDisk1ID == pDisk->DiskID[i])
            {
                for(j=i; j<pDisk->LunNum-1; j++)
                    pDisk->DiskID[j] = pDisk->DiskID[j+1];

                pDisk->DiskID[j] = 0;
                pDisk->LunNum--;
            }
        }
    }
    #endif

    MDRegNotify(FsgMDNotify);

}

/*
Name:       FsgInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
int32 FsgInit(void *pArg)
{
    FSG_DEVICE *pFsg = &FsgDevice;

#ifndef USB_DISPLAY
    IntDisable(FAULT_ID15_SYSTICK);
#endif

    memset(pFsg, 0, sizeof(FSG_DEVICE));
    pFsg->pDev = &FsgDriver;

    pFsg->BulkXfer.buf = (uint8*)FsgBulkBuf;
    pFsg->cbw = (FSG_CBW *)FsgEpBuf;
    pFsg->BulkOut = BULK_OUT_EP;
    pFsg->BulkIn = BULK_IN_EP;
    pFsg->BulkMPS = HS_BULK_RX_SIZE;
    pFsg->mode = (uint8)pArg;

    FsgFindLun(pFsg);

    USBDriverProbe(&FsgDriver);

    return 0;
}

/*
Name:       FsgDeInit
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
void FsgDeInit(void)
{
    FSG_DEVICE *pFsg = &FsgDevice;

#ifndef USB_DISPLAY
    IntEnable(FAULT_ID15_SYSTICK);
#endif
    FsgFlush(pFsg);
    MDUnRegNotify(FsgMDNotify);
}

/*
Name:       FsgService
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_USB_MSC_CODE_
int32 FsgThread(void)
{
    static uint32 timer1 = 0;
    static uint32 timer2 = 0;
    static uint32 timer3 = 0;
    int32 ret = 0;
    FSG_LUN_REQ *req;
    FSG_DEVICE *pFsg = &FsgDevice;

    do
    {
        if (GetMsg(MSG_USB_SUSPEND_FUSB) || GetMsg(MSG_USB_DISCONNECT_FUSB))
        {
            FsgFlush(pFsg);
        }

        if (0 == CheckVbus() || (0 == USBIsConnect()))
        {
            //SendMsg(MSG_USB_DISCONNECT);
            break;
        }
        if (1 == USBIsSuspend())
        {
            SendMsg(MSG_USB_SUSPEND);
            break;
        }

        if (GetMsg(MSG_USB_RESUMED_FUSB))
        {
            SendMsg(MSG_USB_RESUMED);
        }

        req = FsgReqNext(pFsg);             //��ȡ��һ������
        if (req)
        {
            pUSB_BULK_XFER Xfer = &pFsg->BulkXfer;
            timer1 = 0;
            timer3 = 0;

            if (req->cmd == K_SCSICMD_WRITE_10)
            {
                if (req->cmd)
                {
                    MDWrite(req->LUN, req->LBA, req->len, req->buf);
                    //MscChkWrite(req->LUN, req->LBA, req->len, req->buf);
                    req->cmd = 0;
                    FUSBUpdateHook();
                    FUSBWriteHook();
                }
                //USBClearNak(pFsg->BulkOut);
                //ENABLE_USB_INT;
                DisableIntMaster();
                UsbIntEnable();
                EnableIntMaster();
                #if 0
                if (K_InCSWPhase == pFsg->BulkPhase)  // return the csw packet
                {
                    FsgSendCSW(pFsg);
                }
                #else
                if (Xfer->SendCsw)
                {
                    FsgSendCSW(pFsg);
                    Xfer->SendCsw = 0;
                }
                #endif
            }
            else if (req->cmd == K_SCSICMD_READ_10)
            {
                MDRead(req->LUN, req->LBA, req->len, req->buf);
                //printf("MDRead: %x, %d \n", req->LBA, req->len);

                Xfer->XferLen = 0;
                Xfer->BufLen = req->len<<9;
                req->cmd = 0;

                //DISABLE_USB_INT;

                DisableIntMaster();
                UsbIntDisalbe();
                EnableIntMaster();

                FsgBulkInPkt(pFsg);
                //ENABLE_USB_INT;

                DisableIntMaster();
                UsbIntEnable();
                EnableIntMaster();

                FUSBReadHook();


            }
            else if (req->cmd == K_RKCMD_FLUSH_REQUEST)
            {

            }
            else if (req->cmd == K_SCSICMD_PREVENT_ALLOW_MEDIUM_REMOVAL)
            {
                req->cmd = 0;
            }
        }
        else
        {
            timer1++;
            if (timer1 >= 30) // real delay 300ms
            {
                timer2++;
                timer1 = 0;

                if (++timer3 == 16)
                {
                    timer3 = 17;
                    FUSBIdleHook();
                }
            }
            if (timer2 >= 100)
            {
                timer2 = 0;
#if defined(_SDCARD_)
                //DISABLE_USB_INT;
                DisableIntMaster();
                UsbIntDisalbe();
                EnableIntMaster();

                MDScanDev(1);
                //ENABLE_USB_INT;
                DisableIntMaster();
                UsbIntEnable();
                EnableIntMaster();
#endif

            }

            if (pFsg->mode)
                USBDelayUS(1);
        }
    }
    while (pFsg->mode);

    return ret;
}

#endif

