/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   Main.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#ifdef _MUSIC_
#include "effect.h"
#endif

#undef  EXT
#ifdef _IN_MAIN_
#define EXT
#else
#define EXT extern
#endif
//PAGE
#define SN_Length 20 //The MAX lebgth of Serial Number

#define SYS_SUPPROT_STRING_MAX_LEN 255 // sys support string max len(31, 63, 127, 255(windows))

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/
//section define
#define _ATTR_SYS_CODE_         
#define _ATTR_SYS_DATA_         
#define _ATTR_SYS_BSS_          

#define _ATTR_SYS_INIT_CODE_    
#define _ATTR_SYS_INIT_DATA_    
#define _ATTR_SYS_INIT_BSS_     

#define _ATTR_SYS_REBOOT_BSS_   


#define _ATTR_BB_SYS_CODE_          
#define _ATTR_BB_SYS_DATA_          
#define _ATTR_BB_SYS_BSS_           

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/


//system keeping information for music module
#ifdef _MUSIC_
typedef __packed struct _MUSICCONFIG
{
    uint16 FileNum;                 //current file number.
    uint32 CurTime;                 //current time.
    uint8  HoldOnPlaySaveFlag;
    uint8  RepeatMode;              //repeat mode
    uint8  RepeatModeBak;
    uint8  PlayOrder;               //play order
    uint32 ShuffleSeed;
    RKEffect Eq;                    //Eq
    UINT32 HoldMusicFullInfoSectorAddr ;
    UINT32 HoldMusicSortInfoSectorAddr;
    UINT16 HoldMusicuiBaseSortId[4];
    UINT16 HoldMusicPlayType ;
    UINT16 HoldMusicTypeSelName[SYS_SUPPROT_STRING_MAX_LEN + 1];
    UINT16 HoldMusicucCurDeep ;
    UINT16 HoldMusicuiCurId[4]  ;
    UINT16 HoldMusicTotalFiles;
    UINT16 HoldClusSave;
}MUSIC_CONFIG;
#endif

typedef __packed struct _SYS_MEDIABRO_DIR_TREE_STRUCT //这个结构必须和媒体那边定义的结构相同
{
    UINT8  Flag;//是否需要使用该结构体数据恢复资源信息的标记
    UINT8  MediaType;
    UINT16 MusicDirBaseSortId[4];
    UINT16 MusicDirDeep;
    UINT16 CurId[4];

}SYS_MEDIABRO_DIR_TREE_STRUCT;//TestTreeStruct;

//system keeping information for video module
#ifdef _VIDEO_
typedef __packed struct _VIDEOCONFIG
{
    uint16 FileNum;                 //current file number.
    uint32 Video_Current_FrameNum;  //current time
    uint8  HoldOnPlaySaveFlag;
}VIDEO_CONFIG;
#endif

//system keeping information for FM module
#ifdef _RADIO_
typedef __packed struct _RADIOCONFIG
{
    uint8  FmArea;                  //receive area.
    uint8  FmStereo;                //stereo switch
    uint8  FmState;                 //current status, hand or auto,
    uint8  FmSaveNum;               //current saved stations number.
    uint16 FmFreq;                  //current saved station frequency.
    uint16 FmFreqArray[40];         //saved station frequency group.
    uint8  ScanSensitivity;
    uint8  HoldOnPlaySaveFlag;

}RADIO_CONFIG;
#endif

//system keeping information for record module
#ifdef _RECORD_
typedef __packed struct _RECORDCONFIG
{
    uint8 RecordVol;                //record volume
    uint8 RecordQuality;            //record quality
    uint8 RecordFmt;                //record format :normal -- mp3/wav(adpcm), high quality -- pcm
    uint16 RecordIndex;

}RECORD_CONFIG;
#endif

//system keeping information for ebook module
#ifdef _EBOOK_
typedef __packed struct _TEXTCONFIG
{
    uint16 FileNum;                 //current file number it is use for breakpoint recover.

    uint8 AutoPlay;                 //automatic browse switch
    uint8 AutoPlayTime;             //automatic browse time

}TEXT_CONFIG;
#endif

