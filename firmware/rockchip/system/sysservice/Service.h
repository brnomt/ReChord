/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   SysService.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SYSSERVICE_H_
#define _SYSSERVICE_H_

#undef  EXT
#ifdef _IN_SYSSERVICE_
#define EXT
#else
#define EXT extern
#endif

typedef enum
{
    Voice_PLAY,
    Voice_STOP
}VoiceNotifySTATE;

typedef enum
{
	BEEP_PLAY = 0,
	BEEP_AMS_PLUS,
	BEEP_AMS_MINUS,
	BEEP_FM_PRESET_REGIST,
	BEEP_FM_PRESET_DELETE,
	BEEP_LOW_BATTERY,
	BEEP_AMS_RETURN,
	BEEP_STOP,
	BEEP_NUM

}BEEP_ID;


_ATTR_SYS_BSS_ EXT  UINT8   SetPowerOffFlag;
_ATTR_SYS_BSS_ EXT  INT8    AutoPowerOffDisableCounter;
_ATTR_SYS_BSS_ EXT  uint32 DemoModePauseSystickCounterBack;

_ATTR_SYS_BSS_ EXT  UINT16  AudioPlayState;
_ATTR_SYS_BSS_ EXT  UINT16  AudioPlayerState;
_ATTR_SYS_BSS_ EXT  UINT16  AudioCodecOpenErr;
_ATTR_SYS_BSS_ EXT  uint32  AudioPtr;
_ATTR_SYS_BSS_ EXT  uint32  AudioDecodeing;
_ATTR_SYS_BSS_ EXT  uint32  DmaTransting;
_ATTR_SYS_BSS_ EXT  uint32  AudioNeedDecode;
_ATTR_SYS_BSS_ EXT  uint32  AudioLen;
_ATTR_SYS_BSS_ EXT  UINT32  AudioDecodeCnt;
_ATTR_SYS_BSS_ EXT  UINT32  Acodec_over_cur_value;
_ATTR_SYS_BSS_ EXT  UINT32  Acodec_over_cur_count;
_ATTR_SYS_BSS_ EXT  UINT16      REC_Flag;                   // flag to store updating status for recording;

#ifdef _RADIO_
//HJ
_ATTR_SYS_BSS_  EXT UINT32   FmFunctionSele;
_ATTR_SYS_BSS_  EXT UINT16   FmListSaveAndDel;
#endif

/*-----------------------------------------------------------------------------*/
//Service.c
extern void SysServiceInit(void);
extern UINT32 SysService(void);

extern void AutioPowerOffTimerRest(void);
extern void AutoPowerOffEnable(void);
extern void AutoPowerOffDisable(void);
extern void SetPowerOffTimerEnable(void);
extern void SetPowerOffTimerDisable(void);

extern void AudioDmaIsrHandler(void);
extern void AudioDecoding(void);
extern UINT8 Audio_HP_DET(void);
extern uint8 ACodec_Get_Over_Current(void);

/*
********************************************************************************
*
*                         End of SysService.h
*
********************************************************************************
*/
#endif
