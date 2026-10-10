/*
********************************************************************************
*                   Copyright (c) 2009,chenfen
*                         All rights reserved.
*
* File Name��   UsbAdapterProbe.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             chenfen      2008-12-12          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_USB_ADAPTER

#include "SysInclude.h"

#include "driver/USB/host/USB_DWCHost.h"  /* HOST_DEV & host stack */
#include "driver/USB/USBConfig.h"
#include "freq_enums.h"            /* FREQ_USB */
#include "driver/CRU/Hw_cru.h"      /* CLK_USBPHY_GATE, USBPHY_SRST... */         /* USB_CLASS_* */
#include "fsinclude.h"

#ifdef _USB_

#ifdef _USB_HOST_
_ATTR_SYS_BSS_ HOST_DEV pHDev;


_ATTR_SYS_CODE_
int32 USBHostEnumDevice(void)
{
    int32 ret = 0;
    memset (&pHDev, 0 , sizeof(HOST_DEV));
    //��ö�ٺ���������Ȳ��,��ʼĬ�ϴ�0��ַ��д����
    pHDev.DevNum = 0;
    pHDev.toggle[0] = 0;
    ret = HostPortInit(&pHDev);
    if (ret < 0)
    {

    }

    ret = HostNewDev(&pHDev);
    if (ret < 0)
    {

    }
    #if 0
    printf ("\n---------------------------------------------\n");
    printf("Enum USB Device OK! \nUSB Device Class is:0x%x\n",
           pHDev.intf.IfDesc.bInterfaceClass);
    printf ("0x01:USB Audio Device\n");
    printf ("0x02:USB Commucation Device\n");
    printf ("0x03:USB HID Device\n");
    printf (".....\n");
    printf ("0x08:USB Massage Storage Device\n");
    printf ("\n---------------------------------------------\n");
    #endif
    switch (pHDev.intf.IfDesc.bInterfaceClass)
    {
        case USB_CLASS_AUDIO:
            break;
        case USB_CLASS_COMM:
            break;
        case USB_CLASS_HID:
            printf ("USB HID Device\n");
            break;
        case USB_CLASS_PHYSICAL:
            break;
        case USB_CLASS_STILL_IMAGE:
            break;
        case USB_CLASS_PRINTER:
            break;
        case USB_CLASS_MASS_STORAGE:
            ret = MscHost(&pHDev.intf);
            break;
        case USB_CLASS_HUB:

            break;
        default:
            break;
    }

    return ret;
}

#endif
_ATTR_SYS_BSS_ UINT8  UsbAdapterProbeConnectFlag;
_ATTR_SYS_BSS_ UINT8  UsbAdapterProbeStartFlag;
_ATTR_SYS_BSS_ UINT32 UsbAdapterProbeSystickCounter;
_ATTR_SYS_BSS_ UINT32 UsbAdapterProbeLdoBak;
_ATTR_SYS_BSS_ UINT8  UsbAdapterProbe300Cnt;
_ATTR_SYS_BSS_ UINT32 UsbAdapterProbeCount;


