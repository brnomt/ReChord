/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   battry.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             yangwenjie      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _BATTERY_H_
#define _BATTERY_H_

#undef  EXT
#ifdef _IN_BATTERY_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------
  
                        Macro define  
  
--------------------------------------------------------------------------------
*/
#define VCC_VALUE                   2500       //AD reference voltage

#if (BATTERY_LEVEL == 5)

#define BATT_TOTAL_STEPS            5
#define BATT_USB_FULL_VALUE         4100 //ywj-7-27
#define BATT_FULL_VALUE             4000  //1 charge full voltage 4.19v,hardware use it when hardware do not support interupt of charge full.
#define BATT_LEVEL2                 3800
#define BATT_LEVEL1                 3600
#define BATT_LEVEL0                 3500
#define BATT_EMPTY_VALUE            3350  //1 3.3V low voltage warning,it is  3.2 /3.3v really.

#elif ((BATTERY_LEVEL == 4))

#define BATT_TOTAL_STEPS            4
#define BATT_USB_FULL_VALUE         4100 //ywj-7-27
#define BATT_FULL_VALUE             4000  //1 charge full voltage 4.19v,hardware use it when hardware do not support interupt of charge full.
//#define BATT_LEVEL2                 3800
#define BATT_LEVEL1                 3750
#define BATT_LEVEL0                 3500
#define BATT_EMPTY_VALUE            3350  //1 3.3V low voltage warning,it is  3.2 /3.3v really.

#endif

#define CHARGEOKCNTRMAX             100//20        //check many a time,check battery is in charge full status
#define BATT_POWEROFF_CNT           20
#define BATT_POWEROFF_VALUE         BATT_EMPTY_VALUE
#define BATT_CHECK_TIME             1//message box display 5 seconds.

/*
--------------------------------------------------------------------------------
  
                        Struct Define
  
--------------------------------------------------------------------------------
*/
typedef enum
{
    CHARGE_NO,  //power is not changed.
    CHARGE_BATT,//power is changed.
    CHARGE_FULL //battery charge full.
}CHARGE_STATE_t;


typedef struct
{
    UINT16 Batt_Level;
    UINT32 Batt_Value;
    UINT16 Batt_LowCnt;
    BOOL   Batt_LowPower;
    UINT16 Batt_Count;
    UINT16 UsbBatt_Level;
    BOOL   IsBatt_Charge;
    BOOL   IsBattChange;//USB take it when pull out or power up.
}BATT_INFO;

/*
--------------------------------------------------------------------------------
  
                        Variable Define
  
--------------------------------------------------------------------------------
*/

_ATTR_SYS_BSS_ EXT BATT_INFO    gBattery;
_ATTR_SYS_BSS_ EXT UINT32       ChargeFullFlag;
_ATTR_SYS_BSS_ EXT UINT32       BatterySystickCounterBack;
_ATTR_SYS_BSS_ EXT UINT32       BatteryCounter;
_ATTR_SYS_BSS_ EXT UINT32       BatteryCounter1;

#ifdef _IN_BATTERY_
_ATTR_SYS_DATA_ UINT16 Batt_Level[BATT_TOTAL_STEPS] =
{
    BATT_EMPTY_VALUE,
    BATT_LEVEL0,
    BATT_LEVEL1, 

#if (BATTERY_LEVEL == 5)

    BATT_LEVEL2, 
    BATT_FULL_VALUE
    
#elif (BATTERY_LEVEL == 4)
    //BATT_LEVEL2, 
    BATT_FULL_VALUE
    
#endif
};
#else
_ATTR_SYS_DATA_ EXT UINT16 Batt_Level[BATT_TOTAL_STEPS];
#endif

#define BATTERY_VALUE_SETP              20

/*
*-------------------------------------------------------------------------------
*
*                            Function declaration
*
*-------------------------------------------------------------------------------
*/

EXT UINT32 Battery_GetLevel(void);
EXT UINT16 Battery_GetChargeState(void);
EXT UINT32 RealBattValue(UINT16 adc);


/*
********************************************************************************
*
*                         End of battry.h
*
********************************************************************************
*/
#endif
