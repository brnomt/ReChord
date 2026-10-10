/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name£º   PowerManager.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo         2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _POWER_H
#define _POWER_H
#include "freq_enums.h"

#undef  EXT
#ifdef _IN_POWER
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/
/* eFREQ_APP defined in freq_enums.h */

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
typedef struct tagCRU_CLK_INFO
{
    uint32 armFreq;     //ARM PLL FREQ
    uint32 armFreqLast;

} CRU_CLK_INFO,*pCRU_CLK_INFO;

typedef struct tagFREQ_APP_TABLE
{
    uint8  scuAppId;
    uint8  counter;

    uint32 PllFreq;
    uint32 syshclk; //sysfclk
    uint32 sysstclk;
    uint32 syspclk;

    uint32 calhclk; //calfclk
    uint32 calstclk;

}FREQ_APP_TABLE,*pFREQ_APP_TABLE;


/*
*-------------------------------------------------------------------------------
*
*                           Functon Declaration
*
*-------------------------------------------------------------------------------
*/
extern int32 FREQ_Enable(void);
extern int32 FREQ_Disable(void);

extern int32 FREQ_EnterModule(eFREQ_APP modulename);
extern int32 FREQ_ExitModule(eFREQ_APP modulename);

/*
********************************************************************************
*
*                         End of Pmu.h
*
********************************************************************************
*/
#endif


