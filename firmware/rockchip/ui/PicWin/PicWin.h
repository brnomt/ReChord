/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£∫   PicWin.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*              anzhiguo         2009-3-2          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _PICWIN_H_
#define _PICWIN_H_

#undef  EXT
#ifdef _IN_PICWIN_H_
#define EXT
#else
#define EXT extern
#endif
/*
*-------------------------------------------------------------------------------
*
*                            Õº∆¨∂Œ∂®“Â
*
*-------------------------------------------------------------------------------
*/

#define _ATTR_PIC_CODE_                    
#define _ATTR_PIC_DATA_                    
#define _ATTR_PIC_BSS_                     

/*
********************************************************************************
*
*                         Functon Declaration
*
********************************************************************************
*/
EXT void    PicWinInit(void *pArg);
EXT void    PicWinDeInit(void);
EXT UINT32  PIcWinService(void);
EXT void    PicWinPaint(void);
EXT UINT32  PicWinKey(void);

/*
********************************************************************************
*
*                         Description:  window sturcture definition
*
********************************************************************************
*/
#ifdef _IN_PICWIN_H_
_ATTR_PIC_DATA_ WIN PicWin = {
    
    NULL,
    NULL,
    
    PIcWinService,                    //window service handle function.
    PicWinKey,                    //window key service handle function.
    PicWinPaint,                      //window display service handle function.
    
    PicWinInit,                       //window initial handle function.
    PicWinDeInit                      //window auti-initial handle function.
    
};
#else 
_ATTR_PIC_DATA_ EXT WIN PicWin;
#endif

_ATTR_PIC_DATA_
EXT int    gSilidShowTimeCnt;

_ATTR_PIC_BSS_
EXT uint32 ImageSlideShowTickBake;

_ATTR_PIC_BSS_
EXT uint8 PicWinFromWhere;

/*
********************************************************************************
*
*                         End of  PicWin.h
*
********************************************************************************
*/
#endif

