/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   audio_main.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-8          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _IMAGE_MAIN_H_
#define _IMAGE_MAIN_H_

#include "SysConfig.h"

/************************************************************
1. Select any IMAGE CODEC plug-in
*************************************************************/
#ifdef _JPG_DECODE_
#define JPG_DEC_INCLUDE
#define THUMB_DEC_INCLUDE
#endif
#ifdef _BMP_DECODE_
#define BMP_DEC_INCLUDE
#endif

#endif		// _AUDIO_IMAGE_H_

