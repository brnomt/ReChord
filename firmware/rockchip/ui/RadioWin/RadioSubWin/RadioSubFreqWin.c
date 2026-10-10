/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                       All rights reserved.
*
* File Name:  RadioSubFreqWin.c
*
* Description:
*
* History:      <author>          <time>        <version>
*                WangBo         2009-4-10        1.0
*    desc:    ORG.
********************************************************************************
*/


#define _IN_RADIOSUBFREQWIN

#include "SysInclude.h"

#ifdef _RADIO_

#include "RadioSubFreqWin.h"
#include "RadioSubWinInterface.h"
#include "Hold.h"

extern FM_GLOBAL* gpRadioplayerRegKey;

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFreqWinIntInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
void RadioSubFreqWinIntInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFreqWinIntDeInit(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
void RadioSubFreqWinIntDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFreqWinMsgInit(void)
  Author        : WangBo
  Description   : message initial function,to initial setting the message parameter that will be used.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
void RadioSubFreqWinMsgInit(void)
{
    SendMsg(MSG_RADIOWIN_DISPLAY_SUB_LIST_BK);
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFreqWinMsgDeInit(void)
  Author        : WangBo
  Description   : message auti-initial function.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
void RadioSubFreqWinMsgDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFreqWinInit(void)
  Author        : WangBo
  Description   : child window initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
void RadioSubFreqWinInit(void)
{
    RadioSubFreqWinMsgInit();//message initial
    RadioSubFreqWinIntInit();//interrupt initial
    KeyReset();//key initial
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFreqWinDeInit(void)
  Author        : WangBo
  Description   : window auti-initial function

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
void RadioSubFreqWinDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 RadioSubFreqWinService(void)
  Author        : WangBo
  Description   : child window service progarm it is use to handle window message, window service etc.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
UINT32 RadioSubFreqWinService(void)
{
    INT32 Retval = 0;
    TASK_ARG TaskArg;

    return(Retval);
}


/*
--------------------------------------------------------------------------------
  Function name : void RadioFreq2Ascii(void)
  Author        : WangBo
  Description   : turn frequency to ASCII code,get output string

  Input         : ch1:current frequency id(0~39),freq:current frequeny,pDispBuf:output string pointer.
  Return        :

  History:     <author>         <time>         <version>
                WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
void RadioFreq2Ascii(UINT16 chl, UINT16 freq, UINT16 *pDispBuf)
{
    pDispBuf[0] = 'C';
    pDispBuf[1] = 'H';
    pDispBuf[2] = (chl+1)/10 + '0';
    pDispBuf[3] = (chl+1)%10 + '0';
    pDispBuf[4] = ':';
    pDispBuf[5] = ' ';
    pDispBuf[8] = '.';
    pDispBuf[10] = 'M';
    pDispBuf[11] = 'H';
    pDispBuf[12] = 'z';
    pDispBuf[13] = 0;

    if (freq != 0)
    {
        pDispBuf[5]  = freq /10000;              //����:105.8MHZ
        pDispBuf[5] += (pDispBuf[5]?'0':' ');
        pDispBuf[6]  = (freq%10000)/1000 + '0';
        pDispBuf[7]  = (freq%1000)/100 + '0';

        pDispBuf[9]  = (freq%100)/10   + '0';
    }
    else
    {
        pDispBuf[6]  = 'E';
        pDispBuf[7]  = 'm';
        pDispBuf[8]  = 'p';
        pDispBuf[9]  = 't';
        pDispBuf[10] = 'y';
        pDispBuf[11] = 0;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 RadioSubFreqWinKey(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
UINT32 RadioSubFreqWinKey(void)
{
    INT32  RetVal = 0;
    INT32  KeyValTemp;
    TASK_ARG TaskArg;

    PrevChannel = CurrentChannel;

    KeyValTemp = GetKeyVal();
/*
    //5 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
       switch(KeyValTemp)
       {
           case KEY_VAL_FFW_DOWN:
           case KEY_VAL_FFW_PRESS:
               KeyValTemp = KEY_VAL_UP_DOWN;
               break;

           case KEY_VAL_FFD_DOWN:
           case KEY_VAL_FFD_PRESS:
               KeyValTemp = KEY_VAL_DOWN_DOWN;
               break;

           default:
               break;
       }
    }

    //6 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
       switch(KeyValTemp)
       {
           case KEY_VAL_FFW_SHORT_UP:
               KeyValTemp = KEY_VAL_ESC_SHORT_UP;
               break;

           default:
               break;
       }
    }
*/
    switch (KeyValTemp)
    {
        case KEY_VAL_ESC_SHORT_UP: //�������ɾ����Ӧ��Ƶ�ʺ��˳�
        {
            RadioRefreshFreq();  //�ȸ���Ƶ���б�
            WinDestroy(&RadioSubFreqWin);
            KeyReset();
            SendMsg(MSG_RADIOSUBWIN_DISPLAY_FM_MENU);
            RetVal = 1;
            break;
        }

        case KEY_VAL_ESC_PRESS_START:
        {
            RadioRefreshFreq();
            WinDestroy(&RadioSubFreqWin);
            KeyReset();
            SendMsg(MSG_RADIOWIN_DISPLAY_EXIT_FM_MENU);
            RetVal = 1;
            break;
        }

        case KEY_VAL_PLAY_SHORT_UP:
        {
            WinDestroy(&RadioSubFreqWin);
            KeyReset();

            ClearMsg(MSG_RADIOSUBWIN_DISPLAY_FLAG);
            SendMsg(MSG_RADIOSUBWIN_DIALOG);

            RetVal = 1;   //�򲻶ϸ�����
            break;
        }

        case KEY_VAL_FFW_PRESS:
        case KEY_VAL_FFW_DOWN:
        {
            if (CurrentChannel > 0)
            {
                CurrentChannel --;
            }
            else
            {
                CurrentChannel = FM_SAVE_MAX_COUNTER-1;
            }

            SendMsg(MSG_RADIOWIN_DISPLAY_SUB_LIST_SEL);
            RetVal=0;
            break;
        }

        case KEY_VAL_FFD_PRESS:
        case KEY_VAL_FFD_DOWN:
        {
            if (CurrentChannel < FM_SAVE_MAX_COUNTER-1)
            {
                CurrentChannel++;
            }
            else
            {
                CurrentChannel = 0;
            }

            SendMsg(MSG_RADIOWIN_DISPLAY_SUB_LIST_SEL);
            RetVal=0;
            break;
        }

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&RadioSubFreqWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&RadioSubFreqWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return(RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void RadioSubFreqWinDisplay(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo         2009-4-10        1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOSUBFREQWIN_CODE_
void RadioSubFreqWinDisplay(void)
{
    UINT16 i,j,m,n;
    UINT16 temp, TempColor, Temp_DrawMode;

    UINT16 Per_ID = PrevChannel;
    UINT16 Current_ID = CurrentChannel;

    UINT16 k;
    UINT16 *pFmFreq;
    UINT16 FmDispBuf[64];

    TempColor      = LCD_GetColor();
    Temp_DrawMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);

    if ((TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_SUB_LIST_BK))||(TRUE == GetMsg(MSG_NEED_PAINT_ALL)))
    {
         DispPictureWithIDNumAndXYoffset(IMG_ID_FM_MENU_BACKGROUND1,0,0);

        k = Current_ID / RADIO_SUB_FM_DISNUM;
        k = k * RADIO_SUB_FM_DISNUM; //if Current_ID is not even,frist divide then multipity, value reduce one,just ok,

        pFmFreq = &gpRadioplayerRegKey->FmFreqArray[k];  //stroe all FM frequncy,get by hackgroud,30 frequency together.

        for (i = 0; i < RADIO_SUB_FM_DISNUM; i++, pFmFreq++)
        {

            RadioFreq2Ascii(i+k, *pFmFreq, FmDispBuf);  //get frequency string

            if ((i + k) == Current_ID)
            {
                LCD_SetColor(COLOR_RED);
            }

            LCD_NFDispStringAt(RADIO_SUB_LIST_TXT_X,
                               (RADIO_SUB_LIST_TXT_Y + i * RADIO_SUB_LIST_TXT_YSTEP), FmDispBuf); //��ʾƵ���ַ���
            LCD_SetColor(COLOR_BLACK);
        }
    }

    if (TRUE == GetMsg(MSG_RADIOWIN_DISPLAY_SUB_LIST_SEL))  //fm store stations select
    {
        LCD_SetDispRect(25, 76+10, 25+90-1,76+ 36);
        DispPictureWithIDNumAndXYoffset(IMG_ID_FM_MENU_BACKGROUND1,0,0);
        LCD_SetDispRect(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);

        temp = Current_ID/RADIO_SUB_FM_DISNUM;

        if ((Per_ID / RADIO_SUB_FM_DISNUM) != temp)
        {
            if (Current_ID == (FM_SAVE_MAX_COUNTER-1) && (Per_ID == 0))
            {//turn up is special situation,for example Current_ID = 39,Per_ID = 0,then the next screen display 38 39
                m = Current_ID%RADIO_SUB_FM_DISNUM;
                k = Current_ID - m;
                pFmFreq = &gpRadioplayerRegKey->FmFreqArray[k];
            }
            else if (((Per_ID % RADIO_SUB_FM_DISNUM) == 0)&&
                     ((Current_ID % RADIO_SUB_FM_DISNUM) == (RADIO_SUB_FM_DISNUM - 1)))
            {

                m = Current_ID%RADIO_SUB_FM_DISNUM;
                k = Current_ID - m;
                pFmFreq = &gpRadioplayerRegKey->FmFreqArray[k];
            }
            else
            {
                pFmFreq = &gpRadioplayerRegKey->FmFreqArray[Current_ID];
                m = FM_SAVE_MAX_COUNTER - (Current_ID+1);
                k = Current_ID;
            }

            for (i=0; (i<RADIO_SUB_FM_DISNUM && i<=m); i++, pFmFreq++)
            {
                RadioFreq2Ascii(k+i, *pFmFreq, FmDispBuf);

                if ((i + k) == Current_ID)
                {
                    LCD_SetColor(COLOR_RED);
                }

                //DispPictureWithIDNumAndXYoffset(IMG_ID_FM_SAVEBACKGROUND1,0,(RADIO_SUB_LIST_TXT_Y + i * RADIO_SUB_LIST_TXT_YSTEP)+13); //display background

                LCD_NFDispStringAt(RADIO_SUB_LIST_TXT_X,(RADIO_SUB_LIST_TXT_Y + i * RADIO_SUB_LIST_TXT_YSTEP), FmDispBuf);
                LCD_SetColor(COLOR_BLACK);
            }
        }
        else
        {
            j = Per_ID%RADIO_SUB_FM_DISNUM;       //j = 0
            i = Current_ID%RADIO_SUB_FM_DISNUM;   // i = 1

            LCD_SetColor(COLOR_BLACK);
            RadioFreq2Ascii(Per_ID, gpRadioplayerRegKey->FmFreqArray[Per_ID], FmDispBuf);
            LCD_NFDispStringAt(RADIO_SUB_LIST_TXT_X,(RADIO_SUB_LIST_TXT_Y + j * RADIO_SUB_LIST_TXT_YSTEP), FmDispBuf);

            RadioFreq2Ascii(Current_ID, gpRadioplayerRegKey->FmFreqArray[Current_ID], FmDispBuf);
            LCD_SetColor(COLOR_RED);
            LCD_NFDispStringAt(RADIO_SUB_LIST_TXT_X,(RADIO_SUB_LIST_TXT_Y + i * RADIO_SUB_LIST_TXT_YSTEP), FmDispBuf);
        }
        LCD_SetColor(TempColor);
    }

    LCD_SetTextMode(Temp_DrawMode);
    LCD_SetColor(TempColor);
}

/*
********************************************************************************
*
*                         End of Dialogbox.c
*
********************************************************************************
*/
#endif