//system keeping information for picture module
#ifdef _PICTURE_
typedef __packed struct _PICTURECONFIG
{
    uint16 FileNum;                 //current file number it is use for breakpoint recover.

    uint8 FromWhere;
    uint8 AutoPlaySwitch;
    uint8 AutoPlayTime;
    uint16 CurId;
    uint16 KeyCnt;

}PICTURE_CONFIG;
#endif

#ifdef _BLUETOOTH_
#define  BT_LINK_KEY_MAX_NUM    6    // do not modify, if you want to modify, must be Notice RK
#define  BT_LOCAL_DEV_NAME_MAX  30
typedef __packed struct _BT_LINK_KEY
{
    uint8 BdAddr[6];                 //current file number it is use for breakpoint recover.
    uint8 LinkKey[16];           //automatic browse switch
    #ifdef _A2DP_SOUCRE_
    uint8 name[32+1];           //automatic browse switch
    #endif
    uint8 KeyTpye;
    uint16 A2dpVersion;
    uint16 AvrcpVersion;

}BT_LINK_KEY;

typedef __packed struct _BLUETOOTHCONFIG
{
    uint8 LastConnectMac[6];                 //current file number it is use for breakpoint recover.
    uint8 LocalBTMacAddr[6];
    char LocalBTName[BT_LOCAL_DEV_NAME_MAX];

    BT_LINK_KEY BtLinkKey[BT_LINK_KEY_MAX_NUM];           //automatic browse switch
    uint8 KeyIndex;                     //automatic browse time
    uint8 PairedDevCnt;
    uint8 btConnected;
    uint8 btOpened;
}BLUETOOTH_CONFIG;
#endif

typedef __packed struct _MEDIALIBCONFIG
{
    UINT16 gMusicFileNum;       // the total number of all music.
    UINT16 gID3TitleFileNum;    // the file number who has id3 title infomation.
    UINT16 gID3ArtistFileNum;   // the file number who has id3 artist infomation.
    UINT16 gID3AlbumFileNum;    // the file number who has id3 album infomation.
    UINT16 gID3GenreFileNum;    // the file number who has id3 genre infomation.
    UINT16 gMyFavoriteFileNum;  // the file number of stroe folder.
    UINT16 gRecordFmFileNum;
    UINT32 gRecordVoiceFileNum;
    UINT16 gTotalFileNum;
    UINT16 MediaUpdataFlag;     // the flag that media libary had update.

    UINT32 gJpegFileNum;
    UINT16 gJpegTotalFileNum;

}MEDIALIB_CONFIG;


typedef __packed struct _SYS_PICBRO_DIR_TREE_STRUCT
{
    UINT8  Flag;
    UINT8  MediaType;
    UINT16 DirBaseSortId[4];
    UINT16 DirDeep;
    UINT16 CurId[4];

}SYS_PICBRO_DIR_TREE_STRUCT;//TestTreeStruct;


//system setting parameter structure definition.
typedef __packed struct _SYSCONFIG
{
    //////////////////////////////////////////////////
    // don't change followd
    uint32 FirmwareFlag;            // "SAVE"

    uint16 FirmwareYear;
    uint16 FirmwareDate;
    uint16 MasterVersion;
    uint16 SlaveVersion;
    uint16 SmallVersion;

    // don't change above
    //////////////////////////////////////////////////

    uint8  Memory;                  //storage media options.
    UINT8  SysLanguage;             //current system language environment.
    uint8  ImageIndex;

    UINT8  OutputVolume;            //system volume
    UINT8  BLmode;                  //backlight display mode.
    UINT8  BLevel;                  //backlight level
    UINT8  BLtime;                  //backlight time.
    UINT8  LockTime;                //lock keyboard time
    UINT8  ShutTime;                //auto close time
    UINT8  SDEnable;                //sd card select
    UINT8  FMEnable;                //FM setting menu judge.
    UINT8  KeyNum;                  //it is used for 5.6.7 keys judgement.
    UINT32 dwTotalMem;
    UINT32 dwFreeMem;

    UINT8  EQEnable;                  //it is used for 5.6.7 keys judgement.
    UINT8  AudioOutputMode;

#ifdef _CDROM_
    UINT8  bShowCdrom;
#endif

    MEDIALIB_CONFIG MedialibPara;
    SYS_MEDIABRO_DIR_TREE_STRUCT MediaDirTreeInfo;

#ifdef _MUSIC_
    MUSIC_CONFIG MusicConfig;
#endif

#ifdef _VIDEO_
    VIDEO_CONFIG VideoConfig;
#endif

#ifdef _RADIO_
    RADIO_CONFIG RadioConfig;
#endif

#ifdef _RECORD_
    RECORD_CONFIG RecordConfig;
#endif

#ifdef _EBOOK_
    TEXT_CONFIG  TextConfig;
#endif

#ifdef _PICTURE_
    PICTURE_CONFIG  PicConfig;

    SYS_PICBRO_DIR_TREE_STRUCT JpegMediaDirTreeInfo;

#endif

#ifdef _BLUETOOTH_
    BLUETOOTH_CONFIG BtConfig;
#endif
    UINT8  DemoMode; // Enable:1, Disable:0
    UINT8  ProfuctionTestMode; // Enable:1, Disable:0
    UINT8  FMmodule; // FMmodule:1, NO FMmodule:0

#ifdef _USB_HOST_
    uint8  UsbHost;
    uint8  Listen_UsbHost;
#endif
    uint8  FindFileType;
    // this variable must at "END"
    uint32 SafePoweroff;
    uint32 FirmwareCheck;            // "INFO"

    uint32 crc;    // don't change
} SYSCONFIG,*PSYSCONFIG;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/
//Firmware Info
_ATTR_SYS_BSS_  EXT uint32      Font12LogicAddress;
_ATTR_SYS_BSS_  EXT uint32      Font16LogicAddress;

