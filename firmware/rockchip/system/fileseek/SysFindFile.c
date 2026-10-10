


#define  IN_SYS_FINDFILE

#include "SysInclude.h"
#include "freq_enums.h"   /* FREQ_* enum incl. FREQ_MAX */
#include "fsinclude.h"
#include "SysFindFile.h"
#include "AudioControl.h"
#include "myRandom.h"

#include "MediaBroWin.h"
#include "medialibwin.h"
#include "AddrSaveMacro.h"

extern int  server_ori_seed ;
extern int  rand_first_flag ;

/*
--------------------------------------------------------------------------------
  Function name :  UINT16 SysCheckTotalFileNum(uint8 *pExtStr)
  Author        :  zs
  Description   :  check there is the type file in media
  Input         :  uint8 *pExtStr: file name
  Return        :  temp: file total.
  History       :  <author>         <time>         <version>
                     zs            2009/02/27         Ver1.0
  desc          : org
--------------------------------------------------------------------------------
_ATTR_SYS_FINDFILE_TEXT_
UINT16 SysCheckTotalFileNum(uint8 *pExtStr)
{
    UINT16 temp = 0;

    temp = dglBuildDirInfo(pExtStr);
    return(temp);
}

*/

/*
-----------------------------------------------------------------------------------------------
  Function name :  INT16 SysFindFileInit(SYS_FILE_INFO *pSysFileInfo, UINT16
                       GlobalFileNum,UINT16 FindFileRange,UINT16 FindFileMode,uint8 *pExtStr)
  Author        :  zs
  Description   :  initial the search variables when enter module.
  Input         :  SYS_FILE_INFO *pSysFileInfo : return file information.
                   UINT16        GlobalFileNum : Input: global file number.
                   UINT16        FindFileRange : Input: the range to find file
                   UINT16        FindFileMode  : Input: random mode
                   uint8         *pExtStr      : Input: file type

  Return        :  return value:0
  History       :  <author>         <time>         <version>
                     zs            2009/02/27         Ver1.0
  desc          : org
-------------------------------------------------------------------------------------------------
*/
_ATTR_SYS_FINDFILE_TEXT_
INT16 SysFindFileInit(SYS_FILE_INFO *pSysFileInfo,UINT16 GlobalFileNum,UINT16 FindFileRange,UINT16 PlayMode, uint8 *pExtStr)
{
    UINT16 i,j;
    UINT16 tempFileNum;
    INT16  FindFileResult;
    INT16  RetVal = 0;
    FS_TYPE FsType;

    if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_FOLDER)
    {
        FsType = MUSIC_DB;
    }
#ifdef _RECORD_
    else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_FMFILE)
    {
        FsType = RECORD_DB;
    }
#endif
    else
    {
        FsType = FS_FAT;
    }

    pSysFileInfo->TotalFiles = BuildDirInfo(pExtStr, FsType);

    if (0 == pSysFileInfo->TotalFiles)
    {
        return -1;
    }

    pSysFileInfo->DiskTotalFiles = pSysFileInfo->TotalFiles;
    pSysFileInfo->CurrentFileNum = GlobalFileNum;
    pSysFileInfo->pExtStr        = pExtStr;
    pSysFileInfo->Range          = FindFileRange;
    pSysFileInfo->PlayOrder       = PlayMode;


    if (pSysFileInfo->CurrentFileNum > pSysFileInfo->TotalFiles)
    {
        pSysFileInfo->CurrentFileNum = 1;
    }

    tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum,&pSysFileInfo->FindData, pExtStr, FsType);


    FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pExtStr, FsType);

    for(i = 1; i < tempFileNum; i++)
    {
        FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pExtStr, FsType);
    }

    if (pSysFileInfo->Range == FIND_FILE_RANGE_DIR)
    {
        pSysFileInfo->CurrentFileNum = tempFileNum;
        pSysFileInfo->TotalFiles = GetTotalFiles((pSysFileInfo->FindData).Clus,pExtStr, FsType);
    }

    pSysFileInfo->PlayedFileNum = pSysFileInfo->CurrentFileNum;

    ShuffleNum  = 1;
    ShuffleNextFlag = 1;
    ShufflePrevFlag = 1;
    MusicFileNumBack = AudioFileInfo.CurrentFileNum;

    return 0;
}


