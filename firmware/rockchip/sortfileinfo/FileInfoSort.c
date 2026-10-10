/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   FileInfoSort.C
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-06-03          1.0
*    desc:      ORG.
********************************************************************************
*/
#define  _IN_FILEINFOSORT_
#include "SysInclude.h"
#include "fsinclude.h"
#include "FileInfo.h"

#include "AddrSaveMacro.h"

#include "MDBBuildWin.h"

#ifdef MEDIA_UPDATE

#define   SORT_TYPE_ITEM_NUM  4   /* �����������͸��� */

#define     SORT_FILENAME_LEN               (4)  /* ���ļ����н�ȡ�Ĳ��������ַ��ĸ�����Ŀǰ��16k������ռ������Ա���8*1024���ļ���������Ϣ*/

#define     CHILD_CHAIN_NUM             72 /* ȷ�������������ĸ��� */

//#define   FILE_NAME_TYPE    0       //���˳������AddrSaveMacro.h�ж���Ĵ洢˳��һ��
#define   ID3_TITLE_TYPE    0
#define   ID3_ALBUM_TYPE    1
#define   ID3_ARTIST_TYPE   2
#define   ID3_GENRE_TYPE    3


typedef struct _FILE_INFO_ADD_STRUCT
{

    UINT32  add1;//��һ��������ϢдFlash��ַ
    UINT32  add2;//�ڶ���������ϢдFlash��ַ
    UINT32  add3;//������������ϢдFlash��ַ
    UINT32  add4;//���Ĳ�������ϢдFlash��ַ

}FILE_INFO_ADD_STRUCT; /* �ļ���������Ϣ,define by phc*/

typedef __packed struct _FILE_INFO_INDEX_STRUCT
{

    struct  _FILE_INFO_INDEX_STRUCT   *pNext;
    UINT16  SortFileName[SORT_FILENAME_LEN + 1];

}FILE_INFO_INDEX_STRUCT; /* �ļ���������Ϣ,define by phc*/


_FILE_INFO_SORT_CODE_   UINT16 gChildChainDivValue[CHILD_CHAIN_NUM];//72*2�ֽ�  ��Ÿ������������׸�Ԫ���ĵ�һ����ĸ��Ҫ�Ƚϵ�unicode ͨ���������Ԫ���Ա��Ҹ���Ӧ��������
_FILE_INFO_SORT_BSS_    FILE_INFO_INDEX_STRUCT  *pChildChainHead[CHILD_CHAIN_NUM]; //72*4�ֽ� ���������������ͷ�ڵ��ָ��
_FILE_INFO_SORT_BSS_    FILE_INFO_INDEX_STRUCT  gChildChainHead[CHILD_CHAIN_NUM];  //72*8�ֽ� ���������������ͷ���

_FILE_INFO_SORT_CODE_   FILE_INFO_INDEX_STRUCT  *pChildChainLast;  //72*8�ֽ� ���������������ͷ���
_FILE_INFO_SORT_CODE_   FILE_INFO_INDEX_STRUCT  *pChildChainLast2;  //72*8�ֽ� ���������������ͷ���


/* ע�⡦������������������ַ���롦��������ʵ���ɽ��������һ���� */
_FILE_INFO_SORT_BSS_    FILE_INFO_INDEX_STRUCT  gSortNameBuffer0[SORT_FILENUM_DEFINE];//1536*8�ֽ� �����ȡ���ļ�����Ϣ����פ���ڴ桦����ʱ����, Buffer0
_FILE_INFO_SORT_BSS_   UINT32 gFileSortBufferIndex;
_FILE_INFO_SORT_CODE_  __align(4) UINT8 gFileSortBuffer[MEDIAINFO_PAGE_SIZE];

_FILE_INFO_SORT_CODE_  UINT16 TempBuf1[MEDIA_ID3_SAVE_CHAR_NUM];
_FILE_INFO_SORT_CODE_  UINT16 TempBuf2[MEDIA_ID3_SAVE_CHAR_NUM];
_FILE_INFO_SORT_CODE_  UINT32 gBkMDReadAdrs1;
_FILE_INFO_SORT_CODE_  UINT32 gBkMDReadAdrs2;
_FILE_INFO_SORT_DATA_   UINT32 gwSectorOffset;
_FILE_INFO_SORT_DATA_   UINT32 gwDataBaseAddr;

_FILE_INFO_SORT_CODE_ __align(4) UINT8 SingerBuffer[SORT_FILENUM_DEFINE * sizeof(SORTINFO_STRUCT)]; //16k �ֽ� ���ַ�����Ϣbuf ��Ϊ����������Ϊ2048�������buf��16k
_FILE_INFO_SORT_CODE_ __align(4) UINT8  GerneAblumBuffer[SORT_FILENUM_DEFINE * sizeof(SORTINFO_STRUCT)]; //8k  ����ר������buf ��¼������Ϣ����Ϊר��������Ϊ1024�������buf��8k
_FILE_INFO_SORT_BSS_ __align(4) UINT8   FileIDBuffer[2 * SORT_FILENUM_DEFINE]; //��¼�����ļ��� 2010.05.17 ����28ʱ����ý�������buff�ռ䱻дflash������޸Ľű�������ռ䲻����޸�
_FILE_INFO_SORT_BSS_ __align(4) UINT8   FileIDBuffer1[2 * SORT_FILENUM_DEFINE]; //��¼�����ļ���

_FILE_INFO_SORT_CODE_   UINT32  MediaInfoSaveAdd[4];
_FILE_INFO_SORT_CODE_   UINT32  MediaInfoReadAdd[4];

extern UINT32 SaveSortInfo(UINT16 Deep, UINT8* SubInfoBuffer,UINT16 StartID,UINT8* FileIDBuf,UINT16 Flag);

//extern UINT32 SaveSortInfo(UINT32 uiSaveType, UINT32 ulSortSectorAddr,UINT32 MediaInfoAddress);
/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetPYCode(UINT32 wch)
  Author        : anzhiguo
  Description   : ��ȡ�����ַ��������

  Input         : wch�������ַ���unicodeֵ
  Return        : pinCode:�ַ�ƴ�������

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef  PYSORT_ENABLE
_FILE_INFO_SORT_CODE_
UINT32 GetPYCode(UINT16 wch)
{
    UINT32 pinCode;

    extern UINT32 ReadDataFromIRAM(UINT32 addr);

    pinCode = ReadDataFromIRAM(wch- UNICODE_BEGIN+BASE_PYTABLE_ADDR_IN_IRM); // �ӵ���IRAM��ƴ��������õ����ֵ�ƴ������ֵ
    pinCode += UNICODE_BEGIN;

    return (pinCode);
}

