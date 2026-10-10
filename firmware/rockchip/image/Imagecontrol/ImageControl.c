/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��  ImageControl.C
*
* Description:
*
* History:      <author>          <time>        <version>
*               evan wu      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_IMAGE_CONTROL_

#include "SysInclude.h"
#ifdef _PICTURE_
#pragma arm section code = "ImageContrlCode", rodata = "ImageContrlCode", rwdata = "ImageContrlData", zidata = "ImageContrlBss"
#define  _IN_IMAGE_CONTROL_
#include "pmu.h"
#include "../ImageInclude/image_globals.h"
#include "../ImageInclude/image_file_access.h"
#include "fsinclude.h"

#include "SysFindFile.h"

#include "ImageControl.h"
#include "PicInterface.h"
#include "PicWin.h"
#include "File.h"
extern int ImageCurrentCodec;

static void ImageEnterPMU(CurCodec)
{
    switch (CurCodec)
    {
#ifdef JPG_DEC_INCLUDE
        case (CODEC_JPG_DEC):
            {
                FREQ_EnterModule(FREQ_JPG);
                break;
            }
#endif

    }

}
static void ImageExitPMU(CurCodec)
{
    switch (CurCodec)
    {
#ifdef JPG_DEC_INCLUDE
        case (CODEC_JPG_DEC):
            {
                FREQ_ExitModule(FREQ_JPG);
                break;
            }
    }
#endif
}

