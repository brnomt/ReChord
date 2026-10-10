/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  Backlight.c
*
* Description:
*
*
* History:      <author>          <time>        <version>
*               yangwenjie        2009-01-19         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_BACKLIGHT_

#include "SysInclude.h"
#include "freq_enums.h"   /* FREQ_BLON & freq module IDs */

_ATTR_SYS_DATA_
uint8 BL_Table[BL_LEVEL_MAX + 1] = {85,80,70,60,10,95};

/*
--------------------------------------------------------------------------------
  Function name : void BL_SetBright(BL_LIGHT_en_t light)
  Author        : yangwenjie
  Description   : setting backlight

  Input         :

  Return        :

  History:     <author>         <time>         <version>
              yangwejie    2009-1-5        Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void BL_SetLevel(uint8 Level)
{
    PwmRateSet(BL_PWM_CH,BL_Table[Level],PWM_FREQ);
}
/*
--------------------------------------------------------------------------------
  Function name : void BL_DeInit(void)
  Author        : yangwenjie
  Description   : close backlight

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void BL_Compensate(uint8 Level)
{
    uint8 TmpPwmRate;

    if (Level < BL_LEVEL_MAX)
    {
        Level = (BL_LEVEL_MAX - 1) - Level;
        if (Level == 0 )
            TmpPwmRate = BL_PWM_RATE_MIN + BL_PWM_RATE_STEP * Level+55;
        else if (Level == 1 )
            TmpPwmRate = BL_PWM_RATE_MIN + BL_PWM_RATE_STEP * Level+45;
        else if (Level == 2)
            TmpPwmRate = BL_PWM_RATE_MIN + BL_PWM_RATE_STEP * Level+35;
        else if (Level == 3)
            TmpPwmRate = BL_PWM_RATE_MIN + BL_PWM_RATE_STEP * Level+28;
        else
            TmpPwmRate = BL_PWM_RATE_MIN + BL_PWM_RATE_STEP * Level+20;
    }
    PwmRateSet(BL_PWM_CH, TmpPwmRate,PWM_FREQ);

}

/*
--------------------------------------------------------------------------------
  Function name : void BL_DeInit(void)
  Author        : yangwenjie
  Description   :

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void BL_On(void)
{
    //FREQ_EnterModule(FREQ_BLON);

    PWM_Start(BL_PWM_CH);
    BL_SetLevel(gSysConfig.BLevel);

    BacklightSystickCounterBack	= SysTickCounter;
    ClearMsg(MSG_BL_OFF);
}

/*
--------------------------------------------------------------------------------
  Function name : void BL_DeInit(void)
  Author        : yangwenjie
  Description   : close backlight

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void BL_Off(void)
{
    PWM_Stop(BL_PWM_CH);
    //FREQ_ExitModule(FREQ_BLON);

    SendMsg(MSG_BL_OFF);
}

/*
--------------------------------------------------------------------------------
  Function name : void BLOffEnable(void)
  Author        : ZhengYongzhi
  Description   : close backlight enable.

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void BLOffEnable(void)
{
    ClearMsg(MSG_BL_OFF_DISABLE);
    BacklightSystickCounterBack = SysTickCounter;
}

/*
--------------------------------------------------------------------------------
  Function name : void BLOffDisable(void)
  Author            : ZhengYongzhi
  Description      : close backlight disable.

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void BLOffDisable(void)
{
    SendMsg(MSG_BL_OFF_DISABLE);
}

/*
--------------------------------------------------------------------------------
  Function name : void BLOffEnable(void)
  Author        : ZhengYongzhi
  Description   : close backlight enable

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void LcdStandbyEnable(void)
{
    ClearMsg(MSG_LCD_STANDBY_DISABLE);
}

/*
--------------------------------------------------------------------------------
  Function name : void BLOffDisable(void)
  Author            : ZhengYongzhi
  Description      : 禁止关闭背光

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void LcdStandbyDisable(void)
{
    SendMsg(MSG_LCD_STANDBY_DISABLE);
}

/*
--------------------------------------------------------------------------------
  Function name : void BL_DeInit(void)
  Author        : yangwenjie
  Description   : backlight detect

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void BacklightDetec(void)
{
    UINT32 Count;
    uint32 Batt, Temp;

    if ((FALSE == IsBackLightOn) && (FALSE == CheckMsg(MSG_BL_OFF_DISABLE)))
    {
        if (gSysSetBLTimeArray[gSysConfig.BLtime])
        {
            Count = SysTickCounter - BacklightSystickCounterBack;
            if (Count>gSysSetBLTimeArray[gSysConfig.BLtime]*100)
            {
                //if(gSysSetBLModeArray[gSysConfig.BLmode] == BL_DARK)
                {
                    BL_Off();
                    if (FALSE == CheckMsg(MSG_LCD_STANDBY_DISABLE))
                    {
                        LcdStandby();
                        AdcSleepEnter();
                    }
                    //zyz: 当系统允许自动关机时，一般就允许进入待机模式
                    if (AutoPowerOffDisableCounter == 0)
                    {
                        DEBUG("###OS idle###");
                        Os_idle();
                    }
                    else
                    {
                        FREQ_ExitModule(FREQ_BLON);
                    }
                }

                IsBackLightOn = TRUE;

                return;
            }
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void BL_DeInit(void)
  Author        : yangwenjie
  Description   : backlight resume

  Input         :
  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie       2008-1-15          Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void BL_Resume(void)
{
    BacklightSystickCounterBack = SysTickCounter;

    if (IsBackLightOn == TRUE)
    {
        IsBackLightOn = FALSE;

        //if(gSysSetBLModeArray[gSysConfig.BLmode] == BL_DARK)
        {
            if (FALSE == CheckMsg(MSG_LCD_STANDBY_DISABLE))
            {
                LcdWakeUp();
                AdcSleepExit();
            }

            DEBUG("=====BL RESUME=====");
            SendMsg(MSG_NEED_BACKLIGHT_ON);
            SendMsg(MSG_NEED_PAINT_ALL);
        }

#ifdef KEY_RESUME_ENABLE
        KeyReset();
#endif
    }
}

/*
********************************************************************************
*
*                         End of backlight.c
*
********************************************************************************
*/

