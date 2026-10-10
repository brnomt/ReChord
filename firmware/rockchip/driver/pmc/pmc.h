/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  pmc.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*               wangping         2015-3-5          1.0
*    desc:      ORG.
********************************************************************************
*/
#ifndef _PMC_H
#define _PMC_H

#undef EXT
#ifdef _IN_PMC_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/
//DCDC VID
#define SCU_DCOUT_100                   ((uint32)0x100)
#define SCU_DCOUT_120                   ((uint32)0x120)

typedef enum
{   
    CHARGE_CURRENT_LOW = 0,
    CHARGE_CURRENT_HIGH, 
    
}Charge_Current_Value;


/*
********************************************************************************
*
*                         End of pmc.h
*
********************************************************************************
*/

#endif      