static BOOLEAN ImagegetFileName()
{
    UINT16  bStatus;
    gwSaveDirClus = 0;
    bStatus = SysFindFileInit(&PicSysFileInfo, gbPicFileNum,FIND_FILE_RANGE_ALL,NULL,(UINT8*)PictureFileExtString);

    if (0xFFFF != bStatus)
    {
        strncpy((char *)PicFileInfo.FilePath, (char *)PicSysFileInfo.Path, PIC_PATH_NAME_LENGTH);  // path
        //long file name
        GetLongFileName(PicSysFileInfo.FindData.Clus,PicSysFileInfo.FindData.Index, FS_FAT, PicFileInfo.LongFileName);

        memcpy(PicFileInfo.FileName, PicSysFileInfo.Fdt.Name, 11);
        PicFileInfo.FileName[11]='\0';
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static UINT8 ImageCheckStreamType(UINT8 *pBuffer, FILE * hFile)
{
    UINT8 Retval = 0xff;
    uint8 buf[10];

    FileRead(buf, 10, (HANDLE)hFile);

    if ((buf[0] == 0xFF) && (buf[1] == 0xD8))   //SOI
    {
        printf("\nimage auto anlayse == jpg\n");
        *pBuffer++ = 'J';
        *pBuffer++ = 'P';
        *pBuffer++ = 'G';

        Retval = 1;
    }
    else if ((buf[0] == 0x42) && (buf[1] == 0x4D))  //BM
    {
        printf("\nimage auto anlayse == bmp\n");
        *pBuffer++ = 'B';
        *pBuffer++ = 'M';
        *pBuffer++ = 'P';

        Retval = 3;
    }

    FileSeek(0, SEEK_SET, (HANDLE)hFile);
    return Retval;
}

static UINT8 ImageGetFileType(UINT8 *pBuffer, UINT8 *pStr)
{
    UINT8 Len;
    UINT8 Retval = 0xff;
    UINT8 i;

    i = 0;
    Len = strlen((char*)pStr);

    while (i <= Len)
    {
        i += 3;
        if ((*(pBuffer + 0) == *(pStr + 0)) && (*(pBuffer + 1) == *(pStr + 1)) &&
                (*(pBuffer + 2) == *(pStr + 2)))
        {
            break;
        }
        pStr += 3;
    }

    if (i <= Len)
    {
        Retval = i / 3;
    }
    return (Retval);
}

int ImageCodecOverLay(unsigned long fileType)
{
    switch (fileType)
    {
        case 1:     //jpg
        case 2:
#ifdef JPG_DEC_INCLUDE
            ImageCurrentCodec = CODEC_JPG_DEC;
            ModuleOverlay(MODULE_ID_JPG_DECODE, MODULE_OVERLAY_ALL);
#endif
            break;
        case 3:
#ifdef BMP_DEC_INCLUDE
            ImageCurrentCodec = CODEC_BMP_DEC;
            ModuleOverlay(MODULE_ID_BMP_DECODE, MODULE_OVERLAY_ALL);
#endif
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name :void AudioStart(void)
  Author        : evan wu
  Description   :
  Input         :
  Return        :
  History       :<author>         <time>         <version>
                    evan wu            2009/02/20         Ver1.0
  desc          :  ORG
--------------------------------------------------------------------------------
*/
//uint8 * ImageFileExtString;
//extern UINT16 PicBuffer[LCD_HEIGHT][LCD_WIDTH];

static void ImageStart(void)
{
    unsigned long ret = 0;
    unsigned long ret1 = 0;

    //memset(PicBuffer, 0 , sizeof(PicBuffer));   //164373 clk
    //ImageMemSet((uint32*)PicBuffer, 0 , sizeof(PicBuffer)/(sizeof(int32) * 8)); //17943

    ImageDecState = IMAGE_STATE_START;

    ret = ImagegetFileName();
    if (FALSE == ret)
    {
        SendMsg(MSG_IMAGE_NOFILE);
        return;
    }

    //pImageFileHandle = (FILE*)FileOpen("\\", "12345678jpg", "W");
    pImageFileHandle = (FILE*)FileOpenA(PicFileInfo.FilePath, PicFileInfo.FileName, FileOpenStringR);
    if ((int)pImageFileHandle == NOT_OPEN_FILE)
    {
        SendMsg(MSG_IMAGE_OPENERROR);
        return;
    }

    ret = ImageGetFileType(&PicSysFileInfo.Fdt.Name[8],(UINT8 *)PictureFileExtString); //�Ժ�׺������
    ret1 = ImageCheckStreamType(&PicSysFileInfo.Fdt.Name[8], pImageFileHandle); //�Ա�־λ����
    if (0xFF == ret && 0xFF == ret1)
    {
        SendMsg(MSG_IMAGE_OPENERROR);
        return;
    }

    if (0xFF != ret1)
    {
        if (FALSE == ImageCodecOverLay(ret1))
        {
            SendMsg(MSG_IMAGE_OPENERROR);
            return;
        }
    }
    else if (0xFF != ret)
    {
        if (FALSE == ImageCodecOverLay(ret))
        {
            SendMsg(MSG_IMAGE_OPENERROR);
            return;
        }
    }

    //if(NULL != pImageFileHandle)
    {
        //FileClose((HANDLE)pImageFileHandle);
        //pImageFileHandle = NULL;
    }

    //file control function pointer initialization.
    ImageFileFuncInit();

    if (0 == ImageCodecOpen(0,(unsigned long)pImageFileHandle))
    {
        SendMsg(MSG_IMAGE_INIT_ERROR);
        return ;
    }

    ImageCodecGetOutputWH(&PicFileInfo.OutputWidth, &PicFileInfo.OutputHeight, &PicFileInfo.FirstOffset);

#ifdef _FRAME_BUFFER_
    ImageCodecGetResolution(&PicFileInfo.Width, &PicFileInfo.Height);
    if(PicFileInfo.Width > 1000 && PicFileInfo.Height > 1000)
    {
        //������ ��ʾ'���ڽ���'��ͼƬ �滻 ��������
        LCD_ClrSrc();//clear screen

        #ifdef IMAGE_DEC_ADVANCE
        if(CurImageIsDecoding())
        #endif
        {
            Lcd_FreshEn();
            Lcd_BuferTranfer();
        }
    }
    else
    {
        LCD_ClrSrc();
    }

    Lcd_FreshDisable();
#else
    LCD_ClrSrc();
#endif

    ImageCodecGetLen(&PicFileInfo.Len);
#ifdef AUTO_ADAPT_TO_LCD
    if(PicFileInfo.Len == ImageMaxWidth)
        LCD_SetDiaplayMode(LCD_MODE_0);
    else
        LCD_SetDiaplayMode(LCD_MODE_90);
#else
#ifdef IMAGE_DEC_ADVANCE
    LCD_SetDiaplayMode(LCD_MODE_90);
#endif
#endif
    SendMsg(MSG_IMAGE_INIT_SUCESS);
    ImageErrorNum = 0;
}
/*
--------------------------------------------------------------------------------
  Function name : BOOLEAN ImageDecoding(void)
  Author        : evan wu
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                   evan wu            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/

static void ImageDecoding(void)
{
    ImageDecState = IMAGE_STATE_DECODING;

    if (0 == ImageCodecDecode())
    {
        SendMsg(MSG_IMAGE_DEC_END);
        return;
    }

    ImageCodecGetCaptureBuffer((short*)&(PicFileInfo.pPicBuf),&PicFileInfo.dispOffset);
    //ImageCodecGetLen(&PicFileInfo.Len);
    SendMsg(MSG_IMAGE_DEC_SUCESS);

}

static BOOLEAN ImagePause(void)
{
    gPicAutoBrowserOn = 0;
    ImageSlideState = IMAGE_STATE_PAUSE;
    return TRUE;
}


static BOOLEAN ImageStop()
{
    if ((FILE*)(-1) != pImageFileHandle)
    {
        FileClose((HANDLE)pImageFileHandle);
        pImageFileHandle = (FILE*)(-1);
    }

    if (IMAGE_STATE_STOP == ImageDecState)
    {
        return FALSE;
    }

    ClearMsg(MSG_IMAGE_DEC);
    ClearMsg(MSG_IMAGE_DEC_SUCESS);
    ClearMsg(MSG_IMAGE_DEC_ERROR);
    ClearMsg(MSG_IMAGE_DEC_END);
    ClearMsg(MSG_IMAGE_DEC_SUCESS);
    ClearMsg(MSG_IMAGE_UNSPORTED_PIC);

    ImageDecState = IMAGE_STATE_STOP;
    ImageSlideShowTickBake = SysTickCounter;
    return TRUE;
}


static BOOLEAN ImageResume(void)
{
    gPicAutoBrowserOn = 1;
    ImageSlideState = IMAGE_STATE_SLIDING;
    return TRUE;

}
/*
--------------------------------------------------------------------------------
  Function name :BOOLEAN PictureGetNext(UINT32 msg)
  Author        :azg
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                  azg            2009/02/20         Ver1.0
  desc          :         ORG
--------------------------------------------------------------------------------
*/

//static BOOLEAN PictureGetNext(UINT32 msg)
static BOOLEAN ImageGetNextFile(UINT32 msg)
{
    BOOLEAN ret = TRUE;

    if (msg)
    {
        gbPicFileNum++;
        if (gbPicFileNum > PicSysFileInfo.TotalFiles )
        {
            gbPicFileNum = 1;
        }
    }
    else
    {
        gbPicFileNum--;
        if (gbPicFileNum == 0)
        {
            gbPicFileNum = PicSysFileInfo.TotalFiles;
        }
    }

    return TRUE;//(ImagegetFileName());
}

#ifdef IMAGE_DEC_ADVANCE
BOOLEAN CurImageIsDecoding(void)
{
    return (IMAGE_DEC_DECODING == CurImageDecState);
}
BOOLEAN NextImageIsDecoding(void)
{
    return (IMAGE_DEC_DECODING == NextImageDecState);
}
BOOLEAN PreImageIsDecoding(void)
{
    return (IMAGE_DEC_DECODING == PreImageDecState);
}

BOOLEAN CurImageIsStop(void)
{
    return (IMAGE_DEC_STOP == CurImageDecState);
}
BOOLEAN NextImageIsStop(void)
{
    return (IMAGE_DEC_STOP == NextImageDecState);
}
BOOLEAN PreImageIsStop(void)
{
    return (IMAGE_DEC_STOP == PreImageDecState);
}

BOOLEAN CurImageIsUnSupported(void)
{
    return (IMAGE_DEC_UNSUPPORTED == CurImageDecState);
}
BOOLEAN NextImageIsUnSupported(void)
{
    return (IMAGE_DEC_UNSUPPORTED == NextImageDecState);
}
BOOLEAN PreImageIsUnSupported(void)
{
    return (IMAGE_DEC_UNSUPPORTED == PreImageDecState);
}

BOOLEAN NextImageIsWaiting(void)
{
    return (IMAGE_DEC_WAITING == NextImageDecState);
}
BOOLEAN PreImageIsWaiting(void)
{
    return (IMAGE_DEC_WAITING == PreImageDecState);
}

void AdvanceDecVariableInit(void)
{
    CurImageDecState = IMAGE_DEC_DECODING;
    NextImageDecState = IMAGE_DEC_WAITING;
    PreImageDecState = IMAGE_DEC_WAITING;

    gCurKeyInfo = MENU_KEY;
    gCurDisFrameIndex = LCD_Get_Current_Index();
    gUnsupportFrameIndex = BUFFER_MAX_NUM;

    gIsContinueDecode = 1;
    gIsDisPreImage = 0;
    gIsDisNextImage = 0;
    gClearLCD = 0;

    gMarkSysTickCounter = SysTickCounter;
    gCurImageSlideShowTick = SysTickCounter;
}

//����Ԥ�����ͼƬ����״̬
static void SetAdvanceDecState(void)
{
    if(CurImageIsDecoding() && ImageIsStop())
    {
        CurImageDecState = IMAGE_DEC_STOP;
    }
    else if(NextImageIsDecoding() && ImageIsStop())
    {
        NextImageDecState = IMAGE_DEC_STOP;
    }
    else if(PreImageIsDecoding() && ImageIsStop())
    {
        PreImageDecState = IMAGE_DEC_STOP;
    }
}

static BOOLEAN ImageGetTwoNextFile(UINT32 msg)
{
    BOOLEAN ret = TRUE;
    int PicFileNum = gbPicFileNum;
    if (msg)
    {
        PicFileNum += 2;
        if (PicFileNum > PicSysFileInfo.TotalFiles)
            gbPicFileNum = PicFileNum - PicSysFileInfo.TotalFiles;
        else
            gbPicFileNum = PicFileNum;

    }
    else
    {
        PicFileNum -= 2;
        if (PicFileNum <= 0)
            gbPicFileNum = PicFileNum + PicSysFileInfo.TotalFiles;
        else
            gbPicFileNum = PicFileNum;
    }

    return TRUE;
}

//Ԥ����������ͼƬ
static void ImageDecNext(void)
{
    int PreFrameIndex, NextFrameIndex;

    PreFrameIndex = gCurDisFrameIndex - 1;
    if(PreFrameIndex < 0)
        PreFrameIndex = 2;

    NextFrameIndex = gCurDisFrameIndex + 1;
    if(NextFrameIndex > 2)
        NextFrameIndex = 0;

    if(gCurKeyInfo == MENU_KEY || gCurKeyInfo == FFD_KEY)
    {
        if(CurImageIsStop() && NextImageIsWaiting())
        {
            //decode next picture
            gIsContinueDecode = 1;
            NextImageDecState = IMAGE_DEC_DECODING;
            LCD_Set_Current_BufferFrame_Index(NextFrameIndex);

            ImageGetNextFile(1);
            ImageStart();
            ImageGetNextFile(0);
        }
        else if(NextImageIsStop() || NextImageIsUnSupported())
        {
            //decode previous picture
            gIsContinueDecode = 0;
            PreImageDecState = IMAGE_DEC_DECODING;
            LCD_Set_Current_BufferFrame_Index(PreFrameIndex);

            ImageGetNextFile(0);
            ImageStart();
            ImageGetNextFile(1);
        }
    }
    else if(gCurKeyInfo == FFW_KEY) //���������һ��
    {
        if(CurImageIsStop() && PreImageIsWaiting())
        {
            //decode previous picture
            gIsContinueDecode = 1;
            PreImageDecState = IMAGE_DEC_DECODING;
            LCD_Set_Current_BufferFrame_Index(PreFrameIndex);

            ImageGetNextFile(0);
            ImageStart();
            ImageGetNextFile(1);

        }
        else if(PreImageIsStop() || PreImageIsUnSupported())
        {
            //decode next picture
            gIsContinueDecode = 0;
            NextImageDecState = IMAGE_DEC_DECODING;
            LCD_Set_Current_BufferFrame_Index(NextFrameIndex);

            ImageGetNextFile(1);
            ImageStart();
            ImageGetNextFile(0);
        }
    }
    else if(gCurKeyInfo == PLAY_KEY) //�Զ������һ��
    {
        //decode next picture
        gIsContinueDecode = 0;
        NextImageDecState = IMAGE_DEC_DECODING;
        LCD_Set_Current_BufferFrame_Index(NextFrameIndex);

        ImageGetNextFile(1);
        ImageStart();
        ImageGetNextFile(0);
    }
}
#endif
//------------interface------------------
BOOLEAN ImageDecodeProc(MSG_ID id,void * msg)
{
    BOOLEAN ret = TRUE;
    switch (id)
    {
        case MSG_IMAGE_DECSTART://start decode
            ImageStart();
            break;

        case MSG_IMAGE_DEC://one line to decode
            ImageDecoding();
            break;

        case MSG_IMAGE_PAUSE://stop slide
            ImagePause();
            break;

        case MSG_IMAGE_RESUME://slide resume.
            ImageResume();
            break;

        case MSG_IMAGE_STOP://stop decoder.
            ImageStop();
            break;

        case MSG_IMAGE_GET_NEXT_FILE:
            ImageGetNextFile((UINT32)msg);
            break;
#ifdef IMAGE_DEC_ADVANCE
        case MSG_IMAGE_GET_TWO_NEXT_FILE:
            ImageGetTwoNextFile((UINT32)msg);
            break;
        case MSG_IMAGE_DECNEXT:
            ImageDecNext();
            break;
#endif
        default:
            break;
    }
    return ret;
}


//-----------------------functions for interface with ui layer.------------------------

/*
--------------------------------------------------------------------------------
  Function name :  void PictureInit(void)
  Author        :  evan wu
  Description   :

  Input         :
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     evan wu            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
void PictureInit(void *pArg)
{
    UINT16  bStatus;
    BOOLEAN ret;
    gbPicFileNum = ((PICTURE_THREAD_ARG*)pArg)->FileNum;
    gbPicFromWhere = ((PICTURE_THREAD_ARG*)pArg)->FromWhere;

    ClearMsg(MSG_IMAGE_DEC);
    ClearMsg(MSG_IMAGE_DEC_SUCESS);
    ClearMsg(MSG_IMAGE_DEC_ERROR);
    ClearMsg(MSG_IMAGE_DEC_END);
    ClearMsg(MSG_IMAGE_DEC_SUCESS);
    ClearMsg(MSG_IMAGE_UNSPORTED_PIC);

#ifndef AUTO_ADAPT_TO_LCD
    LCD_SetDiaplayMode(LCD_MODE_90);
#endif

    ImageMaxWidth = LcdContext.LcdMaxWidth;
    ImageMaxHeight = LcdContext.LcdMaxHeight;
    ImageLeft = 0;
    ImageTop = 0;

#ifdef IMAGE_DEC_ADVANCE
    ClearMsg(MSG_IMAGE_DECNEXT);
    AdvanceDecVariableInit();
#endif

    //������ͼƬ���ͣ���ʼ��ֱ����Ƶ������ʼ����Ƶ
    //����Ƶ����ImageStart��Ƶ��ImageStop��Ƶ�����ϵͳ���ȶ�
    ImageEnterPMU(CODEC_JPG_DEC);

    SendMsg(MSG_IMAGE_DECSTART);
}

/*
--------------------------------------------------------------------------------
  Function name :  UINT32 PictureService(void)
  Author        :  azg
  Description   :

  Input         :  null
  Return        :
  History       :  <author>         <time>         <version>
                    azg            2009/02/20         Ver1.0
                    evan wu        2009/04/30
  desc          :   ORG
--------------------------------------------------------------------------------
*/
extern void PictureDecodimg(void);

UINT32 PictureService(void)
{
    int ret = 0;
    //dec
    if (GetMsg(MSG_IMAGE_DECSTART))
    {
        ImageDecodeProc(MSG_IMAGE_DECSTART,0);
    }
    else if (GetMsg(MSG_IMAGE_DEC))
    {
        ImageDecodeProc(MSG_IMAGE_DEC,0);
    }
    else if (GetMsg(MSG_IMAGE_STOP))
    {
        ImageDecodeProc(MSG_IMAGE_STOP,0);
#ifdef _FRAME_BUFFER_
        #ifdef IMAGE_DEC_ADVANCE
        //��ǰ��ʾͼƬ������ɣ�ˢ��
        if(CurImageIsDecoding() && ImageIsStop())
        {
            gCurImageSlideShowTick = ImageSlideShowTickBake;
            Lcd_FreshEn();
            Lcd_BuferTranfer();
        }
        SetAdvanceDecState();
        #else
        Lcd_FreshEn();
        #endif
#endif
    }

    //slide
    if (GetMsg(MSG_IMAGE_PAUSE))
    {
        ImageDecodeProc(MSG_IMAGE_PAUSE,0);
    }
    else if (GetMsg(MSG_IMAGE_RESUME))
    {
        ImageDecodeProc(MSG_IMAGE_RESUME,0);
    }

#ifdef IMAGE_DEC_ADVANCE
    if(GetMsg(MSG_IMAGE_DECNEXT))
    {
        ImageDecodeProc(MSG_IMAGE_DECNEXT, 0);
    }
#endif
    return ret;
}
/*
--------------------------------------------------------------------------------
  Function name :  BOOLEAN AudioDecodeProc(MSG_ID id,void *msg)
  Author        :  azg
  Description   :

  Input         :  null
  Return        :
  History       :  <author>         <time>         <version>
                     azg            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
void PictureDeInit(void)
{
    ImageExitPMU(CODEC_JPG_DEC);
    ImageDecodeProc(MSG_IMAGE_STOP,NULL) ;

    LCD_SetDiaplayMode(LCD_MODE_0);

#ifdef _FRAME_BUFFER_
    LCD_Set_Current_BufferFrame_Index(0);
    Lcd_FreshEn();
#endif
}

/*
---------------------------------------



----------------------------------------
*/
//slide state
BOOLEAN ImageIsSliding(void)
{
    return (IMAGE_STATE_SLIDING == ImageSlideState);
}

BOOLEAN ImageIsPause(void)
{
    return (IMAGE_STATE_PAUSE == ImageSlideState);
}

//dec state
BOOLEAN ImageIsStart(void)
{
    return (IMAGE_STATE_START == ImageDecState);
}

BOOLEAN ImageIsDecoding(void)
{
    return (IMAGE_STATE_DECODING == ImageDecState);
}

BOOLEAN ImageIsStop(void)
{
    return (IMAGE_STATE_STOP == ImageDecState);
}

#pragma arm section code

//-----------------------------------------------------

#endif
/*
********************************************************************************
*
*                         End of ImageControl.c
*
********************************************************************************
*/


