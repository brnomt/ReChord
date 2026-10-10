/*
********************************************************************************
*                   Copyright (c) 2008,Rockchips
*                         All rights reserved.
*
* File Name��  SysReservedOperation.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:
********************************************************************************
*/
#define _IN_SYSRESERVED_OP_

#include "SysInclude.h"
#include "fsinclude.h"
#include "SysReservedOperation.h"

#ifdef _MUSIC_
#include "AudioControl.h"
#include "SysFindFile.h"
#endif

#ifdef _VIDEO_
#include "VideoControl.h"
#endif

#include "medialibwin.h"
#include "SetMenu.h"   /* DIS_EQ_* mode enum */

/*
--------------------------------------------------------------------------------
  Function name : uint32 FindIDBlock(void)
  Author        : ZHengYongzhi
  Description   : find IDBlock

  Input         :
  Return        : IDBlock OR 0xffffffff

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYSRESERVED_OP_DATA_
static  char crc_table[256] =
{
    0x00, 0x1D, 0x3A, 0x27, 0x74, 0x69, 0x4E, 0x53,
    0xE8, 0xF5, 0xD2, 0xCF, 0x9C, 0x81, 0xA6, 0xBB,
    0xCD, 0xD0, 0xF7, 0xEA, 0xB9, 0xA4, 0x83, 0x9E,
    0x25, 0x38, 0x1F, 0x02, 0x51, 0x4C, 0x6B, 0x76,
    0x87, 0x9A, 0xBD, 0xA0, 0xF3, 0xEE, 0xC9, 0xD4,
    0x6F, 0x72, 0x55, 0x48, 0x1B, 0x06, 0x21, 0x3C,
    0x4A, 0x57, 0x70, 0x6D, 0x3E, 0x23, 0x04, 0x19,
    0xA2, 0xBF, 0x98, 0x85, 0xD6, 0xCB, 0xEC, 0xF1,
    0x13, 0x0E, 0x29, 0x34, 0x67, 0x7A, 0x5D, 0x40,
    0xFB, 0xE6, 0xC1, 0xDC, 0x8F, 0x92, 0xB5, 0xA8,
    0xDE, 0xC3, 0xE4, 0xF9, 0xAA, 0xB7, 0x90, 0x8D,
    0x36, 0x2B, 0x0C, 0x11, 0x42, 0x5F, 0x78, 0x65,
    0x94, 0x89, 0xAE, 0xB3, 0xE0, 0xFD, 0xDA, 0xC7,
    0x7C, 0x61, 0x46, 0x5B, 0x08, 0x15, 0x32, 0x2F,
    0x59, 0x44, 0x63, 0x7E, 0x2D, 0x30, 0x17, 0x0A,
    0xB1, 0xAC, 0x8B, 0x96, 0xC5, 0xD8, 0xFF, 0xE2,
    0x26, 0x3B, 0x1C, 0x01, 0x52, 0x4F, 0x68, 0x75,
    0xCE, 0xD3, 0xF4, 0xE9, 0xBA, 0xA7, 0x80, 0x9D,
    0xEB, 0xF6, 0xD1, 0xCC, 0x9F, 0x82, 0xA5, 0xB8,
    0x03, 0x1E, 0x39, 0x24, 0x77, 0x6A, 0x4D, 0x50,
    0xA1, 0xBC, 0x9B, 0x86, 0xD5, 0xC8, 0xEF, 0xF2,
    0x49, 0x54, 0x73, 0x6E, 0x3D, 0x20, 0x07, 0x1A,
    0x6C, 0x71, 0x56, 0x4B, 0x18, 0x05, 0x22, 0x3F,
    0x84, 0x99, 0xBE, 0xA3, 0xF0, 0xED, 0xCA, 0xD7,
    0x35, 0x28, 0x0F, 0x12, 0x41, 0x5C, 0x7B, 0x66,
    0xDD, 0xC0, 0xE7, 0xFA, 0xA9, 0xB4, 0x93, 0x8E,
    0xF8, 0xE5, 0xC2, 0xDF, 0x8C, 0x91, 0xB6, 0xAB,
    0x10, 0x0D, 0x2A, 0x37, 0x64, 0x79, 0x5E, 0x43,
    0xB2, 0xAF, 0x88, 0x95, 0xC6, 0xDB, 0xFC, 0xE1,
    0x5A, 0x47, 0x60, 0x7D, 0x2E, 0x33, 0x14, 0x09,
    0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C,
    0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4
};

_ATTR_SYSRESERVED_OP_CODE_
static char sbc_crc8(const char *data, size_t len)  // len��λΪbit
{
    char crc = 0x0f;
    size_t i;
    char octet;

    for (i = 0; i < len / 8; i++)
        crc = crc_table[crc ^ data[i]];

    octet = data[i];

    for (i = 0; i < len % 8; i++)
    {
        char bit = ((octet ^ crc) & 0x80) >> 7;

        crc = ((crc & 0x7f) << 1) ^ (bit ? 0x1d : 0);

        octet = octet << 1;
    }

    return crc;
}

_ATTR_SYS_INIT_CODE_
void LoadSysInformation(void)
{
    uint32 i, j = 0;
    uint32 Len;
    uint32 SystemDefaultParaAddr;
    uint8  TempBuffer[2048];
    uint8  FlashBuf[512];

    uint16 FirmwareYear;
    uint16 FirmwareDate;
    uint16 MasterVersion;
    uint16 SlaveVersion;
    uint16 SmallVersion;
    uint32 MenuLogicAddress0, MenuLogicAddress1;

    uint32 mac_size;

    SYSCONFIG             *pSysConfig;
    FIRMWARE_INFO_T       *pFirmwareModuleInfo;
    SYSTEM_DEFAULT_PARA_T *pSystemDefaultPara;
    FIRMWARE_HEADER       *pFWHead;
    BOOL CleanHistortyFlag = FALSE;

    ////////////////////////////////////////////////////////////////////////////
    //read resource module address.
    pFWHead = (FIRMWARE_HEADER *)FlashBuf;

    if (ValidSysDisk == 0)
    {
        MDRead(SysDiskID, 0, 1, FlashBuf);
    }
    else
    {
        MDRead(SysDiskID, 0 + SysProgRawDiskCapacity, 1, FlashBuf);
    }

    CodeLogicAddress   = pFWHead->CodeInfo.CodeInfoTbl[SYS_CODE].ModuleOffset;
    GBKLogicAddress    = pFWHead->CharEncInfo.CharEncInfoTbl[GBK].ModuleOffset ;
    MenuLogicAddress0  = pFWHead->MenuInfo.MenuInfoTbl[0].ModuleOffset;
    MenuLogicAddress1  = pFWHead->MenuInfo.MenuInfoTbl[1].ModuleOffset;
    GetAsicTableAdd();

    ////////////////////////////////////////////////////////////////////////////
    //read system default parameter.
    MDReadData(SysDiskID, CodeLogicAddress, 512, FlashBuf);
    pFirmwareModuleInfo = (FIRMWARE_INFO_T *)FlashBuf;

    SystemDefaultParaAddr  = CodeLogicAddress + sizeof(pFirmwareModuleInfo -> LoadStartBase);
    SystemDefaultParaAddr += sizeof(pFirmwareModuleInfo -> ModuleInfo.ModuleNum);
    SystemDefaultParaAddr += pFirmwareModuleInfo -> ModuleInfo.ModuleNum * sizeof(CODE_INFO_T);
    MDReadData(SysDiskID, SystemDefaultParaAddr, sizeof(SYSTEM_DEFAULT_PARA_T), FlashBuf);
    pSystemDefaultPara = (SYSTEM_DEFAULT_PARA_T *)FlashBuf;

    ///////////////////////////////////////////////////////////////////////
    //read system setting parameters and judge version number.
#if(FW_IN_DEV == 3)
    pSysConfig = (SYSCONFIG *)TempBuffer;
    EmmcReadIDB(1, 4, TempBuffer);

    if ((TempBuffer[2048 - 4] == 'F') &&
        (TempBuffer[2048 - 3] == 'W') &&
        (TempBuffer[2048 - 2] == 'U') &&
        (TempBuffer[2048 - 1] == 'P'))
    {
        //zyz: SYSCONFIG Restore & Clear F/W upgrade Flag
        TempBuffer[2048 - 4] = 0xff;
        TempBuffer[2048 - 3] = 0xff;
        TempBuffer[2048 - 2] = 0xff;
        TempBuffer[2048 - 1] = 0xff;
        EmmcWriteIDB(1, 4, TempBuffer);
    }
    else
    {
        ReadReservedData(SYSDATA_BLK, TempBuffer, 2048);
    }

#else
    pSysConfig = (SYSCONFIG *)TempBuffer;
    ReadReservedData(SYSDATA_BLK, TempBuffer, 2048);

#endif

    // Check "SAVE INFO"
    {
        uint32 crcval;
        //crc������bitΪ��λ������ĸ��ֽڱ���crcֵ��������crcУ��
        crcval = sbc_crc8((const char *)pSysConfig, (sizeof(SYSCONFIG) - 4) * 8);
        DEBUG("crcval = %d, pconfig->crc = %d", crcval, pSysConfig->crc);

        if (crcval != pSysConfig->crc)
        {
            memset(TempBuffer, 0xff, 2048);
#if(FW_IN_DEV == 3)
#ifdef ENABLE_MBR
            SendMsg(MSG_FORMAT_DISK);
            //DEBUG();
#endif
#endif
        }
        else
        {
            if ((pSysConfig->FirmwareFlag != 0x45564153) || (pSysConfig->FirmwareCheck != 0x4f464e49))
            {
                // if no "SAVEINFO" Flag, 1st upgrade poweron and some error, need clear configer Param
                // and format disk
                memset(TempBuffer, 0xff, 2048);
#if(FW_IN_DEV == 3)
#ifdef ENABLE_MBR
                SendMsg(MSG_FORMAT_DISK);
                //DEBUG();
#endif
#endif
            }
        }

        gSysConfig.crc = crcval;
    }

    gSysConfig.FirmwareFlag  = 0x45564153;      // "SaveInfo" Flag
    gSysConfig.FirmwareCheck = 0x4f464e49;      //

    GetFirmwareVersion(&FirmwareYear,
                       &FirmwareDate,
                       &MasterVersion,
                       &SlaveVersion,
                       &SmallVersion);

    if ((pSysConfig->FirmwareYear  != FirmwareYear)  ||
        (pSysConfig->FirmwareDate  != FirmwareDate)  ||
        (pSysConfig->MasterVersion != MasterVersion) ||
        (pSysConfig->SlaveVersion  != SlaveVersion)  ||
        (pSysConfig->SmallVersion  != SmallVersion))
    {
        ;
    }

    gSysConfig.FirmwareYear  = FirmwareYear;
    gSysConfig.FirmwareDate  = FirmwareDate;
    gSysConfig.MasterVersion = MasterVersion;
    gSysConfig.SlaveVersion  = SlaveVersion;
    gSysConfig.SmallVersion  = SmallVersion;

    gSysConfig.SDEnable = (uint8)pSystemDefaultPara->SDEnable; //sd card founction select.
    gSysConfig.FMEnable = (uint8)pSystemDefaultPara->FMEnable; //FM resource select.
    gSysConfig.KeyNum   = (uint8)pSystemDefaultPara->KeyNumIndex;

    if (gSysConfig.FMEnable) //menu resource address select.
    {
        MenuLogicAddress = MenuLogicAddress0;//FM resource address.
    }
    else
    {
        MenuLogicAddress = MenuLogicAddress1;//FM menu resource address.
    }

    //storage media
    gSysConfig.Memory = pSysConfig->Memory;

    if (gSysConfig.Memory > CARD)
    {
        gSysConfig.Memory = FLASH0;
    }

#ifdef _CDROM_
    gSysConfig.bShowCdrom = pSysConfig->bShowCdrom & 0x01;
#endif

    //system language.
    gSysConfig.SysLanguage = pSysConfig->SysLanguage;

    if (gSysConfig.SysLanguage > (TOTAL_LANAUAGE_NUM - 1))
    {
        gSysConfig.SysLanguage = (uint8)pSystemDefaultPara->gLanguage;
    }

    for (i = 0; i < TOTAL_LANAUAGE_NUM; i++)
    {
        if (pSystemDefaultPara->LanguagesEnableTbl[i])
        {
            SetMenuLanguageInfo[j] = i;

            if (i == gSysConfig.SysLanguage)
                DefaultLanguage = j;

            j++;
        }
    }

    LanguageNum = EVK_LANGUAGE_MAX_COUNT;
    DefaultLanguageID = DefaultLanguage;
    gSysConfig.SysLanguage = SetMenuLanguageInfo[DefaultLanguage];

    gLangSel = gSysConfig.SysLanguage;

    //picture resource.
    gSysConfig.ImageIndex = pSysConfig->ImageIndex;

    if (gSysConfig.ImageIndex >= (IMAGE_ID_MAX))
    {
        gSysConfig.ImageIndex = (uint8)pSystemDefaultPara->ImageIndex;
    }

    //system volume
    gSysConfig.OutputVolume = pSysConfig->OutputVolume;

    if (gSysConfig.OutputVolume > 32)
    {
        gSysConfig.OutputVolume = (uint8)pSystemDefaultPara->Volume;
    }

    //backlight mode
    gSysConfig.BLmode = pSysConfig->BLmode;

    if (gSysConfig.BLmode > 1)
    {
        gSysConfig.BLmode = (uint8)pSystemDefaultPara->BLMode;
    }

    //backlight level
    gSysConfig.BLevel = pSysConfig->BLevel;

    if (gSysConfig.BLevel > 5)
    {
        gSysConfig.BLevel = (uint8)pSystemDefaultPara->BLevel;
    }

    //backlight time
    gSysConfig.BLtime = pSysConfig->BLtime;

    if (gSysConfig.BLtime > 5)
    {
        gSysConfig.BLtime = (uint8)pSystemDefaultPara->BLtime;
    }

    //automatic shutdown time
    gSysConfig.ShutTime = pSysConfig->ShutTime;

    if (gSysConfig.ShutTime > 5)
    {
        gSysConfig.ShutTime = (uint8)pSystemDefaultPara->SettingPowerOffTime;
    }

#ifdef _VIDEO_
    gSysConfig.VideoConfig.FileNum = pSysConfig->VideoConfig.FileNum;

    if (gSysConfig.VideoConfig.FileNum == 0)
    {
        gSysConfig.VideoConfig.FileNum = 1;
    }

    gSysConfig.VideoConfig.HoldOnPlaySaveFlag = pSysConfig->VideoConfig.HoldOnPlaySaveFlag;

    if (gSysConfig.VideoConfig.HoldOnPlaySaveFlag > 1)
    {
        gSysConfig.VideoConfig.HoldOnPlaySaveFlag = 0;
    }

    gSysConfig.VideoConfig.Video_Current_FrameNum = pSysConfig->VideoConfig.Video_Current_FrameNum;

    VideoHoldOnPlayInfo.CurrentFileNum = gSysConfig.VideoConfig.FileNum;
    VideoHoldOnPlayInfo.Video_Current_FrameNum = gSysConfig.VideoConfig.Video_Current_FrameNum;
    VideoHoldOnPlayInfo.HoldVideoGetSign = gSysConfig.VideoConfig.HoldOnPlaySaveFlag;

#endif

#ifdef _MUSIC_
    gSysConfig.MusicConfig.HoldOnPlaySaveFlag = pSysConfig->MusicConfig.HoldOnPlaySaveFlag;

    if (gSysConfig.MusicConfig.HoldOnPlaySaveFlag > 1)
    {
        gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
    }

    gSysConfig.MusicConfig.FileNum = pSysConfig->MusicConfig.FileNum;

    if (gSysConfig.MusicConfig.FileNum == 0)
    {
        gSysConfig.MusicConfig.FileNum = 1;
    }

    gSysConfig.MusicConfig.CurTime = pSysConfig->MusicConfig.CurTime;

    gSysConfig.MusicConfig.RepeatModeBak = pSysConfig->MusicConfig.RepeatMode;

    if (gSysConfig.MusicConfig.RepeatModeBak > AUDIO_REPEAT1)
    {
        gSysConfig.MusicConfig.RepeatModeBak = (uint8)pSystemDefaultPara->MusicRepMode;
    }

    gSysConfig.MusicConfig.RepeatMode = pSysConfig->MusicConfig.RepeatMode;

    if (gSysConfig.MusicConfig.RepeatMode > AUDIO_REPEAT)
    {
        gSysConfig.MusicConfig.RepeatMode = (uint8)pSystemDefaultPara->MusicRepMode;
    }

    gSysConfig.MusicConfig.PlayOrder = pSysConfig->MusicConfig.PlayOrder;

    if (gSysConfig.MusicConfig.PlayOrder > AUDIO_RAND)
    {
        gSysConfig.MusicConfig.PlayOrder = (uint8)pSystemDefaultPara->MusicPlayOrder;
    }

#ifdef _RK_EQ_
    gSysConfig.MusicConfig.Eq.Mode = pSysConfig->MusicConfig.Eq.Mode;

    if ((gSysConfig.MusicConfig.Eq.Mode == DIS_EQ_USER) || (gSysConfig.MusicConfig.Eq.Mode > DIS_EQ_BASS))
    {
        gSysConfig.MusicConfig.Eq.Mode = pSystemDefaultPara->MusicEqSel;
    }

    for (i = 0; i < 5; i++)
    {
        gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i] = pSysConfig->MusicConfig.Eq.RKCoef.dbGain[i];

        if ((UINT16)(gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i]) > 7)
        {
            gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i] = 3;
        }
    }

#endif



#ifdef _MUSIC_
    //7.30 azg �޸��������ιػ��󣬶ϵ㲻���������ŵ�bug���Լ��ɶϵ������˻�ý������ʱ����궨λ��׼��bug
    gSysConfig.MusicConfig.HoldMusicFullInfoSectorAddr = pSysConfig->MusicConfig.HoldMusicFullInfoSectorAddr;
    gSysConfig.MusicConfig.HoldMusicSortInfoSectorAddr =  pSysConfig->MusicConfig.HoldMusicSortInfoSectorAddr ;
    gSysConfig.MusicConfig.HoldMusicPlayType           = pSysConfig->MusicConfig.HoldMusicPlayType;
    gSysConfig.MusicConfig.HoldMusicTotalFiles         = pSysConfig->MusicConfig.HoldMusicTotalFiles;

    gSysConfig.MusicConfig.HoldMusicucCurDeep          = pSysConfig->MusicConfig.HoldMusicucCurDeep;

    if (gSysConfig.MusicConfig.HoldMusicucCurDeep > 3)
    {
        gSysConfig.MusicConfig.HoldMusicucCurDeep = 3;
    }

    for (i = 0; i <= gSysConfig.MusicConfig.HoldMusicucCurDeep; i++)
    {
        gSysConfig.MusicConfig.HoldMusicuiBaseSortId[i] = pSysConfig->MusicConfig.HoldMusicuiBaseSortId[i] ;
        gSysConfig.MusicConfig.HoldMusicuiCurId[i]      = pSysConfig->MusicConfig.HoldMusicuiCurId[i];
    }

    for (i = 0; i < (SYS_SUPPROT_STRING_MAX_LEN + 1); i++)
    {
        gSysConfig.MusicConfig.HoldMusicTypeSelName[i] = pSysConfig->MusicConfig.HoldMusicTypeSelName[i];
    }

    gSysConfig.MusicConfig.ShuffleSeed = pSysConfig->MusicConfig.ShuffleSeed;

    if ((gSysConfig.MusicConfig.ShuffleSeed == 0) || (gSysConfig.MusicConfig.ShuffleSeed > 4096))
    {
        gSysConfig.MusicConfig.ShuffleSeed = 1;
    }

#endif

#endif

#ifdef _RADIO_
    gSysConfig.RadioConfig.FmArea = pSysConfig->RadioConfig.FmArea;

    if (gSysConfig.RadioConfig.FmArea > 3)
    {
        gSysConfig.RadioConfig.FmArea = (uint8)pSystemDefaultPara->FMArea;
    }

    gSysConfig.RadioConfig.FmStereo = pSysConfig->RadioConfig.FmStereo;

    if (gSysConfig.RadioConfig.FmStereo > 1)
    {
        gSysConfig.RadioConfig.FmStereo = (uint8)pSystemDefaultPara->FMStereo;
    }

    gSysConfig.RadioConfig.FmState = pSysConfig->RadioConfig.FmState;

    if (gSysConfig.RadioConfig.FmState > 1)
    {
        gSysConfig.RadioConfig.FmState = 0;
    }

    gSysConfig.RadioConfig.FmSaveNum = pSysConfig->RadioConfig.FmSaveNum;

    if (gSysConfig.RadioConfig.FmSaveNum > 40)
    {
        gSysConfig.RadioConfig.FmSaveNum = 0;
    }

    gSysConfig.RadioConfig.FmFreq = pSysConfig->RadioConfig.FmFreq;

    for (i = 0; i < 40; i++)
    {
        gSysConfig.RadioConfig.FmFreqArray[i] = pSysConfig->RadioConfig.FmFreqArray[i];
    }

    gSysConfig.RadioConfig.ScanSensitivity = pSysConfig->RadioConfig.ScanSensitivity;

    if (gSysConfig.RadioConfig.ScanSensitivity > 1)
    {
        gSysConfig.RadioConfig.ScanSensitivity = pSystemDefaultPara->FMScanSensitivity;;
    }

    gSysConfig.RadioConfig.HoldOnPlaySaveFlag = pSysConfig->RadioConfig.HoldOnPlaySaveFlag;

    if (gSysConfig.RadioConfig.HoldOnPlaySaveFlag > 2)
    {
        gSysConfig.RadioConfig.HoldOnPlaySaveFlag = 0;
    }

#endif

#ifdef _RECORD_
    gSysConfig.RecordConfig.RecordQuality = pSysConfig->RecordConfig.RecordQuality;

    if (gSysConfig.RecordConfig.RecordQuality > 1)
    {
        gSysConfig.RecordConfig.RecordQuality = 0;
    }

    gSysConfig.RecordConfig.RecordFmt = pSysConfig->RecordConfig.RecordFmt;

    if (gSysConfig.RecordConfig.RecordFmt > 1)
    {
        gSysConfig.RecordConfig.RecordFmt = 0;
    }

    gSysConfig.RecordConfig.RecordVol = pSysConfig->RecordConfig.RecordVol;

    if (gSysConfig.RecordConfig.RecordVol > 5)
    {
        gSysConfig.RecordConfig.RecordVol = 2;
    }

    gSysConfig.RecordConfig.RecordIndex = pSysConfig->RecordConfig.RecordIndex;

    if ((gSysConfig.RecordConfig.RecordIndex > 9999) || (gSysConfig.RecordConfig.RecordIndex == 0))
    {
        gSysConfig.RecordConfig.RecordIndex = 1;
    }

#endif

#ifdef _EBOOK_
    gSysConfig.TextConfig.FileNum = pSysConfig->TextConfig.FileNum;

    if (gSysConfig.TextConfig.FileNum == 0)
    {
        gSysConfig.TextConfig.FileNum = 1;
    }

    gSysConfig.TextConfig.AutoPlay = pSysConfig->TextConfig.AutoPlay;

    if (gSysConfig.TextConfig.AutoPlay > 1)
    {
        gSysConfig.TextConfig.AutoPlay = 1;
    }

    gSysConfig.TextConfig.AutoPlayTime = pSysConfig->TextConfig.AutoPlayTime;

    if (gSysConfig.TextConfig.AutoPlayTime > 5)
    {
        gSysConfig.TextConfig.AutoPlayTime = (uint8)pSystemDefaultPara->TextAutoPlayTime;
    }

#endif

#ifdef _PICTURE_
    gSysConfig.PicConfig.FileNum = pSysConfig->PicConfig.FileNum;

    if (gSysConfig.PicConfig.FileNum == 0)
    {
        gSysConfig.PicConfig.FileNum = 1;
    }

    gSysConfig.PicConfig.AutoPlaySwitch = pSysConfig->PicConfig.AutoPlaySwitch;

    if (gSysConfig.PicConfig.AutoPlaySwitch > 1)
    {
        gSysConfig.PicConfig.AutoPlaySwitch = 0;
    }

    gSysConfig.PicConfig.AutoPlayTime = pSysConfig->PicConfig.AutoPlayTime;

    if (gSysConfig.PicConfig.AutoPlayTime > 5)
    {
        gSysConfig.PicConfig.AutoPlayTime = (uint8)pSystemDefaultPara->PicAutoPlayTime;
    }

#endif

#ifdef _BLUETOOTH_
    gSysConfig.BtConfig.KeyIndex = pSysConfig->BtConfig.KeyIndex;

    if (gSysConfig.BtConfig.KeyIndex >= BT_LINK_KEY_MAX_NUM)
    {
        // gSysConfig.BtConfig.KeyIndex = 0;

        memset(&gSysConfig.BtConfig, 0x00, sizeof(BLUETOOTH_CONFIG));
    }
    else
    {
        memcpy(&gSysConfig.BtConfig, &pSysConfig->BtConfig, sizeof(BLUETOOTH_CONFIG));
    }

    //device name
    memcpy(gSysConfig.BtConfig.LocalBTName, pSystemDefaultPara->BTDevName, BT_LOCAL_DEV_NAME_MAX);

    //device mac address
#ifdef _EMMC_
    mac_size = EmmcGetBluetoothMac(gSysConfig.BtConfig.LocalBTMacAddr);
#else   //_SPINOR_
    mac_size = SFlashGetBluetoothMac(gSysConfig.BtConfig.LocalBTMacAddr);
#endif

    if (mac_size == 6)
    {
        DEBUG("## get bt mac addr from system ##");
    }
    else
    {
        DEBUG("!!! Eorror,Not get bt mac addr from system");
    }

#endif

#ifdef _MEDIA_MODULE_ //ý���
    gSysConfig.MedialibPara.gID3AlbumFileNum     = pSysConfig->MedialibPara.gID3AlbumFileNum;

    if (gSysConfig.MedialibPara.gID3AlbumFileNum == 0xffff)
        gSysConfig.MedialibPara.gID3AlbumFileNum = 0;

    gSysConfig.MedialibPara.gID3ArtistFileNum    = pSysConfig->MedialibPara.gID3ArtistFileNum;

    if (gSysConfig.MedialibPara.gID3ArtistFileNum == 0xffff)
        gSysConfig.MedialibPara.gID3ArtistFileNum = 0;

    gSysConfig.MedialibPara.gID3TitleFileNum     = pSysConfig->MedialibPara.gID3TitleFileNum;

    if (gSysConfig.MedialibPara.gID3TitleFileNum == 0xffff)
        gSysConfig.MedialibPara.gID3TitleFileNum = 0;

    gSysConfig.MedialibPara.gID3GenreFileNum     = pSysConfig->MedialibPara.gID3GenreFileNum;

    if (gSysConfig.MedialibPara.gID3GenreFileNum == 0xffff)
        gSysConfig.MedialibPara.gID3GenreFileNum = 0;

    gSysConfig.MedialibPara.gMusicFileNum        = pSysConfig->MedialibPara.gMusicFileNum;

    if (gSysConfig.MedialibPara.gMusicFileNum == 0xffff)
        gSysConfig.MedialibPara.gMusicFileNum = 0;

    gSysConfig.MedialibPara.gMyFavoriteFileNum   = pSysConfig->MedialibPara.gMyFavoriteFileNum;

    if (gSysConfig.MedialibPara.gMyFavoriteFileNum == 0xffff)
        gSysConfig.MedialibPara.gMyFavoriteFileNum = 0;

    gSysConfig.MedialibPara.gRecordFmFileNum = pSysConfig->MedialibPara.gRecordFmFileNum;

    if (gSysConfig.MedialibPara.gRecordFmFileNum == 0xffff)
        gSysConfig.MedialibPara.gRecordFmFileNum = 0;


    gSysConfig.MedialibPara.gRecordVoiceFileNum = pSysConfig->MedialibPara.gRecordVoiceFileNum;

    if (gSysConfig.MedialibPara.gRecordVoiceFileNum == 0xffffffff)
        gSysConfig.MedialibPara.gRecordVoiceFileNum = 0;


    gSysConfig.MedialibPara.MediaUpdataFlag = pSysConfig->MedialibPara.MediaUpdataFlag;

    if (gSysConfig.MedialibPara.MediaUpdataFlag > 1)
        gSysConfig.MedialibPara.MediaUpdataFlag = 1;

    #ifdef _USB_HOST_
    gSysConfig.UsbHost = 0;
    gSysConfig.Listen_UsbHost = 0;
    #endif
#endif

    gSysConfig.SafePoweroff = pSysConfig->SafePoweroff;

    if (gSysConfig.SafePoweroff != 0x55aa)
    {
        gSysConfig.MedialibPara.MediaUpdataFlag = 1;
    }

    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
    gSysConfig.SafePoweroff = 0;
    {
        uint32 crcval;

        //crc������bitΪ��λ������ĸ��ֽڱ���crcֵ��������crcУ��
        gSysConfig.crc = sbc_crc8((const char *)&gSysConfig, (sizeof(SYSCONFIG) - 4) * 8);
        DEBUG("gSysConfig.crc = %d", gSysConfig.crc);
    }
    memcpy(TempBuffer, (uint8*)&gSysConfig, sizeof(SYSCONFIG));
    WriteReservedData(SYSDATA_BLK, TempBuffer, 2048);

}

/*
--------------------------------------------------------------------------------
  Function name : uint32 FindIDBlock(void)
  Author        : ZHengYongzhi
  Description   : find IDBlock

  Input         :
  Return        : IDBlock or 0xffffffff

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYSRESERVED_OP_CODE_
void SaveSysInformation(int flag)
{
    uint8  TempBuffer[2048];
    memset(TempBuffer, 0xff, 2048);

    if (flag)
        gSysConfig.SafePoweroff = 0x55aa;
    else
        gSysConfig.SafePoweroff = 0;

    {
        uint32 crcval;

        //crc������bitΪ��λ������ĸ��ֽڱ���crcֵ��������crcУ��
        gSysConfig.crc = sbc_crc8((const char *)&gSysConfig, (sizeof(SYSCONFIG) - 4) * 8);
        DEBUG("gSysConfig.crc = %d", gSysConfig.crc);
    }

    {
        //add just for debug
        uint32 cfgsize;
        cfgsize = sizeof(SYSCONFIG);

        if (cfgsize > 2048) //avoid SYSCONFIG size bigger than TempBuffer size.
        {
            DEBUG("gSysConfig size = %d", cfgsize);
            DEBUG("////////////////////error happen/////////////////");
        }
        else
        {
            DEBUG("gSysConfig size = %d", cfgsize);
        }
    }

    memcpy(TempBuffer, (uint8*)&gSysConfig, sizeof(SYSCONFIG));
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);

    WriteReservedData(SYSDATA_BLK, TempBuffer, 2048);
}

//zyz: SYSCONFIG Backup & Clear F/W upgrade Flag
_ATTR_SYSRESERVED_OP_CODE_
void BackupSysInformation(void)
{
    uint8  TempBuffer[2048];

    ReadReservedData(SYSDATA_BLK, TempBuffer, 2048);

    TempBuffer[2048 - 4] = 'F';
    TempBuffer[2048 - 3] = 'W';
    TempBuffer[2048 - 2] = 'U';
    TempBuffer[2048 - 1] = 'P';

#if(FW_IN_DEV == 3)
    //EmmcWriteIDB2(SDM_DEF_ID, 2, TempBuffer);
#endif
}



/*
********************************************************************************
*
*                         End of SysReservedOperation.c
*
********************************************************************************
*/

