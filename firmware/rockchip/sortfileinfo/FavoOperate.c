/*
********************************************************************************
*          Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All Rights Reserved
*                                    V1.00
* FileName   : FavoOperate.c
* Author     : phc
* Description:
* History    :
*           <author>        <time>     <version>       <desc>
*            phc       07/08/07       1.0            ORG
*
********************************************************************************
*/

#define _IN_FAVORITE_OPERATE_

#include "SysInclude.h"
//#ifdef FAVOSUB

#ifdef _MEDIA_MODULE_
#include "fsinclude.h"

#include "medialibwin.h"
#include "MediaBroWin.h"

#include "AddrSaveMacro.h"
#include "FavoOperateMacro.h"
#include "FavoOperateGloable.h"

#include "SysFindFile.h"

/*
--------------------------------------------------------------------------------
  Function name : void SysCpuInit(void)
  Author        : ZHengYongzhi
  Description   : pll setting.disable interrupt,pwm Etc.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void FavoBlockInit(void)
{
    UINT8 ucBuffer[4];
    //read flag area and floder of favourite block
    MDReadData(DataDiskID, ((MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START2) << 9), 4, ucBuffer);

    //judgu the flag of favourite
    if (ucBuffer[0] == 0xFA && ucBuffer[1] == 0x00)
    {
        gFavoBlockSectorAddr = MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START2;
        gSysConfig.MedialibPara.gMyFavoriteFileNum = (UINT16)ucBuffer[2] + (((UINT16)ucBuffer[3]) << 8);
    }
    else
    {
        MDReadData(DataDiskID, ((MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START1) << 9), 4, ucBuffer);

        if (ucBuffer[0] == 0xFA && ucBuffer[1] == 0x00)
        {
            gFavoBlockSectorAddr = MediaInfoAddr +  FAVORITE_BLOCK_SECTOR_START1;
            gSysConfig.MedialibPara.gMyFavoriteFileNum = (UINT16)ucBuffer[2] + (((UINT16)ucBuffer[3]) << 8);
        }
        else
        {
            gFavoBlockSectorAddr = MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START1;
            gSysConfig.MedialibPara.gMyFavoriteFileNum = 0;
        }
    }
}



/*
--------------------------------------------------------------------------------
  Function name : void MusicBroKeyFun(void)
  Author        : anzhiguo
  Description   : get file FDT information by the path infomation and short file name.

  Input         :

  Return        : RETURN_OK
                  NOT_FIND_FILE

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
#if 0
_ATTR_SYS_CODE_
UINT16 FindFileByClus(FDT *Rt, FIND_DATA FindData)
{
    FIND_DATA FindData;
    UINT32 i;
    UINT16 uiCurDirNum;

    ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);

    while (1) // find file
    {
        //uiFindFileResult = FindNext(&AudioFileInfo.Fdt, &FindDataInfo, AudioFileExtString);
        if (RETURN_OK != FindNext(Rt, &FindData, "*"))
        {
            return NOT_FIND_FILE;
        }

        for (i = 0; i < 11; i++)
        {
            if (ShortFileName[i] != Rt->Name[i])
            {
                break;
            }
        }

        if (i == 11)
        {
            return RETURN_OK;
        }
    }

    return NOT_FIND_FILE;
}
#endif


/*
--------------------------------------------------------------------------------
  Function name : UINT16 GetFavoInfo(FDT *Rt, UINT16 uiFavoID)
  Author        : anzhiguo
  Description   : get fath info

  Input         : uiFavoID -- the serial number in favourite folder.

  Return        : Rt--find the pointer of file fdt info.

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
//sytem code is called in FileModule.c
_ATTR_SYS_CODE_
UINT16 GetFavoInfo(FIND_DATA * pFindData, UINT16 uiFavoID, INT8* longname)
{
    MDReadData(DataDiskID, (UINT32)((gFavoBlockSectorAddr + FAVO_PATH_SECTOR_OFFSET) << 9) + (UINT32)(uiFavoID)*FAVORITE_MUSIC_SAVE_SIZE, 8, (uint8 *)pFindData);
    GetLongFileName(pFindData->Clus, pFindData->Index - 1, FS_FAT, (uint16 *)longname);
    return (RETURN_OK);

    #ifdef _RK_CUE_
    MDReadData(DataDiskID, (UINT32)((gFavoBlockSectorAddr + FAVO_PATH_SECTOR_OFFSET) << 9) + (UINT32)(uiFavoID)*FAVORITE_MUSIC_SAVE_SIZE, 30, (uint8 *)pFindData);
    GetLongFileName(pFindData->Clus, pFindData->Index - 1, FS_FAT, (uint16 *)longname);

    #else
    MDReadData(DataDiskID, (UINT32)((gFavoBlockSectorAddr + FAVO_PATH_SECTOR_OFFSET) << 9) + (UINT32)(uiFavoID)*FAVORITE_MUSIC_SAVE_SIZE, 8, (uint8 *)pFindData);
    GetLongFileName(pFindData->Clus, pFindData->Index - 1, FS_FAT, (uint16 *)longname);

    #endif
}

extern  uint32   FlashSec[3];
_ATTR_SYS_CODE_
void SetFlashSecTag(void)
{
    FlashSec[0] = 0xffffffff;
    FlashSec[1] = 0xffffffff;
    FlashSec[2] = 0xffffffff;
}

#ifdef FAVOSUB
/*
--------------------------------------------------------------------------------
  Function name : void ChangeFavoSaveBlock(void)
  Author        : anzhiguo
  Description   : exchange block address that keeping favourite floder.prevent multiple write,and make bad block.

  Input         :

  Return        :
  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
void ChangeFavoSaveBlock(void)
{
    if (gFavoBlockSectorAddr == (MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START2))
        gFavoBlockSectorAddr = MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START1;
    else
        gFavoBlockSectorAddr = MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START2;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 SaveGroupFavoSong(UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiBaseSortId, UINT16 uiSaveItemNum)
  Author        : anzhiguo
  Description   : save a group songs to favourite

  Input         : ulFullInfoSectorAddr --sec address to save thedetailed file info
                  ulSortSectorAddr --sec address that to save the file sort info
                  uiBaseSortId--
                  uiSaveItemNum -- the number that add to favourite.
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROSUBWIN_CODE_
UINT16 SaveGroupFavoSong(UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiBaseSortId, UINT16 uiSaveItemNum)
{
    UINT16 i, j, k;
    UINT8 *pBuff;
    UINT8 *pBuff1;
    FDT Fdt;
    UINT32 ulPreBlockSectorAddr;
    UINT16 uiPageCount = 0;  //
    UINT16 uiFavoCountIndex = 0;//the file number when page is not been fill full?
    UINT16 uiExistFileCount = 0;

    UINT16 basesortid;
    UINT16 filenum = uiSaveItemNum;
    basesortid = uiBaseSortId;
    ModuleOverlay(MODULE_ID_FAVO_RESET, MODULE_OVERLAY_ALL);//just for utilize the favoreset data memory(quote other global varible group.)
    FREQ_EnterModule(FREQ_MEDIAUPDATA);

    if (gSysConfig.MedialibPara.gMyFavoriteFileNum == 0) //no file in favourite
    {
        ulPreBlockSectorAddr = MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START2;
        gFavoBlockSectorAddr = MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START1;
        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//call write flash code
        memset(gFavoResetBuffer, 0xff, MEDIAINFO_PAGE_SIZE);
        MDWrite(DataDiskID, ulPreBlockSectorAddr, MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE, gFavoResetBuffer); //
    }
    else
    {
        ulPreBlockSectorAddr = gFavoBlockSectorAddr;//backup old favourite address(read out old information in here,then add new file information write to new favourite address.)
        ChangeFavoSaveBlock();//exchang the favourite block address,to ready to write to flash
    }

    memset(gFavoResetBuffer, 0, MEDIAINFO_PAGE_SIZE);
    memset(gFavoResetBuffer1, 0, MEDIAINFO_PAGE_SIZE);
    gFavoResetBuffer[0] = 0xFA; // FAVORITE block flag bit
    gFavoResetBuffer[1] = 0x00;
    gFavoResetBuffer[2] = (gSysConfig.MedialibPara.gMyFavoriteFileNum + filenum) & 0xff;
    gFavoResetBuffer[3] = ((gSysConfig.MedialibPara.gMyFavoriteFileNum + filenum) >> 8) & 0xff;
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//call write flash code
    MDWrite(DataDiskID, gFavoBlockSectorAddr, MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE, gFavoResetBuffer);
    SetFlashSecTag();

    uiPageCount = gSysConfig.MedialibPara.gMyFavoriteFileNum/FAVORITE_NUM_PER_PAGE;

    for(i=0; i<=uiPageCount; i++)//old favourite information write to new favourite address
    {
        //read out old favourite information from old address.
        //FtlReadLogic(((ulPreBlockSectorAddr+FAVO_PATH_SECTOR_OFFSET)<<9+((UINT32)i*MEDIAINFO_PAGE_SIZE)), MEDIAINFO_PAGE_SIZE, gFavoResetBuffer);
        memset(gFavoResetBuffer,0,MEDIAINFO_PAGE_SIZE);
        MDReadData(DataDiskID,((ulPreBlockSectorAddr+FAVO_PATH_SECTOR_OFFSET*(i+1)))<<9, MEDIAINFO_PAGE_SIZE, gFavoResetBuffer);

//        if(i == uiPageCount)
//        {
//            break;
//        }
        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//call write flash code
        //wirte to new favourite address
        MDWrite(DataDiskID, (gFavoBlockSectorAddr+FAVO_PATH_SECTOR_OFFSET+(UINT32)i*MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFavoResetBuffer);
        SetFlashSecTag();
    }

    //read out a favourite information from a not full page.
    //FtlReadLogic(((ulPreBlockSectorAddr+FAVO_PATH_SECTOR_OFFSET)<<9+((UINT32)uiPageCount*MEDIAINFO_PAGE_SIZE)), MEDIAINFO_PAGE_SIZE, gFavoResetBuffer);

    //read out the file number of farourite information in the last page.(0 mean file infomation total )
    uiFavoCountIndex = gSysConfig.MedialibPara.gMyFavoriteFileNum % FAVORITE_NUM_PER_PAGE;

    //get file path information and save it.
    for (i = 0; i < filenum; i++)
    {
        gSysConfig.MedialibPara.gMyFavoriteFileNum ++;
        //get a file number,and save it to favourite,it can be used to judge that whether this file had been in favourite.
        GetSavedMusicDir(&stFindData, SortInfoAddr.ulFileFullInfoSectorAddr, SortInfoAddr.ulFileSortInfoSectorAddr, i + uiBaseSortId);
//        ModuleOverlay(MODULE_ID_FILE_FIND, MODULE_OVERLAY_ALL);//call the code of write flash.
        {
            UINT32 count;

            for (k = 0; k < uiPageCount; k++) //wirte the old favourite information write to new favourite address
            {
                //read out the information in favourite to judge that whether the file had been added
                MDReadData(DataDiskID,(((ulPreBlockSectorAddr+FAVO_PATH_SECTOR_OFFSET) << 9)+((UINT32)k*MEDIAINFO_PAGE_SIZE)), MEDIAINFO_PAGE_SIZE, gFavoResetBuffer1);
                for(count = 0; count < FAVORITE_NUM_PER_PAGE; count++)
                {
                    pBuff = &gFavoResetBuffer1[count*FAVORITE_MUSIC_SAVE_SIZE];
                    if(memcmp((uint8 *)&stFindData,pBuff,8) == 0)// path information is the same,favourite had saved,do not need to save again.
                    {
                        printf("\n the file is exist\n");
                        uiExistFileCount++;
                        gSysConfig.MedialibPara.gMyFavoriteFileNum--;
                        SendMsg(MSG_FILE_IN_FAVO);
                        break;
                    }
                }

                if (CheckMsg(MSG_FILE_IN_FAVO))
                {
                    break;
                }
            }

            if (GetMsg(MSG_FILE_IN_FAVO)) //jump out add next file
            {
                continue;
            }

            memset(gFavoResetBuffer1,0,MEDIAINFO_PAGE_SIZE);
            MDReadData(DataDiskID,(((ulPreBlockSectorAddr+FAVO_PATH_SECTOR_OFFSET)<<9)+((UINT32)uiPageCount*MEDIAINFO_PAGE_SIZE)), MEDIAINFO_PAGE_SIZE, gFavoResetBuffer1)	;

            //get the number of favourite in the last page,(0 means the total size of file information is multiple page size)

            for (count = 0; count < uiFavoCountIndex; count++)
            {
                pBuff = &gFavoResetBuffer1[count * FAVORITE_MUSIC_SAVE_SIZE];

                if (memcmp((uint8 *)&stFindData, pBuff, 8) == 0) // path information is the same,favourite had saved,do not need to save again.
                {
                    uiExistFileCount++;
                    gSysConfig.MedialibPara.gMyFavoriteFileNum--;
                    SendMsg(MSG_FILE_IN_FAVO);
                    break;
                }
            }

            if (GetMsg(MSG_FILE_IN_FAVO))
            {
                continue;
            }
        }
        pBuff1 = &gFavoResetBuffer[uiFavoCountIndex * FAVORITE_MUSIC_SAVE_SIZE];
        //printf("fav.clus = %d, fav.index = %d\n", stFindData.Clus, stFindData.Index);
        //printf("uiFavoCountIndex = %d\n", uiFavoCountIndex);
        memcpy(pBuff1, (uint8 *)(&stFindData), 8);
        uiFavoCountIndex++;

        if (uiFavoCountIndex == FAVORITE_NUM_PER_PAGE)
        {
            ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//call code to write flash.
            MDWrite(DataDiskID, (gFavoBlockSectorAddr + FAVO_PATH_SECTOR_OFFSET) + (UINT32)(uiPageCount * MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE), MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE, gFavoResetBuffer);
            SetFlashSecTag();
            uiFavoCountIndex = 0;
            uiPageCount++;
        }
    }

    if (uiFavoCountIndex)
    {
        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//call code to write flash.
        MDWrite(DataDiskID, (gFavoBlockSectorAddr + FAVO_PATH_SECTOR_OFFSET) + (UINT32)(uiPageCount * MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE), MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE, gFavoResetBuffer);
        //printf("gFavoBlockSectorAddr = %d\n", gFavoBlockSectorAddr);
    }

    if(uiExistFileCount)
    {
        memset(gFavoResetBuffer,0,MEDIAINFO_PAGE_SIZE);
        gFavoResetBuffer[0] = 0xFA; // FAVORITE block flag bit
        gFavoResetBuffer[1] = 0x00;
        gFavoResetBuffer[2] = (gSysConfig.MedialibPara.gMyFavoriteFileNum)&0xff;
        gFavoResetBuffer[3] = ((gSysConfig.MedialibPara.gMyFavoriteFileNum)>>8)&0xff;

        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//call write flash code
        MDWrite(DataDiskID, gFavoBlockSectorAddr, MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFavoResetBuffer);
    }

    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//call code to write flash.
    memset(gFavoResetBuffer, 0xff, MEDIAINFO_PAGE_SIZE);
    MDWrite(DataDiskID, ulPreBlockSectorAddr, MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE, gFavoResetBuffer); //clear th memory in favourite
    SetFlashSecTag();
    FREQ_ExitModule(FREQ_MEDIAUPDATA);
}


/*
--------------------------------------------------------------------------------
  Function name : void RemoveOneFavoSong(UINT16 uiDelID)
  Author        : anzhiguo
  Description   : delete a file info in favourite.

  Input         : uiDelID -- the file serial number that will delete.

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIAFAVOSUBWIN_CODE_
void RemoveOneFavoSong(UINT16 uiDelID)
{
    UINT16 j, i = 0;
    UINT16 uiSaveIndex = 0;
    UINT16 uiPageCount = 0;
    UINT32 ulPreBlockSectorAddr;
    ModuleOverlay(MODULE_ID_FAVO_RESET, MODULE_OVERLAY_ALL);//just for utilize the memort of favoreset(quote the global data group)
    FREQ_EnterModule(FREQ_MEDIAUPDATA);
    ulPreBlockSectorAddr = gFavoBlockSectorAddr;
    ChangeFavoSaveBlock();
    memset(gFavoResetBuffer, 0, MEDIAINFO_PAGE_SIZE);
    gFavoResetBuffer[0] = 0xFA; // the flag of FAVORITE block
    gFavoResetBuffer[1] = 0x00; //
    gFavoResetBuffer[2] = (gSysConfig.MedialibPara.gMyFavoriteFileNum - 1) & 0xff;
    gFavoResetBuffer[3] = ((gSysConfig.MedialibPara.gMyFavoriteFileNum - 1) >> 8) & 0xff;
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//call code to write flash.
    //write the mark page of favourite.
    MDWrite(DataDiskID, gFavoBlockSectorAddr, MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE, gFavoResetBuffer);
    SetFlashSecTag();
    //get out all file numbers

    while (1)
    {
        if (i == uiDelID) //when we want to delete file id,then read its next file information,put it to the position of this  delete file
        {
            ++i;
        }

        if (i == gSysConfig.MedialibPara.gMyFavoriteFileNum)
        {
            break;
        }

        MDReadData(DataDiskID,(((ulPreBlockSectorAddr+FAVO_PATH_SECTOR_OFFSET)<<9)+((UINT32)i*FAVORITE_MUSIC_SAVE_SIZE)), FAVORITE_MUSIC_SAVE_SIZE, &gFavoResetBuffer[uiSaveIndex*FAVORITE_MUSIC_SAVE_SIZE]);

        uiSaveIndex++;

        if (uiSaveIndex == FAVORITE_NUM_PER_PAGE)
        {
            uiSaveIndex = 0;
            MDWrite(DataDiskID, (gFavoBlockSectorAddr+FAVO_PATH_SECTOR_OFFSET)+(UINT32)(uiPageCount*MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFavoResetBuffer);
            SetFlashSecTag();

            uiPageCount++;
        }
        ++i;
    }

    if (uiSaveIndex)
    {
        MDWrite(DataDiskID, (gFavoBlockSectorAddr+FAVO_PATH_SECTOR_OFFSET)+(UINT32)(uiPageCount*MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE), MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFavoResetBuffer);
    }

    --gSysConfig.MedialibPara.gMyFavoriteFileNum;

    memset(gFavoResetBuffer,0xff,MEDIAINFO_PAGE_SIZE);

    MDWrite(DataDiskID, ulPreBlockSectorAddr, MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFavoResetBuffer);
    SetFlashSecTag();

    FREQ_ExitModule(FREQ_MEDIAUPDATA);

    return;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 GetPathInfo(UINT8 *Path)
  Author        : anzhiguo
  Description   : delete whole favourite.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIAFAVOSUBWIN_CODE_
void RemoveAllFavoSong(void)
{
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//call write flash code
    memset(gFavoResetBuffer, 0xff, MEDIAINFO_PAGE_SIZE);
    MDWrite(DataDiskID, MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START1, MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE, gFavoResetBuffer);
    MDWrite(DataDiskID, MediaInfoAddr + FAVORITE_BLOCK_SECTOR_START2, MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE, gFavoResetBuffer);
    gSysConfig.MedialibPara.gMyFavoriteFileNum = 0;

    SetFlashSecTag();
}
#endif
#endif

