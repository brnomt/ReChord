#ifndef    _USBCONTROL_
#define    _USBCONTROL_

#undef  EXT
#ifdef _IN_USB_CONTROL_
#define EXT
#else
#define EXT extern
#endif


/*
*-------------------------------------------------------------------------------
*  
*                           Macro define
*  
*-------------------------------------------------------------------------------
*/
#define         USB_CLASS_TYPE_DEFAULT      (USB_CLASS_TYPE_MSC|USB_CLASS_TYPE_AUDIO|USB_CLASS_TYPE_SERIAL)

/*
*-------------------------------------------------------------------------------
*  
*                           Struct define
*  
*-------------------------------------------------------------------------------
*/
typedef struct tagFUNCTION_USB
{
    uint32 SelClassType;
    uint8 StorageUpdate;
    uint8 StorageStatus;
    uint8 MscCmdStatus;
    uint32 MscStatusStartCounter;
    uint8 DevNum;
} FUNCTION_USB, *pFUNCTION_USB;

typedef  int32 (*FUDINIT)(void);
typedef  int32 (*FUDTHREAD)(void);
typedef  void (*FUDDEINIT)(void);

typedef struct tagFUN_USB_DEV
{
    uint32 ClassType;
    FUDINIT FUInit;
    FUDTHREAD FUThead;
    FUDDEINIT FUDeinit;
} FUN_USB_DEV, *pFUN_USB_DEV;

typedef __packed struct tagPRODUCT_DEVINFO
{
    uint8 DevInfo[7];
    uint8 size;
    uint8 reserved1[8];                  //8-15 
    uint8 VendorID[8];                  //16-23
    uint8 ProductID[16];                //24-39
    uint8 ProductRevision[4];           //40-43
    uint8 ProductSubrevision[4];        //44-47
    uint8 StorageSize[4];               //48-51
    uint8 SerialNum[16];                //52-67
    uint8 reserved2[28];                 //68-95
    uint8 SubID[4];                     //96-99
    uint8 VendorSpecific[28];           //100-127
}PRODUCT_DEVINFO, *pPRODUCT_DEVINFO;

/*
*-------------------------------------------------------------------------------
*  
*                           Variable define
*  
*-------------------------------------------------------------------------------
*/
_ATTR_USBCONTROL_BSS_ EXT uint32 FunUsbLdoBak;
_ATTR_USBCONTROL_BSS_ EXT uint32 UsbIdleCounter;
_ATTR_USBCONTROL_BSS_ EXT FUNCTION_USB  FUSBDevice;

/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
void   USBControlInit(void *pArg);
UINT32 USBControlService(void);
void   USBControlDeInit(void);

/*
--------------------------------------------------------------------------------
  
  Description: USB thread definition.
  
--------------------------------------------------------------------------------
*/
#ifdef _IN_USB_CONTROL_
_ATTR_USBCONTROL_DATA_ EXT THREAD USBControlThread = {

    NULL,
    NULL,
    
    USBControlInit,
    USBControlService,
    USBControlDeInit,
    
    NULL                           
};
#else
_ATTR_USBCONTROL_DATA_ EXT THREAD USBControlThread;
#endif
#endif

