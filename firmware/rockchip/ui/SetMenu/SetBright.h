/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  SetBright.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SETBRIGHT_H_
#define _SETBRIGHT_H_

#undef  EXT
#ifdef  _SETBRIGHT_IN_
#define EXT
#else
#define EXT extern
#endif


/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                      */
/*                                                                            */
/******************************************************************************/

//setting  menu permanent code.
#define _ATTR_SYS_SET_BRIGHT_CODE_         
#define _ATTR_SYS_SET_BRIGHT_DATA_         
#define _ATTR_SYS_SET_BRIGHT_BSS_          
 
_ATTR_SYS_SET_BRIGHT_BSS_ EXT UINT8 curBrightLevel;

/*
--------------------------------------------------------------------------------
  
                ENUMERATIONS AND STRUCTURES
  
--------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/


/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
EXT void SetBrightLevelInit(void);
EXT void SetBrightLevelDeInit(void);

EXT UINT32 SetBrightLevelService(void);
EXT UINT32 SetBrightLevelKey(void);
EXT void SetBrightLevelDisplay(void);



/*
--------------------------------------------------------------------------------
  
  Description:  window sturcture definition
  
--------------------------------------------------------------------------------
*/
#ifdef _SETBRIGHT_IN_
_ATTR_SYS_SET_BRIGHT_DATA_ WIN SetBrightLevelWin = {
    
    NULL,
    NULL,
    
    SetBrightLevelService,               
    SetBrightLevelKey,                   
    SetBrightLevelDisplay,               
    
    SetBrightLevelInit,                  
    SetBrightLevelDeInit                 
    
};
#else 
_ATTR_SYS_SET_BRIGHT_DATA_ EXT WIN SetBrightLevelWin;
#endif

/*
********************************************************************************
*
*                         End of SetBright.h
*
********************************************************************************
*/
#endif

