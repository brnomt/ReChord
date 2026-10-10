/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   ImageControl.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _IMAGE_CONTROL_H_
#define _IMAGE_CONTROL_H_

#include "fsinclude.h"
#include "../ImageInclude/image_globals.h"
#undef  EXT
#ifdef _IN_IMAGE_CONTROL_
#define EXT
#else
#define EXT extern
#endif
//section define
#define _ATTR_IMAGE_TEXT_     
#define _ATTR_IMAGE_DATA_     
#define _ATTR_IMAGE_BSS_      

typedef enum
{
	//dec state
	IMAGE_STATE_START,//status of decode head
    IMAGE_STATE_DECODING, //decode status.
    IMAGE_STATE_STOP,   //decode finish.

    //sliding state
	IMAGE_STATE_PAUSE,//the slide pause status.
    IMAGE_STATE_SLIDING//the slide playing status.
}IMAGE_STATE;

typedef struct IMAGE_OUTPUT_INFO
{
	unsigned long outptr;
	unsigned long len;
	unsigned long dispOffset;
}ImageOutputInfo;

/*
*-------------------------------------------------------------------------------
*
*                       Ԥ����������ͼƬ, add by ctf
*
*-------------------------------------------------------------------------------
*/
//#ifdef _FRAME_BUFFER_
#if (BUFFER_MAX_NUM >= 3)
#define IMAGE_DEC_ADVANCE
#endif

#ifdef IMAGE_DEC_ADVANCE
typedef enum
{
    IMAGE_DEC_DECODING,     //���ڽ���
    IMAGE_DEC_STOP,         //�������
    IMAGE_DEC_UNSUPPORTED,    //��ʽ��֧��
    IMAGE_DEC_WAITING       //�ȴ�����
}IMAGE_DEC_ADVANCE_STATE;

typedef enum
{
    MENU_KEY,
    FFD_KEY,    //next picture.
    FFW_KEY,    //previous picture.
    PLAY_KEY    //automatic scan
}KEY_INFO;

EXT IMAGE_DEC_ADVANCE_STATE CurImageDecState;
EXT IMAGE_DEC_ADVANCE_STATE NextImageDecState;
EXT IMAGE_DEC_ADVANCE_STATE PreImageDecState;

EXT KEY_INFO gCurKeyInfo;
EXT UINT16 gCurDisFrameIndex;       //��ǰ��Ļ��ʾ��framebuffer
EXT UINT16 gUnsupportFrameIndex;
EXT UINT16 gIsContinueDecode;       //�Ƿ����������һ�Ż���һ��ͼƬ

EXT uint32 gMarkSysTickCounter;     //����play������һ���Զ�������һ��ͼƬ��ʱ����
EXT uint32 gCurImageSlideShowTick;  //��������ͼƬ֮���ʱ����

EXT UINT16 gIsDisNextImage;         //ˢ����һ��ͼƬ��־
EXT UINT16 gIsDisPreImage;          //ˢ����һ��ͼƬ��־
EXT UINT16 gClearLCD;

EXT TASK_ARG NextImageTaskArg;      //��¼��һ��ͼƬ�ļ���ʽ��֧�ֵ���Ϣ
EXT TASK_ARG PreImageTaskArg;       //��¼��һ��ͼƬ�ļ���ʽ��֧�ֵ���Ϣ
#endif
/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/
EXT int ImageDecState;//decode status--head, body,finish
EXT int ImageSlideState;//the status of slide.
_ATTR_IMAGE_BSS_
EXT BOOL  PicDirection;
_ATTR_IMAGE_BSS_
EXT UINT16  ImageErrorNum;
EXT uint32 ImageMaxWidth;
EXT uint32 ImageMaxHeight;
EXT uint32 ImageLeft;
EXT uint32 ImageTop;

/*
    image memset
*/
void ImageMemSet(uint32 *trg, int data, uint32 len);

/*
--------------------------------------------------------------------------------

   Functon Declaration

--------------------------------------------------------------------------------
*/
EXT void    PictureInit(void *pArg);
EXT UINT32  PictureService(void);
EXT void    PictureDeInit(void);
EXT BOOLEAN ImageDecodeProc(MSG_ID id,void * msg);

//slide state
EXT BOOLEAN ImageIsSliding(void);
EXT BOOLEAN ImageIsPause(void);

//dec state
EXT BOOLEAN ImageIsStart(void);
EXT BOOLEAN ImageIsDecoding(void);
EXT BOOLEAN ImageIsStop(void);

#ifdef IMAGE_DEC_ADVANCE
EXT BOOLEAN CurImageIsDecoding(void);
EXT BOOLEAN NextImageIsDecoding(void);
EXT BOOLEAN PreImageIsDecoding(void);

EXT BOOLEAN CurImageIsStop(void);
EXT BOOLEAN NextImageIsStop(void);
EXT BOOLEAN PreImageIsStop(void);

EXT BOOLEAN CurImageIsUnSupported(void);
EXT BOOLEAN NextImageIsUnSupported(void);
EXT BOOLEAN PreImageIsUnSupported(void);

EXT BOOLEAN NextImageIsWaiting(void);
EXT BOOLEAN PreImageIsWaiting(void);

EXT void AdvanceDecVariableInit(void);
#endif
/*
--------------------------------------------------------------------------------

  Description:  image thread.

--------------------------------------------------------------------------------
*/

#ifdef _IN_IMAGE_CONTROL_
EXT THREAD PictureThread = {

    NULL,
    NULL,

    PictureInit,
    PictureService,
    PictureDeInit,

    NULL
};
#else
EXT THREAD PictureThread;
#endif

/*
********************************************************************************
*
*                         End of PictureControl.h
*
********************************************************************************
*/

#endif


