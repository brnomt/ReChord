/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��  PicInterface.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             yangwenjie          2009-3-2        1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_PICINTERFACE_

#include "SysInclude.h"
#ifdef _PICTURE_
#pragma arm section code = "PicWinCode", rodata = "PicWinCode", rwdata = "PicWinData", zidata = "PicWinBss"
#include "fsinclude.h"
#include "SysFindFile.h"
#include "PicWin.h"
#include "PicInterface.h"
#include "ImageControl.h"
/*
--------------------------------------------------------------------------------
  Function name : void PictureWinSvcStart(void *pArg)
  Author        : azg
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
                azg     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void PictureWinSvcStart(void *pArg)
{
    
    if (TRUE != ThreadCheck(pMainThread, &PictureThread))
    {
        //ThreadDeleteAll(&pMainThread);		//do not allow to play music in backgroud.
        ModuleOverlay(MODULE_ID_PICTURE_CONTROL, MODULE_OVERLAY_ALL);
        ThreadCreat(&pMainThread, &PictureThread, pArg);
    }
}


#pragma arm section code
/*
********************************************************************************
*
*                         End of PicInterface.c
*
********************************************************************************
*/
#endif

