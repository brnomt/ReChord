/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   LcdImage.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_LCDIMAGE_
#include "fsinclude.h"
#include "LcdInclude.h"
#include "ModuleOverlay.h"


/*
--------------------------------------------------------------------------------
  Function name : void LCD_FillRect(INT16 x0, INT16 y0, INT16 x1, INT16 y1)
  Author        : ZHengYongzhi
  Description   : fill rectangle

  Input         : x0,y0 ���� the upper left coordinate
                  x1,y1 ���� the lower riht coordinate
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_FillRect(INT16 x0, INT16 y0, INT16 x1, INT16 y1)
{
    INT16 i;

    Lcd_SetWindow(x0, y0, x1, y1);

    for (; y0 <= y1 ;y0++)
    {
        i = x0;
        for (; i <= x1; i++)
        {
            Lcd_SendData(LCD_COLOR);
        }
    }
    // LCD_RevertWindow(0, 0, LCD_MAX_XSIZE-1, LCD_MAX_YSIZE-1);
}


/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_SetCharSize(UINT16 size)
  Author        : yangwenjie
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
UINT16 LCD_SetImageIndex(UINT16 Index)
{
    UINT8 ImageIndexBack;
    uint8  FlashBuf[512];
    FIRMWARE_HEADER *pFWHead = (FIRMWARE_HEADER *)FlashBuf;

    ImageIndexBack = LCD_IMAGEINDEX;
    if (LCD_IMAGEINDEX != Index)
    {
        LCD_IMAGEINDEX = Index;
        if (LCD_IMAGEINDEX >= IMAGE_ID_MAX)
        {
            LCD_IMAGEINDEX = 0;
        }

        MDReadData(SysDiskID, 0, 512,FlashBuf);

        ImageLogicAddress = pFWHead->UiInfo.UiInfoTbl[Index].ModuleOffset;
        DEBUG("ImageLogicAddress = 0x%x", ImageLogicAddress);
    }
    return ImageIndexBack;
}

/*
--------------------------------------------------------------------------------
Function name : void GetPictureInfoWithIDNum(UINT16 pictureIDNum, PICTURE_INFO_STRUCT *psPictureInfo)
Author        : anzhiguo
Description   : get structure information of picture resource.

Input         : pictureIDNum  picture ID(it is produced by tool)
*              : *psPictureInfo the picture information resource sturcture.

Return        :
History:     <author>         <time>         <version>
          anzhiguo     2009-2-24         Ver1.0
desc:        ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void GetPictureInfoWithIDNum(UINT16 pictureIDNum, PICTURE_INFO_STRUCT *psPictureInfo)
{
    UINT32 flashAddr;
    int8  hMainFile;

    //32�ǹ̼���ͼƬ��Դͷ��Ϣ�ĳ��� IMAGE_RESOURCE_INFO_SIZE ��ÿ��ͼƬ��Ϣ�ṹ����ռ�õ��ֽ���(16)
    flashAddr = ImageLogicAddress + 32 + ((UINT32)pictureIDNum * IMAGE_RESOURCE_INFO_SIZE);
#if (RES_IN_DEV == 1)
    LcdGetResourceInfo(flashAddr, (UINT8*)psPictureInfo, 16);
#else
    //for SPI Flash Test
    if ((hMainFile = FileOpenW(NULL,L"\\",L"RkNanoImage.uis", "R")) == -1)//���ļ��ɹ������ش���
    {
        DEBUG("Open File ERROR");
    }
    FileSeek((flashAddr - ImageLogicAddress), 0, hMainFile);
    FileRead((UINT8*)psPictureInfo, 16, hMainFile);
    FileClose(hMainFile);
#endif
}

/*
--------------------------------------------------------------------------------
Function name : void DispPictureWithIDNum(UINT16 pictureIDNum)
Author        : anzhiguo
Description   : display picture by its resource id.

Input         : pictureIDNum picture id(it be produced by tool)
*

Return        :
History:     <author>         <time>         <version>
          anzhiguo     2009-2-24         Ver1.0
desc:        ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void DispPictureWithIDNum(UINT16 pictureIDNum)
{
    DispPictureWithIDNumAndXYoffset(pictureIDNum, 0, 0);
}

/*
--------------------------------------------------------------------------------
Function name :void DisplayPicture_part(UINT16 pictureIDNum, INT16 x,INT16 y, INT16 yOffset, INT16 ysize)
Author        : anzhiguo
Description   : display picture by its resource id.just display the part of the picture.
Input         : pictureIDNum:picture id that produced by tool
               y --the y positon offset that origin is lcd display area upper left corner.
               ysize -- the size in y driction.
               yOffset--the offset in picture y direction.
                _ _ _ _ _ _ _ _ _ _ _
               |     ^                |
               |     | x              |
               |<--> v _ _ _ _ _ _    |
               |  y  | ^           |  |
               |     | |yOffset    |  |
               |     |_v___________|  |
               |     |    ^        |  |
               |     |    | ysize  |  |
               |     |____v________|  |
               |_ _ _ _ _ _ _ _ _ _ _ |
Return        :
History:     <author>         <time>         <version>
          anzhiguo     2009-2-24         Ver1.0
desc:        ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void DisplayPicture_part(UINT16 pictureIDNum, INT16 x,INT16 y, INT16 yOffset, INT16 ysize)
{
    UINT32 flashAddr;

    PICTURE_INFO_STRUCT psPictureInfo;

    if (ysize == 0)
        return;
    GetPictureInfoWithIDNum( pictureIDNum, &psPictureInfo);

    if ((ysize + yOffset ) > psPictureInfo.ySize)
        ysize =  psPictureInfo.ySize - yOffset;

    flashAddr = psPictureInfo.offsetAddr + psPictureInfo.xSize * yOffset * 2;

    DisplayPicture(x, y+yOffset, psPictureInfo.xSize, ysize, flashAddr);
}


_ATTR_LCD_CODE_
void DisplayPicture_part_xoffset(UINT16 pictureIDNum, INT16 x, INT16 xOffset, INT16 xsize, INT16 y, INT16 yOffset, INT16 ysize)
{
     UINT32 flashAddr;
    UINT32 i;

     PICTURE_INFO_STRUCT psPictureInfo;

     if(ysize == 0 || xsize == 0)
        return;
     GetPictureInfoWithIDNum( pictureIDNum, &psPictureInfo);

    if((xsize + xOffset ) > psPictureInfo.xSize)
        xsize =  psPictureInfo.xSize - xOffset;

     if((ysize + yOffset ) > psPictureInfo.ySize)
        ysize =  psPictureInfo.ySize - yOffset;

    for(i=0;i<ysize;i++){
        flashAddr = psPictureInfo.offsetAddr + (psPictureInfo.xSize * (yOffset+i) * 2) + (xOffset * 2);
        DisplayPicture(x+xOffset, y+yOffset+i, xsize, 1, flashAddr);
    }
}

/*
 ********************************************************************************
 *  Function name :  DisplayPicture()
 *  Author:          anzhiguo
 *  Description:
 *  Calls:
 *
 *  Input:           x,y          : start coordinate
 *                   xsize,ysize  : the picture size.
 *                   Flash_Addr   : the flash address of picture.
 *  Return:
 *  Remark:
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            AZG          2009-2-24      1.0           ORG
 *
 ********************************************************************************
 */

