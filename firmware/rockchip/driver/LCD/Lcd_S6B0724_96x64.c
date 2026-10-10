/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   Lcd.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             yangwenjie      2009-1-15          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_Lcd_

#include "DriverInclude.h"
#include "LcdInclude.h"
#define  UC1604C_column_offset  96
#define  UC1604C_LCD_BUF_XSIZE  96
#define  UC1604C_LCD_BUF_YSIZE  8

_ATTR_LCDDRIVER_UC1604C_DATA_ UINT8  UC1604C_Buffer[UC1604C_LCD_BUF_XSIZE][UC1604C_LCD_BUF_YSIZE];

/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_Command(unsigned int cmd)
  Author        : yangwenjie
  Description   :向LCD传输命令字
                  
  Input         : cmd LCD命令字
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/ 
_ATTR_LCDDRIVER_UC1604C_CODE_
void UC1604C_Command(UINT16 cmd)
 {

     Lcd->cmd = cmd;
 }


//++++++++++++++++++++++++++++++++++++




//++++++++++++++++++++++++++++++++++++
/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_Data(UINT16 data)
  Author        : yangwenjie
  Description   :向LCD传输显示数据
                  
  Input         : data LCD数据
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_ 
 void UC1604C_Data(UINT16 data)
 {
    Lcd->data = data;
 }
 
/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_WriteReg(UINT16 addr, UINT16 data)
  Author        : yangwenjie
  Description   :向LCD传输数据和命令
                  
  Input         : data LCD数据
                  addr LCD命令寄存器地址
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_ 
 void UC1604C_WriteReg(UINT16 addr, UINT16 data)
 {
     UC1604C_Command(addr);
     UC1604C_Data(data);
 }
 
