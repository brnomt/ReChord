/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   DIALOG.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*                anzhiguo      2009-2-27         1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_DIALOG_BOX

#include "SysInclude.h"
#include "DialogBox.h"
#include "Hold.h"
/*
--------------------------------------------------------------------------------
  Function name : void DialogInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             anzhiguo        2009-2-27         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DIALOGBOX_CODE_
void DialogInit(void *pArg)
{
    ButtonSel           = ((DIALOG_WIN_ARG*)pArg)->Button;
    DialogBoxTitleID    = ((DIALOG_WIN_ARG*)pArg)->TitleID;
    DialogBoxContentID  = ((DIALOG_WIN_ARG*)pArg)->ContentID;

    KeyReset();

    SendMsg(MSG_DIALOG_DISPLAY_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : void DialogDeInit(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
              anzhiguo        2009-2-27         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DIALOGBOX_CODE_
void DialogDeInit(void)
{
    KeyReset();
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 DialogService(void)
  Author        : anzhiguo
  Description   : it is dialog service handle that is used to handle window message,
                    window service task etc.
  Input         :
  Return        :

  History:     <author>         <time>         <version>
               anzhiguo     2009-2-27         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DIALOGBOX_CODE_
UINT32 DialogService(void)
{
    INT16 Retval = 0;

    return(Retval);
}


/*
--------------------------------------------------------------------------------
  Function name : UINT32 DialogKey(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo        2009-2-27         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DIALOGBOX_CODE_
UINT32 DialogKey(void)
{
    UINT32 RetVal = 0;
    UINT32 KeyTemp;
    TASK_ARG TaskArg;
    KeyTemp =  GetKeyVal();

    switch (KeyTemp)
    {
    case KEY_VAL_PLAY_SHORT_UP:
    case KEY_VAL_MENU_SHORT_UP:
        if (ButtonSel == DIALOG_BUTTON_YES)
        {
            SendMsg(MSG_DIALOG_KEY_OK);
        }
        else
        {
            SendMsg(MSG_DIALOG_KEY_CANCEL);
        }
        WinDestroy(&DialogWin);
        RetVal = 1;
        break;

    #if (KEY_NUM == KEY_NUM_5)
    case KEY_VAL_DOWN_SHORT_UP:
    case KEY_VAL_UP_SHORT_UP:
    #else
    case KEY_VAL_FFD_SHORT_UP:
    case KEY_VAL_FFW_SHORT_UP:
    #endif
        if (ButtonSel == DIALOG_BUTTON_YES)
        {
            ButtonSel = DIALOG_BUTTON_NO;
        }
        else
        {
            ButtonSel = DIALOG_BUTTON_YES;
        }
        SendMsg(MSG_DIALOG_DISPLAY_BUTTON);
        break;

    case KEY_VAL_HOLD_ON:
        TaskArg.Hold.HoldAction = HOLD_STATE_ON;
        WinCreat(&DialogWin, &HoldWin, &TaskArg);
        break;

    case KEY_VAL_HOLD_OFF:
        TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
        WinCreat(&DialogWin, &HoldWin, &TaskArg);
        break;

    default:
        break;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void DialogDisplay(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo        2009-2-27         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DIALOGBOX_CODE_
void DialogDisplay(void)
{
    PICTURE_INFO_STRUCT     PicInfo;//it will be use resource location and txt.
    UINT16  TempColor, TempBkColor, TempCharSize,TempTxtMode;

    TempColor    = LCD_GetColor();
    TempBkColor  = LCD_GetBkColor();
    TempTxtMode  = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_DIALOG_DISPLAY_ALL)))
    {
        //display backgroud and dialog title and content.
        DispPictureWithIDNumAndXYoffset(IMG_ID_WARNING_BOX,/*32,24*/4,36);
        DisplayMenuStrWithIDNum(DIALOG_TEXT_TITLE_X, DIALOG_TEXT_TITLE_Y, DIALOG_TEXT_TITLE_XSIZE, DIALOG_TEXT_TITLE_YSIZE, LCD_TEXTALIGN_CENTER, DialogBoxTitleID);                  //Title
        DisplayMenuStrWithIDNum(DIALOG_TEXT_CONTENT_X, DIALOG_TEXT_CONTENT_Y, DIALOG_TEXT_CONTENT_XSIZE, DIALOG_TEXT_CONTENT_YSIZE, LCD_TEXTALIGN_CENTER, DialogBoxContentID);        //Content

        //initial the other picture display mssage
        SendMsg(MSG_DIALOG_DISPLAY_BUTTON);
    }

    if (TRUE == GetMsg(MSG_DIALOG_DISPLAY_BUTTON))
    {
        if(ButtonSel)
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_SEL01_ICON ,DIALOG_BUTTON_YES_X-4,DIALOG_BUTTON_YES_Y-2);
            DispPictureWithIDNumAndXYoffset(IMG_ID_NOSEL00_ICON ,DIALOG_BUTTON_NO_X-4,DIALOG_BUTTON_NO_Y-2);
        }
        else
        {
            DispPictureWithIDNumAndXYoffset(IMG_ID_SEL01_ICON ,DIALOG_BUTTON_NO_X-4,DIALOG_BUTTON_NO_Y-2);
            DispPictureWithIDNumAndXYoffset(IMG_ID_NOSEL00_ICON ,DIALOG_BUTTON_YES_X-4,DIALOG_BUTTON_YES_Y-2);
        }
        DisplayMenuStrWithIDNum(DIALOG_BUTTON_YES_X, DIALOG_BUTTON_YES_Y, DIALOG_BUTTON_YES_XSIZE, DIALOG_BUTTON_YES_YSIZE, LCD_TEXTALIGN_CENTER, SID_YES);//确认字符
        DisplayMenuStrWithIDNum(DIALOG_BUTTON_NO_X, DIALOG_BUTTON_NO_Y, DIALOG_BUTTON_NO_XSIZE, DIALOG_BUTTON_NO_YSIZE, LCD_TEXTALIGN_CENTER, SID_NO);//取消字符
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

