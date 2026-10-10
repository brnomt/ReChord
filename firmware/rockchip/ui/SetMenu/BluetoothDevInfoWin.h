/*
********************************************************************************
*                   Copyright (C),2004-2016, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  BluetoothDevInfoWin.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             chad.ma
*    desc:    ORG.
********************************************************************************
*/

#ifndef _BLUETOOTH_DEV_INFO_WIN_H_
#define _BLUETOOTH_DEV_INFO_WIN_H_

#undef  EXT
#ifdef _IN_BLUETOOTH_DEV_INFO_WIN_
#define EXT
#else
#define EXT extern
#endif


/******************************************************************************/
/*                                                                                                                                         */
/*                          Macro Define                                                                                              */
/*                                                                                                                                         */
/******************************************************************************/

/******************************************************************************/
/*                                                                                                                                         */
/*                          Variable Define                                                                                           */
/*                                                                                                                                         */
/******************************************************************************/

/******************************************************************************/
/*                                                                                                                                         */
/*                          Function Define                                                                                          */
/*                                                                                                                                         */
/******************************************************************************/
EXT void BTLocalDevInfoWinInit(void *pArg);

EXT void BTLocalDevInfoWinDeInit(void);

EXT UINT32 BTLocalDevInfoWinService(void);

EXT UINT32 BTLocalDevInfoWinKeyProc(void);

EXT void BTLocalDevInfoWinPaint(void);

char * BtMacAddr2String(uint8* macAddr);

EXT void BTPariedDevInfoWinInit(void *pArg);

EXT void BTPariedDevInfoWinDeInit(void);

EXT UINT32 BTPariedDevInfoWinService(void);

EXT UINT32 BTPariedDevInfoWinKeyProc(void);

EXT void BTPariedDevInfoWinPaint(void);

/******************************************************************************/
/*                                                                                                                                         */
/*                    Description:  window sturcture definition                                                               */
/*                                                                                                                                         */
/******************************************************************************/
#ifdef _IN_BLUETOOTH_DEV_INFO_WIN_
//for local device info.
_ATTR_SYS_SET_BT_DATA_ WIN BTLocalDevInfoWin =
{
    NULL,
    NULL,

    BTLocalDevInfoWinService,             //window service handle function.
    BTLocalDevInfoWinKeyProc,             //window key service handle function.
    BTLocalDevInfoWinPaint,               //window display service handle function.

    BTLocalDevInfoWinInit,                //window initial handle function.
    BTLocalDevInfoWinDeInit,              //window auti-initial handle function.

};

//for remote paried device info
_ATTR_SYS_SET_BT_DATA_ WIN BTPariedDevInfoWin =
{
    NULL,
    NULL,

    BTPariedDevInfoWinService,             //window service handle function.
    BTPariedDevInfoWinKeyProc,             //window key service handle function.
    BTPariedDevInfoWinPaint,               //window display service handle function.

    BTPariedDevInfoWinInit,                //window initial handle function.
    BTPariedDevInfoWinDeInit,              //window auti-initial handle function.

};

#else
_ATTR_SYS_SET_BT_CODE_ EXT WIN BTLocalDevInfoWin;
_ATTR_SYS_SET_BT_CODE_ EXT WIN BTPariedDevInfoWin;
#endif

/*
********************************************************************************
*
*                         End of BlueToothDevInfoWin.h
*
********************************************************************************
*/
#endif