_ATTR_SYS_BSS_  EXT uint32      IDBlockByteAddress;
_ATTR_SYS_BSS_  EXT uint32      CodeLogicAddress;
_ATTR_SYS_BSS_  EXT uint32      FontLogicAddress;
_ATTR_SYS_BSS_  EXT uint32      GBKLogicAddress;
_ATTR_SYS_BSS_  EXT uint32      MenuLogicAddress;
_ATTR_SYS_BSS_  EXT uint32      ImageLogicAddress;

#ifdef _MULT_DISK_
_ATTR_SYS_BSS_  EXT BOOL        bShowFlash1;
#endif

#ifdef _CDROM_
_ATTR_SYS_BSS_  EXT BOOL        bShowCdrom;
#endif

_ATTR_SYS_BSS_  EXT uint32      DCDCLastLevel;
_ATTR_SYS_BSS_  EXT uint32      DCDCOutputVol;
_ATTR_SYS_BSS_  EXT uint32      LDOLastLevel;
_ATTR_SYS_BSS_  EXT uint32      LDOOutputVol;
_ATTR_SYS_BSS_  EXT volatile  uint32      SysTickCounter;  //system clock counter,add one every 10 ms.
_ATTR_SYS_BSS_  EXT SYSCONFIG   gSysConfig;


#ifdef _MEDIA_MODULE_
   EXT _ATTR_SYS_BSS_ UINT16 gMusicTypeSelID; // add by phc,
   EXT _ATTR_SYS_BSS_ UINT16 ID3TypeSelFlag; // add by phc, for ID3Type select
   EXT _ATTR_SYS_BSS_ UINT16 FileSortUpdateKey; //whether to operate usb disk.
   EXT _ATTR_SYS_BSS_ UINT32 gFavoBlockSectorAddr;
   EXT _ATTR_SYS_BSS_ UINT16 gDisSortedMusicFlag;    //
#endif

EXT _ATTR_SYS_BSS_ UINT16 gMusicTypeSelName[SYS_SUPPROT_STRING_MAX_LEN + 1];    //zyz: size must = MEDIA_ID3_SAVE_CHAR_NUM
EXT _ATTR_SYS_BSS_ UINT16 gMusicTypeSelID; // add by phc,
EXT _ATTR_SYS_BSS_ UINT32 MediaInfoAddr;  //media information store start address.

EXT _ATTR_SYS_BSS_ UINT16 gJpegTypeSelName[SYS_SUPPROT_STRING_MAX_LEN + 1];    //zyz: size must = MEDIA_ID3_SAVE_CHAR_NUM

_ATTR_SYS_BSS_  EXT BOOL        IsSetmenu;
_ATTR_SYS_BSS_  EXT BOOL        BroswerFlag;//the flag that is distinguish return to media or explorer
_ATTR_SYS_BSS_  EXT UINT16      GlobalFilenum;

