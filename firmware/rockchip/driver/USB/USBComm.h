/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: USBComm.h
Desc:

Author: chenfen@rock-chips.com
Date: 12-06-20
Notes:

$Log: $
 *
 *
*/

#ifndef _USBCOMM_H
#define _USBCOMM_H

/*-------------------------------- Includes ----------------------------------*/
#include "SysConfig.h"
#include "typedef.h"


/*------------------------------ Global Defines ------------------------------*/

#define USB_STRING_MANUFACTURER                         1
#define USB_STRING_PRODUCT                              2
#define USB_STRING_SERIAL                               3
#define USB_STRING_CONFIG                               0
#define USB_STRING_INTERFACE                            0
#define USB_STRING_MS_OS                                0xee


#define USB_SPEED_LS            0x00
#define USB_SPEED_FS            0x01
#define USB_SPEED_HS            0x02

#define MAX_FIFO_SIZE           0x200

#define CBWCDBLENGTH            16


#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */


#define __get_unaligned_be32(p)      (uint32)(p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3])

#define __get_unaligned_be16(p)      (uint16)(p[0] << 8 | p[1])

#define         MIN(x,y) ((x) < (y) ? (x) : (y))


/*------------------------------ Global Typedefs -----------------------------*/

enum DATA_DIRECTION
{
	DATA_DIR_FROM_HOST = 0,
	DATA_DIR_TO_HOST,
	DATA_DIR_NONE
};

/* Command Block Wrapper */
typedef __packed struct _HOSTMSC_CBW
{
    uint32      dCBWSignature;
    uint32      dCBWTag;
    uint32      dCBWDataTransLen;
    uint8       bCBWFlags;
    uint8       bCBWLUN;
    uint8       bCDBLength;
    uint8       CBWCDB[CBWCDBLENGTH];
} HOST_CBW, FSG_CBW, *pHOST_CBW;


/* Command Status Wrapper */
typedef __packed struct _HOSTMSC_CSW
{
    uint32      dCSWSignature;
    uint32      dCSWTag;
    uint32      dCSWDataResidue;
    uint8       bCSWStatus;
} HOST_CSW, FSG_CSW;

/*----------------------------- External Variables ---------------------------*/

extern	uint8 USBLangId[];
extern	uint8 StringProduct[];
extern	uint8 StringManufacture[];
extern	uint8 StringSerialNumbel[];
extern  uint8 MSOSString[];
extern  uint8 USBD_STATUS_STALL_PID[];

/*------------------------- Global Function Prototypes -----------------------*/

extern void USBDelayUS(uint32 us);

extern void USBDelayMS(uint32 us);

extern void USBResetPhy(void);

extern uint8 USBOtgMode(void); // 1: Host mode  0: Device mode

//extern int32 USBGetDescriptor(USB_CTRL_REQUEST *ctrl, USB_DEVICE *pDev);

#endif
