/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:       USBControl.C
*
* Description:
*
* History:      <author>          <time>        <version>
*                 ZS      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_USB_CONTROL_

#include "SysInclude.h"
#include "freq_enums.h"           /* FREQ_BLON */
#include "driver/CRU/Hw_cru.h"     /* USBPHY_SRST, CLK_USBPHY_GATE */

#ifdef _USB_

#include "fsinclude.h"
#include "USBConfig.h"
#include "USBControl.h"


/*
--------------------------------------------------------------------------------

   Forward Declaration

--------------------------------------------------------------------------------
*/
extern int32 FUSBFsgInit(void);
extern void  FUSBFsgDeInit(void);
extern int32 FUSBSerialInit(void);

/*
*-------------------------------------------------------------------------------
*
*                           Struct Data define
*
*-------------------------------------------------------------------------------
*/
_ATTR_USBCONTROL_DATA_
static FUN_USB_DEV FUSBDevTab[] =
{
    #ifdef USB_MSC
    {
        USB_CLASS_TYPE_MSC,
        FUSBFsgInit,
        FsgThread,
        FUSBFsgDeInit,
    },
    #endif
    #if 0
    {
        USB_CLASS_TYPE_MTP,
        FUSBMtpInit,
        RunMtpDevice,
        MtpDeInit,
    },
    #endif
    #ifdef USB_SERIAL
    {
        USB_CLASS_TYPE_SERIAL,
        FUSBSerialInit,
        SrlThread,
        SrlDeInit,
    }
    #endif
};

_ATTR_USBCONTROL_DATA_
static int Really_Suppend = 0;

#ifdef USB_MSC
#include "AudioControl.h"

/*
--------------------------------------------------------------------------------
  Function name : void UsbBusyHook(void)
  Author        : ZhengYongzhi
  Description   : USB busy status flag function

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_MSC_CODE_
void FUSBBusyHook(void)
{

}

/*
--------------------------------------------------------------------------------
  Function name : void UsbReadyHook(void)
  Author        : ZhengYongzhi
  Description   : USB Ready status flag function

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_MSC_CODE_
void FUSBReadyHook(void)
{

}

/*
--------------------------------------------------------------------------------
  Function name : void UsbConnectedHook(void)
  Author        : ZhengYongzhi
  Description   : USB Connected status flag function

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_MSC_CODE_
void FUSBConnectHook(void)
{
    if(GetMsg(MSG_USB_CONNECT_FAIL))
    {
        BatteryChargeInit();
        SendMsg(MSG_CHARGE_START);      //ʹ�ܳ��
        FREQ_EnterModule(FREQ_BLON);    //�Ƿ��Ӱ�쵽USB
        SendMsg(MSG_SYS_RESUME);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void UsbRKMSCHook(uint32 cmd)
  Author        : ZhengYongzhi
  Description   : USB busy status display function

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_MSC_CODE_
void FUSBRKCmdHook(uint32 cmd, uint32 param)
{
    if (cmd == 0xFFFFFFFE)
    {
        if (0 == param)
            SendMsg(MSG_MES_FIRMWAREUPGRADE);//system backgroud check this message ,and execute the firmware update
        else
            SendMsg(MSG_ENTER_LOADER_USB);
    }

    #ifdef _CDROM_
    else if (cmd == 0xFFFFFFF6)       // Show User Disk, not enum the cdrom
    {
        gSysConfig.bShowCdrom = 0;
        SendMsg(MSG_SYS_REBOOT);
    }
    #endif
}

/*
--------------------------------------------------------------------------------
Name:       FUSBFsgUpdateHook
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
--------------------------------------------------------------------------------
*/
_ATTR_USB_MSC_CODE_
void FUSBUpdateHook(void)
{
    FUSBDevice.StorageUpdate = 1;
}

_ATTR_USB_MSC_CODE_
void FUSBWriteHook(void)
{
    if (FUSBDevice.StorageStatus != 1)
    {
        FUSBDevice.StorageStatus = 1;
        FUSBDevice.MscStatusStartCounter = SysTickCounter;
    }
}


