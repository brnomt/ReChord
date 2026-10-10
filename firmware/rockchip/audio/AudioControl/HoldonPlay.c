/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  HoldonPlay.C
*
* Description:
*
* History:      <author>          <time>        <version>
*               Chenwei          2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_HOLDON_PLAY_

#include "SysInclude.h"
#include "fsinclude.h"
#include "audio_globals.h"
#include "AudioControl.h"
#include "HoldonPlay.h"

#include "MediaBroWin.h"
#include "medialibwin.h"
#include "FileInfo.h"
#include "AddrSaveMacro.h"

#ifdef _MUSIC_

#ifdef AUDIOHOLDONPLAY

extern track_info*          pAudioRegKey;

/*
--------------------------------------------------------------------------------
  Function name : void HoldMusicStart(void)
  Author        : zs
  Description   : start breakpoint,get time and global file number.
  Input         : null
  Return        : null
  History       :  <author>         <time>         <version>
                    zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
//_ATTR_MEDIALIBWIN_SERVICE_CODE_
void HoldMusicStart(void)
{
    //if ((AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_BROWSER) && (AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_FOLDER))//enter the audio play from media.
    {
        UINT16 temp1;
        UINT8 ucBufTemp[2];
        UINT32 DirClus;
        UINT32 DirIndex;

        {
            AudioFileInfo.ulFullInfoSectorAddr = gSysConfig.MusicConfig.HoldMusicFullInfoSectorAddr ;
            AudioFileInfo.ulSortInfoSectorAddr = gSysConfig.MusicConfig.HoldMusicSortInfoSectorAddr ;
            AudioFileInfo.ucCurDeep            = gSysConfig.MusicConfig.HoldMusicucCurDeep;
            AudioFileInfo.TotalFiles           = gSysConfig.MusicConfig.HoldMusicTotalFiles;
            AudioFileInfo.CurrentFileNum       = gSysConfig.MusicConfig.FileNum;

            for (temp1 = 0; temp1 <= gSysConfig.MusicConfig.HoldMusicucCurDeep; temp1++)
            {
                AudioFileInfo.uiBaseSortId[temp1] = gSysConfig.MusicConfig.HoldMusicuiBaseSortId[temp1];
                AudioFileInfo.uiCurId[temp1]      =  gSysConfig.MusicConfig.HoldMusicuiCurId[temp1];
            }
        }

        if (AudioFileInfo.ucSelPlayType != MUSIC_TYPE_SEL_MYFAVORITE)
        {
            MDReadData(DataDiskID, (AudioFileInfo.ulSortInfoSectorAddr<<9)+(AudioFileInfo.uiCurId[AudioFileInfo.ucCurDeep]+AudioFileInfo.uiBaseSortId[AudioFileInfo.ucCurDeep])*2, 2, ucBufTemp);
            temp1 = (UINT16)ucBufTemp[0]+(((UINT16)ucBufTemp[1])<<8);

            MDReadData(DataDiskID, (AudioFileInfo.ulFullInfoSectorAddr<<9) + (UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)DIR_CLUS_SAVE_ADDR_OFFSET, 4, &DirClus);
            MDReadData(DataDiskID, (AudioFileInfo.ulFullInfoSectorAddr<<9) + (UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)DIR_INDEX_SAVE_ADDR_OFFSET, 4, &DirIndex);
        }
        else
        {
            AudioFileInfo.TotalFiles = gSysConfig.MedialibPara.gMyFavoriteFileNum;
        }

        //get breakpoint file path.
#ifdef _RECORD_
        if (AudioFileInfo.ucSelPlayType ==  MUSIC_TYPE_SEL_RECORDFILE)
        {
            // FIND_DATA FindDataPlay;
            // ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
#if 0
            AudioFileInfo.FindData.Clus = GetDirClusIndex("U:\\RECORD     \\FM         ");

            AudioFileInfo.FindData.Index = 0;
            for (temp1 = 0; temp1 < AudioFileInfo.CurrentFileNum; temp1++)
            {
                FindNextFile(&AudioFileInfo.Fdt, &(AudioFileInfo.FindData), RecordFileExtString, FS_FAT);
            }
#else
            AudioFileInfo.FindData.Clus = 0;

            AudioFileInfo.FindData.Index = 0;
            AudioFileInfo.FindData.TotalItem = gSysConfig.MedialibPara.gRecordFmFileNum;

            for (temp1 = 0; temp1 < AudioFileInfo.CurrentFileNum; temp1++)
            {
                FindNextFile(&AudioFileInfo.Fdt, &(AudioFileInfo.FindData), RecordFileExtString, RECORD_DB);
            }
#endif
        }
        else
#endif
        {
            AudioFileInfo.FindData.Clus = DirClus;
            AudioFileInfo.FindData.Index = DirIndex;
        }
    }

    //printf("\n HoldMusicStart Out \n");
}


/*
--------------------------------------------------------------------------------
  Function name : void MusicHoldOnInforSave(UINT CurrentTime, SYS_FILE_INFO *SysFileInfo)
  Author        : zs
  Description   : save breakpoint information.
  Input         : CurrentTime               :breakpoint time.
                  SYS_FILE_INFO *SysFileInfo:audio structure.
  Return        : null
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_INIT_TEXT_
void MusicHoldOnInforSave(UINT32 CurrentTime, SYS_FILE_INFO *SysFileInfo)
{
    FS_TYPE FsType;
    UINT16 i;

    if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_BROWSER)
    {
        FsType = FS_FAT;
    }
    else if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER)
    {
        FsType = MUSIC_DB;
    }
#ifdef _RECORD_
    else if(AudioFileInfo.ucSelPlayType == MUSIC_TYPE_SEL_FMFILE)
    {
        FsType = RECORD_DB;
    }
#endif

//    ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);

    gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 1;
    gSysConfig.MusicConfig.HoldMusicPlayType  = AudioFileInfo.ucSelPlayType;//debug cannot save audio breakpoint when play music from explorer

    for (i = 0; i < (SYS_SUPPROT_STRING_MAX_LEN + 1); i++)
    {
        gSysConfig.MusicConfig.HoldMusicTypeSelName[i] = gMusicTypeSelName[i];
    }


#ifdef _MEDIA_MODULE_

    if ((AudioFileInfo.ucSelPlayType !=  SORT_TYPE_SEL_BROWSER) && (AudioFileInfo.ucSelPlayType !=  SORT_TYPE_SEL_FOLDER))
    {
        gSysConfig.MusicConfig.HoldMusicFullInfoSectorAddr = AudioFileInfo.ulFullInfoSectorAddr;
        gSysConfig.MusicConfig.HoldMusicSortInfoSectorAddr = AudioFileInfo.ulSortInfoSectorAddr;

        gSysConfig.MusicConfig.HoldMusicucCurDeep = AudioFileInfo.ucCurDeep;

        for (i = 0; i <= gSysConfig.MusicConfig.HoldMusicucCurDeep; i++)
        {
            gSysConfig.MusicConfig.HoldMusicuiBaseSortId[i] = AudioFileInfo.uiBaseSortId[i];
            gSysConfig.MusicConfig.HoldMusicuiCurId[i]      = AudioFileInfo.uiCurId[i];
        }
        gSysConfig.MusicConfig.HoldMusicTotalFiles = AudioFileInfo.TotalFiles;
        gSysConfig.MusicConfig.HoldClusSave = gwSaveDirClus;
    }
    else
#endif
    {
        //SysFileInfo->CurrentFileNum = GetCurFileNum(SysFileInfo->CurrentFileNum,SysFileInfo->pExtStr);
        if (SysFileInfo->Range == FIND_FILE_RANGE_DIR)//cycle among directory.
        {
            SysFileInfo->CurrentFileNum = GetGlobeFileNum(SysFileInfo->CurrentFileNum ,SysFileInfo->FindData.Clus, SysFileInfo->pExtStr, FsType);
        }
    }
    gSysConfig.MusicConfig.FileNum = SysFileInfo->CurrentFileNum;//solve the bug:it cannot save the breakpoint when shutdown.
    gSysConfig.MusicConfig.CurTime = CurrentTime;
}

#endif //AUDIOHOLDONPLAY

#endif //_MUSIC_
/*
********************************************************************************
*
*                         End of HoldonPlay.c
*
********************************************************************************
*/

