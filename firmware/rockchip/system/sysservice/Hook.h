/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Hook.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HOOK_H_
#define _HOOK_H_

#undef  EXT
#ifdef _IN_HOOK_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                            Function declaration
*
*-------------------------------------------------------------------------------
*/
extern bool CheckCard(void);
extern bool CheckVbus(void);
extern void NandIOMux(void);
extern void NandIODeMux(void);
extern void NandSetClk(void);

/*
********************************************************************************
*
*                         End of SysService.h
*
********************************************************************************
*/
#endif