#define     LCD_DISPLAY_BUF     512//2048//256

_ATTR_LCD_CODE_
void DisplayPicture(INT16 x, INT16 y, INT16 xsize, INT16 ysize, unsigned long Flash_Addr)
{
    UINT16  DispPicBuf[LCD_DISPLAY_BUF>>1];
    UINT16  DispPicLen;
    INT16   ysize_Seg;
    INT16   ysize_Spare;
    INT16   Disp_Counter;
    uint16 i,j;
    UINT16  *pBuf;

    if ((xsize == 0) || (ysize == 0))
        return;

    /*  ����ͼƬ�ڡ�NAND��Flash���еĵ�ַ   */
    //if (((unsigned long)(xsize)*ysize) <= DISP_PIC_BUF_SIZE)//����buf���Դ��һ��������ͼƬ
    if (((unsigned long)(xsize)*ysize) <= (LCD_DISPLAY_BUF>>1))//����buf���Դ��һ��������ͼƬ
    {
        /*  �ڡ�NAND��Flash���ж�ȡͼƬ����     */
        DispPicLen   = xsize * ysize;
        //LcdGetResourceData(Flash_Addr + ImageLogicAddress,(UINT8*) DispPicBuf, DispPicLen<<1);
        LcdGetResourceData(Flash_Addr + ImageLogicAddress,(UINT8*) DispPicBuf, LCD_DISPLAY_BUF);
        DisplayPictureFromBuffer(x, y, xsize, ysize, DispPicBuf);
        return;
    }

    //ͼƬ���ݲ���һ�ζ���ʱ����Ҫ�ִζ�ȡ��ÿ�ζ�ȡ�ĳ�����ͼƬx����ĳ���Ϊ��λ��
    //��һ�ο��Զ�ȡ�������ݣ�������ݵĴ�С��������ʾbuf DISP_PIC_BUF_SIZE�Ĵ�С
    //ysize_Seg    = DISP_PIC_BUF_SIZE / xsize;//�����ȡ������һ�ο�����ʾ����
    ysize_Seg    = (LCD_DISPLAY_BUF>>1) / xsize;//�����ȡ������һ�ο�����ʾ����
    if (ysize_Seg == 0)
        return;

    //printf("\nyszie_seg = %d", ysize_Seg);

    Disp_Counter = ysize / ysize_Seg;//����ͼƬ��Ҫ���ζ�ȡ(ʵ�ʵĴ��������ֵ+1����Ϊ���ܻ���ʣ���ͼƬ����)
    ysize_Spare  = ysize % ysize_Seg;//��ȡ��ʣ������ݲ���һ��buffer

    DispPicLen   = ysize_Seg * xsize;//һ�ζ�ȡ�����ݵĳ���

    for (; Disp_Counter > 0; Disp_Counter--)
    {
        /*  ����ͼƬ��ǰ���Ӳ�����NAND��FLASH�еĵ�ַ               */
        /*  ��NAND��FLASH�ж�ȡ����ͼƬ���ݣ����ݳ���ΪDispPicLen�� */
        //LcdGetResourceData(Flash_Addr + ImageLogicAddress, (UINT8*)DispPicBuf, DispPicLen<<1);
        LcdGetResourceData(Flash_Addr + ImageLogicAddress, (UINT8*)DispPicBuf, DispPicLen*2);
        DisplayPictureFromBuffer(x, y, xsize, ysize_Seg, DispPicBuf);
        y += ysize_Seg;
        Flash_Addr += DispPicLen*2;
    }
    if (ysize_Spare)//��ȡʣ���ͼƬ���ݣ��ⲿ�����ݣ�
    {
        DispPicLen = ysize_Spare*xsize;
        /*����ͼƬʣ�ಿ����NAND��FLASH�еĵ�ַ*/
        /*��NAND��FLASH�ж�ȡʣ�ಿ��ͼƬ���ݣ����ݳ���ΪDispPicLen��*/
        //LcdGetResourceData(Flash_Addr + ImageLogicAddress, (UINT8*)DispPicBuf, DispPicLen<<1);
        LcdGetResourceData(Flash_Addr + ImageLogicAddress, (UINT8*)DispPicBuf, DispPicLen*2);
        DisplayPictureFromBuffer(x, y, xsize, ysize_Spare, DispPicBuf);
    }
}