/*
--------------------------------------------------------------------------------
  Function name : void EMCTL_Config(void)
  Author        : yangwenjie
  Description   : 配置LCD的刷屏时间
                  
  Input         : 无
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_ 
void UC1604C_WaitSet(UINT32 data )
 {
    //emctl->LCDWait = data;
 }
 /*
--------------------------------------------------------------------------------
  Function name : void Lcd_WriteRAM_Prepare(void)
  Author        : yangwenjie
  Description   :LCD数据传输使能
                  
  Input         : 无
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_ 
  void UC1604C_WriteRAM_Prepare(void)
 {   
    UC1604C_Command(0x2c);
 }
  /*
--------------------------------------------------------------------------------
  Function name :  void Lcd_Init(void)
  Author        : yangwenjie
  Description   : 初始化LCD
                  
  Input         : 无
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_
 void UC1604C_Init(void)
 { 

//    IoMuxSetLCD(LCD_MODE);

//    Lcd_WaitSet(0x000011d1);
	

#if 1
    // reg 18
    UC1604C_Command(0xE2); //System Reset 11100010
    DelayMs(10);
    //-------------------------------------------
    //Lcd_Command(0xF8); //MTP READ ENABLE
    //Lcd_Command(0x19); //
    //DelayMs(120);       //Delay 100mS// ADD 100 MS
    //-----------------------------------
    // reg 5   reg 6
    UC1604C_Command(0x2E);//Set Power Control: PC[2:0]=110
    UC1604C_Command(0x25);//Set Temperature Compensation  TC[1:0]=01(-0.05%)

    // DelayMs(10);

    //Lcd_Command(0x8C); //Set RAM Address Control:AC[2:0]=100
    //reg 9
    UC1604C_Command(0xB0);//Set Page Address PA[3:0]=0000

    //reg 4
    UC1604C_Command(0x00);//Set Column Address   LSB CA[3:0]=0000
    UC1604C_Command(0x16); //MSB CA[7:4]=0000//10

    //reg 21
    UC1604C_Command(0xEB);//Set LCD Bias Ratio BR[1:0]=11(1/9)

    //reg 10
    UC1604C_Command(0x81); //Set Vbias Potentiometer
    UC1604C_Command(0x70);//PM[7:0]=70H

    UC1604C_Command(0xA1);//Set Frame Rate   LC[4:3]=01(95fps); 00(76fps); 10(132fps); 11(168fps)

    UC1604C_Command(0xA6);//Set Inverse Display  DC0=0
    UC1604C_Command(0xC6);//Set LCD Mapping Control  MY=1, MX=1

    UC1604C_Command(0xF1);//Set COM End
    UC1604C_Command(0x3F);//CEN[5:0]=3FH


    UC1604C_Command(0xA4);//Set All Pixel ON,DC1=0
    UC1604C_Command(0xAf); //Set Display Enable  DC2=1

    DelayMs(10);
#else
    Lcd_Command(0xE2); //System Reset
    DelayMs(10);
    //-------------------------------------------
    Lcd_Command(0xF8); //MTP READ ENABLE
    Lcd_Command(0x19); //
    DelayMs(120);       //Delay 100mS// ADD 100 MS
    //-----------------------------------
    Lcd_Command(0x2E);//Set Power Control: PC[2:0]=110
    Lcd_Command(0x25);//Set Temperature Compensation  TC[1:0]=01(-0.05%)

    // DelayMs(10);

    //Lcd_Command(0x8C); //Set RAM Address Control:AC[2:0]=100
    Lcd_Command(0xB0);//Set Page Address PA[3:0]=0000

    Lcd_Command(0x00);//Set Column Address   LSB CA[3:0]=0000
    Lcd_Command(0x10); //MSB CA[7:4]=0000//10

    Lcd_Command(0xEB);//Set LCD Bias Ratio BR[1:0]=11(1/9)
    Lcd_Command(0x81); //Set Vbias Potentiometer
    Lcd_Command(0x72);//PM[7:0]=70H

    Lcd_Command(0xA1);//Set Frame Rate   LC[4:3]=01(95fps); 00(76fps); 10(132fps); 11(168fps)

    Lcd_Command(0xA6);//Set Inverse Display  DC0=0
    Lcd_Command(0xC6);//Set LCD Mapping Control  MY=0,MX=0

    Lcd_Command(0xF1);//Set COM End
    Lcd_Command(0x3F);//CEN[5:0]=3FH


    Lcd_Command(0xA4);//Set All Pixel ON,DC1=0
    Lcd_Command(0xAf); //Set Display Enable  DC2=1

    DelayMs(10);
#endif

  

#if 0
     //------------------Soft ware Re se t-------------------------//       
    Lcd_Command(0xE2);// Soft Reset

    DelayMs(10);       //Delay 100mS

    Lcd_Command(0xa2);              //bias select    1/9
    Lcd_Command(0xa1);              //ADC select(s131-s0)
  //Lcd_Command(0xc8);              //SHL select(com0-com31) 
    Lcd_Command(0xc0);              //SHL select(com0-com31) 

  //Lcd_Command(0x40);              //Start Line
        
  //Lcd_Command(0x25);               // 1+Ra/Rb=5.5	//IVAN101125
    Lcd_Command(0x25); 
    Lcd_Command(0x81);              //set EVR
    DelayMs(10);                    //100   
    Lcd_Command(0x13);              //+5-(4-gSysConfig.BLevel)*2);              //(ContrastSelect)  14 //sen #20100701#2

    Lcd_Command(0x2f);              //POWER control set

    Lcd_Command(0xaf);              //display on
    Lcd_Command(0xa4);              //entire display off

    DelayMs(10);
#endif
 }

/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_SendData(UINT16 data)
  Author        : yangwenjie
  Description   : 发送LCD数据
                  
  Input         : x0,y0: 显示图形的起始坐标
                  x1,y1: 显示图形的终点坐标
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_ 
void UC1604C_SendData(UINT16 data)
{
    UC1604C_Data(data);   
}
 /*
--------------------------------------------------------------------------------
  Function name : void Lcd_SetCursor(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1)
  Author        : yangwenjie
  Description   : 设置显示坐标
                  
  Input         : x0,y0: 显示图形的起始坐标
                  x1,y1: 显示图形的终点坐标
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_  
 void UC1604C_SetWindow(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1)
 {  

 
	//wWidth=wHeight;//无用
       //x0+=19; // modify by Kitty 2010-03-23#1
	//Lcd_Command(0xb7-(y0/8));//设置页地址Page addresss，1个page 8个点高
	//Lcd_Command(  ( (x0>>4)&0x0F )  | 0x10);
	//Lcd_Command(x0&0x0F);
	 
 }
 /*
--------------------------------------------------------------------------------
  Function name : void Lcd_Clear(UINT16 color)
  Author        : yangwenjie
  Description   : 清屏
                  
  Input         : color： 清屏的颜色
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/

 


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ fjp
_ATTR_LCDDRIVER_UC1604C_CODE_
void UC1604C_SetPixel(UINT16 x, UINT16 y, UINT16 data)
{
#if 0
    LCD_SetDispAddr(x,y);
    LCD_Data(data);
#endif
    if(x>=UC1604C_LCD_BUF_XSIZE||y>=UC1604C_LCD_BUF_YSIZE*8)
        return;
    if(data)
        UC1604C_Buffer[x][(unsigned int )y/8]= UC1604C_Buffer[x][(unsigned int )y/8]|(0x01<<(y%8));
    else
        UC1604C_Buffer[x][(unsigned int )y/8]= UC1604C_Buffer[x][(unsigned int )y/8]&(~(0x01<<(y%8)));
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ fjp
_ATTR_LCDDRIVER_UC1604C_CODE_
//void LCD_Buffer_Display1(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1)
void UC1604C_Buffer_Display1(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1)

{
    unsigned int  x,y,page_,x0_temp;
    page_=0xB0;
    x0_temp = UC1604C_column_offset;
    for (y=0;y<UC1604C_LCD_BUF_YSIZE;y++)
    {
#if(LCD_XL_UC1604)
    	UC1604C_Command(0x10+(x0_temp&0xf0)/16);
    	UC1604C_Command(0x00+(x0_temp&0x0f));
#else
    	UC1604C_Command(0x10+(x0_temp&0x7f)/16);
    	UC1604C_Command(0x00+(x0_temp&0x7f)%16);
#endif
    	UC1604C_Command(page_);
        for (x=0;x<UC1604C_LCD_BUF_XSIZE;x++)
    	{
    	    UC1604C_Data(UC1604C_Buffer[UC1604C_LCD_BUF_XSIZE - x - 1][y]);
    	}
        page_++;
    }

}
_ATTR_LCDDRIVER_UC1604C_CODE_
void UC1604C_Buffer_Display(void)
{
    unsigned int  x,y,page_,x0_temp;
    page_=0xB0;
    x0_temp = UC1604C_column_offset;
    //LCD_Buffer_Display1(0,0,128,64);

    //LCD_Command(0x40);  //start line
    //LCD_Command(0xaf);

    for (y=0;y<UC1604C_LCD_BUF_YSIZE;y++)
    {
#if(LCD_XL_UC1604)
    	UC1604C_Command(0x10+(x0_temp&0xf0)/16);
    	UC1604C_Command(0x00+(x0_temp&0x0f));
#else
    	UC1604C_Command(0x10+(x0_temp&0x7f)/16);
    	UC1604C_Command(0x00+(x0_temp&0x7f)%16);
#endif
    	UC1604C_Command(page_);
        for (x=0;x<UC1604C_LCD_BUF_XSIZE;x++)
    	{
    	    UC1604C_Data(UC1604C_Buffer[UC1604C_LCD_BUF_XSIZE - x - 1][y]);
    	}
        page_++;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  fjp
_ATTR_LCDDRIVER_UC1604C_CODE_
void UC1604C_Buffer_Clr(void)
{
    UINT16 x,y;
    for (y=0;y<UC1604C_LCD_BUF_YSIZE;y++)
	    for (x=0;x<UC1604C_LCD_BUF_XSIZE;x++)
		{
		    UC1604C_Buffer[x][y]=0x00;
		}
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
--------------------------------------------------------------------------------
  Function name : void Lcd_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *src)
  Author        : yangwenjie
  Description   : DMA传输数据
                  
  Input         : x0,y0: 显示图形的起始坐标
                  x1,y1: 显示图形的终点坐标
                  pSrc：  显示图形的源地址
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_    
 void UC1604C_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *pSrc,pFunc CallBack)
 {
    UINT32 size;
    UINT32 *pDst; 
    eDMA_CHN channel; 
    DMA_CFGX DmaCfg = {CTLL_M2LCD_HALF,CFGL_M2LCD_SINGLE,CFGH_M2LCD_SINGLE,0};
    
    size = (x1 - x0) * (y1 - y0);
    
    if(size <= 0)
        return;
    size = size>>1;
    pDst = &(Lcd->data); 

  //  SSD1307_SetWindow(x0+1,y0, x1+1,y1); 
    channel = DmaGetChannel();
    if (channel != DMA_FALSE)
    {
        size = (((x1 + 1) - x0) * ((y1+1) - y0));
        DmaStart((uint32)(channel), (UINT32)(pSrc),(uint32)(&(Lcd->data)),size,&DmaCfg, CallBack);
//        return channel;
    }
 }



//+++++++++++++++++++++++++++++++++++++++++++++fjp
_ATTR_LCDDRIVER_UC1604C_CODE_
void UC1604C_ClrSrc()
{
    UC1604C_Buffer_Clr();
    UC1604C_Buffer_Display();
//LCD_Buffer_Display();
}
//++++++++++++++++++++++++++++++++++++++++++++++++

/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_Data(UINT16 data)
  Author        : yangwenjie
  Description   :让屏进入省电模式
                  
  Input         : data LCD数据
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_  
void UC1604C_Standby(void)
{
	UC1604C_Command(0xAE);

}
_ATTR_LCDDRIVER_UC1604C_CODE_  
void UC1604C_PowerOff(void)
{
	UC1604C_Command(0xAE);
	UC1604C_Command(0x28);
	UC1604C_Command(0xA5);	
//    Gpio_SetPinDirection(POWER_ON , GPIO_OUT);      //POWER_ON
 //   Gpio_SetPinLevel(POWER_ON,  GPIO_LOW);	
//	    DelayMs(5);
}
/*
--------------------------------------------------------------------------------
  Function name :  void Lcd_Data(UINT16 data)
  Author        : yangwenjie
  Description   :让屏进入省电模式
                  
  Input         : data LCD数据
  
  Return        : 无

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  注释:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_UC1604C_CODE_  
void UC1604C_WakeUp(void)
{
#if 0//(!LCD_XL_UC1604)
	Lcd_Command(0x28);
	Lcd_Command(0xA5);
#endif	
   UC1604C_Command(0xAF);
}

//sen #20100701#2 s
_ATTR_LCDDRIVER_UC1604C_CODE_  
void UC1604CSetContrast(uint8 Level)
{
   //DisplayTestDecNum(2,2,Level);

#if(LCD_XL_UC1604)
   UC1604C_Command(0x81);                               //set EVR
   DelayMs(10); 
   
	if(Level == 2)
		UC1604C_Command(0x6E);
	//else if(Level == 4)
		//Lcd_Command(0x62);
	else
		UC1604C_Command(127-Level*8);              //(ContrastSelect)  14 
#else
   UC1604C_Command(0x81);                               //set EVR
   DelayMs(10);  
   UC1604C_Command(0x13+4-Level*2);              //(ContrastSelect)  14 
#endif
}
//sen #20100701#2 e

//fjp
//_ATTR_DRIVER_CODE_
//void LCDDEV_DrawHLine(int x0, int y, int x1)
//{
//	unsigned int data=0x01 ;
//	unsigned int i;
//   for (i=x0;i<x1;i++)
//   	LCD_SetPixel(i,y,data);
//   LCD_Buffer_Display1(x0,y,x1,y);
//}
//_ATTR_DRIVER_CODE_
//void LCDDEV_DrawVLine(int x, int y0, int y1)
//{
//	unsigned int data=0x01 ;
//	unsigned int i;
//   for (i=y0;i<y1;i++)
//   	LCD_SetPixel(x,i,data);
//   LCD_Buffer_Display1(x,y0,x,y1);
//}
//_ATTR_DRIVER_CODE_
//void LCDDEV_ClrRect(int x0, int y0, int x1, int y1)
//{
//		int i,j;
//	for (i=x0;i<=x1;i++)
//		for(j=y0; j<=y1; j++)
//			LCD_SetPixel(i,j,0);
//		 // LCD_Buffer_Display();
//
//}
_ATTR_LCDDRIVER_UC1604C_CODE_
void UC1604C_ClrRect(int x0, int y0, int x1, int y1)
{
	int i,j;
//	for (i=x0;i<=x1;i++)
//		for(j=y0; j<=y1; j++)
//			LCD_SetPixel(i,j,0);
//		  	LCD_Buffer_Display();
	for(j=y0; j<=y1; j++)
        for (i=x0;i<=x1;i++)
			LCD_SetPixel(i,j,0);

    UC1604C_Buffer_Display1(x0,y0,x1,y1);

}
//_ATTR_DRIVER_CODE_
//void LCD_ClrRectBuf(int x0, int y0, int x1, int y1)
//{
//		int i,j;
//	for (i=x0;i<=x1;i++)
//		for(j=y0; j<=y1; j++)
//			LCD_SetPixel(i,j,0);
//		  //LCD_Buffer_Display();
//
//}


_ATTR_LCDDRIVER_UC1604C_CODE_
void UC1604C_DEV_FillRect(int x0, int y0, int x1, int y1)
{
	int i,j;
	for (i=x0;i<=x1;i++)
		for(j=y0; j<=y1; j++)
			LCD_SetPixel(i,j,1);
	   UC1604C_Buffer_Display1(x0,y0,x1,y1);

}
/*
********************************************************************************
*
*                         End of Lcd.c
*
********************************************************************************
*/
