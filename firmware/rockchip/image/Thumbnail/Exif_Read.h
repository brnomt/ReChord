/* ************************************************ */
/*  Exif_Read.h                                     */
/*  2015/4/27                                       */
/*  by Yuki.KIshimoto                               */
/* ************************************************ */


#ifndef EXIF_H_
#define EXIF_H_ "0.01"


#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS         /* Visual Studio 特有 */
#endif

#include <stdio.h>
#include <string.h>



/* ******************** */
/* IFD                  */
/* Image File Directory */
/* ******************** */
typedef enum {
	IFD_UNKNOWN = 0,
	IFD_0TH,                /* 主画像情報 */
	IFD_1ST,                /* サムネイル情報 */
	IFD_EXIF,               /* 撮影情報 */
	IFD_GPS,                /* GPS情報 */
	IFD_IO                  /* 互換性情報 */
} IFD_TYPE;

/* ******************** */
/* TAG TYPE             */
/* 型情報               */
/* ******************** */
typedef enum {
	TYPE_BYTE = 1,
	TYPE_ASCII,
	TYPE_SHORT,
	TYPE_LONG,
	TYPE_RATIONAL,
	TYPE_SBYTE,
	TYPE_UNDEFINED,
	TYPE_SSHORT,
	TYPE_SLONG,
	TYPE_SRATIONAL
} TAG_TYPE;

/* ******************** */
/* TAG                  */
/* TAG　構造体          */
/* ******************** */
typedef struct _tagNodeInfo TagNodeInfo;
struct _tagNodeInfo {
	unsigned short tagId;    // tag ID (e.g. TAG_Model = 0x0110)
	unsigned short type;     // data Type (e.g. TYPE_ASCII = 2)
	unsigned int count;      // count of the data
	unsigned int *numData;   // numeric data array
	unsigned char *byteData; // byte data array
	unsigned short error;    // 0: no error 1: parse error
};

typedef struct {
	unsigned short tag;
	unsigned short type;
	unsigned int count;
	unsigned int offset;
} TAG_FIELD;


// Tag IDs
// 0th IFD, 1st IFD, Exif IFD
#define TAG_ImageWidth                   0x0100
#define TAG_ImageLength                  0x0101
#define TAG_PixelXDimension              0xA002
#define TAG_PixelYDimension              0xA003
#define TAG_Orientation                  0x0112
#define TAG_DateTimeOriginal             0x9003
#define TAG_ColorSpace                   0xA001
#define TAG_JPEGInterchangeFormatLength  0x0202
#define TAG_StripOffsets                 0x0111
#define TAG_RowsPerStrip                 0x0116
#define TAG_StripByteCounts              0x0117
#define TAG_JPEGInterchangeFormat        0x0201
#define TAG_ExifIFDPointer               0x8769


#endif  // EXIF_H_