#endif
/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetCmpResult(UINT32 wch)
  Author        : anzhiguo
  Description   : ��ȡ�����ַ��������

  Input         : wch�������ַ���unicodeֵ
  Return        : pinCode:�ַ�ƴ�������

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_

UINT32 GetCmpResult(UINT16 wch)
{
    UINT32 cmpCode;

#ifdef  PYSORT_ENABLE
    if ((wch >= UNICODE_BEGIN) && (wch <= UNICODE_END))
    {
        cmpCode = GetPYCode(wch);
    }
    else
#endif

        if (wch>=97&&wch<=122) // 97 = 'a', 122 = 'z'  Сд��ĸȫ��ת���ɴ�д��ĸ
        {
            cmpCode = wch-32;
        }
        else
        {
            cmpCode = wch;
        }

    return (cmpCode);
}
/*
--------------------------------------------------------------------------------
  Function name : int PinyinCharCmp(UINT32 wch1, UINT32 wch2)
  Author        : anzhiguo
  Description   : ��ƴ�����򡦱Ƚ������ַ������С

  Input         : wch1���ַ�1��unicodeֵ   wch2���ַ�2��unicodeֵ
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

_FILE_INFO_SORT_CODE_
int PinyinCharCmp(UINT16 wch1, UINT16 wch2)
{
    return (GetCmpResult(wch1) - GetCmpResult(wch2));
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 PinyinStrnCmp(UINT16 *str1, UINT16 *str2, UINT32 length)
  Author        : anzhiguo
  Description   : ��ƴ�����򡦱Ƚ������ַ�����С

  Input         : str1���ַ���1   str2���ַ���2
  Return        : 0 str1 < str2
                  1 str1 = str2
                  2 str1 > str2
  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_

UINT32 PinyinStrnCmp(UINT16 *str1, UINT16 *str2/*, UINT32 length*/)
{
    UINT32  i = 0;
    UINT16 * pBkMDReadAdrs1, * pBkMDReadAdrs2;
    UINT16 * longStr1, *longStr2;

    UINT32  BkMDReadAdrs1, BkMDReadAdrs2;


//    if (str1 == NULL)
//    {
//        return 0;
//    }
//
//    if (str2 == NULL)
//    {
//          return 2;
//    }
//
//    for (i = 0; i < length; i++)
//    {
//          if ((str1[i] == NULL) || (str2[i] == NULL) || (/*GetCmpResult*/(str1[i]) != /*GetCmpResult*/(str2[i])))
//          {
//          break;
//          }
//    }
//
//    if(i==length)  return 1;

//      if((str1[i]) > (str2[i]))
//         return 2;
//      else if((str1[i]) == (str2[i]))
//         return 1;  //����
//      else
//         return 0;



    //if(NULL == (str1[0] | str2[0]))   return 1;

    if (str1[0] > str2[0])   return 2;
    if (str1[0] < str2[0])   return 0;
    if (str1[1] > str2[1])   return 2;
    if (str1[1] < str2[1])   return 0;
    if (str1[2] > str2[2])   return 2;
    if (str1[2] < str2[2])   return 0;
    if (str1[3] > str2[3])   return 2;
    if (str1[3] < str2[3])   return 0;
    //if(str1[4] > str2[4]) return 2;
    //if(str1[4] < str2[4]) return 0;
    //if(str1[5] > str2[5]) return 2;
    //if(str1[5] < str2[5]) return 0;

#if 1
    if (str1[3] == NULL)
    {
        //printf("error2\n");
        return 1;
    }
    pBkMDReadAdrs1 = (uint16 *)(&str1[SORT_FILENAME_LEN]);
    pBkMDReadAdrs2 = (uint16 *)(&str2[SORT_FILENAME_LEN]);

    BkMDReadAdrs1 = ((UINT32)(MediaInfoAddr+gwDataBaseAddr)<<9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(*pBkMDReadAdrs1)+gwSectorOffset;
    BkMDReadAdrs2 = ((UINT32)(MediaInfoAddr+gwDataBaseAddr)<<9) + BYTE_NUM_SAVE_PER_FILE * (UINT32)(*pBkMDReadAdrs2)+gwSectorOffset;

#if 1
    if (BkMDReadAdrs1 == gBkMDReadAdrs1)
    {
        longStr1 = TempBuf1;
    }
    else
    {
        MDReadData(DataDiskID,BkMDReadAdrs1, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf1);
        longStr1 = TempBuf1;
        gBkMDReadAdrs1 = BkMDReadAdrs1;
#if 0
        {
            uint32 i;
            for (i = 0; i < 41; i++)
            {
                printf("%c ", TempBuf1[i]);
            }
            printf("\n");
        }
#endif
    }
#else
    MDReadData(DataDiskID,*pBkMDReadAdrs1, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf1);
    longStr1 = TempBuf1;
#if 0
    {
        uint32 i;
        for (i = 0; i < 41; i++)
        {
            printf("%x ", TempBuf1[i]);
        }
        printf("\n");
    }
#endif
#endif

#if 1

    if (BkMDReadAdrs2 == gBkMDReadAdrs2)
    {
        longStr2 = TempBuf2;
    }
    else
    {
        MDReadData(DataDiskID,BkMDReadAdrs2, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf2);
        longStr2 = TempBuf2;
        gBkMDReadAdrs2 = BkMDReadAdrs2;
    }
#else
    MDReadData(DataDiskID,*pBkMDReadAdrs2, MEDIA_ID3_SAVE_CHAR_NUM*2, TempBuf2);
    longStr2 = TempBuf2;
#endif


    for (i = 4; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)
    {
        if (longStr1[i] > longStr2[i])   return 2;
        if (longStr1[i] < longStr2[i])   return 0;
        if (longStr1[i] == NULL)
        {
            //printf("i = %d\n", i);
            return 1;
        }
    }
#endif

    //printf("errror\n");
    return 1;



}
/*
--------------------------------------------------------------------------------
  Function name : UINT8 BrowserListInsertBySort(FILE_INFO_INDEX_STRUCT *head, FILE_INFO_INDEX_STRUCT *pNode)
  Author        : anzhiguo
  Description   : ��ʼ��˫������

  Input         : *head   ��������ͷָ��
                  *pFileSaveTemp   ��������ṹ
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_

UINT8 BrowserListInsertBySort(FILE_INFO_INDEX_STRUCT *head, FILE_INFO_INDEX_STRUCT *pNode)
{
    FILE_INFO_INDEX_STRUCT *p,*q;
    UINT32 i;

    p = head;

    //˳��ɨ������,���½����뵽������Ľ��ǰ
    if ((pChildChainLast->pNext!=NULL) || (PinyinStrnCmp((uint16*)(pChildChainLast->SortFileName), (uint16*)(pNode->SortFileName)/*, SORT_FILENAME_LEN*/)==2))
    {
        if ((pChildChainLast2->pNext!=NULL) && (PinyinStrnCmp((uint16*)(pChildChainLast2->SortFileName), (uint16*)(pNode->SortFileName)/*, SORT_FILENAME_LEN*/)==2))
        {
            //printf(" bk reset : pChildChainLast2 = %p  \n", p );
        }
        else
        {
            p = pChildChainLast2;

        }

        while (p->pNext!=NULL)
        {
            q = p->pNext;
            // 2 ��ʾ q->SortFileName > pNode->SortFileName
            if (PinyinStrnCmp((uint16*)(q->SortFileName), (uint16*)(pNode->SortFileName)/*, SORT_FILENAME_LEN*/)==2) //˳��ɨ������,���½����뵽������Ľ��ǰ
            {
                pNode->pNext = q;
                p->pNext = pNode;
                pChildChainLast2 = pNode;
                return 1;
            }
            p = q;
        }
    }
    else
    {
        p = pChildChainLast;
    }

    if (p->pNext==NULL) // ��������β,˵������Ľ��Ϊ��ǰ�������ֵ,������뵽��β
    {
        pNode->pNext = NULL;
        p->pNext = pNode;
        pChildChainLast = pNode;
        return 1;
    }

    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void SortPageBufferInit(UINT8 *pBuffer)
  Author        : anzhiguo
  Description   :

  Input         : *pBuffer   Ҫ��ʼ����bufָ��

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_

