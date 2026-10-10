/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   audio_file_access.c
* 
* Description:  Audio File Operation Interface
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung    2009-01-08         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "AudioConfig.h"

//#include <stdio.h>
//#include <string.h>
#include "fsinclude.h"
#include "File.h"
#pragma arm section code = "ImageContrlCode", rodata = "ImageContrlCode", rwdata = "ImageContrlData", zidata = "ImageContrlBss"

FILE *pImageFileHandle = NULL;

typedef unsigned int size_t;


size_t   (*IMAGE_RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;


int      (*IMAGE_RKFIO_FSeek)(long int /*offset*/, int /*whence*/ , FILE * /*stream*/);


long int (*IMAGE_RKFIO_FTell)(FILE * /*stream*/);


size_t   (*IMAGE_RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);


unsigned long (*IMAGE_RKFIO_FLength)(FILE *in /*stream*/);
int      (*IMAGE_RKFIO_FClose)(FILE * /*stream*/);




void ImageFileFuncInit(void)
{
#if 0//def SIMULATION
    IMAGE_RKFIO_FRead=RKFRead_sim;
    IMAGE_RKFIO_FWrite=RKFWrite_sim;
    IMAGE_RKFIO_FSeek=fseek;
    IMAGE_RKFIO_FTell=ftell;
    //RKFIO_FClose=fclose;
#endif    	
#if 0//fdef MEMFILE_SIM
	RKFIO_FRead = RKmem_FRead;
	RKFIO_FWrite = RKmem_FWrite;
#else
	IMAGE_RKFIO_FRead = FileRead;
	//RKFIO_FWrite = FileWrite;  
	IMAGE_RKFIO_FSeek = FileSeek;
//	IMAGE_RKFIO_FTell = RKFTell;
	IMAGE_RKFIO_FClose = FileClose;

#endif
}
#pragma arm section code
