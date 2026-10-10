/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��o  FunUSB.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             chenfen          2008-3-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_FUNUSB_

#include "SysInclude.h"
#include "driver/AD_KEY/AD_Key.h"   /* KEY_VAL_* event codes */
#include "fsinclude.h"
#include "ImageResourceID.h"

#ifdef _USB_

#include "FunUSB.h"
#include "USBControl.h"
#include "USBConfig.h"
#include "mainmenu.h"


_ATTR_USB_UI_DATA_ UINT16 UsbDataAccessImageID = 0;
_ATTR_USB_UI_DATA_ uint32 UsbDataAccessLastUpdateCounter = 0;
_ATTR_USB_UI_DATA_ UINT8 UsbDataAccessLineCount = 1;


#ifdef _FILE_DEBUG_

#define USB_IDBG_BUFSIZE     1024*60
#define USB_DDBG_BUFSIZE     1024*40

_ATTR_USB_UI_CODE_  char   USBIDgbBuf[USB_IDBG_BUFSIZE];
_ATTR_USB_UI_BSS_   char   USBDDgbBuf[USB_DDBG_BUFSIZE];
_ATTR_USB_UI_DATA_  uint32 UsbDgbBufCnt = 0;
_ATTR_USB_UI_DATA_  HANDLE UsbDgbHandle = -1;

_ATTR_USB_UI_CODE_
int USBDebugHook(char ch)
{
    uint32 cnt = UsbDgbBufCnt % (USB_IDBG_BUFSIZE + USB_DDBG_BUFSIZE);

    if (cnt < USB_IDBG_BUFSIZE)
    {
        USBIDgbBuf[cnt]= ch;
    }
    else if (cnt < (USB_IDBG_BUFSIZE + USB_DDBG_BUFSIZE))
    {
        USBDDgbBuf[cnt - USB_IDBG_BUFSIZE] = ch;
    }
    UsbDgbBufCnt++;

}

_ATTR_USB_UI_CODE_
int USBDebugHookInit(void)
{
    UsbDgbBufCnt = 0;
    memset(USBIDgbBuf, 0x20, USB_IDBG_BUFSIZE);
    memset(USBDDgbBuf, 0x20, USB_DDBG_BUFSIZE);

    pDebugHook = USBDebugHook;
}

_ATTR_USB_UI_CODE_
int USBDebugHookDeInit(void)
{
    uint32 i_cnt = 0;
    uint32 d_cnt = 0;

    pDebugHook = NULL;

    //IntMasterDisable();
    //DisableIntMaster();

    if ((UsbDgbHandle = FileOpenA("\\","USBDEBUGLOG", "WR")) == NOT_OPEN_FILE)//after open file ok,mount disk.
    {
        UsbDgbHandle = FileCreate("\\","USBDEBUGLOG");
    }

    FileSeek(FileInfo[UsbDgbHandle].FileSize, SEEK_SET, UsbDgbHandle);

    if (UsbDgbBufCnt < USB_IDBG_BUFSIZE)
    {
        i_cnt = ((UsbDgbBufCnt+1023)/1024)*1024;        //���д�밴��1024����
        d_cnt = 0;
    }
    else if (UsbDgbBufCnt < (USB_IDBG_BUFSIZE + USB_DDBG_BUFSIZE))
    {
        i_cnt = USB_IDBG_BUFSIZE;
        d_cnt = (UsbDgbBufCnt - USB_IDBG_BUFSIZE);
        d_cnt = ((d_cnt+1023)/1024)*1024;               //���д�밴��1024����
    }
    else
    {
        i_cnt = USB_IDBG_BUFSIZE;
        d_cnt = USB_DDBG_BUFSIZE;
    }

    if (i_cnt > 0)
    {
        FileWrite(USBIDgbBuf, FileInfo[UsbDgbHandle].FileSize, i_cnt, UsbDgbHandle);
    }
    DelayMs(200);
    if (d_cnt > 0)
    {
        FileWrite(USBDDgbBuf, FileInfo[UsbDgbHandle].FileSize, d_cnt, UsbDgbHandle);
    }

    FileClose(UsbDgbHandle);
    UsbDgbHandle = -1;

    //IntMasterEnable();
    //EnableIntMaster();

}

#endif

