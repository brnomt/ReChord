/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  backlight.h
* 
* Description:  
*                      
*
* History:      <author>          <time>        <version>       
*               yangwenjie        2009-01-19         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _BACKLIGHT_H_
#define _BACKLIGHT_H_

#undef  EXT
#ifdef _IN_BACKLIGHT_
#define EXT
#else
#define EXT extern
#endif
/*
--------------------------------------------------------------------------------
  
                        Macro define  
  
--------------------------------------------------------------------------------
*/
#define PWM_FREQ        4000
#define BL_PWM_CH       0

/*
--------------------------------------------------------------------------------
  
                        Struct Define
  
--------------------------------------------------------------------------------
*/
typedef enum BL_LIGHT
{
    BL_DARK     = 100,
	BL_HALFDARK = BL_PWM_RATE_MAX,	
	BL_LOWEST   = BL_PWM_RATE_MIN + BL_PWM_RATE_STEP * 4,
    BL_LUNAR    = BL_PWM_RATE_MIN + BL_PWM_RATE_STEP * 3,
    BL_NORMAL   = BL_PWM_RATE_MIN + BL_PWM_RATE_STEP * 2,
    BL_HIGH     = BL_PWM_RATE_MIN + BL_PWM_RATE_STEP * 1,
    BL_FULL     = BL_PWM_RATE_MIN
} BL_LIGHT_en_t;

typedef enum
{
    BL_T15s  = 15,
    BL_T30s = 30,
    BL_T60s = 60,
    BL_T180s = 180,
    BL_T300s = 300,
    BL_T1800s  = 1800
    
} B_L_TimerS;

/*
--------------------------------------------------------------------------------
  
                        Variable Define
  
--------------------------------------------------------------------------------
*/
_ATTR_SYS_BSS_ EXT UINT32      BacklightSystickCounterBack;
_ATTR_SYS_BSS_ EXT UINT32      BacklightLDOBackup;
_ATTR_SYS_BSS_ EXT BOOL        IsBackLightOn;

#if 0
#ifdef _IN_BACKLIGHT_
_ATTR_SYS_DATA_ UINT16 gSysSetBLBrightArray[] =  
{
    BL_LOWEST,                 
    BL_LUNAR,
    BL_NORMAL,
    BL_HIGH,
    BL_FULL,
	BL_HALFDARK
};
#else
_ATTR_SYS_DATA_ EXT UINT16 gSysSetBLBrightArray[];
#endif
#endif    
    
#ifdef _IN_BACKLIGHT_
_ATTR_SYS_DATA_   UINT16 gSysSetBLTimeArray[] = 
{
    BL_T15s,
    BL_T30s,
    BL_T60s,
    BL_T180s,
    BL_T300s,
    BL_T1800s
};
#else
_ATTR_SYS_DATA_ EXT UINT16 gSysSetBLTimeArray[];
#endif


#ifdef _IN_BACKLIGHT_
_ATTR_SYS_DATA_   UINT16 gSysSetBLModeArray[] = 
{
    BL_HALFDARK ,
    BL_DARK
};
#else
_ATTR_SYS_DATA_ EXT UINT16 gSysSetBLModeArray[];
#endif

/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/ 
 EXT void BL_SetLevel(uint8 Level);
 EXT void BL_On(void);                                 
 EXT void BL_Off(void); 
 EXT void BLOffEnable(void);
 EXT void BLOffDisable(void);
 EXT void LcdStandbyEnable(void);
 EXT void LcdStandbyDisable(void);
 EXT void BacklightDetec(void);
 EXT void BL_Resume(void);
 
/*
********************************************************************************
*
*                         End of backlight.h
*
********************************************************************************
*/
#endif