/*
--------------------------------------------------------------------------------
  Function name : INT16 SysFindFileExt(SYS_FILE_INFO *pSysFileInfo,INT16 Offset)
  Author        :  zs
  Description   :  search file.
  Input         :  SYS_FILE_INFO *pSysFileInfo:structure to find file.
                   Offset == 0   find current file.
                   Offset > 0    find backward file,the offset is the offset file number relatived to the current file.
                   Offset < 0    find forward file,the offset is the offset file number relatived to the current file.

  Return        :
  History       :  <author>         <time>         <version>
                     zs            2009/02/27         Ver1.0
  desc          : org
--------------------------------------------------------------------------------
*/

_ATTR_SYS_FINDFILE_TEXT_
INT16 SysFindFileExt(SYS_FILE_INFO *pSysFileInfo,INT16 Offset)
{
    UINT16  i, tempFileNum;

    UINT16  uiNeedFindNext = 1;
    INT16   FindFileResult = -1;
    UINT16  seed;

    if(Offset == 0)
    {
        return (RETURN_OK);
    }

    //���ݲ���˳��Ͳ���ģʽ(Ŀ¼����ȫ��)������ǰ�����ļ����ļ���
    if(pSysFileInfo->PlayOrder == AUDIO_RAND) //�������   azg 8.18
    {
//        if(GetMsg(MSG_MEDIA_BREAKPOINT_PLAY)==FALSE)
        {
            FREQ_EnterModule(FREQ_MAX);

            {
                seed =(SysTickCounter % AudioFileInfo.TotalFiles);
                if (AudioFileInfo.TotalFiles == 1)
                {
                    pSysFileInfo->CurrentFileNum = 1;
                }
                else if (AudioFileInfo.TotalFiles == 2)
                {
                    if (AudioFileInfo.CurrentFileNum == 1)
                    {
                        pSysFileInfo->CurrentFileNum = 2;
                    }
                    else
                    {
                        pSysFileInfo->CurrentFileNum = 1;
                    }
                }
                else
                {
                    if (Offset > 0)
                    {
                        if ((ShuffleNextFlag == 0) && (ShufflePrevFlag == 0))
                        {
                            pSysFileInfo->CurrentFileNum = randomGenerator(Offset, seed)+1;
                        }
                        else
                        {
                            if (GetMsg(MSG_SHUFFLE_FIRST_PREV))
                            {
                                pSysFileInfo->CurrentFileNum = PrevLastNumBack;
                                ShuffleNum += Offset;
                            }
                            else
                            {
                                pSysFileInfo->CurrentFileNum = randomGenerator(Offset, seed)+1;
                                ShuffleNum += Offset;
                                //DEBUG("ShuffleNum == %d",ShuffleNum);
                                if ((pSysFileInfo->CurrentFileNum == MusicFileNumBack) && (ShuffleNextFlag == 1))
                                {
                                    pSysFileInfo->CurrentFileNum = randomGenerator(Offset, seed)+1;
                                    ShuffleNum += Offset;
                                    //DEBUG("ShuffleNum == %d",ShuffleNum);
                                    ShuffleNextFlag = 0;
                                    ShufflePrevFlag = 1;
                                }
                                else if ((ShuffleNum >= pSysFileInfo->TotalFiles) && (ShuffleNextFlag == 1))
                                {
                                    tempFileNum = randomGenerator(Offset, seed)+1;
                                    ShuffleNum += Offset;
                                    //DEBUG("ShuffleNum == %d",ShuffleNum);
                                    ShuffleNextFlag = 0;
                                    ShufflePrevFlag = 1;
                                }
                                if (ShuffleNum > pSysFileInfo->TotalFiles + 1)
                                {
                                    ShuffleNextFlag = 0;
                                    ShufflePrevFlag = 0;
                                }
                                //DEBUG("ShuffleNum = %d, CurrentNum = %d, NextFlag = %d", ShuffleNum, pSysFileInfo->CurrentFileNum, ShuffleNextFlag);
                            }
                        }
                    }
                    else
                    {
                        if ((ShuffleNextFlag == 0) && (ShufflePrevFlag == 0))
                        {
                            pSysFileInfo->CurrentFileNum = randomGenerator(Offset, seed)+1;
                        }
                        else
                        {
                            if (ShuffleNum > 2)
                            {
                                tempFileNum = pSysFileInfo->CurrentFileNum;
                                pSysFileInfo->CurrentFileNum = randomGenerator(Offset, seed)+1;
                                ShuffleNum += Offset;
                                //DEBUG("ShuffleNum == %d",ShuffleNum);
                                if (((pSysFileInfo->CurrentFileNum == MusicFileNumBack) && (ShufflePrevFlag == 1)) || (tempFileNum == pSysFileInfo->CurrentFileNum))
                                {
                                    pSysFileInfo->CurrentFileNum = randomGenerator(Offset, seed)+1;
                                    ShuffleNum += Offset;
                                    //DEBUG("ShuffleNum == %d",ShuffleNum);
                                    ShufflePrevFlag = 0;
                                    ShuffleNextFlag = 1;
                                }
                            }
                            else if (ShuffleNum == 2)
                            {
                                SendMsg(MSG_SHUFFLE_FIRST_PREV);
                                PrevLastNumBack = pSysFileInfo->CurrentFileNum;
                                pSysFileInfo->CurrentFileNum = MusicFileNumBack;
                                ShuffleNum += Offset;
                            }
                            else
                            {
                                pSysFileInfo->CurrentFileNum = randomGenerator(Offset, seed)+1;
                                ShufflePrevFlag = 0;
                                ShuffleNextFlag = 0;
                            }
                            //DEBUG("ShuffleNum = %d, CurrentNum = %d, PrevFlag = %d", ShuffleNum, pSysFileInfo->CurrentFileNum, ShufflePrevFlag);
                        }
                    }
                }
            }
            FREQ_ExitModule(FREQ_MAX);
        }
        if(pSysFileInfo->CurrentFileNum > pSysFileInfo->TotalFiles)
        {
            pSysFileInfo->CurrentFileNum = 1;
        }
        pSysFileInfo->PlayedFileNum = pSysFileInfo->CurrentFileNum;
    }
    else //˳�򲥷�
    {
        if(Offset > 0) //��һ��
        {
            pSysFileInfo->CurrentFileNum ++ ;

            if (pSysFileInfo->CurrentFileNum > pSysFileInfo->TotalFiles)
            {
                pSysFileInfo->CurrentFileNum = 1;

                if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_BROWSER)//��Դ��������ʽ�¡���Ҫ���ļ�·��ָ�ص�һ���ļ���·��
                {
                    if(pSysFileInfo->Range == FIND_FILE_RANGE_DIR)//Ŀ¼��ѭ��  //�ļ����ҷ�Χ��һ���ļ���
                    {
                        //�������һ�������ش�ͷ��ʼ��Ŀ¼�еĵ�һ�׸�
                        FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
                        uiNeedFindNext = 0;
                    }
                    else
                    {
                        tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum,&pSysFileInfo->FindData,pSysFileInfo->pExtStr, FS_FAT);

                        FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
                        uiNeedFindNext = FindFileResult;
                    }
                }
                else if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_FOLDER)
                {
                    if(pSysFileInfo->Range == FIND_FILE_RANGE_DIR)//Ŀ¼��ѭ��  //�ļ����ҷ�Χ��һ���ļ���
                    {
                        //�������һ�������ش�ͷ��ʼ��Ŀ¼�еĵ�һ�׸�
                        FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
                        uiNeedFindNext = 0;
                    }
                    else
                    {
                        tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum,&pSysFileInfo->FindData,pSysFileInfo->pExtStr, MUSIC_DB);

                        FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
                        uiNeedFindNext = FindFileResult;
                    }
                }
                #ifdef _RECORD_
                else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_RECORDFILE)//ý����е�¼��Ҳ��Ҫָ�ص�һ���ļ�
                {
                    FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
                    uiNeedFindNext = 0;
                }
                else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_FMFILE)
                {

                    FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
                    uiNeedFindNext = 0;

                }
                #endif
            }

        }
        else if (Offset < 0) //��һ��
        {

            pSysFileInfo->CurrentFileNum -- ;

            if (pSysFileInfo->CurrentFileNum == 0)
            {
                pSysFileInfo->CurrentFileNum = pSysFileInfo->TotalFiles;
            }
        }
        pSysFileInfo->PlayedFileNum = pSysFileInfo->CurrentFileNum;
    }

    #ifdef _MEDIA_MODULE_
    pSysFileInfo->uiCurId[pSysFileInfo->ucCurDeep]= pSysFileInfo->CurrentFileNum - 1;//7.3 azg add
    #endif

    //�����ļ��ſ�ʼ���ļ�����ȡ·���Ͷ��ļ�����Ϣ��Ϊ����Ĵ��ļ���׼��
    if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_BROWSER)
    {
        if((Offset < 0) || (pSysFileInfo->PlayOrder == AUDIO_RAND))
        {

            tempFileNum = pSysFileInfo->CurrentFileNum;

            if (pSysFileInfo->Range != FIND_FILE_RANGE_DIR)//Ŀ¼��ѭ��
            {
                tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
            }

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);

            for (i = 1; i < tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
            }
            uiNeedFindNext = 0;
        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
            FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);

            if (FindFileResult == NOT_FIND_FILE) /* û�з���ָ���ļ�*/
            {
                tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
                FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
            }
        }
    }
    else if(pSysFileInfo->ucSelPlayType == SORT_TYPE_SEL_FOLDER)
    {
        if((Offset < 0) || (pSysFileInfo->PlayOrder == AUDIO_RAND))
        {

            tempFileNum = pSysFileInfo->CurrentFileNum;

            if (pSysFileInfo->Range != FIND_FILE_RANGE_DIR)//Ŀ¼��ѭ��
            {
                tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
            }

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);

            for (i = 1; i < tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
            }
            uiNeedFindNext = 0;
        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
            FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);

            if (FindFileResult == NOT_FIND_FILE) /* û�з���ָ���ļ�*/
            {
                tempFileNum = GetCurFileNum(pSysFileInfo->CurrentFileNum, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
                FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, MUSIC_DB);
            }
        }
    }

    #ifdef _RECORD_
    // ----���¼���ļ������ļ�������ǰһ�׸������
    else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_RECORDFILE)
    {
        #if 0
        if(pSysFileInfo->PlayOrder == AUDIO_RAND || Offset < 0)
        {
            tempFileNum = pSysFileInfo->CurrentFileNum;

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, FS_FAT);
            for (i=1; i<tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, FS_FAT);
            }
            uiNeedFindNext = 0;

        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
          FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
          if (FindFileResult == NOT_FIND_FILE)
          {
             FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, FS_FAT);
          }
        }
        #else
        if((pSysFileInfo->PlayOrder == AUDIO_RAND) || (Offset < 0))
        {
            tempFileNum = pSysFileInfo->CurrentFileNum;

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, RECORD_DB);
            for (i=1; i<tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, RECORD_DB);
            }
            uiNeedFindNext = 0;

        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
          FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
          if (FindFileResult == NOT_FIND_FILE)
          {
             FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
          }
        }
        #endif

    }
    else if(pSysFileInfo->ucSelPlayType == MUSIC_TYPE_SEL_FMFILE)
    {
        if(pSysFileInfo->PlayOrder == AUDIO_RAND || Offset < 0)
        {
            tempFileNum = pSysFileInfo->CurrentFileNum;

            FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, RECORD_DB);
            for (i=1; i<tempFileNum; i++)
            {
                FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData,  pSysFileInfo->pExtStr, RECORD_DB);
            }
            uiNeedFindNext = 0;

        }
        else if ((uiNeedFindNext) && (Offset > 0))
        {
          FindFileResult = FindNextFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
          if (FindFileResult == NOT_FIND_FILE)
          {
             FindFileResult = FindFirstFile(&pSysFileInfo->Fdt, &pSysFileInfo->FindData, pSysFileInfo->pExtStr, RECORD_DB);
          }
        }

    }
    #endif
    else
    {
        switch(pSysFileInfo->ucSelPlayType)
        {
           case SORT_TYPE_SEL_NOW_PLAY:

           #ifdef _MEDIA_MODULE_
           case SORT_TYPE_SEL_FILENAME:
           case SORT_TYPE_SEL_ID3TITLE:
           case SORT_TYPE_SEL_ID3SINGER:
           case SORT_TYPE_SEL_ID3ALBUM:
           case SORT_TYPE_SEL_GENRE:
                i = 0;
                do
                {
                    UINT8 ucBufTemp[2];
                    UINT16 temp1;

                    MDReadData(DataDiskID, (pSysFileInfo->ulSortInfoSectorAddr << 9) + (UINT32) ((pSysFileInfo->CurrentFileNum + pSysFileInfo->uiBaseSortId[pSysFileInfo->ucCurDeep] - 1) * 2), 2, ucBufTemp);

                    temp1 = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8); // ��ȡ�ļ��� (�ڡ�ϸ�ļ���Ϣ���е�λ��)
                    MDReadData(DataDiskID, (pSysFileInfo->ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)DIR_CLUS_SAVE_ADDR_OFFSET, 8, (uint8 *)&(pSysFileInfo->FindData));

#ifdef _RK_CUE_
                    MDReadData(DataDiskID, (pSysFileInfo->ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)CUE_START_SAVE_ADDR_OFFSET, 4, (uint8 *)&(pSysFileInfo->FindData.CueStartTime));
                    MDReadData(DataDiskID, (pSysFileInfo->ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)CUE_END_SAVE_ADDR_OFFSET, 4, (uint8 *)&(pSysFileInfo->FindData.CueEndTime));

                    if(pSysFileInfo->FindData.CueStartTime != 0 || pSysFileInfo->FindData.CueEndTime != 0)
                    {
                        pSysFileInfo->FindData.IsCue = 1;
                    }
                    else
                    {
                        pSysFileInfo->FindData.IsCue = 0;
                    }
#endif

                    FindFileResult = RETURN_OK;

                    if(FindFileResult==RETURN_OK)
                        break; // �ҵ��ɲ��Ÿ���

                    if(i==pSysFileInfo->TotalFiles)
                        break; // ��ѭ�����и�������û�пɲ�������ʱ�˳��ó���

                   i++;
                }while(1);

                break;

            case MUSIC_TYPE_SEL_MYFAVORITE:
                pSysFileInfo->TotalFiles = gSysConfig.MedialibPara.gMyFavoriteFileNum; // ��ֹ�ڲ�������ʱɾ�����ղؼи���

                if(pSysFileInfo->TotalFiles ==0)
                {
                    break;
                }

                i = 0;
                do
                {
                   FindFileResult = GetFavoInfo(&pSysFileInfo->FindData, (pSysFileInfo->CurrentFileNum + pSysFileInfo->uiBaseSortId[pSysFileInfo->ucCurDeep] - 1), MusicLongFileName);

                   if(FindFileResult==RETURN_OK) break; // �ҵ��ɲ��Ÿ���

                   if(pSysFileInfo->TotalFiles == i)  break; // �����ղؼ�����ʱ����������ղؼ�
                   i++;
                }while(1);
                break;

            #endif

            default:
                break;
        }

    }
    return(FindFileResult);
}