void SortPageBufferInit(UINT8 *pBuffer)
{
    UINT32  i;

    if (pBuffer == GerneAblumBuffer)
    {
        //memset(GerneAblumBuffer,0,8*1024);
        memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));
    }

    if (pBuffer == SingerBuffer)
    {
        //memset(SingerBuffer,0,16*1024);
        memset(SingerBuffer,0,sizeof(SingerBuffer));
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void ChildChainInit(void)
  Author        : anzhiguo
  Description   : ��ʼ��˫������

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         gChildChainDivValue[i] �� pChildChainHead[i] һһ��Ӧ
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
void ChildChainInit(UINT16 Flag)
{
    UINT32 i,j;
    for (i=0;i<CHILD_CHAIN_NUM;i++) // ��ʼ��������ͷ�ڵ�
    {

        //pChildChainHead ��һ��ָ�����顦����ÿ��Ԫ�ؾ�Ϊһ��FILE_INFO_INDEX_STRUCT���͵�ָ�롦
        //gChildChainHead ��һ���ṹ�����顦����ÿ��Ԫ�ؾ�Ϊһ��FILE_INFO_INDEX_STRUCT���͵Ľṹ�������
        pChildChainHead[i] = &gChildChainHead[i];
        pChildChainHead[i]->pNext = NULL;
    }

    //��ε���ʱ���˴�����Ĵ���ֻ���ڽ�������ģ��ʱִ��һ�ξͿ��ԡ���˿��Կ��Ǵ�һ�����������ж��ǲ��ǵ�һ�ν���
    if (Flag)
    {
        gChildChainDivValue[0]=  0x0000; // �����ַ�Ϊ�յķ����ڴ�
        gChildChainDivValue[1]=   0x41; // С��Ӣ����ĸ���ַ����������ּ���������  0x41 A��ĸ��ascall ��

        j = 2;
        for (i='A';i<'Z'+1;i++)
        {
            gChildChainDivValue[j] = i+1; // Ӣ���ַ�,A~Z
            j++;
        }

        gChildChainDivValue[j++] = 0x4DFF; // Ӣ���ַ�������֮���unicode�ַ�

        j = 29;
        for (i=j;i<CHILD_CHAIN_NUM-1;i++)
        {
            gChildChainDivValue[i] = gChildChainDivValue[i-1]+(0x51a5)/(CHILD_CHAIN_NUM-30); // �Ժ��ֲ����ַ�����42�ȷ�
        }

        gChildChainDivValue[CHILD_CHAIN_NUM-1] = 0xffff; // ���ں��ֵ������ַ�ȫ������������
    }
    /*for(i=0;i<CHILD_CHAIN_NUM;i++)
    {
       DisplayTestDecNum((i%6)*50,(i/6)*15,gChildChainDivValue[i]);
    }
    while(1);*/
    /* ����������Ϊ����ƴ������ʱ����ƴ����ĸ�ָ��������� */
    /*gChildChainDivValue[0]=   0x0000;
    gChildChainDivValue[1]=   0x4DFF; //0x0000~0x4DFF; //  0~9,��ĸ,��������
    gChildChainDivValue[2] =  0x4EC3; //0x4E00~0x4EC3; //  A, ߹
    gChildChainDivValue[3] =  0x5235; //0x4EC4~0x5235; //  B����
    gChildChainDivValue[4] =  0x576a; //0x5236~0x576a; //  C, ��
    gChildChainDivValue[5] =  0x5b40; //0x576b~0x5b40; //  D, ��
    gChildChainDivValue[6] =  0x5bf0; //0x5b41~0x5bf0; //  E����
    gChildChainDivValue[7] =  0x5e55; //0x5bf1~0x5e55; //  F, ��
    gChildChainDivValue[8] =  0x6198; //0x5e56~0x6198; //  G, �
    gChildChainDivValue[9] =  0x65b3; //0x6199~0x65b3; //  H, ��,ha
    gChildChainDivValue[10] = 0x6c00; //0x65b4~0x6c00; //  J, آ
    gChildChainDivValue[11] = 0x6e30; //0x6c01~0x6e30; //  K, ��
    gChildChainDivValue[12] = 0x7448; //0x6e31~0x7448; //  L, ��
    gChildChainDivValue[13] = 0x7810; //0x7449~0x7810; //  M, ��
    gChildChainDivValue[14] = 0x7953; //0x7811~0x7953; //  N, ��
    gChildChainDivValue[15] = 0x7978; //0x7954~0x7978; //  O, ��
    gChildChainDivValue[16] = 0x7be2; //0x7979~0x7be2; //  P, ��
    gChildChainDivValue[17] = 0x7fe9; //0x7be3~0x7fe9; //  q, ��,qi
    gChildChainDivValue[18] = 0x8131; //0x7fea~0x8131; //  r, ��,ra
    gChildChainDivValue[19] = 0x8650; //0x8132~0x8650; //  s, ��,sa
    gChildChainDivValue[20] = 0x89d0; //0x8651~0x89d0; //  t, ��
    gChildChainDivValue[21] = 0x8c43; //0x89d1~0x8c43; //  w, ��
    gChildChainDivValue[22] = 0x9169; //0x8c44~0x9169; //  x, Ϧ
    gChildChainDivValue[23] = 0x9904; //0x916a~0x9904; //  y, Ѿ
    gChildChainDivValue[24] = 0x9fa4; //0x9905~0x9fa4; //  z, */
}

/*
--------------------------------------------------------------------------------
  Function name : void GetSortName(UINT32 SectorOffset)
  Author        : anzhiguo
  Description   : ��flash�ж�ȡ�����ļ��ĳ��ļ���������ŵ� gSortNameBuffer0�� gSortNameBuffer1 ��
                  ������ռ��������ġ�ÿ�����Դ�ŵ��ļ�����ϵͳ�����������ļ�����һ��

  Input         : SectorOffset���������ַ����ļ���Ϣ�е�ƫ�Ƶ�ַ

  Return        : Flag �ж϶�ȡ�ļ���˳��,Flag =0����FileIDBuffer�е�˳���ȡ�ļ�

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         �ӱ�����Flash�е��ļ���Ϣ��ȡ��Ҫ������ַ�(�ļ�����ID3��Ϣ)�����Ҵ���gSortNameBuffer0[].SortFileName��
                �������ļ�����Ϣ(��Ҫ������ַ���Ϣ)��ȡ
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
void GetSortName(UINT32 SectorOffset,UINT16 FileNum,UINT16 Flag)
{
    UINT16  i,j;
    UINT16  FileCount;
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];
    FILE_INFO_INDEX_STRUCT *pTemp;
    UINT16  *FileID;
    uint16 * BkMDReadAdrs;
    pTemp = gSortNameBuffer0;
    FileCount = FileNum;


    FileID = (UINT16 *)FileIDBuffer;

    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);

    gwSectorOffset = SectorOffset;
    gwDataBaseAddr = MUSIC_SAVE_INFO_SECTOR_START;

    for (i=0;i<FileCount;i++)
    {
        //��flash�ж�ȡ��ȡ�ĳ��ļ�����Ϣ  //�����ַ�ļ�����4k page��flash�л������
        if (Flag)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            //*BkMDReadAdrs = ((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(i)+SectorOffset;
            *BkMDReadAdrs = i;
            //printf("*BkMDReadAdrs = %d\n", *BkMDReadAdrs);
        }
        else
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            //*BkMDReadAdrs = ((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileID[i])+SectorOffset;
            *BkMDReadAdrs = FileID[i];
            //printf("*BkMDReadAdrs = %d\n", *BkMDReadAdrs);
        }

        for (j=0;j<SORT_FILENAME_LEN;j++)
        {
            pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
            if (
                (97 <=pTemp->SortFileName[j])
                && (122 >=pTemp->SortFileName[j])
            )
            {
                pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
            }
        }

        pTemp++;
        BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
    }

}


_FILE_INFO_SORT_CODE_
void GetDirSortName(UINT32 SectorOffset,UINT32 StartID, UINT16 * FileNum, UINT16 Flag, UINT8 DirSort)
{
    UINT16  i,j;
    UINT16  FileCount;
    UINT8  TempBuffer[SORT_FILENAME_LEN*2];
    FILE_TREE_BASIC stFileTreeBasic;
    uint32 TreeSectStart, SaveInfoStart;

    FILE_INFO_INDEX_STRUCT *pTemp;
    UINT16  *FileID;
    uint16 * BkMDReadAdrs;

    pTemp = gSortNameBuffer0;
    FileID = (UINT16 *)FileIDBuffer;

    BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);

    if (DirSort == 1)
    {
        TreeSectStart = MUSIC_TREE_INFO_SECTOR_START;
        SaveInfoStart = MUSIC_SAVE_INFO_SECTOR_START;
    }
    else if (DirSort == 2)
    {
        TreeSectStart = RECORD_TREE_INFO_SECTOR_START;
        SaveInfoStart = RECORD_SAVE_INFO_SECTOR_START;
    }
    else
    {
        *FileNum = 0;
        return;
    }

    gwSectorOffset = SectorOffset;
    gwDataBaseAddr = SaveInfoStart;

    if (Flag)
    {
        //first get info need count totalnum
        FileCount = 0;
        do
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + TreeSectStart)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(StartID + FileCount), sizeof(FILE_TREE_BASIC), (uint8 *)&stFileTreeBasic);
            FileCount++;

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            //*BkMDReadAdrs = ((UINT32)(MediaInfoAddr+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset;
            *BkMDReadAdrs = stFileTreeBasic.dwBasicInfoID;

            for (j=0;j<SORT_FILENAME_LEN;j++)
            {
                pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
                if (
                    (97 <=pTemp->SortFileName[j])
                    && (122 >=pTemp->SortFileName[j])
                )
                {
                    pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
                }
            }

            pTemp++;
            BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
        }
        while (stFileTreeBasic.dwNextBrotherID != 0xffffffff);

        *FileNum = FileCount;

    }
    else
    {
        FileCount = *FileNum;

        for (i=0;i<FileCount;i++)
        {
            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + TreeSectStart)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(StartID + FileID[i]), sizeof(FILE_TREE_BASIC), (uint8 *)&stFileTreeBasic);

            MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+SaveInfoStart)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(stFileTreeBasic.dwBasicInfoID)+SectorOffset, SORT_FILENAME_LEN*2, TempBuffer);
            *BkMDReadAdrs = stFileTreeBasic.dwBasicInfoID;

            for (j=0;j<SORT_FILENAME_LEN;j++)
            {
                pTemp->SortFileName[j] = (TempBuffer[j*2]&0xff)+((TempBuffer[j*2+1]&0xff)<<8);
                if (
                    (97 <=pTemp->SortFileName[j])
                    && (122 >=pTemp->SortFileName[j])
                )
                {
                    pTemp->SortFileName[j] = pTemp->SortFileName[j] - 32;
                }
            }

            pTemp++;
            BkMDReadAdrs = (uint16 *)(pTemp->SortFileName + SORT_FILENAME_LEN);
        }
    }
}


/*
--------------------------------------------------------------------------------
  Function name : UINT32 SaveSortInfo(UINT32 uiSaveType, UINT32 ulSortSectorAddr)
  Author        : anzhiguo
  Description   : ���������Ϣ

  Input         : uiSaveType ������Ϣ������ : ID3_TITLE_TYPE :FILE_NAME_TYPE:ID3_ARTIST_TYPE:ID3_ALBUM_TYPE ��������
                  ulSortSectorAddr ��Ϣ����ĵ�ַ��������ַ����sec��ַ ?
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
UINT32 SaveSortInfo(UINT16 Deep, UINT8* SubInfoBuffer,UINT16 StartID,UINT8* FileIDBuf,UINT16 Flag)
{
    UINT32  i,j;

    UINT32 uiFileCount = 0;
    UINT32 uiFileSubCount = 0;  // �ļ���������
    UINT32 uiSameCount = 0;     // ID3��Ϣ����ʱ����ͬ���ͼ���
    UINT32 uiFileIndex = 0;     // Flash Page���Ƽ���
    UINT16 uiFileSortTemp = 0;  // �ļ��������ʱ����

    UINT32 uiSubCount = 0;      // ����ÿ���������ļ�ID3��Ϣ�������
    UINT32 uiSubCountTotal = 0; // ����ID3��Ϣ����������ļ�����

    UINT32 ulID3SubSectorAddr = 0; // ID3��Ϣ������Ϣ�����ַ

    UINT16 StartfileID = StartID; // ������Ϣ��ʼ�ļ���
    UINT16 PreSortFileName[SORT_FILENAME_LEN + 2];

    FILE_INFO_INDEX_STRUCT  *pTemp = NULL; // ��������ͷָ��
    if (!Flag)
    {
        //memcpy(FileIDBuf,FileIDBuffer,4096);
        memcpy(FileIDBuf, FileIDBuffer, 2 * SORT_FILENUM_DEFINE);
    }
    ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);//����дflash����

    switch (Deep)
    {
        case 1:
            // case FILE_NAME_TYPE:
            // case ID3_TITLE_TYPE:
            i = 1;
            break;
        case 2:
            // case ID3_ARTIST_TYPE:
            i = 0;
            break;
            // case ID3_ALBUM_TYPE:
        case 3:
            i = 0;
            break;
            // case ID3_GENRE_TYPE:
        case 4:
            i = 0;
            break;
        default:
            break;

    }

    for (;i<CHILD_CHAIN_NUM;i++)
    {
        pTemp = pChildChainHead[i];

#ifdef _WATCH_DOG_
        WatchDogReload();
#endif
        while (pTemp->pNext!=NULL)
        {
            pTemp = pTemp->pNext;
            uiFileSortTemp = (UINT16)(pTemp - &gSortNameBuffer0[0]); //���㵱ǰ���ļ��������ļ��е������

            //���㵱ǰҪ�������Ϣ��gSortNameBuffer0[0]��ƫ�Ƶ�ַ����ΪpChildChainHead��������Ϣ��Դ��gSortNameBuffer0
            if (Flag)
            {
                FileIDBuffer[StartID*2 +uiFileIndex++] =  uiFileSortTemp & 0xFF;     //�����ļ����������
                FileIDBuffer[StartID*2 +uiFileIndex++] = (uiFileSortTemp>>8) & 0xFF;//�����ļ����������

                //printf("uiFileSortTemp = %d = %d\n", uiFileSortTemp, StartID);
            }
            else
            {

                FileIDBuffer[StartID*2 +uiFileIndex++] =  FileIDBuf[uiFileSortTemp*2] & 0xFF;     //�����ļ����������
                FileIDBuffer[StartID*2 +uiFileIndex++] =  FileIDBuf[uiFileSortTemp*2+1] & 0xFF;//�����ļ����������

                //printf("Start = %d\n", StartID);
            }

            //printf("id = %d\n", *(uint16 *)(FileIDBuffer + StartID*2 + uiFileIndex - 2));
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif

            switch (Deep)
            {
                case 1://һ�����ʱ��ֻҪ���յ��ļ���˳������������Ҫsub�ӡ�����Ϣ
                    // case FILE_NAME_TYPE:
                    // case ID3_TITLE_TYPE:
                    uiFileSubCount++;
                    break;
                    // case ID3_ARTIST_TYPE://ͬһ�������� ��ר���м����ļ����������һ���ļ���ƫ�Ƶ�ַ���������ר���µ��ļ�����
                    //  case ID3_ALBUM_TYPE :
                    // case ID3_GENRE_TYPE   :
                case 2:
                case 3:
                case 4:
                    {
                        if (0 == uiFileSubCount)
                        {
                            SubInfoBuffer[uiSubCount++] = StartfileID&0xFF; // ��ʼ�ļ���
                            SubInfoBuffer[uiSubCount++] = (StartfileID>>8)&0xFF;// ��ʼ�ļ���

                            SubInfoBuffer[uiSubCount++] = 0;//�ӡ�����ʼλ�á���������һ��ʱ��д������
                            SubInfoBuffer[uiSubCount++] = 0;
                            // printf("uiFileSubCount = %d\n", uiFileSubCount);
                            uiFileSubCount++;
                        }
                        //PinyinStrnCmp ���� 1 ��ʾ �����ַ��ܡ��ȡ�ͬһ�������һ�ͬһ��ר��
                        else if (1==PinyinStrnCmp((uint16*)(pTemp->SortFileName), PreSortFileName/*, SORT_FILENAME_LEN*/)) //(pTemp->SortFileName)����(pNodePrev->SortFileName)
                        {
                            //printf("same name\n");
                            uiSameCount++;
                        }
                        else // (pTemp->SortFileName)����(pNodePrev->SortFileName),������С�ڵ��������Ϊ�Ѿ��Ź������
                        {
                            StartfileID += (uiSameCount+1);
                            SubInfoBuffer[uiSubCount++] = 0; // �����ռ�, ʹÿ�α����СΪ2ָ����
                            SubInfoBuffer[uiSubCount++] = 0; // �����ռ�, ʹÿ�α����СΪ2ָ����

                            //��ǰ������� ��ר�����������ж��ٸ��ļ�
                            SubInfoBuffer[uiSubCount++] = (uiSameCount+1)&0xFF; // ��¼��һ��Item������Ԫ�ظ���
                            SubInfoBuffer[uiSubCount++] = ((uiSameCount+1)>>8)& 0xFF;


                            //��¼��һ�����ࡦ����ʼ�ļ�λ��
                            SubInfoBuffer[uiSubCount++] = StartfileID&0xFF;
                            SubInfoBuffer[uiSubCount++] = (StartfileID>>8)&0xFF;

                            SubInfoBuffer[uiSubCount++] = 0; // //�ӡ�����ʼλ�á���������һ��ʱ��д������
                            SubInfoBuffer[uiSubCount++] = 0; // //�ӡ�����ʼλ�á���������һ��ʱ��д������


                            uiFileSubCount++;

                            uiSameCount = 0;
                        }
                    }
                    break;

                default:
                    break;

            }

            for (j=0;j<(SORT_FILENAME_LEN + 2);j++)
            {
                PreSortFileName[j] = pTemp->SortFileName[j];
            }
            uiFileCount++;
        }

    }
