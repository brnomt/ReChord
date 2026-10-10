/*
********************************************************************************
*          Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All Rights Reserved
*                                    V1.00
* FileName   : MediaBroWin.c
* Author     : azg
* Description:
* History    :
*           <author>        <time>     <version>       <desc>
*           azg      05/11/24       1.0            ORG
*
********************************************************************************
*/
#define _IN_MEDIABRO_WIN_

#include "SysInclude.h"

#ifdef _MEDIA_MODULE_
#include "fsinclude.h"
#include "FileInfo.h"
#include "SortInfoGetMacro.h"
#include "AddrSaveMacro.h"

#include "MediaBroWin.h"
#include "medialibwin.h"
#include "SysFindFile.h"

#include "BrowserUI.h"

#include "MessageBox.h"
#include "mainmenu.h"
#include "AudioControl.h"
#include "Hold.h"
#include "myRandom.h"
#include "DialogBox.h"
#include "AddrSaveMacro.h"

#include "image_main.h"
#include "ImageControl.h"
#ifdef THUMB_DEC_INCLUDE
#include "thumbnail_parse.h"
#endif

UINT32 SaveAndPostMusicPlayInfo(void);
void FomatTheOtherItem(UINT16 *ucFileName);
UINT16 GetListItem(UINT16 *pListName, UINT16 uiListNO);


extern SORTINFO_STRUCT Subinfo;

static int gMusicDirTreeInfo_KeyCounter = 0;
static int gPreviousFileNum = 0;
static UINT16 MusicBroUnknownCnt = 0;
static UINT16 MusicBroAllfileCnt = 0;

#ifdef THUMB_DEC_INCLUDE
static BOOL gIsImproveFreq;
static UINT32 gTickCounter;
//static UINT8  gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP];
static UINT8  gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP+4];
static UINT8  gThumbNowDecodeLine;
static UINT8  gThumbNowPicPos;
static UINT8  gThumbDecodeMode;
//static UINT32 gThumbKeyWaitCounter;
static UINT16 gThumbDecodeBuf[12*12*2];
static FILE*  gThumbHandle;
//<----sanshin_20151026

static UINT16 gThumbDecM1Buf[12*12*2];
static UINT16 gThumbDecP1Buf[12*12*2];
static UINT16 gThumbDecP2Buf[12*12*2];
static UINT16 gThumbDecM2Buf[12*12*2];
#endif

/*
--------------------------------------------------------------------------------
  Function name : void check_album(void)
  Author        :
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                                     2015/06/25         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
static UINT16 check_album(void)
{
    UINT16 ret = 0;

    if (((gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER) && (MusicDirTreeInfo.MusicDirDeep == 1)) ||
        ((gMusicTypeSelID == SORT_TYPE_SEL_ID3ALBUM)  && (MusicDirTreeInfo.MusicDirDeep == 0)) ||
        ((gMusicTypeSelID == SORT_TYPE_SEL_GENRE)     && (MusicDirTreeInfo.MusicDirDeep == 2)))
    {
        ret = 1;
    }

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 check_allfile(void)
  Author        :
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                            2015/10/26         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
static UINT16 check_allfile(void)
{
    UINT16 ret = 0;

    if(
        ((gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER) && (MusicDirTreeInfo.MusicDirDeep == 1)) ||
        ((gMusicTypeSelID == SORT_TYPE_SEL_GENRE)     && (MusicDirTreeInfo.MusicDirDeep == 2))
    ){
        ret = 1;
    }
    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : void SortInfoAddrInit(void)
  Author        : anzhiguo
  Description   : the vaious informations saved address of media libary initial.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         point to sec address of correspond information in flash.
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_INIT_CODE_
void SortInfoAddrInit(void)
{
    SortInfoAddr.ulFileFullInfoSectorAddr = MediaInfoAddr + MUSIC_SAVE_INFO_SECTOR_START;

    switch (gMusicTypeSelID)
    {
        case SORT_TYPE_SEL_FILENAME:
            //SortInfoAddr.uiSortInfoAddrOffset[0] = FILE_NAME_SAVE_ADDR_OFFSET;//
            //SortInfoAddr.ulFileSortInfoSectorAddr = MediaInfoAddr + FILENAME_SORT_INFO_SECTOR_START;
            break;

        case SORT_TYPE_SEL_ID3TITLE:
            SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_TITLE_SAVE_ADDR_OFFSET;
            SortInfoAddr.ulFileSortInfoSectorAddr = MediaInfoAddr + ID3TITLE_SORT_INFO_SECTOR_START;
            break;

        case SORT_TYPE_SEL_ID3SINGER:
            SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_SINGLE_SAVE_ADDR_OFFSET;//����ƫ�Ƶ�ַ
            SortInfoAddr.uiSortInfoAddrOffset[1] = ID3_ALBUM_SAVE_ADDR_OFFSET; //ר��ƫ�Ƶ�ַ
            SortInfoAddr.uiSortInfoAddrOffset[2] = ID3_TITLE_SAVE_ADDR_OFFSET;//carl FILE_NAME_SAVE_ADDR_OFFSET; //���ļ���ƫ�Ƶ�ַ
            SortInfoAddr.ulFileSortInfoSectorAddr = MediaInfoAddr + ID3ARTIST_SORT_INFO_SECTOR_START;
            SortInfoAddr.ulSortSubInfoSectorAddr[0] = MediaInfoAddr + ID3ARTIST_SORT_SUB_SECTOR_START;
            SortInfoAddr.ulSortSubInfoSectorAddr[1] = MediaInfoAddr + ID3ARTIST_ALBUM_SORT_SUB_SECTOR_START;
            break;

        case SORT_TYPE_SEL_ID3ALBUM:
            SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_ALBUM_SAVE_ADDR_OFFSET;//ר��ƫ�Ƶ�ַ
            SortInfoAddr.uiSortInfoAddrOffset[1] = ID3_TITLE_SAVE_ADDR_OFFSET;//carl FILE_NAME_SAVE_ADDR_OFFSET;//���ļ���ƫ�Ƶ�ַ
            SortInfoAddr.ulFileSortInfoSectorAddr = MediaInfoAddr + ID3ALBUM_SORT_INFO_SECTOR_START;
            SortInfoAddr.ulSortSubInfoSectorAddr[0] = MediaInfoAddr + ID3ALBUM_SORT_SUB_SECTOR_START;
            break;

        case SORT_TYPE_SEL_GENRE:
            SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_GENRE_SAVE_ADDR_OFFSET; //����ƫ�Ƶ�ַ
            SortInfoAddr.uiSortInfoAddrOffset[1] = ID3_SINGLE_SAVE_ADDR_OFFSET;//����ƫ�Ƶ�ַ
            SortInfoAddr.uiSortInfoAddrOffset[2] = ID3_ALBUM_SAVE_ADDR_OFFSET; //ר��ƫ�Ƶ�ַ
            SortInfoAddr.uiSortInfoAddrOffset[3] = ID3_TITLE_SAVE_ADDR_OFFSET; //ר��ƫ�Ƶ�ַ
            SortInfoAddr.ulFileSortInfoSectorAddr = MediaInfoAddr +  ID3GENRE_SORT_INFO_SECTOR_START; //�������ļ�˳���б�
            SortInfoAddr.ulSortSubInfoSectorAddr[0] = MediaInfoAddr + ID3GENRE_SORT_SUB_SECTOR_START; //���ɷ�����Ϣ��ŵ�ַ
            SortInfoAddr.ulSortSubInfoSectorAddr[1] = MediaInfoAddr + ID3GENRE_ARTIST_SORT_SUB_SECTOR_START;//����-���ַ�����Ϣ��ŵ�ַ
            SortInfoAddr.ulSortSubInfoSectorAddr[2] = MediaInfoAddr + ID3GENRE_ALBUM_SORT_SUB_SECTOR_START;//����-����-ר��������Ϣ��ŵ�ַ
            break;

        case MUSIC_TYPE_SEL_MYFAVORITE:
            SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_TITLE_SAVE_ADDR_OFFSET;//carl addFILE_NAME_SAVE_ADDR_OFFSET;
            SortInfoAddr.ulFileSortInfoSectorAddr = MediaInfoAddr +  FAVORITE_MUSIC_INFO_SECTOR_START;
            break;

        default:
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 GetCurItemNum()
  Author        : anzhiguo
  Description   : ��ȡý����е�ǰӵ�е���Ŀ��

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
UINT16 GetCurItemNum()
{
    UINT16  i = 0;
    UINT16  uiTotalItemNum;
    FDT     Fdt;
    FIND_DATA   FindData;
    SORTINFO_STRUCT Subinfo;

    if (MusicDirTreeInfo.MusicDirDeep == 0)
    {
        switch (gMusicTypeSelID) // add by phc, 2007.6.14
        {
            case SORT_TYPE_SEL_FILENAME:
                uiTotalItemNum = gSysConfig.MedialibPara.gMusicFileNum;
                break;

            case SORT_TYPE_SEL_ID3TITLE:
                uiTotalItemNum = gSysConfig.MedialibPara.gID3TitleFileNum;
                break;

            case SORT_TYPE_SEL_ID3SINGER:
                uiTotalItemNum = gSysConfig.MedialibPara.gID3ArtistFileNum;
                break;

            case SORT_TYPE_SEL_ID3ALBUM:
                uiTotalItemNum = gSysConfig.MedialibPara.gID3AlbumFileNum;
                break;

            case SORT_TYPE_SEL_GENRE:
                uiTotalItemNum = gSysConfig.MedialibPara.gID3GenreFileNum;
                break;

            case MUSIC_TYPE_SEL_MYFAVORITE://�ղؼ�
                uiTotalItemNum = gSysConfig.MedialibPara.gMyFavoriteFileNum;// = GetFavoriteFileNume();
                break;
#ifdef _RECORD_

            case MUSIC_TYPE_SEL_RECORDFILE_DEL:
            case MUSIC_TYPE_SEL_RECORDFILE://recording
//                ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
#if 0
                RecordDirClus = GetDirClusIndex(RecordPathString);
                uiTotalItemNum = GetTotalFiles(RecordDirClus, RecordFileExtString, FS_FAT);
#else
                RecordDirClus = 0;
                uiTotalItemNum = gSysConfig.MedialibPara.gRecordFmFileNum;
                //printf("\n totalFmfile = %d",uiTotalItemNum);
#endif
                break;

            case MUSIC_TYPE_SEL_FMFILE://recording
                //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
                RecordDirClus = 0;
                uiTotalItemNum = gSysConfig.MedialibPara.gRecordFmFileNum;
                break;
#endif

            default:
                uiTotalItemNum = 0;
                break;
        }
    }
    /* sch120416
    else if ((MusicDirTreeInfo.MusicDirDeep == 2)&&(gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER))//��ȡר�����������µ��ļ���
    {
        uiTotalItemNum = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirTreeInfo.MusicDirDeep-1], MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep-1]+MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep-1]-1, FIND_SUM_ITEMNUM);
    }
    */
    else //if(MusicDirTreeInfo.MusicDirDeep == 1)//��ȡר�����������µ��ļ���
    {
        if ((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_TITLE_SAVE_ADDR_OFFSET) && ((gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER) || (gMusicTypeSelID == SORT_TYPE_SEL_GENRE)))
        {
            uiTotalItemNum = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirTreeInfo.MusicDirDeep - 1], MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep - 1] - 1 + MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep - 1], FIND_SUM_ITEMNUM);
        }
        else
        {
            uiTotalItemNum = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirTreeInfo.MusicDirDeep - 1], MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep - 1] + MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep - 1], FIND_SUM_ITEMNUM);
        }

        // MusicDirTreeInfo.TotalItem[MusicDirTreeInfo.MusicDirDeep-1] = Subinfo.ItemNum;
    }

    return uiTotalItemNum;
}
/*
--------------------------------------------------------------------------------
  Function name : void SortInfoAddrInit(void)
  Author        : anzhiguo
  Description   : ý����и�����Ϣ��ŵ�ַ�����ĳ�ʼ��

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ָ����Ӧ��Ϣ��flash�е�sec��ַ
--------------------------------------------------------------------------------
*/

_ATTR_MEDIABROWIN_CODE_

UINT16 GetBaseId()
{
    UINT16 uiBaseId;

    if (MusicDirTreeInfo.MusicDirDeep == 0)
    {
        uiBaseId = 0;
    }
    /*  sch120418
     else if ((MusicDirTreeInfo.MusicDirDeep == 2)&&(gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER))
     {
         uiBaseId = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirTreeInfo.MusicDirDeep-1], MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep-1]+MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep-1]-1, FIND_SUM_SORTSTART);
     }
     */
    else //if((MusicDirTreeInfo.MusicDirDeep == 1)||)
    {
        //Rk Aaron.sun
        if ((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_TITLE_SAVE_ADDR_OFFSET) && ((gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER) || (gMusicTypeSelID == SORT_TYPE_SEL_GENRE)))
        {
            uiBaseId = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirTreeInfo.MusicDirDeep - 1], MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep - 1] - 1 + MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep - 1], FIND_SUM_SORTSTART);
        }
        else
        {
            uiBaseId = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirTreeInfo.MusicDirDeep - 1], MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep - 1] + MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep - 1], FIND_SUM_SORTSTART);
        }
    }

    return uiBaseId;
}

