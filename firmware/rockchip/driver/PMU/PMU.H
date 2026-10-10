/*
********************************************************************************************
*
*				  Copyright (c): 2015 - 2015 + 5, aaron.sun
*							   All rights reserved.
*
* FileName: Common\Driver\PMU\PMU.H
* Owner: aaron.sun
* Date: 2015.3.5
* Time: 15:56:51
* Desc: 
* History:
*    <author>	 <date> 	  <time>	 <version>	   <Desc>
*    aaron.sun     2015.3.5     15:56:51   1.0
********************************************************************************************
*/

#ifndef __COMMON_DRIVER_PMU_PMU_H__
#define __COMMON_DRIVER_PMU_PMU_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define									 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define						 
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare							 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare          							 
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void PmuSoftReset(uint32 module, uint32 reset);
extern void PmuGpio2NegedegeIntDisable(uint32 gpio);
extern void PmuGpio2NegedegeIntEnable(uint32 gpio);
extern void PmuGpio2PosedegeIntDisable(uint32 gpio);
extern void PmuGpio2PosedegeIntEnable(uint32 gpio);
extern void PmuIntDisable(uint32 intmode);
extern void PmuIntEnable(uint32 intmode);
extern void PmuOscPowerDown(void);
extern void PmuSelLowFreq(uint32 selpath);
extern void PmuUseLowFreqEn(void);
extern void PmuSetPowerMode(uint32 PowerMode);
extern BOOL PmuCheckPdLogicPowerDown(uint32  pdhigh);
extern void PmuPdLogicPowerDown(uint32 pdhigh);
extern void PmuGpioNegedegePlusWakeUpDisable(uint32 GpioNum);
extern void PmuGpioNegedegePlusWakeUpEnable(uint32 GpioNum);
extern void PmuGpioPosedegePlusWakeUpDisable(uint32 GpioNum);
extern void PmuGpioPosedegePlusWakeUpEnable(uint32 GpioNum);
extern void PmuGpioIntWakeUpDisable(void);
extern void PmuGpioIntWakeUpEnable(void);



#endif

