/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  VideoWinInterface.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*                  cw           2009-02-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef VIDEOWIN_INTERFACE_H_
#define VIDEOWIN_INTERFACE_H_



#undef  EXT
#ifdef _IN_VIDEOWIN_INTERFACE_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*  
*                           function declaration
*  
*-------------------------------------------------------------------------------
*/
extern void    VideoWinSvcStart(void *pArg);
extern void    VideoWinSvcStop(void);
extern BOOLEAN VideoDecodeProc(MSG_ID id,void *msg);


#endif 

