/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name��   LcdInterface.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#define _IN_LCDINTERFACE_
#include "fsinclude.h"
#include "DriverInclude.h"
#include "LcdInclude.h"
#include "ModuleInfoTab.h"
#include "ModuleOverlay.h"
#include "LcdInterface.h"


#ifdef _FRAME_BUFFER_
typedef struct RectInfoStruct
{
    UINT16  x0;
    UINT16  y0;
    UINT16  x1;
    UINT16  y1;
    UINT16  x;
    UINT16  y;
} RECT_INFO_STRUCT;

#if (LCD_PIXEL == LCD_PIXEL_1)
    __align(4)
    /*static*/ UINT8 frame_buffer[BUFFER_MAX_NUM][LCD_HEIGHT/8][LCD_WIDTH];
#else
    __align(4)
    _ATTR_LCD_BSS_  UINT16 frame_buffer[BUFFER_MAX_NUM][LCD_HEIGHTA][LCD_WIDTH];
    #if (FRAME_SUB_BUFFER_NUM > 1)
    _ATTR_LCD_BUF_  UINT16 frame_bufferB[BUFFER_MAX_NUM][LCD_HEIGHTB][LCD_WIDTH];
    #endif
#endif

//bin: 1111 0111 1101 1110 ; RGB fmt: 11110 111110 11110
_ATTR_LCD_DATA_ static UINT16  RGB_Gray_Scale_Mask_Lev1 = 0xF7DE ;

//bin: 1110 0111 1001 1100 ; RGB fmt: 11100 111100 11100
_ATTR_LCD_DATA_ static UINT16  RGB_Gray_Scale_Mask_Lev2 = 0xE79C ;

//bin: 1100 0111 0001 1000 ; RGB fmt: 11000 111000 11000
_ATTR_LCD_DATA_ static UINT16  RGB_Gray_Scale_Mask_Lev3 = 0xC718 ;

//bin: 1000 0110 0001 0000 ; RGB fmt: 10000 110000 10000
_ATTR_LCD_DATA_ static UINT16  RGB_Gray_Scale_Mask_Lev4 = 0x8610 ;

//bin: 0000 0100 0000 0000 ; RGB fmt: 00000 100000 00000
_ATTR_LCD_DATA_ static UINT16  RGB_Gray_Scale_Mask_Lev5 = 0x0400 ;

_ATTR_LCD_BSS_ /*static*/ UINT16 CurrentFrameIndex;

_ATTR_LCD_BSS_ static UINT16 RGB_Gray_Scale_Mask;  //init at LCD_Buffer_Gray_Scale()
_ATTR_LCD_BSS_ static UINT16 Ymin;
_ATTR_LCD_BSS_ static UINT16 Ymax;
_ATTR_LCD_BSS_ static UINT32 CrossBorder;
_ATTR_LCD_BSS_ static UINT32 Refresh;
_ATTR_LCD_BSS_ static UINT32 RefreshCnt;
_ATTR_LCD_BSS_ static UINT32 FrameMode;
_ATTR_LCD_BSS_ static RECT_INFO_STRUCT RECT;
#endif

/*
--------------------------------------------------------------------------------
  Function name : void LCD_GPIO_Init(void)
  Author        : chad.ma
  Description   : lcd gpio iomux

  Input         : null
  Return        :

  History:     <author>         <time>         <version>
                                2015-2-14 11:06:14
  desc:         ORG
--------------------------------------------------------------------------------
*/
void LCD_GPIO_Init(void)
{
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin7,IOMUX_GPIO0B7_LCD_CSN);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_LCD_WRN);
}


