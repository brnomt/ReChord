/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                         All rights reserved.
*
* File Name£º   RadioWinInterface.c
*
* Description:  FM external interface quote
*
* History:      <author>          <time>        <version>
*               WangBo      2009-3-20          1.0
*    desc:    ORG.
********************************************************************************
*/
//#define _IN__RADIOWIN_INTERFACE_
#define _IN_RADIOWIN_INTERFACE_

#include "SysInclude.h"
#ifdef _RADIO_

#include "RadioWin.h"
#include "RadioWinInterface.h"

extern  THREAD FMThread;
/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN MusicDecodeProc(MSG_ID id,void *msg)
  Author        : Chenwei
  Description   : audio decode handle function
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                Chenwei     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_CODE_
void RadioWinSvcStart(void *pArg)
{
    //FM backgroud service progarm start
    if (TRUE != ThreadCheck(pMainThread, &FMThread))
    {
        //ThreadDeleteAll(&pMainThread);
        ModuleOverlay(MODULE_ID_FM_CONTROL, MODULE_OVERLAY_ALL);
        ThreadCreat(&pMainThread, &FMThread, pArg);
        FmStandbyFlag = FALSE;//HJ
    }
}

/*
------------------------------------------------ --------------------------------
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
_ATTR_RADIOWIN_CODE_
void RadioWinSvcStop(void)
{
    
}

/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN MusicDecodeProc(MSG_ID id,void *msg)
  Author        : Chenwei
  Description   : audio decode handle function
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                Chenwei     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RADIOWIN_CODE_
BOOLEAN RadioDecodeProc(MSG_ID id,void *msg)
{
   return FM_Process(id,msg);
}


#endif

/*
********************************************************************************
*
*                         End of RadioWinInterface.c
*
********************************************************************************
*/

