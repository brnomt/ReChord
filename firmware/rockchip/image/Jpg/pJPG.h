#ifndef PJPEG_TYPEDEF_H
#define PJPEG_TYPEDEF_H

#include "../ImageInclude/image_file_access.h"

#define jpg_rk_nano_bord

typedef struct
{
    int (*read_func)(void *Pbuf, int bytes);
} JPEG_CALLBACKS_DECODE;

//jpgpro output data format

typedef struct
{
	//input 
	//unsigned int fhandle;
	FILE* fhandle;
    int OutputFormat;
    unsigned long ptr_output_buf;	
    int OutputW;
    int OutputH;	
	
	//output
    int ValidW;
    int ValidH;
	
	int ImageW;
	int ImageH;
	
	unsigned int CurrentDecLine;
	unsigned short *ScreenBuffer;
	
	unsigned long dispOffset;
} IMAGE_DEC_INFO;
#endif
