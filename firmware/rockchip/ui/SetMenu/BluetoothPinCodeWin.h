/*
********************************************************************************
*                   Copyright (C),2004-2016, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:  BluetoothPinCodeWin.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             chad.ma
*    desc:    ORG.
********************************************************************************
*/

#ifndef _BLUETOOTH_PIN_CODE_WIN_H_
#define _BLUETOOTH_PIN_CODE_WIN_H_

#undef  EXT
#ifdef _IN_BLUETOOTH_PIN_CODE_WIN_
#define EXT
#else
#define EXT extern
#endif


/******************************************************************************/
/*                                                                                                                                         */
/*                          Macro Define                                                                                              */
/*                                                                                                                                         */
/******************************************************************************/
#define BT_PIN_CODE_NUM_MAX        8

#define BT_PIN_KEY_BUTTON_YES             1
#define BT_PIN_KEY_BUTTON_NO              0
/******************************************************************************/
/*                                                                                                                                         */
/*                          Variable Define                                                                                           */
/*                                                                                                                                         */
/******************************************************************************/

typedef struct
{
    uint8 pin_val;
    bool  changed;
}BT_PINCODESs;

_ATTR_SYS_SET_BT_BSS_ EXT UINT8 curPincodeXcorVal;
_ATTR_SYS_SET_BT_BSS_ EXT UINT8 curPincodeYcorVal;
_ATTR_SYS_SET_BT_BSS_ EXT UINT8 gPinCode[BT_PIN_CODE_NUM_MAX + 1];
_ATTR_SYS_SET_BT_BSS_ EXT BT_PINCODESs BTPinCodes[BT_PIN_CODE_NUM_MAX];


_ATTR_SYS_SET_BT_BSS_  EXT UINT16    pinKeyButtonSel;
_ATTR_SYS_SET_BT_BSS_  EXT UINT16    pinKeyTitleID;
_ATTR_SYS_SET_BT_BSS_  EXT UINT16    pinKeyContentID;

/******************************************************************************/
/*                                                                                                                                         */
/*                          Function Define                                                                                          */
/*                                                                                                                                         */
/******************************************************************************/
EXT void BTPinCodeWinInit(void *pArg);

EXT void BTPinCodeWinDeInit(void);

EXT UINT32 BTPinCodeWinService(void);

EXT UINT32 BTPinCodeWinKeyProc(void);

EXT void BTPinCodeWinPaint(void);


EXT void BTPinKeyCfmWinInit(void *pArg);

EXT void BTPinKeyCfmWinDeInit(void);

EXT UINT32 BTPinKeyCfmWinService(void);

EXT UINT32 BTPinKeyCfmWinKeyProc(void);

EXT void BTPinKeyCfmWinPaint(void);


/******************************************************************************/
/*                                                                                                                                         */
/*                    Description:  window sturcture definition                                                               */
/*                                                                                                                                         */
/******************************************************************************/
#ifdef _IN_BLUETOOTH_PIN_CODE_WIN_

//for simple paried input pin code
_ATTR_SYS_SET_BT_DATA_ WIN BTPinCodeWin =
{
    NULL,
    NULL,

    BTPinCodeWinService,             //window service handle function.
    BTPinCodeWinKeyProc,             //window key service handle function.
    BTPinCodeWinPaint,               //window display service handle function.

    BTPinCodeWinInit,                //window initial handle function.
    BTPinCodeWinDeInit,              //window auti-initial handle function.

};

//for simple paried confirm pin key
_ATTR_SYS_SET_BT_DATA_ WIN BTPinKeyCfmWin =
{
    NULL,
    NULL,

    BTPinKeyCfmWinService,             //window service handle function.
    BTPinKeyCfmWinKeyProc,             //window key service handle function.
    BTPinKeyCfmWinPaint,               //window display service handle function.

    BTPinKeyCfmWinInit,                //window initial handle function.
    BTPinKeyCfmWinDeInit,              //window auti-initial handle function.

};

#else
_ATTR_SYS_SET_BT_CODE_ EXT WIN BTPinCodeWin;
_ATTR_SYS_SET_BT_CODE_ EXT WIN BTPinKeyCfmWin;
#endif

/*
********************************************************************************
*
*                         End of BluetoothPinCodeWin.h
*
********************************************************************************
*/
#endif

