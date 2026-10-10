/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                         All rights reserved.
*
* File Name?¨ºo   RadioWin.h
*
* Description:  FM
*
* History:      <author>          <time>        <version>
*                 WangBo      2009-4-10       1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _RADIOWIN_H_
#define _RADIOWIN_H_

#undef  EXT
#ifdef _IN_RADIOWIN_
#define EXT
#else
#define EXT extern
#endif


/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                      */
/*                                                                            */
/******************************************************************************/


//section define
//FM menu permanent code.
#define _ATTR_RADIOWIN_CODE_         
#define _ATTR_RADIOWIN_DATA_         
#define _ATTR_RADIOWIN_BSS_          

//radio menu initial code
#define _ATTR_RADIOWIN_INIT_CODE_    
#define _ATTR_RADIOWIN_INIT_DATA_    
#define _ATTR_RADIOWIN_INIT_BSS_     

//radio menu auti-initial code
#define _ATTR_RADIOWIN_DEINIT_CODE_  
#define _ATTR_RADIOWIN_DEINIT_DATA_  
#define _ATTR_RADIOWIN_DEINIT_BSS_   

//radio menu dispatch code
#define _ATTR_RADIOWIN_SERVICE_CODE_ 
#define _ATTR_RADIOWIN_SERVICE_DATA_ 
#define _ATTR_RADIOWIN_SERVICE_BSS_  



/******************************************************************************/
/*                                                                            */
/*                          Variable Define                                   */
/*                                                                            */
/******************************************************************************/
#define     FM_GUAGE_CONST                111 // display frequency progress length
#define     FM_VOL_GUAGE_CONST          40 //freq schedule length

#define     RADIOAREA_CHINA                 0
#define     RADIOAREA_EUROPE                1
#define     RADIOAREA_JAPAN                 2
#define     RADIOAREA_USA                   3
#define     RadioArea                       gSysConfig.RadioConfig.FmArea


_ATTR_RADIOWIN_BSS_ EXT    UINT16         CurrentChannel;  //current station.
_ATTR_RADIOWIN_BSS_ EXT    UINT16         PrevChannel;     //previous station
_ATTR_RADIOWIN_BSS_ EXT    UINT32         FmFunctionSele_bit;
_ATTR_RADIOWIN_BSS_ EXT    UINT32         RadioGuageVal;
_ATTR_RADIOWIN_BSS_ EXT    UINT16         SeekingStateTracker;
_ATTR_RADIOWIN_BSS_ EXT    BOOL         FmInitFlag;

typedef enum
{
    SEEKING_STATE_RADIOWIN_NONE = 0,
    SEEKING_STATE_RADIOWIN_TRIGER,
    SEEKING_STATE_FMCONTROL_SEARCHING
}SEEKING_STATE;

#ifdef _IN_RADIOWIN_
_ATTR_RADIOWIN_DATA_ UINT16 RadioWinArea[4]=
{
   RADIOAREA_CHINA,
   RADIOAREA_EUROPE,
   RADIOAREA_JAPAN,
   RADIOAREA_USA
};

_ATTR_RADIOWIN_DATA_ UINT16 FMFont[]={SID_MANUAL, SID_PRESET, SID_MEM, SID_DEL, SID_AUTO};

#else
_ATTR_RADIOWIN_DATA_ EXT UINT16 RadioWinArea[4];
_ATTR_RADIOWIN_DATA_ EXT UINT16 FMFont[];

#endif
/******************************************************************************/
/*                                                                            */
/*                          Function Define                                   */
/*                                                                            */
/******************************************************************************/

EXT void RadioWinInit(void *pArg);
EXT void RadioWinIntDeInit(void);
EXT void RadioWinMsgInit(void *pArg);
EXT void RadioWinMsgDeInit(void);

EXT void RadioWinInit(void *pArg);
EXT void RadioWinDeInit(void);

EXT UINT32 RadioWinService(void);
EXT UINT16 FmSaveAndDel(void);
EXT void RadioWinPaint(void);
EXT void RadioGetGuaVal(void);
EXT void RadioRefreshFreq(void);

EXT UINT32 RadioWinKeyProc(void);


/******************************************************************************/
/*                                                                            */
/*                    Description:  window sturcture definition               */
/*                                                                            */
/******************************************************************************/
#ifdef _IN_RADIOWIN_
_ATTR_RADIOWIN_DATA_ WIN RadioWin =
{

    NULL,
    NULL,

    RadioWinService,             //window service handle function.
    RadioWinKeyProc,             //window key service handle function.
    RadioWinPaint,               //window display service handle function.

    RadioWinInit,                //window initial handle function.
    RadioWinDeInit               //window auti-initial handle function.

};
#else
_ATTR_RADIOWIN_DATA_ EXT WIN RadioWin;
#endif

/*
********************************************************************************
*
*                         End of RadioWin.h
*
********************************************************************************
*/
#endif



