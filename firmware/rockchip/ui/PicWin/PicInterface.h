/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   PicInterface.h
*
* Description:
*
* History:      <author>          <time>        <version>
*              yangwenjie         2009-3-2          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _PICINTERFACE_H_
#define _PICINTERFACE_H_

#undef  EXT
#ifdef _IN_PICINTERFACE_
#define EXT
#else
#define EXT extern
#endif


#define     PIC_PATH_NAME_LENGTH            (2+MAX_DIR_DEPTH*(11+1)+4)  //unit :byte
#define     PIC_FILE_NAME_LENGTH             12                          //unit :byte

/*
--------------------------------------------------------------------------------

                        Struct Define

--------------------------------------------------------------------------------
*/

typedef struct
{
    //output
    UINT32  FirstOffset;
    UINT32  dispOffset;//refresh position
    UINT32  *pPicBuf;
    UINT32  Len;
    UINT32  Width;          //图片实际宽
    UINT32  Height;         //图片实际高
    UINT32  OutputWidth;    //缩放后宽
    UINT32  OutputHeight;   //缩放后高
    //input
    UINT8   FilePath[PIC_PATH_NAME_LENGTH];                // picture file path
    UINT8   FileName[PIC_FILE_NAME_LENGTH];                // picture short file name
    UINT16  LongFileName[MAX_FILENAME_LEN];    // picture long file name
    UINT16  TotalFiles;     // current direction or total file number of diak

}PIC_FILE_INFO;
/*
--------------------------------------------------------------------------------

                        Variable Define

--------------------------------------------------------------------------------
*/
#define     gPicAutoBrowserTime             gSysConfig.PicConfig.AutoPlayTime//automatic skim time
#define     gPicAutoBrowserOn               gSysConfig.PicConfig.AutoPlaySwitch//automatic skim switch



EXT PIC_FILE_INFO       PicFileInfo;                   //get file information from resource


/*
********************************************************************************
*
*                         End of  PicInterface.h
*
********************************************************************************
*/
#endif
