/*
********************************************************************************
*                   Copyright (c) 2008,Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   MusicLrc.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               Chenwei          2009-4-7          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_MUSIC_LRC

#include "SysInclude.h"

#ifdef _MUSIC_

#include "MusicWinInterface.h"
#include "MusicLrc.h"

#include "MediaBroWin.h"

_ATTR_MUSIC_LRCCOMMON_DATA_   EXT int8           hLrcFile = (int8)-1;

//_ATTR_LCD_CODE_
UINT16 TxtGbk2Unicode(UINT16 *pSbuf, UINT16   *pTbuf, UINT16 Len)
{
    UINT16        iUnicodeChars;
    UINT8         GbkCode;
    UINT8         GbkCode1;
    UINT16        GbkCode2;
    UINT16        iType;                  // 1,signal byte charactor£»2, doubule bytes charactors

    UINT8       *pGbk;
    UINT16        *pUnicode;

    pGbk            =  (UINT8*) pSbuf;
    pUnicode        =   pTbuf;
    iType           =   1;
    iUnicodeChars   =   0;

    while   ('\0' != (*pGbk)){

        if (iUnicodeChars >= Len) {

            break;
        }

        GbkCode = *pGbk;
        GbkCode1 = *(pGbk+1);
        switch (Language)
        {
            case LANGUAGE_ENGLISH:                // english,CP936
            case LANGUAGE_CHINESE_S:              // simplified chinese, CP936
            case LANGUAGE_CHINESE_T:              // traditional chinese , CP950
            case LANGUAGE_KOREAN:
            case LANGUAGE_JAPANESE:               // korean , CP949
                iType = GetGbkCodeType(GbkCode, GbkCode1);
                break;

            default:
                iType = 1;
                break;
        }

        if (1 == iType) {

            GbkCode1 = *pGbk;
            *pUnicode = Gbk2Unicode((UINT16)GbkCode1,0);
        }
        else// if (2 == iType)
        {
            if('\0' == (*(pGbk + 1)))
            {
                break;
            }

            GbkCode2 = *(UINT16*)pGbk;
            *pUnicode = Gbk2Unicode(GbkCode2,1);
        }
        pUnicode++;
        pGbk          += iType;
        iUnicodeChars += iType;

    }
    *pUnicode = '\0';

    return iUnicodeChars;
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  INT16 MusicLrcCheck(UINT16 *pLongFileName)
*  Author:          ZhengYongzhi
*  Description:     find whether the LRC file that its name is the same with CRC name
*
*  Input:
*  Output:
*  Return:      0: OK
*               1: open fail
*               -1: find fail
*  Calls:
*
*  History:     <author>         <time>         <version>
*                Chenwei       2009-4-7        Ver1.0
*     desc: ORG
*
********************************************************************************
*/
_ATTR_MUSIC_LRC_INIT_CODE_
INT16 MusicLrcCheck(UINT16 *pLongFileName)
{
    UINT8  char1,char2;
    UINT16  TempLongFileName[MAX_FILENAME_LEN];
    UINT16 FileNameLen;
    INT16  FindFileResult;
    INT16  i;
    uint32 DirClus,Index;
    FILE_TREE_BASIC FileTreeBasic;
    uint16 temp;

    //hLrcFile = (int8)-1;// by zs 09.06.24
    memcpy(&LrcFileInfo, &AudioFileInfo, sizeof(AudioFileInfo));
    LrcFileInfo.pExtStr = (uint8*)"LRC";

    //if from medialib play music
    if(AudioFileInfo.ucSelPlayType != SORT_TYPE_SEL_BROWSER)
    {
        DirClus = AudioFileInfo.FindData.Clus;
        Index   = AudioFileInfo.FindData.Index -1;
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), 2, (uint8 *)&temp);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp) , sizeof(FILE_TREE_BASIC), (uint8 *)&FileTreeBasic);
        MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_CLUS_SAVE_ADDR_OFFSET, 4, (uint8 *)&(DirClus));
        //MDReadData(DataDiskID,((UINT32)(MediaInfoAddr+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_INDEX_SAVE_ADDR_OFFSET, 4, (uint8 *)&(Index));

        DEBUG("Dirclus = %d,index = %d",DirClus,Index);
        LrcFileInfo.TotalFiles = GetTotalFiles(DirClus, LrcFileInfo.pExtStr, FS_FAT);
        LrcFileInfo.FindData.Clus = DirClus;
    }
    else    //from browser play music
    {
        LrcFileInfo.TotalFiles = GetTotalFiles(LrcFileInfo.FindData.Clus, LrcFileInfo.pExtStr, FS_FAT);
    }

    for(i = 0 ; i < MAX_FILENAME_LEN; i++)
    {
        if(pLongFileName[i] == 0)
            break;
    }
    FileNameLen = i - 4;//XXX.LRC  exclude  .LRC four character

    FindFileResult = FindFirstFile(&LrcFileInfo.Fdt, &LrcFileInfo.FindData, LrcFileInfo.pExtStr, FS_FAT);

    if (FindFileResult == RETURN_OK)
    {
        for (LrcFileInfo.CurrentFileNum = 1; LrcFileInfo.CurrentFileNum <= LrcFileInfo.TotalFiles; LrcFileInfo.CurrentFileNum++)
        {
            if (FindFileResult != RETURN_OK)
            {
                return -1;
            }

            GetLongFileName(LrcFileInfo.FindData.Clus, LrcFileInfo.FindData.Index - 1, FS_FAT,TempLongFileName);

            for (i = 0; i < FileNameLen; i++)
            {
                char1 = pLongFileName[i];
                char2 = TempLongFileName[i];

                if(char1 > 96 && char1 < 123)
                    char1 -= 32;

                if(char2 > 96 && char2 < 123)
                    char2 -= 32;

                if (char1 != char2)
                    break;

            }

            if( i == FileNameLen )
            {
                return RETURN_OK;
            }

            FindFileResult = FindNextFile(&LrcFileInfo.Fdt, &LrcFileInfo.FindData, LrcFileInfo.pExtStr, FS_FAT);
        }
    }

    return -1;
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  MusicLrcInit()
*  Author:          Chenwei
*  Description:     LRC display initial,open lrc file.get current LRC data of current time
*
*  Input:
*  Output:
*  Return:      0: OK
*               1: open fail
*               -1: find fail
*  Calls:
*
*  History:     <author>         <time>         <version>
*               Chenwei         2009-4-7        Ver1.0
*  desc:        ORG
*
********************************************************************************
*/
_ATTR_MUSIC_LRC_INIT_CODE_
INT16 MusicLrcInit(void)
{
    INT16  temp;
    UINT32 TempCurrTimeSec,TempCurrTimeMS,TempMinTime;
    UINT32 TempMaxTime,TempDispCurrTime;
    bool   MaxFlag = 1;

    CodecGetTime(&TempCurrTimeMS);          //get current playing time
    TempCurrTimeSec = TempCurrTimeMS / 1000;   //turn unit from ms to s

    // hLrcFile = (uint8)-1;
    if(hLrcFile >= 0)// by zs 09.06.24
    {
        return 1;
    }
    hLrcFile = FileOpen(LrcFileInfo.Fdt.Name, LrcFileInfo.FindData.Clus, LrcFileInfo.FindData.Index - 1, FS_FAT, (uint8*)"R");
    if (hLrcFile == NOT_OPEN_FILE)
    {
        return 1;
    }

    memset(LrcReadoutBuf, 0, sizeof(LrcReadoutBuf));
    memset(UnicodeLrcBuffer, 0, sizeof(UnicodeLrcBuffer));
    memset(GBKLrcBuffer.ArrayU8, 0, sizeof(GBKLrcBuffer));

    //PrevLrcPlayTimeSec = 0;
    NextLrcPlayTimeSec = 0xffffffff;

    LrcFlag = LRC_FLAG_NULL;
    LrcBufferOffset = 0;
    LrcBufferLen = FileRead((uint8 *)LrcReadoutBuf, LRC_BUFFER_LENGTH, hLrcFile);
    if (LrcBufferLen < LRC_BUFFER_LENGTH)
        LrcFlag |= LRC_FLAG_FILE_END;

    if (AUDIO_STATE_PAUSE != MusicPlayState)
    {
        do
        {
            temp = GetNextLrcTime(&TempCurrTimeMS);
            if ((TempCurrTimeMS < 0xffffffff) && (TempCurrTimeMS > TempCurrTimeSec))
            {
                if(TempMinTime > TempCurrTimeMS)
                {
                    TempMinTime = TempCurrTimeMS;
                }

                if(MaxFlag)
                {
                    TempMinTime = TempCurrTimeMS;
                    MaxFlag = 0;
                }
            }

            if ((TempCurrTimeMS > 0) && (TempCurrTimeMS <= TempCurrTimeSec))
            {
                if(TempCurrTimeMS > TempMaxTime)
                {
                    TempMaxTime = TempCurrTimeMS;
                    TempDispCurrTime = TempMaxTime;
                }
            }

        } while (temp != -1);
        NextLrcPlayTimeSec = TempMinTime;
        PrevLrcPlayTimeSec = TempDispCurrTime;
    }
    else
    {
          NextLrcPlayTimeSec = 0;
    }

    if (NextLrcPlayTimeSec != 0xffffffff)
    {
        FileSeek(0, SEEK_SET, hLrcFile);

        LrcFlag &= ~LRC_FLAG_FILE_END;
        LrcBufferOffset = 0;
        LrcBufferLen = FileRead((uint8*)LrcReadoutBuf, LRC_BUFFER_LENGTH, hLrcFile);
        if (LrcBufferLen < LRC_BUFFER_LENGTH)
        {
            LrcFlag |= LRC_FLAG_FILE_END;
        }

        GetLrcBuffer(PrevLrcPlayTimeSec);
        TxtGbk2Unicode(GBKLrcBuffer.ArrayU16, UnicodeLrcBuffer, (LRC_BUFFER_LENGTH / 2));
        SendMsg(MSG_MUSIC_DISPFLAG_LRC);
    }

    return 0;
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  MusicLrcDeInit()
*  Author:          Chenwei
*  Description:    LRC display auti-initial,close file and variables
*  Input:
*  Output:
*  Return:      0: close success
*               1: open fail
*
*  Calls:
*
*  History:     <author>         <time>         <version>
*               Chenwei         2009-4-7        Ver1.0
*  desc:        ORG
*
********************************************************************************
*/
_ATTR_MUSIC_LRCCOMMON_CODE_
INT16 MusicLrcDeInit(void)
{
    UINT16 temp;

    LrcFlag         = LRC_FLAG_NULL;
    LrcBufferOffset = 0;

    FileClose(hLrcFile);
    hLrcFile = (int8)-1;

    return 0;
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  MusicLrcPlay()
*  Author:          ZhengYongzhi
*  Description:    LRC display handle,check song play time and update display data.
*
*  Input:           LrcOffsetMS: LRC offset(ms)
*  Output:
*  Return:
*  Calls:
*
*  History:     <author>         <time>         <version>
*               Chenwei         2009-4-7         Ver1.0
*     desc: ORG
*
********************************************************************************
*/
_ATTR_MUSIC_LRCPLAY_CODE_
void MusicLrcPlay(INT32 LrcOffsetMS)
{
    INT16  temp;
    UINT32 TempCurrTimeSec,TempCurrTimeMS,TempMinTime;
    UINT32 TempMaxTime,TempDispCurrTime;
    bool   MaxFlag = 1;

    CodecGetTime(&TempCurrTimeMS);
    TempCurrTimeSec = (TempCurrTimeMS + LrcOffsetMS) / 1000;//add lrc offset,turn ms to s.

    if ((TempCurrTimeSec < NextLrcPlayTimeSec)&&(TempCurrTimeSec >= PrevLrcPlayTimeSec)
        ||(AUDIO_STATE_PAUSE == MusicPlayState))
    {
        return;
    }

    LrcFlag &= ~LRC_FLAG_FILE_END;
    LrcBufferOffset = 0;
    FileSeek(0, SEEK_SET, hLrcFile);
    LrcBufferLen = FileRead((uint8*)LrcReadoutBuf, LRC_BUFFER_LENGTH, hLrcFile);

    if (LrcBufferLen < LRC_BUFFER_LENGTH)
    {
        LrcFlag |= LRC_FLAG_FILE_END;
    }

    TempMaxTime = 0;
    TempMinTime = 0xffffffff;

    do
    {
        temp = GetNextLrcTime(&TempCurrTimeMS);
        if ((TempCurrTimeMS < 0xffffffff) && (TempCurrTimeMS > TempCurrTimeSec))
        {
            if(TempMinTime > TempCurrTimeMS)
            {
                TempMinTime = TempCurrTimeMS;
            }

            if(MaxFlag)
            {
                TempMinTime = TempCurrTimeMS;
                MaxFlag = 0;
            }
        }

        if ((TempCurrTimeMS > 0) && (TempCurrTimeMS <= TempCurrTimeSec))
        {
            if(TempCurrTimeMS > TempMaxTime)
            {
                TempMaxTime = TempCurrTimeMS;
                TempDispCurrTime = TempMaxTime;
            }
        }

    } while (temp != -1);
    NextLrcPlayTimeSec = TempMinTime;
    if(PrevLrcPlayTimeSec == TempDispCurrTime )
    {
       return;
    }
    PrevLrcPlayTimeSec = TempDispCurrTime;

    LrcFlag &= ~LRC_FLAG_FILE_END;
    LrcBufferOffset = 0;
    FileSeek(0, SEEK_SET, hLrcFile);
    LrcBufferLen = FileRead((uint8*)LrcReadoutBuf, LRC_BUFFER_LENGTH, hLrcFile);

    GetLrcBuffer(TempDispCurrTime);
    TxtGbk2Unicode(GBKLrcBuffer.ArrayU16, UnicodeLrcBuffer, (LRC_BUFFER_LENGTH / 2));
    SendMsg(MSG_MUSIC_DISPFLAG_LRC);

}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  MusicLrcBufferOffsetInc()
*  Author:          Chenwei
*  Description:     M LRC display data buffer add 1,and judge data exclude the range.
*
*  Input:
*  Output:
*  Return:           0: ok
*                   -1: reach to the end of file
*  Calls:
*
*  History:     <author>         <time>         <version>
*               Chenwei         2009-4-7         Ver1.0
*     desc: ORG
*
********************************************************************************
*/
_ATTR_MUSIC_LRCCOMMON_CODE_
INT16 MusicLrcBufferOffsetInc(void)
{
    if (++LrcBufferOffset >= LrcBufferLen)
    {
        if (LrcFlag & LRC_FLAG_FILE_END)//return when reach to the end of file
            return (-1);
        else
        {
            LrcBufferLen = FileRead((uint8 *)LrcReadoutBuf, LRC_BUFFER_LENGTH, hLrcFile);//get next length is LRC_BUFFER_LENGTH data.
            if (LrcBufferLen < LRC_BUFFER_LENGTH)
                LrcFlag |= LRC_FLAG_FILE_END;
            LrcBufferOffset = 0;
        }
    }
    return (0);
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  MusicLrcSeach()
*  Author:          ZhengYongzhi
*  Description:     search LRC data,adjust LrcReadoutBuf to format [xx:xx.xx]
*
*  Input:
*  Output:
*  Return:          0: find the data the format is match with [xx:xx......
*                   1: do not find this form data.
*                  -1: reach the end of file.err
*  Calls:
*
*  History:     <author>         <time>         <version>
*               Chenwei         2009-4-7         Ver1.0
*     desc: ORG
********************************************************************************
*/
_ATTR_MUSIC_LRCCOMMON_CODE_
INT16 MusicLrcSeach(void)
{
    unsigned int i,j;

    if (LrcReadoutBuf[LrcBufferOffset] != '[')//do not find '[',buffer offset increse one and exit
    {
        if ( -1 == MusicLrcBufferOffsetInc())
            return (-1);
    }
    else
    {
        i = 0;
        do
        {
            if ((LrcBufferOffset + i) >= (LrcBufferLen /*- 1*/))//if do not find the ']' in all LrcBufferLen,excute the code below,if not add one to judge.
            {
                if (LrcFlag & LRC_FLAG_FILE_END)
                    return (-1);

                for (j = 0; j < i; j++) //move the left i bytes to the front-end
                    LrcReadoutBuf[j] = LrcReadoutBuf[LRC_BUFFER_LENGTH - i + j];
                LrcBufferLen = FileRead((uint8 *)LrcReadoutBuf + i, LRC_BUFFER_LENGTH - i, hLrcFile);//read LRC_BUFFER_LENGTH - i byte data,and fill buf full
                if (LrcBufferLen < (LRC_BUFFER_LENGTH - i))
                {
                    LrcFlag |= LRC_FLAG_FILE_END;
                }
                else
                {
                    LrcBufferLen = LRC_BUFFER_LENGTH;
                }
                LrcBufferOffset = 0;
            }
            i++;
        }while (LrcReadoutBuf[LrcBufferOffset + i] != ']');

        if ((LrcReadoutBuf[LrcBufferOffset] == '[') &&
            (LrcReadoutBuf[LrcBufferOffset + 3] == ':') &&
           ((LrcReadoutBuf[LrcBufferOffset + 1] >= '0') &&
            (LrcReadoutBuf[LrcBufferOffset + 1] <= '9')) &&
           ((LrcReadoutBuf[LrcBufferOffset + 2] >= '0') &&
            (LrcReadoutBuf[LrcBufferOffset + 2] <= '9')) &&
           ((LrcReadoutBuf[LrcBufferOffset + 4] >= '0') &&
            (LrcReadoutBuf[LrcBufferOffset + 4] <= '9')) &&
           ((LrcReadoutBuf[LrcBufferOffset + 5] >= '0') &&
            (LrcReadoutBuf[LrcBufferOffset + 5] <= '9')))
        {
                return (0);
        }
        else
        {
            if ( -1 == MusicLrcBufferOffsetInc())
                return (-1);
        }
    }
    return (1);
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  GetNextLrcTime()
*  Author:          ZhengYongzhi
*  Description:     get the display time of next LRC
*
*  Input:
*  Output:
*  Return:
*  Calls:
*
*  History:     <author>         <time>         <version>
*               Chenwei         2009-4-7         Ver1.0
*     desc: ORG
********************************************************************************
*/
_ATTR_MUSIC_LRCCOMMON_CODE_
INT16 GetNextLrcTime(UINT32 *TimeSec)
{
    INT16 ReturnVal;

    while(1)
    {
        ReturnVal = MusicLrcSeach();
        if (ReturnVal == 0)
        {
            if (LrcReadoutBuf[LrcBufferOffset + 6] != ':')//time format is minute and second format.
            {
                *TimeSec = ((LrcReadoutBuf[LrcBufferOffset + 1] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 2] - '0')) * 60//Minutes
                            + (LrcReadoutBuf[LrcBufferOffset + 4] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 5] - '0');//Seconds
                LrcBufferOffset += 5;//jump out time lable
                break;
            }
            else if (((LrcReadoutBuf[LrcBufferOffset + 7] >= '0') && //time format is hour,minute and second format.
                        (LrcReadoutBuf[LrcBufferOffset + 7] <= '9')) &&
                       ((LrcReadoutBuf[LrcBufferOffset + 8] >= '0') &&
                        (LrcReadoutBuf[LrcBufferOffset + 8] <= '9')))
            {
                *TimeSec = ((LrcReadoutBuf[LrcBufferOffset + 1] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 2] - '0')) * 3600//Hours
                            + ((LrcReadoutBuf[LrcBufferOffset + 4] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 5] - '0')) * 60//Minutes
                            + (LrcReadoutBuf[LrcBufferOffset + 7] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 8] - '0');//Seconds
                LrcBufferOffset += 8;//jump out time lable
                break;
            }

            if ( -1 == MusicLrcBufferOffsetInc())
                return (-1);
        }
        else if (ReturnVal == -1)
        {
            return(-1);
        }
    }
    return (0);
}