/*
--------------------------------------------------------------------------------
  Function name : void RecordDBFileDelete(void)
  Author        : allen
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                allen               2013/09/25         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
extern  uint32   FlashSec[3];
_ATTR_MEDIABROWIN_BSS_  uint8 DataTempBuf[2048];

_ATTR_MEDIABROWIN_CODE_
UINT16 RecordDBFileDelete(FIND_DATA * FindData)
{
    uint32 CurSec, SecOffset;
    FILE_TREE_BASIC * pFileTreeBasic;
    uint32 i;
    //printf("FindData->DirOffsetInRDB = %d\n", FindData->DirOffsetInRDB);
    CurSec = MediaInfoAddr + RECORD_TREE_INFO_SECTOR_START + (FindData->DirOffsetInRDB * sizeof(FILE_TREE_BASIC)) / SECTOR_BYTE_SIZE;
    SecOffset = (FindData->DirOffsetInRDB % (SECTOR_BYTE_SIZE / sizeof(FILE_TREE_BASIC))) * sizeof(FILE_TREE_BASIC);
    MDRead(DataDiskID, CurSec, 1, DataTempBuf);
#if 0
    printf("read tree basic info\n");
    {
        uint16 i, j;

        for (i = 0; i < 32; i++)
        {
            printf("\n");

            for (j = 0; j < 16; j++)
            {
                printf("%02x ", DataTempBuf[i * 16 + j]);
            }
        }

        printf("\n");
    }
#endif
    pFileTreeBasic = (FILE_TREE_BASIC *)&DataTempBuf[SecOffset];
    pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_DELETED;
#if 0
    printf("write tree basic info\n");
    {
        uint16 i, j;

        for (i = 0; i < 32; i++)
        {
            printf("\n");

            for (j = 0; j < 16; j++)
            {
                printf("%02x ", DataTempBuf[i * 16 + j]);
            }
        }

        printf("\n");
    }
#endif
    MDWrite(DataDiskID, CurSec, 1, DataTempBuf);
    MDRead(DataDiskID, MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START, 4, DataTempBuf);

    //printf("Index = %d, TotalItem = %d\n", FindData->Index, FindData->TotalItem);

    for (i = FindData->Index; i < FindData->TotalItem; i++)
    {
        ((uint16 *)DataTempBuf)[i - 1] = ((uint16 *)DataTempBuf)[i];
        //printf("RemoveData = %d\n", ((uint16 *)DataTempBuf)[i]);
    }

    MDWrite(DataDiskID, MediaInfoAddr + RECORD_TREE_SORT_INFO_SECTOR_START, 4, DataTempBuf);
    FlashSec[0] = 0xffffffff;
    FlashSec[1] = 0xffffffff;
    FlashSec[2] = 0xffffffff;
    gSysConfig.MedialibPara.gRecordFmFileNum--;
    FindData->TotalItem--;
    SaveSysInformation(1);
}


/*
--------------------------------------------------------------------------------
  Function name : void DeleteRecFile(void)
  Author        : allen
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                allen               2013/09/25         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/

_ATTR_MEDIABROWIN_CODE_
void DeleteRecFile()
{
    UINT16 ret, i, FileNum;
    FIND_DATA FindData;
    FDT Fdt;
//    ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
//    ModuleOverlay(MODULE_ID_FILE_ENCODE, MODULE_OVERLAY_ALL);
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);
    FindData.Clus = RecordDirClus;
    FindData.Index = 0;
    FindData.TotalItem = MusicDirTreeInfo.MusicDirTotalItem;

    FREQ_EnterModule(FREQ_MAX);

    if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] == 0)  //delete all
    {
        FileNum = MusicDirTreeInfo.MusicDirTotalItem;
        FindFirstFile(&Fdt, &FindData, RecordFileExtString, RECORD_DB);

        for (i = 0; i < (FileNum - 1); i++)
        {
            ret = FileDelete1(FindData.Clus, FindData.Index - 1, RECORD_DB);
            RecordDBFileDelete(&FindData);
        }

        ret = FileDelete1(FindData.Clus, FindData.Index - 1, RECORD_DB);
        RecordDBFileDelete(&FindData);
    }
    else
    {
        FileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - 1;
        FindFirstFile(&Fdt, &FindData, RecordFileExtString, RECORD_DB);

        for (i = 0; i < FileNum; i++)
        {
            FindNextFile(&Fdt, &FindData, RecordFileExtString, RECORD_DB);
        }

        ret = FileDelete1(FindData.Clus, FindData.Index - 1, RECORD_DB);
        RecordDBFileDelete(&FindData);
    }

    FREQ_ExitModule(FREQ_MAX);

    //gSysConfig.MedialibPara.MediaUpdataFlag = 1;//it should notice media update when file be deleted
    //return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : void DisplayNowPlayingIcon(void)
  Author        : allen
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                allen               2013/11/07         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/

_ATTR_MEDIABROWIN_CODE_
UINT16 DisplayNowPlayingIcon(UINT16 id)
{
    UINT16 ret;
    FILE_TREE_BASIC FileTreeBasic;
    uint16 temp;
    uint32 DirClus, Index;
    ret = 0;

    if ((TRUE == ThreadCheck(pMainThread, &MusicThread)) && (gMusicTypeSelID == SORT_TYPE_SEL_ID3TITLE))
    {
        GetSavedMusicDir(&stFindData, SortInfoAddr.ulFileFullInfoSectorAddr, SortInfoAddr.ulFileSortInfoSectorAddr, (id + MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep]));

        if (AudioFileInfo.ucSelPlayType == SORT_TYPE_SEL_FOLDER)
        {
            MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START) << 9) + 2 * (UINT32)(AudioFileInfo.FindData.Clus + AudioFileInfo.FindData.Index - 1), 2, (uint8 *)&temp);
            MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START) << 9) + sizeof(FILE_TREE_BASIC) * (UINT32)(AudioFileInfo.FindData.Clus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
            MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + MUSIC_SAVE_INFO_SECTOR_START) << 9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(FileTreeBasic.dwBasicInfoID) + DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *) & (DirClus));
            MDReadData(DataDiskID, ((UINT32)(MediaInfoAddr + MUSIC_SAVE_INFO_SECTOR_START) << 9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(FileTreeBasic.dwBasicInfoID) + DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *) & (Index));

            if ((stFindData.Clus == DirClus) &&
                (stFindData.Index == Index))
            {
                ret = 1;
            }
        }
        else
        {
            if ((stFindData.Clus == AudioFileInfo.FindData.Clus) &&
                (stFindData.Index == AudioFileInfo.FindData.Index))
            {
                ret = 1;
            }
        }
    }

    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : void MusicBroVariableInit(void)
  Author        : anzhiguo
  Description   : favourite module initial

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2008/07/21         Ver1.0
  desc:         according to favourite folder block flag 00fa,then to set or read out the file number
                of favourite and favourite sec address.
--------------------------------------------------------------------------------
*/

_ATTR_MEDIABROWIN_INIT_CODE_

void MusicBroVariableInit(void *pArg)
{
    UINT16 j, i;
    j = ((MEDIABRO_WIN_ARG*)pArg)->CurId;
    MusicBroUnknownCnt = 0;
    MusicBroAllfileCnt = 0;
    MediaBroTitle = ((MEDIABRO_WIN_ARG*)pArg)->TitleAdd;

    if (j)
    {
        MusicDirTreeInfo.MusicDirDeep = ((MEDIABRO_WIN_ARG*)pArg)->MediaDirTreeInfo.MusicDirDeep;

        for (i = 0; i <= MusicDirTreeInfo.MusicDirDeep; i++)
        {
            MusicDirTreeInfo.MusicDirBaseSortId[i] = ((MEDIABRO_WIN_ARG*)pArg)->MediaDirTreeInfo.MusicDirBaseSortId[i] ;
            MusicDirTreeInfo.CurId[i] = ((MEDIABRO_WIN_ARG*)pArg)->MediaDirTreeInfo.CurId[i];
        }

#ifdef _RECORD_

        if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
        {
            MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = AudioFileInfo.PlayedFileNum;
        }
        else
#endif
        {
            //Rk Aaron.sun
            if ((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0) )
            {
                MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = 0;
            }
            else
            {
                MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = AudioFileInfo.PlayedFileNum - 1;
            }
        }

        if (gPreviousFileNum != MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep])
        {
            if (gPreviousFileNum > MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep])
            {
                gMusicDirTreeInfo_KeyCounter -=  (gPreviousFileNum - MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]);

                if (gMusicDirTreeInfo_KeyCounter < 0)
                    gMusicDirTreeInfo_KeyCounter = 0;
            }
            else
            {
                gMusicDirTreeInfo_KeyCounter += (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - gPreviousFileNum);

                if (gMusicDirTreeInfo_KeyCounter > MAX_ITEM_NUM_MEDIABRO_DISP - 1)
                    gMusicDirTreeInfo_KeyCounter = MAX_ITEM_NUM_MEDIABRO_DISP - 1;
            }
        }

        if (gMusicDirTreeInfo_KeyCounter >= 0 && gMusicDirTreeInfo_KeyCounter < MAX_ITEM_NUM_MEDIABRO_DISP)
        {
            MusicDirTreeInfo.KeyCounter = gMusicDirTreeInfo_KeyCounter;
        }
        else
        {
            if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] >= MAX_ITEM_NUM_MEDIABRO_DISP - 1)
            {
                MusicDirTreeInfo.KeyCounter = MAX_ITEM_NUM_MEDIABRO_DISP - 1;
            }
            else
            {
                MusicDirTreeInfo.KeyCounter = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
            }
        }

        gPreviousFileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void MusicDirValueInit(void)
  Author        : anzhiguo
  Description   : MusicDirValueInit

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
void MusicDirValueInit(void)
{
    unsigned int i;
    unsigned int j = 0;

    //MusicBroItem clear zero to long file name and file number of MusicBroItem
    for (i = 0; i < MAX_ITEM_NUM_MEDIABRO_DISP; i++)
    {
        memset(&(MusicBroItem[i].LongFileName[j]), 0, MAX_FILENAME_LEN * 2);
        MusicBroItem[i].ItemNumber = 0xFFFF;
#ifdef THUMB_DEC_INCLUDE
        gThumbChkBuf[i] = 0x00;
#endif
    }

#ifdef THUMB_DEC_INCLUDE
    gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+1] = 0x00;
    gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+2] = 0x00;
    gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+3] = 0x00;
    gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+4] = 0x00;
#endif
    //the items in MusicBroItem build one double drection list
    for (i = 0; i < MAX_ITEM_NUM_MEDIABRO_DISP - 1; i++)
    {
        MusicBroItem[i].pNext = &MusicBroItem[i + 1];
    }

    for (i = 1; i < MAX_ITEM_NUM_MEDIABRO_DISP; i++)
    {
        MusicBroItem[i].pPrev = &MusicBroItem[i - 1];
    }

#ifdef THUMB_DEC_INCLUDE
    gThumbNowPicPos = 0;
    gThumbDecodeMode = 0;
    gThumbNowDecodeLine = 0;
#endif

    MusicBroItem[0].pPrev = &MusicBroItem[MAX_ITEM_NUM_MEDIABRO_DISP - 1];
    MusicBroItem[MAX_ITEM_NUM_MEDIABRO_DISP - 1].pNext = &MusicBroItem[0];
    MusicDirTreeInfo.PreCounter = 0;
    MusicDirTreeInfo.pMusicBro    = &MusicBroItem[0];
    MusicDirTreeInfo.CurId[0]     = 0;//uiDivValueTemp;
    MusicDirTreeInfo.CurId[1]     = 0;//uiDivValueTemp;
    MusicDirTreeInfo.CurId[2]     = 0;//uiDivValueTemp;
    MusicDirTreeInfo.CurId[3]     = 0;//uiDivValueTemp;
    MusicDirTreeInfo.MusicDirDeep      = 0;
    MusicDirTreeInfo.MusicDirTotalItem = 0;
    MusicDirTreeInfo.KeyCounter   = 0;
    MusicDirTreeInfo.ItemStar     = 0;
    MusicDirTreeInfo.MusicDirBaseSortId[0] = 0;
    MusicDirTreeInfo.MusicDirBaseSortId[1] = 0;
    MusicDirTreeInfo.MusicDirBaseSortId[2] = 0;
    MusicDirTreeInfo.MusicDirBaseSortId[3] = 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void MediaBroUpProc(UINT16 uiUpdateType)

  Author        : anzhiguo
  Description   : it is the handle child progarm of media libary module

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
void MediaBroUpProc(UINT16 uiUpdateType)
{
    MUSICBRO_STRUCT  *pBro;
    UINT16  StartItem = 0;
    UINT16  i, j;
    StartItem = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter;

    switch (uiUpdateType)
    {
        case ALL_BROITEM_UPDATE:
            //MusicBroUnknownCnt = 0;
            //MusicBroAllfileCnt = 0;
            for (i = 0; i < MAX_ITEM_NUM_MEDIABRO_DISP; i++) //clear the items that is diaplay
            {
                for (j = 0; j < MAX_FILENAME_LEN; j++)
                {
                    MusicBroItem[i].LongFileName[j] = 0;
                }

                MusicBroItem[i].ItemNumber = 0xFFFF;
                MusicBroItem[i].FileType = 0xFFFF;
            }

            MusicDirTreeInfo.pMusicBro = &MusicBroItem[0];
            pBro = MusicDirTreeInfo.pMusicBro;

            for (i = 0; (i < MAX_ITEM_NUM_MEDIABRO_DISP) && ((i + StartItem) < MusicDirTreeInfo.MusicDirTotalItem); i++)
            {
                // Rk Aaron.sun
                if ((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0) )
                {
                    if ((StartItem == 0) && (i == 0))
                    {
                        memcpy(pBro->LongFileName, L"All Album\0", 20);
                        MusicBroAllfileCnt = 1;
                    }
                    else
                    {
                        GetListItem(pBro->LongFileName, StartItem - 1 + i);
                        FomatTheOtherItem(pBro->LongFileName);
                    }
                }
                else
                {
                    GetListItem(pBro->LongFileName, StartItem + i);
                    FomatTheOtherItem(pBro->LongFileName);
                }

                pBro->FileType = FileTypeAudio;

                if (i >= (MAX_ITEM_NUM_MEDIABRO_DISP - 1))
                    break;

                pBro = pBro->pNext;
            }

            break;

        case UP_UPDATE:
            //MusicBroUnknownCnt = 0;
            //MusicBroAllfileCnt = 0;
            MusicDirTreeInfo.pMusicBro = MusicDirTreeInfo.pMusicBro->pPrev;
            pBro = MusicDirTreeInfo.pMusicBro;

            for (j = 0; j < MAX_FILENAME_LEN; j++)
            {
                pBro->LongFileName[j] = 0;
            }

            //RK Aaron.sun
            if ((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0) )
            {
                if (StartItem == 0)
                {
                    memcpy(pBro->LongFileName, L"All Album\0", 20);
                    MusicBroAllfileCnt = 1;
                }
                else
                {
                    GetListItem(pBro->LongFileName, StartItem - 1);
                    FomatTheOtherItem(pBro->LongFileName);
                }
            }
            else
            {
                GetListItem(pBro->LongFileName, StartItem);
                FomatTheOtherItem(pBro->LongFileName);
            }

            pBro->FileType = FileTypeAudio;
            break;

        case DOWN_UPDATE:
            //MusicBroUnknownCnt = 0;
            //MusicBroAllfileCnt = 0;
            pBro = MusicDirTreeInfo.pMusicBro;

            for (j = 0; j < MAX_FILENAME_LEN; j++)
            {
                pBro->LongFileName[j] = 0;
            }

            if ((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0) )
            {
                GetListItem(pBro->LongFileName, MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - 1);
                FomatTheOtherItem(pBro->LongFileName);
            }
            else
            {
                GetListItem(pBro->LongFileName, MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]);
                FomatTheOtherItem(pBro->LongFileName);
            }

            pBro->FileType = FileTypeAudio;
            MusicDirTreeInfo.pMusicBro = MusicDirTreeInfo.pMusicBro->pNext;
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void MediaBroUpProcWithConstantHead(UINT16 uiUpdateType)

  Author        : allen
  Description   : it is the handle child progarm of media libary module

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                allen     2013/09/24         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
void MediaBroUpProcWithConstantHead(UINT16 uiUpdateType)
{
    MUSICBRO_STRUCT  *pBro;
    UINT16  StartItem = 0;
    UINT16  i, j;
#if 1
    StartItem = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter;

    switch (uiUpdateType)
    {
        case ALL_BROITEM_UPDATE:
            for (i = 0; i < MAX_ITEM_NUM_MEDIABRO_DISP; i++) //clear the items that is diaplay
            {
                for (j = 0; j < MAX_FILENAME_LEN; j++)
                {
                    MusicBroItem[i].LongFileName[j] = 0;
                }

                MusicBroItem[i].ItemNumber = 0xFFFF;
                MusicBroItem[i].FileType = 0xFFFF;
            }

            MusicDirTreeInfo.pMusicBro = &MusicBroItem[0];
            pBro = MusicDirTreeInfo.pMusicBro;

            if (StartItem == 0)
            {
                #ifdef _RECORD_
                if (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE)
                {
                    GetResourceStr(SID_ALL_REC_DATA, pBro->LongFileName, MAX_FILENAME_LEN);
                }
                else if (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL)
                {
                    GetResourceStr(SID_DELETE_ALL, pBro->LongFileName, MAX_FILENAME_LEN);
                }
                #endif

                pBro->FileType = FileTypeAudio;
                pBro = pBro->pNext;

                for (i = 0; (i < MAX_ITEM_NUM_MEDIABRO_DISP - 1) && ((i + StartItem) < MusicDirTreeInfo.MusicDirTotalItem + 1); i++)
                {
                    GetListItem(pBro->LongFileName, StartItem + i);
                    FomatTheOtherItem(pBro->LongFileName);
                    pBro->FileType = FileTypeAudio;

                    if (i >= MAX_ITEM_NUM_MEDIABRO_DISP - 1)
                        break;

                    pBro = pBro->pNext;
                }
            }
            else
            {
                for (i = 0; (i < MAX_ITEM_NUM_MEDIABRO_DISP) && ((i + StartItem) < MusicDirTreeInfo.MusicDirTotalItem + 1); i++) //+1
                {
                    GetListItem(pBro->LongFileName, StartItem + i - 1); //-1
                    FomatTheOtherItem(pBro->LongFileName);
                    pBro->FileType = FileTypeAudio;

                    if (i >= MAX_ITEM_NUM_MEDIABRO_DISP - 1)
                        break;

                    pBro = pBro->pNext;
                }
            }

            break;

        case UP_UPDATE:
            MusicDirTreeInfo.pMusicBro = MusicDirTreeInfo.pMusicBro->pPrev;
            pBro = MusicDirTreeInfo.pMusicBro;

            for (j = 0; j < MAX_FILENAME_LEN; j++)
            {
                pBro->LongFileName[j] = 0;
            }

            if (StartItem == 0)
            {
                #ifdef _RECORD_
                if (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE)
                {
                    GetResourceStr(SID_ALL_REC_DATA, pBro->LongFileName, MAX_FILENAME_LEN);
                }
                else if (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL)
                {
                    GetResourceStr(SID_DELETE_ALL, pBro->LongFileName, MAX_FILENAME_LEN);
                }
                #endif

                pBro->FileType = FileTypeAudio;
                break;
            }

            //GetListItem(pBro->LongFileName, StartItem);
            GetListItem(pBro->LongFileName, StartItem - 1);
            FomatTheOtherItem(pBro->LongFileName);
            pBro->FileType = FileTypeAudio;
            break;

        case DOWN_UPDATE:
            pBro = MusicDirTreeInfo.pMusicBro;

            for (j = 0; j < MAX_FILENAME_LEN; j++)
            {
                pBro->LongFileName[j] = 0;
            }

            GetListItem(pBro->LongFileName, MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - 1);
            FomatTheOtherItem(pBro->LongFileName);
            pBro->FileType = FileTypeAudio;
            MusicDirTreeInfo.pMusicBro = MusicDirTreeInfo.pMusicBro->pNext;
            break;
    }

#endif
}

/*
--------------------------------------------------------------------------------
  Function name : void MusicBroMusicDirInit(void)
  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         call when key enter next layer,so permanent
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
void MusicBroMusicDirInit(void)
{
    MusicDirTreeInfo.MusicDirTotalItem = GetCurItemNum();//��ǰ��ʾĿ¼�µ��ļ�����
    MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep] = GetBaseId();//���������ʲô ?????? �ǵ�ǰ��ʾ��ʼ�ļ������?

    if (MusicDirTreeInfo.MusicDirTotalItem > MAX_ITEM_NUM_MEDIABRO_DISP)
    {
        MusicDirTreeInfo.DispTotalItem = MAX_ITEM_NUM_MEDIABRO_DISP;
    }
    else
    {
        MusicDirTreeInfo.DispTotalItem = MusicDirTreeInfo.MusicDirTotalItem;
    }

#ifdef _RECORD_

    if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
    {
        MediaBroUpProcWithConstantHead(ALL_BROITEM_UPDATE);
    }
    else
#endif
    {
        MediaBroUpProc(ALL_BROITEM_UPDATE);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void MusicBroModule(void)

  Author        : anzhiguo
  Description   : meida's module entance

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
void MediaBroInit(void *pArg)
{
    UINT16 i, j;
#ifdef THUMB_DEC_INCLUDE
    ModuleOverlay(MODULE_ID_PICTURE_CONTROL, MODULE_OVERLAY_ALL);
    gIsImproveFreq = FALSE;
    gTickCounter = SysTickCounter;
#endif
    SortInfoAddrInit();
    MusicDirValueInit();
    MusicBroVariableInit(pArg);
    /*
       if(j)
       {
           MusicDirTreeInfo.MusicDirDeep = AudioFileInfo.ucCurDeep;
           for(i=0;i<=MusicDirTreeInfo.MusicDirDeep;i++)
           {
               MusicDirTreeInfo.CurId[i] = AudioFileInfo.uiCurId[i];
           }
           MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = AudioFileInfo.CurrentFileNum -1;

           if(MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] >= MAX_ITEM_NUM_MEDIABRO_DISP-1)
           {
               MusicDirTreeInfo.KeyCounter = MAX_ITEM_NUM_MEDIABRO_DISP-1;
           }
           else
           {
               MusicDirTreeInfo.KeyCounter = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
           }
       }
      */
    MusicBroMusicDirInit();
    KeyReset();
    SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
    SendMsg(MSG_MEDIA_SCROLL_PAINT);
}