#if 1
    //  if (uiSubCount)//// ����������Ϣ����2K�Ĳ���
    //  {

    SubInfoBuffer[uiSubCount++] = 0; // �����ռ�, ʹÿ�α����СΪ2ָ����
    SubInfoBuffer[uiSubCount++] = 0; // �����ռ�, ʹÿ�α����СΪ2ָ����

    SubInfoBuffer[uiSubCount++] = (uiSameCount+1)&0xFF; // ͳ�����һ��Item�ĸ���
    SubInfoBuffer[uiSubCount++] = ((uiSameCount+1)>>8)&0xFF;

    // }
    //printf("uiFileSubCount = %d\n", uiFileSubCount);
#endif

    return uiFileSubCount;


}

/*
--------------------------------------------------------------------------------
  Function name : void SortUpdateFun(void)
  Author        : anzhiguo
  Description   : ��ָ����Χ�ڵ��ļ���������

  Input         : StartNum--�����ļ�����ʼ�ļ���
                  FileNum --������ļ�����
                  SubInfoBuffer  --��ű���������sub��Ϣ
                  PreSubInfoBuffer -- �����һ�������sbu��Ϣ��Ϊ���������ṩһЩ��Ҫ�Ĳ���
                  Deep    -- �жϵ�ǰ���������
  Return        : ���ص�ǰ��Χ�������֡�����

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
UINT32 SingleTypeSortFunction(UINT16 StartNum,UINT16 FileNum,UINT16 *SubInfoBuffer,UINT16 Deep,UINT16 Flag)
{
    UINT32  i,j,SortSubNum;
    UINT16  filenum;
    UINT32 uiFirsCharSortVal=0; // ��ǰ�������������ֵ����ַ���ƴ������ֵ

    FILE_INFO_INDEX_STRUCT *pHeadBk = NULL;
    FILE_INFO_INDEX_STRUCT *pTemp;

    pTemp = &gSortNameBuffer0[StartNum];

    filenum = FileNum;

    for (i=0;i<filenum;i++)
    {
#ifdef _WATCH_DOG_
        if(i % 200 == 0)
        {
            WatchDogReload();
        }
#endif
        uiFirsCharSortVal = /*GetCmpResult*/(pTemp->SortFileName[0]);
        j = 0;
        while (uiFirsCharSortVal>gChildChainDivValue[j]) // ͨ�����ļ��ĵ�һ���ַ����ж���Ҫ���ĸ�������������
            j++;

        if (pHeadBk != pChildChainHead[j])
        {
            pHeadBk = pChildChainHead[j];                                       /////////////
            pChildChainLast = pChildChainHead[j];
            pChildChainLast2 = pChildChainHead[j];
        }
        BrowserListInsertBySort(pChildChainHead[j], pTemp); // ��gSortNameBuffer0�е��ļ���Ϣ�����������pChildChainHead��
        pTemp++;
    }

    //SortSubNum ���������ķ���sub��Ŀ
    SortSubNum = SaveSortInfo(Deep, (UINT8*)SubInfoBuffer, StartNum, FileIDBuffer1, Flag); //�����ļ�������š��Լ�������Ϣ�������ظ����ļ�������

    return SortSubNum;
}

