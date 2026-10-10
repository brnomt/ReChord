/*
********************************************************************************
*                   Copyright (C),2004-2016, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  BluetoothScanWin.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             chad.ma
*    desc:    ORG.
********************************************************************************
*/

#ifndef _BLUETOOTH_SCAN_WIN_H_
#define _BLUETOOTH_SCAN_WIN_H_

#undef  EXT
#ifdef _IN_BLUETOOTH_SCAN_WIN_
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

EXT void BluetoothScanWinSvcStart(void *pArg);

EXT void BluetoothScanWinIntInit(void *pArg);

EXT void BluetoothMsgIntInit(void *pArg);

EXT void BluetoothScanWinMsgDeInit(void *pArg);

EXT void BluetoothScanWinInit(void *pArg);

EXT void BluetoothScanWinDeInit(void);

EXT UINT32 BluetoothScanWinService(void);

EXT UINT32 BluetoothScanWinKeyProc(void);

EXT void BluetoothScanWinPaint(void);


/******************************************************************************/
/*                                                                                                                                         */
/*                    Description:  window sturcture definition                                                               */
/*                                                                                                                                         */
/******************************************************************************/
#ifdef _IN_BLUETOOTH_SCAN_WIN_
_ATTR_SYS_SET_BT_DATA_ WIN BlueToothScanWin =
{
    NULL,
    NULL,

    BluetoothScanWinService,             //window service handle function.
    BluetoothScanWinKeyProc,             //window key service handle function.
    BluetoothScanWinPaint,               //window display service handle function.

    BluetoothScanWinInit,                //window initial handle function.
    BluetoothScanWinDeInit,              //window auti-initial handle function.

};
#else
_ATTR_SYS_SET_BT_CODE_ EXT WIN BlueToothScanWin;
#endif

/*
********************************************************************************
*
*                         End of BlueToothScanWin.h
*
********************************************************************************
*/
#endif



