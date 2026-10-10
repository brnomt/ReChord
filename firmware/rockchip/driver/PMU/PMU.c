/*
********************************************************************************************
*
*                Copyright (c): 2015 - 2015 + 5, aaron.sun
*                             All rights reserved.
*
* FileName: Common\Driver\PMU\PMU.C
* Owner: aaron.sun
* Date: 2015.3.5
* Time: 15:56:46
* Desc: 
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.3.5     15:56:46   1.0
********************************************************************************************
*/

#define __COMMON_DRIVER_PMU_PMU_C__
#ifdef __COMMON_DRIVER_PMU_PMU_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define                                   
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "typedef.h"
#include "DriverInclude.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define     					 
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _COMMON_DRIVER_PMU_PMU_READ_  
#define _COMMON_DRIVER_PMU_PMU_WRITE_ 
#define _COMMON_DRIVER_PMU_PMU_INIT_  
#define _COMMON_DRIVER_PMU_PMU_SHELL_  


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define     	     				 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define     	     				 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare     	     				 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define                    				 
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: PmuSoftReset
** Input:uint32 module
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 15:40:58
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuSoftReset(uint32 module, uint32 reset)
{
    if(reset)
    {
        Pmu_Reg->PMU_SOFTRST_CON |= module;
    }
    else
    {
        Pmu_Reg->PMU_SOFTRST_CON &= ~module;
    }
}