_ATTR_SYS_BSS_  EXT INT8       OutputVolOffset;//it is use to modify volume after eq setting.
_ATTR_SYS_BSS_  EXT UINT8       MaxShuffleAllCount;
#define  MemorySelect           gSysConfig.Memory
#define  gbTextFileNum          gSysConfig.TextConfig.FileNum
#define  gbAudioFileNum         gSysConfig.MusicConfig.FileNum
#define  gbVideoFileNum         gSysConfig.VideoConfig.FileNum
#define  gbPicFileNum           gSysConfig.PicConfig.FileNum //an 4.27 picture mode.

#define  gbPicCurid             gSysConfig.PicConfig.CurId
#define  gbPicKeyCnt            gSysConfig.PicConfig.KeyCnt
#define  gbPicFromWhere         gSysConfig.PicConfig.FromWhere

#ifdef _BLUETOOTH_
_ATTR_SYS_BSS_  EXT BOOL        IsBTOpened;
_ATTR_SYS_BSS_  EXT BOOL        gBTFisrtScan;
//#define  SetBTSwitch            gSysConfig.BtConfig.btOpened        //add for bt swith on / off
#define  gbBTSwitchOn           gSysConfig.BtConfig.btOpened        //add for bt swith on / off
#define  gbBTConnected          gSysConfig.BtConfig.btConnected     //add for bt connect successs / fail
#endif

#ifdef _IN_MAIN_
_ATTR_SYS_DATA_ EXT char        FileOpenStringR[]      = "R";
_ATTR_SYS_DATA_ EXT char        ALLFileExtString[]     = "*";
_ATTR_SYS_DATA_ EXT char        AudioFileExtString[]   = "MP1MP2MP3REVWAVAPEFLAAACM4AOGGMP43GPDFFDSF";
_ATTR_SYS_DATA_ EXT char        MusicFileExtString[]   = "MP1MP2MP3REVWAVAPEFLAAACM4AOGGMP43GPDFFDSF";
_ATTR_SYS_DATA_ EXT char        RecordFileExtString[]  = "WAV";
_ATTR_SYS_DATA_ EXT char        TextFileExtString[]    = "TXTLRC";
_ATTR_SYS_DATA_ EXT char        VideoFileExtString[]   = "AVI";
_ATTR_SYS_DATA_ EXT char        PictureFileExtString[] = "JPGJPEBMP";//JPE it is used judge jpeg picture.
#else
_ATTR_SYS_DATA_ EXT char        FileOpenStringR[];
_ATTR_SYS_DATA_     EXT char    ALLFileExtString[];
_ATTR_SYS_DATA_ EXT char        AudioFileExtString[];
_ATTR_SYS_DATA_ EXT char        MusicFileExtString[];
_ATTR_SYS_DATA_ EXT char        RecordFileExtString[];
_ATTR_SYS_DATA_ EXT char        TextFileExtString[];
_ATTR_SYS_DATA_ EXT char        VideoFileExtString[];
_ATTR_SYS_DATA_ EXT char        PictureFileExtString[];

#endif
_ATTR_SYS_DATA_ EXT BOOL        FmStandbyFlag;
_ATTR_SYS_DATA_ EXT BOOL        gIsMusicWinFlg;

_ATTR_SYS_REBOOT_BSS_ EXT uint32  RebootTag[4];

//set language information group of real support in menu.this information is the offset of language in explorer.
_ATTR_SYS_BSS_  EXT UINT8       SetMenuLanguageInfo[LANGUAGE_MAX_COUNT];
_ATTR_SYS_BSS_  EXT UINT8       DefaultLanguage;//default language item number it is used to locate default language cursor in menu display.
_ATTR_SYS_BSS_  EXT UINT8       LanguageNum;//set the real suppot language number.
_ATTR_SYS_BSS_  EXT UINT8       DefaultLanguageID;

_ATTR_SYS_BSS_  EXT UINT8       gLangSel;

#ifdef _FILE_DEBUG_
typedef int32   (*pFUN_DEBUG_HOOK)(char ch);
_ATTR_SYS_BSS_  EXT pFUN_DEBUG_HOOK pDebugHook;
#endif

/*
********************************************************************************
*
*                         End of Main.h
*
********************************************************************************
*/
#endif
