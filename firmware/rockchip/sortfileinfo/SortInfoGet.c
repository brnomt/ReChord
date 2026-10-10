/*
********************************************************************************
*          Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All Rights Reserved
*                                    V1.00
* FileName   : SortInfoGet.c
* Author     :
* Description: ý���uiʱ����
* History    :
*           <author>        <time>     <version>       <desc>
*            azg            06/08/09       1.0            ORG
*
********************************************************************************
*/


#include "SysInclude.h"

#include "fsinclude.h"

#include "AddrSaveMacro.h"
#include "FileInfo.h"
#include "MediaBroWin.h" 

#include "SortInfoGetMacro.h"
#include "AudioControl.h"
#ifdef _MEDIA_MODULE_

_ATTR_MEDIABROWIN_BSS_ SORTINFO_STRUCT Subinfo;

/*
--------------------------------------------------------------------------------
  Function name : void GetSavedMusicPath(UINT8 *pPathBuffer, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId)

  Author        : anzhiguo
  Description   : ��ȡ·����Ϣ�����ļ��ţ��Լ����ļ���

  Input         : pPathBuffer -- ��ȡ��·����Ϣ���buf
                  ulFullInfoSectorAddr --- flash�д����ϸ�ļ���Ϣ����ʼ��ַ
                  ulSortSectorAddr --- flash�д�ŷ���������Ϣ����ʼ��ַ
                  uiSortId -- ��Ҫ��ȡ���ļ��ڷ�����Ϣ�е������
                  Filenum -- ��ǰҪ��ȡ���ļ��������ļ��е����(�� 1 ��ʼ)���������ղؼ��������ļ����ж��Ƿ��ظ�����
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_SYS_CODE_
void GetSavedMusicDir(FIND_DATA * pFindData, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId )
{
    UINT32 temp1;
    UINT8 ucBufTemp[2];

    MDReadData(DataDiskID,(ulSortSectorAddr<<9)+((UINT32)uiSortId)*2, 2, ucBufTemp);
    temp1 = (UINT16)ucBufTemp[0]+(((UINT16)ucBufTemp[1])<<8);
    MDReadData(DataDiskID, (ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)DIR_CLUS_SAVE_ADDR_OFFSET, 8 , (uint8 *)pFindData);

#ifdef _RK_CUE_
    MDReadData(DataDiskID, (ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)CUE_START_SAVE_ADDR_OFFSET, 4, (uint8 *)&(pFindData->CueStartTime));
    MDReadData(DataDiskID, (ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)CUE_END_SAVE_ADDR_OFFSET, 4, (uint8 *)&(pFindData->CueEndTime));

    if(pFindData->CueStartTime != 0 || pFindData->CueEndTime != 0)
    {
        pFindData->IsCue = 1;
    }
    else
    {
        pFindData->IsCue = 0;
    }
#endif
}
/*
--------------------------------------------------------------------------------
  Function name :
  void GetSavedMusicFileName(unsigned char *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, unsigned int uiSortId, unsigned int uiCharNum, unsigned int uiCurDeep)
  Author        : anzhiguo
  Description   : ��ȡý�����ʾ��Ŀ����Ϣ(����������������ר���������ļ�����ID3Tilte)

  Input         : AddrInfo -- �ļ���Ϣ��ŵ�ַ�ṹ�����
                : uiSortId -- �ļ���
                : uiCharNum --- Ҫ��ȡ���ֽ���
                : uiCurDeep --- ͨ���ñ���ȷ��ƫ�Ƶ�ַ
                : Flag --- �ж��Ƿ��ȡ�ļ�����ID3Title Ϊ1��ʾ�Ƕ�ȡ�ļ�����ID3Title��Ϊ0��ʾ��ȡ����ID3��Ϣ
  Return        : pFileName Ҫ��ȡ���ļ����ļ�����ָ��

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
void GetMediaItemInfo(UINT16 *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, UINT16 uiSortId, UINT16 uiCharNum, UINT16 uiCurDeep, UINT16 Flag)
{
    UINT16 i;
    UINT16 temp1;
    UINT8   ucBufTemp[8],ucBufTemp1[2];
    UINT32  AddrOffset;
    UINT8 	FileInfoBuf[MEDIA_ID3_SAVE_CHAR_NUM *2];//���ļ�����ID3����ϢҪ������ѡ���ļ��ĳ��ȿ��ռ䲻���������Խ��

   SORTINFO_STRUCT *Subinfo = (SORTINFO_STRUCT *)ucBufTemp;

   if(Flag)
   {
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+(unsigned long)(uiSortId*2), 2, ucBufTemp);
        temp1 = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8); // ��ö�Ӧ���ļ������
   }
   else
   {
        //��ȡһ���������Ϣ�ṹ SORTINFO_STRUCT
        MDReadData(DataDiskID,(AddrInfo.ulSortSubInfoSectorAddr[uiCurDeep]<<9)+(unsigned long)(uiSortId*sizeof(SORTINFO_STRUCT)), sizeof(SORTINFO_STRUCT), ucBufTemp);
        //ͨ����Ϣ�ṹ�е� BaseID ������ȡ��Ҫ���ļ���
        MDReadData(DataDiskID,(AddrInfo.ulFileSortInfoSectorAddr<<9)+Subinfo->BaseID*2, 2, ucBufTemp1);
        temp1 = (ucBufTemp1[0]&0xff)+((ucBufTemp1[1]&0xff)<<8); // ��ö�Ӧ���ļ������
   }

    AddrOffset = (UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + AddrInfo.uiSortInfoAddrOffset[uiCurDeep]; // ͨ��ƫ��λ����ȷ����ȡ���ǳ��ļ�����Ϣ����ID3Title��Ϣ

    MDReadData(DataDiskID,(AddrInfo.ulFileFullInfoSectorAddr<<9)+AddrOffset, uiCharNum*2, FileInfoBuf);

    for(i=0;i<uiCharNum;i++)
    {
        *pFileName++ = (UINT16)FileInfoBuf[2*i]+((UINT16)FileInfoBuf[2*i+1]<<8);
    }
}


/*
--------------------------------------------------------------------------------
  Function name : unsigned int GetSummaryInfo(unsigned long ulSumSectorAddr, unsigned int uiSumId, unsigned int uiFindSumType)
{
  Author        : anzhiguo
  Description   : ��ȡID3������Ϣ����Ŀ(ר���µ��ļ�����������ͬһ���������µ��ļ�����)

  Input         : ulSumSectorAddr ID3��Ϣ����洢��ַ
                : uiSumId  -- ��Ŀid
                : uiFindSumType -- ��ȡ��Ŀ��Ϣ�����ͣ�(��Ŀ�������ǣ���ʼ��Ŀ��ţ����Ǿ����ID3��Ϣ)
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ָ����Ӧ��Ϣ��flash�е�sec��ַ
--------------------------------------------------------------------------------
*/
_ATTR_MEDIABROWIN_CODE_
UINT16 GetSummaryInfo(UINT32 ulSumSectorAddr, UINT16 uiSumId, UINT16 uiFindSumType)
{
    UINT16 uiSumInfo;
    UINT8 	*ucBufTemp;
    UINT32 uiSumTypeOffset;
   ucBufTemp = (UINT8 *)(&Subinfo);

    MDReadData(DataDiskID,(ulSumSectorAddr<<9)+(UINT32)uiSumId*8, 8, ucBufTemp);

    if(uiFindSumType==FIND_SUM_STARTFILEID)//��������Ϣ
    {
        uiSumInfo = Subinfo.BaseID;
    }
    else if(uiFindSumType==FIND_SUM_SORTSTART)//��ʼ�ļ���
    {
        uiSumInfo = Subinfo.ItemBaseID;
    }
    else if(uiFindSumType==FIND_SUM_ITEMNUM)//��Ŀ��Ϣ
    {
        uiSumInfo = Subinfo.ItemNum;
 	 //Rk Aaron.sun
 	 if((SortInfoAddr.uiSortInfoAddrOffset[MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (MusicDirTreeInfo.MusicDirDeep != 0))
 	 {
		uiSumInfo++;  // for All Album
	 }
    }
    else if (uiFindSumType==FIND_SUM_FILENUM)//�ļ�����
    {
        uiSumInfo = Subinfo.FileNum;
    }

    return uiSumInfo;
}

#endif

