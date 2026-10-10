/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                         All rights reserved.
*
* File Name：   RecordWin.h
*
* Description:
*
* History:      <author>          <time>        <version>
*                WangBo           2009-4-10          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _RECORDWIN_H_
#define _RECORDWIN_H_

#undef  EXT
#ifdef _IN_RECORDWIN_
#define EXT
#else
#define EXT extern
#endif


/******************************************************************************/
/*                                                                            */
/*                          Macro Define                                      */
/*                                                                            */
/******************************************************************************/


//section define
//Record menu permanent code.
#define _ATTR_RECORDWIN_CODE_         
#define _ATTR_RECORDWIN_DATA_         
#define _ATTR_RECORDWIN_BSS_          

//record menu initial code
#define _ATTR_RECORDWIN_INIT_CODE_    
#define _ATTR_RECORDWIN_INIT_DATA_    
#define _ATTR_RECORDWIN_INIT_BSS_     

//record menu auti-initial code
#define _ATTR_RECORDWIN_DEINIT_CODE_  
#define _ATTR_RECORDWIN_DEINIT_DATA_  
#define _ATTR_RECORDWIN_DEINIT_BSS_   

//record menu dispatch code
#define _ATTR_RECORDWIN_SERVICE_CODE_ 
#define _ATTR_RECORDWIN_SERVICE_DATA_ 
#define _ATTR_RECORDWIN_SERVICE_BSS_  

#define  _RECODE_IMAGE_DIAPLAY_
#define  _RECODE_CHAR_DIAPLAY_

//------------------------------------------------------------------------------
// recording interface display coordinate

// recording status MenuStr
#define     STR_RECORD_STATUS_X               0  //X
#define     STR_RECORD_STATUS_Y               54  //Y
#define     STR_RECORD_STATUS_WIDTH           128 //XSize 
#define     STR_RECORD_STATUS_HEIGHT          16  //YSize

//display file name
#define     STR_RECORD_FILENAME_LEFT_X       0  //upper left x coordinate
#define     STR_RECORD_FILENAME_LEFT_Y       90  //upper y left coordinate
#define     STR_RECORD_FILENAME_RIGHT_X      128 //lower right X coordinate
#define     STR_RECORD_FILENAME_RIGHT_Y     84  //lower right y coordinate

// current recording left time
#define     STR_RECORD_REMAIN_TIME_LEFT_X     79
#define     STR_RECORD_REMAIN_TIME_LEFT_Y     100
#define     STR_RECORD_REMAIN_TIME_RIGHT_X    125
#define     STR_RECORD_REMAIN_TIME_RIGHT_Y    110

#ifdef    _RECODE_IMAGE_DIAPLAY_
#define     RECORD_PIC_FIRSTNUM               IMG_ID_RECORD_CURRTIME_BACK//the frist digit that record use digits.
#define     RECORD_PIC_BUF_SIZE               48*12*2 //the record digit size
#define     RECORD_PIC_BUF_NUM                1

_ATTR_RECORDWIN_DATA_ EXT UINT8                RecordDispPicBuf[RECORD_PIC_BUF_NUM][RECORD_PIC_BUF_SIZE];
_ATTR_RECORDWIN_BSS_ EXT PICTURE_INFO_STRUCT   RecCurTimePicInfo;
#endif

#ifdef    _RECODE_CHAR_DIAPLAY_
#define     CHAR_ID_RECORD_CURTIMENUM0         0
#define     CHAR_ID_RECORD_CURTIMENUM10        10    //显示冒号
#define     RECORD_CHAR_BUF_SIZE               64 //录音数字的大小
#define     RECORD_CHAR_BUF_NUM                11

_ATTR_RECORDWIN_DATA_ EXT UINT8                 RecordDispCharBuf[RECORD_CHAR_BUF_NUM][RECORD_CHAR_BUF_SIZE];
#endif


/******************************************************************************/
/*                                                                            */
/*                          Variable Define                                   */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                          Function Define                                   */
/*                                                                            */
/******************************************************************************/

EXT void RecordWinInit(void *pArg);
EXT void RecordWinIntDeInit(void);
EXT void RecordWinMsgInit(void);
EXT void RecordWinMsgDeInit(void);

EXT void RecordWinInit(void *pArg);
EXT void RecordWinDeInit(void);

EXT UINT32 RecordWinService(void);
EXT void RecordWinPaint(void);
EXT UINT32 RecordWinKeyProc(void);


/******************************************************************************/
/*                                                                            */
/*                    Description:  window sturcture definition               */
/*                                                                            */
/******************************************************************************/
#ifdef _IN_RECORDWIN_
_ATTR_RECORDWIN_DATA_ WIN RecordWin =
{

    NULL,
    NULL,

    RecordWinService,             //window service handle function.
    RecordWinKeyProc,             //window key service handle function.
    RecordWinPaint,               //window display service handle function.

    RecordWinInit,                //window initial handle function.
    RecordWinDeInit               //window auti-initial handle function.

};
#else
_ATTR_RECORDWIN_DATA_ EXT WIN RecordWin;
#endif

/*
********************************************************************************
*
*                         End of RecordWin.h
*
********************************************************************************
*/
#endif


