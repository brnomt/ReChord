/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  SetRecord.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SETRECORD_H_
#define _SETRECORD_H_

#undef  EXT
#ifdef  _SETRECORD_IN_
#define EXT
#else
#define EXT extern
#endif
/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                      */
/*                                                                            */
/******************************************************************************/

// record permanent code.
#define _ATTR_SYS_SET_RECORD_CODE_         
#define _ATTR_SYS_SET_RECORD_DATA_         
#define _ATTR_SYS_SET_RECORD_BSS_          

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
EXT UINT32 RecordSetMenuQuality (UINT32 iSelectedItem);
EXT UINT32 RecordSetMenuVolume (UINT32 iSelectedItem);


/*
********************************************************************************
*
*                         End of SetRecord.h
*
********************************************************************************
*/
#endif