/*
********************************************************************************
*  Function name :  DisplayPictureWithPicID()
*  Author:          anzhiguo
*  Description:     draw picture with id and x,y offset.the origin is derived by tool,
*
*  Input:           pictureIDNum     :  id
*                   xOffset, yOffset �� corodinate offset of picture display.
*  Output:          null
*  Return:          null
*  Calls:           FlashReadLogicRaw(): get picture from flash by 16 bits.
*                   DisplayPicture():
*
*  History:     <author>         <time>         <version>   <desc>
*               anzhiguo        2009-2-24         Ver1.0       ORG
********************************************************************************
*/
_ATTR_LCD_CODE_
void DispPictureWithIDNumAndXYoffset(UINT16 pictureIDNum, UINT16 xOffset, UINT16 yOffset)
{
    PICTURE_INFO_STRUCT  picInfo;

    GetPictureInfoWithIDNum(pictureIDNum, &picInfo);//get picture resource information structure.
    DisplayPicture((picInfo.x + xOffset), (picInfo.y + yOffset), picInfo.xSize, picInfo.ySize, picInfo.offsetAddr);
}

/*
********************************************************************************
*  Function name :  DispPictureWithIDNumAndXY()
*  Author:          anzhiguo
*  Description:     display picture with id and coordinate x y them will mask the value that are derived by tool.
*
*  Input:           pictureIDNum     : id
*                   x, y ��            coordinate.
*  Output:          null
*  Return:          null
*
*
*  History:     <author>         <time>         <version>   <desc>
*               anzhiguo       2009-2-24     Ver1.0       ORG
********************************************************************************
*/
_ATTR_LCD_CODE_
void DispPictureWithIDNumAndXY(UINT16 pictureIDNum, UINT16 x, UINT16 y)
{
    PICTURE_INFO_STRUCT  picInfo;

    GetPictureInfoWithIDNum(pictureIDNum, &picInfo);
    DisplayPicture(x, y, picInfo.xSize, picInfo.ySize, picInfo.offsetAddr);

    return;
}