/*
--------------------------------------------------------------------------------
  Function name : usb_in_ep_intr
  Author        : chenfen
  Description   :

  Input         :
  Return        : TRUE

  History:     <author>         <time>         <version>
             chenfen     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void usb_in_ep_intr_hook(void)
{
    uint32 ep_intr;
    uint32 diepint;
    uint32 msk;
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    /* Read in the device interrupt bits */
    ep_intr = (dev_regs->daint & dev_regs->daintmsk) & 0xFFFF;
    if ( ep_intr & 0x01)
    {
        ep_intr = 0;     //�˵�0
    }
    else if ( ep_intr & 0x02)
    {
        ep_intr = 1;       //�˵�1
    }
    else
    {
        return;
    }

    /* Service the Device IN interrupts for each endpoint */

    msk = dev_regs->diepmsk | ((dev_regs->dtknqr4_fifoemptymsk & 0x01)<<7);   //<<7����Ϊmsk�Ǳ���?
    diepint = dev_regs->in_ep[ep_intr].diepint & msk;

    /* Transfer complete */
    if (diepint & 0x01 )
    {
        /* Disable the NP Tx FIFO Empty Interrrupt */
        dev_regs->dtknqr4_fifoemptymsk = 0;

        /* Clear the bit in DIEPINTn for this interrupt */
        dev_regs->in_ep[ep_intr].diepint = 0x01;
    }
    /* Endpoint disable  */
    if (diepint & 0x02 )
    {
        /* Clear the bit in DIEPINTn for this interrupt */
        dev_regs->in_ep[ep_intr].diepint = 0x02;

    }
    /* AHB Error */
    if ( diepint & 0x04 )
    {
        /* Clear the bit in DIEPINTn for this interrupt */
        dev_regs->in_ep[ep_intr].diepint = 0x04;
    }
    /* TimeOUT Handshake (non-ISOC IN EPs) */
    if ( diepint & 0x08 )
    {
        dev_regs->in_ep[ep_intr].diepint = 0x08;
    }
    /** IN Token received with TxF Empty */
    if ( diepint & 0x20 )
    {
        dev_regs->in_ep[ep_intr].diepint = 0x20;
    }
    /** IN EP Tx FIFO Empty Intr */
    if ( diepint & 0x80 )
    {
        dev_regs->in_ep[ep_intr].diepint = 0x80;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : UsbAdpterProbeISR
  Author        : chenfen
  Description   :
                  the device interrupts.  Many conditions can cause a
   * device interrupt. When an interrupt occurs, the device interrupt
   * service routine determines the cause of the interrupt and
   * dispatches handling to the appropriate function. These interrupt
   * handling functions are described below.
     *
   * All interrupt registers are processed from LSB to MSB.
  Input         :
  Return        : TRUE

  History:     <author>         <time>         <version>
             chenfen     2008/012/10         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void UsbAdpterProbeISR(void)
{
    uint32 intr_status;
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;

    intr_status = otg_core->Core.gintsts & otg_core->Core.gintmsk;

    if (!intr_status)
    {
        return ;
    }

    if(intr_status & (1<<10))    //Early Suspend
    {
        //UDEBUG("e suspend\n");
        otg_core->Core.gintsts = 1<<10;
    }
    if(intr_status & (1<<11))    //USB Suspend
    {
        //UDEBUG("suspend\n");
        otg_core->Core.gintsts = 1<<11;
        if (UsbAdapterProbeConnectFlag)
        {
            USBDEBUG("Reset->Suspend");
            UsbAdapterProbeConnectFlag = 0;
        }
        else
        {
            UsbAdapterProbeConnectFlag = 2;
            USBDEBUG("Suspend");
        }
    }
    if(intr_status & (1<<12))  //USB Reset
    {
        //UDEBUG("reset\n");
        USBDEBUG("Reset");
        UsbAdapterProbeConnectFlag = 1;


        otg_core->Device.dcfg &= ~0x07f0;
        otg_core->Device.dctl &= ~0x01;
        otg_core->Core.gintsts = 1<<12;

		//DISABLE_USB_INT;
		UsbIntDisalbe();
        //otg_core->Device.dctl |= 0x02;         //soft disconnect
    }

    if(intr_status & (1<<13))  //Enumeration Done
    {
        //otg_core->core_regs.gintsts = 1<<13;
        //UDEBUG("enum\n");
    }

    if(intr_status & (1<<30))  //USB VBUS�ж�  this interrupt is asserted when the utmiotg_bvalid signal goes high.
    {
        //UDEBUG("vbus\n");
        otg_core->Core.gintsts = 1<<30;
    }

    if(intr_status & (1ul<<31))    //resume
    {
        USBDEBUG("resume");
        UsbAdapterProbeConnectFlag = 3;
        otg_core->Core.gintsts = 1ul<<31;
    }

    if(intr_status & (1<<18))       //IN�ж�
    {
        usb_in_ep_intr_hook();
    }

    if(intr_status & ((1<<22)|(1<<6)|(1<<7)|(1<<17)))
    {
        otg_core->Core.gintsts = intr_status & ((1<<22)|(1<<6)|(1<<7)|(1<<17));
    }
}

_ATTR_SYS_CODE_
void UsbAdpterProbeStart(void)
{
    USB_OTG_REG *  otg_core = (USB_OTG_REG * )USB_REG_BASE;
    int count = 0;

    if (UsbAdapterProbeStartFlag == 0)
    {
        //variable init
        UsbAdapterProbeConnectFlag = 0;
        UsbAdapterProbeStartFlag   = 1;
        UsbAdapterProbe300Cnt = 0;

        #ifdef _FILE_DEBUG_
        SysDebugHookDeInit();
        SysDebugHookInitMem();
        SysDebugHookInit();
        #endif

        //CPU init
        {
            //DISABLE_USB_INT;
            FREQ_EnterModule(FREQ_USB); //sch for PMU_USB = PMU_MAX, so can disale
            FREQ_Disable(); //pmu disable for usb otg stability

            ScuClockGateCtr(CLK_USBPHY_GATE, 1);
            ScuClockGateCtr(HCLK_USBC_GATE, 1);
            ScuSoftResetCtr(USBPHY_SRST, 0);
            ScuSoftResetCtr(USBGLB_SRST, 0);
            ScuSoftResetCtr(USBOTG_SRST, 0);
            DelayUs(5);
            Grf_otgphy_suspend(0);
            DelayUs(200);

            //interrupt init
            IntRegister(INT_ID_USBC, (void*)UsbAdpterProbeISR);
            //ENABLE_USB_INT;
        }
        USBDEBUG("Detect Start");

        DelayMs(10);

        #ifdef _USB_
        Grf_Otgphy_Driver_Confige();
        USBConnect(0);
        #endif

        UsbAdapterProbeSystickCounter = GetSysTick();        //start timer
    }
}

_ATTR_SYS_CODE_
void UsbAdpterProbeStop(void)
{
    DEVICE_REG *dev_regs = (DEVICE_REG *)USB_DEV_BASE;

    if (UsbAdapterProbeStartFlag == 1)
    {
        //Variable deinit
        UsbAdapterProbeStartFlag = 0;

        //Usb Stop
        //DISABLE_USB_INT;
        DisableIntMaster();
        UsbIntDisalbe();
        EnableIntMaster();

        dev_regs->dctl |= 0x02;       //soft disconnect

        //CPU DeInit
        {
            Grf_otgphy_suspend(1);

            IntUnregister(INT_ID_USBC);

            ScuSoftResetCtr(USBPHY_SRST, 1);
            ScuSoftResetCtr(USBGLB_SRST, 1);
            ScuSoftResetCtr(USBOTG_SRST, 1);
            ScuClockGateCtr(CLK_USBPHY_GATE, 0);
            ScuClockGateCtr(HCLK_USBC_GATE, 0);

            FREQ_Enable(); //UsbAdpterProbeStart -> PmuDisable()
            FREQ_ExitModule(FREQ_USB);
        }
    }
}

_ATTR_SYS_CODE_
int32 UsbAdpterProbe(void)
{
    uint32 SystickTmp;

    if (UsbAdapterProbeStartFlag == 1)
    {
        if (UsbAdapterProbeConnectFlag == 1)
        {
            //UsbAdpterProbeStop();
            //DEBUG("OK");
            UsbAdapterProbeStartFlag = 0;
            USBDEBUG("Detect Ok");
            return 1;
        }
        else if(UsbAdapterProbeConnectFlag == 2)
        {
            //usb suspend
            //if ((GetSysTick() - UsbAdapterProbeCount)>150) //500ms delay
            {
                BatteryChargeDeInit();
                BL_Off();
                LCD_ClrSrc();

                //Lcd_BuferTranfer(0,0,LCD_MAX_XSIZE,LCD_MAX_YSIZE, NULL);
                IsBackLightOn = TRUE;
                if (FALSE == CheckMsg(MSG_LCD_STANDBY_DISABLE))
                {
                    LcdStandby();
                    AdcSleepEnter();
                }

                while(1)
                {
                    #ifdef _WATCH_DOG_
                    WatchDogReload();
                    #endif

                    if (CheckVbus() == 0)
                    {
                        break;
                    }

                    if ((UsbAdapterProbeConnectFlag == 1)
                        ||(UsbAdapterProbeConnectFlag == 3))
                    {
                        break;
                    }
                    DelayMs(5);
                }
                return 0;
            }
        }
        else
        {
            //probe timeout, it means charge
            SystickTmp = GetSysTick();
            if ((SystickTmp - UsbAdapterProbeSystickCounter) > 300)    //1000ms��ʱ
            {
                USBDEBUG("Detect TimeOut");

                #if 1
                UsbAdapterProbeSystickCounter = SystickTmp;
                #ifdef _FILE_DEBUG_
                SysDebugHookDeInit();
                SysDebugHookInitMem();
                SysDebugHookInit();
                #endif
                #else
                UsbAdpterProbeStop();
                #endif
                if(UsbAdapterProbe300Cnt == 0)
                {
                    UsbAdapterProbe300Cnt = 1;
                    return 2;
                }
                else
                {
                    return 0;
                }
            }
        }
    }
    return 0;
}

#endif

/*
********************************************************************************
*
*                         End of UsbAdapterProbe.c
*
********************************************************************************
*/

