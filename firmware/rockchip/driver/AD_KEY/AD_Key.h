/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  ADKEY.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             anzhiguo      2009-2-10          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _ADKEY_H
#define _ADKEY_H

#undef  EXT
#ifdef _IN_ADKEY_
#define EXT
#else
#define EXT extern
#endif

/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                 */
/*                                                                            */
/******************************************************************************/


//------------------------------------------------------------------------------
//Key Time Define
#define KEY_TIME_DOWN               2 // 4  // added by chenz
#define KEY_TIME_PRESS_START        20  //40  //added by chenz
#define KEY_TIME_PRESS_STEP         2   // 8  //added by chenz
              
//------------------------------------------------------------------------------
//Key Status Define
#define KEY_STATUS_NONE             ((UINT32)0xffffffff)

#define KEY_STATUS_DOWN             ((UINT32)0x0001 << 28)      //0x1000,0000
#define KEY_STATUS_PRESS_START      ((UINT32)0x0002 << 28)      //0x2000,0000
#define KEY_STATUS_PRESS            ((UINT32)0x0003 << 28)      //0x3000,0000
#define KEY_STATUS_SHORT_UP         ((UINT32)0x0004 << 28)      //0x4000,0000
#define KEY_STATUS_LONG_UP          ((UINT32)0x0005 << 28)      //0x5000,0000

#define KEY_VAL_HOLD_ON             (((UINT32)0x0001 << 24) | (KEY_STATUS_SHORT_UP))
#define KEY_VAL_HOLD_OFF            (((UINT32)0x0001 << 25) | (KEY_STATUS_SHORT_UP))

#define KEY_VAL_PLAY_DOWN           ((KEY_VAL_PLAY)|(KEY_STATUS_DOWN))
#define KEY_VAL_PLAY_PRESS_START    ((KEY_VAL_PLAY)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_PLAY_PRESS          ((KEY_VAL_PLAY)|(KEY_STATUS_PRESS))
#define KEY_VAL_PLAY_SHORT_UP       ((KEY_VAL_PLAY)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_PLAY_LONG_UP        ((KEY_VAL_PLAY)|(KEY_STATUS_LONG_UP))
                                    
#define KEY_VAL_UP_DOWN            ((KEY_VAL_UP)|(KEY_STATUS_DOWN))
#define KEY_VAL_UP_PRESS_START     ((KEY_VAL_UP)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_UP_PRESS           ((KEY_VAL_UP)|(KEY_STATUS_PRESS))
#define KEY_VAL_UP_SHORT_UP        ((KEY_VAL_UP)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_UP_LONG_UP         ((KEY_VAL_UP)|(KEY_STATUS_LONG_UP))

#define KEY_VAL_DOWN_DOWN            ((KEY_VAL_DOWN)|(KEY_STATUS_DOWN))           
#define KEY_VAL_DOWN_PRESS_START     ((KEY_VAL_DOWN)|(KEY_STATUS_PRESS_START))    
#define KEY_VAL_DOWN_PRESS           ((KEY_VAL_DOWN)|(KEY_STATUS_PRESS))          
#define KEY_VAL_DOWN_SHORT_UP        ((KEY_VAL_DOWN)|(KEY_STATUS_SHORT_UP))       
#define KEY_VAL_DOWN_LONG_UP         ((KEY_VAL_DOWN)|(KEY_STATUS_LONG_UP))        

#define KEY_VAL_ESC_DOWN           ((KEY_VAL_ESC)|(KEY_STATUS_DOWN))           
#define KEY_VAL_ESC_PRESS_START    ((KEY_VAL_ESC)|(KEY_STATUS_PRESS_START))    
#define KEY_VAL_ESC_PRESS          ((KEY_VAL_ESC)|(KEY_STATUS_PRESS))          
#define KEY_VAL_ESC_SHORT_UP       ((KEY_VAL_ESC)|(KEY_STATUS_SHORT_UP))       
#define KEY_VAL_ESC_LONG_UP        ((KEY_VAL_ESC)|(KEY_STATUS_LONG_UP))        

#define KEY_VAL_MENU_DOWN           ((KEY_VAL_MENU)|(KEY_STATUS_DOWN))           
#define KEY_VAL_MENU_PRESS_START    ((KEY_VAL_MENU)|(KEY_STATUS_PRESS_START))    
#define KEY_VAL_MENU_PRESS          ((KEY_VAL_MENU)|(KEY_STATUS_PRESS))          
#define KEY_VAL_MENU_SHORT_UP       ((KEY_VAL_MENU)|(KEY_STATUS_SHORT_UP))       
#define KEY_VAL_MENU_LONG_UP        ((KEY_VAL_MENU)|(KEY_STATUS_LONG_UP))        

#define KEY_VAL_FFW_DOWN            ((KEY_VAL_FFW)|(KEY_STATUS_DOWN))           
#define KEY_VAL_FFW_PRESS_START     ((KEY_VAL_FFW)|(KEY_STATUS_PRESS_START))    
#define KEY_VAL_FFW_PRESS           ((KEY_VAL_FFW)|(KEY_STATUS_PRESS))          
#define KEY_VAL_FFW_SHORT_UP        ((KEY_VAL_FFW)|(KEY_STATUS_SHORT_UP))       
#define KEY_VAL_FFW_LONG_UP         ((KEY_VAL_FFW)|(KEY_STATUS_LONG_UP))        