/*
--------------------------------------------------------------------------------
  Function name : void MediaBroDeInit(void)

  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
extern unsigned int Is_MetaData_Block_Picture;
extern HANDLE MetaBlockPicHandle;
_ATTR_MEDIABROWIN_CODE_
void MediaBroDeInit(void)
{
#ifdef THUMB_DEC_INCLUDE
    if(Is_MetaData_Block_Picture)
    {
        FileClose(MetaBlockPicHandle);
        MetaBlockPicHandle = -1;
        FileDelete("\\", "PICTURE OGG");

        Is_MetaData_Block_Picture = 0;
    }

    if (gIsImproveFreq)
    {
        gIsImproveFreq = FALSE;
        FREQ_ExitModule(FREQ_JPG);
    }

    if(gThumbDecodeMode == 1)
    {
        FileClose((HANDLE)gThumbHandle);
        gThumbHandle = (FILE*)-1;
    }
#endif
    ClearMsg(MSG_MEDIABRO_DISPFLAG_SCROLL_FILENAME);
}



/***************************key handle***********************************/
/******************************************************************************

                            key handle child module

*******************************************************************************/
/*
--------------------------------------------------------------------------------
  Function name : void MediaBroKey(void)

  Author        : anzhiguo
  Description   : entery function of meida module key handle

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_SERVICE_CODE_
UINT32 MediaBroKey(void)
{
    uint32 RetVal;
    UINT32 KeyVal;
    UINT16 TempFileNum;
    INT16 loop;
    TASK_ARG TaskArg;
    UINT16 TempDirTotalItem;
    RetVal = RETURN_OK;
    KeyVal =  GetKeyVal();

    //5 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (KeyVal)
        {
            case KEY_VAL_FFW_DOWN:
            case KEY_VAL_FFW_PRESS:
                KeyVal = KEY_VAL_UP_DOWN;
                break;

            case KEY_VAL_FFD_DOWN:
            case KEY_VAL_FFD_PRESS:
                KeyVal = KEY_VAL_DOWN_DOWN;
                break;

            default:
                break;
        }
    }

    //6 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
        switch (KeyVal)
        {
            case KEY_VAL_FFW_SHORT_UP:
                KeyVal = KEY_VAL_ESC_SHORT_UP;
                break;

            default:
                break;
        }
    }

    //7 Key function modification
    switch (KeyVal)
    {
        case KEY_VAL_MENU_SHORT_UP:
            MusicBroUnknownCnt = 0;
            MusicBroAllfileCnt = 0;
#ifdef _RECORD_
            if (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL)
            {
                if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] == 0)
                {
                    TaskArg.Dialog.ContentID = SID_DELETE_ALL_COMFIRM;
                }
                else
                {
                    TaskArg.Dialog.ContentID = SID_DELETE_CHANNEL;
                }

                TaskArg.Dialog.Button  = DIALOG_BUTTON_YES;
                TaskArg.Dialog.TitleID = SID_WARNING;
                WinCreat(&MediaBroWin, &DialogWin, &TaskArg);
                break;
            }

#endif

            if (MusicDirTreeInfo.MusicDirTotalItem == 0)
                break;

            ModuleOverlay(MODULE_ID_MEDIABRO_SORTGET, MODULE_OVERLAY_ALL);

            if (MusicDirTreeInfo.MusicDirDeep == 0)
            {
                switch (gMusicTypeSelID)
                {
                    case SORT_TYPE_SEL_FILENAME:
                    case SORT_TYPE_SEL_ID3TITLE:
                    case MUSIC_TYPE_SEL_MYFAVORITE:
#ifdef  _RECORD_
                    case MUSIC_TYPE_SEL_RECORDFILE: //
                    case MUSIC_TYPE_SEL_FMFILE:
#endif
                        RetVal = SaveAndPostMusicPlayInfo();
                        break;

                    case SORT_TYPE_SEL_ID3SINGER:
                    case SORT_TYPE_SEL_ID3ALBUM:
                    case SORT_TYPE_SEL_GENRE:
                        MusicDirTreeInfo.MusicDirDeep++;
                        {
                            MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = 0;
                        }
                        MusicDirTreeInfo.KeyCounter = 0;
                        MusicBroMusicDirInit();
                        SendMsg(MSG_MEDIABRO_ALL_ITEM);
                        break;

                    default:
                        break;
                }
            }
            else if (MusicDirTreeInfo.MusicDirDeep == 1)
            {
                switch (gMusicTypeSelID)
                {
                    case SORT_TYPE_SEL_ID3SINGER:

                        //Rk Aaron.sun
                        if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] == 0)
                        {
                            // Aritist->All Album
                            RetVal =  SaveAndPostMusicPlayInfo(); //sch120413 for mediabrowse
                            break;
                        }

                    case SORT_TYPE_SEL_GENRE:
                        MusicDirTreeInfo.MusicDirDeep++;
                        {
                            MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = 0;
                        }
                        MusicDirTreeInfo.KeyCounter = 0;
                        MusicBroMusicDirInit();
                        SendMsg(MSG_MEDIABRO_ALL_ITEM);
                        break;

                    case SORT_TYPE_SEL_ID3ALBUM:
                        RetVal =  SaveAndPostMusicPlayInfo(); //sch120413 for mediabrowse
                        break;

                    default:
                        break;
                }
            }
            else if (MusicDirTreeInfo.MusicDirDeep == 2)
            {
                switch (gMusicTypeSelID)
                {
                    case SORT_TYPE_SEL_GENRE:

                        //RK Aaron.sun
                        if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] == 0)
                        {
                            // Aritist->All Album
                            RetVal =  SaveAndPostMusicPlayInfo(); //sch120413 for mediabrowse
                        }
                        else
                        {
                            MusicDirTreeInfo.MusicDirDeep++;
                            {
                                MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = 0;
                            }
                            MusicDirTreeInfo.KeyCounter = 0;
                            MusicBroMusicDirInit();
                            SendMsg(MSG_MEDIABRO_ALL_ITEM);
                        }

                        break;

                    case SORT_TYPE_SEL_ID3SINGER:
                        RetVal =  SaveAndPostMusicPlayInfo(); //sch120413 for mediabrowse
                        break;

                    default:
                        break;
                }
            }
            else if (MusicDirTreeInfo.MusicDirDeep == 3)
            {
                switch (gMusicTypeSelID)
                {
                    case SORT_TYPE_SEL_GENRE:
                        RetVal =  SaveAndPostMusicPlayInfo(); //sch120413 for mediabrowse
                        break;

                    default:
                        break;
                }
            }

            gMusicDirTreeInfo_KeyCounter = MusicDirTreeInfo.KeyCounter;
            gPreviousFileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
            break;

        //--------------------- UP KEY ---------------------------
        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_SHORT_UP:
            if ((MusicDirTreeInfo.MusicDirTotalItem) == 0)
                break;

            MusicDirTreeInfo.PreCounter = MusicDirTreeInfo.KeyCounter;

            if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] > 0)
            {
                MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]--;

                if (MusicDirTreeInfo.KeyCounter == 0)
                {
#ifdef _RECORD_

                    if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
                    {
                        MediaBroUpProcWithConstantHead(UP_UPDATE);
                    }
                    else
#endif
                    {
                        MediaBroUpProc(UP_UPDATE);
                    }

#ifdef _FRAME_BUFFER_
                    MusicDirTreeInfo.PreCounter = 1;
                    //LCD_Shift_Window(0, DIRECTION_DOWN, 17, 0, 3, 122, 139);
                    SendMsg(MSG_MEDIABRO_FRESH_ITEM);

                    #ifdef THUMB_DEC_INCLUDE
                    SendMsg(MSG_MEDIABRO_FRESH_THUMBNAIL_UP);
                    #endif

#else
                    SendMsg(MSG_MEDIABRO_ALL_ITEM);
#endif
                }
                else
                {
                    MusicDirTreeInfo.KeyCounter--;
                    SendMsg(MSG_MEDIABRO_FRESH_ITEM);
                }
            }
            else
            {
                TempFileNum = MusicDirTreeInfo.MusicDirTotalItem;
#ifdef _RECORD_

                if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
                {
                    MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = TempFileNum;

                    if ( (TempFileNum + 1) > MAX_ITEM_NUM_MEDIABRO_DISP)
                    {
                        MusicDirTreeInfo.KeyCounter = MAX_ITEM_NUM_MEDIABRO_DISP - 1;
                        MusicBroMusicDirInit();
                        SendMsg(MSG_MEDIABRO_ALL_ITEM);
                    }
                    else
                    {
                        MusicDirTreeInfo.KeyCounter = TempFileNum;
                        SendMsg(MSG_MEDIABRO_FRESH_ITEM);
                    }
                }
                else
#endif
                {
                    MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = TempFileNum - 1;

                    if ( TempFileNum > MAX_ITEM_NUM_MEDIABRO_DISP)
                    {
                        MusicDirTreeInfo.KeyCounter = MAX_ITEM_NUM_MEDIABRO_DISP - 1;
                        MusicBroMusicDirInit();
                        SendMsg(MSG_MEDIABRO_ALL_ITEM);
                    }
                    else
                    {
                        MusicDirTreeInfo.KeyCounter = TempFileNum - 1;
                        SendMsg(MSG_MEDIABRO_FRESH_ITEM);
                    }
                }
            }

            SendMsg(MSG_MEDIA_SCROLL_PAINT);
            break;

        //------------------- DOWN KEY ---------------------------

        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP:
            if ((MusicDirTreeInfo.MusicDirTotalItem) == 0)
                break;

            MusicDirTreeInfo.PreCounter = MusicDirTreeInfo.KeyCounter;
#ifdef _RECORD_

            if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
            {
                TempDirTotalItem = MusicDirTreeInfo.MusicDirTotalItem;
            }
            else
#endif
            {
                TempDirTotalItem = MusicDirTreeInfo.MusicDirTotalItem - 1;
            }

            if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] < TempDirTotalItem)
            {
                MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]++;

                if (MusicDirTreeInfo.KeyCounter >= MAX_ITEM_NUM_MEDIABRO_DISP - 1)
                {
#ifdef _RECORD_

                    if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
                    {
                        MediaBroUpProcWithConstantHead(DOWN_UPDATE);
                    }
                    else
#endif
                    {
                        MediaBroUpProc(DOWN_UPDATE);
                    }

#ifdef _FRAME_BUFFER_
                    MusicDirTreeInfo.PreCounter = 6;
                    //LCD_Shift_Window(0, DIRECTION_UP, 17, 0, 3, 122, 139);
                    SendMsg(MSG_MEDIABRO_FRESH_ITEM);

                    #ifdef THUMB_DEC_INCLUDE
                    SendMsg(MSG_MEDIABRO_FRESH_THUMBNAIL_DOWN);
                    #endif


#else
                    SendMsg(MSG_MEDIABRO_ALL_ITEM);//display all screen
#endif
                }
                else
                {
                    MusicDirTreeInfo.KeyCounter++;
                    SendMsg(MSG_MEDIABRO_FRESH_ITEM);
                }
            }
            else
            {
                MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = 0;
                MusicDirTreeInfo.KeyCounter = 0;
#ifdef _RECORD_

                if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
                {
                    if ((MusicDirTreeInfo.MusicDirTotalItem + 1) > MAX_ITEM_NUM_MEDIABRO_DISP)
                    {
                        MusicBroMusicDirInit();
                        SendMsg(MSG_MEDIABRO_ALL_ITEM);
                    }
                    else
                    {
                        SendMsg(MSG_MEDIABRO_FRESH_ITEM);
                    }
                }
                else
#endif
                {
                    if (MusicDirTreeInfo.MusicDirTotalItem > MAX_ITEM_NUM_MEDIABRO_DISP)
                    {
                        MusicBroMusicDirInit();
                        SendMsg(MSG_MEDIABRO_ALL_ITEM);
                    }
                    else
                    {
                        SendMsg(MSG_MEDIABRO_FRESH_ITEM);
                    }
                }
            }

            SendMsg(MSG_MEDIA_SCROLL_PAINT);
            break;

        case KEY_VAL_PLAY_SHORT_UP://display sub window
            if (MusicDirTreeInfo.MusicDirTotalItem == 0)
                break;

            if (gMusicTypeSelID == MUSIC_TYPE_SEL_MYFAVORITE)
            {
#ifdef FAVOSUB
                ModuleOverlay(MODULE_ID_MEDIAFAVOSUB, MODULE_OVERLAY_ALL);
                WinCreat(&MediaBroWin, &MediaFavoSubWin, NULL);
#endif
            }
            else if (gMusicTypeSelID == SORT_TYPE_SEL_ID3TITLE ||
                     (gMusicTypeSelID == SORT_TYPE_SEL_ID3SINGER && MusicDirTreeInfo.MusicDirDeep == 2) ||
                     (gMusicTypeSelID == SORT_TYPE_SEL_ID3ALBUM && MusicDirTreeInfo.MusicDirDeep == 1) ||
                     (gMusicTypeSelID == SORT_TYPE_SEL_GENRE && MusicDirTreeInfo.MusicDirDeep == 3))
            {
#ifdef BROSUB
                ModuleOverlay(MODULE_ID_MEDIABROSUB, MODULE_OVERLAY_ALL);
                WinCreat(&MediaBroWin, &MediaBroSubWin, NULL);
#endif
            }

            break;

        case KEY_VAL_ESC_SHORT_UP:
            MusicBroUnknownCnt = 0;
            MusicBroAllfileCnt = 0;
#ifdef _RECORD_
            if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
            {
                TaskSwitch(TASK_ID_RADIO, NULL);
                RetVal = 1;
            }
            else if (MusicDirTreeInfo.MusicDirDeep == 0)
#else
            if (MusicDirTreeInfo.MusicDirDeep == 0)
#endif
            {
#ifdef _MUSIC_

                if ((FALSE == ThreadCheck(pMainThread, &MusicThread)))//&& (HoldOnPlayInfo.HoldMusicGetSign == 0))//no music playing
                {
                    TaskArg.Medialib.CurId = gMusicTypeSelID - 1;
                }
                else
#endif
                {
                    TaskArg.Medialib.CurId = gMusicTypeSelID;
                }

                TaskSwitch(TASK_ID_MEDIALIB, &TaskArg);
                RetVal = 1;
            }
            else
            {
                MusicDirTreeInfo.MusicDirDeep--;

                if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] >= MAX_ITEM_NUM_MEDIABRO_DISP - 1)
                {
                    MusicDirTreeInfo.KeyCounter =  MAX_ITEM_NUM_MEDIABRO_DISP - 1;
                }
                else
                {
                    MusicDirTreeInfo.KeyCounter = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
                }

                MusicBroMusicDirInit();
                SendMsg(MSG_MEDIABRO_ALL_ITEM);
                SendMsg(MSG_MEDIA_SCROLL_PAINT);
            }

            break;

        case KEY_VAL_HOLD_ON://8.4 azg
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&MediaBroWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF://8.4 azg
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&MediaBroWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return (RetVal);
}
/*
--------------------------------------------------------------------------------
  Function name : void SaveAndPostMusicPlayInfo(void)

  Author        : anzhiguo
  Description   : enter media player

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABRO_SORTGET_CODE_
UINT32 SaveAndPostMusicPlayInfo(void)
{
    UINT16   i;
    UINT16    uiFindResult;
    UINT16    temp1;
    UINT16    tempFileNum;
    UINT16    musicfilenum;
    UINT16    longname[SYS_SUPPROT_STRING_MAX_LEN + 1];
    TASK_ARG TaskArg;
    uint32 RetVal;
    RetVal = 0;

#ifdef _SDCARD_

    if (MemorySelect != FLASH0)
    {
        MemorySelect = FLASH0;
        FileSysSetup(MemorySelect);
    }

#endif
#ifdef _RECORD_

    if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_FMFILE))
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            if ((gMusicTypeSelID != AudioFileInfo.ucSelPlayType) || (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] != AudioFileInfo.CurrentFileNum))//��ͬר��֮����л�
            {
                ThreadDelete(&pMainThread, &MusicThread);
#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif

#ifdef AUDIOHOLDONPLAY  //4.23 zs ��Դ�����������л�������bug
                gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif
            }
            else
            {
                TaskArg.Music.MediaTitleAdd = MediaBroTitle;
                TaskSwitch(TASK_ID_MUSIC, &TaskArg);
                return 1;
            }
        }
        else
        {
#ifdef AUDIOHOLDONPLAY
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif
        }
    }
    else
#endif
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            if ((gMusicTypeSelID != AudioFileInfo.ucSelPlayType) || (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] != AudioFileInfo.CurrentFileNum - 1) || (MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep] != AudioFileInfo.uiBaseSortId[MusicDirTreeInfo.MusicDirDeep]) || (MusicDirTreeInfo.MusicDirDeep != gSysConfig.MediaDirTreeInfo.MusicDirDeep))//��ͬר��֮����л�
            {
                ThreadDelete(&pMainThread, &MusicThread);
#ifdef _BLUETOOTH_  //chad.ma add
                BluetoothThreadDelete(gbBTConnected);
#endif

#ifdef AUDIOHOLDONPLAY  //4.23 zs ��Դ�����������л�������bug
                gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif
            }
            else
            {
                TaskArg.Music.MediaTitleAdd = MediaBroTitle;
                TaskSwitch(TASK_ID_MUSIC, &TaskArg);
                return 1;
            }
        }
        else
        {
#ifdef AUDIOHOLDONPLAY
            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
#endif
        }
    }

    switch (gMusicTypeSelID)
    {
        case SORT_TYPE_SEL_FILENAME:
        case SORT_TYPE_SEL_ID3TITLE:
        case SORT_TYPE_SEL_ID3SINGER:
        case SORT_TYPE_SEL_ID3ALBUM:
        case SORT_TYPE_SEL_GENRE:

            //Rk Aaron.sun
            if ((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0) )
            {
                {
                    uint16 i, TotalNum, BaseID;
                    TotalNum = 0;

                    for (i = 0; i < MusicDirTreeInfo.MusicDirTotalItem - 1; i++)
                    {
                        TotalNum += (GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirTreeInfo.MusicDirDeep], MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep] + i, FIND_SUM_ITEMNUM) - 1);
                    }

                    DEBUG("All Album TotalNum = %d", TotalNum);
#if 1
                    BaseID = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirTreeInfo.MusicDirDeep],  MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep], FIND_SUM_SORTSTART); // first Song;
                    GetSavedMusicDir(&stFindData, SortInfoAddr.ulFileFullInfoSectorAddr, SortInfoAddr.ulFileSortInfoSectorAddr, BaseID);
                    uiFindResult = RETURN_OK;
                    DEBUG("st.clus = %d, st.index = %d", stFindData.Clus, stFindData.Index);
                    memcpy((uint8 *)&AudioFileInfo.FindData , (uint8 *)&stFindData , 8);
#ifdef _RK_CUE_
                    AudioFileInfo.FindData.IsCue        = stFindData.IsCue;
                    AudioFileInfo.FindData.CueStartTime = stFindData.CueStartTime;
                    AudioFileInfo.FindData.CueEndTime   = stFindData.CueEndTime;
#endif
                    MusicDirTreeInfo.MusicDirTotalItem =  TotalNum;
                    MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep] = BaseID;
#else
                    MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]++;
                    MusicDirTreeInfo.MusicDirDeep++;
                    MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep] = GetBaseId();
                    MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] = 0;
                    GetSavedMusicDir(&stFindData, SortInfoAddr.ulFileFullInfoSectorAddr, SortInfoAddr.ulFileSortInfoSectorAddr, MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep] );
                    uiFindResult = RETURN_OK;
                    printf("st.clus = %d, st.index = %d\n", stFindData.Clus, stFindData.Index);
                    memcpy((uint8 *)&AudioFileInfo.FindData , (uint8 *)&stFindData , 8);
                    MusicDirTreeInfo.MusicDirTotalItem =  TotalNum;
#endif
                }
            }
            else
            {
                GetSavedMusicDir(&stFindData, SortInfoAddr.ulFileFullInfoSectorAddr, SortInfoAddr.ulFileSortInfoSectorAddr, (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] + MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep]));
                //uiFindResult = FindFileByShortPath(&AudioFileInfo.Fdt,  &gPathBuffer[0], &gPathBuffer[PATH_SIZE]);
                uiFindResult = RETURN_OK;
                DEBUG("st.clus = %d, st.index = %d\n", stFindData.Clus, stFindData.Index);
                memcpy((uint8 *)&AudioFileInfo.FindData , (uint8 *)&stFindData , 8);
#ifdef _RK_CUE_
                AudioFileInfo.FindData.IsCue        = stFindData.IsCue;
                AudioFileInfo.FindData.CueStartTime = stFindData.CueStartTime;
                AudioFileInfo.FindData.CueEndTime   = stFindData.CueEndTime;
#endif
            }

            break;

        case MUSIC_TYPE_SEL_MYFAVORITE:
#ifdef FAVOSUB
            uiFindResult = GetFavoInfo(&stFindData, MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep], (UINT8*)longname);
            memcpy((uint8 *)&AudioFileInfo.FindData , (uint8 *)&stFindData , 8);
#endif
            break;

        case SORT_TYPE_PLAY_LIST:
            //GetPlayListFilePath((MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]+ MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep]), gPathBuffer);
            //uiFindResult = FindFileByShortPath(&AudioFileInfo.Fdt,  &gPathBuffer[0], &gPathBuffer[PATH_SIZE]);
            //memcpy(AudioFileInfo.Path , gPathBuffer ,PATH_SIZE);
            break;
#ifdef  _RECORD_

        case MUSIC_TYPE_SEL_RECORDFILE:
        {
            //FIND_DATA FindDataPlay;
//          ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
#if 0
            tempFileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];

            if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] != 0)
            {
                tempFileNum--;
            }

            AudioFileInfo.FindData.Clus = RecordDirClus;
            AudioFileInfo.FindData.Index = 0;
            AudioFileInfo.FindData.TotalItem = MusicDirTreeInfo.MusicDirTotalItem;//SCH

            for (temp1 = 0; temp1 <= tempFileNum; temp1++)
            {
                uiFindResult = FindNextFile(&AudioFileInfo.Fdt, &AudioFileInfo.FindData, AudioFileExtString, FS_FAT);
            }

#else

            if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] > 0)
            {
                tempFileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - 1;
            }
            else
            {
                tempFileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
            }

            AudioFileInfo.FindData.Clus = RecordDirClus;
            AudioFileInfo.FindData.Index = 0;
            AudioFileInfo.FindData.TotalItem = MusicDirTreeInfo.MusicDirTotalItem;

            for (temp1 = 0; temp1 <= tempFileNum; temp1++)
            {
                uiFindResult = FindNextFile(&AudioFileInfo.Fdt, &AudioFileInfo.FindData, RecordFileExtString, RECORD_DB);
            }

            uiFindResult = RETURN_OK;
#endif
            break;
        }

        case MUSIC_TYPE_SEL_FMFILE:
        {
            tempFileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
            AudioFileInfo.FindData.Clus = RecordDirClus;
            AudioFileInfo.FindData.Index = tempFileNum + 1;
            AudioFileInfo.FindData.TotalItem = MusicDirTreeInfo.MusicDirTotalItem;
            uiFindResult = RETURN_OK;
            break;
        }

#endif

        default:
            break;
    }

    if (RETURN_OK == uiFindResult)
    {
        AudioFileInfo.ucSelPlayType = gMusicTypeSelID;
        AudioFileInfo.ulFullInfoSectorAddr = SortInfoAddr.ulFileFullInfoSectorAddr;
        AudioFileInfo.ulSortInfoSectorAddr = SortInfoAddr.ulFileSortInfoSectorAddr;

        for (i = 0; i <= MusicDirTreeInfo.MusicDirDeep; i++)
        {
            AudioFileInfo.uiCurId[i] = MusicDirTreeInfo.CurId[i];
            AudioFileInfo.uiBaseSortId[i] = MusicDirTreeInfo.MusicDirBaseSortId[i];
        }

        AudioFileInfo.ucCurDeep = MusicDirTreeInfo.MusicDirDeep;
        //AudioFileInfo.uiBaseSortId = MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
        AudioFileInfo.TotalFiles = MusicDirTreeInfo.MusicDirTotalItem;
#ifdef _RECORD_

        if (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE)
        {
            if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] == 0)
            {
                AudioFileInfo.CurrentFileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] + 1; //for All Rec Data item
            }
            else
            {
                AudioFileInfo.CurrentFileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];  //remove +1
            }

            //workaround
            if (AudioFileInfo.CurrentFileNum == (AudioFileInfo.PlayedFileNum - 1)) //bug:fail to play last track
                ThreadDeleteAll(&pMainThread);
        }
        else
#endif
        {
            AudioFileInfo.CurrentFileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] + 1;
        }

        AudioFileInfo.pExtStr = MusicFileExtString;
        AudioFileInfo.PlayedFileNum = AudioFileInfo.CurrentFileNum;
        GetMediaItemInfo(gMusicTypeSelName, SortInfoAddr, (MusicDirTreeInfo.CurId[0] + MusicDirTreeInfo.MusicDirBaseSortId[0]), MEDIA_ID3_SAVE_CHAR_NUM, 0, 0);
        RetVal = 1;
    }
    else
    {
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&MediaBroWin, &MessageBoxWin, &TaskArg);
        SendMsg(MSG_MEDIABRO_NOFIND_FILE);
        return 0;
    }

#ifdef _MUSIC_
#ifdef _RECORD_

    if (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE)
    {
        TaskArg.Music.FileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
    }
    else
#endif
    {
        TaskArg.Music.FileNum = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] + 1;
        DEBUG("music filenum = %d",TaskArg.Music.FileNum);
    }

    gSysConfig.MediaDirTreeInfo.MusicDirDeep = MusicDirTreeInfo.MusicDirDeep;
    gSysConfig.MediaDirTreeInfo.CurId[0] = MusicDirTreeInfo.CurId[0];
    gSysConfig.MediaDirTreeInfo.MusicDirBaseSortId[0] = MusicDirTreeInfo.MusicDirBaseSortId[0];
    gSysConfig.MediaDirTreeInfo.CurId[1] = MusicDirTreeInfo.CurId[1];
    gSysConfig.MediaDirTreeInfo.MusicDirBaseSortId[1] = MusicDirTreeInfo.MusicDirBaseSortId[1];
    gSysConfig.MediaDirTreeInfo.CurId[2] = MusicDirTreeInfo.CurId[2];
    gSysConfig.MediaDirTreeInfo.MusicDirBaseSortId[2] = MusicDirTreeInfo.MusicDirBaseSortId[2];
    gSysConfig.MediaDirTreeInfo.CurId[3] = MusicDirTreeInfo.CurId[3];
    gSysConfig.MediaDirTreeInfo.MusicDirBaseSortId[3] = MusicDirTreeInfo.MusicDirBaseSortId[3];
    TaskArg.Music.MediaTitleAdd = MediaBroTitle;
    TaskSwitch(TASK_ID_MUSIC, &TaskArg);
    return RetVal;
#endif
}
/*
--------------------------------------------------------------------------------
  Function name : void FomatTheOtherItem(UINT16 *ucFileName)

  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
void FomatTheOtherItem(UINT16 *ucFileName)
{
    unsigned int j;

    if (ucFileName[0] == 0) // (ucFileName[0]==0x20)
    {
        for (j = 0; j < MAX_FILENAME_LEN; j++)
        {
            if (ucFileName[j] != 0)
            {
                break;
            }
        }

        if (j == MAX_FILENAME_LEN)
        {
            //ucFileName[0]=0x003c; // '<'
            //ucFileName[1]=0x004F; // 'O'
            //ucFileName[2]=0x0074; // 't'
            //ucFileName[3]=0x0068; // 'h'
            //ucFileName[4]=0x0065; // 'e'
            //ucFileName[5]=0x0072; // 'r'
            //ucFileName[6]=0x003e; // '>'
            ucFileName[0] = 'U'; // '<'
            ucFileName[1] = 'n'; // 'O'
            ucFileName[2] = 'K'; // 't'
            ucFileName[3] = 'n'; // 'h'
            ucFileName[4] = 'o'; // 'e'
            ucFileName[5] = 'w'; // 'r'
            ucFileName[6] = 'n'; // '>'
            MusicBroUnknownCnt = 1;
        }
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void FomatTheOtherItem(UINT16 *ucFileName)

  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
UINT16 GetListItem(UINT16 *pListName, UINT16 uiListNO)
{
    unsigned int i;
    FDT BroFDTBuf;
    FIND_DATA FindDataBro;
    UINT8 buf[PATH_SIZE];

    switch (MusicDirTreeInfo.MusicDirDeep)
    {
        case 0:
        {
            switch (gMusicTypeSelID)
            {
                case SORT_TYPE_SEL_FILENAME:  //��ȡ���ļ���
                    GetMediaItemInfo(pListName, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep, 1);
                    // GetSavedMusicFileName(pListName, SortInfoAddr, uiListNO, SYS_SUPPROT_STRING_MAX_LEN, MusicDirTreeInfo.MusicDirDeep);
                    break;

                case SORT_TYPE_SEL_ID3TITLE://��ȡID3Title��Ϣ
                    GetMediaItemInfo(pListName, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep, 1);
                    //  GetSavedMusicFileName(pListName, SortInfoAddr, uiListNO, MEDIA_ID3_SAVE_CHAR_NUM, MusicDirTreeInfo.MusicDirDeep,1);
                    break;

                case SORT_TYPE_SEL_ID3ALBUM:    //��ȡ������ר����Ϣ
                case SORT_TYPE_SEL_ID3SINGER:   //��ȡ�����ĸ�����Ϣ
                case SORT_TYPE_SEL_GENRE:       //��ȡ������������Ϣ
                    uiListNO += MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep, 0);
                    // GetSummaryFileName(pListName, SortInfoAddr, uiListNO,MusicDirTreeInfo.MusicDirDeep);
                    break;

                case MUSIC_TYPE_SEL_MYFAVORITE:
#ifdef FAVOSUB
                    GetFavoInfo(&stFindData, uiListNO, pListName);
#endif
                    break;
#ifdef  _RECORD_

                case MUSIC_TYPE_SEL_RECORDFILE_DEL:
                case MUSIC_TYPE_SEL_RECORDFILE:
//                    ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
#if 0
                    FindDataBro.Clus = RecordDirClus;
                    FindDataBro.Index = 0;

                    for (i = 0; i < (uiListNO + 1); i++)
                    {
                        FindNextFile(&BroFDTBuf, &FindDataBro, RecordFileExtString, FS_FAT);
                    }

                    GetLongFileName(FindDataBro.Clus, FindDataBro.Index - 1, FS_FAT, pListName);
#else
                    //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
                    FindDataBro.Clus = RecordDirClus;
                    FindDataBro.Index = 0;
                    FindDataBro.TotalItem = gSysConfig.MedialibPara.gRecordFmFileNum;

                    for (i = 0; i < (uiListNO + 1); i++)
                    {
                        FindNextFile(&BroFDTBuf, &FindDataBro, RecordFileExtString, RECORD_DB);
                    }

                    GetLongFileName(FindDataBro.Clus, FindDataBro.Index - 1, RECORD_DB, pListName);
#endif
                    break;

                case MUSIC_TYPE_SEL_FMFILE:
                    //ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);
                    FindDataBro.Clus = RecordDirClus;
                    FindDataBro.Index = 0;
                    FindDataBro.TotalItem = gSysConfig.MedialibPara.gRecordFmFileNum;

                    for (i = 0; i < (uiListNO + 1); i++)
                    {
                        FindNextFile(&BroFDTBuf, &FindDataBro, RecordFileExtString, RECORD_DB);
                    }

                    GetLongFileName(FindDataBro.Clus, FindDataBro.Index - 1, RECORD_DB, pListName);
                    break;
#endif

                default:
                    for (i = 0; i < MAX_FILENAME_LEN; i++)
                    {
                        *pListName++ = 0;
                    }

                    break;
            }

            break;
        }

        case 1://�����(����-���֣�,����-ר����ר��-���ļ�����)
        {
            switch (gMusicTypeSelID)
            {
                case SORT_TYPE_SEL_ID3ALBUM:    //��ȡ������ר����Ϣ
                    uiListNO += MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep, 1);
                    break;

                case SORT_TYPE_SEL_ID3SINGER:   //��ȡ�����ĸ�����Ϣ
                case SORT_TYPE_SEL_GENRE:       //��ȡ������������Ϣ
                    uiListNO += MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep, 0);
                    break;

                case SORT_TYPE_PLAY_LIST:
                    uiListNO += MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                    //GetPlayListFileName(uiListNO, pListName);
                    break;

                default:
                    break;
            }

            break;
        }

        case 2://�ڲ�(����-����-ר����,����-ר��-���ļ�����)
        {
            switch (gMusicTypeSelID)
            {
                case SORT_TYPE_SEL_ID3SINGER:    //��ȡ������ר����Ϣ
                    uiListNO += MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep, 1);
                    break;

                case SORT_TYPE_SEL_GENRE:       //��ȡ������������Ϣ
                    uiListNO += MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep, 0);
                    // GetSummaryFileName(pListName, SortInfoAddr, uiListNO,MusicDirTreeInfo.MusicDirDeep);
                    break;

                default:
                    break;
            }

            break;
        }

        case 3://�ڲ�(����-����-ר��-���ļ�����)
        {
            switch (gMusicTypeSelID)
            {
                case SORT_TYPE_SEL_GENRE:       //��ȡ������������Ϣ
                    uiListNO += MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep, 1);
                    //GetSummaryFileName(pListName, SortInfoAddr, uiListNO,MusicDirTreeInfo.MusicDirDeep);
                    break;

                default:
                    break;
            }

            break;
        }

        default:
            break;
    }

    FileExtNameRemove(pListName, AudioFileExtString);
    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name : void GetListItemIconId()

  Author        : Allen
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                Allen              2013/09/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
UINT32 GetListItemIconId()
{
#if 0
    unsigned int i;
    FDT BroFDTBuf;
    FIND_DATA FindDataBro;
    UINT8 buf[PATH_SIZE];
    UINT16 longname[SYS_SUPPROT_STRING_MAX_LEN + 1];
    UINT32 RetVal = IMG_ID_MUSIC_MENU_SONG;

    switch (MusicDirTreeInfo.MusicDirDeep)
    {
        case 0:
        {
            switch (gMusicTypeSelID)
            {
                case SORT_TYPE_SEL_FILENAME:
                    RetVal = IMG_ID_MUSIC_MENU_SONG;
                    break;

                case SORT_TYPE_SEL_ID3SINGER:
                    RetVal = IMG_ID_MUSIC_MENU_ARTIST;
                    break;

                case SORT_TYPE_SEL_ID3ALBUM:
                    RetVal = IMG_ID_MUSIC_MENU_ALBUM;
                    break;

                case SORT_TYPE_SEL_GENRE:
                    RetVal = IMG_ID_MUSIC_MENU_GENRE;
                    break;

                default:
                    break;
            }

            break;
        }

        case 1:
        {
            switch (gMusicTypeSelID)
            {
                case SORT_TYPE_SEL_ID3SINGER:
                    RetVal = IMG_ID_MUSIC_MENU_ALBUM;
                    break;

                case SORT_TYPE_SEL_ID3ALBUM:
                    RetVal = IMG_ID_MUSIC_MENU_SONG;
                    break;

                case SORT_TYPE_SEL_GENRE:
                    RetVal = IMG_ID_MUSIC_MENU_ARTIST;
                    break;

                default:
                    break;
            }

            break;
        }

        case 2:
        {
            switch (gMusicTypeSelID)
            {
                case SORT_TYPE_SEL_ID3SINGER:
                    RetVal = IMG_ID_MUSIC_MENU_SONG;
                    break;

                case SORT_TYPE_SEL_GENRE:
                    RetVal = IMG_ID_MUSIC_MENU_ALBUM;
                    break;

                default:
                    break;
            }

            break;
        }

        case 3:
        {
            switch (gMusicTypeSelID)
            {
                case SORT_TYPE_SEL_GENRE:
                    RetVal = IMG_ID_MUSIC_MENU_SONG;
                    break;

                default:
                    break;
            }

            break;
        }

        default:
            break;
    }

    return RetVal;
#endif
}
/*
--------------------------------------------------------------------------------
  Function name : void   MediaBroService(void)


  Author        : anzhiguo
  Description   : ý���ģ����ʾģ��

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_SERVICE_CODE_
UINT32 MediaBroService(void)
{
    TASK_ARG TaskArg;
    UINT32 RetVal = 0;

    if (GetMsg(MSG_MEDIAFAVO_DELALL))
    {
        TaskArg.Message.TitleID   = SID_WARNING;
        TaskArg.Message.ContentID = SID_NO_MUSIC_FILE;
        TaskArg.Message.HoldTime  = 3;
        TaskArg.Message.CurDisFrameIndex = 0;
        TaskArg.Message.UnsupportFrameIndex = 1;
        WinCreat(&MediaBroWin, &MessageBoxWin, &TaskArg);
    }

    if (CheckMsg(MSG_MEDIABRO_NOFIND_FILE))
    {
        if (CheckMsg(MSG_MESSAGEBOX_DESTROY))
        {
            ClearMsg(MSG_MEDIABRO_NOFIND_FILE);
            ClearMsg(MSG_MESSAGEBOX_DESTROY);
            SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
        }
    }

    if (TRUE == GetMsg(MSG_MESSAGEBOX_DESTROY))
    {
#ifdef _RECORD_

        if (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL)
        {
            TaskSwitch(TASK_ID_RADIO, NULL);
            RetVal = 1;
        }
        else
#endif
        {
#ifdef _MUSIC_

            if ((FALSE == ThreadCheck(pMainThread, &MusicThread)))//&& (HoldOnPlayInfo.HoldMusicGetSign == 0))//û�к�̨����
            {
                TaskArg.Medialib.CurId = gMusicTypeSelID - 1;
            }
            else
#endif
            {
                TaskArg.Medialib.CurId = gMusicTypeSelID;
            }

            TaskSwitch(TASK_ID_MEDIALIB, &TaskArg);
            RetVal = 1;
        }
    }

#ifdef _RECORD_

    if (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL)
    {
        if ((TRUE == GetMsg(MSG_DIALOG_KEY_OK)))
        {
            if (TRUE == ThreadCheck(pMainThread, &MusicThread))
            {
                ThreadDelete(&pMainThread, &MusicThread);
            }

#ifdef _BLUETOOTH_  //chad.ma add
            BluetoothThreadDelete(gbBTConnected);
#endif

            gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
            //DeleteRecFile();
            TaskArg.Message.ContentID = SID_DELETING;
            TaskArg.Message.TitleID   = SID_WARNING;
            TaskArg.Message.HoldTime  = 2;
            TaskArg.Message.CurDisFrameIndex = 0;
            TaskArg.Message.UnsupportFrameIndex = 1;
            MessageBoxHook = DeleteRecFile;
            WinCreat(&MediaBroWin, &MessageBoxWin, &TaskArg);
        }

        if ((TRUE == GetMsg(MSG_DIALOG_KEY_CANCEL)))  //back to preset list screen too
        {
            SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
        }
    }

#endif

    if (TRUE == GetMsg(MSG_MEDIABRO_DEMO_ALL_SONGS))
    {
        gSysConfig.MusicConfig.Eq.Mode = 5;           //EQ none
        SendMsg(MSG_AUDIO_EQSET_UPDATA);
        gSysConfig.MusicConfig.RepeatMode = 0;        //Repeat off
        SendMsg(MSG_SERVICE_MUSIC_MODE_UPDATE);
        gSysConfig.OutputVolume = 13;                 //Volume 13
        gSysConfig.BLevel = 4;                        //Brightness leve 5
        BL_SetLevel(4);
        gSysConfig.BLtime = 5;                        //Screen off timer 30 mins
        BacklightSystickCounterBack = SysTickCounter;
        ModuleOverlay(MODULE_ID_MEDIABRO_SORTGET, MODULE_OVERLAY_ALL);
        ThreadDeleteAll(&pMainThread);
        RetVal = SaveAndPostMusicPlayInfo();
    }

    return RetVal;
}

/*
--------------------------------------------------------------------------------
  Function name : BrowserValueInit
  Author        : chenfen
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                chenfen      2009/03/16         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_SERVICE_CODE_
void MediaBroWinScrollInit(LCD_RECT *pRect, uint16 ImageID, uint16 *pstr, uint16 Speed)
{
    PicturePartInfo     PicPartInfo;
    LCD_RECT           pRect1;
    PicPartInfo.x       = pRect->x0;
    PicPartInfo.y       = pRect->y0;
    PicPartInfo.yoffset = 0;
    PicPartInfo.ysize   = 16;
    pRect1.x0 = pRect->x0;
    pRect1.y0 = pRect->y0 + 2;
    pRect1.x1 = pRect->x1;
    pRect1.y1 = pRect->y0 + CH_CHAR_XSIZE_12 - 1;;
    PicPartInfo.pictureIDNump = ImageID;
    SetScrollStringInfo(&pRect1, PicPartInfo, pstr, Speed);
}


/*
--------------------------------------------------------------------------------
  Function name : void   MediaBroDisplay(void)


  Author        : anzhiguo
  Description   : the module of media display

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_SERVICE_CODE_
void   MediaBroDisplay(void)
{
    MUSICBRO_STRUCT  *pBro;
    MUSIC_DIR_TREE_STRUCT *pMusicDirInfo;
    LCD_RECT      r;
    UINT32 MusicBroNFAddr, PicBroNFAddr;
    UINT16  i,j,k,m,n,ret;
    UINT16  color_temp;
    UINT16  temp_iD;
    UINT16  gap;
    INT16  fno_temp;
    UINT16 uiListNO;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;
    int loop;
    UINT16 tempfilename[MAX_FILENAME_LEN];
#ifdef THUMB_DEC_INCLUDE
    //FILE* AudioHandle;
    UINT16  StartItem = 0;
#endif
    UINT16  TotalItem, TempColor, TempCharSize, TextMode;
    pBro = MusicDirTreeInfo.pMusicBro;
    TempColor = LCD_GetColor();
    LCD_SetColor(COLOR_BLACK);
    TempCharSize = LCD_SetCharSize(FONT_12x12);
    TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
#ifdef THUMB_DEC_INCLUDE
    ImageMaxHeight = 12;
    ImageMaxWidth = 12;
    SetPicFileType(AUDIO_PIC);
    IsDisplayBackground(0);
    StartItem = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter;
#endif

    //if(CheckMsg(MSG_NEED_PAINT_ALL)&& (MediaBroWin.Son != NULL ))  //���Ҫ�У��������Ӵ���ʱ��������� 5.20 anzhiguo
    //    return;
#ifdef THUMB_DEC_INCLUDE
    if(GetMsg(MSG_MEDIABRO_FRESH_THUMBNAIL_DOWN))
    {
        LCD_Shift_Window(0, DIRECTION_UP, 17, 0, 3, 122, 139);
        if(1 == check_album())
        {
            if(gThumbDecodeMode == 1)
            {
                FileClose((HANDLE)gThumbHandle);
                gThumbHandle = (FILE*)-1;
            }
            gThumbDecodeMode = 0;
            gThumbNowDecodeLine = 0;
            for(loop = 0; loop < (MAX_ITEM_NUM_MEDIABRO_DISP-1); loop++)
            {
                gThumbChkBuf[loop] = gThumbChkBuf[loop+1];
            }
            gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1] = gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+1];
            gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+1] = gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+2];
            gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+2] = 0;
            gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+4] = gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+3];
            gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+3] = 0;
            GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
            GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);
            ImageLeft = PicInfo1.x;
            ImageTop = 5 + 17 * (MAX_ITEM_NUM_MEDIABRO_DISP-1);
            if(0xff == gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1])
            {
                ThumbJpgDrawBmp(gThumbDecP1Buf);
            }
            else
            {
                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + FileTypeAudio, PicInfo1.x, PicInfo.y+5+ 17 *  MusicDirTreeInfo.KeyCounter);
            }
            memcpy(&gThumbDecP1Buf[0], &gThumbDecP2Buf[0], 12*12*2*2/*byte*/);
            memcpy(&gThumbDecM2Buf[0], &gThumbDecM1Buf[0], 12*12*2*2/*byte*/);
            SendMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL);
        }
    }

    if(GetMsg(MSG_MEDIABRO_FRESH_THUMBNAIL_UP))
    {
        LCD_Shift_Window(0, DIRECTION_DOWN, 17, 0, 3, 122, 139);
        if(1 == check_album())
        {
            if(gThumbDecodeMode == 1)
            {
                FileClose((HANDLE)gThumbHandle);
                gThumbHandle = (FILE*)-1;
            }
            gThumbDecodeMode = 0;
            gThumbNowDecodeLine = 0;
            for(loop = MAX_ITEM_NUM_MEDIABRO_DISP-2/**/; loop >= 0; loop--)
            {
                gThumbChkBuf[loop+1] = gThumbChkBuf[loop];
            }
            gThumbChkBuf[0] = gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+3];
            gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+3] = gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+4];
            gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+4] = 0;
            gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+2] = gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+1];
            gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+1] = 0;
            GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
            GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);
            ImageLeft = PicInfo1.x;
            ImageTop = 5 + 17 * (0);
            if((0xff == gThumbChkBuf[0]) && (StartItem >= (MusicBroUnknownCnt + MusicBroAllfileCnt)))
            {
                ThumbJpgDrawBmp(gThumbDecM1Buf);
            }
            else
            {
                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + FileTypeAudio, PicInfo1.x, PicInfo.y+5+ 17 *  MusicDirTreeInfo.KeyCounter);
            }
            memcpy(&gThumbDecP2Buf[0], &gThumbDecP1Buf[0], 12*12*2*2/*byte*/);
            memcpy(&gThumbDecM1Buf[0], &gThumbDecM2Buf[0], 12*12*2*2/*byte*/);
            SendMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL);
        }
    }
