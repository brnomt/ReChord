/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Win.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _WIN_H_
#define _WIN_H_

#undef  EXT
#ifdef _IN_WIN_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------
  
  Description:  definition of window structure.
  
--------------------------------------------------------------------------------
*/
typedef struct WIN_STRUCT {
    
    struct WIN_STRUCT  *Son;
    struct WIN_STRUCT  *Father;
    
    UINT32 (*pgWinService)(void);           //window service handle funciton.
    UINT32 (*pgWinKeyScan)(void);           //handle function of window key.
    void   (*pgWinPaint)(void);             //handle function of window diaplay.
    
    void   (*pgWinInit)(void *pArg);        //initial function of window.
    void   (*pgWinDeInit)(void);            //auti-initial function of window.
    
}WIN;

_ATTR_OS_BSS_   EXT WIN     *pMainWin;
    
/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/


extern UINT32 WinCreat(WIN *pFatherWin, WIN *pWin, void *pArg);
extern UINT32 WinDestroy(WIN *pWin);
extern WIN*   WinGetFocus(WIN *pWin);
extern UINT32 WinServiceProc(WIN *pWin);
extern UINT32 WinKeyProc(WIN *pWin);


extern UINT32 WinPaintProc(WIN *pWin);
extern UINT32 WinProcess(WIN *pWin);
extern void   OSStart(WIN **pWin, THREAD **pThread);

/*
********************************************************************************
*
*                         End of Win.h
*
********************************************************************************
*/
#endif