#define KEY_VAL_FFD_DOWN            ((KEY_VAL_FFD)|(KEY_STATUS_DOWN))           
#define KEY_VAL_FFD_PRESS_START     ((KEY_VAL_FFD)|(KEY_STATUS_PRESS_START))    
#define KEY_VAL_FFD_PRESS           ((KEY_VAL_FFD)|(KEY_STATUS_PRESS))          
#define KEY_VAL_FFD_SHORT_UP        ((KEY_VAL_FFD)|(KEY_STATUS_SHORT_UP))       
#define KEY_VAL_FFD_LONG_UP         ((KEY_VAL_FFD)|(KEY_STATUS_LONG_UP))        

#define KEY_VAL_VOL_DOWN            ((KEY_VAL_VOL)|(KEY_STATUS_DOWN))           
#define KEY_VAL_VOL_PRESS_START     ((KEY_VAL_VOL)|(KEY_STATUS_PRESS_START))    
#define KEY_VAL_VOL_PRESS           ((KEY_VAL_VOL)|(KEY_STATUS_PRESS))          
#define KEY_VAL_VOL_SHORT_UP        ((KEY_VAL_VOL)|(KEY_STATUS_SHORT_UP))       
#define KEY_VAL_VOL_LONG_UP         ((KEY_VAL_VOL)|(KEY_STATUS_LONG_UP))        

//hold 是memu 和play 的长按
#define KEY_VAL_HOLD_DOWN           ((KEY_VAL_HOLD)|(KEY_STATUS_DOWN))           
#define KEY_VAL_HOLD_PRESS_START    ((KEY_VAL_HOLD)|(KEY_STATUS_PRESS_START))    
#define KEY_VAL_HOLD_PRESS          ((KEY_VAL_HOLD)|(KEY_STATUS_PRESS))          
#define KEY_VAL_HOLD_SHORT_UP       ((KEY_VAL_HOLD)|(KEY_STATUS_SHORT_UP))       
#define KEY_VAL_HOLD_LONG_UP        ((KEY_VAL_HOLD)|(KEY_STATUS_LONG_UP)) 

#define KEY_VAL_UNHOLD_DOWN           ((KEY_VAL_UNHOLD)|(KEY_STATUS_DOWN))           
#define KEY_VAL_UNHOLD_PRESS_START    ((KEY_VAL_UNHOLD)|(KEY_STATUS_PRESS_START))    
#define KEY_VAL_UNHOLD_PRESS          ((KEY_VAL_UNHOLD)|(KEY_STATUS_PRESS))          
#define KEY_VAL_UNHOLD_SHORT_UP       ((KEY_VAL_UNHOLD)|(KEY_STATUS_SHORT_UP))       
#define KEY_VAL_UNHOLD_LONG_UP        ((KEY_VAL_UNHOLD)|(KEY_STATUS_LONG_UP)) 

#define KEY_VAL_POWER_DOWN          (KEY_VAL_PLAY_PRESS)

/******************************************************************************/
/*                                                                            */
/*                          Struct Define                                     */
/*                                                                            */
/******************************************************************************/
typedef union 
{
    struct 
    {       
        unsigned int    bKeyHave:            1,     //flag key is valid
                        bKeyDown:            1,     //set active if any key down.
                        bKeyHold:            1,     //flag hold status.
                        bReserved:           29;
    }bc;
    unsigned int word;
} KEY_FLAG;

/******************************************************************************/
/*                                                                            */
/*                          Variable Define                                   */
/*                                                                            */
/******************************************************************************/
//extern SYSTICK_LIST AdcKey_SysTimer;

_ATTR_DRIVER_BSS_          EXT UINT32              KeyVal;
_ATTR_DRIVER_BSS_          EXT UINT32              KeyScanVal;
_ATTR_DRIVER_BSS_          EXT UINT32              PrevKeyVal;
_ATTR_DRIVER_BSS_          EXT UINT32              KeyStatus;
_ATTR_DRIVER_BSS_          EXT UINT32              KeyScanCnt;
_ATTR_DRIVER_BSS_          EXT UINT32              KeyScanCounter;
_ATTR_DRIVER_BSS_          EXT KEY_FLAG            KeyFlag;
_ATTR_DRIVER_BSS_          EXT UINT32              KeyVolAdjustCounter;
_ATTR_SYS_DATA_            EXT UINT32              KeyLockCount;
_ATTR_SYS_DATA_            EXT UINT32              KeyUnLockCount;

/******************************************************************************/
/*                                                                            */
/*                         Function Declare                                   */
/*                                                                            */
/******************************************************************************/
EXT void KeyInit(void);
EXT void KeyReset(void);

EXT uint32 Get_PlayKey_State(void);
EXT UINT32 GetKey(UINT32 *pKeyVal);
EXT void KeyScan(void);
EXT UINT32 GetKeyVal(void);
EXT void KeyScanDisable(void);
EXT void KeyScanEnable(void);

EXT void KeyVolAdjStart(void);


/*
********************************************************************************
*
*                         End of main.c
*
********************************************************************************
*/
#endif

