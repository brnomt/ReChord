/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  SetBright.c
* 
* Description:  set menu execution function configuration of brightness
*
* History:      <author>          <time>        <version>       
*               Jeff Wu      2013-10-08          1.0
*    desc:      
********************************************************************************
*/
#define _SETBRIGHT_IN_

#include "SysInclude.h"

#ifdef _SYSSET_
#include "SetCommon.h"
#include "SetBright.h"
#include "SetMenu.h"
#include "SetMenuInterface.h"
#include "Hold.h"

/*
--------------------------------------------------------------------------------
  Function name : void SetBrightLevelInit(void)
  Author        : Jeff Wu
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
               Jeff Wu       2013-10-08         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_BRIGHT_CODE_
void SetBrightLevelInit(void)
{
    curBrightLevel = SetBackLightLevel;
    KeyReset();             //initial
    SendMsg(MSG_BRIGHT_DISPLAY_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : void SetBrightLevelDeInit(void)
  Author        : Jeff Wu
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
               Jeff Wu     2013-10-08         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_BRIGHT_CODE_
void SetBrightLevelDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SetBrightLevelService(void)
  Author        : Jeff Wu
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                Jeff Wu     2013-10-08         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_BRIGHT_CODE_
UINT32 SetBrightLevelService(void)
{
    TASK_ARG TaskArg;

    return(RETURN_OK);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SetBrightLevelKey(void)
  Author        : Jeff Wu
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
               Jeff Wu     2013-10-08         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_BRIGHT_CODE_
UINT32 SetBrightLevelKey(void)
{
    UINT32 setMenuKeyVal;
    TASK_ARG TaskArg;
    UINT32  retVal = 0;

    setMenuKeyVal = GetKeyVal();

    //5 Key function adjustment    
     if (gSysConfig.KeyNum == KEY_NUM_5)
     {
         switch(setMenuKeyVal)
         {
         case KEY_VAL_FFW_DOWN:
         case KEY_VAL_FFW_PRESS:
             setMenuKeyVal = KEY_VAL_UP_PRESS;
             break;

         case KEY_VAL_FFD_DOWN:
         case KEY_VAL_FFD_PRESS:
             setMenuKeyVal = KEY_VAL_DOWN_PRESS;
             break;

         default:
             break;
         }
     }

     //6 Key function adjustment     
     if (gSysConfig.KeyNum == KEY_NUM_6)
     {
         switch(setMenuKeyVal)
         {            
         case KEY_VAL_FFW_SHORT_UP:
            setMenuKeyVal = KEY_VAL_ESC_SHORT_UP;
             break;
         default:
             break;
         }
     }

    switch (setMenuKeyVal)
    { 
        case KEY_VAL_FFD_SHORT_UP:
        case KEY_VAL_FFD_PRESS:
            if (curBrightLevel < 4) {
                curBrightLevel++;
            }

            BL_SetLevel(curBrightLevel);
            SendMsg(MSG_BRIGHT_DISPLAY_ALL);
            break;       

        case KEY_VAL_FFW_SHORT_UP:
        case KEY_VAL_FFW_PRESS:
            if (curBrightLevel > 0) {
                curBrightLevel--;
            }

            BL_SetLevel(curBrightLevel);
            SendMsg(MSG_BRIGHT_DISPLAY_ALL);
            break;

        case KEY_VAL_ESC_SHORT_UP: //return up level        
           	BL_SetLevel(SetBackLightLevel);

            WinDestroy(&SetBrightLevelWin);
            SendMsg(MSG_SYS_SET_DISPLAY);
            break; 

        case KEY_VAL_MENU_SHORT_UP:  
        case KEY_VAL_PLAY_SHORT_UP:
            SetBackLightLevel = curBrightLevel;
           	BL_SetLevel(SetBackLightLevel);

            WinDestroy(&SetBrightLevelWin);
            SendMsg(MSG_SYS_SET_DISPLAY);
            break; 

        case KEY_VAL_ESC_PRESS_START:
           	BL_SetLevel(SetBackLightLevel);

            WinDestroy(&SetBrightLevelWin); 
            SendMsg(MSG_SYS_SET_DISPLAY);
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&SetBrightLevelWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&SetBrightLevelWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    } 

    return (retVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void SetBrightLevelDisplay(void)
  Author        : Jeff Wu
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
               Jeff Wu     2013-10-08         Ver1.0
  desc:         
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_BRIGHT_CODE_
void SetBrightLevelDisplay(void)
{
	#if 0
    if (CheckMsg(MSG_NEED_PAINT_ALL) || CheckMsg(MSG_BRIGHT_DISPLAY_ALL))
    {
        LCD_ClrSrc(); 
        DispPictureWithIDNumAndXY(IMG_ID_IMG_SETTINGS_BRIGHT_BASE, 0, 18);

        DisplayMenuStrWithIDNum(SETMENU_TITLE_TXT_X, SETMENU_TITLE_TXT_Y,
                                SETMENU_TITLE_TXT_XSIZE, SETMENU_TITLE_TXT_YSIZE,
                                LCD_TEXTALIGN_CENTER, SID_COMMON_BRIGHTNESS);

        DispPictureWithIDNumAndXY(IMG_ID_IMG_SETTINGS_BRIGHT_GRIP, 12+(curBrightLevel*25), 16);
    }     
#endif		
}

/*
********************************************************************************
*
*                         End of SetBright.c
*
********************************************************************************
*/

#endif

