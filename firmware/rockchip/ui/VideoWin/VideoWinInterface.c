/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   VideoWinInterface.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                  cw           2009-02-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_VIDEOWIN_INTERFACE_

#include "SysInclude.h"

#ifdef _VIDEO_

#include "VideoControl.h"
#include "VideoWin.h"
#include "VideoWinInterface.h"

/*
--------------------------------------------------------------------------------
  Function name : void VideoWinSvcStart(void *pArg)  
  Author        : Chenwei
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                Chenwei     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VIDEOWIN_CODE_
void VideoWinSvcStart(void *pArg)
{
    if (TRUE != ThreadCheck(pMainThread, &VideoThread))
    {
       ThreadDeleteAll(&pMainThread);
       ModuleOverlay(MODULE_ID_VIDEO_CONTROL, MODULE_OVERLAY_ALL);
       ThreadCreat(&pMainThread, &VideoThread, pArg);
    }
    return;
}

/*
--------------------------------------------------------------------------------
  Function name :  void VideoWinSvcStop(void)  
  Author        : Chenwei
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                Chenwei     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VIDEOWIN_CODE_
void VideoWinSvcStop(void)
{
    
}

/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN VideoDecodeProc(MSG_ID id,void *msg)  
  Author        : Chenwei
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                Chenwei     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_VIDEOWIN_CODE_
BOOLEAN VideoWinDecodeProc(MSG_ID id,void *msg)
{
   //return AudioDecodeProc(id,msg);
   return TRUE;
}


/*
********************************************************************************
*
*                         End of MusicWinInterface.c
*
********************************************************************************
*/
#endif


