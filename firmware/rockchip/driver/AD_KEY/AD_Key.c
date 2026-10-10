/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  AD_key.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-2-10          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_ADKEY_

#include "DriverInclude.h"
#include "OsInclude.h"

/*
--------------------------------------------------------------------------------
  Function name : KeyReset(void)
  Author        : anzhiguo
  Description   : key reset

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void KeyReset(void)
{
    KeyVal       = KEY_VAL_NONE;
    KeyScanVal   = KEY_VAL_NONE;
    KeyStatus    = KEY_STATUS_NONE;

    KeyScanCounter  = 0;

    KeyFlag.bc.bKeyHave = 0;
    KeyFlag.bc.bKeyDown = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 Get_PlayKey_State(void)
  Author           :
  Description     :
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
uint32 Get_PlayKey_State(void)
{
    uint8 temp;

    temp = Gpio_GetPinLevel(GPIO_CH2,GPIOPortA_Pin5);

    return (temp);
}

/*
--------------------------------------------------------------------------------
  Function name : KeyReset(void)
  Author        : anzhiguo
  Description   : get key value

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
uint32 keybackup = 0;
_ATTR_DRIVER_CODE_
UINT32 GetKey(UINT32 *pKeyVal)
{
    UINT32 KeyTemp,HoldTemp;
    UINT32 RetVal = 0;

	KeyTemp = 0;
    if (Get_PlayKey_State())
    {
        KeyTemp = 1;
    }
	HoldTemp = KeyTemp & 0x01;

	if(KeyScanCounter == 0)
	{
	    *pKeyVal = (UINT32)((KeyTemp & 0x0001));    //paly 键 或者没有键
		RetVal = -1;
	}
	else if(KeyScanCounter == 1)
	{
        if (CheckAdcState(ADC_CHANEL_KEY))
        {
			KeyTemp = GetAdcData(ADC_CHANEL_KEY);
            //DEBUG("ADKey = %d", KeyTemp);

			if ((ADKEY2_MIN <= KeyTemp ) && (KeyTemp < ADKEY2_MAX))
			{
				*pKeyVal |= KEY_VAL_ADKEY2;
			}
			else if ((ADKEY3_MIN <= KeyTemp ) && (KeyTemp < ADKEY3_MAX))
			{
				*pKeyVal |= KEY_VAL_ADKEY3;
			}
			else if ((ADKEY4_MIN <= KeyTemp ) && (KeyTemp < ADKEY4_MAX))
			{
				*pKeyVal |= KEY_VAL_ADKEY4;
			}
			else if ((ADKEY5_MIN <= KeyTemp ) && (KeyTemp < ADKEY5_MAX))
			{
				*pKeyVal |= KEY_VAL_ADKEY5;
			}
			else if ((ADKEY6_MIN <= KeyTemp ) && (KeyTemp < ADKEY6_MAX))
			{
				*pKeyVal |= KEY_VAL_ADKEY6;
			}
			else if ((ADKEY7_MIN <= KeyTemp ) && (KeyTemp < ADKEY7_MAX))
			{
				*pKeyVal |= KEY_VAL_ADKEY7;
			}
		}
		else if(IsBackLightOn == TRUE)
		{
		}
		else
		{
		    return -1;  //ad 转换未完成，等待下一次scan
		}

		RetVal = 0;
	}

	if ((HoldTemp == 1) && (*pKeyVal & KEY_VAL_MENU))
	{
	    *pKeyVal = KEY_VAL_HOLD;
	}

    KeyScanCounter = (KeyScanCounter + 1) % 2;//为什么要进行2次 ，如果两个按键合在一起会有什么哪?

    return RetVal;
}

/*
--------------------------------------------------------------------------------
  Function name : void KeyScan(void)
  Author        : anzhiguo
  Description   : key scan program

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void KeyScan(void)
{
    UINT32 RetVal, i;

    if (KeyFlag.bc.bKeyHave != 0) //KeyFlag.bc.bKeyHave是在外部调用GetKeyVal时清零
        return;

    if (0 != GetKey(&KeyScanVal)) //KeyScanVal 获取了当前的按键值
        return;

    //DEBUG("KeyScanVal = 0x%x", KeyScanVal);
    if (KeyScanVal != KEY_VAL_NONE)
    {
        if (KeyScanVal != PrevKeyVal)
        {
            if (KeyScanCnt >= KEY_TIME_PRESS_START) //ylz++
            {
                KeyFlag.bc.bKeyHave = 1;
                KeyVal = (KEY_STATUS_LONG_UP) | (KeyVal & KEY_VAL_MASK);
            }
            else if (KeyScanCnt >= KEY_TIME_DOWN) //ylz++
            {
                KeyFlag.bc.bKeyHave = 1;
                KeyVal = (KEY_STATUS_SHORT_UP) | (KeyVal & KEY_VAL_MASK);
            }
            KeyScanCnt = 0;
            KeyStatus = KEY_STATUS_DOWN;
        }
        else
        {
            if (++KeyScanCnt == 0)
            {
                KeyScanCnt = KEY_TIME_PRESS_START + KEY_TIME_PRESS_STEP;
            }
            else if (KeyScanCnt == KEY_TIME_DOWN)
            {
                KeyFlag.bc.bKeyDown = 1;
                if((IsBackLightOn == TRUE))
                {
                    if ((gSysSetBLModeArray[gSysConfig.BLmode] == BL_DARK))
                    {
                        #ifdef DEEP_SLEEP
                        //if (KeyScanVal == KEY_VAL_POWER)
                        #endif
                        {
                            SendMsg(MSG_SYS_RESUME);
                            //FREQ_EnterModule(FREQ_BLON);
                        }

                        //#ifdef KEY_RESUME_ENABLE
                        KeyReset();
                        return;
                        //#endif
                    }
                    else
                    {
                        SendMsg(MSG_SYS_RESUME);
                    }
                }
            }
        }
        BacklightSystickCounterBack = SysTickCounter - 5;

        #ifdef KEY_RESUME_ENABLE
        if((KeyScanVal != KEY_VAL_UNHOLD) && (KeyScanVal != KEY_VAL_HOLD))
        {

            if(IsBackLightOn == TRUE)
            {
                SendMsg(MSG_SYS_CLEAR_KEYVAL);
            }
        }
        #endif


        switch (KeyStatus)
        {
        case KEY_STATUS_DOWN:
            if (KeyScanCnt == KEY_TIME_DOWN)
            {
                KeyFlag.bc.bKeyHave = 1;
                KeyVal = (KEY_STATUS_DOWN) | KeyScanVal;

            }
            else if (KeyScanCnt == KEY_TIME_PRESS_START)
            {
                KeyStatus = KEY_STATUS_PRESS;
                KeyFlag.bc.bKeyHave = 1;
                KeyVal = (KEY_STATUS_PRESS_START) | KeyScanVal;
            }
            break;

        case KEY_STATUS_PRESS:
            if (((KeyScanCnt - KEY_TIME_PRESS_START) % KEY_TIME_PRESS_STEP) == 0 )
            {
                KeyFlag.bc.bKeyHave = 1;
                KeyFlag.bc.bKeyDown = 1;

                if (KeyScanVal == KEY_VAL_POWER)
                {
                    if (KeyScanCnt > 100)
                    {
                        if (KeyFlag.bc.bKeyHold == 0)
                        {
                            SendMsg(MSG_POWER_DOWN);
                        }
                    }
                }

                KeyVal = (KEY_STATUS_PRESS) | KeyScanVal;
            }
            break;

        default:

            break;
        }

    }
    else
    {
        if (KeyScanCnt >= KEY_TIME_PRESS_START) //ylz++
        {
            KeyFlag.bc.bKeyHave = 1;
            KeyVal = (KEY_STATUS_LONG_UP) | (KeyVal & KEY_VAL_MASK);
        }
        else if (KeyScanCnt >= KEY_TIME_DOWN)//ylz++
        {
            KeyFlag.bc.bKeyHave = 1;
            KeyVal = (KEY_STATUS_SHORT_UP) | (KeyVal & KEY_VAL_MASK);
        }

        KeyScanCnt = 0;
        KeyStatus  = KEY_STATUS_NONE;
        KeyFlag.bc.bKeyDown = 0;//ywj 09-7-25

    }
    PrevKeyVal = KeyScanVal;
}

/*
--------------------------------------------------------------------------------
  Function name : AdcKey_SysTimer_Handler()
  Author        : anzhiguo
  Description   : ADC Key Timer Isr

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void AdcKey_SysTimer_Handler(void)
{
    KeyScan();
    AdcStart(ADC_CHANEL_KEY);
}

_ATTR_DRIVER_DATA_
SYSTICK_LIST AdcKey_SysTimer =
{
    NULL,
    0,
    1,
    0,
    AdcKey_SysTimer_Handler,
};

/*
--------------------------------------------------------------------------------
  Function name : KeyInit()
  Author        : anzhiguo
  Description   : key scan variables initialization.

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void KeyInit(void)
{
    KeyVal          = 0;
    PrevKeyVal      = 0;
    KeyStatus       = 0;
    KeyScanCnt      = 0;

    KeyScanVal      = KEY_VAL_NONE;
    KeyScanCounter  = 0;
    KeyFlag.word    = 0;

    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin5,IOMUX_GPIO2A5_IO);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin5,GPIO_IN);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin5,ENABLE);

    SystickTimerStart(&AdcKey_SysTimer);
}
/*
--------------------------------------------------------------------------------
  Function name : void KeyVolAdjStart(void)
  Author        : anzhiguo
  Description   :

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_DATA_ UINT8 showtimes = 0;
_ATTR_DRIVER_CODE_
void ClearVolMsg(void)
{
    showtimes = 0;
    ClearMsg(MSG_KEY_VOL_STATE);
    ClearMsg(MSG_KEY_VOL_SHOWSTATE);
}

/*
--------------------------------------------------------------------------------
  Function name : void KeyVolAdjStart(void)
  Author        : anzhiguo
  Description   :

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void KeyVolAdjStart(void)
{
    KeyVolAdjustCounter = SysTickCounter;
    SendMsg(MSG_KEY_VOL_STATE);
    SendMsg(MSG_KEY_VOL_STATE_UPDATE);
}

/*
--------------------------------------------------------------------------------
  Function name : void KeyVolAdjStart(void)
  Author        : anzhiguo
  Description   :

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void KeyVolAdjCheck(void)
{
    UINT32 Count;

    if (CheckMsg(MSG_KEY_VOL_STATE))
    {
        if((SysTickCounter - KeyVolAdjustCounter) >= 50)//0.5s
        {
            KeyVolAdjustCounter = SysTickCounter;
            showtimes++;
            if (showtimes & 0x01)
            {
                SendMsg(MSG_KEY_VOL_SHOWSTATE);
            }
            else
            {
                ClearMsg(MSG_KEY_VOL_SHOWSTATE);
            }

            if(showtimes > 10)// 8s显示时间
            {
                showtimes = 0;
                ClearMsg(MSG_KEY_VOL_STATE);
			    ClearMsg(MSG_KEY_VOL_SHOWSTATE);
            }

            SendMsg(MSG_KEY_VOL_STATE_UPDATE);
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetKeyVal(void)
  Author        : anzhiguo
  Description   :

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo        2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
UINT32 GetKeyVal(void)
{
    UINT32 imask_tmp;
    UINT32 KeyValTemp;

    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        KeyVolAdjCheck();
    }

    if(KeyFlag.bc.bKeyHave == 0) //没有任何按键按下
    {
        return  KEY_VAL_NONE;
    }

    //TIMER0_INTERRUPT_DISABLE;
    KeyValTemp = KeyVal;
    KeyFlag.bc.bKeyHave = 0;
    //TIMER0_INTERRUPT_ENABLE;

    if (KeyFlag.bc.bKeyHold == 0)
    {
        if (KeyValTemp == KEY_VAL_HOLD_PRESS_START)
        {
            KeyFlag.bc.bKeyHold = 1;

            KeyValTemp = KEY_VAL_HOLD_ON;
        }
    }
    else
    {
        if ((KeyValTemp & KEY_VAL_MASK) == KEY_VAL_HOLD)
        {
            if (KeyValTemp == KEY_VAL_HOLD_PRESS_START)
            {
                KeyFlag.bc.bKeyHold = 0;

                KeyValTemp = KEY_VAL_HOLD_OFF;
            }
            else
            {
                KeyValTemp = KEY_VAL_NONE;
            }
        }
        else
        {
            if ((KeyValTemp & KEY_VAL_UNMASK) == KEY_STATUS_DOWN)
            {
                KeyValTemp = KEY_VAL_HOLD_ON;
            }
            else
            {
                KeyValTemp = KEY_VAL_NONE;
            }
        }
    }

    return (KeyValTemp);
}

/*
********************************************************************************
*
*                         End of AD_Key.c
*
********************************************************************************
*/

