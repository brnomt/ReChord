/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                       All rights reserved.
*
* File Name:  RadioSubFuncWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*                WangBo         2009-4-10        1.0
*    desc:    ORG.
********************************************************************************
*/


#define _IN_RADIOSUBFUNCWIN

#include "SysInclude.h"

#ifdef _RADIO_

#include "RadioSubFuncWin.h"
#include "RadioSubWinInterface.h"
#include "Hold.h"
#include "AudioControl.h"

#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#include "SetBluetooth.h"

extern uint8  BtWinStatus;
#endif


extern FM_GLOBAL* gpRadioplayerRegKey;
extern UINT16  FmFreqMinVal;

#define RadioBatteryLevel       gBattery.Batt_Level

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFuncWinIntDeInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
void RadioSubFuncWinIntDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFuncWinMsgInit(void)
  Author        : WangBo
  Description   : message init

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
void RadioSubFuncWinMsgInit(void)
{
    if ((FALSE == CheckMsg(MSG_RADIOSUBWIN_SETTING_AUTO_PRESET)) &&
        (FALSE == CheckMsg(MSG_RADIOSUBWIN_HOLDONPLAYSAVE)) &&
        (FALSE == CheckMsg(MSG_RADIOSUBWIN_HOME_BACK)))
    {
        SendMsg(MSG_RADIOWIN_DISPLAY_SUB_BK);
        SendMsg(MSG_RADIOWIN_DISPLAY_SUB_SEL);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFuncWinMsgDeInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
void RadioSubFuncWinMsgDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFuncWinInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
              WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
void RadioSubFuncWinInit(void)
{
    RadioSubFuncWinMsgInit();//init message
    RadioSubFuncWinIntInit();//init int
    KeyReset();//init key
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFuncWinDeInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
void RadioSubFuncWinDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN RadioGetFmSaveNum(void)
  Author        : WangBo
  Description   : get valid tune number
                  return ture when find not less than one
  Input         : null
  Return        : null

  History:     <author>         <time>         <version>
                WangBo         2009-5-15       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
BOOLEAN RadioGetFmSaveNum(void)
{
    INT8U i;

    for (i = 0; i < FREQMAXNUMBLE; i++)
    {
        if (gpRadioplayerRegKey->FmFreqArray[i] >= FmFreqMinVal && gpRadioplayerRegKey->FmFreqArray[i] <= FmFreqMaxVal)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioFunctionProc(void)
  Author        : WangBo
  Description   : radio function, include auto search tune, manual search tune,save, delete and preset

  Input         : null
  Return        : null

  History:     <author>         <time>         <version>
                WangBo         2009-5-6       1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
void RadioFunctionProc(void)
{
    UINT32 freq;
    INT8U i;

    switch (FmFunctionSele)
    {
        case FM_OPT_MODE_MANUAL:
        {
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }

#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif
            RadioWinSvcStart(NULL);

            if (RadioPlayerSeekingState() == FM_State_AutoSearch)
            {
                //pause if auto search tune
                gpRadioplayerRegKey->FmState = FM_State_HandSearch;
                freq = gpRadioplayerRegKey->FmFreq;
                gpRadioplayerRegKey->FmFreq = freq;
                gpRadioplayerRegKey->FmState = FM_State_HandSearch;
                gpRadioplayerRegKey->FmSearchMode = FM_SearchModeState_ByHand;
                FM_Process(MSG_FM_START,  NULL);
                //------update fm freq----------------
            }
            else
            {
                gpRadioplayerRegKey->FmState = FM_State_HandStepFreq; // FM_State_HandSearch;
                gpRadioplayerRegKey->FmSearchMode = FM_SearchModeState_ByHand;
            }

            if (FmStandbyFlag == TRUE)
            {
                //ThreadDelete(&pMainThread, &FMThread);//HJ
            }

            WinDestroy(&RadioSubFuncWin);
            SendMsg(MSG_RADIOWIN_DISPLAY_ALL);
            break;
        }
        break;

        case FM_OPT_MODE_AUTOSAVE:
        {
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }

#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif

            if (FALSE == ThreadCheck(pMainThread, &FMThread))
            {
                RadioWinSvcStart(NULL);
            }

            FmStandbyFlag = FALSE;
            SendMsg(MSG_RADIOSUBWIN_DISPLAY_OFF); //HJ
            gpRadioplayerRegKey->PreFmSaveNum = 0xff; //fjp ��һ�ν���Ҫ��ʾƵ��ch
            WinDestroy(&RadioSubFuncWin);
            SendMsg(MSG_RADIOWIN_DISPLAY_ALL);
            SendMsg(MSG_RADIOWIN_DISPLAY_CH);  //��ʾ CH

            if (RadioPlayerSeekingState() == FM_State_AutoSearch)
            {
                ;// WindowDestroy(gpMainListBoxWin);
            }
            else
            {
                gpRadioplayerRegKey->FmState = FM_State_AutoSearch;
                gpRadioplayerRegKey->FmState = FM_State_AutoSearch;
                gpRadioplayerRegKey->FmSearchMode = FM_SearchModeState_Auto;
                gpRadioplayerRegKey->FmSaveNum = 0;    // ��ͷ��ʼ�洢��̨
                gpRadioplayerRegKey->FmFreq = FmFreqMinVal;

                for (i = 0; i < FREQMAXNUMBLE; i++)
                {
                    gpRadioplayerRegKey->FmFreqArray[i] = 0;
                }

                SendMsg(MSG_RADIOWIN_DISPLAY_SCAN);
                SendMsg(MSG_FM_AUTOSEARCH);
                //FM_Process(MSG_FM_AutoSearch,(void *)1);
            }

            break;
        }

        case FM_OPT_MODE_SAVE:
        {
            if (TRUE == ThreadCheck(pMainThread, &FMThread))
            {
                RadioRefreshFreq();
                FmListSaveAndDel = 1;
                WinDestroy(&RadioSubFuncWin);
                ClearMsg(MSG_BATTERY_UPDATE);  //avoid battery icon show when enter preset edit screen
                SendMsg(MSG_RADIOSUBWIN_DISPLAY_FLAG);  //send message to radio window, after displaying window, enter fm freq sub window
            }
            else
            {
                SendMsg(MSG_RADIOSUBWIN_DISPLAY_CANNOT_CONTINUE);
            }
        }
        break;

        case FM_OPT_MODE_PRESET:
        {
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }

#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif
            RadioWinSvcStart(NULL);

            //-----------if no find tune ,create dialog----------------------
            //if((0 == gpRadioplayerRegKey->FmSaveNum) && (gpRadioplayerRegKey->FmState != FM_State_StepStation))
            if (RadioGetFmSaveNum() == FALSE)
            {
                //create dialog,but not save tune
                gpRadioplayerRegKey->FmState = FM_State_HandStepFreq;
                WinDestroy(&RadioSubFuncWin);
                SendMsg(MSG_RADIOWIN_DISPLAY_ALL);
            }
            else
            {
                if (RadioPlayerSeekingState() == FM_State_StepStation)
                {
                    ;// WindowDestroy(gpMainListBoxWin);
                }
                else
                {
                    gpRadioplayerRegKey->PreFmSaveNum = 0xff; //fjp
                    gpRadioplayerRegKey->FmState = FM_State_StepStation;
                    gpRadioplayerRegKey->FmState = FM_State_StepStation;
                    gpRadioplayerRegKey->FmSearchMode = FM_SearchModeState_Auto;
                    FmFreqNum = gpRadioplayerRegKey->FmSaveNum;
                    gpRadioplayerRegKey->FmFreq = gpRadioplayerRegKey->FmFreqArray[ gpRadioplayerRegKey->FmSaveNum];

                    // search next preset when P01 is empty
                    if ((gpRadioplayerRegKey->FmFreq < FmFreqMinVal) || (gpRadioplayerRegKey->FmFreq > FmFreqMaxVal))
                    {
                        RadioSetStepStation(FM_DIRECT_INC);
                    }

                    //if(FmStandbyFlag == FALSE)
                    {
                        //DEBUG();
                        FM_Process(MSG_FM_START,  (void *)1);
                    }
                }

                if (FmStandbyFlag == TRUE)
                {
                    //FmStandbyFlag = FALSE;
                    //SendMsg(MSG_RADIOSUBWIN_DISPLAY_OFF); //HJ
                    //ThreadDelete(&pMainThread, &FMThread);//HJ
                }

                WinDestroy(&RadioSubFuncWin);
                RadioGetGuaVal();
                SendMsg(MSG_RADIOWIN_DISPLAY_ALL);
                SendMsg(MSG_RADIOWIN_DISPLAY_CH);  //display CH
            }
        }
        break;

        case FM_OPT_MODE_DELETE:
        {
            if (TRUE == ThreadCheck(pMainThread, &FMThread))
            {
                RadioRefreshFreq();
                FmListSaveAndDel = 0;
                WinDestroy(&RadioSubFuncWin);
                ClearMsg(MSG_BATTERY_UPDATE);  //avoid battery icon show when enter preset edit screen
                SendMsg(MSG_RADIOSUBWIN_DISPLAY_FLAG);
            }
            else
            {
                SendMsg(MSG_RADIOSUBWIN_DISPLAY_CANNOT_CONTINUE);
            }

            break;
        }
        break;
//-----------------------------------------------------------------------------
#ifdef _RECORD_
        case FM_OPT_MODE_REC:
        {
            DelayMs(150);
            WinDestroy(&RadioSubFuncWin);
            ClearMsg(MSG_BATTERY_UPDATE);
            SendMsg(MSG_RADIOSUBWIN_REC_OR_DELETE_REC);

            if (TRUE == ThreadCheck(pMainThread, &FMThread))
            {
                if (fm_is_stop == 0)
                {
                    SendMsg(MSG_RADIOSUBWIN_DISPLAY_REC);
                }
                else
                {
                    SendMsg(MSG_RADIOSUBWIN_DISPLAY_CANNOT_CONTINUE);
                }
            }
            else
            {
                SendMsg(MSG_RADIOSUBWIN_DISPLAY_CANNOT_CONTINUE);
            }

            break;
        }
        break;

        case FM_OPT_MODE_PLAY_REC_DATA:
        {
            DelayMs(150);
            WinDestroy(&RadioSubFuncWin);
            ClearMsg(MSG_BATTERY_UPDATE);
            SendMsg(MSG_RADIOSUBWIN_REC_OR_DELETE_REC);
            SendMsg(MSG_RADIOSUBWIN_DISPLAY_PLAY_RECDATA);
            break;
        }
        break;

        case FM_OPT_MODE_DELETE_REC_DATA:
        {
            WinDestroy(&RadioSubFuncWin);
            ClearMsg(MSG_BATTERY_UPDATE);
            SendMsg(MSG_RADIOSUBWIN_REC_OR_DELETE_REC);
            SendMsg(MSG_RADIOSUBWIN_DISPLAY_DELETE_RECDATA);
            break;
        }
        break;
#endif

//-----------------------------------------------------------------------------
        default:
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioItemUpProc(UINT16 uiUpdateType)

  Author        : anzhiguo
  Description   : it is the handle child progarm of media libary module

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
void RadioItemUpProc(UINT16 uiUpdateType)
{
    FMBRO_STRUCT  *pBro;

    UINT16  StartItem = 0;
    UINT16  i;

    if (FmFunctionSele != 0)                     //ZHU TO AVOID WHEN THE SELE =0 KEYCOUNTER =1
        StartItem = FmFunctionSele - FmKeyCounter;
    else
        StartItem = 0;

    switch (uiUpdateType)
    {
        case UPDATE_ALL: //ALL_BROITEM_UPDATE:
            for (i = 0; i < RADIO_SCREEN_PER_LINE; i++) //clear the items that is diaplay
            {
                FMBroItem[i].ItemSidNumber = 0xFFFF;
            }

            pFMBro = &FMBroItem[0];
            pBro = pFMBro;

            for (i = 0; (i < RADIO_SCREEN_PER_LINE) && ((i + StartItem) <= FM_OPT_MODE_MAX); i++)
            {
                pBro->ItemSidNumber = StartItem + i;

                //DEBUG("FmFunctionSele= %d ItemSidNumber = %d \n",FmFunctionSele,pBro->ItemSidNumber);

                if (i >= (RADIO_SCREEN_PER_LINE - 1))
                    break;

                pBro = pBro->pNext;
            }

            break;

        case UPDATE_UP: //UP_UPDATE:
            pFMBro = pFMBro->pPrev;
            pBro = pFMBro;
            pBro->ItemSidNumber = StartItem;
            break;

        case UPDATE_DOWN: //DOWN_UPDATE:
            pBro = pFMBro;
            pBro->ItemSidNumber = FmFunctionSele;
            pFMBro = pFMBro->pNext;
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFuncWinIntInit(void)
  Author        : WangBo
  Description   : int initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
void RadioSubFuncWinIntInit(void)
{
    unsigned int i;

    for (i = 0; i < RADIO_SCREEN_PER_LINE; i++)
    {
        FMBroItem[i].ItemSidNumber = 0xFFFF;
    }

    for (i = 0; i < RADIO_SCREEN_PER_LINE - 1; i++)
    {
        FMBroItem[i].pNext = &FMBroItem[i + 1];
    }

    for (i = 1; i < RADIO_SCREEN_PER_LINE; i++)
    {
        FMBroItem[i].pPrev = &FMBroItem[i - 1];
    }

    FMBroItem[0].pPrev = &FMBroItem[RADIO_SCREEN_PER_LINE - 1];
    FMBroItem[RADIO_SCREEN_PER_LINE - 1].pNext = &FMBroItem[0];
    pFMBro = &FMBroItem[0];
    //MusicDirTreeInfo.MusicDirTotalItem = 0;
    //PreFmKeyCounter  = 0;
    //FmKeyCounter = 0;

    //avoid FM menu display abnormally after first download
    if ((FmFunctionSele > FM_OPT_MODE_MAX) || (FmFunctionSele < FM_OPT_MODE_MANUAL))
    {
        FmFunctionSele = FM_OPT_MODE_MANUAL;
    }

//    if (FmFunctionSele != 0)
//        FmFunctionSele = 0;
//
//    PreFmKeyCounter  = 0;
//    FmKeyCounter = 0;
    RadioItemUpProc(UPDATE_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 RadioSubFuncWinService(void)
  Author        : WangBo
  Description   : it is radio service handle that is used to handle window message,window service task etc.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
UINT32 RadioSubFuncWinService(void)
{
    INT16 Retval = 0;
    TASK_ARG TaskArg;

    if (TRUE == GetMsg(MSG_RADIOSUBWIN_SETTING_AUTO_PRESET))
    {
        FmFunctionSele = FM_OPT_MODE_AUTOSAVE;
        RadioFunctionProc();
    }

    if (TRUE == GetMsg(MSG_RADIOSUBWIN_HOLDONPLAYSAVE))
    {
        if (gSysConfig.RadioConfig.HoldOnPlaySaveFlag == 2)
        {
            FmFunctionSele = FM_OPT_MODE_PRESET;
            RadioFunctionProc();
        }
        else
        {
            FmFunctionSele = FM_OPT_MODE_MANUAL;
            RadioFunctionProc();
        }
    }

    if (TRUE == GetMsg(MSG_RADIOSUBWIN_HOME_BACK))
    {
        if (gpRadioplayerRegKey->FmState == FM_State_StepStation)
        {
            FmFunctionSele = FM_OPT_MODE_PRESET;
            RadioFunctionProc();
        }
        else if (gpRadioplayerRegKey->FmState == FM_State_HandStepFreq)
        {
            FmFunctionSele = FM_OPT_MODE_MANUAL;
            RadioFunctionProc();
        }
    }

    return (Retval);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 RadioSubFuncWinKey(void)
  Author        : WangBo
  Description   : FM child window key handle program

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
UINT32 RadioSubFuncWinKey(void)
{
    UINT32 RadioSubMenKeyVal;
    INT32  RetVal = 0;
    TASK_ARG TaskArg;

    RadioSubMenKeyVal = GetKeyVal();

    switch (RadioSubMenKeyVal)
    {
        case KEY_VAL_MENU_SHORT_UP: //MENU���Ĵ�������ѡ����Ӧ�Ĺ���
        {
            RadioFunctionProc();
            break;
        }

        case KEY_VAL_ESC_SHORT_UP:
        case KEY_VAL_ESC_PRESS_START:
        {
            WinDestroy(&RadioSubFuncWin);
            ClearMsg(MSG_BATTERY_UPDATE);
            SendMsg(MSG_RADIOWIN_DISPLAY_EXIT_FM_MENU);
            RetVal = 1;
            break;
        }

        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_SHORT_UP:
        {
            PreFmKeyCounter = FmKeyCounter;

            if (FmFunctionSele > 0)
            {
                FmFunctionSele--;

                if (FmKeyCounter == 0)
                {
                    RadioItemUpProc(UPDATE_UP); //up update
                    SendMsg(MSG_RADIOWIN_DISPLAY_SUB_BK);
                }
                else
                {
                    FmKeyCounter--;
                    SendMsg(MSG_RADIOWIN_DISPLAY_SUB_SEL);
                }
            }
            else                //zhu to repair when the sele == 0
            {
                FmFunctionSele = FM_OPT_MODE_MAX - 1;

                if (FmKeyCounter + 1 > RADIO_SCREEN_PER_LINE)
                {
                    if ((FmKeyCounter + 1) % RADIO_SCREEN_PER_LINE)
                    {
                        FmKeyCounter = (FmKeyCounter + 1) % RADIO_SCREEN_PER_LINE - 1;
                    }
                    else
                    {
                        FmKeyCounter =  RADIO_SCREEN_PER_LINE - 1;
                    }

                    SendMsg(MSG_BROW_DIS_ALL_ITEM);
                }
                else
                {
                    FmKeyCounter = FM_OPT_MODE_MAX - 1;
                    SendMsg(MSG_RADIOWIN_DISPLAY_SUB_BK);
                }
            }

            break;
        }

        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
        {
            PreFmKeyCounter = FmKeyCounter;

            if (FmFunctionSele < FM_OPT_MODE_MAX)
            {
                FmFunctionSele++;

                if (FmFunctionSele == FM_OPT_MODE_MAX)
                {
                    FmFunctionSele = 0;
                    FmKeyCounter = 0;
                    SendMsg(MSG_RADIOWIN_DISPLAY_SUB_BK);
                    break;
                }

                if (FmKeyCounter >= (RADIO_SCREEN_PER_LINE - 1))
                {
                    RadioItemUpProc(UPDATE_DOWN); //down update
                    SendMsg(MSG_RADIOWIN_DISPLAY_SUB_BK);
                }
                else
                {
                    FmKeyCounter++;
                    SendMsg(MSG_RADIOWIN_DISPLAY_SUB_SEL);
                }
            }

            break;
        }

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&RadioSubFuncWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&RadioSubFuncWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFuncWinDisplay(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFUNCWIN_CODE_
void RadioSubFuncWinDisplay(void)
{
    UINT16      temp;
    UINT16      TempColor, TempBkColor, TempTxtMode, TempCharSize;
    UINT16      k, i;
    PICTURE_INFO_STRUCT     PicInfo;
    LCD_RECT      r;
    FMBRO_STRUCT  *pBro;
    pBro = pFMBro;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if ((TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_SUB_BK)) || (TRUE == GetMsg(MSG_NEED_PAINT_ALL)) )
    {
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);
        DisplayMenuStrWithIDNum(RADIO_SUB_MENU_TXT_X, RADIO_SUB_MENU_TXT_Y, RADIO_SUB_MENU_TXT_XSIZE,
                                RADIO_SUB_MENU_TXT_YSIZE, LCD_TEXTALIGN_CENTER, SID_RADIO );
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + gBattery.Batt_Level, 105, 146);
        SendMsg(MSG_RADIOWIN_DISPLAY_SUB_SEL);

        for (i = 0; i < RADIO_SCREEN_PER_LINE; i++)
        {
            if (i == FmKeyCounter)
            {
                if (pBro->ItemSidNumber != 0xFFFF)
                    DisplayMenuStrWithIDNum(10, (i * 17) + 4, 128, 12, LCD_TEXTALIGN_LEFT, (SID_MANUAL + pBro->ItemSidNumber));
            }
            else
            {
                if (pBro->ItemSidNumber != 0xFFFF)
                    DisplayMenuStrWithIDNum(10, (i * 17) + 4, 128, 12, LCD_TEXTALIGN_LEFT, (SID_MANUAL + pBro->ItemSidNumber));
            }
            pBro = pBro->pNext;
        }
        SendMsg(MSG_RADIOWIN_DISPLAY_SUB_SCROLL);
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_SUB_SEL))
    {
        for (i = 0; i < RADIO_SCREEN_PER_LINE; i++)
        {
            if ((i == FmKeyCounter) || (i == PreFmKeyCounter))
            {
                if (i == FmKeyCounter)
                {
                    DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON, 10, 3 + i * 17);

                    if (pBro->ItemSidNumber != 0xFFFF)
                        DisplayMenuStrWithIDNum(10, (i * 17) + 4, 128, 12, LCD_TEXTALIGN_LEFT, (SID_MANUAL + pBro->ItemSidNumber));
                }
                else if (i == PreFmKeyCounter)
                {
                    DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, 3 + i * 17, 17);

                    if (pBro->ItemSidNumber != 0xFFFF)
                        DisplayMenuStrWithIDNum(10, (i * 17) + 4, 128, 12, LCD_TEXTALIGN_LEFT, (SID_MANUAL + pBro->ItemSidNumber));
                }
            }

            pBro = pBro->pNext;
        }

        SendMsg(MSG_RADIOWIN_DISPLAY_SUB_SCROLL);
    }

    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + gBattery.Batt_Level, 105, 146);
    }

    LCD_SetTextMode(TempTxtMode);
    LCD_SetBkColor(TempBkColor);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
}

/*
********************************************************************************
*
*                         End of Dialogbox.c
*
********************************************************************************
*/
#endif