/*******************************************************************************
** Name: PmuGpio2NegedegeIntDisable
** Input:uint32 gpio
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 15:31:55
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpio2NegedegeIntDisable(uint32 gpio)
{
    Pmu_Reg->PMU_GPIO_NEG_INT_CON &= ~(1 << gpio);
}

/*******************************************************************************
** Name: PmuGpio2NegedegeIntEnable
** Input:uint32 gpio
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 15:31:10
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpio2NegedegeIntEnable(uint32 gpio)
{
    Pmu_Reg->PMU_GPIO_NEG_INT_CON |= (1 << gpio);
}

/*******************************************************************************
** Name: PmuGpio2PosedegeIntDisable
** Input:uint32 gpio
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 15:20:10
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpio2PosedegeIntDisable(uint32 gpio)
{
    Pmu_Reg->PMU_GPIO_POS_INT_CON &= ~(1 << gpio);
}


/*******************************************************************************
** Name: PmuGpio2PosedegeIntEnable
** Input:uint32 gpio
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 14:46:15
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpio2PosedegeIntEnable(uint32 gpio)
{
    Pmu_Reg->PMU_GPIO_POS_INT_CON |= (1 << gpio);
}


/*******************************************************************************
** Name: PmuIntDisable
** Input:uint32 intmode
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 14:44:18
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuIntDisable(uint32 intmode)
{
    Pmu_Reg->PMU_INT_CON &= ~intmode;
}

/*******************************************************************************
** Name: PmuIntEnable
** Input:uint32 intmask
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 14:42:55
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuIntEnable(uint32 intmode)
{
    Pmu_Reg->PMU_INT_CON |= intmode;
}

/*******************************************************************************
** Name: PmuOscPowerDown
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 14:41:37
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuOscPowerDown(void)
{
    Pmu_Reg->PMU_SOFT_CON |= PMU_OSC_POWER_DOWN_EN;
}

/*******************************************************************************
** Name: PmuSelLowFreq
** Input:uint32 selpath
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 14:36:35
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuSelLowFreq(uint32 selpath)
{
    if(selpath == PMU_LOW_FREQ_CLK_SEL_PVTM)
    {
        Pmu_Reg->PMU_SOFT_CON |= PMU_LOW_FREQ_CLK_SEL_PVTM;
    }
    else
    {
        Pmu_Reg->PMU_SOFT_CON &= PMU_LOW_FREQ_CLK_SEL_24M;
    }
        
}

/*******************************************************************************
** Name: PmuUseLowFreqEn
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 14:36:00
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuUseLowFreqEn(void)
{
    Pmu_Reg->PMU_SOFT_CON |= PMU_USE_LOW_FREQ_CLK_EN;
}


/*******************************************************************************
** Name: PmuSetPowerMode
** Input:uint32 PowerMode
** Return: void
** Owner:aaron.sun
** Date: 2015.3.9
** Time: 14:32:51
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuSetPowerMode(uint32 PowerMode)
{
    Pmu_Reg->PMU_OSC_CNT = 0x1000;
    Pmu_Reg->PMU_PLLLOCK_CNT = 0x1000;
    Pmu_Reg->PMU_CORE_PWRUP_CNT = 0x1000;    
    Pmu_Reg->PMU_POWER_MODE_CON = PowerMode;
    
    //printf("\n%x = %x", &Pmu_Reg->PMU_POWER_MODE_CON, PowerMode);
}

/*******************************************************************************
** Name: PmuCheckPdLogicPowerDown
** Input:uint32  pdhigh
** Return: void
** Owner:aaron.sun
** Date: 2015.3.6
** Time: 14:46:33
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API BOOL PmuCheckPdLogicPowerDown(uint32  pdhigh)
{
    Pmu_Reg->PMU_PWRDN_ST |= PMU_PD_HIGH_POWER_DOWN;
}

/*******************************************************************************
** Name: PmuPdLogicPowerDown
** Input:uint32 pdhigh
** Return: void
** Owner:aaron.sun
** Date: 2015.3.6
** Time: 14:43:46
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuPdLogicPowerDown(uint32 pdhigh)
{
    if(pdhigh)
    {
        Pmu_Reg->PMU_PWRDN_CN |= PMU_PD_HIGH_POWER_DOWN;
    }
    else        
    {    
        Pmu_Reg->PMU_PWRDN_CN &= ~(PMU_PD_HIGH_POWER_DOWN);
    }    
}

/*******************************************************************************
** Name: PmuGpioNegedegePlusWakeUpDisable
** Input:uint32 GpioNum
** Return: void
** Owner:aaron.sun
** Date: 2015.3.6
** Time: 14:40:03
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpioNegedegePlusWakeUpDisable(uint32 GpioNum)
{
    Pmu_Reg->PMU_WAKEUP_CFG1 &= ~(1 << GpioNum);
}

/*******************************************************************************
** Name: PmuGpioNegedegePlusWakeUpEnable
** Input:uint32 GpioNum
** Return: void
** Owner:aaron.sun
** Date: 2015.3.6
** Time: 14:39:02
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpioNegedegePlusWakeUpEnable(uint32 GpioNum)
{
    Pmu_Reg->PMU_WAKEUP_CFG1 |= (1 << GpioNum);
}

/*******************************************************************************
** Name: PmuGpioPosedegePlusWakeUpDisable
** Input:uint32 GpioNum
** Return: void
** Owner:aaron.sun
** Date: 2015.3.6
** Time: 14:33:55
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpioPosedegePlusWakeUpDisable(uint32 GpioNum)
{
    Pmu_Reg->PMU_WAKEUP_CFG2 &= ~(1 << GpioNum);
}

/*******************************************************************************
** Name: PmuGpioPosedegePlusWakeUpEnable
** Input:uint32 GpioNum
** Return: void
** Owner:aaron.sun
** Date: 2015.3.6
** Time: 14:32:46
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpioPosedegePlusWakeUpEnable(uint32 GpioNum)
{
    Pmu_Reg->PMU_WAKEUP_CFG2 |= (1 << GpioNum);
    //printf("Pmu_Reg->PMU_WAKEUP_CFG2 = %x\n", Pmu_Reg->PMU_WAKEUP_CFG2);
}

/*******************************************************************************
** Name: PmuGpioIntWakeUpDisable
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.3.6
** Time: 14:26:00
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpioIntWakeUpDisable(void)
{
    Pmu_Reg->PMU_WAKEUP_CFG0 &= ~PMU_GPIO_INT_WAKEUP_ENABLE;
}

/*******************************************************************************
** Name: PmuGpioIntWakeUpEnable
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.3.6
** Time: 14:25:08
*******************************************************************************/
_COMMON_DRIVER_PMU_PMU_READ_
READ API void PmuGpioIntWakeUpEnable(void)
{
    Pmu_Reg->PMU_WAKEUP_CFG0 |= PMU_GPIO_INT_WAKEUP_ENABLE;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define      				 
*
*---------------------------------------------------------------------------------------------------------------------
*/




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define                    			 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define         			 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define             	     			 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define              	  		 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define              	  		 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define              	  		 
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

