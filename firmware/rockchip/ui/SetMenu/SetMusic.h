/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  SetMusic.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SETMUSIC_H_
#define _SETMUSIC_H_

#undef  EXT
#ifdef  _SETMUSIC_IN_
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
#define _ATTR_SYS_SET_MUSIC_CODE_         
#define _ATTR_SYS_SET_MUSIC_DATA_         
#define _ATTR_SYS_SET_MUSIC_BSS_          

_ATTR_SYS_SET_MUSIC_BSS_ EXT UINT16 curEqXcorVal;

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


EXT  UINT32 MusicPlayModeSet(UINT32 itemNum);
EXT  UINT32 MusicPlayOrderSet(UINT32 itemNum);
EXT  UINT32 MusicPlayEqSet(UINT32 itemNum);

/*
--------------------------------------------------------------------------------
  
   Functon Declaration 
  
--------------------------------------------------------------------------------
*/
EXT void SetMusicUserEqInit(void);
EXT void SetMusicUserEqDeInit(void);

EXT UINT32 SetMusicUserEqService(void);
EXT UINT32 SetMusicUserEqKey(void);
EXT void SetMusicUserEqDisplay(void);



/*
--------------------------------------------------------------------------------
  
  Description:  window sturcture definition
  
--------------------------------------------------------------------------------
*/
#ifdef _SETMUSIC_IN_
_ATTR_SYS_SET_MUSIC_DATA_ WIN SetMusicUserEqWin = {
    
    NULL,
    NULL,
    
    SetMusicUserEqService,               
    SetMusicUserEqKey,                   
    SetMusicUserEqDisplay,               
    
    SetMusicUserEqInit,                  
    SetMusicUserEqDeInit                 
    
};
#else 
_ATTR_SYS_SET_MUSIC_DATA_ EXT WIN SetMusicUserEqWin;
#endif

/*
********************************************************************************
*
*                         End of SetMusic.h
*
********************************************************************************
*/
#endif

