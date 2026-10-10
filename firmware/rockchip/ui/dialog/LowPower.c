/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name£º   LowPower.c
*
* Description:
*
* History:      <author>          <time>        <version>
*                anzhiguo      2009-2-27         1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_DIALOG_LOWPOWER_

#include "SysInclude.h"
#include "LowPower.h"

/*
--------------------------------------------------------------------------------
  Function name : void MessageBoxInit(void *pArg)
  Author        : ZhengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi      2009-4-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LOWERPOWER_CODE_
void DialogLowPowerInit(void *pArg)
{
    KeyReset();

    //initial regular time
    DialogLowPowerSystickCounterBack = SysTickCounter;

    SendMsg(MSG_LOWPOWER_DISPLAY_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : void MessageBoxDeInit(void)
  Author        : ZhengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi      2009-4-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LOWERPOWER_CODE_
void DialogLowPowerDeInit(void)
{

}

/*
--------------------------------------------------------------------------------
  Function name : uint32 MessageBoxService(void)
  Author        : ZhengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi      2009-4-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LOWERPOWER_CODE_
uint32 DialogLowPowerService(void)
{
    INT16 Retval = 0;
    UINT32 Count;

    //check regular time,rugular time come,shut down.
    Count = SysTickCounter - DialogLowPowerSystickCounterBack;
    if (Count>LOWERPOWER_DISPLAY_TIME*100)
    {
        SendMsg(MSG_POWER_DOWN);
    }

    return(Retval);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MessageBoxKey(void)
  Author        : ZhengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi      2009-4-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LOWERPOWER_CODE_
uint32 DialogLowPowerKey(void)
{
    return (0);
}

/*
--------------------------------------------------------------------------------
  Function name : void MessageBoxDisplay(void)
  Author        : ZhengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi      2009-4-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LOWERPOWER_CODE_
void DialogLowPowerDisplay(void)
{
    UINT16              color_tmp, TempCharSize;
    PICTURE_INFO_STRUCT PicInfo;    //it will be used when operating text that is related with brower position

    color_tmp = LCD_GetColor();
    LCD_SetColor(COLOR_BLACK);
    TempCharSize = LCD_SetCharSize(FONT_12x12);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || GetMsg(MSG_LOWPOWER_DISPLAY_ALL))
    {
        if (TRUE == CheckMsg(MSG_VIDEO_VERTICAL_DISPLAY))//ylz++
        {
            DispPictureWithIDNumAndXY(IMG_ID_LOWPOWER,32,32);
        }
        else
        {
            DispPictureWithIDNum(IMG_ID_LOWPOWER);
        }
    }

    LCD_SetColor(color_tmp);
    LCD_SetCharSize(TempCharSize);
}

/*
********************************************************************************
*
*                         End of LowPower.c
*
********************************************************************************
*/
