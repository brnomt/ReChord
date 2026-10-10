/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  SetMenuInterface.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SETMENUINTERFACE_H_
#define _SETMENUINTERFACE_H_

#undef  EXT
#ifdef  _SETMENUINTERFACE_IN_
#define EXT
#else
#define EXT extern
#endif

#define SetBatteryLevel               gBattery.Batt_Level
#define SetHoldState                  0

//system configuration
#define  SetLanguage            gSysConfig.SysLanguage
#define  SetLightOffTime        gSysConfig.BLtime
#define  SetBackLightLevel      gSysConfig.BLevel
#define  SetBackLightMode       gSysConfig.BLmode
#define  SetPowerOffTime        gSysConfig.ShutTime
#ifdef _MUSIC_
#define  SetMusicPlayOrder      gSysConfig.MusicConfig.PlayOrder
#define  SetMusicPlayMode       gSysConfig.MusicConfig.RepeatMode
#define  SetEqType              gSysConfig.MusicConfig.Eq.Mode
#define  SysSetUserEq           gSysConfig.MusicConfig.Eq.RKCoef.dbGain
#endif

#ifdef _RADIO_
#define  SetRadioArea           gSysConfig.RadioConfig.FmArea
#define  SetRadioStereo         gSysConfig.RadioConfig.FmStereo
#define  SetScanSensitivity     gSysConfig.RadioConfig.ScanSensitivity
#endif

#ifdef _RECORD_
#define  SetRecordQuality       gSysConfig.RecordConfig.RecordQuality
#define  SetRecordVol           gSysConfig.RecordConfig.RecordVol
#define  SetRecordFormat        gSysConfig.RecordConfig.RecordFmt           //add for record format
#endif

#ifdef _EBOOK_
#define  SetTextAutoBrowse      gSysConfig.TextConfig.AutoPlay              //automatic browser switch
#define  SetTextAutoBrowseTime  gSysConfig.TextConfig.AutoPlayTime          //automatic browser time
#endif

#ifdef _RADIO_
#define  SetRadioArea           gSysConfig.RadioConfig.FmArea
#define  SetRadioStereo         gSysConfig.RadioConfig.FmStereo
#endif

#ifdef _PICTURE_
#define  SetPicBroSwitch        gSysConfig.PicConfig.AutoPlaySwitch
#define  SetPicBroTimer         gSysConfig.PicConfig.AutoPlayTime
#endif

_ATTR_SYS_SET_COMMON_BSS_ EXT UINT32  SetUserEq[5];

EXT void SetFirmwareUpgrade(void);

EXT void SetLightOffTimerReset(void);

EXT void SetLcdBackLightOn(void);

EXT void SetPowerOffTimerReset(void);

EXT uint32 SetGetFirmwareVersion(uint16 *Year,
                                 UINT16 *Date,
                                 UINT16 *MasterVer,
                                 UINT16 *SlaveVer,
                                 UINT16 *SmallVer);
extern void SetLightOffTimerResetClone(void);
extern void SetLcdBackLightOnClone(void);
extern void SetPowerOffTimerResetClone(void);
/*
********************************************************************************
*
*                         End of SetMenuInterface.h
*
********************************************************************************
*/
#endif

