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
#if 0
#define _IN_Lcd_

#include "DriverInclude.h"

_ATTR_LCDDRIVER_JWM12864_CODE_ UINT8  JWM12864_Buffer[LCD_BUF_XSIZE][LCD_BUF_YSIZE];
/*
--------------------------------------------------------------------------------
  Function name :  void JWM12864_Command(unsigned int cmd)
  Author        : yangwenjie
  Description   :send command to lcd
                  
  Input         : cmd LCD word
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/ 
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_Command(UINT16 cmd)
 {

     Lcd->cmd = cmd;
 }


//++++++++++++++++++++++++++++++++++++




//++++++++++++++++++++++++++++++++++++
/*
--------------------------------------------------------------------------------
  Function name :  void JWM12864_Data(UINT16 data)
  Author        : yangwenjie
  Description   :send display data to lcd.
                  
  Input         : data -- LCD data
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_JWM12864_CODE_ 
 void JWM12864_Data(UINT16 data)
 {
    Lcd->data = data;
 }
 
/*
--------------------------------------------------------------------------------
  Function name :  void JWM12864_WriteReg(UINT16 addr, UINT16 data)
  Author        : yangwenjie
  Description   : send command and data to LCD.
                  
  Input         : data -- LCD data
                  addr -- LCD command register address.
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_JWM12864_CODE_ 
 void JWM12864_WriteReg(UINT16 addr, UINT16 data)
 {
     JWM12864_Command(addr);
     JWM12864_Data(data);
 }
 
/*
--------------------------------------------------------------------------------
  Function name : void EMCTL_Config(void)
  Author        : yangwenjie
  Description   : config the LCD refresh time. 
                  
  Input         : null
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_JWM12864_CODE_ 
void JWM12864_WaitSet(UINT32 data )
 {
    //emctl->LCDWait = data;
 }
 /*
--------------------------------------------------------------------------------
  Function name : void JWM12864_WriteRAM_Prepare(void)
  Author        : yangwenjie
  Description   : to enable LCD data transfer.
                  
  Input         : null
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_JWM12864_CODE_ 
  void JWM12864_WriteRAM_Prepare(void)
 {   
    JWM12864_Command(0x2c);
 }
  /*
--------------------------------------------------------------------------------
  Function name :  void JWM12864_Init(void)
  Author        : yangwenjie
  Description   : LCD initialization.
                  
  Input         : null
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_JWM12864_CODE_
 void JWM12864_Init(void)
 { 
    JWM12864_WaitSet(0x000011d1);
    //------------------Soft ware Re se t-------------------------//       

    DelayMs(10);       //Delay 100mS

    Lcd_SplitMode(LCD_SPLIT1);
    
    //st7565r_ini
    JWM12864_Command(0xE2);
    //   LCD_Data(0);   // Soft Reset
    DelayMs(10);//2850
    //Delay(52850);
    //     WriteData(0,0xAE);
    //      Delay(100);
    JWM12864_Command(0xA3);
    //      LCD_Data(0);   //LCD Bias Select 1/9
    DelayMs(10);//1000
    JWM12864_Command(0xA1);
    //      LCD_Data(0);    //ADC Select   SEG0-SEG131
    DelayMs(10);//1000
    JWM12864_Command(0xC8);
    //     LCD_Data(0);    //SHL Select   COM0-COM63
    DelayMs(10);//100
    JWM12864_Command(0x26);
    //      LCD_Data(0);   //Regulator Select 0x20-0x27  对比度调节参数一
    DelayMs(10);//5000
    JWM12864_Command(0x81);
    //    LCD_Data(0);   //Set Reference Voltage Select Mode
    DelayMs(10);//100
    JWM12864_Command(0x00); 
    //      LCD_Data(0);   //Set Reference Voltage Register 0x00-0x3F 对比度调节参数二
    DelayMs(10);//10000

    JWM12864_Command(0xf8); 

    DelayMs(10);//5000
    JWM12864_Command(0x00); 

    DelayMs(10);//5000
    JWM12864_Command(0x2f); 

    DelayMs(10);//5000

    JWM12864_Command(0x40); 

    DelayMs(10);//100

    JWM12864_Command(0xaf); 

    DelayMs(10);

    //clear buffer
    {
        uint16 x,y, page_;
        page_=0xB0;
        for (y = 0; y < 64/8; y++)
    	{

    		JWM12864_Command(0x10);
            JWM12864_Command(0x02);
    	    JWM12864_Command(page_);
        	for (x = 0; x < 128; x++)
    		{
    		    JWM12864_Data(0x00);
    		}
        	page_++;
        }
    }

 }

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_JWM12864_CODE_ 
void JWM12864_SendData(UINT16 data)
{
    JWM12864_Data(data);   
}
 /*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_JWM12864_CODE_  
 void JWM12864_SetWindow(UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1)
 {   
	JWM12864_Command(0x40|(x0));
	JWM12864_Command(0x80|(y0+6));
	 
 }
 /*
--------------------------------------------------------------------------------
  Function name : void JWM12864_Clear(UINT16 color)
  Author        : yangwenjie
  Description   : clear the screen
                  
  Input         : color：clear lcd to the color.
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/

 


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ fjp
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_SetPixel(UINT16 x, UINT16 y, UINT16 data)
{
#if 0
    LCD_SetDispAddr(x,y);
	LCD_Data(data);
#endif
    if(x>=LCD_BUF_XSIZE||y>=LCD_BUF_YSIZE*8)
        return;
    if(data)
        JWM12864_Buffer[x][(unsigned int )y/8]=JWM12864_Buffer[x][(unsigned int )y/8]|(0x01<<(y%8));
    else
        JWM12864_Buffer[x][(unsigned int )y/8]=JWM12864_Buffer[x][(unsigned int )y/8]&(~(0x01<<(y%8)));
	
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ fjp
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_Buffer_Display1(unsigned  int x0,unsigned int y0,unsigned int x1,unsigned int y1)
{//show line one by one

    UINT16  x,y,x0_temp,y0_temp,x1_temp,y1_temp,page_temp;
	x0_temp=x0;
	x1_temp=x1;
	page_temp=y0/8;
	y0_temp=page_temp*8;

    if(y1>=LCD_BUF_YSIZE*8)
	    y1=LCD_BUF_YSIZE*8-1;

	
	
    for (y=y0/8;y<=y1/8;y++)
	{
#if 1	  
		//JWM12864_Command(0x10);  //column addr upper
		JWM12864_Command(0x10);
        JWM12864_Command(0x02);
	    JWM12864_Command(0xb0+y);
#else
        JWM12864_Command(0x11);  //column addr upper
		JWM12864_Command(0x02);  //column addr lower
		JWM12864_Command(0xB0+y);  //page addr set
#endif
            
	    for (x=0;x<LCD_MAX_XSIZE;x++)
		{
		    JWM12864_Data(JWM12864_Buffer[x][y]);
		}
	
	    page_temp++;
	
	}
}
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_Buffer_Display(void)
{
    unsigned int  x,y,page_;
    //LCD_Buffer_Display1(0,0,128,64);

	//LCD_Command(0x40);  //start line
	//LCD_Command(0xaf);
 	page_=0xB0;
    for (y=0;y<LCD_BUF_YSIZE;y++)
	{

		JWM12864_Command(0x10);
        JWM12864_Command(0x02);
	    JWM12864_Command(page_);
	for (x=0;x<LCD_BUF_XSIZE;x++)
		{
		    JWM12864_Data(JWM12864_Buffer[x][y]);
		}
	page_++;
	}
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  fjp
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_Buffer_Clr(void)
{
    UINT16 x,y;
    for (y=0;y<LCD_BUF_YSIZE;y++)
        for (x=0;x<LCD_BUF_XSIZE;x++)
        {
            JWM12864_Buffer[x][y]=0x00;
        }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
--------------------------------------------------------------------------------
  Function name : void JWM12864_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *src)
  Author        : yangwenjie
  Description   : DMA data transfer
                  
  Input         : x0,y0: the start coordinate of display pictrue.
                  x1,y1: the end coordinate of display pictrue.
                  pSrc： the source address.
  
  Return        : null

  History:     <author>         <time>         <version>       
             yangwenjie     2008-1—15         Ver1.0
  desc:         ORG
  Note:          
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_JWM12864_CODE_    
 void JWM12864_DMATranfer (UINT8 x0,UINT8 y0,UINT8 x1,UINT8 y1,UINT16 *pSrc)
 {
    UINT32 size;
    UINT32 *pDst; 
   
    size = (x1 - x0) * (y1 - y0);
    
    if(size <= 0)
        return;
    size = size>>1;
    pDst = &(Lcd->data); 

  //  JWM12864_SetWindow(x0+1,y0, x1+1,y1); 
  //  DmaTransmit(1,(UINT32)(pSrc),(UINT32)(pDst),size,DMA_DRAM_TO_LCD,0);
 }



//+++++++++++++++++++++++++++++++++++++++++++++fjp
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_ClrSrc(void)
{
    JWM12864_Buffer_Clr();
    JWM12864_Buffer_Display();
}
//++++++++++++++++++++++++++++++++++++++++++++++++

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/
_ATTR_LCDDRIVER_JWM12864_CODE_  
void JWM12864_Standby(void)
{
  //  JWM12864_Command(0x10);
  JWM12864_Command(0xAE);
}

/*
*/
_ATTR_LCDDRIVER_JWM12864_CODE_  
void JWM12864_WakeUp(void)
{
   // JWM12864_Command(0x11);
   JWM12864_Command(0xAF);
}