/*
********************************************************************************
*  Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*  Function name :  GetLrcBuffer()
*  Author:          ZhengYongzhi
*  Description:     get the LCR data of current time,read to LrcBuffer from LrcReadoutBuf
*
*  Input:
*  Output:
*  Return:
*  Calls:
*
*  History:     <author>         <time>         <version>
*               Chenwei         2009-4-7         Ver1.0
*     desc: ORG
*
********************************************************************************
*/
_ATTR_MUSIC_LRCCOMMON_CODE_
INT16 GetLrcBuffer(UINT32 TimeSec)
{
    unsigned int i,j;
    int ReturnVal;

    while (1)
    {
        ReturnVal = MusicLrcSeach();
        if (ReturnVal == 0)
        {
            if (LrcReadoutBuf[LrcBufferOffset + 6] != ':')
            {
                if (TimeSec == ((LrcReadoutBuf[LrcBufferOffset + 1] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 2] - '0')) * 60
                            + (LrcReadoutBuf[LrcBufferOffset + 4] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 5] - '0'))
                {
                    LrcBufferOffset += 5;
                    break;
                }
                LrcBufferOffset += 5;
            }
            else if (((LrcReadoutBuf[LrcBufferOffset + 7] >= '0') &&
                        (LrcReadoutBuf[LrcBufferOffset + 7] <= '9')) &&
                       ((LrcReadoutBuf[LrcBufferOffset + 8] >= '0') &&
                        (LrcReadoutBuf[LrcBufferOffset + 8] <= '9')))
            {
                if (TimeSec == ((LrcReadoutBuf[LrcBufferOffset + 1] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 2] - '0')) * 3600
                            + ((LrcReadoutBuf[LrcBufferOffset + 4] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 5] - '0')) * 60
                            + (LrcReadoutBuf[LrcBufferOffset + 7] - '0') * 10 + (LrcReadoutBuf[LrcBufferOffset + 8] - '0'))
                {
                    LrcBufferOffset += 8;
                    break;
                }
                LrcBufferOffset += 8;//jump out time lable,but there is LRC file is MM:SS.MS format,then add 8 byte,it must get ']'
            }

            if ( -1 == MusicLrcBufferOffsetInc())
                return (-1);
        }
        else if (ReturnVal == -1)
        {
            return(-1);
        }
    }

    while (1)
    {
        if (LrcReadoutBuf[LrcBufferOffset] == ']')//find ']', if not,add one for buffer offset pointer.
        {
            if ((LrcReadoutBuf[LrcBufferOffset + 1] != '[') &&
                ((LrcReadoutBuf[LrcBufferOffset + 1] != 0x0d) && (LrcReadoutBuf[LrcBufferOffset + 1] != 0x0a)))//enter,wrap flag
            {
                if ( -1 == MusicLrcBufferOffsetInc())
                    return (-1);
                i = 0;
                while ((LrcReadoutBuf[LrcBufferOffset] != '[') &&
                      ((LrcReadoutBuf[LrcBufferOffset] != 0x0d) && (LrcReadoutBuf[LrcBufferOffset] != 0x0a)))
                {

                    GBKLrcBuffer.ArrayU8[i ++ ] = LrcReadoutBuf[LrcBufferOffset/* ++ */];
                    if ( -1 == MusicLrcBufferOffsetInc())
                        break;
                }
                GBKLrcBuffer.ArrayU8[i] = '\0';
                break;
            }
        }
        if ( -1 == MusicLrcBufferOffsetInc())
            return (-1);
    }
}

/*
********************************************************************************
*
*                         End of MusicLrc.c
*
********************************************************************************
*/
#endif
