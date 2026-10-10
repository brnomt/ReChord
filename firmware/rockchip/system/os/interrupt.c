/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   interrupt.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-7-21          1.0
*    desc:
********************************************************************************
*/
#define _IN_INTTERUPT_

#include "OsInclude.h"
#include "DriverInclude.h"

extern void PowerOn_Reset(void);
extern UINT32 Image$$AP_MAIN_STACK$$ZI$$Limit;


#define ENABLE_USB_INT          *((volatile unsigned long*)(0xE000E100)) = 0x00000020
#define DISABLE_USB_INT         *((volatile unsigned long*)(0xE000E180)) = 0x00000020
 __align(4) uint32 MainStack1[4096];
/*
--------------------------------------------------------------------------------
  Function name :
  Author        : ZHengYongzhi
  Description   : CortexM3 interrupt vector table

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VECTTAB_
ExecFunPtr exceptions_table[NUM_INTERRUPTS] =
{

    /* Core Fixed interrupts start here...*/

    (ExecFunPtr)&Image$$AP_MAIN_STACK$$ZI$$Limit,
    (ExecFunPtr)PowerOn_Reset,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,

    //RKNano Process Intterupt
    IntDefaultHandler,                                          //00 int_dma
    IntDefaultHandler,                                          //01 uart
    IntDefaultHandler,                                          //02 sd mmc
    IntDefaultHandler,                                          //03 pwm1
    IntDefaultHandler,                                          //04 pwm2
    IntDefaultHandler,                                          //05 imdct36
    IntDefaultHandler,                                          //06 synthesize
    IntDefaultHandler,                                          //07 usb
    IntDefaultHandler,                                          //08 i2c
    IntDefaultHandler,                                          //09 i2s
    IntDefaultHandler,                                          //10 gpio
    IntDefaultHandler,                                          //11 spi
    IntDefaultHandler,                                          //12 pwm0
    IntDefaultHandler,                                          //13 timer
    IntDefaultHandler,                                          //14 sar-adc
    IntDefaultHandler,                                          //15 reserved
    IntDefaultHandler,                                          //16 reserved
    IntDefaultHandler,                                          //17 reserved
    IntDefaultHandler,                                          //18 rtc
    IntDefaultHandler,                                          //19 reserved
    IntDefaultHandler,                                          //20 ool_bat_snsen
    IntDefaultHandler,                                          //21 reserved
    IntDefaultHandler,                                          //22 ool_pause
    IntDefaultHandler,                                          //23 ool_PlayOn
    IntDefaultHandler,                                          //24 pwr_5v_ready     Adapter supply detected
    IntDefaultHandler,                                          //25 pwr_uvlo_vdd      Vbat under 3.2V indicator
    IntDefaultHandler,                                          //26 pwr_uvp              DC-DC under voltage alarm
    IntDefaultHandler,                                          //27 pwr_ovp              DC-DC over voltage alarm
    IntDefaultHandler,                                          //28 pwr_ot                DC-DC over temperature alarm
    IntDefaultHandler,                                          //29 pwr_oc                DC-DC over current alarm
    IntDefaultHandler,                                          //30 pwr_charge         Charge state changing indicator
    IntDefaultHandler,                                          //31 reserved
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler
};
//Interrupts Total Nums
/*
--------------------------------------------------------------------------------
  Function name :  IntDefaultHandler(void)
  Author        :  ZHengYongzhi
  Description   :  default interrupt service program.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int SysDebugHook(char ch);
int USBDebugHook(char ch);

_ATTR_INTRRUPT_CODE_
__attribute__((naked)) uint32 __MSR_XPSR(void)
{
    __asm__ volatile("mrs r0, xpsr; bx lr");
}

_ATTR_INTRRUPT_CODE_
__attribute__((interrupt("IRQ"), naked)) void IntDefaultHandler(void)
{
    __asm__ volatile("b __CPU_IntDefaultHandler");
}

/*
--------------------------------------------------------------------------------
  Function name :  IntMasterEnable(void)
  Author        :  ZHengYongzhi
  Description   :  Enable processor interrupts.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_//_ATTR_INTRRUPT_CODE_
void IntMasterEnable(void)
{
    __SETPRIMASK();
}

/*
--------------------------------------------------------------------------------
  Function name :  IntMasterEnable(void)
  Author        :  ZHengYongzhi
  Description   :  Disable processor interrupts.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

void IntMasterDisable(void)
{
    __RESETPRIMASK();
}

/*
--------------------------------------------------------------------------------
  Function name :  IntMasterEnable(void)
  Author        :  ZHengYongzhi
  Description   :  Enable processor interrupts.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_//_ATTR_INTRRUPT_CODE_
void FaultMasterEnable(void)
{
    __RESETFAULTMASK();
}

/*
--------------------------------------------------------------------------------
  Function name :  IntMasterEnable(void)
  Author        :  ZHengYongzhi
  Description   :  Disable processor interrupts.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void FaultMasterDisable(void)
{
    __SETFAULTMASK();
}
//#endif

/*
--------------------------------------------------------------------------------
  Function name : IntRegister(UINT32 ulInterrupt, void (*pfnHandler)(void))
  Author        : ZHengYongzhi
  Description   : interrupts register

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_INTRRUPT_CODE_
void IntRegister(UINT32 ulInterrupt, ExecFunPtr(*pfnHandler)(void))
{
    UINT32 ulIdx;

    // Check the arguments.
    ASSERT(ulInterrupt < NUM_INTERRUPTS);

    // Make sure that the RAM vector table is correctly aligned.
    ASSERT(((UINT32)exceptions_table & 0x000003ff) == 0);

    // See if the RAM vector table has been initialized.
    if (nvic->VectorTableOffset != (UINT32)exceptions_table)
    {
        // Initiale the RAM vector table.
        exceptions_table[0] = (ExecFunPtr)&Image$$AP_MAIN_STACK$$ZI$$Limit;
        exceptions_table[1] = (ExecFunPtr)PowerOn_Reset;
        for (ulIdx = 2; ulIdx < NUM_INTERRUPTS; ulIdx++)
        {
            exceptions_table[ulIdx] = (ExecFunPtr)IntDefaultHandler;
        }

        // Point NVIC at the RAM vector table.
        nvic->VectorTableOffset = (UINT32)exceptions_table;
    }

    // Save the interrupt handler.
    exceptions_table[ulInterrupt] = (ExecFunPtr)pfnHandler;
}

/*
--------------------------------------------------------------------------------
  Function name : IntUnregister(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : interrupts Unregister

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_INTRRUPT_CODE_
void IntUnregister(UINT32 ulInterrupt)
{
    //
    // Check the arguments.
    //
    ASSERT(ulInterrupt < NUM_INTERRUPTS);

    //
    // Reset the interrupt handler.
    //
    exceptions_table[ulInterrupt] = (ExecFunPtr)IntDefaultHandler;
}

/*
--------------------------------------------------------------------------------
  Function name : void IntPriorityGroupingSet(UINT32 ulBits)
  Author        : ZHengYongzhi
  Description   : priority is grouping setting
                  This function specifies the split between preemptable priority
                  levels and subpriority levels in the interrupt priority speci-
                  fication.  The range of the grouping values are dependent upon
                  the hardware implementation.

  Input         : ulBits specifies the number of bits of preemptable priority.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_//_ATTR_INTRRUPT_CODE_
void IntPriorityGroupingSet(UINT32 ulBits)
{
    //
    // Set the priority grouping.
    //
    nvic->APIntRst = NVIC_APINTRST_VECTKEY | ((7 - ulBits) << 8);
}

/*
--------------------------------------------------------------------------------
  Function name : void IntPriorityGroupingSet(UINT32 ulBits)
  Author        : ZHengYongzhi
  Description   : get priority grouping.
                  This function returns the split between preemptable priority
                  levels and subpriority levels in the interrupt priority spe-
                  cification.
  Input         :
  Return        : The number of bits of preemptable priority.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 IntPriorityGroupingGet(void)
{
    UINT32 ulValue;

    //
    // Read the priority grouping.
    //
    ulValue = nvic->APIntRst & NVIC_APINTRST_PRIGROUP_MASK;

    //
    // Return the number of priority bits.
    //
    return(7 - (ulValue >> 8));
}

/*
--------------------------------------------------------------------------------
  Function name : void IntPrioritySet(UINT32 ulInterrupt, unsigned char ucPriority)
  Author        : ZHengYongzhi
  Description   : Sets the priority of an interrupt.
                  This function is used to set the priority of an interrupt.
  Input         : ulInterrupt -- INT_ID;   ucPriority -- Priority Num.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void IntPrioritySet(UINT32 ulInterrupt, UINT32 ucPriority)
{
    UINT32 ulBitOffset;
    UINT32 *pRegister;

    //
    // Set the interrupt priority.
    //
    ulBitOffset   = (ulInterrupt & 0x03) * 8;

    if (ulInterrupt < 16 )
    {
        pRegister = (UINT32*)((uint32)nvic->SystemPriority - 4);
    }
    else
    {
        pRegister = (UINT32 *)nvic->Irq.Priority;
        ulInterrupt = ulInterrupt - 16;
    }
    pRegister = pRegister + (ulInterrupt >> 2);

    *pRegister = *pRegister & (~(0xFF << ulBitOffset)) | (ucPriority << ulBitOffset);
}

/*
--------------------------------------------------------------------------------
  Function name :  UINT32 IntPriorityGet(UINT32 ulInterrupt)
  Author        :  ZHengYongzhi
  Description   :  Gets the priority of an interrupt.
                   This function gets the priority of an interrupt.
  Input         :  ulInterrupt -- INT_ID
  Return        :  Returns the interrupt priority.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 IntPriorityGet(UINT32 ulInterrupt)
{
    UINT32 ulBitOffset;
    UINT32 *pRegister;

    //
    // Return the interrupt priority.
    //
    ulBitOffset   = (ulInterrupt & 0x03) * 8;

    if (ulInterrupt < 16 )
    {
        pRegister = (UINT32 *)(nvic->SystemPriority - 4);
    }
    else
    {
        pRegister = (UINT32 *)nvic->Irq.Priority;
    }
    pRegister = pRegister + (ulInterrupt & 0x03);

    return((*pRegister >> ulBitOffset) & 0xFF);
}

/*
--------------------------------------------------------------------------------
  Function name : void IntEnable(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : Enables an interrupt.
                  The specified interrupt is enabled in the interrupt controller.
  Input         : ulInterrupt -- INT_ID
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void IntEnable(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt to enable.
    //
    if (ulInterrupt == FAULT_ID4_MPU)
    {
        //
        // Enable the MemManage interrupt.
        //
        nvic->SystemHandlerCtrlAndState |= NVIC_SYSHANDCTRL_MEMFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID5_BUS)
    {
        //
        // Enable the bus fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState |= NVIC_SYSHANDCTRL_BUSFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID6_USAGE)
    {
        //
        // Enable the usage fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState |= NVIC_SYSHANDCTRL_USGFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Enable the System Tick interrupt.
        //
        nvic->SysTick.Ctrl |= NVIC_SYSTICKCTRL_TICKINT;
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Enable the general interrupt.
        //
        nvic->Irq.Enable[(ulInterrupt - 16) / 32] = 1 << (ulInterrupt - 16) % 32;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void IntDisable(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : Disables an interrupt.
                  The specified interrupt is Disabled in the interrupt controller.
  Input         : ulInterrupt -- INT_ID
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void IntDisable(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt to Disable.
    //
    if (ulInterrupt == FAULT_ID4_MPU)
    {
        //
        // Disable the MemManage interrupt.
        //
        nvic->SystemHandlerCtrlAndState &= ~NVIC_SYSHANDCTRL_MEMFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID5_BUS)
    {
        //
        // Disable the bus fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState &= ~NVIC_SYSHANDCTRL_BUSFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID6_USAGE)
    {
        //
        // Disable the usage fault interrupt.
        //
        nvic->SystemHandlerCtrlAndState &= ~NVIC_SYSHANDCTRL_USGFAULTENA;
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Disable the System Tick interrupt.
        //
        nvic->SysTick.Ctrl &= ~NVIC_SYSTICKCTRL_TICKINT;
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Disable the general interrupt.
        //
        nvic->Irq.Disable[(ulInterrupt - 16) / 32] = 1 << (ulInterrupt - 16) % 32;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void IntPendingSet(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : Set pending bit of the interrupt.

  Input         :
  Return        : Pending IRQ Channel Identifier.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void IntPendingSet(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt pend to Set.
    //
    if (ulInterrupt == FAULT_ID2_NMI)
    {
        //
        // Set the NMI interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_NMIPENDSET;
    }
    else if (ulInterrupt == FAULT_ID14_PENDSV)
    {
        //
        // Set the PendSV interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSVSET;
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Set the System Tick interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSTSET;
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Set the general interrupt pend.
        //
        nvic->Irq.SetPend[(ulInterrupt - 16) / 32] = 1 << (ulInterrupt - 16) % 32;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void IntPendClear(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : Clear pending bit of the irq.

  Input         :
  Return        : Pending IRQ Channel Identifier.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void IntPendingClear(UINT32 ulInterrupt)
{
    //
    // Determine the interrupt pend to Set.
    //
    if (ulInterrupt == FAULT_ID14_PENDSV)
    {
        //
        // Set the PendSV interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSVCLR;
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Set the System Tick interrupt pend.
        //
        nvic->INTcontrolState |= NVIC_INTCTRLSTA_PENDSTCLR;
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Set the general interrupt pend.
        //
        nvic->Irq.ClearPend[(ulInterrupt - 16) / 32] = 1 << (ulInterrupt - 16) % 32;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : BOOL IntISRPendingCheck(void)
  Author        : ZHengYongzhi
  Description   : Check interrupt pending or not.

  Input         :
  Return        : pending or not.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
BOOL IntISRPendingCheck(void)
{
    //
    // Check the arguments.
    //
    return ((BOOL)(nvic->INTcontrolState & NVIC_INTCTRLSTA_ISRPENDING));
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 IntVectPendingGet(void)
  Author        : ZHengYongzhi
  Description   : Get interrupt number of the highest priority pending ISR.

  Input         :
  Return        : Return interrupt number of the highest priority pending ISR.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 IntVectPendingGet(void)
{
    //
    // Check the arguments.
    //
    return ((nvic->INTcontrolState & NVIC_INTCTRLSTA_ISRPENDING) >> 12);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 IntVectActiveGet(void)
  Author        : ZHengYongzhi
  Description   : Get interrupt number of the currently running ISR.

  Input         :
  Return        : Return interrupt number of the currently running ISR.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 IntVectActiveGet(void)
{
    //
    // Check the arguments.
    //
    return (nvic->INTcontrolState & NVIC_INTCTRLSTA_VECTACTIVE_MASK);
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 IsrDisable(UINT32 ulInterrupt)
  Author        : ZHengYongzhi
  Description   : close interrupt temporarily,go to use with IsrEnable.

  Input         :
  Return        : current interrupt status.

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
uint32 IsrDisable(UINT32 ulInterrupt)
{
    uint32 IsEnable = 0xff0055aa;

    //
    // Determine the interrupt to enable.
    //
    if (ulInterrupt == FAULT_ID4_MPU)
    {
        //
        // Is the MemManage interrupt Enabled.
        //
        IsEnable = (nvic->SystemHandlerCtrlAndState & NVIC_SYSHANDCTRL_MEMFAULTENA);
    }
    else if (ulInterrupt == FAULT_ID5_BUS)
    {
        //
        // Is the bus fault interrupt Enabled.
        //
        IsEnable = (nvic->SystemHandlerCtrlAndState & NVIC_SYSHANDCTRL_BUSFAULTENA);
    }
    else if (ulInterrupt == FAULT_ID6_USAGE)
    {
        //
        // Is the usage fault interrupt Enabled.
        //
        IsEnable = (nvic->SystemHandlerCtrlAndState & NVIC_SYSHANDCTRL_USGFAULTENA);
    }
    else if (ulInterrupt == FAULT_ID15_SYSTICK)
    {
        //
        // Is the System Tick interrupt Enabled.
        //
        IsEnable = (nvic->SysTick.Ctrl & NVIC_SYSTICKCTRL_TICKINT);
    }
    else if (ulInterrupt >= 16)
    {
        //
        // Is the general interrupt Enabled.
        //
        IsEnable = (nvic->Irq.Enable[(ulInterrupt - 16) / 32] & (1 << (ulInterrupt - 16) % 32));
    }

    IntDisable(ulInterrupt);

    return(IsEnable);

}

/*
--------------------------------------------------------------------------------
  Function name : void IsrEnable(UINT32 ulInterrupt, uint32 IsEnable)
  Author        : ZHengYongzhi
  Description   : open interrupt and it act in concert with IsrDisable.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void IsrEnable(UINT32 ulInterrupt, uint32 IsEnable)
{
    if (IsEnable)
    {
        IntEnable(ulInterrupt);
    }
}
//#endif

/*
--------------------------------------------------------------------------------
  Function name : void IsrEnableCheck(void)
  Author        : ZHengYongzhi
  Description   : Get Intrrupt Enable Set value.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2016/06/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void IsrEnableCheck(void)
{
    uint32 irq0, irq1;

    irq0 = nvic->Irq.Enable[0];
    irq1 = nvic->Irq.Enable[1];
    printf("\n     irq0 = 0x%08x, irq1 = 0x%08x", irq0, irq1);
}

/*
--------------------------------------------------------------------------------
  Function name : void IsrEnableCheck(void)
  Author        : ZHengYongzhi
  Description   : Get Intrrupt Enable Set value.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2016/06/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_INTRRUPT_CODE_
uint32 IntMasterControlInit(void)
{
    IntMasterCounter = 0;
}

/*
--------------------------------------------------------------------------------
  Function name :
  Author        : ZHengYongzhi
  Description   : Enable and Disable global interrupt, for critical protection

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2016/06/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_INTRRUPT_CODE_
uint32 EnableIntMaster(void)
{
    if (IntMasterCounter > 0)
    {
        IntMasterCounter--;
        if (IntMasterCounter == 0)
        {
            IntMasterEnable();
        }
    }
    else
    {
        #ifdef _LOG_DEBUG_
        DEBUG("error!!!!");
        while(1);
        #endif
    }
}


_ATTR_INTRRUPT_CODE_
uint32 DisableIntMaster(void)
{
    IntMasterDisable();
    IntMasterCounter++;
}

/*
--------------------------------------------------------------------------------
  Function name :
  Author        : ZHengYongzhi
  Description   : Enable and Disable USB int, for USB critical protection

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2016/06/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_INTRRUPT_CODE_
uint32 UsbHostIntEnable(void)
{
    ENABLE_USB_INT;
}

_ATTR_INTRRUPT_CODE_
uint32 UsbHostIntDisable(void)
{
    DISABLE_USB_INT;
}

_ATTR_INTRRUPT_CODE_
uint32 UsbIntControlInit(void)
{
    DisableIntMaster();

    UsbIntCounter = 0;
    UsbIntDisalbe();

    EnableIntMaster();
}

_ATTR_INTRRUPT_CODE_
uint32 UsbIntEnable(void)
{
    DisableIntMaster();

    if (UsbIntCounter > 0)
    {
        UsbIntCounter--;
        if (UsbIntCounter == 0)
        {
            ENABLE_USB_INT;
        }
    }
    else
    {
        #ifdef _LOG_DEBUG_
        DEBUG("error!!!!");
        while(1);
        #endif
    }

    EnableIntMaster();
}


_ATTR_INTRRUPT_CODE_
uint32 UsbIntDisalbe(void)
{
    DisableIntMaster();

    DISABLE_USB_INT;
    UsbIntCounter++;

    EnableIntMaster();
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 UserIsrDisable(void)
  Author        : ZhengYongzhi
  Description   : PendSV is soft interrupt, it cannot disable and enable,to use semaphore MSG_USER_ISR_ENABLE
                  to enable PendSV,it act in concert with UserIsrEnable,

  Input         :
  Return        : NULL

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_INTRRUPT_DATA_   uint32 UserIsrFlagMb1 = 0;
_ATTR_INTRRUPT_DATA_   uint32 UserIsrFlagMb2 = 0;
_ATTR_INTRRUPT_DATA_   uint32 UserIsrDisableFlag = 0;

_ATTR_INTRRUPT_CODE_
uint32 UserIsrDisable(void)
{
    DisableIntMaster();
    if (UserIsrDisableCount == 0)
    {
        if (UserIsrDisableFlag == 0)
        {
            UserIsrFlagMb2 = IsrDisable(INT_ID_MAILBOX2);
            UserIsrFlagMb1 = IsrDisable(INT_ID_MAILBOX1);
        }

        UserIsrDisableCount++;
    }
    else
    {
        if (UserIsrDisableFlag == 0)
        {
            IntDisable(INT_ID_MAILBOX2);
            IntDisable(INT_ID_MAILBOX1);
        }

        UserIsrDisableCount++;
        if(UserIsrDisableCount > 250)
        {
            #ifdef _LOG_DEBUG_
            DEBUG("error: UserIsrDisableCount = %d",UserIsrDisableCount);
            while(1);
            #endif
        }
    }

    EnableIntMaster();
    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void UserIsrEnable(uint32 data)
  Author        : ZhengYongzhi
  Description   : PendSV is soft interrupt, it cannot disable and enable,use semaphore MSG_USER_ISR_ENABLE
                  to enable PendSV,it act in concert with UserIsrEnable,it shall judge the status of FAULT_ID14_PENDSV,
                  it must to initial both MSG_USER_ISR_ENABLE and MSG_USER_ISR_STATUES two semaphores before to use
                  these two functions.

  Input         :
  Return        : NULL

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_INTRRUPT_CODE_
void UserIsrEnable(void)
{
    DisableIntMaster();
    if (UserIsrDisableCount > 0)
    {
        UserIsrDisableCount--;

        if (UserIsrDisableCount == 0)
        {
            if (UserIsrDisableFlag == 0)
            {
                IsrEnable(INT_ID_MAILBOX2, UserIsrFlagMb2);
                IsrEnable(INT_ID_MAILBOX1, UserIsrFlagMb1);
            }
        }

        if ((UserIsrDisableCount == 0) && (UserIsrRequestCount > 0))
        {
            //UserIsrRequestCount --;
            UserIsrRequestCount = 0;
            IntPendingSet(FAULT_ID14_PENDSV);
        }
    }
    else
    {
        #ifdef _LOG_DEBUG_
        DEBUG("error: UserIsrDisableCount = %d", UserIsrDisableCount);
        while(1);
        #endif
    }
    EnableIntMaster();
}

_ATTR_INTRRUPT_CODE_
void UserIsrRequest(void)
{
    if (UserIsrDisableCount == 0)
    {
        IntPendingSet(FAULT_ID14_PENDSV);
    }
    else
    {
        //UserIsrRequestCount++;
        DisableIntMaster();
        UserIsrRequestCount = 1;
        EnableIntMaster();
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void UserIsrEnable(uint32 data)
  Author        : ZhengYongzhi
  Description   : PendSV作为软中断，不能进行Disable和Enable，使用信号量MSG_USER_ISR_ENABLE
                  进行PendSV的使能标志，和UserIsrDisable配合使用，打开时需要判断FAULT_ID14_PENDSV
                  的状态。使用这两个函数之前必须对信号量MSG_USER_ISR_ENABLE和MSG_USER_ISR_STATUES
                  进行初始化
  Input         :
  Return        : 无

  History:     <author>         <time>         <version>
             ZhengYongzhi      2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


uint32 IrqPriorityTab[NUM_INTERRUPTS] =
{
    //  group        SubPriority                                 //Cortex M3 System Intterupt
    -3,                                 //SP Point                FAULT_ID0_REV,
    -3,                                 //Reset                   FAULT_ID1_REV,
    -2,                                 //NMI fault               FAULT_ID2_NMI,
    -1,                                 //Hard fault              FAULT_ID3_HARD,
    ((0x00 << 6) | (0x00 << 3)),        //MPU fault               FAULT_ID4_MPU,
    ((0x00 << 6) | (0x00 << 3)),        //Bus fault               FAULT_ID5_BUS,
    ((0x00 << 6) | (0x00 << 3)),        //Usage fault             FAULT_ID6_USAGE,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID7_REV,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID8_REV,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID9_REV,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID10_REV,
    ((0xFF << 6) | (0xFF << 3)),        //SVCall                  FAULT_ID11_SVCALL,
    ((0xFF << 6) | (0xFF << 3)),        //Debug monitor           FAULT_ID12_DEBUG,
    ((0xFF << 6) | (0xFF << 3)),        //                        FAULT_ID13_REV,
    ((0x03 << 6) | (0x00 << 3)),        //PendSV                  FAULT_ID14_PENDSV,
    ((0x02 << 6) | (0x01 << 3)),        //System Tick             FAULT_ID15_SYSTICK,

    //RKNano Process Int
    ((0x02 << 6) | (0x02 << 3)),            //00 sfc                  INT_ID16_SFC,
    ((0x02 << 6) | (0x00 << 3)),            //01 synth                INT_ID17_SYNTH,
    ((0x02 << 6) | (0x07 << 3)),            //02 ebc                  INT_ID18_EBC,
    ((0x01 << 6) | (0x00 << 3)),            //03 emmc                 INT_ID19_EMMC,
    ((0x01 << 6) | (0x00 << 3)),            //04 sdmmc                INT_ID20_SDMMC,
    ((0x01 << 6) | (0x00 << 3)),            //05 usbc                 INT_ID21_USBC,
    ((0x00 << 6) | (0x00 << 3)),            //06 dma                  INT_ID22_DMA,
    ((0x01 << 6) | (0x01 << 3)),            //07 imdct                INT_ID23_IMDCT,
    ((0x00 << 6) | (0x00 << 3)),            //08 wdt                  INT_ID24_WDT,
    ((0x03 << 6) | (0x00 << 3)),            //09 mailbox 0             INT_ID25_MAILBOX0,
    ((0x03 << 6) | (0x00 << 3)),            //10 mailbox 1            INT_ID26_MAILBOX1,
    ((0x02 << 6) | (0x00 << 3)),            //11 mailbox 2             INT_ID27_MAILBOX2,
    ((0x03 << 6) | (0x00 << 3)),            //12 mailbox 3             INT_ID28_MAILBOX3,
    ((0x02 << 6) | (0x04 << 3)),            //13 pwm4                 INT_ID29_PWM4,
    ((0x02 << 6) | (0x05 << 3)),            //14 pwm3                 INT_ID30_PWM3,
    ((0x02 << 6) | (0x06 << 3)),            //15 pwm2                 INT_ID31_PWM2,
    ((0x02 << 6) | (0x07 << 3)),            //16 pwm1                 INT_ID32_PWM1,
    ((0x01 << 6) | (0x02 << 3)),            //17 pwm0                 INT_ID33_PWM0,
    ((0xFF << 6) | (0xFF << 3)),            //18 timer 1               INT_ID34_TIMER1,
    ((0xFF << 6) | (0xFF << 3)),            //19 timer 0               INT_ID35_TIMER0,
    ((0x03 << 6) | (0x00 << 3)),            //20 sradc                INT_ID36_SRADC,
    ((0x01 << 6) | (0x07 << 3)),            //21 uart5                INT_ID37_UART5,
    ((0x02 << 6) | (0xFF << 3)),            //22 uart4                INT_ID38_UART4,
    ((0x02 << 6) | (0x07 << 3)),            //23 uart3                INT_ID39_UART3,
    ((0x03 << 6) | (0x00 << 3)),            //24 uart2                INT_ID40_UART2,
    ((0x00 << 6) | (0x07 << 3)),            //25 uart1                INT_ID41_UART1,
    ((0x02 << 6) | (0x07 << 3)),            //26 uart0                INT_ID42_UART0,
    ((0x02 << 6) | (0x07 << 3)),            //27 spi 1                INT_ID43_SP1,
    ((0x02 << 6) | (0x07 << 3)),            //28 spi 0                INT_ID44_SP0,
    ((0x02 << 6) | (0x07 << 3)),            //29 i2c2                 INT_ID45_I2C2,
    ((0x02 << 6) | (0x07 << 3)),            //30 i2c1                 INT_ID46_I2C1,
    ((0x02 << 6) | (0x07 << 3)),            //31 i2c0                 INT_ID47_I2C0,
    ((0x02 << 6) | (0x07 << 3)),            //32 i2s1                 INT_ID48_I2S1,
    ((0x02 << 6) | (0x07 << 3)),            //33 i2s0                 INT_ID49_I2S0,
    ((0xFF << 6) | (0xFF << 3)),            //34 hifi                 INT_ID50_HIFI,
    ((0xFF << 6) | (0xFF << 3)),            //35 pmu                  INT_ID51_PMU,
    ((0xFF << 6) | (0xFF << 3)),            //36 gpio2                INT_ID52_GPIO2,
    ((0xFF << 6) | (0xFF << 3)),            //37 gpio1                INT_ID53_GPIO1,
    ((0xFF << 6) | (0xFF << 3)),            //38 gpio0             INT_ID54_GPIO0,
    ((0xFF << 6) | (0xFF << 3)),            //39 VOP               INT_ID55_VOP,
    ((0xFF << 6) | (0xFF << 3)),            //40 dma2              INT_ID56_DMA2,

};



void IrqPriorityInit(void)              //中断优先级初始化, 在IntPriorityGroupingSet执行后调用
{
    uint32 i;

    IntPriorityGroupingSet(2);
    for (i = 4; i < NUM_INTERRUPTS; i++)
    {
        IntPrioritySet(i, IrqPriorityTab[i]);
    }
}

/*
********************************************************************************
*
*                         End of interrupt.c
*
********************************************************************************
*/

