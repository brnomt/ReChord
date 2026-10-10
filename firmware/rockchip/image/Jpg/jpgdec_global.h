/************************************************************************
 *       Smart Jpeg Decoder
 *
 * File:
 *  global.h
 *
************************************************************************/

#ifndef JPGDEC_GLOBAL_H
#define JPGDEC_GLOBAL_H

#include "pJPG.h"

#define int8 char
#define schar char
#define uint8 unsigned char
#define int16 short
#define uint16 unsigned short
#define ushort unsigned short
#define uint unsigned int
#define int32 long
#define uint32 unsigned long
#define ulong unsigned long
#define uchar unsigned char
#define NULL 0

#define TRUE (1)
#define FALSE (0)

#define ARM_PLATFORM  
//#define RGB16BITS
#define MACROROW_DATA_LEN (24*320)    //(long)24*SCR_LENGTH  //SCR_LENGTH*2/16*6*64/2

#ifdef  RGB16BITS
typedef unsigned short  rgbtype; //RGB的输出为16位,含565和555格式
#else
typedef unsigned long  rgbtype;  //RGB输出为32位或24位
#endif


#define JPGD_MAX_HEIGHT   8000  //2048
#define JPGD_MAX_WIDTH   8000  //2048

// Increase this if you increase the max width!
#define JPGD_MAXBLOCKSPERROW 8000//6000/8*6
// 支持的最大的LCD分辨率
#define MAX_SCR_LENGTH 800
#define MAX_SCR_HEIGHT 600

// 当前使用的LCD分辨率
#define SCR_LENGTH (gJpgProOutput->OutputW) //128//320//800 
#define SCR_HEIGHT (gJpgProOutput->OutputH) //160//240//480

//#define JPG_PROGRESSIVE
#define MINUS_ROOM
#define MAX_IMAGE_X_SIZE 8000  //2048
#define MAX_IMAGE_Y_SIZE 8000  //2048
//#define Y_WORD
//#define USE_ASSEMBLY_OPT
#define skip_ac_scan

#endif