#endif

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_MEDIABRO_DISPLAY_ALL)))
    {
        //Display BackGround
        DispPictureWithIDNum(IMG_ID_BROWSER_BACKGROUND);
        //Display Title
        DisplayMenuStrWithIDNum(MEDIABRO_TITLE_TXT_X, MEDIABRO_TITLE_TXT_Y,
                                MEDIABRO_TITLE_TXT_XSIZE, MEDIABRO_TITLE_TXT_YSIZE,
                                LCD_TEXTALIGN_CENTER, MediaBroTitle );
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + BrowserBatteryLevel, 105, 146);
        //Send other display message
        SendMsg(MSG_MEDIABRO_ALL_ITEM);
    }

    if (TRUE == GetMsg(MSG_BATTERY_UPDATE))
    {
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_BATTERY01 + BrowserBatteryLevel, 105, 146);
    }

    if (GetMsg(MSG_MEDIABRO_ALL_ITEM))
    {
        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND, 0, 0, 3, 143);
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);
        TotalItem = MusicDirTreeInfo.DispTotalItem;
#ifdef _RECORD_

        if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
        {
            TotalItem++;
        }

#endif

        //if(1 == check_album())
        //{
        //  UINT16  temp_iD;
        //
        //  MusicDirThumbTreeInfo.MusicDirTotalItem = MusicDirTreeInfo.MusicDirTotalItem;
        //  MusicDirThumbTreeInfo.MusicDirDeep = MusicDirTreeInfo.MusicDirDeep;
        //  MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep] = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter;
        //  MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep] += MusicBroUnknownCnt;// unknown
        //  MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep] = MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
        //
        //  MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep+1] = 0;//1 st song
        //  temp_iD = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirThumbTreeInfo.MusicDirDeep-1+1],
        //              MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep-1+1] + MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep-1+1], FIND_SUM_SORTSTART);
        //  MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1] = temp_iD;
        //}

