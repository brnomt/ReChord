/* service_globals.h — audio service globals (Service.c needs these).
 * Separate from Service.h (which redefines EXT and breaks inclusion). */
#ifndef SERVICE_GLOBALS_H
#define SERVICE_GLOBALS_H
#include "typedef.h"
#include "Thread.h"
#include "Win.h"

extern UINT8   SetPowerOffFlag;
extern INT8    AutoPowerOffDisableCounter;
extern UINT16  AudioPlayState;
extern UINT16  AudioCodecOpenErr;
extern uint32  AudioPtr;
extern uint32  AudioDecodeing;
extern uint32  DmaTransting;
extern uint32  AudioNeedDecode;
extern uint32  AudioLen;
extern UINT32  AudioDecodeCnt;

extern THREAD FMThread;
extern THREAD RecordThread;
extern THREAD BlueToothThread;
extern WIN LowPowerWin;
typedef struct {
    uint32 FmState;
} RADIO_REG_KEY;
extern RADIO_REG_KEY *gpRadioplayerRegKey;


extern uint32 FM_State_StepStation;

/* BOOT_SECTOR_T removed 2026-10-09: the vendor filesys/fat.h owns
 * `BootSector` (_FAT_BOOTSECTOR, with BPB_RootClus). This synthesized copy
 * had zero users and collided with it. */
extern uint32 FileDiskID;

extern UINT16 AudioPlayerState;
extern void AudioDmaIsrHandler(void);


#include "FileInfo.h"


extern void AudioDecoding(void);

#endif /* SERVICE_GLOBALS_H */
