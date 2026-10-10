/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\Common\pCODECS.c
Desc    : 

Author  : Vincent Hsiung
Date    : 2009-01-08
Notes   :

$Log    :
* 
*
*/
/****************************************************************/
#pragma arm section code = "ImageContrlCode", rodata = "ImageContrlCode", rwdata = "ImageContrlData", zidata = "ImageContrlBss"
#include "../ImageConfig.h"
#include "image_globals.h"

#ifdef _PICTURE_

//global variable.
int ImageCurrentCodec;

//shoud be keep consist with IOCTL enum define in file image_globals.h.
static unsigned long (*ImageCodecPFn[IMAGE_NUMCODECS])(unsigned long ulSubFn,
                                                   unsigned long ulParam1,
                                                   unsigned long ulParam2,
                                                   unsigned long ulParam3) =
{
#ifdef JPG_DEC_INCLUDE
		JPGFunction,
#endif
#ifdef BMP_DEC_INCLUDE
		BMPFunction,
#endif		   
};

unsigned long ImageCodecOpen(unsigned long ulCodec, unsigned long ulFlags)
{
    unsigned long ulRet;
    
    // Pass the open request to the entry point for the codec.
    ulRet = (ImageCodecPFn[ImageCurrentCodec])(IMAGE_SUBFN_CODEC_OPEN_DEC, 0, 0, ulFlags);
        
    // Return the result to the caller.
    return(ulRet);
}

unsigned long ImageCodecDecode(void)
{
    return((ImageCodecPFn[ImageCurrentCodec])(IMAGE_SUBFN_CODEC_DECODE, 0, 0, 0));
}
unsigned long ImageCodecClose(void)
{
    unsigned long ulRet;
    // Pass the close request to the entry point for the specified codec.
    ulRet = (ImageCodecPFn[ImageCurrentCodec])(IMAGE_SUBFN_CODEC_CLOSE, 0, 0, 0);
    
    return(ulRet);
}

unsigned long ImageCodecGetCaptureBuffer(short *ppsBuffer, long *pOffset)
{
    // Pass the get capture buffer request to the entry point for the specified
    // codec.
    return((ImageCodecPFn[ImageCurrentCodec])(IMAGE_SUBFN_CODEC_GETBUFFER,(unsigned long)ppsBuffer,
            (unsigned long)pOffset, 0));
}
unsigned long ImageCodecGetLen(long *plLength)
{
    // Pass the get capture buffer request to the entry point for the specified
    // codec.
    return((ImageCodecPFn[ImageCurrentCodec])(IMAGE_SUBFN_CODEC_GETLENGTH,(unsigned long)plLength,
            0, 0));
}

unsigned long ImageCodecGetResolution(long *plWidth, long *plHeight)
{
    return((ImageCodecPFn[ImageCurrentCodec])(IMAGE_SUBFN_CODEC_GETRESOLUTION,(unsigned long)plWidth,
            (unsigned long)plHeight, 0));
}

unsigned long ImageCodecGetOutputWH(long *plOutputWidth, long *plOutputHeight, long *plFirstOffset)
{
    return((ImageCodecPFn[ImageCurrentCodec])(IMAGE_SUBFN_CODEC_GETOUTPUTWH,(unsigned long)plOutputWidth,
            (unsigned long)plOutputHeight, (unsigned long)plFirstOffset));
}

unsigned long ImageCodecEncode(void)
{
    // Pass the encode request to the entry point for the specified codec.
    return((ImageCodecPFn[ImageCurrentCodec])(IMAGE_SUBFN_CODEC_ENCODE, 0, 0, 0));
}

#endif

#pragma arm section code