#ifdef THUMB_DEC_INCLUDE

        if (!gIsImproveFreq)
        {
            gIsImproveFreq = TRUE;
            gTickCounter = SysTickCounter;
            FREQ_EnterModule(FREQ_JPG); //��������ͼ���뷴����Ƶ����Ƶ
        }

        if(gThumbDecodeMode == 1)
        {
            FileClose((HANDLE)gThumbHandle);
            gThumbHandle = (FILE*)-1;
        }
        gThumbDecodeMode = 0;
        gThumbNowDecodeLine = 0;
#endif

        for (i = 0; ((i < MAX_ITEM_NUM_MEDIABRO_DISP) && (i < TotalItem )); i++)
        {
            //Display file Type Icon
#ifdef THUMB_DEC_INCLUDE
            //if( (1 == check_album()) && (i >= MusicBroUnknownCnt) && (i >= MusicBroAllfileCnt) )        //<----sanshin_20150625
            if( (1 == check_album()) && ((StartItem + i) >= (MusicBroUnknownCnt + MusicBroAllfileCnt)) )  //<----sanshin_20151110
            {
                gThumbChkBuf[i]=0x00;
                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType,PicInfo1.x, PicInfo.y+5+ 17 * i);
                SendMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL);
                //GetSavedMusicDir(&stFindData,
                //                 SortInfoAddr.ulFileFullInfoSectorAddr,
                //                 SortInfoAddr.ulFileSortInfoSectorAddr,
                //                 MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1]);
                //gThumbHandle = (FILE*)FileOpen(NULL, stFindData.Clus, stFindData.Index - 1, FS_FAT, FileOpenStringR);
                //if ((int)gThumbHandle == NOT_OPEN_FILE)
                //{
                //    DEBUG("=====FileOpen error=====\n");
                //    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType,
                //                              PicInfo1.x, PicInfo.y+5+ 17 * i);
                //}
                //else
                //{
                //  gTickCounter = SysTickCounter;
                //  ImageLeft = PicInfo1.x;
                //  ImageTop = 5 + 17 * i;
                //
                //  if (!ThumbParse(gThumbHandle))
                //  {
                //      //printf("\n Draw %s Icon.",FileType2Str(pBro->FileType));
                //      DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType,
                //                                PicInfo1.x, PicInfo.y+5+ 17 * i);
                //    }
                //
                //    FileClose((HANDLE)gThumbHandle);
                //}
            }
            else
            {
                #ifdef THUMB_DEC_INCLUDE
                ClearMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL);
                gThumbChkBuf[i]=0x00;
                #endif

                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType,PicInfo1.x, PicInfo.y+5+ 17 * i);
            }

