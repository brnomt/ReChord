/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   audio_globals.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-8          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _IMAGE_GLOBALS_H_
#define _IMAGE_GLOBALS_H_

#include "image_main.h"
//****************************************************************************
// The following values are the IOCTLs which are sent to the individual codec
// drivers.
//****************************************************************************
#define IMAGE_MAX_OUTPUT_WIDTH    160//屏的宽度
#define AUTO_ADAPT_TO_LCD   //关闭，始终横屏显示
#define IMAGE_MAX(a,b) ((a)>=(b)?(a):(b))
#define IMAGE_MIN(a,b) ((a)<=(b)?(a):(b))

//图片显示方向
enum
{
    IMAGE_HORIZONTLE,
    IMAGE_VERTIAL
};
#if 0
static int  image_max(int a,int b)
{
    if(a >= b)
        return a;
    else
        return b;
}
static int  image_max(int a,int b)
{
    if(a >= b)
        return a;
    else
        return b;
}
#endif
enum
{
    IMAGE_SUBFN_CODEC_GETNAME,
    IMAGE_SUBFN_CODEC_GETARTIST,
    IMAGE_SUBFN_CODEC_GETTITLE,
    IMAGE_SUBFN_CODEC_GETBITRATE,
    IMAGE_SUBFN_CODEC_GETSAMPLERATE,
    IMAGE_SUBFN_CODEC_GETCHANNELS,
    IMAGE_SUBFN_CODEC_GETLENGTH,
    IMAGE_SUBFN_CODEC_GETRESOLUTION,
    IMAGE_SUBFN_CODEC_GETOUTPUTWH,
    IMAGE_SUBFN_CODEC_OPEN_DEC,
    IMAGE_SUBFN_CODEC_GETBUFFER,
    IMAGE_SUBFN_CODEC_DECODE,
    IMAGE_SUBFN_CODEC_ENCODE,
    IMAGE_SUBFN_CODEC_CLOSE,
    IMAGE_SUBFN_CODEC_ZOOM
};
// pJPG.C
extern unsigned long JPGFunction(unsigned long ulSubFn, unsigned long ulParam1, unsigned long ulParam2,
                          unsigned long ulParam3);

//pBMPDEC.c
extern unsigned long BMPFunction(unsigned long ulSubFn, unsigned long ulParam1, unsigned long ulParam2,
                          unsigned long ulParam3);

// From codec.c
///////////////////////////////////////////////////////////////////////////////
extern unsigned long ImageCodecOpen(unsigned long ulCodec, unsigned long ulFlags);
extern unsigned long ImageCodecGetCaptureBuffer(short *ppsBuffer, long *plLength);
extern unsigned long ImageCodecGetLen(long *plLength);
extern unsigned long ImageCodecGetResolution(long *plWidth, long *plHeight);
extern unsigned long ImageCodecGetOutputWH(long *plOutputWidth, long *plOutputHeight, long *plFirstOffset);
extern unsigned long ImageCodecSetBuffer(short *psBuffer);
extern unsigned long ImageCodecDecode(void);
extern int get_output_wh(int *w,int *h,int image_w,int image_h);

//------------------------------------------------------------
enum {
#ifdef JPG_DEC_INCLUDE
    CODEC_JPG_DEC =0,
#endif
#ifdef BMP_DEC_INCLUDE
    CODEC_BMP_DEC,
#endif

    IMAGE_NUMCODECS
};
enum
{
    IMAGE_RGB565,
    IMAGE_RGB888,
    IMAGE_YUV420,
};

//-------------------------------------------------------------

typedef struct
{
    unsigned long  width;
    unsigned long  height;
    //exif info....
}image_info;

typedef short IMAGE_PIXEL;
#endif