/*
--------------------------------------------------------------------------------
  Function name : void GetSortName(UINT32 SectorOffset)
  Author        : anzhiguo
  Description   : �ļ������ܺ���

  Input         : uiSaveAddrOffset���������ַ�(�ļ�����id3title��id3singer��id3ablum)���ļ���Ϣ�е�ƫ�Ƶ�ַ
                  FileNum---������ļ�����
                  Deep  --- ��������(Ŀ¼���)
                 // uiSortType ---���������
                  SortSubNum ---��������ɵ��ӡ�����
                  buffer1   --- ������Ϣ���buff
                  buffer2   --- �������õ��ϼ��������Ϣbuf
                  Flag  --- ��ȡ�ļ���Ϣ�ķ�ʽ
                  SubNum --- ���ڷּ�����ʱ��ȷ�����ѭ������
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ��gSortNameBuffer0��gSortNameBuffer1 �е���Ϣ���з������С������ɵ���Ӧ������pChildChainHead���ʵ���λ����
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
void SortFunction(UINT32* uiSaveAddrOffset,UINT16 * FileNum,UINT16 Deep,UINT32 *SortSubNum,UINT16 Flag,UINT16*buffer1,UINT16*buffer2,UINT16 SubNum, UINT32 StartID, UINT8 DirSort)
{
    UINT32  i,j;

    UINT16* subbuf1,subbuf2;
    UINT32 uiFirsCharSortVal=0; // ��ǰ�������������ֵ����ַ���ƴ������ֵ
    UINT16  filenum = 0;
    SORTINFO_STRUCT * pSortInfo1;
    SORTINFO_STRUCT * pSortInfo2;

    FILE_INFO_INDEX_STRUCT *pTemp;

    pTemp = gSortNameBuffer0;

    memset(&gSortNameBuffer0[0], 0, sizeof(FILE_INFO_INDEX_STRUCT) * SORT_FILENUM_DEFINE);

    //DEBUG("FileNum = %d", FileNum);
    gBkMDReadAdrs1 = 0xffffffff;
    gBkMDReadAdrs2 = 0xffffffff;

    {
        if (DirSort)
        {
            GetDirSortName(uiSaveAddrOffset[Deep-1],StartID, FileNum, Flag, DirSort);
        }
        else
        {
            GetSortName(uiSaveAddrOffset[Deep-1], *FileNum, Flag);     // ���ݵ�ǰ��������ȡ���ȡ��Ҫ��������Ϣ(�����ļ�)��gSortNameBuffer0��
        }
    }

    if (Flag)
    {
        ChildChainInit(1);  // �Ը���������ͷ�ڵ���г�ʼ������������ÿ�������ķֶιؼ���

        //SortSubNum��ȡ����һ�������ķ����ӡ�����
        //��gSortNameBuffer0�ĵ�0��λ�õ��ļ���ʼ������FileNum���ļ��������ķ�����Ϣ�Խṹ�����buffer1��
        //buffer2 �����һ���������Ϣ�����ڲ�����ʱ���ṩһЩ����
        filenum = SingleTypeSortFunction(0,*FileNum,buffer1,Deep,Flag);
        *SortSubNum  = filenum;
    }
    else
    {
        UINT16 IDnum ,basenum;
        pSortInfo1 = (SORTINFO_STRUCT *)buffer2 ;
        pSortInfo1->ItemBaseID = 0 ;
        pSortInfo1->ItemNum = 0;
        basenum = 0;
        pSortInfo2 = (SORTINFO_STRUCT *)buffer1 ;

        for (i=0; i < SubNum; i++)
        {   //��ȡ�����ӡ�����
            ChildChainInit(0);
            IDnum = SingleTypeSortFunction(pSortInfo1->BaseID,pSortInfo1->FileNum,(UINT16*)pSortInfo2,Deep,Flag);
            //ƫ�ƴ�������sub��ͷָ�롦Ϊ�´�����׼��,�����Ƿ���Ҫ�ӱ�����ʩ?????

            pSortInfo2 += IDnum;

            //numtemp = pSortInfo1->ItemNum;
            //��������ӡ�����
            basenum += IDnum ;
            pSortInfo1->ItemNum = IDnum;
            //�ṹָ���Լӡ�ָ��һ��sub�����Ϣ�ṹ������������ʼid
            if (i < (SORT_FILENUM_DEFINE - 1))
            {
                pSortInfo1 ++;

                pSortInfo1->ItemBaseID = basenum ;
            }
            filenum +=IDnum;

        }
        //������ϲ�������ɡ���Ҫдbuffer2��flash���Ա�����Ϣ

        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);

        if (DirSort == 0)
        {
            MDWrite(DataDiskID, MediaInfoSaveAdd[Deep], SORT_FILENUM_DEFINE * sizeof(SORTINFO_STRUCT) / SECTOR_BYTE_SIZE, buffer2);
        }
        SortPageBufferInit((UINT8*)buffer2);
    }

    Deep --;

    if (Deep == 0)
    {
        //��fileidbuffer дflash���Լ�����sub��Ϣbuffer
        ModuleOverlay(MODULE_ID_FLASH_PROG, MODULE_OVERLAY_ALL);

        if (DirSort == 0)
        {
            MDWrite(DataDiskID, MediaInfoSaveAdd[0], (2 * SORT_FILENUM_DEFINE) / SECTOR_BYTE_SIZE, FileIDBuffer);
        }
        else
        {

            //printf("xxx\n");
            for (i = 0; i < (*FileNum * 2); i++)
            {
                gFileSortBuffer[gFileSortBufferIndex++] = FileIDBuffer[i];
                if (gFileSortBufferIndex == MEDIAINFO_PAGE_SIZE)
                {
                    MDWrite(DataDiskID, MediaInfoSaveAdd[0], MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFileSortBuffer);
                    gFileSortBufferIndex = 0;
                    memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);
                    MediaInfoSaveAdd[0] += MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE;
                }
            }
        }
        return;
    }
    else
    {
        SortFunction(uiSaveAddrOffset, FileNum, Deep, SortSubNum,0,buffer2,buffer1,filenum, StartID, DirSort);
    }

}


/*
--------------------------------------------------------------------------------
  Function name : void SortUpdateFun(void)
  Author        : anzhiguo
  Description   : ���ļ�����Ϣ�������򡦲���ָ��Flash�д洢������Ϣ

  Input         :

  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_FILE_INFO_SORT_CODE_
void SortUpdateFun(MEDIALIB_CONFIG * Sysfilenum ,UINT32 MediaInfoAddress)
{
    UINT16   PathDeep;//ý����������(Ŀ¼����)
    UINT16   *buffer1;
    UINT16   *buffer2;
    UINT32   uiSortTypeCount = 0;            // ��������ѡ��

    UINT32   uiCountTemp[SORT_TYPE_ITEM_NUM];
    UINT16   FileNum;
    UINT32   StartID;

    if (Sysfilenum->gMusicFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;

        for (uiSortTypeCount=0;uiSortTypeCount<SORT_TYPE_ITEM_NUM;uiSortTypeCount++)
            //uiSortTypeCount = ID3_ARTIST_TYPE;
        {
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif
            switch (uiSortTypeCount)//���á�Ӧ�ĵ�ַ��Ϣ
            {
                    /*
                    case FILE_NAME_TYPE:
                        //������������������浽flash�еĵ�ַ
                        MediaInfoSaveAdd[0] = MediaInfoAddress + FILENAME_SORT_INFO_SECTOR_START;
                        //����ʼǰ����Ҫ�Ӹõ�ַ����ȡ��Ҫ��������Ϣ
                        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;

                        PathDeep = 1;
                        uiSortType = SORT_TYPE1;
                        break;
                    */
                case ID3_ALBUM_TYPE:
                    MediaInfoReadAdd[0] = TRACKID_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;

                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3ALBUM_SORT_INFO_SECTOR_START;
                    MediaInfoSaveAdd[1] = MediaInfoAddress + ID3ALBUM_SORT_SUB_SECTOR_START;

                    PathDeep = 2;
                    break;


                case ID3_TITLE_TYPE:
                    MediaInfoReadAdd[0] = ID3_TITLE_SAVE_ADDR_OFFSET;//������ļ���Ϣ��ƫ�Ƶ�ַ(�����ڱ����ļ���Ϣ��ʼ��ַ)
                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3TITLE_SORT_INFO_SECTOR_START;

                    PathDeep = 1;
                    break;

                case ID3_ARTIST_TYPE:
                    MediaInfoReadAdd[0] = TRACKID_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[2] = ID3_SINGLE_SAVE_ADDR_OFFSET;

                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3ARTIST_SORT_INFO_SECTOR_START;
                    MediaInfoSaveAdd[1] = MediaInfoAddress + ID3ARTIST_ALBUM_SORT_SUB_SECTOR_START;
                    MediaInfoSaveAdd[2] = MediaInfoAddress + ID3ARTIST_SORT_SUB_SECTOR_START;
                    //printf("adddr = %d\n", MediaInfoAddress + ID3ARTIST_SORT_INFO_SECTOR_START);
                    PathDeep = 3;

                    buffer2 = (UINT16*)GerneAblumBuffer;
                    buffer1 = (UINT16*)SingerBuffer;
                    break;


                case ID3_GENRE_TYPE:
                    MediaInfoReadAdd[0] = TRACKID_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[2] = ID3_SINGLE_SAVE_ADDR_OFFSET;
                    MediaInfoReadAdd[3] = ID3_GENRE_SAVE_ADDR_OFFSET;

                    MediaInfoSaveAdd[0] = MediaInfoAddress + ID3GENRE_SORT_INFO_SECTOR_START;
                    MediaInfoSaveAdd[1] = MediaInfoAddress + ID3GENRE_ALBUM_SORT_SUB_SECTOR_START;
                    MediaInfoSaveAdd[2] = MediaInfoAddress + ID3GENRE_ARTIST_SORT_SUB_SECTOR_START;
                    MediaInfoSaveAdd[3] = MediaInfoAddress + ID3GENRE_SORT_SUB_SECTOR_START;

                    buffer1 = (UINT16*)GerneAblumBuffer;
                    buffer2 = (UINT16*)SingerBuffer;
                    PathDeep = 4;
                    break;

            }
            //��ʼһ��(���ɡ�ר�������֡��ļ���)��Ϣ������
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE

            SortFunction(MediaInfoReadAdd,(uint16 *)&(Sysfilenum->gMusicFileNum),PathDeep,&uiCountTemp[uiSortTypeCount],1,buffer1,buffer2,0, 0, 0);//��ȡ��Ӧ���ļ���Ϣ��������һ��ֻ����һ����Ϣ

        }

        Sysfilenum->gID3TitleFileNum =  uiCountTemp[0]; // �õ�����ID3 Title��Ϣ���ļ�����   uiCountTemp[0] �����е������ļ�����
        Sysfilenum->gID3AlbumFileNum =  uiCountTemp[1]; // �õ�����ID3 Album��Ϣ���ļ�����
        Sysfilenum->gID3ArtistFileNum = uiCountTemp[2]; // �õ�����ID3 Artist��Ϣ���ļ�����
        Sysfilenum->gID3GenreFileNum =  uiCountTemp[3]; // �õ�����ID3 Genre��Ϣ���ļ�����
    }

    if (Sysfilenum->gTotalFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
        MediaInfoSaveAdd[0] = MediaInfoAddress + MUSIC_TREE_SORT_INFO_SECTOR_START;
        gFileSortBufferIndex = 0;
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

        StartID = 0;
        FileNum = 0;

        //printf("totalnum = %d\n", Sysfilenum->gTotalFileAndDirNum);
        while (StartID < Sysfilenum->gTotalFileNum)
        {
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif
            MedialibUpdateDisplayHook();//PAGE
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 2;
            //printf("StartID = %d\n", StartID);
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 1);
            StartID += FileNum;
            FileNum = 0;
        }

        if (gFileSortBufferIndex)
        {
            MDWrite(DataDiskID, MediaInfoSaveAdd[0], MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFileSortBuffer);
        }

    }

    if (Sysfilenum->gRecordFmFileNum != 0)
    {
        buffer1 = (UINT16*)GerneAblumBuffer;
        buffer2 = (UINT16*)SingerBuffer;
        MediaInfoReadAdd[0] = FILE_NAME_SAVE_ADDR_OFFSET;
        MediaInfoReadAdd[1] = ATTR_SAVE_ADDR_OFFSET;
        MediaInfoSaveAdd[0] = MediaInfoAddress + RECORD_TREE_SORT_INFO_SECTOR_START;
        gFileSortBufferIndex = 0;
        memset(gFileSortBuffer, 0, MEDIAINFO_PAGE_SIZE);

        StartID = 0;
        FileNum = 0;

        while (StartID < Sysfilenum->gRecordFmFileNum)
        {
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif
            memset(FileIDBuffer,0,sizeof(FileIDBuffer));           // 4 * SORT_FILE_NAME_BUF_SIZE
            memset(GerneAblumBuffer,0,sizeof(GerneAblumBuffer));   // 8*SORT_FILE_NAME_BUF_SIZE
            memset(SingerBuffer,0,sizeof(SingerBuffer));           // 16*SORT_FILE_NAME_BUF_SIZE
            PathDeep = 2;
            SortFunction(MediaInfoReadAdd,&FileNum,PathDeep,&uiCountTemp[0],1,buffer1,buffer2,0, StartID, 2);
            StartID += FileNum;
            FileNum = 0;
        }

        if (gFileSortBufferIndex)
        {
            MDWrite(DataDiskID, MediaInfoSaveAdd[0], MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE, gFileSortBuffer);
        }
    }
}


#endif

/*
********************************************************************************
*
*                         End of FileInfoSort.c
********************************************************************************
*/