#else
            DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType,
                                      PicInfo1.x, PicInfo.y + 5 + 17 * i);
#endif
            r.x0 = 20;
            r.y0 = i * 17 + 3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0 + 17; //r.y0 + CH_CHAR_XSIZE_12 - 1;

            if (i == MusicDirTreeInfo.KeyCounter)
            {
                DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                {
                    MediaBroPrintfBuf = pBro->LongFileName;
                    MediaBroWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 30);

                    SendMsg(MSG_MEDIABRO_DISPFLAG_SCROLL_FILENAME);
                }
                else
                {
                    ClearMsg(MSG_MEDIABRO_DISPFLAG_SCROLL_FILENAME);
                }
            }

            r.y0 += 2;
            r.y1 =  r.y0+CH_CHAR_XSIZE_12-1;

            LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);
            pBro = pBro->pNext;

            //if( (1 == check_album()) && (i >= MusicBroUnknownCnt) && (i >= MusicBroAllfileCnt) )
            //{
            //  MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep]++;
            //  MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep+1] = 0;//1 st song
            //  temp_iD = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirThumbTreeInfo.MusicDirDeep-1+1],
            //              MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep-1+1] + MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep-1+1], FIND_SUM_SORTSTART);
            //  MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1] = temp_iD;
            //}
        }

#ifdef THUMB_DEC_INCLUDE
        gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+1]=0x00;
        gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+2]=0x00;
        gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+3]=0x00;
        gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+4]=0x00;
#endif

        SendMsg(MSG_MEDIA_SCROLL_PAINT);

    }

    if (GetMsg(MSG_MEDIABRO_FRESH_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);

#ifdef THUMB_DEC_INCLUDE
        if(!gIsImproveFreq)
        {
            gIsImproveFreq = TRUE;
            gTickCounter = SysTickCounter;
            FREQ_EnterModule(FREQ_JPG); //��������ͼ���뷴����Ƶ����Ƶ
        }
#endif

        for (i = 0; i < BROWSER_SCREEN_PER_LINE; i++)
        {
            if ((i == MusicDirTreeInfo.KeyCounter) || (i == MusicDirTreeInfo.PreCounter))
            {
                r.x0 = 20;
                r.y0 = i*17+3; // //4 + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
                r.x1 = r.x0 + PicInfo.xSize;
                r.y1 = r.y0+17;//r.y0 + CH_CHAR_XSIZE_12 - 1;

#ifdef THUMB_DEC_INCLUDE
                if (i == MusicDirTreeInfo.PreCounter)
                {
                    if(0 == check_album())
                    {
                        DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+i*17,17);
                    }
                    else
                    {
                        DisplayPicture_part_xoffset(IMG_ID_BROWSER_BACKGROUND,0,17,128-17,0,3+i*17,17);
                    }
                }
                if(0 == check_album())
                {
                    DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType, PicInfo1.x, PicInfo.y+5+ 17 * i);
                }
#else
                if (i == MusicDirTreeInfo.PreCounter)
                {
                    DisplayPicture_part(IMG_ID_BROWSER_BACKGROUND,0,0,3+i*17,17);
                }

                DispPictureWithIDNumAndXY(IMG_ID_BROWSER_FILETYPE_FOLDER + pBro->FileType,
                                          PicInfo1.x, PicInfo.y+5+ 17 * i);
#endif

                if (i == MusicDirTreeInfo.KeyCounter)
                {
                    DispPictureWithIDNumAndXY(IMG_ID_SEL_ICON,19,r.y0);
                    if (LCD_GetStringSize(pBro->LongFileName) > PicInfo.xSize)
                    {
                        MediaBroPrintfBuf = pBro->LongFileName;
                        MediaBroWinScrollInit(&r, IMG_ID_SEL_ICON, pBro->LongFileName, 30);
                        SendMsg(MSG_MEDIABRO_DISPFLAG_SCROLL_FILENAME);
                    }
                    else
                    {
                        ClearMsg(MSG_MEDIABRO_DISPFLAG_SCROLL_FILENAME);
                    }
                }

                r.y0 += 2;
                r.y1 = r.y0+CH_CHAR_XSIZE_12-1;
                LCD_DispStringInRect(&r, &r, pBro->LongFileName, LCD_TEXTALIGN_LEFT);

            }
            pBro = pBro->pNext;
        }

        SendMsg(MSG_MEDIA_SCROLL_PAINT);
    }

#ifdef THUMB_DEC_INCLUDE
    if (TRUE == GetMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL))
    {
        if(!gIsImproveFreq)
        {
            //DEBUG("FREQ_EnterModule(FREQ_JPG)");
            gIsImproveFreq = TRUE;
            gTickCounter = SysTickCounter;
            FREQ_EnterModule(FREQ_JPG); //��������ͼ���뷴����Ƶ����Ƶ
        }

        if(0 == gThumbDecodeMode)
        {
            TotalItem = MusicDirTreeInfo.DispTotalItem;
            #ifdef _RECORD_
            if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
            {
                TotalItem++;
            }
            #endif

//  0~9     ///////////////////////////////////////////////
            fno_temp = MusicDirTreeInfo.MusicDirTotalItem;
            fno_temp = fno_temp - MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
            fno_temp = fno_temp + MusicDirTreeInfo.KeyCounter;
            fno_temp = fno_temp + MusicBroUnknownCnt + MusicBroAllfileCnt;
            if(MAX_ITEM_NUM_MEDIABRO_DISP+1 <= fno_temp)
            {
                TotalItem++;
            }

            /* File Set */
            for(i = 0; (i < (MAX_ITEM_NUM_MEDIABRO_DISP+1)) && (i < TotalItem ); i++)
            {

                if(gThumbChkBuf[i] == 0x00)
                {
                    if((StartItem + i) >= (MusicBroUnknownCnt+MusicBroAllfileCnt))
                    {
                        MusicDirThumbTreeInfo.MusicDirTotalItem = MusicDirTreeInfo.MusicDirTotalItem;
                        MusicDirThumbTreeInfo.MusicDirDeep = MusicDirTreeInfo.MusicDirDeep;
                        MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep] =
                                MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter + i/*gThumbNowPicPos*/ - MusicBroAllfileCnt/*gap*/;
                        MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep] = MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                        MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep+1] = 0;//1 st song

                        temp_iD = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirThumbTreeInfo.MusicDirDeep-1+1],
                                    MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep-1+1] + MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep-1+1], FIND_SUM_SORTSTART);

                        MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1] = temp_iD;

                        GetSavedMusicDir(&stFindData,
                                             SortInfoAddr.ulFileFullInfoSectorAddr,
                                             SortInfoAddr.ulFileSortInfoSectorAddr,
                                             MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1]);

                        gThumbHandle = (FILE*)FileOpen(NULL, stFindData.Clus, stFindData.Index - 1, FS_FAT, FileOpenStringR);

                        if ((int)gThumbHandle == NOT_OPEN_FILE)
                        {
                            DEBUG("=====FileOpen error=====\n");
                            gThumbChkBuf[i/*gThumbNowPicPos*/] = 0xee;
                        }
                        else
                        {
                            if(0 == ThumbJpgFileSet(gThumbHandle))
                            {
                                /* No Thumbnail */
                                gThumbChkBuf[i/*gThumbNowPicPos*/] = 0xee;
                                FileClose((HANDLE)gThumbHandle);
                                gThumbHandle = (FILE*)-1;
                            }
                            else
                            {
                                gThumbNowPicPos = i;
                                gThumbDecodeMode = 1;
                                gThumbNowDecodeLine = 0;
                            }
                        }
                        SendMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL);
                        break;
                    }
                    else
                    {
                        gThumbChkBuf[i] = 0xee;
                    }
                }
            }

