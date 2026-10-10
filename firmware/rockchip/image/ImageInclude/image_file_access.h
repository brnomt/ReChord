/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   audio_file_access.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             Vincent Hsiung     2009-1-8          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _IMAGE_FILE_ACCESS_H_
#define _IMAGE_FILE_ACCESS_H_

#include <stdio.h>

#define RKNANO_FS

#define SEEK_SET 0 /* start of stream (see fseek) */
#define SEEK_CUR 1 /* current position in stream (see fseek) */
#define SEEK_END 2 /* end of stream (see fseek) */
#define NULL 0

typedef unsigned int size_t;

extern size_t   (*IMAGE_RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;
extern int      (*IMAGE_RKFIO_FSeek)(long int /*offset*/, int /*whence*/ ,FILE * /*stream*/);
extern long int (*IMAGE_RKFIO_FTell)(FILE * /*stream*/);
extern size_t   (*IMAGE_RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);
extern unsigned long (*IMAGE_RKFIO_FLength)(FILE *in /*stream*/);
extern int      (*IMAGE_RKFIO_FClose)(FILE * /*stream*/);
extern FILE *pImageFileHandle;
#endif
