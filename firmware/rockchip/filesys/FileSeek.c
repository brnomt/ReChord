/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Flash.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-10-21          1.0
*    desc:    ORG.
********************************************************************************
*/
#define  IN_FILESEEK
#include "FsInclude.h"

/*
--------------------------------------------------------------------------------
  Function name : uint8 FileSeekRefSet(HANDLE Handle) 
  Author        : ZHengYongzhi
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
IRAM_FAT
uint8 FileSeekRefSet(HANDLE Handle) 
{
    uint16 i;
    uint32 SeekStep;
	uint8  Rt;
    _TFILESEEK CurFileSeek;
	MY_FILE *fp;
    
	Rt = PARAMETER_ERR;
	if (Handle >= 0 && Handle < MAX_OPEN_FILES)
    {
		fp = OpenFileInfo + Handle;
        SeekStep = fp->FileSize / FILESEEKNUM;
        
        //save cur file offset info
        CurFileSeek.FileClus = fp->Clus;
        CurFileSeek.FileOffset = fp->Offset;
        
        FileSeek(0,SEEK_SET,Handle);
        
        for(i = 0 ; i < FILESEEKNUM ; i++)
        {
            FileSeekData[i].FileClus   = fp->Clus;
            FileSeekData[i].FileOffset = fp->Offset;
            
            FileSeek(SeekStep, SEEK_CUR, Handle);
        }
        
        //reload cur file offset info
        fp->Clus   = CurFileSeek.FileClus;
        fp->Offset = CurFileSeek.FileOffset;
        
        Rt = RETURN_OK;
	}
    
	return (Rt);
}

/*
--------------------------------------------------------------------------------
  Function name : uint8 FileSeekFast(HANDLE Handle , uint32 Offset)
  Author        : ZHengYongzhi
  Description   : 
                  
  Input         : 
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
IRAM_FAT
uint8 FileSeekFast(HANDLE Handle , uint32 Offset) 
{
    uint16 i;
	MY_FILE *fp;
    
	if (Handle >= 0 && Handle < MAX_OPEN_FILES)
    {
		fp = OpenFileInfo + Handle;
        
        //Find File Seek Info
        for(i = 1 ; i < FILESEEKNUM; i++)
        {
            if(FileSeekData[i].FileOffset > Offset)
            {
                break;
            }
        }
        
        //Load File Seek Info
        i--;
        fp->Clus   = FileSeekData[i].FileClus;
        fp->Offset = FileSeekData[i].FileOffset;
        
        return(FileSeek(Offset - FileSeekData[i].FileOffset,SEEK_CUR,Handle));
	}
    return(OK);
}

/*
********************************************************************************
*
*                         End of FileSeek.c
*
********************************************************************************
*/