//  -1      ///////////////////////////////////////////////
            fno_temp = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter;
            fno_temp = fno_temp - MusicBroAllfileCnt/*gap*/;
            fno_temp--; //pre file

            for(i = 0; i < MAX_FILENAME_LEN; i++)
            {
                tempfilename[i] = 0;
            }
            uiListNO = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]-1 - MusicDirTreeInfo.KeyCounter;
            uiListNO += MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
            GetMediaItemInfo(tempfilename, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep,0);

            if (tempfilename[0]==0) // (ucFileName[0]==0x20)
            {
                for (j=0;j<MAX_FILENAME_LEN;j++)
                {
                    if (tempfilename[j]!=0)
                    {
                        break;
                    }
                }

                if (j==MAX_FILENAME_LEN)
                {
                    fno_temp--;
                }
            }

            if( (0 ==CheckMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL)) &&
                (0 == gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+3]) &&
                (0 <= fno_temp)
            ){
                //DEBUG("-1 %d",fno_temp);

                MusicDirThumbTreeInfo.MusicDirTotalItem = MusicDirTreeInfo.MusicDirTotalItem;
                MusicDirThumbTreeInfo.MusicDirDeep = MusicDirTreeInfo.MusicDirDeep;
                MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep] =
                        MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter - MusicBroAllfileCnt/*gap*/ - 1;

                MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep] = MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep+1] = 0;//1 st song

                temp_iD = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirThumbTreeInfo.MusicDirDeep-1+1],
                        MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep-1+1] + MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep-1+1], FIND_SUM_SORTSTART);

                MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1] = temp_iD;

                GetSavedMusicDir(&stFindData,
                                 SortInfoAddr.ulFileFullInfoSectorAddr,
                                 SortInfoAddr.ulFileSortInfoSectorAddr,
                                 MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1]);

                gThumbHandle = (FILE*)FileOpen(NULL, stFindData.Clus, stFindData.Index - 1, FS_FAT, FileOpenStringR);

                if ((int)gThumbHandle == NOT_OPEN_FILE)
                {
                    DEBUG("=====FileOpen error=====\n");
                    gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+3] = 0xee;
                }
                else
                {
                    if(0 == ThumbJpgFileSet(gThumbHandle))
                    {
                        /* No Thumbnail */
                        gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+3] = 0xee;
                        FileClose((HANDLE)gThumbHandle);
                        gThumbHandle = (FILE*)-1;
                    }
                    else
                    {
                        gThumbNowPicPos = MAX_ITEM_NUM_MEDIABRO_DISP-1+3;
                        gThumbDecodeMode = 1;
                        gThumbNowDecodeLine = 0;
                    }
                }
                SendMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL);
            }
//  10      ///////////////////////////////////////////////

            fno_temp = MusicDirTreeInfo.MusicDirTotalItem;
            fno_temp = fno_temp - MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
            fno_temp = fno_temp + MusicDirTreeInfo.KeyCounter;
            fno_temp = fno_temp + MusicBroUnknownCnt + MusicBroAllfileCnt;
            //fno_temp = fno_temp + gap;

            if( (0 ==CheckMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL)) &&
                (0 == gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+2]) &&
                (MAX_ITEM_NUM_MEDIABRO_DISP+2 <= fno_temp)
            ){
                //DEBUG("+2 %d",fno_temp);

                MusicDirThumbTreeInfo.MusicDirTotalItem = MusicDirTreeInfo.MusicDirTotalItem;
                MusicDirThumbTreeInfo.MusicDirDeep = MusicDirTreeInfo.MusicDirDeep;
                MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep] =
                        MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter + 9/*gThumbNowPicPos*/ - MusicBroAllfileCnt/*gap*/;
                MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep] = MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep+1] = 0;//1 st song

                temp_iD = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirThumbTreeInfo.MusicDirDeep-1+1],
                            MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep-1+1] + MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep-1+1], FIND_SUM_SORTSTART);

                MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1] = temp_iD;

                GetSavedMusicDir(&stFindData,
                                     SortInfoAddr.ulFileFullInfoSectorAddr,
                                     SortInfoAddr.ulFileSortInfoSectorAddr,
                                     MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1]);

                gThumbHandle = (FILE*)FileOpen(NULL, stFindData.Clus, stFindData.Index - 1, FS_FAT, FileOpenStringR);

                if ((int)gThumbHandle == NOT_OPEN_FILE)
                {
                    DEBUG("=====FileOpen error=====\n");
                    gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+2] = 0xee;
                }
                else
                {
                    if(0 == ThumbJpgFileSet(gThumbHandle))
                    {
                        /* No Thumbnail */
                        gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+2] = 0xee;
                        FileClose((HANDLE)gThumbHandle);
                        gThumbHandle = (FILE*)-1;
                    }
                    else
                    {
                        gThumbNowPicPos = MAX_ITEM_NUM_MEDIABRO_DISP-1+2;
                        gThumbDecodeMode = 1;
                        gThumbNowDecodeLine = 0;
                    }
                }
                SendMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL);
            }

//  -2      ///////////////////////////////////////////////
            fno_temp = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter;
            fno_temp = fno_temp - MusicBroAllfileCnt/*gap*/;
            fno_temp--; //pre file
            fno_temp--; //prepre file

            for(i = 0; i < MAX_FILENAME_LEN; i++)
            {
                tempfilename[i] = 0;
            }
            uiListNO = MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] -2 - MusicDirTreeInfo.KeyCounter;
            uiListNO += MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
            GetMediaItemInfo(tempfilename, SortInfoAddr, uiListNO, MAX_FILENAME_LEN, MusicDirTreeInfo.MusicDirDeep,0);

            if (tempfilename[0]==0) // (ucFileName[0]==0x20)
            {
                for (j=0;j<MAX_FILENAME_LEN;j++)
                {
                    if (tempfilename[j]!=0)
                    {
                        break;
                    }
                }

                if (j==MAX_FILENAME_LEN)
                {
                    fno_temp--;
                }
            }

            if( (0 ==CheckMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL)) &&
                (0 == gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+4]) &&
                (0 <= fno_temp)
            ){
                //DEBUG("-2 %d",fno_temp);

                MusicDirThumbTreeInfo.MusicDirTotalItem = MusicDirTreeInfo.MusicDirTotalItem;
                MusicDirThumbTreeInfo.MusicDirDeep = MusicDirTreeInfo.MusicDirDeep;
                MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep] =
                        MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] - MusicDirTreeInfo.KeyCounter - MusicBroAllfileCnt/*gap*/ - 2;

                MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep] = MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep];
                MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep+1] = 0;//1 st song

                temp_iD = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirThumbTreeInfo.MusicDirDeep-1+1],
                        MusicDirThumbTreeInfo.CurId[MusicDirThumbTreeInfo.MusicDirDeep-1+1] + MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep-1+1], FIND_SUM_SORTSTART);

                MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1] = temp_iD;

                GetSavedMusicDir(&stFindData,
                                 SortInfoAddr.ulFileFullInfoSectorAddr,
                                 SortInfoAddr.ulFileSortInfoSectorAddr,
                                 MusicDirThumbTreeInfo.MusicDirBaseSortId[MusicDirThumbTreeInfo.MusicDirDeep+1]);

                gThumbHandle = (FILE*)FileOpen(NULL, stFindData.Clus, stFindData.Index - 1, FS_FAT, FileOpenStringR);

                if ((int)gThumbHandle == NOT_OPEN_FILE)
                {
                    DEBUG("=====FileOpen error=====\n");
                    gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+4] = 0xee;
                }
                else
                {
                    if(0 == ThumbJpgFileSet(gThumbHandle))
                    {
                        /* No Thumbnail */
                        gThumbChkBuf[MAX_ITEM_NUM_MEDIABRO_DISP-1+4] = 0xee;
                        FileClose((HANDLE)gThumbHandle);
                        gThumbHandle = (FILE*)-1;
                    }
                    else
                    {
                        gThumbNowPicPos = MAX_ITEM_NUM_MEDIABRO_DISP-1+4;
                        gThumbDecodeMode = 1;
                        gThumbNowDecodeLine = 0;
                    }
                }
                SendMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL);
            }

//ALL CHANGE/////////////////////////////////////////////////////////////////

        }
        else
        {
            /* Decode */
            gTickCounter = SysTickCounter;

            ret = ThumbJpgDecOnly(&gThumbDecodeBuf[gThumbNowDecodeLine*12]);
            gThumbNowDecodeLine++;
            if((ret == 0) || (gThumbNowDecodeLine >= 12))
            {
                /* Decode End */
                FileClose((HANDLE)gThumbHandle);
                gThumbHandle = (FILE*)-1;

                //DEBUG("Decode End\n");
                GetPictureInfoWithIDNum(IMG_ID_SEL_ICON, &PicInfo);
                GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);
                ImageLeft = PicInfo1.x;
                ImageTop = 5 + 17 * gThumbNowPicPos;

                if(gThumbNowPicPos < MAX_ITEM_NUM_MEDIABRO_DISP)
                {
                    ThumbJpgDrawBmp(gThumbDecodeBuf);
                }
                else
                {
                    // buf copy
                    if(gThumbNowPicPos == MAX_ITEM_NUM_MEDIABRO_DISP-1+1){
                        memcpy(&gThumbDecP1Buf[0], gThumbDecodeBuf, 12*12*2*2/*byte*/);
                    }else if(gThumbNowPicPos == MAX_ITEM_NUM_MEDIABRO_DISP-1+2){
                        memcpy(&gThumbDecP2Buf[0], gThumbDecodeBuf, 12*12*2*2/*byte*/);
                    }else if(gThumbNowPicPos == MAX_ITEM_NUM_MEDIABRO_DISP-1+3){
                        memcpy(&gThumbDecM1Buf[0], gThumbDecodeBuf, 12*12*2*2/*byte*/);
                    }else{
                        memcpy(&gThumbDecM2Buf[0], gThumbDecodeBuf, 12*12*2*2/*byte*/);
                    }
                }

                gThumbChkBuf[gThumbNowPicPos] = 0xFF;
                gThumbDecodeMode = 0;
                gThumbNowDecodeLine = 0;
            }
            else
            {
                /* Decode Continue */
                gThumbDecodeMode = 1;
            }
            SendMsg(MSG_MEDIABRO_UPDATE_THUMBNAIL);
        }
    }
#endif

    if (CheckMsg(MSG_MEDIABRO_DISPFLAG_SCROLL_FILENAME))//an 4.21
    {
        TextMode = LCD_SetTextMode(LCD_DRAWMODE_TRANS);
        ScrollStringCommon(MediaBroPrintfBuf);
        LCD_SetTextMode(TextMode);
    }

    if (GetMsg(MSG_MEDIA_SCROLL_PAINT))
    {
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_SCOLL_BLOCK, &PicInfo1);

#ifdef _RECORD_
        if ((gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE) || (gMusicTypeSelID == MUSIC_TYPE_SEL_RECORDFILE_DEL))
        {
            TotalItem = (MusicDirTreeInfo.MusicDirTotalItem + 1);
        }
        else
#endif
        {
            TotalItem = MusicDirTreeInfo.MusicDirTotalItem;
        }

        if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] > 0)
        {
            i = (PicInfo.ySize - PicInfo1.ySize) * MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]  / ( TotalItem - 1); //+picInfo1.ySize/2;
        }
        else
        {
            i = 0;
        }

        if ((i + PicInfo1.ySize) > PicInfo.ySize) //an 4.22
        {
            i = PicInfo.ySize - PicInfo1.ySize;
        }

        DispPictureWithIDNumAndXY(IMG_ID_BROWSER_SCOLL, 122, 0);
        DispPictureWithIDNumAndXYoffset(IMG_ID_BROWSER_SCOLL_BLOCK, 0 + 123, i); //scroll small icon
    }

    LCD_SetTextMode(TextMode);
    LCD_SetColor(TempColor);
    LCD_SetCharSize(TempCharSize);
#ifdef THUMB_DEC_INCLUDE

    if (SysTickCounter - gTickCounter > 500) //����5sû������ͼ�����������Ƶ
    {
        if (gIsImproveFreq)
        {
            gIsImproveFreq = FALSE;
            FREQ_ExitModule(FREQ_JPG);
        }
    }

#endif
}