/*
--------------------------------------------------------------------------------
  Function name : void LCD_DrawBmp(INT16 x0, INT16 y0, INT16 xsize, INT16 ysize, INT16 Pixel, UINT32 *pData)
  Author        : ZHengYongzhi
  Description   : display the picture in specif postion.
  Input         :   x,y          : the start position of picture.
                    xsize,ysize  : picture size.
                    pData   :      picture buffer size.
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_DrawBmp(UINT16 x0, UINT16 y0, UINT16 xsize, UINT16 ysize, UINT16 Pixel, UINT16 *pData)
{
    UINT16 i,j;
    INT16  x, y;
    INT16  x1,y1;
    INT16  BytesPerLine;
    UINT16 BitOffset =0;
    UINT16 Xpos,Ypos;
    UINT8 bPage,bColNum;

    UINT16 x_temp,y_temp;
    x1 = x0+xsize-1;
    y1 = y0+ysize-1;

    if (xsize < 0 || ysize < 0)
    {
        return;
    }

    switch (Pixel)
    {
        case 1:
            y = y0;
            for (; ysize > 0; ysize--, y++)
            {

                x = x0;
                for (i = xsize; i > 0; i--, x++)
                {

                    if (*pData & (0x8000 >> BitOffset))
                    {

                        if (LCD_DRAWMODE & LCD_DRAWMODE_REV)
                        {

#if (LCD_PIXEL == LCD_PIXEL_16)
                            {
                                LCD_SetPixel(x, y, LCD_BKCOLOR);
                            }
#elif (LCD_PIXEL == LCD_PIXEL_1)
                            {
                                LCD_SetPixel(x, y, 0/*LCD_BKCOLOR*/);
                            }