//fjp
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_DEV_DrawHLine(int x0, int y, int x1)
{
	unsigned int data=0x01 ;
	unsigned int i;
    for (i=x0;i<x1;i++)
   	    JWM12864_SetPixel(i,y,data);
    JWM12864_Buffer_Display1(x0,y,x1,y);
}
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_DEV_DrawVLine(int x, int y0, int y1)
{
	unsigned int data=0x01 ;
	unsigned int i;
    for (i=y0;i<y1;i++)
   	    JWM12864_SetPixel(x,i,data);
    JWM12864_Buffer_Display1(x,y0,x,y1);
}
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_DEV_ClrRect(int x0, int y0, int x1, int y1)
{
    int i,j;
    for (i=x0;i<=x1;i++)
        for(j=y0; j<=y1; j++)
            JWM12864_SetPixel(i,j,0);
    JWM12864_Buffer_Display();

}
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_ClrRect(int x0, int y0, int x1, int y1)
{
    int i,j;
    for (i=x0;i<=x1;i++)
        for(j=y0; j<=y1; j++)
            JWM12864_SetPixel(i,j,0);
    JWM12864_Buffer_Display();

}
_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_ClrRectBuf(int x0, int y0, int x1, int y1)
{
    int i,j;
    for (i=x0;i<=x1;i++)
        for(j=y0; j<=y1; j++)
            JWM12864_SetPixel(i,j,0);
    JWM12864_Buffer_Display();

}


_ATTR_LCDDRIVER_JWM12864_CODE_
void JWM12864_DEV_FillRect(int x0, int y0, int x1, int y1)
{
    int i,j;
    for (i=x0;i<=x1;i++)
        for(j=y0; j<=y1; j++)
            JWM12864_SetPixel(i,j,1);
    JWM12864_Buffer_Display1(x0,y0,x1,y1);

}
#endif
/*
********************************************************************************
*
*                         End of Lcd.c
*
********************************************************************************
*/ 