/*
--------------------------------------------------------------------------------
  Function name : void PowerOnLcdinit(void)
  Author        : yangwenjie
  Description   : system lcd initialization.

  Input         : null
  Return        :

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_SYS_INIT_CODE_
void PowerOnLcdinit(void)
{
    LCD_RECT *r;
    int type = 0;

    //ScuClockGateCtr(HCLK_LCDC_GATE, 1);
    ScuSoftResetCtr(LCDC_SRST, 0);

    r = &LcdContext.ClipRect;
    r->x0 = 0;
    r->y0 = 0;
    r->x1 = LCD_WIDTH - 1;
    r->y1 = LCD_HEIGHT - 1;

    LCD_SetColor(COLOR_BLACK);
    LCD_SetBkColor(COLOR_WHITE);
    LCD_SetDiaplayMode(LCD_MODE_0);

    LcdContext.DrawMode = LCD_DRAWMODE_NORMAL;
    LcdContext.DispPosX = 0;
    LcdContext.DispPosY = 0;
    LcdContext.LBorder  = 0;
    LcdContext.XDist    = 0;
    LcdContext.YDist    = 0;
    LcdContext.TextAlign= LCD_TEXTALIGN_LEFT;
    LcdContext.RBorder  = LCD_WIDTH - 1;

    Lcd_Init();

#if (LCD_PIXEL == LCD_PIXEL_16)
    Lcd_Clear(0x0000);
    LCD_FillRect(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);
#elif (LCD_PIXEL == LCD_PIXEL_1)
    LCD_ClrSrc();
#endif
}



#ifndef _FRAME_BUFFER_
/*
--------------------------------------------------------------------------------
  Function name : void Lcd_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *src)
  Author        : yangwenjie
  Description   : DMA data transfer

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-1��15         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
uint32 Lcd_Dma_ch = 0xff;
_ATTR_LCD_CODE_
void Lcd_Dma_Isr(void)
{
    Lcd_SplitMode(LCD_SPLIT2);
    Lcd_Dma_ch = 0xff;
}

_ATTR_LCD_CODE_
int32 Lcd_DMATranfer (UINT16 x0,UINT16 y0,UINT16 x1,UINT16 y1,UINT16 *pSrc)
{
    UINT32 size;
    eDMA_CHN channel;
    DMA_CFGX DmaCfg = {CTLL_M2LCD_HALF,CFGL_M2LCD_SINGLE,CFGH_M2LCD_SINGLE,0};

    Lcd_SetWindow(x0,y0, x1,y1);

    channel = DmaGetChannel();
    if (channel != DMA_FALSE)
    {
        size = (((x1 + 1) - x0) * ((y1+1) - y0)) >> 1;
        //UserIsrDisable();
        DmaStart((uint32)(channel), (UINT32)(pSrc),(uint32)(&(Lcd->data)),size,&DmaCfg, Lcd_Dma_Isr);
        Lcd_Dma_ch = channel;
        //UserIsrEnable();
        return channel;
    }
    return (-1);
}


/*
--------------------------------------------------------------------------------
  Function name : int32 Lcd_DMATranfer32 (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT32 *pSrc)
  Author        : yangwenjie
  Description   : DMA data transfer

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-1��15         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
int32 Lcd_DMATranfer32 (UINT16 x0,UINT16 y0,UINT16 x1,UINT16 y1,UINT32 *pSrc)
{
    UINT32 size;
    UINT32 *pDst;
    eDMA_CHN channel;
    DMA_CFGX DmaCfg = {CTLL_M2LCD_WORD,CFGL_M2LCD_SINGLE,CFGH_M2LCD_SINGLE,0};

    Lcd_SetWindow(x0,y0, x1,y1);

    channel = DmaGetChannel();
    if (channel != DMA_FALSE)
    {
        size = (((x1 + 1) - x0) * ((y1+1) - y0)) >> 1;
        Lcd_SplitMode(LCD_SPLIT5);
        //UserIsrDisable();
        DmaStart((uint32)(channel), (UINT32)(pSrc),(uint32)(&(Lcd->data)),size,&DmaCfg, Lcd_Dma_Isr);
        Lcd_Dma_ch = channel;
        //UserIsrEnable();
        return channel;
    }
    return (-1);
}
#endif
/*
--------------------------------------------------------------------------------
  Function name : void  GetResourceData(UINT32 Addr,UINT32*Buffer,UINT16 Length)
  Author        : yangwenjie
  Description   : get resource data.

  Input         : Addr      ��Flash resource address
                  Buffer    ��charactor data buffer.
				  Length    ��get data length.
  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void  LcdGetResourceData(UINT32 Addr,UINT8*pData,UINT16 Length)
{
    int8  hMainFile;

#if (RES_IN_DEV == 1)
    ReadResourceData(Addr,pData,Length);
#else
    //for SPI Flash Test
    if ((hMainFile = FileOpenW(NULL,L"\\",L"RkNanoImage.uis", "R")) == -1)//���ļ��ɹ������ش���
    {
        DEBUG("Open File ERROR");
    }
    FileSeek((Addr- ImageLogicAddress), 0, hMainFile);
    FileRead((UINT8*)pData, Length, hMainFile);
    FileClose(hMainFile);
#endif
    Lcdchang((UINT16*)pData,Length);
}

/*
--------------------------------------------------------------------------------
  Function name : void  LcdGetFront12Data(UINT32 Addr,uint8 * pBuf, short dat)
  Author        : yangwenjie
  Description   : get front12 resource data.

  Input         : Addr      ��Flash resource address
                  Buffer    ��charactor data buffer.
                  Length    ��get data length.
  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void  LcdGetFront12Data(UINT32 Addr,UINT8*pData,UINT16 Length)
{
    int8  hMainFile;
	uint8 * pstData, temp;
	uint16 i;

#if (RES_IN_DEV == 1)
    ReadResourceData(Addr,pData,Length);
#else
    //for SPI Flash Test
    if ((hMainFile = FileOpenW(NULL,L"\\",L"Font12.bin", "R")) == -1)//���ļ��ɹ������ش���
    {
        DEBUG("Open File ERROR");
    }
    FileSeek((Addr- FontLogicAddress), 0, hMainFile);
    FileRead((UINT8*)pData, Length, hMainFile);
    FileClose(hMainFile);
#endif

    Lcdchang((UINT16*)pData,Length);
}

/*
--------------------------------------------------------------------------------
  Function name : void  LcdGetFront16Data(UINT32 Addr,uint8 * pBuf, short dat)
  Author        : yangwenjie
  Description   : get front12 resource data.

  Input         : Addr      ��Flash resource address
                  Buffer    ��charactor data buffer.
                  Length    ��get data length.
  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void  LcdGetFront16Data(UINT32 Addr,UINT8*pData,UINT16 Length)
{
    int8  hMainFile;

#if (RES_IN_DEV == 1)
    ReadResourceData(Addr,pData,Length);
#else
    //for SPI Flash Test
    if ((hMainFile = FileOpenW(NULL,L"\\",L"Font16.bin", "R")) == -1)//���ļ��ɹ������ش���
    {
        DEBUG("Open File ERROR");
    }
    FileSeek((Addr- FontLogicAddress), 0, hMainFile);
    FileRead((UINT8*)pData, Length, hMainFile);
    FileClose(hMainFile);
#endif

    Lcdchang((UINT16*)pData,Length);
}

/*
--------------------------------------------------------------------------------
  Function name : void GetResourceInfo(UINT32 Addr,UINT32*Buffer,UINT16 Length)
  Author        : yangwenjie
  Description   : get resource information

  Input         : Addr      ��Flash��Դ��ַ
                  Buffer    ���ַ�����Buffer
				  Length    ����ȡ���ݵĳ���
  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LcdGetResourceInfo(UINT32 Addr,UINT8*pData,UINT16 Length)
{
    ReadResourceData(Addr,pData,Length);
}

void ImageMemSet(uint32 *trg, int data, uint32 len)
{
    uint32 i;
    for (i = 0; i < len; i++)
    {
        trg[i*8] = 0;
        trg[i*8 + 1] = 0;
        trg[i*8 + 2] = 0;
        trg[i*8 + 3] = 0;
        trg[i*8 + 4] = 0;
        trg[i*8 + 5] = 0;
        trg[i*8 + 6] = 0;
        trg[i*8 + 7] = 0;
    }
}

/*
--------------------------------------------------------------------------------
  Function name :
  Author        : yangwenjie
  Description   : Lcd driver interface definition

  Input         :
  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#if 0
_ATTR_LCD_CODE_
void Lcd_SendData(UINT16 data)
{
    SystemLcdPara.pLcd_SendData(data);
}
#endif
/*
--------------------------------------------------------------------------------
  Function name :
  Author        : yangwenjie
  Description   : Lcd initial

  Input         :
  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
extern FIRMWARE_INFO_T const ModuleInfo;
_ATTR_SYS_INIT_CODE_
void Lcd_Init(void)
{
    uint32 SystemDefaultParaAddr;
    uint8  TempBuffer[512];
    uint8  FlashBuf[512];

    FIRMWARE_INFO_T       *pFirmwareModuleInfo;
    LCD_DRIVER_INFO_T     *pSystemLcdPara;

    ////////////////////////////////////////////////////////////////////////////
    MDReadData(SysDiskID, CodeLogicAddress, 512, FlashBuf);
    pFirmwareModuleInfo = (FIRMWARE_INFO_T *)FlashBuf;

    SystemDefaultParaAddr  = CodeLogicAddress + sizeof(pFirmwareModuleInfo -> LoadStartBase);
    SystemDefaultParaAddr += sizeof(pFirmwareModuleInfo -> ModuleInfo.ModuleNum);
    SystemDefaultParaAddr += pFirmwareModuleInfo -> ModuleInfo.ModuleNum * sizeof(CODE_INFO_T);
    SystemDefaultParaAddr += sizeof(SYSTEM_DEFAULT_PARA_T)+sizeof(FM_DRIVER_INFO_T);
    MDReadData(SysDiskID, SystemDefaultParaAddr, sizeof(LCD_DRIVER_INFO_T), FlashBuf);
    pSystemLcdPara = (LCD_DRIVER_INFO_T*)FlashBuf;

    ///////////////////////////////////////////////////////////////////////
    //DEBUG("pSystemLcdPara->LcdDriverIndex= %d",pSystemLcdPara->LcdDriverIndex);
    ModuleOverlay(pSystemLcdPara->LcdDriverIndex + MODULE_ID_FLASH_PROG + 1, MODULE_OVERLAY_ALL);
    memcpy((UINT8*)(&SystemLcdPara),(UINT8*)(&(pSystemLcdPara->LcdDriver[pSystemLcdPara->LcdDriverIndex])),sizeof(LCD_DRIVER_TABLE_T));

#ifdef _FRAME_BUFFER_
    ImageMemSet((uint32*)frame_buffer, 0 , sizeof(frame_buffer)/(sizeof(uint32) * 8));
#if (FRAME_SUB_BUFFER_NUM > 1)
    ImageMemSet((uint32*)frame_bufferB, 0 , sizeof(frame_bufferB)/(sizeof(uint32) * 8));
#endif

    Ymin = 0;
    Ymax = 0;
    CurrentFrameIndex = 0;
    Refresh = 0;
    RefreshCnt = 0;
    CrossBorder = 1;
    memset(&RECT, 0 , sizeof(RECT_INFO_STRUCT));
#endif

    SystemLcdPara.pLcd_Init();
}


/*
--------------------------------------------------------------------------------
  Function name :
  Author        : yangwenjie
  Description   : Lcd driver interface

  Input         :
  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LcdStandby(void)
{
    SystemLcdPara.pLcd_Standby();

    //ScuSoftResetCtr(LCDC_SRST, 1);
    ScuClockGateCtr(HCLK_LCDC_GATE, 0);
}

/*
--------------------------------------------------------------------------------
  Function name :
  Author        : yangwenjie
  Description   :Lcd driver interface

  Input         :
  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LcdWakeUp(void)
{
    ScuClockGateCtr(HCLK_LCDC_GATE, 1);
    //ScuSoftResetCtr(LCDC_SRST, 0);
    SystemLcdPara.pLcd_WakeUp();
}



#ifdef _DISP_FROM_RAM_
/*
--------------------------------------------------------------------------------
  Function name : void  GetPictureResourceToBuffer(UINT16 pictureIDNum, uint8 *pBuf, uint16 len)
  Author        : yangwenjie
  Description   : get resource.

  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void  GetPictureResourceToBuffer(UINT16 pictureIDNum, uint8 *pBuf, uint16 len)
{
    UINT32 i;
    PICTURE_INFO_STRUCT  picInfo;

    GetPictureInfoWithIDNum(pictureIDNum, &picInfo);
    LcdGetResourceData(picInfo.offsetAddr + ImageLogicAddress, pBuf, len);

}

/*
--------------------------------------------------------------------------------
  Function name : void  GetPictureResourceToBuffer(UINT16 pictureIDNum, uint8 *pBuf, uint16 len)
  Author        : yangwenjie
  Description   :

  Input         :
  Return        :null

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void GetPictureInfoForBuffer(UINT16 pictureIDNum, PICTURE_INFO_STRUCT *pInfo)
{
    UINT32 i;
    PICTURE_INFO_STRUCT  picInfo;

    GetPictureInfoWithIDNum(pictureIDNum, &picInfo);//get resource information structure.
#if 0
    pInfo->x          = picInfo.x;
    pInfo->y          = picInfo.y;
    pInfo->xSize      = picInfo.xSize;
    pInfo->ySize      = picInfo.ySize;
    pInfo->offsetAddr = picInfo.offsetAddr;
    pInfo->totalSize  = picInfo.totalSize;
#else
    memcpy((uint8*)pInfo, (uint8*)(&picInfo), sizeof(PICTURE_INFO_STRUCT));
#endif
}


/*
--------------------------------------------------------------------------------
  Function name : void  GetPictureResourceToBuffer(UINT16 pictureIDNum, uint8 *pBuf, uint16 len)
  Author        : yangwenjie
  Description   :

  Input         :
  Return

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void GetCharResourceToBuffer(char Data, uint8 *pBuf)
{
    UINT16 Len, size;
    UINT32 CharInNFAddr;

    if (LcdContext.TextFort == FONT_12x12)
    {
        CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(Data) * 28) + 4;
        LcdGetFront12Data(CharInNFAddr, pBuf, 24);
    }
    else
    {
        CharInNFAddr = FontLogicAddress + (UINT32)((UINT32)(Data) * 36) + 4;
        LcdGetFront16Data(CharInNFAddr, pBuf, 32);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void  DisplayCharFromBuffer(uint16 *pBuf, uint8 Index,UINT16 x,UINT16 y)
  Author        : yangwenjie
  Description   :

  Input         :
  Return

  History:     <author>         <time>         <version>
              yangwenjie      2009/02/12         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void DisplayCharFromBuffer(uint16 *pBuf, uint8 Index,UINT16 x,UINT16 y)
{
    UINT16 asx,asy,fntsize;

    if (LcdContext.TextFort == FONT_12x12)
    {
        LCD_DrawBmp(x,y,AS_CHAR_XSIZE_12,AS_CHAR_YSIZE_12,1, pBuf + Index * FONT12_CHAR_SIZE / 2);
    }
    else
    {
        LCD_DrawBmp(x,y,AS_CHAR_XSIZE_16,AS_CHAR_YSIZE_16,1, pBuf + Index * FONT16_CHAR_SIZE / 2);
    }
}
#endif

#ifdef _FRAME_BUFFER_

_ATTR_LCD_CODE_
void Lcd_SendData(UINT16 data)
{
    if (CrossBorder == 1)
        return;

    Refresh = 1;

    if (RECT.y < LCD_HEIGHTA)  //����Acore Buffer ����
    {
        if (RECT.x % 2)
            frame_buffer[CurrentFrameIndex][RECT.y][ RECT.x - 1]= data;
        else
            frame_buffer[CurrentFrameIndex][ RECT.y][RECT.x + 1]= data;
    }
#if (FRAME_SUB_BUFFER_NUM > 1)
    else if (RECT.y >= LCD_HEIGHTA)   //
    {
        if (RECT.x % 2)
            frame_bufferB[CurrentFrameIndex][RECT.y-LCD_HEIGHTA][RECT.x - 1]= data;
        else
            frame_bufferB[CurrentFrameIndex][RECT.y-LCD_HEIGHTA][RECT.x + 1]= data;
    }
#endif

    if (FrameMode == 0)
    {
        RECT.x++;

        if (RECT.x > RECT.x1)
        {
            RECT.x = RECT.x0;
            RECT.y++;
            if (RECT.y > RECT.y1)
            {
                RECT.x = RECT.x0;
                RECT.y = RECT.y0;
            }
        }
    }
    else
    {
        RECT.y++;

        if (RECT.y > RECT.y1)
        {
            RECT.y = RECT.y0;

            if (RECT.x <= RECT.x0)
            {
                RECT.x = RECT.x1;
                RECT.y = RECT.y0;
            }
            else
            {
                RECT.x--;
            }
        }
    }

}

_ATTR_LCD_CODE_
void Lcd_SetWindow(uint16 x0,uint16 y0,uint16 x1,uint16 y1)
{
    if (FrameMode == 0)
    {
        RECT.x0 = x0;
        RECT.x1 = (x1 > (LCD_WIDTH - 1))? (LCD_HEIGHT - 1):x1;
        RECT.y0 = y0;
        RECT.y1 = (y1 > (LCD_WIDTH - 1))? (LCD_HEIGHT - 1):y1;
        RECT.x  = RECT.x0;
        RECT.y  = RECT.y0;
    }
    else
    {
        RECT.x0 = (y1 > (LCD_WIDTH - 1)) ? 0 : (LCD_WIDTH - y1 - 1);
        RECT.x1 = (y0 > (LCD_WIDTH - 1)) ? 0 : (LCD_WIDTH - y0 - 1);

        RECT.y0 = x0;
        RECT.y1 = (x1 > LCD_HEIGHT)? (LCD_HEIGHT - 1) : x1;
        RECT.x  = RECT.x1;
        RECT.y  = RECT.y0;
    }


    if (RECT.y1 > Ymax)
    {
        Ymax = RECT.y1;
    }

    if (RECT.y0 < Ymin)
    {
        Ymin = RECT.y0;
    }

    CrossBorder = 0;
}

_ATTR_LCD_CODE_
void Lcd_Clear(uint16 color)
{
    int i,j;

    Lcd_SetWindow(0, 0, LcdContext.LcdMaxWidth - 1, LcdContext.LcdMaxHeight - 1);

    for (i = 0; i < LCD_HEIGHT; i++)
    {
        for (j = 0; j < LCD_WIDTH; j++)
        {
            Lcd_SendData(color);
        }
    }
}

_ATTR_LCD_CODE_
void LCD_ClrSrc(void)
{
#if (LCD_PIXEL == LCD_PIXEL_1)
    LCD_ClrRect(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
#else
    Lcd_Clear(0x00);
#endif
}

_ATTR_LCD_CODE_
void LCD_ClrRect(int x0, int y0, int x1, int y1)
{
    int i,j;

#if (LCD_PIXEL == LCD_PIXEL_1)
    if(y0 < 0 || x0 < 0)
        return;

    if(y1 >= LCD_HEIGHT || x1 >= LCD_WIDTH)
        return;

    Refresh = 1;
    for (i = y0; i <= y1/8; i++)
    {
        for (j = x0; j <= x1; j++)
        {
            frame_buffer[CurrentFrameIndex][i][j] = 0x00;
        }
    }
#else
    Lcd_SetWindow(x0, y0, x1, y1);

    for (i = y0; i <= y1; i++)
    {
        for (j = x0; j <= x1; j++)
        {
            Lcd_SendData(0x00);
        }
    }
#endif
}

_ATTR_LCD_CODE_
void Lcd_WriteRAM_Prepare(void)
{
//scatterloader.c��ע����ST7735����
    SystemLcdPara.pLcd_WriteRAM_Prepare();
}

_ATTR_LCD_CODE_
void Lcd_FreshEn (void)
{
    RefreshCnt = 0;
}

_ATTR_LCD_CODE_
void Lcd_FreshDisable (void)
{
    RefreshCnt = 1;
}

_ATTR_LCD_CODE_
void Lcd_SetFrameMode (uint32 mode)
{
    FrameMode = mode;
    CrossBorder = 1;
}


_ATTR_LCD_CODE_
void Lcd_BuferTranfer (void)
{
    int ret = 0;
    int Ya,Yb;

    if(IsBackLightOn == TRUE)  //add by chad.ma 20151102,if backlight is off,return directly.
        return ;

    if (Refresh && (RefreshCnt == 0))
    {
        #if (LCD_PIXEL == LCD_PIXEL_1)
        {
            ret = SystemLcdPara.pLcd_DMATranfer(0, 0, LCD_WIDTH - 1, LCD_HEIGHT- 1,
                                            &frame_buffer[CurrentFrameIndex][0][0]);
        }
        #else
        {
            if (Ymin < LCD_HEIGHTA)
            {
                Ya = Ymin;
                Yb = LCD_HEIGHTA - 1;
                if (Ymax < LCD_HEIGHTA)
                {
                    Yb = Ymax;
                }
                ret = SystemLcdPara.pLcd_DMATranfer(0, Ya, LCD_WIDTH - 1, Yb,
                                                    &frame_buffer[CurrentFrameIndex][Ya][0]);
            }
            #if (FRAME_SUB_BUFFER_NUM > 1)
            {
                if (Ymax >= LCD_HEIGHTA)
                {
                    Yb = Ymax;
                    Ya = LCD_HEIGHTA;
                    if (Ymin >=  LCD_HEIGHTA)
                    {
                        Ya = Ymin;
                    }
                    DelayMs(1);
                    ret = SystemLcdPara.pLcd_DMATranfer(0, Ya, LCD_WIDTH - 1, Yb,
                                                &frame_bufferB[CurrentFrameIndex][Ya - LCD_HEIGHTA][0]);
                }
            }
            #endif
        }
        #endif

        Refresh = 0;
        Ymin    = LCD_HEIGHT;
        Ymax    = 0;
    }
}

#if (LCD_PIXEL == LCD_PIXEL_1)
_ATTR_LCD_CODE_
void LCDDEV_SetPixel(UINT16 x, UINT16 y, UINT16 data)
{
    if(y < 0 || x < 0)
        return;

    if(y >= LCD_HEIGHT || x >= LCD_WIDTH)
        return;

    Refresh = 1;

    if(data)
        frame_buffer[CurrentFrameIndex][(unsigned int)y/8][x] = frame_buffer[CurrentFrameIndex][(unsigned int)y/8][x]|(0x01<<(y%8));
    else
        frame_buffer[CurrentFrameIndex][(unsigned int)y/8][x] = frame_buffer[CurrentFrameIndex][(unsigned int)y/8][x]&(~(0x01<<(y%8)));
}

_ATTR_LCD_CODE_
void LCD_Buffer_Display1(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1)
{
    Lcd_BuferTranfer();
}

_ATTR_LCD_CODE_
void LCDDEV_FillRect(int x0, int y0, int x1, int y1)
{
	int i,j;
	for (i=y0; i<=y1; i++)
	{
		for(j=x0; j<=x1; j++)
			LCD_SetPixel(i,j,1);
    }

	LCD_Buffer_Display1(x0,y0,x1,y1);
}
#endif
/*
--------------------------------------------------------------------------------
  Function name : int Lcd_Set_Current_BufferFrame_Index(UINT16 index)
  Author        :
  Description   :  for three buffer frame index

  Input         : frame index
  Return       : if index is not in 0,1,2, return ERROR .

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
int LCD_Set_Current_BufferFrame_Index(UINT16  index)
{

    if (index >=0 && index <=BUFFER_MAX_NUM-1)
    {

        CurrentFrameIndex = index;

        return 0;
    }
    else
    {
        return -1;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : int LCD_Copy_FrameBuffer(UINT16 src_index,UINT16 dest_index)
  Author        :
  Description   :  for three buffer frame copy

  Input         : src and dest index; you can use LastFrameIndex as src index,
                     LastFrameIndex is set in Lcd_Set_Current_BufferFrame_Index
  Return       : if index is the same, return ERROR .

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
int LCD_Copy_FrameBuffer(UINT16 src_index,UINT16 dest_index)
{
    //must be different index
    if (src_index==dest_index)
    {
        return -1;
    }

    if(src_index >= BUFFER_MAX_NUM)
    {
        src_index = BUFFER_MAX_NUM - 1;
    }

    if(dest_index >= BUFFER_MAX_NUM)
    {
        dest_index = BUFFER_MAX_NUM - 1;
    }

    //every pixel corresponding 2 Bytes;  you can use LastFrameIndex as src index
    memcpy(&frame_buffer[dest_index][0][0],
           &frame_buffer[src_index][0][0], LCD_WIDTH*LCD_HEIGHTA*2);
#if (FRAME_SUB_BUFFER_NUM > 1)
    memcpy(&frame_bufferB[dest_index][0][0],
           &frame_bufferB[src_index][0][0], LCD_WIDTH*LCD_HEIGHTB*2);
#endif


}



/*
--------------------------------------------------------------------------------
  Function name : void LCD_CREATE_UPPER_LAYER()
  Author        :
  Description   :

  Input         :
  Return       :

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_Create_Upper_Later()
{
    if (CurrentFrameIndex>=2||CurrentFrameIndex<0)
    {
        return;
    }

    LCD_Copy_FrameBuffer(CurrentFrameIndex,CurrentFrameIndex+1);
    LCD_Set_Current_BufferFrame_Index(CurrentFrameIndex+1);
    LCD_Buffer_Section_Refresh(0,LCD_HEIGHT);
}


/*
--------------------------------------------------------------------------------
  Function name : void LCD_Back_To_Under_Layer()
  Author        :
  Description   :

  Input         :
  Return       :

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_Back_To_Under_Layer()
{

    if (CurrentFrameIndex>=3||CurrentFrameIndex<=0)
    {
        return;
    }
    LCD_Set_Current_BufferFrame_Index(CurrentFrameIndex-1);
    LCD_Buffer_Section_Refresh(0,LCD_HEIGHT-1);
}

/*
--------------------------------------------------------------------------------
  Function name : int LCD_Buffer_Gray_Scale(UINT16 level)
  Author        :
  Description   :  for three buffer frame gray scale

  Input         : gray level(only has level 1,2,3)
  Return       : if level is not in 1,2,3, return ERROR .

  History:     <author>         <time>         <version>

  desc:
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
int LCD_Buffer_Gray_Scale(UINT16 frame_index,UINT16 level)
{

    int i, j;

    if(frame_index >= BUFFER_MAX_NUM)
        frame_index = BUFFER_MAX_NUM - 1;

    switch (level)
    {
        case 1:
            RGB_Gray_Scale_Mask = RGB_Gray_Scale_Mask_Lev1;
            break;

        case 2:
            RGB_Gray_Scale_Mask = RGB_Gray_Scale_Mask_Lev2;
            break;

        case 3:
            RGB_Gray_Scale_Mask = RGB_Gray_Scale_Mask_Lev3;
            break;

        case 4:
            RGB_Gray_Scale_Mask = RGB_Gray_Scale_Mask_Lev4;
            break;

        case 5:
            RGB_Gray_Scale_Mask = RGB_Gray_Scale_Mask_Lev5;
            break;

        default:
            //not in the range
            return -1;

    }

    // every pixel
    for (i=0;i<LCD_HEIGHTA;i++)
    {
        for (j=0;j<LCD_WIDTH;j++)
        {
            //first use RGB_Gray_Scale_Mask to execute bit AND
            frame_buffer[frame_index][i][j] &= RGB_Gray_Scale_Mask;
            //the shift right 1 bit
            frame_buffer[frame_index][i][j] >>= level;
        }
    }

    #if (FRAME_SUB_BUFFER_NUM > 1)
    for (i=0;i<LCD_HEIGHTB;i++)
    {
        for (j=0;j<LCD_WIDTH;j++)
        {
            //first use RGB_Gray_Scale_Mask to execute bit AND
            frame_bufferB[frame_index][i][j] &= RGB_Gray_Scale_Mask;
            //the shift right 1 bit
            frame_bufferB[frame_index][i][j] >>= level;
        }
    }
    #endif

    return 0;

    //then shift one bit
}




/*
--------------------------------------------------------------------------------
  Function name : void LCD_Buffer_Section_Refresh(UINT16 pYim,UINT16 pYmax)
  Author        :
  Description   : refresh the section from Ymin to Ymax
                        The Refresh Flag will be set as used in LCD_BufferTransfer,
                        and will not jump to skip in LCD_BufferTransfer.
                        Ymin and Ymax is a condition to refresh the buffer_frame[][][] in LCD_BufferTransfer.
                        Each time you refresh the buffer, it will execute the skip function module.
   Input         :
   Return        :

  History:     <author>         <time>         <version>

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_Buffer_Section_Refresh(UINT16 pYmin,UINT16 pYmax)
{

    if (pYmax > LCD_HEIGHT)
    {
        pYmax = LCD_HEIGHT;
    }

    if (pYmin < 0)
    {
        pYmin = 0;
    }

    Refresh = 1;
    Ymin      = pYmin;
    Ymax     = pYmax;
}

/*
--------------------------------------------------------------------------------
  Function name : UINT16 LCD_Get_Current_Index()
  Author        :
  Description   :

   Input         :
   Return        :

  History:     <author>         <time>         <version>

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
UINT16 LCD_Get_Current_Index()
{
    return CurrentFrameIndex;
}

/*
--------------------------------------------------------------------------------
  Function name : void LCD_Shift_BufferFrame(UINT16 frame_index,UINT16 direction, UINT16 distance)
  Author        :
  Description   : shift exact buffer frame with direction and distance

   Input         :
   Return        :

  History:     <author>         <time>         <version>

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_Shift_BufferFrame(UINT16 frame_index,BUFFER_SHIFT_DIRECTION  direction, UINT16 distance)
{
    LCD_Shift_Window(frame_index,direction,distance,0,0,LCD_WIDTH-1,LCD_HEIGHT-1);
}


/*
--------------------------------------------------------------------------------
  Function name : void LCD_Shift_Window(UINT16 frame_index,UINT16 direction, UINT16 distance,UINT16 x0,UINT16 y0,UINT16 x1,UINT16 y1)
  Author        :
  Description   : shift exact buffer frame with direction and distance

   Input         :
   Return        :

  History:     <author>         <time>         <version>

  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_LCD_CODE_
void LCD_Shift_Window(UINT16 frame_index,BUFFER_SHIFT_DIRECTION  direction, UINT16 distance,UINT16 x0,UINT16 y0,UINT16 x1,UINT16 y1)
{
    uint16 x,y;
    uint16 lcd_width;
    uint16 Xmin, Xmax;
    //get Ymax and Ymin
    Lcd_SetWindow(x0,y0,x1,y1);

    if (x1 > x0)
    {
        Xmax = x1;
        Xmin = x0;
    }
    else
    {

        Xmax = x0;
        Xmin = x1;
    }

    lcd_width = Xmax-Xmin;

//DEBUG("frameindex = %d ",frame_index);
    //param distance hasn't been designed
    switch (direction)
    {
            //all of these for case should include the Ymax and Ymin Line
        case DIRECTION_DOWN:
            for (y=y1;y >= (distance + y0) ;y--)
            {
                if (y < LCD_HEIGHTA)
                {
                    memcpy(&frame_buffer[frame_index][y][Xmin],&frame_buffer[frame_index][y-distance][Xmin],lcd_width*2);
                }
                #if (FRAME_SUB_BUFFER_NUM > 1)
                else
                {
                    if ((y - distance) < LCD_HEIGHTA)
                    {
                        memcpy(&frame_bufferB[frame_index][y - LCD_HEIGHTA][Xmin],&frame_buffer[frame_index][y - distance][Xmin],lcd_width*2);
                    }
                    else
                    {
                        memcpy(&frame_bufferB[frame_index][y - LCD_HEIGHTA][Xmin],&frame_bufferB[frame_index][(y - LCD_HEIGHTA) - distance][Xmin],lcd_width*2);
                    }
                }
                #endif
            }

            break;

        case DIRECTION_UP:
            for (y=(distance + y0); y<=y1 ;y++)
            {
                if (y < LCD_HEIGHTA)
                {
                    memcpy(&frame_buffer[frame_index][y-distance][Xmin],&frame_buffer[frame_index][y][Xmin],lcd_width*2);
                }
                #if (FRAME_SUB_BUFFER_NUM > 1)
                else
                {
                    if ((y - distance) < LCD_HEIGHTA)
                    {
                        memcpy(&frame_buffer[frame_index][y-distance][Xmin],&frame_bufferB[frame_index][y-LCD_HEIGHTA][Xmin],lcd_width*2);
                    }
                    else
                    {
                        memcpy(&frame_bufferB[frame_index][(y-LCD_HEIGHTA)-distance][Xmin],&frame_bufferB[frame_index][y-LCD_HEIGHTA][Xmin],lcd_width*2);
                    }
                }
                #endif
            }

            break;

        case DIRECTION_LEFT:
            DEBUG("DIRECTION_LEFT");
            for (y=0; y<=Ymax;y++)
            {
                //since x coordinate will exchange every 2 bit, I try shift 2 pixel as the minimum unit
                if (y < LCD_HEIGHTA)
                {
                    memcpy(&frame_buffer[frame_index][y][Xmin],&frame_buffer[frame_index][y][distance*2+Xmin],lcd_width*2-distance*2);
                }
                #if (FRAME_SUB_BUFFER_NUM > 1)
                else
                {
                    memcpy(&frame_bufferB[frame_index][y-LCD_HEIGHTA][Xmin],&frame_bufferB[frame_index][y-LCD_HEIGHTA][distance*2+Xmin],lcd_width*2-distance*2);
                }
                #endif
            }
            break;

        case DIRECTION_RIGHT:
            DEBUG("DIRECTION_RIGHT");
            for (y=0; y<= Ymax;y++)
            {
                //since x coordinate will exchange every 2 bit, I try shift 2 pixel as the minimum unit
                if (y < LCD_HEIGHTA)
                {
                    memcpy(&frame_buffer[frame_index][y][distance*2+Xmin],&frame_buffer[frame_index][y][Xmin],lcd_width*2-distance*2);
                }
                #if (FRAME_SUB_BUFFER_NUM > 1)
                else
                {
                    memcpy(&frame_bufferB[frame_index][y-LCD_HEIGHTA][distance*2+Xmin],&frame_bufferB[frame_index][y-LCD_HEIGHTA][Xmin],lcd_width*2-distance*2);
                }
                #endif
            }
            break;

        default:
            break;
    }
}
#endif
/*
********************************************************************************
*
*                         End of LcdInterface.c
*
********************************************************************************
*/
