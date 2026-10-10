/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name£º   pmc.c   power  manage cotrol
*
* Description:  for the charge, ldo etc. extern power cotrol
*
* History:      <author>          <time>        <version>
*               anzhiguo          2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_PMC_
#include "SysConfig.h"
#include "DriverInclude.h"

_ATTR_DRIVER_DATA_ uint8 LDODisable = 0;
_ATTR_DRIVER_DATA_ uint8 LDORequestCnt = 0;
_ATTR_DRIVER_DATA_ uint8 DCoutDisable = 0;
_ATTR_DRIVER_DATA_ uint8 DCoutRequestCnt = 0;
_ATTR_DRIVER_DATA_ uint8 ChargeCurrent = 0;

/*
--------------------------------------------------------------------------------
  Function name : void VCC_VoltageAdjust_Init(void)
  Author        : zyz
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                     zyz        2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void VCC_VoltageAdjust_Init(void)
{
    LDODisable = 0;
    LDORequestCnt = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void VCC_VoltageAdjust_Enable(void)
  Author        : zyz
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                     zyz        2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void VCC_VoltageAdjust_Enable(void)
{
    if(LDORequestCnt >= 1)
    {
        LDORequestCnt--;
    }

    if(LDORequestCnt == 0)
        LDODisable = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void VCC_VoltageAdjust_Disable(void)
  Author        : zyz
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                     zyz        2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void VCC_VoltageAdjust_Disable(void)
{
    LDODisable = 1;
    LDORequestCnt++;
}

/*
--------------------------------------------------------------------------------
  Function name : void VCC_VoltageAdjust_Set(uint32 Level)
  Author        : anzhiguo

  Input         : clk_id
                  Reset:   1:  reset
                           0:  release reset
  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
uint32 VCC_VoltageAdjust_Set(uint32 Level)
{
    uint32 RetVal = LDOLastLevel;

    if (LDODisable == 1)
    {
        return;
    }

    if (Level == LDOLastLevel)
    {
        return Level;
    }

    //TODO....

    return(RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void VDD_VoltageAdjust_Init(void)
  Author        : zyz
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                     zyz        2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void VDD_VoltageAdjust_Init(void)
{
    DCoutDisable = 0;
    DCoutRequestCnt = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void VDD_VoltageAdjust_Enable(void)
  Author        : zyz
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                     zyz        2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void VDD_VoltageAdjust_Enable(void)
{
    if(DCoutRequestCnt >= 1)
    {
        DCoutRequestCnt--;
    }

    if(DCoutRequestCnt == 0)
        DCoutDisable = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void VDD_VoltageAdjust_Disable(void)
  Author        : zyz
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                     zyz        2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void VDD_VoltageAdjust_Disable(void)
{
    DCoutDisable = 1;
    DCoutRequestCnt++;
}

/*
--------------------------------------------------------------------------------
  Function name : void VDD_VoltageAdjust_Set(uint32 Level)
  Author        : anzhiguo

  Input         : clk_id
                  Reset:   1:  reset
                           0:  release reset
  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
uint32 VDD_VoltageAdjust_Set(uint32 Level)
{
    uint32 RetVal = DCDCLastLevel;

    if(DCoutDisable == 1)
    {
        return RetVal;
    }

    if (Level == DCDCLastLevel)
    {
        return Level;
    }

    //TODO...
    if (Level == SCU_DCOUT_120)
    {
        DCDCLastLevel = Level;
        //PWM1 (VDD_CORE FB)
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3,GPIO_OUT);
        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_HIGH);
    }
    else if (Level == SCU_DCOUT_100)
    {
        DCDCLastLevel = Level;
        //PWM1 (VDD_CORE FB)
        Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);
        Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3,GPIO_OUT);
        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_LOW);
    }

    return(RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void VDD_Voltage_Get(uint32 Level)
  Author        : anzhiguo

  Input         : clk_id
                  Reset:   1:  reset
                           0:  release reset
  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
uint32 VDD_Voltage_Get(void)
{
    return DCDCLastLevel;
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 Adapter_State_Get(void)
  Author           :
  Description     :
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
uint32 Adapter_State_Get(void)
{
    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerPath_Set(BOOL Enable)
  Author           :
  Description     :
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void PowerPath_Set(BOOL Enable)
{
    //for USB & Battery Power select
}

/*
--------------------------------------------------------------------------------
  Function name : void Charge_Enable(void)
  Author           :
  Description     :
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Charge_Enable(BOOL Enable)
{
    //Reserved for Charge control
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 Charge_State_Get(void)
  Author           :
  Description     :
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
uint32 Charge_State_Get(void)
{
    uint32 ret = 0;

    //Charge OK Get
    #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
    ret = Gpio_GetPinLevel(GPIO_CH2, GPIOPortA_Pin1);
    #endif

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : void Charge_Current_Set(Charge_Current_Value Value)
  Author           :
  Description     :
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void Charge_Current_Set(Charge_Current_Value Value)
{
    //Reserved for charge current set

    ChargeCurrent = Value;
    if (Value == CHARGE_CURRENT_LOW)
    {
        #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin4,GPIO_LOW);
        #endif
    }
    else
    {
        #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin4,GPIO_HIGH);
        #endif
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void Charger_Current_Get(Charge_Current_Value Value)
  Author           :
  Description     :
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
Charge_Current_Value Charger_Current_Get(void)
{
    uint32 ret = 0;
    //Reserved for charge current get

    ret = ChargeCurrent;

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : void System_Power_On(BOOL Enable)
  Author           :
  Description     :  System PowerDown Or PowerUp
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void System_Power_On(BOOL Enable)
{
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin6,IOMUX_GPIO2A6_IO);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin6,GPIO_OUT);
    if(Enable)
    {
        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin6,GPIO_HIGH);
    }
    else
    {
        Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin6,GPIO_LOW);
    }
}

/*
********************************************************************************
*
*                         End of Scu.c
*
********************************************************************************
*/


