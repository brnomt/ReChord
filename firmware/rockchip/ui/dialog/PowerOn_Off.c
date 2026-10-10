/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��   PowerOn_Off.c
*
* Description:   C program template
*
* History:      <author>          <time>        <version>
*                anzhiguo      2009-2-27         1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_POWER_ON_OFF

#include "SysInclude.h"
#include "fsinclude.h"


/*
--------------------------------------------------------------------------------
  Function name : void DisplayPowerOnFlash(void)
  Author        : ZHengYongzhi
  Description   : display power on picture.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef _FRAME_BUFFER_
extern _ATTR_LCD_CODE_ void Lcd_BuferTranfer (void);
#endif

_ATTR_SYS_INIT_CODE_
uint8 DisplayPowerOnFlash(void)
{
    uint32 i, j;
    uint32 block;
    uint8  RetVal;

    RetVal = OK;
    block = 0;

    LCD_SetDiaplayMode(LCD_MODE_0);

#ifdef _SPINOR_
    gSysConfig.Memory = FLASH0;
    RetVal = FileSysSetup(gSysConfig.Memory);
#endif


    for(i = 0; i < 24; i++)
    {
#ifdef _SPINOR_
        if(RetVal == OK)
            DispPictureWithIDNum(IMG_ID_POWERON0 + i);
#else   //_EMMC_
        DispPictureWithIDNum(IMG_ID_POWERON0 + i);
#endif

#ifdef _FRAME_BUFFER_
        Lcd_BuferTranfer();
#endif

        //DelayMs(50);
    }
    return(RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void PowerOff(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             anzhiguo        2009-2-27         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void PowerOff(void)
{
    uint32 i, counter;

    printf("\n system powerdown!");
    #ifdef _USB_HOST_
    USBHost_Reinit();
    #endif

    #ifndef _FRAME_BUFFER_
    MP4_LCD_DeInit();
    #endif

    if(gSysSetBLModeArray[gSysConfig.BLmode] == BL_DARK)
    {
        LcdWakeUp();
        BL_On();
    }

    for(i = 0; i < 12; i++)
    {
        DispPictureWithIDNum(IMG_ID_POWEROFF0 + i);
#ifdef _FRAME_BUFFER_
        Lcd_BuferTranfer();
#endif
        DelayMs(100);
    }

    BL_Off();
    LcdStandby();
    AdcPowerDown();

    Codec_DeInitial();

    #ifdef _FILE_DEBUG_
    SysDebugHookDeInit();
    #endif

#ifdef _WATCH_DOG_
    WatchDogDisableInt();
    WatchDogDeInit();
#endif

    MDDeInitAll();
    DelayMs(100);

    System_Power_On(0);

    //wait for power key up
    do
    {
        DelayMs(10);
    }
    while(Get_PlayKey_State());

    //wait for power key press
    counter = 0;
    while(1)
    {
        if (Get_PlayKey_State())
        {
            if (++counter > 10)
            {
                SysReboot(0,0);
                while(1);
            }
        }
        else
        {
            counter = 0;
        }
    }
    while(1);
}

/*
********************************************************************************
*
*                         End of PowerOn_Off.c
*
********************************************************************************
*/