/*
--------------------------------------------------------------------------------
  Function name : INT16 SysFindFile(SYS_FILE_INFO *pSysFileInfo,    INT16 Offset)
  Author        :  zs
  Description   :  �����ļ�
  Description   :  search file.
  Input         :  SYS_FILE_INFO *pSysFileInfo:structure to find file.
                   Offset == 0   find current file.
                   Offset > 0    find backward file,the offset is the offset file number relatived to the current file.
                   Offset < 0    find forward file,the offset is the offset file number relatived to the current file.

  Return        :
  History       :  <author>         <time>         <version>
                     zs            2009/02/27         Ver1.0
  desc          :   org
--------------------------------------------------------------------------------
*/
_ATTR_SYS_FINDFILE_TEXT_
INT16 SysFindFile(SYS_FILE_INFO *pSysFileInfo,    INT16 Offset)
{

    UINT16  i, tempFileNum;
    INT16   FindFileResult = -1;

    if(Offset == 0)
        return (RETURN_OK);

    if(Offset<0)
    {
        while(Offset++)
        {
            FindFileResult =  SysFindFileExt(pSysFileInfo,-1);

            if(FindFileResult != RETURN_OK)
                return FindFileResult;
        }
    }
    else
    {
        while(Offset--)
        {
            FindFileResult =  SysFindFileExt(pSysFileInfo,1);

            if(FindFileResult != RETURN_OK)
                return FindFileResult;
        }
    }
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  GetDirPath()
*  Author:          ZHengYongzhi
*  Description:     get file full path
*
*  Input:   pPath           ���� store the start address of file path.
*  Output:  pPath           ���� file path
*  Return:
*  Calls:
*
*  History:     <author>         <time>         <version>
*             ZhengYongzhi     2006/01/01         Ver1.0
*     desc: ORG
********************************************************************************
*/
//_ATTR_SYS_FINDFILE_TEXT_
/*
void GetDirPath(UINT8 *pPath)
{
    UINT16 i,j;

    *pPath++ = 0x55;    //'U';
    *pPath++ = 0x3a;    //':';
    *pPath++ = 0x5c;    //'\\';
    for (i = 1; i <= CurDirDeep; i++)
    {
        for (j = 0; j < 11 ; j++ )
        {
            *pPath++ = SubDirInfo[i].DirName[j];
        }
        *pPath++ = 0x5c;//'\\';
    }
    if (CurDirDeep != 0)
    {
        pPath--;
    }
    *pPath= 0;
}
*/

/*
********************************************************************************
*
*                         End of SysFindFile.c
*
********************************************************************************
*/

