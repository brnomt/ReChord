/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  SetMusic.c
*
* Description:  set menu execution function configuration of music
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-3-3          1.0
*    desc:      the functions below will be excuted when press sure key and it has not child item,
                the itemNum be force set to CurItemInFatherID,that is the index of current item,so it can
                be used to do porper contorl in inside of function.
********************************************************************************
*/
#define _SETMUSIC_IN_

#include "SysInclude.h"

#ifdef _SYSSET_
#ifdef _MUSIC_
#include "SetCommon.h"
#include "SetMusic.h"
#include "SetMenu.h"
#include "SetMenuInterface.h"
#include "Hold.h"

/*
--------------------------------------------------------------------------------
  Function name : void SetMusicEqInit(void)
  Author        : Anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               Anzhiguo       2009/03/01         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_MUSIC_CODE_
void SetMusicUserEqInit(void)
{
    UINT32 i;

#ifdef _RK_EQ_
    for (i=0;i<5;i++)//backup
    {
        SetUserEq[i] = SysSetUserEq[i];
    }
    curEqXcorVal = 0;
#endif

    KeyReset();             //initial
    SendMsg(MSG_USEREQ_SET_INITFLAG);
    SendMsg(MSG_USEREQ_SET_DISPFLAG);
    SendMsg(MSG_USEREQ_SET_TITLE);

}

/*
--------------------------------------------------------------------------------
  Function name : void SetMusicEqDeInit(void)
  Author        : Anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               Anzhiguo     2009/03/01         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_MUSIC_CODE_
void SetMusicUserEqDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SetMusicEqService(void)
  Author        : Anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                Anzhiguo     2009/03/01         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_MUSIC_CODE_
UINT32 SetMusicUserEqService(void)
{
    TASK_ARG TaskArg;

    return(RETURN_OK);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MainMenuKey(void)
  Author        : Anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               Anzhiguo     2009/03/01         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_MUSIC_CODE_
UINT32 SetMusicUserEqKey(void)
{
    UINT16 i,RetVal;
    UINT32 userEqSetKeyVal;
    TASK_ARG TaskArg;

    RetVal = 0;

    userEqSetKeyVal = GetKeyVal();

    //5 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (userEqSetKeyVal)
        {
            case KEY_VAL_FFD_DOWN:
            case KEY_VAL_FFD_PRESS:
                userEqSetKeyVal = KEY_VAL_UP_DOWN;
                break;

            case KEY_VAL_FFW_DOWN:
            case KEY_VAL_FFW_PRESS:
                userEqSetKeyVal = KEY_VAL_DOWN_DOWN;
                break;

            case KEY_VAL_ESC_SHORT_UP:
            case KEY_VAL_ESC_PRESS:
                userEqSetKeyVal = KEY_VAL_FFD_DOWN;
                break;

            default:
                break;
        }
    }

    //6 key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
        switch (userEqSetKeyVal)
        {
            case KEY_VAL_PLAY_SHORT_UP:
                userEqSetKeyVal = KEY_VAL_ESC_SHORT_UP;
                break;

            default:
                break;
        }
    }

    switch (userEqSetKeyVal)
    {
        case KEY_VAL_PLAY_SHORT_UP:
        {
            if (curEqXcorVal < 4)
            {
                curEqXcorVal++;
                SendMsg(MSG_USEREQ_SET_DISPFLAG);
            }
            else
            {
                curEqXcorVal = 0;
                WinDestroy(&SetMusicUserEqWin);
                SendMsg(MSG_SYS_SET_DISPLAY);
            }
            break;
        }

        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_DOWN:
        case KEY_VAL_UP_SHORT_UP:
            break;

        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_DOWN:
        case KEY_VAL_DOWN_SHORT_UP:
            break;

        //case KEY_VAL_FFD_DOWN:
        //case KEY_VAL_FFD_PRESS:
        case KEY_VAL_FFD_SHORT_UP:
        {
            if (curEqXcorVal < 5)//cursor position
            {
                if (SysSetUserEq[curEqXcorVal] < 6)
                {
                    SysSetUserEq[curEqXcorVal]++;
                    SendMsg(MSG_AUDIO_EQSET_UPDATE_USER_EQ); // By Vincent
                    SendMsg(MSG_NEED_PAINT_ALL);
                }
            }
            break;
        }

        //case KEY_VAL_FFW_DOWN:
        //case KEY_VAL_FFW_PRESS:
        case KEY_VAL_FFW_SHORT_UP:
        {
            if (curEqXcorVal < 5)
            {
                if (SysSetUserEq[curEqXcorVal] > 0)
                {
                    SysSetUserEq[curEqXcorVal]--;
                    SendMsg(MSG_AUDIO_EQSET_UPDATE_USER_EQ); // By Vincent
                    SendMsg(MSG_NEED_PAINT_ALL);
                }
            }
            break;
        }

        case KEY_VAL_ESC_PRESS_START:
        case KEY_VAL_ESC_SHORT_UP: //exit key
        {
            if (0 < curEqXcorVal)
            {
                curEqXcorVal--;
                SendMsg(MSG_USEREQ_SET_DISPFLAG);
            }
            else
            {
                for(i = 0; i < 5; i++)//backup after exiting
                {
                    SysSetUserEq[i] = SetUserEq[i];
                }

                curEqXcorVal = 0;
                WinDestroy(&SetMusicUserEqWin);
                SendMsg(MSG_SYS_SET_DISPLAY);
            }
            break;
        }

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            SendMsg(MSG_SYS_SET_MUSIC_HOLD);
            WinCreat(&SetMusicUserEqWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            SendMsg(MSG_SYS_SET_MUSIC_HOLD);
            WinCreat(&SetMusicUserEqWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }


    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void SetMusicEqDisplay(void)
  Author        : Anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               Anzhiguo     2009/03/01         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_MUSIC_CODE_
void SetMusicUserEqDisplay(void)
{
    UINT16 i,RetVal;

    RetVal = 0;
#ifdef _RK_EQ_
    if (CheckMsg(MSG_NEED_PAINT_ALL) )
    {
        SendMsg(MSG_USEREQ_SET_INITFLAG);
        SendMsg(MSG_USEREQ_SET_DISPFLAG);
        SendMsg(MSG_USEREQ_SET_TITLE);
    }
    if (GetMsg(MSG_USEREQ_SET_INITFLAG))
    {
        DispPictureWithIDNum(IMG_ID_SETMENU_USEREQSETTING);
    }

    if (GetMsg(MSG_USEREQ_SET_TITLE))
    {
    }

    if (GetMsg(MSG_USEREQ_SET_DISPFLAG))
    {
        for (i = 0; i < 5; i++)
        {
            if (i == curEqXcorVal)
            {
                DispPictureWithIDNumAndXYoffset((IMG_ID_SETMENU_USERSEL01 + SysSetUserEq[curEqXcorVal]), (curEqXcorVal * 19)+22, 38);
            }
            else
            {
                DispPictureWithIDNumAndXYoffset((IMG_ID_SETMENU_USERNOSEL01 + SysSetUserEq[i]), (i * 19)+22, 38);
            }
        }
    }
#endif
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 MusicPlayModeSet(UINT16 itemNum)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_MUSIC_CODE_//_ATTR_SYS_SET_SERVICE_CODE_
UINT32 MusicPlayModeSet(UINT32 itemNum)
{
    UINT32 retVal=0;

    if (SetMusicPlayMode != itemNum)
    {
        SetMusicPlayMode = itemNum;
        SendMsg(MSG_SERVICE_MUSIC_MODE_UPDATE);
    }
    return retVal;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 MusicPlayModeSet(UINT16 itemNum)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_ATTR_SYS_SET_MUSIC_CODE_
UINT32 MusicPlayOrderSet(UINT32 itemNum)
{
    UINT32 retVal=0;

    if (SetMusicPlayOrder != itemNum)
    {
        SetMusicPlayOrder = itemNum;
        SendMsg( MSG_SERVICE_MUSIC_ORDER_UPDATE);
    }
    return retVal;
}
/*
--------------------------------------------------------------------------------
  Function name : UINT16 MusicPlayEqSet(UINT16 itemNum)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009-3-3       Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_SET_MUSIC_CODE_
UINT32 MusicPlayEqSet(UINT32 itemNum)
{
    UINT32 retVal=0;
    SysSetEqBackup = itemNum;

    return retVal;
}

/*
********************************************************************************
*
*                         End of SetMusic.c
*
********************************************************************************
*/
#endif

#endif

