/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   LcdInterface.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef  _LCDINTERFACE_H_
#define  _LCDINTERFACE_H_

#include "ModuleInfoTab.h"

#undef  EXT
#ifdef _IN_LCDINTERFACE_
#define EXT
#else
#define EXT extern
#endif

EXT _ATTR_SYS_DATA_   LCD_DRIVER_TABLE_T SystemLcdPara;



typedef enum
{
    DIRECTION_UP = (uint16)0,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_MAX
} BUFFER_SHIFT_DIRECTION;

/*
--------------------------------------------------------------------------------

  Description:  LCD Macro define

--------------------------------------------------------------------------------
*/
#define FONT12_CHAR_SIZE 64
#define FONT16_CHAR_SIZE 64

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
#ifdef _FRAME_BUFFER_
void Lcd_WriteRAM_Prepare(void);
//void Lcd_Init(void);
void Lcd_SendData(UINT16 data);
void Lcd_SetWindow(uint16 x0,uint16 y0,uint16 x1,uint16 y1);
void Lcd_Clear(uint16 color);
void Lcd_BuferTranfer (void);
void LCDDEV_SetPixel(UINT16 x, UINT16 y, UINT16 data);
void LCD_Buffer_Display1(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1);
void LCD_ClrSrc(void);
void LCD_ClrRect(int x0, int y0, int x1, int y1);
void LCDDEV_FillRect(int x0, int y0, int x1, int y1);

int LCD_Set_Current_BufferFrame_Index(UINT16 index);
int LCD_Copy_FrameBuffer(UINT16 src_index,UINT16 dest_index);
int LCD_Buffer_Gray_Scale(UINT16 frame_index,UINT16 level);
void LCD_Shift_BufferFrame(UINT16 frame_index,BUFFER_SHIFT_DIRECTION  direction, UINT16 distance);
void LCD_Shift_Window(UINT16 frame_index,BUFFER_SHIFT_DIRECTION  direction, UINT16 distance,UINT16 x0,UINT16 y0,UINT16 x1,UINT16 y1);
void LCD_Buffer_Section_Refresh(UINT16 pYim,UINT16 pYmax);

void LCD_Create_Upper_Later();
void LCD_Back_To_Under_Layer();
UINT16 LCD_Get_Current_Index();
void Lcd_FreshEn (void);
void Lcd_FreshDisable(void);

#else
//#define _DMA_LCD_
//#define LcdGetResourceInfo(Addr,pData,Length)   (ReadResourceData(Addr,pData,Length))
#define Lcd_SendData(data)                      (SystemLcdPara.pLcd_SendData(data))
#define Lcd_SetWindow(x0,y0,x1,y1)              (SystemLcdPara.pLcd_SetWindow(x0,y0,x1,y1))
#define Lcd_Clear(color)                        (SystemLcdPara.pLcd_Clear(color))
#define Lcd_BuferTranfer(x0,y0,x1,y1,pSrc)        (SystemLcdPara.pLcd_DMATranfer(x0,y0,x1,y1,pSrc))
//#define LcdStandby()                            (SystemLcdPara.pLcd_Standby())
//#define LcdWakeUp()                             (SystemLcdPara.pLcd_WakeUp())
#define Lcd_WriteRAM_Prepare()                  (SystemLcdPara.pLcd_WriteRAM_Prepare())
#define MP4_LCD_Init()                          (SystemLcdPara.pLcd_MP4_Init())
#define MP4_LCD_DeInit()                        (SystemLcdPara.pLcd_MP4_DeInit())
#define LCDDEV_SetPixel(x, y, data)             (SystemLcdPara.pLcd_SetPixel(x,y,data))
#define LCD_Buffer_Display1(x0,y0,x1,y1)        (SystemLcdPara.pLcd_Buffer_Display1(x0,y0,x1,y1))
#define LCD_ClrSrc()                            (SystemLcdPara.pLcd_ClrSrc())
#define LCD_ClrRect(x0,y0,x1,y1)                (SystemLcdPara.pLcd_ClrRect(x0,y0,x1,y1))
#define LCDDEV_FillRect(x0,y0,x1,y1)            (SystemLcdPara.LCDDEV_FillRect(x0, y0,x1,y1))

#endif

#define DisplayPictureFromBuffer(x,y,xsize,ysize,pBuf) (LCD_DrawBmp(x, y, xsize, ysize, 16, pBuf))

#define DisplayPictureFromBufferPart(x,y,xsize,ysize,pBuf,yOffset) (DisplayPictureFromBuffer(x, y + yOffset, xsize, ysize - yOffset, pBuf + yOffset * xsize))
#define DispPictureFromBuffer(PicInof,pBuf,Offset,xOffset,yOffset) (DisplayPictureFromBuffer((PicInof->x + xOffset), (PicInof->y + yOffset), PicInof->xSize, PicInof->ySize, (uint16*)((uint16)pBuf + Offset)))

EXT void Lcdchang(UINT16*pSrc,UINT16 len);
EXT void PowerOnLcdinit(void);
EXT void LcdGetResourceData(UINT32 Addr,UINT8*pData,UINT16 Length);
EXT void LcdGetResourceInfo(UINT32 Addr,UINT8*pData,UINT16 Length);

EXT void Lcd_WriteReg(UINT16 addr, UINT16 data);
//EXT void Lcd_SendData(UINT16 data);
EXT void Lcd_Init(void);
//EXT void Lcd_Clear(UINT16 color);
//EXT void Lcd_SetWindow(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1);
EXT int32 Lcd_DMATranfer (UINT16 x0,UINT16 y0,UINT16 x1,UINT16 y1,UINT16 *pSrc);
EXT int32 Lcd_DMATranfer32 (UINT16 x0,UINT16 y0,UINT16 x1,UINT16 y1,UINT32 *pSrc);
//EXT void Lcd_MP4_Init(void);
//EXT void Lcd_MP4_Deinit(void);
//EXT void LCDDEV_SetPixel(UINT16 x, UINT16 y, UINT16 data);
//EXT void LCD_Buffer_Display1(unsigned int x0,unsigned int y0,unsigned int x1,unsigned int y1);
//EXT void LCD_ClrSrc(void);
//EXT void LCD_ClrRect(int x0, int y0, int x1, int y1);

extern void LCD_GPIO_Init(void);
/*
********************************************************************************
*
*                         End of LcdInterface.h
*
********************************************************************************
*/
#endif
