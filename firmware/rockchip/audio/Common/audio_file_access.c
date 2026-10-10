/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:   audio_file_access.c
*
* Description:  Audio File Operation Interface
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung    2009-01-08         1.0
*    desc:    ORG.
********************************************************************************
*/

#include "audio_main.h"
#include "filesys/file.h"   /* OpenFileInfo, FileWrite, FileRead */
#include "audio_file_access.h"
#include <stdio.h>
#include <string.h>
#include "fsinclude.h"
#include "File.h"

/*
*-------------------------------------------------------------------------------
*
*                           type define
*
*-------------------------------------------------------------------------------
*/
typedef unsigned int size_t;


extern size_t   (*RKFIO_FOpen)();   /* defined in bbsystem/audio_file_access2.c */
extern size_t   (*RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *);   /* defined in bbsystem/audio_file_access2.c */
extern int      (*RKFIO_FSeek)(long int /*offset*/, int /*whence*/ , FILE * /*stream*/);   /* defined in bbsystem/audio_file_access2.c */
long int (*RKFIO_FTell)(FILE * /*stream*/);
extern size_t   (*RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);   /* defined in bbsystem/audio_file_access2.c */
unsigned long (*RKFIO_FLength)(FILE *in /*stream*/);
extern int      (*RKFIO_FClose)(FILE * /*stream*/);   /* defined in bbsystem/audio_file_access2.c */
extern int (*RKFIO_FEof)(FILE *);   /* defined in bbsystem/audio_file_access2.c */

FILE *pRawFileCache, *pFlacFileHandleBake, *pAacFileHandleSize, *pAacFileHandleOffset;

/*
*-------------------------------------------------------------------------------
*
*                           AudioFile Buffer define
*
*-------------------------------------------------------------------------------
*/


/*
--------------------------------------------------------------------------------
  Function name : File access interface
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                                    2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
unsigned long RKFLength(FILE *in)
{
    return (OpenFileInfo[(int)in].FileSize); // modified by huweiguo, 09/04/11

}

unsigned long RKFTell(FILE *in)
{
    return (OpenFileInfo[(int)in].Offset);
}

void RKFileFuncInit(void)
{
    RKFIO_FOpen   = FileOpen;
    RKFIO_FLength = RKFLength;
    RKFIO_FRead   = FileRead;
    RKFIO_FWrite  = FileWrite;
    RKFIO_FSeek   = FileSeek;
    RKFIO_FTell   = RKFTell;
    RKFIO_FClose  = FileClose;
    RKFIO_FEof    = FileEof;
}

 _ATTR_FLACDEC_TEXT_
int FLAC_FileSeekFast(int offset, int clus, FILE *in)
{
    OpenFileInfo[(int)in].Offset = offset;
    OpenFileInfo[(int)in].Clus   = clus;
    return 0;
}

_ATTR_FLACDEC_TEXT_
int FLAC_FileGetSeekInfo(int *pOffset, int *pClus, FILE *in)
{
    *pOffset = OpenFileInfo[(int)in].Offset;
    *pClus   = OpenFileInfo[(int)in].Clus;
    return 0;
}

/*
********************************************************************************
*
*                         End of Audio_file_access.c
*
********************************************************************************
*/