#endif

                        }
                        else
                        {

#if (LCD_PIXEL == LCD_PIXEL_16)
                            {
                                LCD_SetPixel(x, y, LCD_COLOR);
                            }
#elif (LCD_PIXEL == LCD_PIXEL_1)
                            {
                                LCD_SetPixel(x, y,/* LCD_COLOR*/1);
                            }
#endif
                        }

                    }
                    else
                    {

                        if (LCD_DRAWMODE & LCD_DRAWMODE_REV)
                        {

#if (LCD_PIXEL == LCD_PIXEL_16)
                            {
                                LCD_SetPixel(x, y, LCD_COLOR);
                            }
#elif (LCD_PIXEL == LCD_PIXEL_1)
                            {
                                LCD_SetPixel(x, y,/* LCD_COLOR*/1);
                            }
#endif

                        }
                        else if (0 == (LCD_DRAWMODE & LCD_DRAWMODE_TRANS))
                        {

#if (LCD_PIXEL == LCD_PIXEL_16)
                            {
                                LCD_SetPixel(x, y, LCD_BKCOLOR);
                            }
#elif (LCD_PIXEL == LCD_PIXEL_1)
                            {
                                LCD_SetPixel(x, y, 0/*LCD_BKCOLOR*/);
                            }
#endif
                        }
                    }

                    if (++BitOffset == xsize)
                    {
                        BitOffset = 0;
                        pData++;
                    }
                }
            }
#if (LCD_PIXEL == LCD_PIXEL_1)
            {
                LCD_Buffer_Display1(x0,  y0,  x1,  y1); //fjp
            }
#endif
            break;

        case 16:
#if (LCD_PIXEL == LCD_PIXEL_1)
            {
                if (LCD_DRAWMODE & LCD_DRAWMODE_REV)
                {
                    for (y_temp=y0; y_temp<=y1; y_temp++)
                    {
                        for (x_temp=x0; x_temp<=x1; x_temp++)
                        {
                            if (!(*(pData++)))
                            {
                                LCDDEV_SetPixel(x_temp,y_temp, 0);
                            }
                            else
                            {
                                LCDDEV_SetPixel(x_temp,y_temp, 1);
                            }
                        }
                    }
                }
                else
                {
                    for (y_temp=y0; y_temp<=y1; y_temp++)
                    {
                        for (x_temp=x0; x_temp<=x1; x_temp++)
                        {
                            if (!(*(pData++)))
                            {
                                LCDDEV_SetPixel(x_temp,y_temp, 1);
                            }
                            else
                            {
                                LCDDEV_SetPixel(x_temp,y_temp, 0);
                            }
                        }
                    }
                }

                LCD_Buffer_Display1(x0,  y0,  x1,  y1);
            }
#else
            {
                //Xpos = x0;
                //Ypos = y0;
                //x1   = x0 + xsize - 1;
                //y1   = y0 + ysize - 1;

#if (LCD_DIRECTION == LCD_HORIZONTAL)
                if (x1>LCD_WIDTH-1) x1 = LCD_WIDTH-1;
                if (y1>LCD_HEIGHT-1) y1 = LCD_HEIGHT-1;
#endif

#ifdef _DMA_LCD_
                Lcdchang(pData,(x1-x0)*(y1-y0+1)*2);
                Lcd_DMATranfer(x0,y0-1,x1,y1,pData);
#else
                Lcd_SetWindow(x0, y0, x1, y1);

                for (j = y0; j < y1 + 1; j++)
                {
                    for (i = x0; i < x1 + 1; i++)
                    {
                        Lcd_SendData (*pData++);
                    }
                    //Ypos++;
                    //Lcd_SetWindow(Xpos, Ypos,x1,y1);
                }
#endif//_DMA_LCD_
            }
#endif//(LCD_PIXEL == LCD_PIXEL_1)
            break;

        default:
            break;
    }
}

/*
********************************************************************************
*
*                         End of LcdImage.c
*
********************************************************************************
*/

