/******************************************************
Copyright(C)2007,Rockchip Co., Ltd. All Rights Reserved.
File:  bmphead_32bits.h
Desc:
Note:
Author:  evan wu
$Log:
Revision  1.2 2007/01/09 evan wu

******************************************************/

#ifndef BMP_DEC_H
#define BMP_DEC_H

#include "../ImageInclude/image_file_access.h"

//#include "stdio.h"
//#include "malloc.h"

//#define OUT_PUT_RGB888
#define bmp_rk_nano_bord

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

/*********************************************************************
INCLUDE
************************************************************************/
typedef unsigned char   INT8U;
typedef signed  char   INT8S;
typedef unsigned short  INT16U;
typedef signed  short  INT16S;
typedef int       INT32S;
typedef unsigned int   INT32U;

typedef unsigned long   L32U;
typedef signed  long  L32S;

typedef unsigned char   BYTE;
typedef unsigned long ULONG;

#ifdef OUT_PUT_RGB888
typedef unsigned int targbmodel;
#else
typedef unsigned short targbmodel;
#endif
typedef unsigned int LCD_RGBDATA;
typedef struct
{
    int Comp_1;
    int Comp_2;
    int Comp_3;
    int Offset;
    int x;
    int y;
    int Src_Fmt;
}IM_PIX_INFO;  
typedef struct TPicRegion //һ����ɫ������������,���ڲ�������
{
    targbmodel* pdata;     //��ɫ�����׵�ַ
    long     byte_width;//һ�����ݵ���������(�ֽڿ���)
    long     width;//���ؿ���
    long     height;//���ظ߶�
}tpicregion;  
/*********************************************************************
DEFINE
************************************************************************/

#define BMP_TYPE            0x4d42 // �����ļ���ʽ
#define BI_RGB            0     // No compression
#define BI_RLE8           1     // RLE8 compression (256 colors)
#define BI_RLE4           2     // RLE4 compression (16 colors)
#define BI_BITFIELDS      3     // BITFIELDS compression (16 bits and 32 bits)
#define ERROR_MARKER     -4
#define ROW_EXCHANGE_ERROR   -100


#define READ_TEMP_LEN   512
#define BMP_READ_MAX_ONCE  10*1024
#define BMP_FILE    FILE
#define RGB565_MARK_R   0xF800
#define RGB565_MARK_G   0x07E0
#define RGB565_MARK_B   0x001F
#define RGB555_MARK_R   0x7C00
#define RGB555_MARK_G   0x03E0
#define RGB555_MARK_B   0x001F

#define MIN(x,y)     ((x)>(y)?(y):(x))

#define FREE(a) 
//#define MALLOC malloc
#define TRUE 1
#define FALSE 0
//#define _DEBUG_
//#define DebugRLE
/*************************************************************************
STRUCT
**************************************************************************/

#ifdef BOARD
typedef __packed struct tagBITMAPINFOHEADER
{
    INT32S  biSize;
    INT32S  biWidth;
    INT32S  biHeight;
    INT16S  biPlanes;
    INT16S  biBitCount;
    INT32S  biCompression;
    INT32S  biSizeImage;
    INT32S  biXPelsPerMeter;
    INT32S  biYPelsPerMeter;
    INT32S  biClrUsed;
    INT32S  biClrImportant;
} BITMAPINFOHEADER;

typedef  __packed  struct tagBITMAPFILEHEADER
{
    INT16S  bfType;
    INT32S  bfSize;
    INT16S  bfReserved1;
    INT16S  bfReserved2;
    INT32S  bfOffBits;
} BITMAPFILEHEADER;
#else
#pragma pack(push)
#pragma pack(1)
typedef struct tagBITMAPINFOHEADER
{
    INT32S  biSize;
    INT32S  biWidth;
    INT32S  biHeight;
    INT16S  biPlanes;
    INT16S  biBitCount;
    INT32S  biCompression;
    INT32S  biSizeImage;
    INT32S  biXPelsPerMeter;
    INT32S  biYPelsPerMeter;
    INT32S  biClrUsed;
    INT32S  biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER
{
    INT16S  bfType;
    INT32S  bfSize;
    INT16S  bfReserved1;
    INT16S  bfReserved2;
    INT32S  bfOffBits;
} BITMAPFILEHEADER;
#pragma pack(pop)
#endif

typedef struct _RGBQUAD
{
    INT8U    rgbBlue;
    INT8U    rgbGreen;
    INT8U    rgbRed;
    INT8U    rgbReserved;
} RGBQUAD;

typedef struct tagMARK
{
    INT32U    BmpReservedMark;
    INT32U    BmpRedMark;
    INT32U    BmpGreenMark;
    INT32U    BmpBlueMark;
}BMPMARK;

typedef struct tagBMP
{
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    RGBQUAD    *bmiColors;
    INT32U    bmnumcolors;
    BMPMARK    BmpMark;
    INT8U    *BmpOutBuf;
    INT32S    bytes_per_line;
    INT32U    FileOffset;
    INT8U    *data;
}BMP;

typedef struct tagBMP* pBMP;


typedef struct
{
    unsigned int (*read_func)(void *pBuff, unsigned int size, unsigned int cnt,  void *Handle);
    unsigned int (*seek_func)(void *stream,   long offset,   int origin);
#ifdef OUT_PUT_RGB888	
    INT32U (*write_func)(IM_PIX_INFO out_info);
#else
    INT16U (*write_func)(IM_PIX_INFO out_info);
#endif
}BMP_CALLBACKS_DECODE;

typedef struct TagBmpOutFactor
{
    FILE* BmpFile;
    unsigned long ImageWInBuf;//��Ч�����
    unsigned long ImageHInBuf;//��Ч�����
    unsigned char *ImageBufAddr;
    unsigned long dsty;
}BMPOUTFACTOR;

typedef struct TagBmpFactor
{
    INT8U *ReadBuf;
    INT32S height;
    float  BytesPerPixs;
    INT32S ReadBufSize;
    tpicregion Dst;
    unsigned long dstxInt;
    unsigned long dstyInt;
    double zoomx;
    double zoomy;
    INT32S x;
    INT32S y;
}BMPFACTOR;

/*************************************************************************
LOCAL VARIABLES
**************************************************************************/
extern INT32S BMP_Decode(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int ScreenWidth, int ScreenHigh);
extern INT32S BMP_ThumbNail_Decode(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight);
extern INT8U BMP_InitHead(BMP_CALLBACKS_DECODE BMP_callbacks_decode,BMP *bmp,BMPOUTFACTOR *BmpOutFactor,int BufWidth, int BufHeight);
extern INT8U BMP_calc_output(BMP_CALLBACKS_DECODE BMP_callbacks_decode,BMP *bmp,BMPOUTFACTOR *BmpOutFactor,int BufWidth, int BufHeight);
extern INT32U BMP_CustomOutput_32BitRGB888(IM_PIX_INFO out_info);
extern INT16U BMP_CustomOutput_16BitRGB565(IM_PIX_INFO out_info);

extern unsigned long BMPFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                     unsigned long ulParam2, unsigned long ulParam3);

#endif

