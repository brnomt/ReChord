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

#define _IN_DIALOG_HOLD_

#include "SysInclude.h"
#include "Hold.h"

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
_ATTR_HOLD_CODE_
void DialogHoldInit(void *pArg)
{
    HoldState = ((HOLD_WIN_ARG*)pArg)->HoldAction;
    
    //initial timing
    HoldSystickCounterBack = SysTickCounter;
    
    KeyReset();
    SendMsg(MSG_HOLD_DISPLAY_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : void MessageBoxDeInit(void)
  Author        : ZhengYongzhi
  Description   : auti-initial function.
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi      2009-4-3         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_HOLD_CODE_
void DialogHoldDeInit(void)
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
_ATTR_HOLD_CODE_
uint32 DialogHoldService(void)
{
    INT16 Retval = 0;
    UINT32 Count;
    
    if (HoldState == HOLD_STATE_OFF)
    {
        WinDestroy(&HoldWin);
        SendMsg(MSG_HOLDDIALOG_DESTROY);
        SendMsg(MSG_NEED_PAINT_ALL);
        return Retval;
    }

    //check regular time, timing is coming   
    Count = SysTickCounter - HoldSystickCounterBack;
    if(Count>HOLD_DISPLAY_TIME*100)
    {
       WinDestroy(&HoldWin);
       SendMsg(MSG_HOLDDIALOG_DESTROY);
       SendMsg(MSG_NEED_PAINT_ALL);
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
_ATTR_HOLD_CODE_
uint32 DialogHoldKey(void)
{
    UINT32 HoldKeyVal;
    WIN    *HoldOffWin;
    
    HoldKeyVal =  GetKeyVal();
    
    switch (HoldKeyVal) 
    {
    case KEY_VAL_HOLD_ON:
        HoldState = HOLD_STATE_ON;
        SendMsg(MSG_HOLD_DISPLAY_ALL);
        break;
        
    case KEY_VAL_HOLD_OFF:
        HoldState = HOLD_STATE_OFF;
        SendMsg(MSG_HOLD_DISPLAY_ALL);
        break;  
        
    default:        
        break;
    }
    
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
_ATTR_HOLD_CODE_
void DialogHoldDisplay(void)
{
	UINT16              color_tmp, TempCharSize;
    PICTURE_INFO_STRUCT PicInfo;    //it will be use in brower text location.
    
    if (HoldState == HOLD_STATE_OFF)
    {
        return;
    }

    color_tmp = LCD_GetColor();     
    LCD_SetColor(COLOR_BLACK);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    
    if(CheckMsg(MSG_NEED_PAINT_ALL) || GetMsg(MSG_HOLD_DISPLAY_ALL))
    {
        DispPictureWithIDNum(IMG_ID_SYSTEM_LOCKON + HoldState);
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

