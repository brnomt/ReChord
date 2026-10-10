/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  SystemSet.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SYSTEMSET_H_
#define _SYSTEMSET_H_

#undef  EXT
#ifdef  _SYSTEMSET_IN_
#define EXT
#else
#define EXT extern
#endif
/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                      */
/*                                                                            */
/******************************************************************************/

//setting menu permanent code.
#define _ATTR_SYS_SET_SYSTEM_CODE_         
#define _ATTR_SYS_SET_SYSTEM_DATA_         
#define _ATTR_SYS_SET_SYSTEM_BSS_          

#ifdef _MUSIC_
//play mode 
#define MUSIC_MODE_ONCE                 0
#define MUSIC_MODE_REPEAT               1
#define MUSIC_MODE_FOLDER_ONCE          2
#define MUSIC_FOLDER_REPEAT             3
#define MUSIC_MODE_ALLONCE              4
#define MUSIC_MODE_ALLREPEAT            5
#define MUSIC_MODE_TRY                  6
// play order
#define MUSIC_ORDER_NORMAL              0
#define MUSIC_ORDER_RANDOM              1

//eq type
#define MUSIC_EQ_NORMAL                 0
#define MUSIC_EQ_MSEQ                   1
#define MUSIC_EQ_ROCK                   2
#define MUSIC_EQ_POP                    3
#define MUSIC_EQ_CLASSIC                4
#define MUSIC_EQ_BASS                   5
#define MUSIC_EQ_JAZZ                   6
#define MUSIC_EQ_USER                   7

//microsoft audio EQ
#define  MUSIC_EQ_MSEQ_3D               0
#define  MUSIC_EQ_MSEQ_BB               1

#endif


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
EXT    UINT32 LanguegeSelect(UINT32 itemNum);
EXT    UINT32 BackLightOffTimeSet(UINT32 itemNum);
EXT    UINT32 BackLightLevelSet(UINT32 itemNum);
EXT    UINT32 BackLightDispModeSet(UINT32 itemNum);
EXT    UINT32 PowerOffTimeSet(UINT32 itemNum);
EXT    UINT32 ConvertMemMB(UINT16 *pBuff, UINT32 *Mem);
EXT    UINT32 AvlsEnableSet(UINT32 itemNum);
EXT    UINT32 BeepEnableSet(UINT32 itemNum);
EXT    UINT32 BassBoostSet(UINT32 itemNum);

EXT    UINT32 FirmwareUpgradeSet(UINT32 itemNum);
EXT    UINT32 SysDefaultValSet(UINT32 itemNum);
EXT    void   SysSetDefaultValue(void);

EXT    void   ProductInfoWinInit(void);
EXT    void   ProductInfoWinDeInit(void);

EXT    UINT32 ProductInfoWinService(void);
EXT    UINT32 ProductInfoWinKey(void);
EXT    void   ProductInfoWinDisplay(void);
extern void SysSetDefaultValueClone(void);
EXT    void SysFormatSDcard(void);
EXT    void SysFormatFlash(void);



/*
--------------------------------------------------------------------------------
  
  Description:  window sturcture definition
  
--------------------------------------------------------------------------------
*/
#ifdef _SYSTEMSET_IN_
_ATTR_SYS_SET_SYSTEM_DATA_ WIN ProductInfoWin = {
    
    NULL,
    NULL,
    
    ProductInfoWinService,               
    ProductInfoWinKey,                   
    ProductInfoWinDisplay,               
    
    ProductInfoWinInit,                  
    ProductInfoWinDeInit                 
    
};
#else 
_ATTR_SYS_SET_SYSTEM_DATA_ EXT WIN ProductInfoWin;
#endif

/*
********************************************************************************
*
*                         End of SystemSet.h
*
********************************************************************************
*/
#endif

