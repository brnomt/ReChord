#ifndef _USB_MSC_HOST_H_
#define _USB_MSC_HOST_H_

#ifdef _USB_HOST_
#include "MDConfig.h"

#define    MAX_LUN_NUM             3

#define    USB_BBB_RESET           0xff
#define    USB_BBB_GET_MAX_LUN     0xfe

#define    SCSI_CHANGE_DEF         0x40    /* Change Definition (Optional) */
#define    SCSI_COMPARE            0x39    /* Compare (O) */
#define    SCSI_COPY               0x18    /* Copy (O) */
#define    SCSI_COP_VERIFY         0x3A    /* Copy and Verify (O) */
#define    SCSI_INQUIRY_CMD        0x12    /* Inquiry (MANDATORY) */
#define    SCSI_LOG_SELECT         0x4C    /* Log Select (O) */
#define    SCSI_LOG_SENSE          0x4D    /* Log Sense (O) */
#define    SCSI_MODE_SEL6          0x15    /* Mode Select 6-byte (Device Specific) */
#define    SCSI_MODE_SEL10         0x55    /* Mode Select 10-byte (Device Specific) */
#define    SCSI_MODE_SEN6          0x1A    /* Mode Sense 6-byte (Device Specific) */
#define    SCSI_MODE_SEN10         0x5A    /* Mode Sense 10-byte (Device Specific) */
#define    SCSI_READ_BUFF          0x3C    /* Read Buffer (O) */
#define    SCSI_REQ_SENSE          0x03    /* Request Sense (MANDATORY) */
#define    SCSI_SEND_DIAG          0x1D    /* Send Diagnostic (O) */
#define    SCSI_TST_U_RDY          0x00    /* Test Unit Ready (MANDATORY) */
#define    SCSI_WRITE_BUFF         0x3B    /* Write Buffer (O) */

#define    CBWSIGNATURE            0x43425355
#define    CBWFLAGS_OUT            0x00
#define    CBWFLAGS_IN             0x80
#define    CBWCDBLENGTH            16

#define    UMASS_BBB_CBW_SIZE      31

#define    CSWSIGNATURE            0x53425355
#define    CSWSTATUS_GOOD          0x0
#define    CSWSTATUS_FAILED        0x1
#define    CSWSTATUS_PHASE         0x2

#define    UMASS_BBB_CSW_SIZE       13


#define    SCSI_COMPARE            0x39    /* Compare (O)                      */
#define    SCSI_FORMAT             0x04    /* Format Unit (MANDATORY)          */
#define    SCSI_LCK_UN_CAC         0x36    /* Lock Unlock Cache (O)            */
#define    SCSI_PREFETCH           0x34    /* Prefetch (O)                     */
#define    SCSI_MED_REMOVL         0x1E    /* Prevent/Allow medium Removal (O) */
#define    SCSI_READ6              0x08    /* Read 6-byte (MANDATORY)          */
#define    SCSI_READ10             0x28    /* Read 10-byte (MANDATORY)         */
#define    SCSI_RD_FORMAT_CAPAC    0x23
#define    SCSI_RD_CAPAC           0x25    /* Read Capacity (MANDATORY)        */
#define    SCSI_RD_DEFECT          0x37    /* Read Defect Data (O)             */
#define    SCSI_READ_LONG          0x3E    /* Read Long (O)                    */
#define    SCSI_REASS_BLK          0x07    /* Reassign Blocks (O)              */
#define    SCSI_RCV_DIAG           0x1C    /* Receive Diagnostic Results (O)   */
#define    SCSI_RELEASE            0x17    /* Release Unit (MANDATORY)         */
#define    SCSI_REZERO             0x01    /* Rezero Unit (O)                  */
#define    SCSI_SRCH_DAT_E         0x31    /* Search Data Equal (O)            */
#define    SCSI_SRCH_DAT_H         0x30    /* Search Data High (O)             */
#define    SCSI_SRCH_DAT_L         0x32    /* Search Data Low (O)              */
#define    SCSI_SEEK6              0x0B    /* Seek 6-Byte (O)                  */
#define    SCSI_SEEK10             0x2B    /* Seek 10-Byte (O)                 */
#define    SCSI_SEND_DIAG          0x1D    /* Send Diagnostics (MANDATORY)     */
#define    SCSI_SET_LIMIT          0x33    /* Set Limits (O)                   */
#define    SCSI_START_STP          0x1B    /* Start/Stop Unit (O)              */
#define    SCSI_SYNC_CACHE         0x35    /* Synchronize Cache (O)            */
#define    SCSI_VERIFY             0x2F    /* Verify (O)                       */
#define    SCSI_WRITE6             0x0A    /* Write 6-Byte (MANDATORY)         */
#define    SCSI_WRITE10            0x2A    /* Write 10-Byte (MANDATORY)        */
#define    SCSI_WRT_VERIFY         0x2E    /* Write and Verify (O)             */
#define    SCSI_WRITE_LONG         0x3F    /* Write Long (O)                   */
#define    SCSI_WRITE_SAME         0x41    /* Write Same (O)                   */

typedef struct tagSCSI_CMD
{
    uint8       cmd[16];            /* command           */
    uint32      sense[64/4];        /* for request sense */
    uint8       status;             /* SCSI Status       */
    uint8       lun;                /* Target LUN        */
    uint8       cmdlen;             /* command len       */
    uint8       dirin;              /* Is dir in         */
    uint8       *pdata;             /* pointer to data   */
    uint32      datalen;            /* Total data length */
    //uint32    actlen;             /* tranfered bytes   */
} SCSI_CMD, pSCSI_CMD;



typedef    struct tagMSC_HOST
{
    uint8       valid;
    uint8       BulkOut;
    uint8       BulkIn;
    uint8       LunNum;

    uint32      SendCtrlPipe;
    uint32      RecvCtrlPipe;
    uint32      SendBulkPipe;
    uint32      RecvBulkPipe;

    uint8       LunMap[MAX_LUN_NUM];
    uint32      capacity[MAX_LUN_NUM];   //����

    HOST_DEV    *pDev;
}MSC_HOST, *pMSC_HOST;

extern int32  MscHost(HOST_INTERFACE* interfaceDes);
extern uint32 USBMSCHost_Write (uint8 lun, uint32 LBA, uint32 nSec, void *pBuf);
extern uint32 USBMSCHost_Read (uint8 lun, uint32 LBA, uint32 nSec, void *pBuf);
extern uint32 USBMSCHost_GetInfo(uint8 lun, pMEMDEV_INFO pDevInfo);
extern uint32 USBMSCHost_Init(void);
extern uint32 USBMSCHost_DeInit(void);
extern bool   USBMSCHost_ChkEvent(uint32 event);
extern uint32 USBMSCHost_Probe(void);

#endif

#endif