/*
--------------------------------------------------------------------------------
  Function name : void FunUSBInit(void)
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               chenfen       2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_UI_CODE_
void FunUSBInit(void *pArg)
{

    UINT16 tempBuf[((MENU_ITEM_LENGTH - MENU_CONTENT_OFFSET) >> 1)];

    #ifdef _FILE_DEBUG_
    SysDebugHookDeInit();
    USBDebugHookInit();
    #endif

    DEBUG("Enter");
    if (((USB_WIN_ARG*)pArg)->FunSel == USB_CLASS_TYPE_UPGRADE)
    {
        SendMsg(MSG_MES_FIRMWAREUPGRADE);
        return;
    }

    if (TRUE != ThreadCheck(pMainThread, &USBControlThread))
    {
        ModuleOverlay(MODULE_ID_USBCONTROL, MODULE_OVERLAY_ALL);
        ThreadCreat(&pMainThread, &USBControlThread, pArg);
    }

    DispPictureWithIDNum(IMG_ID_USB_BACKGROUND);       //display usb picture
    DisplayMenuStrWithIDNum(0, 106, 160, 12, LCD_TEXTALIGN_CENTER, SID_CONNECTING_USB);

    SendMsg(MSG_USB_DISPLAY_ALL);
    SendMsg(MSG_USB_DISPLAY_CHARGE);

    SetPowerOffTimerDisable();

}

/*
--------------------------------------------------------------------------------
  Function name : void MainMenuDeInit(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_UI_CODE_
void FunUSBDeInit(void)
{
    SetPowerOffTimerDisable();

    DEBUG("FunUSB Exit");

    #ifdef _FILE_DEBUG_
    USBDebugHookDeInit();
    SysDebugHookInit();
    #endif

    //�γ�USB���˴������ع̼�������Ϣ��Ŀǰdemo�Ƿ��ڸ���ý���ʱ���̼��ļ�����������
    //�û��ɸ���ʵ����Ҫ�Լ��������ʵ�ֱ��ع̼�������
    SendMsg(MSG_SYS_FW_UPGRADE);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 FunUSBService(void)
  Author        : ZHengYongzhi
  Description   : it is USB service handle that is used to handle window message,window service task etc.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_UI_CODE_
UINT32 FunUSBService(void)
{
    uint32  RetVal;
    TASK_ARG TaskArg;

    RetVal = RETURN_OK;

    if (GetMsg(MSG_MES_FIRMWAREUPGRADE))
    {
        DEBUG("Firmware Upgrade...");
        #ifdef _FILE_DEBUG_
        USBDebugHookDeInit();
        #endif
        SysReboot(0x0000008c, 1);
    }

    if (GetMsg(MSG_ENTER_LOADER_USB))
    {
        #ifdef _FILE_DEBUG_
        USBDebugHookDeInit();
        #endif
        SysReboot(0x00000000, 3);
    }

    if (GetMsg(MSG_SYS_REBOOT))
    {
        DEBUG("System Reboot...");
        #ifdef _FILE_DEBUG_
        USBDebugHookDeInit();
        #endif
        SysReboot(0x00000000, 1);
    }

    if(GetMsg(MSG_USB_EXIT_FUSB))
    {
        ThreadDeleteAll(&pMainThread);  //?��?D??��??????��?����?USBcontrol����3?��??����??a????��??a?��D??����a

        TaskArg.Mdb.TaskID = TASK_ID_MAINMENU;
        SendMsg(MSG_FLASH_MEM0_UPDATE);
   		TaskSwitch(TASK_ID_MDB, &TaskArg);

        RetVal = 1;
    }

    if(GetMsg(MSG_PC_DISCONNECT_USB))   //chad.ma add 20160921
    {
        ThreadDeleteAll(&pMainThread);
        TaskArg.MainMenu.MenuID = 0;
        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
        RetVal = 1;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 FunUSBKey(void)
  Author        : ZHengYongzhi
  Description   : monitor the key handle of charge status

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_UI_CODE_
UINT32 FunUSBKey(void)
{
    uint32 RetVal;
    UINT32 MainMenuKeyVal;
    WIN    *pWin;
    TASK_ARG TaskArg;

    RetVal = RETURN_OK;

    MainMenuKeyVal =  GetKeyVal();

    //5 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (MainMenuKeyVal)
        {
            case KEY_VAL_FFD_SHORT_UP:
                MainMenuKeyVal = KEY_VAL_DOWN_SHORT_UP;
                break;

            case KEY_VAL_FFD_PRESS:
                MainMenuKeyVal = KEY_VAL_DOWN_PRESS;
                break;

            case KEY_VAL_FFW_SHORT_UP:
                MainMenuKeyVal = KEY_VAL_UP_SHORT_UP;
                break;

            case KEY_VAL_FFW_PRESS:
                MainMenuKeyVal = KEY_VAL_UP_PRESS;
                break;

            default:
                break;
        }
    }

    //6 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {

    }

    //7 key function
    switch (MainMenuKeyVal)
    {
        case KEY_VAL_MENU_SHORT_UP:
        case KEY_VAL_MENU_PRESS:

            break;

        case KEY_VAL_FFD_SHORT_UP:
        case KEY_VAL_FFW_PRESS:

            break;

        case KEY_VAL_FFW_SHORT_UP:

            break;

        case KEY_VAL_DOWN_SHORT_UP:
        case KEY_VAL_DOWN_PRESS:

            break;

        case KEY_VAL_UP_SHORT_UP:
        case KEY_VAL_UP_PRESS:

            break;

        case KEY_VAL_ESC_SHORT_UP:
            ThreadDeleteAll(&pMainThread);
            TaskArg.MainMenu.MenuID = 0;
	        TaskSwitch(TASK_ID_MAINMENU, &TaskArg);
            RetVal = 1;
            break;

        default:
            break;
    }
    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void FunUSBDisplay(void)
  Author        : ZHengYongzhi
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_USB_UI_CODE_
void FunUSBDisplay(void)
{
    UINT16 ChargingImageID = 0;
    UINT16 UsbDataAccessImageFrameIndex = 0;
    uint32 sysTime = SysTickCounter;
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode;

#ifdef USB_DISPLAY

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_PURPLE);
    LCD_SetBkColor(COLOR_WHITE);

    if (FUSBDevice.StorageStatus)  /* USB update guard time 1500ms */
    {
        //....
    }
    else
    {
        //....
    }

    if (GetMsg(MSG_USB_DISPLAY_ALL))
    {
        DispPictureWithIDNum(IMG_ID_USB_BACKGROUND);       //display usb picture
        //DisplayMenuStrWithIDNum(0, 106, 160, 12, LCD_TEXTALIGN_CENTER, SID_CONNECTING_USB);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);

#endif
}

/*
********************************************************************************
*
*                         End of MainMenu.c
*
********************************************************************************
*/
#endif

