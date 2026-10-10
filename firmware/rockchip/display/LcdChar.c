/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   LcdChar.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
//******************************************************************************
#define   _IN_LCDCHAR_
#include "fsinclude.h"
#include "LcdInclude.h"
#include "ModuleOverlay.h"

/*
--------------------------------------------------------------------------------
  Function name : void LCD_NFDispUnicodeChar(UINT32   c)
  Author        : yangwenjie
  Description   : charactor display

  Input        c: the charactor will be displayed.
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_NFDispUnicodeChar(UINT16 c)
{
    UINT8    DispCharBuf[72];

    UINT32   CharInNFAddr;
    INT16    x, y, xsize, ysize;
    UINT32   D_mode, Old_mode;
    UINT16   iType;

    if (c == 0)
        return;

    if (LcdContext.TextFort == FONT_12x12)
    {
        //UINT16 DispCharBuf[26];
        CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(c) << 5);
        LcdGetFront12Data(CharInNFAddr, DispCharBuf, 26);
    }
    else
    {

        //UINT16 DispCharBuf[34];
        CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(c) * 33);
        LcdGetFront16Data(CharInNFAddr, DispCharBuf, 34);
    }

    D_mode   = LcdContext.TextMode;
    Old_mode = LcdContext.DrawMode;
    LCD_SetDrawMode(D_mode);

    x        =  LcdContext.DispPosX;
    y        =  LcdContext.DispPosY;

    {
        uint16 chxsize;

        if (LcdContext.TextFort == FONT_12x12)
        {
            iType = *(DispCharBuf + 25);
            ysize = CH_CHAR_YSIZE_12;
            xsize = AS_CHAR_XSIZE_12;

            chxsize = CH_CHAR_XSIZE_12;
        }
        else
        {
            iType = *(DispCharBuf + 33);
            ysize = CH_CHAR_YSIZE_16;
            xsize = AS_CHAR_XSIZE_16;

            chxsize = CH_CHAR_XSIZE_16;
        }

        if (0x10 == iType)
        {
            if (Language == LANGUAGE_RUSSIAN)// && (LcdContext.TextFort == FONT_12x12)
            {
                xsize = AS_CHAR_XSIZE_16;
            }
        }
        else if (0x20 == iType)
        {
            xsize = chxsize;
        }
        else if (0x80 == iType)             // �ո�
        {
            // ��ǿո� ( < 0x7f)
            if (c >= 0x7f)                  // ȫ�ǿո� (>= 0x7f)
            {
                xsize = chxsize;
            }
        }
        else if (iType > 0x80)
        {
            xsize = iType - 0x80;
        }
        else
        {
            LCD_SetDrawMode(Old_mode);
            return;
        }
    }

    if (Language == LANGUAGE_THAI)
    {

        UINT16  i, BufLen, offset;
        UINT8   DispCharBuf_01[34];
        UINT8   DispCharBuf_02[34];

        memset((UINT8 *)DispCharBuf_01, 0x00, 34);
        memset((UINT8 *)DispCharBuf_02, 0x00, 34);

        if (LcdContext.TextFort == FONT_12x12)
        {
            BufLen = 24;
            offset = 32;
        }
        else
        {
            BufLen = 31;
            offset = 33;
        }

        if (0 != Special_TaiWen_Buf[0])
        {
            CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(Special_TaiWen_Buf[0]) * offset);//��Ҫ��֤

            if (LcdContext.TextFort == FONT_12x12)
                LcdGetFront12Data(CharInNFAddr, (UINT8*)DispCharBuf_01, BufLen);
            else
                LcdGetFront16Data(CharInNFAddr, (UINT8*)DispCharBuf_01, BufLen);

            if (0 != Special_TaiWen_Buf[1])
            {
                CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(Special_TaiWen_Buf[1]) * offset);//��Ҫ��֤

                if (LcdContext.TextFort == FONT_12x12)
                    LcdGetFront12Data(CharInNFAddr, (UINT8*)DispCharBuf_02, BufLen);
                else
                    LcdGetFront16Data(CharInNFAddr, (UINT8*)DispCharBuf_02, BufLen);
            }
        }

        for (i = 0; i < BufLen; i++)
        {
            DispCharBuf[i] |= DispCharBuf_01[i + 1];
            DispCharBuf[i] |= DispCharBuf_02[i + 1];
        }
    }

    LCD_DrawBmp(x, y, xsize, ysize, 1, (UINT16*)DispCharBuf);

    if ((LcdContext.XDist > 0) || (LcdContext.YDist > 0))
    {

        if (D_mode != LCD_DRAWMODE_TRANS)
        {

            LCD_SetDrawMode(D_mode ^ LCD_DRAWMODE_REV);
            LCD_FillRect(x + xsize, y, x + xsize + LcdContext.XDist, y + ysize + LcdContext.YDist);
            LCD_FillRect(x, y + ysize, x + xsize,   y + ysize + LcdContext.YDist);
        }
    }

    LCD_SetDrawMode(Old_mode);

    if (LcdContext.TextMode != LCD_DRAWMODE_CIR90)
        LcdContext.DispPosX += (xsize + LcdContext.XDist);
    else
        LcdContext.DispPosY += (xsize + LcdContext.YDist);
}

/*
--------------------------------------------------------------------------------
  Function name : INT32 LCD_GetCharXSize(UINT32 s)
  Author        : yangwenjie
  Description   : get the width of one charactor

  Input       s : charactor
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetCharXSize(UINT16 s)
{
    UINT8   DispCharBuf[2];
    UINT32  CharInNFAddr;
    UINT16   iWidth;

    if (LcdContext.TextFort == FONT_12x12)
    {
        CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(s) << 5) + 24;
        LcdGetFront12Data(CharInNFAddr, (UINT8*)DispCharBuf, 2);
    }
    else
    {
        CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(s) * 33) + 32;
        LcdGetFront16Data(CharInNFAddr, (UINT8*)DispCharBuf, 2);
    }

    {
        uint16 chxsize;

        if (LcdContext.TextFort == FONT_12x12)
        {
            iWidth  = AS_CHAR_XSIZE_12 ;
            chxsize = CH_CHAR_XSIZE_12;
        }
        else
        {
            iWidth  = AS_CHAR_XSIZE_16;
            chxsize = CH_CHAR_XSIZE_16;
        }

        if (0x10 == DispCharBuf[1])// && (LcdContext.TextFort == FONT_12x12))
        {
            if (Language == LANGUAGE_RUSSIAN)
            {
                iWidth = AS_CHAR_XSIZE_16;
            }
        }
        else if (0x20 == DispCharBuf[1])
        {
            iWidth = chxsize;
        }
        else if (0x80 == DispCharBuf[1])
        {
            if (s >= 0x7f)
            {
                iWidth = chxsize;
            }
        }
        else if (DispCharBuf[1] > 0x80)
        {
            iWidth = DispCharBuf[1] - 0x80;
        }
    }

    if (Language == LANGUAGE_THAI)
    {

        // �����̩�ĵ� �ϱ� ����   �±�,   ����Ϊ 0.
        if (0 != Check_Special_TaiWen(s))
        {
            iWidth = 0;
        }
    }

    return iWidth;
}

/*
--------------------------------------------------------------------------------
  Function name : INT32 LCD_GetCharsPerLine(LCD_RECT *r, UINT32   *s)
  Author        : yangwenjie
  Description   : compute the total number of charactors,these can display in one line of rect.

  Input         :   r   :  the rect that is used fo display charactors.
                    s   :  the pointer of string.
  Return        :   width of charactors.

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetCharsPerLine(LCD_RECT *r, UINT16 *s)
{
    INT32 NumChar;
    INT32 StrDistX;

    NumChar  = 0;
    StrDistX = 0;

    while (*s)
    {
        StrDistX += (LcdContext.XDist + LCD_GetCharXSize(*s));

        if ((*s == '\n') || (*s == '\r') || (*s == 0) || (StrDistX > (r->x1 - r->x0)))
            break;

        NumChar++;
        s++;
    }

    return (NumChar);
}


/*
--------------------------------------------------------------------------------
  Function name : INT32   LCD_GetLineDistX(UINT32   * s,    INT32 LineNumChar)
  Author        : yangwenjie
  Description   : get the width of charactors according to number of charactors.�����ַ��������ַ��Ŀ���

  Input         :  s                :   string pointer.
                   LineNumChar      :   charactors number
  Return        :  width of charactors.

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetLineDistX(UINT16 * s,    UINT32 LineNumChar)
{
    INT32 DistX;

    DistX = 0;

    for (; LineNumChar > 0; LineNumChar--)
    {
        DistX += (LcdContext.XDist + LCD_GetCharXSize(*s++));
    }

    return (DistX);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_DispLineChar(UINT32 *s, INT32 LineNumChar)

  Author        : yangwenjie
  Description   : display string.

  Input         :  s            :   the string pointer.
                   LineNumChar  :   charactor number.
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_DispLineChar(UINT16 *s, UINT32 LineNumChar)
{
    UINT32 i;

    if (Language == LANGUAGE_THAI)
    {
        for (i = 0; i < LineNumChar; i++)
        {
            // ����̩�������ַ�, ����ʾ
            if (Check_Special_TaiWen(*s) == 0)
            {
                //Special_TaiWen_Buf[0] = 0;
                //Special_TaiWen_Buf[1] = 0;
                Special_TaiWen_Buf[0] = Check_Special_TaiWen(*(s + 1));
                Special_TaiWen_Buf[1] = Check_Special_TaiWen(*(s + 2));
                LCD_NFDispUnicodeChar(*s);
                s++;
            }
            else
            {
                // ̩�������ַ�, ����ʾ
                s++;
            }
        }
    }
    else
    {
        for (i = 0; i < LineNumChar; i++)
        {
            LCD_NFDispUnicodeChar(*s);
            s++;
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : INT32   LCD_GetStringSize(UINT32 *s)
  Author        : yangwenjie
  Description   : get the width of the string.

  Input         :  s       :   string pointer.

  Return        :  string width.

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT32 LCD_GetStringSize(UINT16 *s)
{
    UINT32    CharSize = 0;

    while (*s)
    {
        CharSize += (LcdContext.XDist + LCD_GetCharXSize(*s++));
    }

    return (CharSize);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_NFDispString(UINT8 *pStr)
  Author        : yangwenjie
  Description   : string display.

  Input         : pStr :string pointer.

  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_NFDispString(UINT16 *pStr)
{
    UINT32     StringSize = 0;
    INT32      xAdjust    = 0;
    INT32      Old_DispPosX;

    Old_DispPosX = LcdContext.DispPosX;

    StringSize = LCD_GetStringSize(pStr);

    switch (LcdContext.TextAlign & LCD_TEXTALIGN_HORIZONTAL)
    {

        case LCD_TEXTALIGN_RIGHT:
            xAdjust = StringSize;
            break;

        case LCD_TEXTALIGN_CENTER:
            xAdjust = StringSize / 2;
            break;

        default:
            xAdjust = 0;
    }

    LcdContext.DispPosX -= xAdjust;

    while (*pStr)
    {
        if (*pStr == '\n')
        {
            switch (LcdContext.TextAlign & LCD_TEXTALIGN_HORIZONTAL)
            {
                case LCD_TEXTALIGN_RIGHT:
                case LCD_TEXTALIGN_CENTER:
                    LcdContext.DispPosX = Old_DispPosX;
                    break;

                default :
                    LcdContext.DispPosX = LcdContext.LBorder;
                    break;
            }

            if (LcdContext.TextFort == FONT_12x12)
            {
                LcdContext.DispPosY += (LcdContext.YDist + CH_CHAR_YSIZE_12);
            }
            else //if(LcdContext.TextFort == FONT_16x16)
            {
                LcdContext.DispPosY += (LcdContext.YDist + CH_CHAR_YSIZE_16);
            }
        }
        else
        {
            if (LcdContext.TextLanguage == LANGUAGE_THAI)
            {
                if (Check_Special_TaiWen(*pStr) == 0)
                {
                    Special_TaiWen_Buf[0] = Check_Special_TaiWen(*(pStr   +   1));
                    Special_TaiWen_Buf[1] = Check_Special_TaiWen(*(pStr   +   2));
                    LCD_NFDispUnicodeChar(*pStr);
                }
                else
                {
                    pStr++;
                }
            }
            else
            {
                LCD_NFDispUnicodeChar(*pStr);
            }
        }

        pStr++;
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void LCD_NFDispStringAt(INT32   x, INT32 y,   UINT32 *pStr)
  Author        : yangwenjie
  Description   : display string in specified postion.

  Input         :   x,y  : coordinate of display string.
                    pStr : string pointer.

  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_NFDispStringAt(UINT32 x, UINT32 y, UINT16 *pStr)
{
    UINT32 bIsBidirectionalString;
    UINT16 StringBuffer[2 * TXT_PER_LINE_CHARS];

    LcdContext.DispPosX = x;
    LcdContext.DispPosY = y;

    if (LcdContext.TextLanguage == LANGUAGE_RABBINIC)
    {
        bIsBidirectionalString = IsBidirectionalString( pStr);

        if (1 == bIsBidirectionalString)
        {
            BidirectionalStringPro(pStr, StringBuffer);
            LCD_NFDispString(StringBuffer);

            return;
        }
    }

    LCD_NFDispString(pStr);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_DispStringInRect(LCD_RECT *pDr, LCD_RECT *pSr,   UINT32 *pStr, UINT32 AlignMode)
  Author        : yangwenjie
  Description   : display string in specified postion.

  Input         :   pDr         : target area.
                    pSr         : source area.
                    pStr        : string pointer.
                    AlignMode   : align mode.

  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_DispStringLine(LCD_RECT *pDr, LCD_RECT   *pSr,   UINT16 *pStr, UINT32 AlignMode, UINT8 YSymbol)
{

    LCD_RECT    old_r;
    INT16       NumChar;
    INT16       xDist;
    INT16       old_DispPosX;
    INT16       xAdjust;
    UINT16      bIsBidirectionalString;
    UINT16      StringBuffer[TXT_PER_LINE_CHARS * 2];
    UINT16      *pString;

    pString = pStr;

    old_r = LcdContext.ClipRect;


    LcdContext.ClipRect = *pDr;

    if (YSymbol)
    {

        if (LcdContext.TextFort == FONT_12x12)
        {
            LcdContext.DispPosY = (pSr->y1 - pSr->y0 - CH_CHAR_YSIZE_12) / 2 + pSr->y0;
        }
        else //if(LcdContext.TextFort == FONT_16x16)
        {
            LcdContext.DispPosY = (pSr->y1 - pSr->y0 - CH_CHAR_YSIZE_16) / 2 + pSr->y0;
        }
    }
    else
    {
        LcdContext.DispPosY = pSr->y0;
    }

    bIsBidirectionalString = IsBidirectionalString (pStr);

    if (1 == bIsBidirectionalString)
    {
        if (IsSetmenu == FALSE)
        {
            //if (LCD_TEXTALIGN_LEFT == AlignMode)
            //{
            BidirectionalStringPro(pStr, StringBuffer);
            //AlignMode   = LCD_TEXTALIGN_RIGHT;
            pString = StringBuffer;

            //}
        }
        else
        {
            bIsBidirectionalString = 0;
            IsSetmenu = FALSE;
        }
    }

    switch (AlignMode & LCD_TEXTALIGN_HORIZONTAL)
    {
        case LCD_TEXTALIGN_RIGHT:
            LcdContext.DispPosX = pSr->x1;
            break;

        case LCD_TEXTALIGN_CENTER:
            LcdContext.DispPosX = pSr->x0 + (pSr->x1 - pSr->x0) / 2;
            break;

        default:
            LcdContext.DispPosX = pSr->x0;
            break;
    }

    old_DispPosX = LcdContext.DispPosX;

    NumChar =   LCD_GetCharsPerLine(pSr, pString);//�����ַ����ȼ�����ھ�����һ������ʾ���ַ���
    xDist   =   LCD_GetLineDistX(pString, NumChar);//�����ַ��������ַ��Ŀ���

    switch (AlignMode & LCD_TEXTALIGN_HORIZONTAL)
    {
        case LCD_TEXTALIGN_RIGHT:
            xAdjust =   xDist;
            break;

        case LCD_TEXTALIGN_CENTER:
            xAdjust =   xDist / 2;
            break;

        default:
            xAdjust =   0;
            break;
    }

    LcdContext.DispPosX -= xAdjust;

    LCD_DispLineChar(pString, NumChar);

    LcdContext.DispPosX  =  old_DispPosX;
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_DispStringInRect(LCD_RECT *pDr, LCD_RECT *pSr,   UINT32 *pStr, UINT32 AlignMode)
  Author        : yangwenjie
  Description   : display string in specified postion.

  Input         :   pDr         : target area.
                    pSr         : source area.
                    pStr        : string pointer.
                    AlignMode   : align mode.

  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_DispStringInRect(LCD_RECT *pDr, LCD_RECT   *pSr,   UINT16 *pStr, UINT32 AlignMode)
{

    LCD_RECT    old_r;
    INT16       NumChar;
    INT16       xDist;
    INT16       old_DispPosX;
    INT16       xAdjust;
    UINT16      bIsBidirectionalString;
    UINT16      StringBuffer[TXT_PER_LINE_CHARS * 2];
    UINT16      *pString;

    pString = pStr;

    old_r = LcdContext.ClipRect;

    LcdContext.ClipRect = *pDr;

    LcdContext.DispPosY = pSr->y0;

    bIsBidirectionalString = IsBidirectionalString (pStr);

    if (1 == bIsBidirectionalString)
    {
        if (IsSetmenu == FALSE)
        {
            //if (LCD_TEXTALIGN_LEFT == AlignMode)
            //{
            BidirectionalStringPro(pStr, StringBuffer);
            //AlignMode   = LCD_TEXTALIGN_RIGHT;
            pString = StringBuffer;

            //}
        }
        else
        {
            bIsBidirectionalString = 0;
            IsSetmenu = FALSE;
        }
    }

    switch (AlignMode & LCD_TEXTALIGN_HORIZONTAL)
    {
        case LCD_TEXTALIGN_RIGHT:
            LcdContext.DispPosX = pSr->x1;
            break;

        case LCD_TEXTALIGN_CENTER:
            LcdContext.DispPosX = pSr->x0 + (pSr->x1 - pSr->x0) / 2;
            break;

        default:
            LcdContext.DispPosX = pSr->x0;
            break;
    }

    old_DispPosX = LcdContext.DispPosX;

    while (*pString)
    {
        NumChar =   LCD_GetCharsPerLine(pSr, pString);//�����ַ����ȼ�����ھ�����һ������ʾ���ַ���
        xDist   =   LCD_GetLineDistX(pString, NumChar);//�����ַ��������ַ��Ŀ���

        switch (AlignMode & LCD_TEXTALIGN_HORIZONTAL)
        {
            case LCD_TEXTALIGN_RIGHT:
                xAdjust =   xDist;
                break;

            case LCD_TEXTALIGN_CENTER:
                xAdjust =   xDist / 2;
                break;

            default:
                xAdjust =   0;
                break;
        }

        LcdContext.DispPosX -= xAdjust;

        //DEBUG("numchar = %d",NumChar);
        LCD_DispLineChar(pString, NumChar);

        pString += NumChar;

        if (*pString == '\n' )
        {
            pString++;
        }

        LcdContext.DispPosX  =  old_DispPosX;

        if (LcdContext.TextFort == FONT_12x12)
        {
            LcdContext.DispPosY += (LcdContext.YDist + CH_CHAR_YSIZE_12);
        }
        else //if(LcdContext.TextFort == FONT_16x16)
        {
            LcdContext.DispPosY += (LcdContext.YDist + CH_CHAR_YSIZE_16);
        }

        if (LcdContext.DispPosY >= LcdContext.ClipRect.y1)
        {
            break;
        }
    }

    LcdContext.ClipRect = old_r;
}

/*
--------------------------------------------------------------------------------
  Function name : void ScrollStringForMusic(UINT16 *pstr )
  Author        : yangwenjie
  Description   : audio string display as a rolling mode.

  Input         : r            :  display area.
                  pstr         :  diaplay string pointer.
                  PictureInfo  :  the information of display image.
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void ScrollStringForMusic(UINT16 *pstr )
{
    UINT16    *pCurternStr;
    UINT16     TextMode;
    UINT16     Count;


    if (IsBidirectionalString(pstr))
    {
        return;
    }

    Count = SysTickCounter - ScrollString.SystickCounterBack;

    if (Count > ScrollString.ScrollSpeed)
    {
        if ((ScrollString.DiplayRect.x0 > ScrollString.StartX))
        {
            pCurternStr = pstr;

            if ((ScrollString.DiplayRect.x0 - ScrollString.StartX) < ScrollString.TextFort)
            {
                ScrollString.DiplayRect.x0 = ScrollString.StartX;
            }
            else
            {
                ScrollString.DiplayRect.x0 = ScrollString.StartX + ScrollString.DiplayRect.x1 - ScrollString.ScrollNumber * ScrollString.TextFort; //ScrollString.CharSizePerLine - ScrollString.ScrollNumber*ScrollString.TextFort;
            }
        }
        else
        {
            if (ScrollString.TotalCharNum >= ScrollString.CharNumber)
            {
                pCurternStr = pstr + ScrollString.CharNumber;
                ScrollString.CharNumber++;
            }
            else
            {
                ScrollString.ScrollNumber = 1;
                ScrollString.DiplayRect.x0 = ScrollString.StartX +
                                             ScrollString.DiplayRect.x1 - ScrollString.ScrollNumber * ScrollString.TextFort; //ScrollString.CharSizePerLine-ScrollString.ScrollNumber*ScrollString.TextFort;
                pCurternStr = pstr;
                ScrollString.CharNumber = 1;
            }
        }

#if (LCD_PIXEL == LCD_PIXEL_1)

        if (ScrollString.pictureID == NULL)
        {
            LCD_ClrRect(ScrollString.DiplayRect.x0, ScrollString.DiplayRect.y0, ScrollString.DiplayRect.x1, ScrollString.DiplayRect.y1);
        }
        else
#endif
        {
            DisplayPicture_part(ScrollString.pictureID, ScrollString.pictureInfo.x, ScrollString.pictureInfo.y, ScrollString.pictureInfo.yoffset, ScrollString.pictureInfo.ysize);
        }

        LCD_DispStringInRect(&(ScrollString.DiplayRect), &(ScrollString.DiplayRect), pCurternStr, LCD_TEXTALIGN_LEFT);
        ScrollString.ScrollNumber++;
        ScrollString.SystickCounterBack = SysTickCounter;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void ScrollStringCommon(UINT16 *pstr )
  Author        : yangwenjie
  Description   : audio string display as a rolling mode.

  Input         : r            :  display area.
                  pstr         :  diaplay string pointer.
                  PictureInfo  :  the information of display image.
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void ScrollStringCommon(UINT16 *pstr )
{
    UINT16    *pCurternStr;
    UINT16     TextMode;
    UINT16     Count;

    if (IsBidirectionalString(pstr))
    {
        return;
    }

    Count = SysTickCounter -  ScrollString.SystickCounterBack;

    if (Count >= ScrollString.ScrollSpeed)
    {
        if (ScrollString.TotalCharNum >= ScrollString.CharNumber)
        {
            pCurternStr = pstr + ScrollString.CharNumber;
            ScrollString.CharNumber++;
        }
        else
        {
            pCurternStr = pstr;
            ScrollString.CharNumber = 1;
        }

#if (LCD_PIXEL == LCD_PIXEL_1)

        if (ScrollString.pictureID == NULL)
        {
            LCD_ClrRect(ScrollString.DiplayRect.x0, ScrollString.DiplayRect.y0, ScrollString.DiplayRect.x1, ScrollString.DiplayRect.y1);
        }
        else
#endif
        {
            DisplayPicture_part(ScrollString.pictureID, ScrollString.pictureInfo.x, ScrollString.pictureInfo.y, ScrollString.pictureInfo.yoffset, ScrollString.pictureInfo.ysize);
        }

        LCD_DispStringInRect(&(ScrollString.DiplayRect), &(ScrollString.DiplayRect), pCurternStr, LCD_TEXTALIGN_LEFT);
        ScrollString.SystickCounterBack = SysTickCounter;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void void SetScrollStringInfo(LCD_RECT *r, PicturePartInfo PictureInfo,UINT16 *pstr,UINT16 Speed)
  Author        : yangwenjie
  Description   : set rolling string information.

  Input         : r            :  display area.
                  PictureInfo  :  display string pointer.
                  PictureInfo  :  display image's information.
  Return        :

  History:     <author>         <time>         <version>
                 yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void SetScrollStringInfo(LCD_RECT *r, PicturePartInfo PictureInfo, UINT16 *pstr, UINT16 Speed)
{
    UINT16 CharNumPerLine;

    ScrollString.TotalSize   =  LCD_GetStringSize(pstr);
    ScrollString.TotalCharNum  =   LCD_GetStringNum(pstr) ;
    ScrollString.pictureID   =  PictureInfo.pictureIDNump;
    CharNumPerLine = LCD_GetCharsPerLine(r, pstr);
    ScrollString.CharSizePerLine = LCD_GetLineDistX(pstr, CharNumPerLine);

    if (LcdContext.TextFort == FONT_12x12)
    {
        ScrollString.TextFort  = CH_CHAR_XSIZE_12;
    }
    else
    {
        ScrollString.TextFort  = CH_CHAR_XSIZE_16;
    }

    ScrollString.ScrollNumber   = 1;
    ScrollString.CharNumber     = 0;// 1
    ScrollString.pictureInfo.x  = PictureInfo.x;
    ScrollString.pictureInfo.y  = PictureInfo.y;
    ScrollString.pictureInfo.yoffset  = PictureInfo.yoffset;
    ScrollString.pictureInfo.ysize    = PictureInfo.ysize;
    ScrollString.DiplayRect.x0  = r->x0;
    ScrollString.DiplayRect.y0  = r->y0;
    ScrollString.DiplayRect.x1  = r->x1;
    ScrollString.DiplayRect.y1  = r->y1;
    ScrollString.StartX  = ScrollString.DiplayRect.x0;
    ScrollString.SystickCounterBack = SysTickCounter;
    ScrollString.ScrollSpeed = Speed;
}
/*
--------------------------------------------------------------------------------
  Function name : void DisplayString(UINT32   x, UINT32 y, UINT32 xsize,
                                        UINT32 ysize, UINT32 mode, UINT32 flash_addr)
  Author        : yangwenjie
  Description   : get string resource from flash.

  Input         :x,y    :     the coordinate of display string.
                 xsize  :     the length of tring(the pixels number.)
                 ysize  :     the height of tring(the pixels number.)
                 mode   :     display mode
                 flash_addr : the address of string that is in flash.
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/10/29         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void DisplayString(UINT32 x, UINT32 y, UINT32 xsize, UINT32 ysize,  UINT32 mode, UINT16 TextID)
{
    LCD_RECT        r;
    UINT16          StringBuf[64];

    //GetResourceInfo(TextID,StringBuf,64);
    GetResourceStr(TextID, StringBuf, 64);
    r.x0 = x;
    r.y0 = y;
    r.x1 = x + xsize - 1;
    r.y1 = y + ysize - 1;

    LCD_DispStringInRect(&r, &r, StringBuf, mode);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_SetCharSize(UINT16 size)
  Author        : yangwenjie
  Description   : set font size

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void LCD_SetCharSizeInit(void)
{
    uint8  FlashBuf[512];

    FIRMWARE_HEADER *pFWHead = (FIRMWARE_HEADER *)FlashBuf;

    MDReadData(SysDiskID, 0, 512, FlashBuf);
    Font16LogicAddress = pFWHead->FontInfo.FontInfoTbl[FONT16].ModuleOffset;
    Font12LogicAddress = pFWHead->FontInfo.FontInfoTbl[FONT12].ModuleOffset;
}

_ATTR_LCD_CODE_
UINT16 LCD_SetCharSize(UINT16 size)
{
    UINT16 TextFortBack;

    TextFortBack = LCD_TEXTFORT;

    if (LCD_TEXTFORT != size)
    {
        LCD_TEXTFORT = size;

        if (LCD_TEXTFORT == FONT_16x16)
        {
            FontLogicAddress = Font16LogicAddress;//pFWHead->FontInfo.FontInfoTbl[FONT16].ModuleOffset;
        }
        else
        {
            FontLogicAddress = Font12LogicAddress;//pFWHead->FontInfo.FontInfoTbl[FONT12].ModuleOffset;
        }
    }

    return TextFortBack;
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetColor(UINT16 color)
  Author        : ZHengYongzhi
  Description   : set front color.

  Input         : color
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#if 0
_ATTR_LCD_CODE_
void LCD_SetColor(UINT16 color)
{
    LCD_COLOR = color;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetColor(void)
  Author        : ZHengYongzhi
  Description   : ��ȡǰ��ɫ

  Input         :
  Return        : ��ǰǰ��ɫ

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetColor(void)
{
    return (LCD_COLOR);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetBkColor(UINT16 color)
  Author        : ZHengYongzhi
  Description   : ���ñ���ɫ

  Input         : color ���� ����ɫ
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetBkColor(UINT16 color)
{
    LCD_BKCOLOR = color;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetBkColor(void)
  Author        : ZHengYongzhi
  Description   : ��ȡ����ɫ

  Input         :
  Return        : ��ǰ����ɫ

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetBkColor(void)
{
    return (LCD_BKCOLOR);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_SetDrawMode(UINT16 mode)
  Author        : ZHengYongzhi
  Description   : ����ϵͳ��ͼģʽ

  Input         : mode ���� �µĻ�ͼģʽ
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetDrawMode(UINT16 mode)
{
    LCD_DRAWMODE = mode;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetDrawMode(void)
  Author        : ZHengYongzhi
  Description   : ��ȡ��ͼģʽ

  Input         :
  Return        : ��ǰ��ͼģʽ

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetDrawMode(void)
{
    return (LCD_DRAWMODE);
}
#endif
/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetTextMode(UINT16 mode)
  Author        : ZHengYongzhi
  Description   : set text display mode.

  Input         : mode ����
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_SetTextMode(UINT16 mode)
{
    //LCD_TEXTMODE = mode;
    UINT16 mode_temp;

#ifdef COLOR_LCD

    if (mode == LCD_DRAWMODE_REV)
    {
        mode = LCD_DRAWMODE_NORMAL;
    }
    else if (mode == LCD_DRAWMODE_NORMAL)
    {
        mode = LCD_DRAWMODE_REV;
    }

#endif

    mode_temp = LcdContext.TextMode;
    LcdContext.TextMode = mode;

    return (mode_temp);

}

#if 0
/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetTextMode(void)
  Author        : ZHengYongzhi
  Description   : ��ȡ�ı���ʾģʽ

  Input         :
  Return        : �ı���ʾģʽ

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetTextMode(void)
{
    return (LCD_TEXTMODE);
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetTextAlign(UINT16 mode)
  Author        : ZHengYongzhi
  Description   : �����ı�����ģʽ

  Input         : mode ���� �ı�����ģʽ
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetTextAlign(UINT16 mode)
{
    LCD_TEXTALIGN = mode;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_GetTextAlign(void)
  Author        : ZHengYongzhi
  Description   : ��ȡ�ı�����ģʽ

  Input         :
  Return        : �ı�����ģʽ

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_GetTextAlign(void)
{
    return (LCD_TEXTALIGN);
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetDiaplayMode(INT16 mode)
  Author        : ZHengYongzhi
  Description   : set display mode

  Input         : mode ����
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetDiaplayMode(INT16 mode)
{
    LCD_DISPMODE = mode;

    if (mode == LCD_MODE_0)
    {

        LcdContext.LcdMaxWidth = LCD_WIDTH;
        LcdContext.LcdMaxHeight = LCD_HEIGHT;

#ifdef _FRAME_BUFFER_
        Lcd_SetFrameMode(0);
#endif

    }
    else
    {
        LcdContext.LcdMaxWidth = LCD_HEIGHT;
        LcdContext.LcdMaxHeight = LCD_WIDTH;

#ifdef _FRAME_BUFFER_
        Lcd_SetFrameMode(1);
#endif

    }

    LcdContext.ClipRect.x0 = 0;
    LcdContext.ClipRect.y0 = 0;
    LcdContext.ClipRect.x1 = LcdContext.LcdMaxWidth - 1;
    LcdContext.ClipRect.y1 = LcdContext.LcdMaxHeight - 1;


}



/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetLanguage(INT16 mode)
  Author        : yangwenjie
  Description   : get and set current language.

  Input         :
  Return        : current language.

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT8 LCD_SetLanguage(UINT8 SysLanguage)
{
    UINT8 LanguageBack;

    LanguageBack =  Language;

    if (Language != SysLanguage)
    {
        Language = SysLanguage;
    }

    return (LanguageBack);
}


/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetDispRect(INT16 x0, INT16 y0, INT16 x1, INT16 y1)
  Author        : ZHengYongzhi
  Description   : set display area in lcd.

  Input         : x0,y0 ���� the upper left coordinate.
                  x1,y1 ���� the lower right coordinate.
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetDispRect(INT16 x0, INT16 y0, INT16 x1, INT16 y1)
{
    LcdContext.ClipRect.x0 = x0;
    LcdContext.ClipRect.y0 = y0;

    if (LCD_DISPMODE == LCD_MODE_0)
    {

        LcdContext.ClipRect.x1 = x1;
        LcdContext.ClipRect.y1 = y1;

    }
    else
    {

        LcdContext.ClipRect.x1 = y1;
        LcdContext.ClipRect.y1 = x1;

    }
}


/*
--------------------------------------------------------------------------------
  Function name : void LCD_SetPixel(UINT16 x, UINT16 y, UINT16 color)
  Author        : ZHengYongzhi
  Description   : set display pixel

  Input         : x0,y0 ���� the upper left coordinate
                  x1,y1 ���� the lower right coordinate
  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_SetPixel(UINT16 x, UINT16 y, UINT16 color)
{
#if (LCD_PIXEL == LCD_PIXEL_16)
    {
        Lcd_SetWindow(x, y, x, y);
        Lcd_SendData (color);
    }
#else
    {
        LCDDEV_SetPixel(x, y, color);
    }
#endif
}

/*
--------------------------------------------------------------------------------
  Function name : void GetResourceStr(UINT16 menuTextID , UINT16 *pMenuStr ,UINT16 StrLen)
  Author        : yangwenjie
  Description   : get the resource of string.

  Input         : menuTextID��string ID
                  pMenuStr  ��the unicode code of charactor group.
                  StrLen    ��get the length of string.
  Return        : null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void GetResourceStr(UINT16 menuTextID , UINT16 *pMenuStr , UINT16 StrLen)
{

    UINT32   CharInNFAddr;
    UINT32   languegeOffsetAddr;
    int8  hMainFile;

    languegeOffsetAddr = MenuLogicAddress + Language * MENU_ITEM_LENGTH * TOTAL_MENU_ITEM + TOTAL_LANAUAGE_NUM * 4 + 2;

    if (!gSysConfig.FMEnable)
    {
        languegeOffsetAddr = MenuLogicAddress + Language * MENU_ITEM_LENGTH * (TOTAL_MENU_ITEM - 9) + TOTAL_LANAUAGE_NUM * 4 + 2;
    }

    //  LcdGetResourceInfo(MenuLogicAddress,(UINT8*)&languegeInfo,sizeof(languegeInfo));
    CharInNFAddr = languegeOffsetAddr + menuTextID * MENU_ITEM_LENGTH + MENU_CONTENT_OFFSET;

#if (RES_IN_DEV == 1)
    LcdGetResourceInfo(CharInNFAddr, (UINT8*)pMenuStr, StrLen);
#else

    //for SPI Flash Test
    if ((hMainFile = FileOpenW(NULL, L"\\", L"menu.res", "R")) == -1) //���ļ��ɹ������ش���
    {
        DEBUG("Open File ERROR");
    }

    FileSeek((CharInNFAddr - MenuLogicAddress), 0, hMainFile);
    FileRead((UINT8*)pMenuStr, StrLen, hMainFile);
    FileClose(hMainFile);
#endif
}
/*
--------------------------------------------------------------------------------
  Function name : void GetMenuTextInfoWithIDNum(UINT32 menuTextID,
                              MENU_TEXT_INFO_STRUCT *pMenuTextInfo)
  Author        : yangwenjie
  Description   : get menu resource structure information(string id number).it be use for muiti-country.

  Input         : menuTextID    ��menu text id.
                  pMenuTextInfo ��save menu resource text structrue information pointer.
  Return        : null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void GetMenuTextInfoWithIDNum(UINT32 menuTextID,
                              MENU_TEXT_INFO_STRUCT *pMenuTextInfo)
{
    UINT32   CharInNFAddr;
    UINT32   languegeOffsetAddr;
    int8  hMainFile;
    languegeOffsetAddr = MenuLogicAddress + Language * MENU_ITEM_LENGTH * (TOTAL_MENU_ITEM - 9) + TOTAL_LANAUAGE_NUM * 4 + 2;

    if (gSysConfig.FMEnable)
    {
        languegeOffsetAddr = MenuLogicAddress + Language * MENU_ITEM_LENGTH * (TOTAL_MENU_ITEM) + TOTAL_LANAUAGE_NUM * 4 + 2;
    }

    CharInNFAddr = languegeOffsetAddr + menuTextID * MENU_ITEM_LENGTH;

#if (RES_IN_DEV == 1)
    LcdGetResourceInfo(CharInNFAddr, (UINT8*)pMenuTextInfo, sizeof(MENU_TEXT_INFO_STRUCT));
#else

    //for SPI Flash Test
    if ((hMainFile = FileOpenW(NULL, L"\\", L"menu.res", "R")) == -1) //���ļ��ɹ������ش���
    {
        DEBUG("Open File ERROR");
    }

    FileSeek((CharInNFAddr - MenuLogicAddress), 0, hMainFile);
    FileRead((UINT8*)pMenuTextInfo, sizeof(MENU_TEXT_INFO_STRUCT), hMainFile);
    FileClose(hMainFile);
#endif

}

/*
--------------------------------------------------------------------------------
  Function name : void DisplayMenuStrWithIDNum(UINT16 x, UINT16 y,
                             UINT16 xsize, UINT16 ysize,
                             UINT16 alignMode, UINT16 menuTextID)
  Author        : yangwenjie
  Description   : display menu resource text.(use text id)

  Input         : x,y        : coordinate of display string.
                  xsize      : string length(pixels)
                  ysize      : string height(pixels)
                  alignMode  : align mode
                  menuTextID ��menu text id.
  Return        : null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void DisplayMenuStrWithIDNum(UINT16 x, UINT16 y,
                             UINT16 xsize, UINT16 ysize,
                             UINT16 alignMode, UINT16 menuTextID)
{
    UINT16  tempBuf[((MENU_ITEM_LENGTH - MENU_CONTENT_OFFSET) >> 1)];
    LCD_RECT      r;

    GetResourceStr(menuTextID, tempBuf, (UINT16)((MENU_ITEM_LENGTH - MENU_CONTENT_OFFSET) >> 1));
    r.x0 = x;
    r.y0 = y;
    r.x1 = x + xsize - 1;
    r.y1 = y + ysize - 1;

    IsSetmenu = TRUE;

    LCD_DispStringInRect(&r, &r, tempBuf, alignMode);

}

/*
********************************************************************************
*
*                         End of LcdChar.c
*
********************************************************************************
*/


