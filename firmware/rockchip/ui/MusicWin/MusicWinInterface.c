/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   MusicWinInterface.c
*
* Description:  音乐UI与外部的接口
*
* History:      <author>          <time>        <version>
*                  cw           2009-02-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_MUSICWIN_INTERFACE_

#include "SysInclude.h"
#ifdef _MUSIC_

#include "MusicWin.h"
#include "MusicWinInterface.h"

#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#endif
/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN MusicDecodeProc(MSG_ID id,void *msg)
  Author        : Chenwei
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                Chenwei     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_INIT_CODE_
void MusicWinSvcStart(void *pArg)
{
    //Music backgroud service handle start
    if (TRUE != ThreadCheck(pMainThread, &MusicThread))
    {
#ifdef _BLUETOOTH_
        if (TRUE == ThreadCheck(pMainThread, &BlueToothThread))
        {
            ThreadDeleteAllExcept(&pMainThread, &BlueToothThread);
        }
        else
        {
            ThreadDeleteAll(&pMainThread);
        }
#else
        ThreadDeleteAll(&pMainThread);
#endif

        ModuleOverlay(MODULE_ID_AUDIO_CONTROL, MODULE_OVERLAY_ALL);
        ThreadCreat(&pMainThread, &MusicThread, pArg);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN MusicDecodeProc(MSG_ID id,void *msg)
  Author        : Chenwei
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                Chenwei     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MUSIC_DEINIT_CODE_
void MusicWinSvcStop(void)
{

}

/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN MusicDecodeProc(MSG_ID id,void *msg)
  Author        : Chenwei
  Description   : music decoding handle function.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                Chenwei     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
BOOLEAN MusicDecodeProc(MSG_ID id,void *msg)
{
   return AudioDecodeProc(id,msg);
}


/*
********************************************************************************
*
*                         End of MusicWinInterface.c
*
********************************************************************************
*/
#endif

