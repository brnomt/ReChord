/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                         All rights reserved.
*
* File Name£º   RecordWinInterface.c
*
* Description:  the interface that record provide for extenal part
*
* History:      <author>          <time>        <version>
*               WangBo         2009-4-10          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_RECORDWIN_INTERFACE_

#include "SysInclude.h"

#ifdef _RECORD_

#include "RecordWinInterface.h"

/*
--------------------------------------------------------------------------------
  Function name : void RecordWinSvcStart(void *pArg)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                WangBo       2009/04/28         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_RECORDWIN_INIT_CODE_
void RecordWinSvcStart(void *pArg)
{
    //Record backgroud service start. 
    if (TRUE != ThreadCheck(pMainThread, &RecordThread))
    {
        //ThreadDeleteAll(&pMainThread);
        //OverLay record backgroud code
        ModuleOverlay(MODULE_ID_RECORD_CONTROL, MODULE_OVERLAY_ALL);
        ThreadCreat(&pMainThread, &RecordThread, pArg);

        //when enter recording interface frist,file name,recording time,recording quality,recording status
        //that get by backgroud do not display at frist, 
        SendMsg(MSG_RECORD_INITUI);
        
        SendMsg(MSG_RECORDWIN_DISPLAY_ALL);
    }
    else
    {
        SendMsg(MSG_RECORDWIN_DISPFLAG_REFRESH);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordWinSvcStop(void)
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                Chenwei     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_RECORDWIN_DEINIT_CODE_
_ATTR_RECORDWIN_SERVICE_CODE_
void RecordWinSvcStop(void)
{
    if (TRUE == ThreadCheck(pMainThread, &RecordThread))
    {
        //ThreadDeleteAll(&pMainThread);   //delete all thread,
        ThreadDelete(&pMainThread, &RecordThread);   //delete current thread,
    }

}

#endif