_ATTR_USB_MSC_CODE_
void FUSBReadHook(void)
{
    if (FUSBDevice.StorageStatus != 2)
    {
        FUSBDevice.StorageStatus = 2;
        FUSBDevice.MscStatusStartCounter = SysTickCounter;
    }
}

_ATTR_USB_MSC_CODE_
void FUSBIdleHook(void)
{
    FUSBDevice.StorageStatus = 0;
    FUSBDevice.MscStatusStartCounter = SysTickCounter;
}

/*
--------------------------------------------------------------------------------
  Function name : void UsbShowCdrom(void)
  Author        : ZhengYongzhi
  Description   : USB busy status display function

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_MSC_CODE_
BOOL FUSBShowCdrom(void)
{
    #ifdef _CDROM_
        return bShowCdrom;
    #else
        return 0;
    #endif

}

/*
--------------------------------------------------------------------------------
  Function name :  void USBMSCInit(void )
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_MSC_CODE_
int32 FUSBFsgInit(void)
{
    return FsgInit((void*)0);
}

/*
--------------------------------------------------------------------------------
  Function name :  void USBMSCDeInit(void)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_MSC_CODE_
void FUSBFsgDeInit(void)
{
    FsgDeInit();
    //-----------------------------------------------------------------
    {
        #ifdef _MEDIA_MODULE_
        if(FUSBDevice.StorageUpdate)
        {
            gSysConfig.MedialibPara.MediaUpdataFlag = 1;
        }
        #endif

        #ifdef AUDIOHOLDONPLAY
        if (FUSBDevice.StorageUpdate)    //Memap switch,clear breakpoint.
        {
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0 ;//zs 2009-5-18
        }
        #endif

        #ifdef _CDROM_
        bShowCdrom = 1;
        #endif
    }

    //#if(defined (_SDCARD_) || defined (_MULT_DISK_))
    gSysConfig.Memory =  FLASH0;
    FileSysSetup(gSysConfig.Memory);
    //#endif

    //zyz: After USB Transfer, need get Flash & Card free memory
    SendMsg(MSG_FLASH_MEM0_UPDATE);
}

/*
--------------------------------------------------------------------------------
  Function name : UsbBGetVetsion
  Author        : ZhengYongzhi
  Description   : USB get system version number.

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
extern uint16 LoaderVer;
_ATTR_USB_MSC_CODE_
void FUSBGetVetsion(PRKNANO_VERSION pVersion)
{
    pVersion->dwBootVer = (uint16)LoaderVer;
    pVersion->dwFirmwareVer = (((uint32)gSysConfig.MasterVersion & 0xFF) << 24) |
                              (((uint32)gSysConfig.SlaveVersion  & 0xFF) << 16) |
                               ((uint32)gSysConfig.SmallVersion & 0xFFFF);
}

#endif

#ifdef USB_SERIAL
/*
--------------------------------------------------------------------------------
Name:       _ATTR_USB_MSC_CODE_
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
--------------------------------------------------------------------------------
*/
_ATTR_USB_SRL_CODE_
static int32 FUSBSerialInit(void)
{
    return SrlInit((void*)0);
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : void Os_idle(void)
  Author        : ZHengYongzhi
  Description   : system enter idle

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
extern UDC_DRIVER UDCDriver;
_ATTR_USBCONTROL_CODE_
void USBControlIdle_Hook(void)
{
    while(1)
    {
#ifdef _WATCH_DOG_
        WatchDogReload();
#endif

        if (CheckVbus() == 0)
        {
            DEBUG("VBUS....");
            break;
        }
        #if 0
        if (CheckMsg(MSG_USB_RESUMED_FUSB))
        {
            DEBUG("Resumed....");
            break;
        }
        #else
        if (UDCDriver.suspend == 0)
        {
            SendMsg(MSG_USB_RESUMED);
            break;
        }
        #endif

        DelayMs(5);
        #if 0
        if ((SysTickCounter - UsbIdleCounter > 60)&&(Really_Suppend == 0))
        {
            BatteryChargeDeInit();
            BL_Off();
            LcdStandby();
            Really_Suppend = 1;
        }
        #endif
    }
}

_ATTR_USBCONTROL_CODE_
void USBControlIdle(void)
{
    uint32 LDOBack,DCDCBack;
    uint32 SysClkBack;
    uint32 hpcon_bk, hpdac_bk;
    uint32 scu_clkgate0, scu_clkgate1;

    //DEBUG("Enter");
    DelayMs(10);

    //Enter Idle
    #if 0//def DEEP_SLEEP
    DebugDisable();

    SysTickDisable();
    AdcPowerDown();

    //wakeup intrrupt
    IntPendingClear(INT_ID26_GPIO);
    IntPendingClear(INT_ID39_OOL_PLAYON);
    IntPendingClear(INT_ID40_PWR_5V_READY);
    IntEnable(INT_ID26_GPIO);
    IntEnable(INT_ID39_OOL_PLAYON);
    IntEnable(INT_ID40_PWR_5V_READY);
    #endif

    #if 0 // def DEEP_SLEEP
    DCDCBack = VDD_VoltageAdjust_Set(SCU_DCOUT_080);
    __WFI();
    VDD_VoltageAdjust_Set(DCDCBack);
    #else
    USBControlIdle_Hook();
    #endif

    //Peripheral resume
    #if 0 //def DEEP_SLEEP
    IntDisable(INT_ID26_GPIO);
    IntDisable(INT_ID39_OOL_PLAYON);
    IntDisable(INT_ID40_PWR_5V_READY);

    AdcPowerUp();
    SysTickEnable();

    DebugEnable();
    #endif

    //DEBUG("Exit");
}

/*
--------------------------------------------------------------------------------
  Function name :  void USBControlInit(void *pArg)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_USBCONTROL_CODE_
void USBControlInit(void *pArg)
{
    uint32 i;
    int32 ret;
    pFUN_USB_DEV   pDev = &FUSBDevTab[0];
    pFUNCTION_USB  pFUSB = &FUSBDevice;
    uint32 USBClassType = ((USB_WIN_ARG*)pArg)->FunSel;

    BLOffDisable();

    memset(pFUSB, 0, sizeof(FUNCTION_USB));
    pFUSB->SelClassType = USBClassType;

    USBDriverInit();

    // Install USB Device
    for(i=0; i<sizeof(FUSBDevTab)/sizeof(FUN_USB_DEV); i++, pDev++)
    {
        if (USBClassType & pDev->ClassType)
        {
            if (pDev->ClassType == USB_CLASS_TYPE_MSC)
            {
                ModuleOverlay(MODULE_ID_USB_MSC, MODULE_OVERLAY_ALL);
                ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
            }

            ret = pDev->FUInit();
            if (ret < 0)
            {
                //printf("FUInit err");
            }
            else
            {
                FUSBDevice.DevNum++;
            }
        }
    }
    USBDEBUG("Reconnect");
    USBReConnect(0);

    #if 1
    ret = USBWaitConnect(10000); //wait 2s
    if (ret == 0)
    {
        //..����ʧ��
        USBDEBUG("USB connect error!");
        SendMsg(MSG_USB_CONNECT_FAIL);
    }
    else
    {
        //..���ӳɹ���֪ͨ��ʾ
        USBDEBUG("Reconnect OK!");
        BatteryChargeInit();
        SendMsg(MSG_CHARGE_START);      //ʹ�ܳ��

        FREQ_EnterModule(FREQ_BLON); //�Ƿ��Ӱ�쵽USB
        SendMsg(MSG_SYS_RESUME);
    }
    #endif


    #ifdef USB_PHY_TEST
    ret = USBWaitConnect(10000);
    if (ret)
    {
        DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

        DEBUG("USB Test Start");

        PowerPath_Set(0);
        Charge_Enable(0);

        dev_regs->dctl=(dev_regs->dctl & (~(0x07<<4))) | (((0x04)%8)<<4);
        DelayMs(100);

        while(1)
        {
            if (Get_PlayKey_State())        //��Play���˳�
            {
                break;
            }
            DelayMs(100);
        }
        DEBUG("USB Test End");
    }
    #endif

    UsbIdleCounter = SysTickCounter;
    //ClearMsg(MSG_USB_DISCONNECT);

}

/*
--------------------------------------------------------------------------------
  Function name :   UINT32 USBControlService(void)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_USBCONTROL_CODE_
UINT32 USBControlService(void)
{
    int32 ret = 0;
    uint32 i;
    pFUN_USB_DEV  pDev = &FUSBDevTab[0];
    pFUNCTION_USB pFUSB = &FUSBDevice;

    //if (0 == CheckVbus())
    if ((FALSE == CheckMsg(MSG_VBUS_INSERT)) || (CheckMsg(MSG_SYS_FW_UPGRADE)))
    {
        //DEBUG("Vbus = 0");
        USBDEBUG("USB MODULE EXIT");
        LcdWakeUp();
        SendMsg(MSG_NEED_BACKLIGHT_ON);
        SendMsg(MSG_NEED_PAINT_ALL);
        SendMsg(MSG_USB_EXIT_FUSB);
        return 0;
    }

    if(GetMsg(MSG_USB_SUSPEND)/* || GetMsg(MSG_USB_DISCONNECT)*/)
    {
        if ((SysTickCounter - UsbIdleCounter) > 100 * 2)
        {
            //DEBUG("MSG_USB_SUSPEND");
            //BatteryChargeDeInit();
            //BL_Off();
            //LcdStandby();

            USBControlIdle();
            UsbIdleCounter = SysTickCounter;

            if (CheckMsg(MSG_POWER_DOWN))
            {
                SendMsg(MSG_USB_EXIT_FUSB); //�͵�ѹ���ѣ��˳� USB
            }
            return 0;
        }
    }

    if (GetMsg(MSG_USB_RESUMED))
    {
        //DEBUG("MSG_USB_RESUMED");
        if (CheckVbus())
        {
            DEBUG("MSG_USB_RESUMED");
            LcdWakeUp();
            SendMsg(MSG_NEED_BACKLIGHT_ON);
            SendMsg(MSG_NEED_PAINT_ALL);
            BatteryChargeInit();
        }
    }

    for(i=0; i<sizeof(FUSBDevTab)/sizeof(FUN_USB_DEV); i++, pDev++)
    {
        if (pFUSB->SelClassType & pDev->ClassType)
        {
            if (!pDev->FUThead)
                continue;

            ret = pDev->FUThead();
            if (ret < 0)
            {
                printf("FUThead err");
            }
        }
    }

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name :  void USBControlDeInit(void)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
extern UINT32 UsbAdapterProbeLdoBak;
_ATTR_USBCONTROL_CODE_
void USBControlDeInit(void)
{
    uint32 i;
    pFUN_USB_DEV pDev = &FUSBDevTab[0];
    pFUNCTION_USB pFUSB = &FUSBDevice;


    //DISABLE_USB_INT;
    DisableIntMaster();
    UsbIntDisalbe();
    EnableIntMaster();

    Grf_otgphy_suspend(1);

    ScuSoftResetCtr(USBPHY_SRST, 1);
    ScuSoftResetCtr(USBGLB_SRST, 1);
    ScuSoftResetCtr(USBOTG_SRST, 1);
    ScuClockGateCtr(CLK_USBPHY_GATE, 0);
    ScuClockGateCtr(HCLK_USBC_GATE, 0);

    IntUnregister(INT_ID_USBC);

    for(i=0; i<sizeof(FUSBDevTab)/sizeof(FUN_USB_DEV); i++, pDev++)
    {
        if (pFUSB->SelClassType & pDev->ClassType)
        {
            pDev->FUDeinit();
        }
    }

    FUSBDevice.DevNum = 0;

    AdcSleepExit();
    BLOffEnable();

    FREQ_Enable();
    FREQ_ExitModule(FREQ_USB);
}

#endif
/*
********************************************************************************
*
*                         End of USBControl.c
*
********************************************************************************
*/




