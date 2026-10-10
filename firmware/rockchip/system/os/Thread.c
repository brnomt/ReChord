/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  Thread.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:
********************************************************************************
*/
#define _IN_THREAD_

#include "OsInclude.h"
#include "Driverconfig.h"

/*
--------------------------------------------------------------------------------
  Function name : UINT WinCreat(WIN *pWin, WIN *pFatherWin)
  Author        : ZHengYongzhi
  Description   : create one thread,append it to the end of the specified thread list

  Input         : pThread      ---- new thread and its structure pointer.
                  pThreadFirst ---- the initial pointer of specified thread table.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 ThreadCreat(THREAD **pThreadFirst, THREAD *pThread, void *pArg)
{
    THREAD *pThreadTemp = *pThreadFirst;

    if (*pThreadFirst == (THREAD*)NULL) //if the thread list table is null,put the new thread pointer to the initial pointer of thread list table.
    {
        *pThreadFirst = pThread;
        pThread->Next = NULL;
        pThread->Prev = NULL;
    }
    else //put the new thread link to the end fo specified thread list table.
    {
        while(pThreadTemp->Next != NULL)
        {
            if (pThreadTemp == pThread)
            {
                return FALSE;
            }
            pThreadTemp = pThreadTemp->Next;
        }
        pThreadTemp->Next = pThread;
        pThread->Prev = pThreadTemp;
        pThread->Next = NULL;
    }

    //call the initial function of thread
    pThread->pArg = pArg;
    pThread->pgThreadInit(pArg);

    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT ThreadDelete(THREAD *pThreadFirst, THREAD *pThread)
  Author        : ZhengYongzhi
  Description   : delete one thread from specified thread list table.

  Input         : pThread      ---- the pointers of thread and structure that will be deleting.
                  pThreadFirst ---- the inital pointer of specified thread list table.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 ThreadDelete(THREAD **pThreadFirst, THREAD *pThread)
{
    THREAD *pThreadTemp;

    if ((*pThreadFirst == NULL) || (pThread == NULL))
        return TRUE;

    //judge the thread that will be deleting is presence in specified thread list table.
    pThreadTemp = *pThreadFirst;
    while((pThreadTemp != pThread) && (pThreadTemp != (THREAD*)NULL))
    {
        pThreadTemp = pThreadTemp->Next;
    }
    if (pThreadTemp != pThread) return FALSE;

    // delete thread
    if (pThread == *pThreadFirst) //if the thread that will be deleting is the frist thread that in thread list table.it is need to adjust the initial pointer of thread list table.
    {
        if (pThread->Next != (THREAD*)NULL)
        {
            *pThreadFirst = pThread->Next;
            (*pThreadFirst)->Prev = (THREAD*)NULL;
        }
        else
        {
            *pThreadFirst = NULL;
        }
    }
    else //delete this thread from list table
    {

       if (pThread->Next != (THREAD*)NULL)
       {
            pThread->Prev->Next = pThread->Next;
            pThread->Next->Prev = pThread->Prev;
        }
        else
        {
            pThread->Prev->Next = (THREAD*)NULL;
        }
    }

    //call thread deinitial program.
    pThread->Prev = (THREAD*)NULL;
    pThread->Next = (THREAD*)NULL;
    pThread->pgThreadDeInit();

    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT WinDestroy(WIN *pWin)
  Author        : ZHengYongzhi
  Description   : delete one thread.

  Input         : pWin ---- pointer of current window.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 ThreadDeleteAll(THREAD **pThreadFirst)
{
    THREAD *pThreadTemp = *pThreadFirst;

    if (*pThreadFirst == NULL)
    {
        return TRUE;
    }

    //find the last thread that location is the last in list table.
    while(pThreadTemp->Next != (THREAD*)NULL)
    {
        pThreadTemp = pThreadTemp->Next;
    }

    while(pThreadTemp != *pThreadFirst)
    {
        pThreadTemp->pgThreadDeInit();
        pThreadTemp = pThreadTemp->Prev;
        pThreadTemp->Next->Prev = NULL;
        pThreadTemp->Next = NULL;
    }

    pThreadTemp->pgThreadDeInit();
    pThreadTemp->Next->Prev = NULL;
    pThreadTemp->Next = NULL;
    pThreadTemp->Prev = NULL;
    *pThreadFirst = NULL;

    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 ThreadCheck(THREAD *pThreadFirst, THREAD *pThread)
  Author        : ZHengYongzhi
  Description   : check whether the specified thread has been registered.

  Input         : pThread      ---- pointer of new thread and structure.
                  pThreadFirst ---- pointer to the frist initial pointer of specified thread list table.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 ThreadCheck(THREAD *pThreadFirst, THREAD *pThread)
{
    THREAD *pThreadTemp = pThreadFirst;

    while(pThreadTemp != (THREAD*)NULL)
    {
        if (pThreadTemp == pThread)
        {
            return TRUE;
        }
        pThreadTemp = pThreadTemp->Next;
    }

    return FALSE;
}

/*
--------------------------------------------------------------------------------
  Function name : void ClearMsg(MSG_ID MsgId)
  Author        : ZHengYongzhi
  Description   : clear message

  Input         : message ID
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
UINT32 ThreadProcess(THREAD *pThreadFirst)
{
    THREAD *pThreadTemp = pThreadFirst;

    while(pThreadTemp != (THREAD*)NULL)
    {

        pThreadTemp->pgThreadProc();

        pThreadTemp = pThreadTemp->Next;
    }

    return (TRUE);
}


_ATTR_DRIVERLIB_CODE_
UINT32 ThreadDeleteAllExcept(THREAD **pThreadFirst, THREAD *pThread)
{
    THREAD *pThreadTemp;

    if ((*pThreadFirst == NULL) || (pThread == NULL))
        return TRUE;

    //judge the thread that will be deleting is presence in specified thread list table.
    pThreadTemp = *pThreadFirst;
    while((pThreadTemp != pThread) && (pThreadTemp != (THREAD*)NULL))
    {
        pThreadTemp = pThreadTemp->Next;
    }
    if (pThreadTemp != pThread) return FALSE;

    // delete thread
    if (pThread == *pThreadFirst) //if the thread that will be deleting is the frist thread that in thread list table.it is need to adjust the initial pointer of thread list table.
    {
        if (pThread->Next != (THREAD*)NULL)
        {
            *pThreadFirst = pThread->Next;
            (*pThreadFirst)->Prev = (THREAD*)NULL;
        }
        else
        {
            *pThreadFirst = NULL;
        }
    }
    else //delete this thread from list table
    {

       if (pThread->Next != (THREAD*)NULL)
       {
            pThread->Prev->Next = pThread->Next;
            pThread->Next->Prev = pThread->Prev;
        }
        else
        {
            pThread->Prev->Next = (THREAD*)NULL;
        }
    }


    ThreadDeleteAll(pThreadFirst);

    *pThreadFirst = pThread;
    pThread->Next = NULL;
    pThread->Prev = NULL;


    return (TRUE);
}

/*
********************************************************************************
*
*                         End of Thread.c
*
********************************************************************************
*/

