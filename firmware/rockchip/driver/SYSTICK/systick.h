/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   systick.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#undef  EXT
#ifdef _IN_SYSTICK_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                            Struct Define
*
*-------------------------------------------------------------------------------
*/

/*
*-------------------------------------------------------------------------------
*
*                            Function Declaration
*
*-------------------------------------------------------------------------------
*/
extern void   SysTickClkSourceSet2(UINT32 Source);
extern BOOL   SysTickClkSourceGet2(void);
extern void   SysTickEnable2(void);
extern void   SysTickDisable2(void);
extern UINT32 SysTickPeriodGet2(void);
extern UINT32 SysTickValueGet2(void);
extern BOOL   SysTickRefClockCheck2(void);
extern BOOL   SysTickCalibrationCheck2(void);
extern UINT32 SysTickCalibrationTenMsGet2(void);
extern void SysTickPeriodSet2(UINT32 mstick );

extern void SysTickPeriodSet(UINT32 mstick );


#endif

/*
********************************************************************************
*
*                         End of systick.h
*
********************************************************************************
*/


