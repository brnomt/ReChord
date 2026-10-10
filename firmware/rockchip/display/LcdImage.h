/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  LcdImage.h
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             yangwenjie      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef  _LCDIMAGE_H_
#define  _LCDIMAGE_H_

#undef  EXT
#ifdef _IN_LCDIMAGE_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------
  
                        Macro define  
  
--------------------------------------------------------------------------------
*/
typedef enum 
{
    IMAGE_ID0 = (uint16)0,
    IMAGE_ID_MAX
} IMAGE_INDEX;

#define     IMAGE_RESOURCE_INFO_SIZE         16

    
#define     DATA_BUS_WIDTH                  16   //data bus width definition
/*
--------------------------------------------------------------------------------
  
                        Struct Define
  
--------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------
  
                        Variable Define
  
--------------------------------------------------------------------------------
*/


/*
--------------------------------------------------------------------------------
  
                        Funtion Declaration
  
--------------------------------------------------------------------------------
*/
EXT void LCD_DrawPixel(INT16 x, INT16 y);
EXT void LCD_DrawHLine(INT16 x0, INT16 y, INT16 x1);
EXT void LCD_DrawVLine(INT16 x, INT16 y0, INT16 y1);
EXT void LCD_DrawRect(INT16 x0, INT16 y0, INT16 x1, INT16 y1);
EXT void LCD_FillRect(INT16 x0, INT16 y0, INT16 x1, INT16 y1);
EXT void LCD_DrawBmp(UINT16 x0, UINT16 y0, UINT16 xsize, UINT16 ysize, UINT16 Pixel, UINT16 *pData);
EXT void LCD_DrawBmpVideo(UINT16 x0, UINT16 y0, UINT16 xsize, UINT16 ysize, UINT16 Pixel, UINT16 *pData, UINT16 width);
EXT void GetPictureInfoWithIDNum(UINT16 pictureIDNum, PICTURE_INFO_STRUCT *psPictureInfo);
EXT void DispPictureWithIDNum(UINT16 pictureIDNum);
EXT void DisplayPicture(INT16 x, INT16 y, INT16 xsize, INT16 ysize, unsigned long Flash_Addr);
EXT void DispPictureWithIDNumAndXYoffset(UINT16 pictureIDNum, UINT16 xOffset, UINT16 yOffset);
EXT void DispPictureWithIDNumAndXY(UINT16 pictureIDNum, UINT16 x, UINT16 y);
EXT void DisplayPicture_part(UINT16 pictureIDNum,INT16 x,INT16 y,INT16 yOffset,INT16 ysize);
EXT void DisplayPicture_part_xoffset(UINT16 pictureIDNum, INT16 x, INT16 xOffset, INT16 xsize, INT16 y, INT16 yOffset, INT16 ysize);

/*
********************************************************************************
*
*                         End of LcdImage.h
*
********************************************************************************
*/
#endif