#ifdef BROSUB
/*
--------------------------------------------------------------------------------
  Function name : void MediaBroSubInit(void)

  Author        : anzhiguo
  Description   : media library sub window entry

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROSUBWIN_CODE_
void MediaBroSubInit(void *pArg)
{
    UINT16 i, j;
    KeyReset();
    MediaSubData.DispTotalItem = 2;
    MediaSubData.KeyCounter = 0;
    MediaSubData.PreCounter = 0;
    SendMsg(MSG_MEDIABROSUB_DISPLAY_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : void MediaBroSubDeInit(void)

  Author        : anzhiguo
  Description   :kill meida library sub window  entry

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROSUBWIN_CODE_
void MediaBroSubDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : void   MediaBroSubService(void)


  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROSUBWIN_CODE_

UINT32 MediaBroSubService(void)
{
    return RETURN_OK;
}
/*
--------------------------------------------------------------------------------
  Function name : UINT32 MediaBroSubKey(void)

  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROSUBWIN_CODE_
UINT32 MediaBroSubKey(void)
{
    uint32 RetVal;
    UINT32 KeyVal;
    UINT16 TempFileNum;
    UINT16 uiSaveItemNum , uiBaseId;
    TASK_ARG TaskArg;
    RetVal = RETURN_OK;
    KeyVal =  GetKeyVal();

    //5 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (KeyVal)
        {
            case KEY_VAL_FFW_DOWN:
            case KEY_VAL_FFW_PRESS:
                KeyVal = KEY_VAL_UP_DOWN;
                break;

            case KEY_VAL_FFD_DOWN:
            case KEY_VAL_FFD_PRESS:
                KeyVal = KEY_VAL_DOWN_DOWN;
                break;

            default:
                break;
        }
    }

    //6 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
    }

    switch (KeyVal)
    {
        //--------------------- MENU KEY ---------------------------
        case KEY_VAL_MENU_PRESS_START:
        {
            WinDestroy(&MediaBroSubWin);
            SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
            break;
        }

        case KEY_VAL_MENU_SHORT_UP:
        {
            if ( MediaSubData.KeyCounter == 0)
            {
                switch (gMusicTypeSelID)
                {
                    case SORT_TYPE_SEL_FILENAME:
                    case SORT_TYPE_SEL_ID3TITLE:

                        SaveGroupFavoSong(SortInfoAddr.ulSortSubInfoSectorAddr, SortInfoAddr.ulFileSortInfoSectorAddr, MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep], 1);
                        break;

                    case SORT_TYPE_SEL_ID3SINGER:
                    case SORT_TYPE_SEL_ID3ALBUM:
                    case SORT_TYPE_SEL_GENRE:
                        if (MusicDirTreeInfo.MusicDirDeep == 0)
                        {
                            uiBaseId = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr, MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep], FIND_SUM_SORTSTART);
                            uiSaveItemNum = GetSummaryInfo(SortInfoAddr.ulSortSubInfoSectorAddr, MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep], FIND_SUM_ITEMNUM);
                        }
                        else
                        {
                            uiBaseId = MusicDirTreeInfo.MusicDirBaseSortId[MusicDirTreeInfo.MusicDirDeep] + MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep];
                            uiSaveItemNum = 1;
                        }

                        SaveGroupFavoSong(SortInfoAddr.ulSortSubInfoSectorAddr, SortInfoAddr.ulFileSortInfoSectorAddr, uiBaseId, uiSaveItemNum);
                        break;

                    default:
                        break;
                }

                WinDestroy(&MediaBroWin);
                SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
            }
            else if ( MediaSubData.KeyCounter == 1)
            {
                WinDestroy(&MediaBroWin);
                SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
            }

            break;
        }

        //--------------------- UP KEY ---------------------------
        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_DOWN:
        {
            MediaSubData.PreCounter = MediaSubData.KeyCounter;

            if (MediaSubData.KeyCounter == 0)
            {
                MediaSubData.KeyCounter = 1;
            }
            else
            {
                MediaSubData.KeyCounter = 0;
            }

            SendMsg(MSG_MEDIABROSUB_FRESH_ITEM);
            break;
        }

        //------------------- DOWN KEY ---------------------------
        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_DOWN:
        {
            MediaSubData.PreCounter = MediaSubData.KeyCounter;

            if (MediaSubData.KeyCounter == 0)
            {
                MediaSubData.KeyCounter = 1;
            }
            else
            {
                MediaSubData.KeyCounter = 0;
            }

            SendMsg(MSG_MEDIABROSUB_FRESH_ITEM);
            break;
        }

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&MediaBroSubWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&MediaBroSubWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void   MediaBroSubDisplay(void)


  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROSUBWIN_CODE_
void MediaBroSubDisplay(void)
{
    MUSICBRO_STRUCT  *pBro;
    MUSIC_DIR_TREE_STRUCT *pMusicDirInfo;
    LCD_RECT      r;
    UINT32 MusicBroNFAddr, PicBroNFAddr;
    UINT16  i, j, k, m, n;
    UINT16  color_temp;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;
    UINT16  TotalItem, TempColor, TempCharSize, TextMode;
    TempColor = LCD_GetColor();
    LCD_SetColor(COLOR_BLACK);
    TempCharSize = LCD_SetCharSize(FONT_12x12);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_MEDIABROSUB_DISPLAY_ALL)))
    {
        //����������ʾ��Ϣ
        SendMsg(MSG_MEDIABROSUB_ALL_ITEM);
    }

    if (GetMsg(MSG_MEDIABROSUB_ALL_ITEM))
    {
        DispPictureWithIDNum(IMG_ID_MEDIABRO_BACKGROUND);
        GetPictureInfoWithIDNum(IMG_ID_MEDIABRO_NOSEL_BACK, &PicInfo);
        GetPictureInfoWithIDNum(IMG_ID_BROWSER_FILETYPE_FOLDER, &PicInfo1);
        TotalItem = MediaSubData.DispTotalItem;

        for (i = 0; ((i < MAX_ITEM_NUM_MEDIABRO_DISP) && (i < TotalItem )); i++)
        {
            r.x0 = PicInfo.x;
            r.y0 = PicInfo.y + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0 + CH_CHAR_XSIZE_12 - 1;
            LCD_SetTextMode(LCD_DRAWMODE_TRANS);

            if (i == MediaSubData.KeyCounter)
            {
                DispPictureWithIDNumAndXYoffset(IMG_ID_MEDIABRO_SEL_BACK, 0, i * PicInfo.ySize);
            }
            else
            {
                DispPictureWithIDNumAndXYoffset(IMG_ID_MEDIABRO_NOSEL_BACK, 0, i * PicInfo.ySize);
            }

            DisplayMenuStrWithIDNum(PicInfo.x + 15, (PicInfo.y + (PicInfo.ySize * i) + 2), 77, 12,
                                    LCD_TEXTALIGN_LEFT, AddFileToFavoString[i]);
        }
    }

    if (GetMsg(MSG_MEDIABROSUB_FRESH_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_MEDIABRO_NOSEL_BACK, &PicInfo);

        for (i = 0; i < BROWSER_SCREEN_PER_LINE; i++)
        {
            if ((i == MediaSubData.KeyCounter) || (i == MediaSubData.PreCounter))
            {
                if (i == MediaSubData.PreCounter)
                {
                    DispPictureWithIDNumAndXYoffset(IMG_ID_MEDIABRO_NOSEL_BACK , 0, i * PicInfo.ySize );
                }

                if (i == MediaSubData.KeyCounter)
                {
                    DispPictureWithIDNumAndXYoffset(IMG_ID_MEDIABRO_SEL_BACK , 0, i * PicInfo.ySize );
                }

                r.x0 = PicInfo.x;
                r.y0 = PicInfo.y + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
                r.x1 = r.x0 + PicInfo.xSize;
                r.y1 = r.y0 + CH_CHAR_XSIZE_12 - 1;
                LCD_SetTextMode(LCD_DRAWMODE_TRANS);
                DisplayMenuStrWithIDNum(PicInfo.x + 15, (PicInfo.y + (PicInfo.ySize * i) + 2), 77, 12,
                                        LCD_TEXTALIGN_LEFT, AddFileToFavoString[i]);//SID_ADD_FAVORITE + i);
            }
        }
    }
}
#endif

#ifdef FAVOSUB
/*
--------------------------------------------------------------------------------
  Function name : void MediaFavoSubInit(void *pArg)

  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIAFAVOSUBWIN_CODE_
void MediaFavoSubInit(void *pArg)
{
    UINT16 i, j;
    KeyReset();
    MediaSubData.DispTotalItem = 3;
    MediaSubData.KeyCounter = 0;
    MediaSubData.PreCounter = 0;
    SendMsg(MSG_MEDIAFAVOSUB_DISPLAY_ALL);
}

/*
--------------------------------------------------------------------------------
  Function name : void MediaFavoSubDeInit(void)
  Author        : anzhiguo
  Description

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIAFAVOSUBWIN_CODE_
void MediaFavoSubDeInit(void)
{
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 MediaFavoSubService(void)
  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIAFAVOSUBWIN_CODE_
UINT32 MediaFavoSubService(void)
{
    return RETURN_OK;
}
/*
--------------------------------------------------------------------------------
  Function name : UINT32 MediaFavoSubKey(void)
  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIAFAVOSUBWIN_CODE_
UINT32 MediaFavoSubKey(void)
{
    uint32 RetVal;
    UINT32 KeyVal;
    UINT16 TempFileNum;
    TASK_ARG TaskArg;
    RetVal = RETURN_OK;
    KeyVal =  GetKeyVal();

    //5 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_5)
    {
        switch (KeyVal)
        {
            case KEY_VAL_FFW_DOWN:
            case KEY_VAL_FFW_PRESS:
                KeyVal = KEY_VAL_UP_DOWN;
                break;

            case KEY_VAL_FFD_DOWN:
            case KEY_VAL_FFD_PRESS:
                KeyVal = KEY_VAL_DOWN_DOWN;
                break;

            default:
                break;
        }
    }

    //6 Key function modification
    if (gSysConfig.KeyNum == KEY_NUM_6)
    {
    }

    switch (KeyVal)
    {
        //--------------------- MENU KEY ---------------------------
        case KEY_VAL_MENU_PRESS_START: //long press menu key, and will return main menu
            WinDestroy(&MediaFavoSubWin);
            SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
            break;

        case KEY_VAL_MENU_SHORT_UP://short press menu key, and will enter menu item
#ifdef AUDIOHOLDONPLAY
            if (AudioFileInfo.ucSelPlayType == MUSIC_TYPE_SEL_MYFAVORITE) //if playing file is from my favorite, then must be clear breakpoint
            {
                gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
            }
#endif
            //excute a function acrodding to the postion of cursor
            if ( MediaSubData.KeyCounter == 0) //delete a file from my favorite
            {
                RemoveOneFavoSong(MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]);

                if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] > MusicDirTreeInfo.KeyCounter)
                {
                    MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]--;
                }
                else if (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] == MusicDirTreeInfo.KeyCounter)
                {
                    if ((MusicDirTreeInfo.KeyCounter > 0) && (MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep] == MusicDirTreeInfo.MusicDirTotalItem - 1))
                    {
                        MusicDirTreeInfo.CurId[MusicDirTreeInfo.MusicDirDeep]--;
                        MusicDirTreeInfo.KeyCounter--;
                    }
                }

                if ((TRUE == ThreadCheck(pMainThread, &MusicThread)) && (gSysConfig.MedialibPara.gMyFavoriteFileNum == 0))
                {
                    ThreadDeleteAll(&pMainThread);
                }

                MusicBroMusicDirInit();
                WinDestroy(&MediaFavoSubWin);
                ClearMsg(MSG_MEDIABRO_DISPFLAG_SCROLL_FILENAME);
                SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
            }
            else if ( MediaSubData.KeyCounter == 1) //clear my favorite
            {
                RemoveAllFavoSong();

                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    ThreadDeleteAll(&pMainThread);
                }

                KeyReset();
                MusicDirValueInit();
                MusicBroMusicDirInit();
                WinDestroy(&MediaFavoSubWin);
                ClearMsg(MSG_MEDIABRO_DISPFLAG_SCROLL_FILENAME);
                SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
                SendMsg(MSG_MEDIAFAVO_DELALL);
            }
            else if ( MediaSubData.KeyCounter == 2) //return media Library
            {
                WinDestroy(&MediaFavoSubWin);
                SendMsg(MSG_MEDIABRO_DISPLAY_ALL);
            }

            break;

        //--------------------- UP KEY ---------------------------
        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_DOWN:
            MediaSubData.PreCounter = MediaSubData.KeyCounter;

            if (MediaSubData.KeyCounter == 0)
            {
                MediaSubData.KeyCounter = 2;
            }
            else
            {
                MediaSubData.KeyCounter --;
            }

            SendMsg(MSG_MEDIAFAVOSUB_FRESH_ITEM);
            break;

        //------------------- DOWN KEY ---------------------------
        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_DOWN:
            MediaSubData.PreCounter = MediaSubData.KeyCounter;

            if (MediaSubData.KeyCounter == 2)
            {
                MediaSubData.KeyCounter = 0;
            }
            else
            {
                MediaSubData.KeyCounter ++;
            }

            SendMsg(MSG_MEDIAFAVOSUB_FRESH_ITEM);
            break;

        case KEY_VAL_HOLD_ON:
            TaskArg.Hold.HoldAction = HOLD_STATE_ON;
            WinCreat(&MediaFavoSubWin, &HoldWin, &TaskArg);
            break;

        case KEY_VAL_HOLD_OFF:
            TaskArg.Hold.HoldAction = HOLD_STATE_OFF;
            WinCreat(&MediaFavoSubWin, &HoldWin, &TaskArg);
            break;

        default:
            break;
    }

    return (RetVal);
}

/*
--------------------------------------------------------------------------------
  Function name : void   MediaFavoSubDisplay(void)


  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIAFAVOSUBWIN_CODE_
void MediaFavoSubDisplay(void)
{
    MUSICBRO_STRUCT  *pBro;
    MUSIC_DIR_TREE_STRUCT *pMusicDirInfo;
    LCD_RECT      r;
    UINT32 MusicBroNFAddr, PicBroNFAddr;
    UINT16  i, j, k, m, n;
    UINT16  color_temp;
    PICTURE_INFO_STRUCT  PicInfo;
    PICTURE_INFO_STRUCT  PicInfo1;
    UINT16  TotalItem, TempColor, TempCharSize, TextMode;
    TempCharSize = LCD_SetCharSize(FONT_12x12);

    if (CheckMsg(MSG_NEED_PAINT_ALL) || (GetMsg(MSG_MEDIAFAVOSUB_DISPLAY_ALL)))
    {
        SendMsg(MSG_MEDIAFAVOSUB_ALL_ITEM);
    }

    if (GetMsg(MSG_MEDIAFAVOSUB_ALL_ITEM))
    {
        DispPictureWithIDNum(IMG_ID_MEDIABRO_BACKGROUND);
        GetPictureInfoWithIDNum(IMG_ID_MEDIABRO_NOSEL_BACK, &PicInfo);
        TotalItem = MediaSubData.DispTotalItem;

        for (i = 0; ((i < MAX_ITEM_NUM_MEDIABRO_DISP) && (i < TotalItem )); i++)
        {
            r.x0 = PicInfo.x;
            r.y0 = PicInfo.y + PicInfo.ySize * i + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
            r.x1 = r.x0 + PicInfo.xSize;
            r.y1 = r.y0 + CH_CHAR_XSIZE_12 - 1;

            if (i == MediaSubData.KeyCounter)
            {
                DispPictureWithIDNumAndXYoffset(IMG_ID_MEDIABRO_SEL_BACK, 0, i * PicInfo.ySize);
            }
            else
            {
                DispPictureWithIDNumAndXYoffset(IMG_ID_MEDIABRO_NOSEL_BACK, 0, i * PicInfo.ySize);
            }

            DisplayMenuStrWithIDNum(PicInfo.x + 15, (PicInfo.y + (PicInfo.ySize * i) + 2), 77, 12,
                                    LCD_TEXTALIGN_LEFT, DeletFileFromFavoString[i]); // SID_DELETE_FILE + i);
        }
    }

    if (GetMsg(MSG_MEDIAFAVOSUB_FRESH_ITEM))
    {
        GetPictureInfoWithIDNum(IMG_ID_MEDIABRO_NOSEL_BACK, &PicInfo);

        for (i = 0; i < BROWSER_SCREEN_PER_LINE; i++)
        {
            if ((i == MediaSubData.KeyCounter) || (i == MediaSubData.PreCounter))
            {
                if (i == MediaSubData.PreCounter)
                {
                    DispPictureWithIDNumAndXYoffset(IMG_ID_MEDIABRO_NOSEL_BACK , 0, i * PicInfo.ySize );
                }

                if (i == MediaSubData.KeyCounter)
                {
                    DispPictureWithIDNumAndXYoffset(IMG_ID_MEDIABRO_SEL_BACK , 0, i * PicInfo.ySize );
                }

                r.x0 = PicInfo.x;
                r.y0 = PicInfo.y + (PicInfo.ySize - CH_CHAR_XSIZE_12) / 2;
                r.x1 = r.x0 + PicInfo.xSize;
                r.y1 = r.y0 + CH_CHAR_XSIZE_12 - 1;
                DisplayMenuStrWithIDNum(PicInfo.x + 15, (PicInfo.y + (PicInfo.ySize * i) + 2), 77, 12,
                                        LCD_TEXTALIGN_LEFT, DeletFileFromFavoString[i]);//SID_DELETE_FILE + i);
            }
        }
    }
}

#endif

#endif

