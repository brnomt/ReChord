/*
********************************************************************************************
*
*                Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\hw_yuv2rgb.c
* Owner: WJR
* Date: 2014.11.12
* Time: 10:54:06
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.12     10:54:06   1.0
********************************************************************************************
*/

#include "SysInclude.h"
#ifdef SUPPORT_YUV

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "LcdInterface.h"
#include "vop.h"
#include "yuv2rgb.h"
#include "hw_vop.h"
#include "Dma.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOC_LIB_HW_YUV2RGB_READ_  
#define _CPU_NANOC_LIB_HW_YUV2RGB_WRITE_ 
#define _CPU_NANOC_LIB_HW_YUV2RGB_INIT_  
#define _CPU_NANOC_LIB_HW_YUV2RGB_SHELL_  

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
uint32 YUV_DmaFinish;
uint32 YUV_DMA_CH = 0;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef USE_LLP
DMA_LLP yuv_llpListn[1][VOP_HEIGNT];
extern pFunc   DmaIsrCallBack[DMA_CHN_MAX];
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: send_UVY_data
** Input:uint32 *src
** Return: void
** Owner:WJR
** Date: 2014.11.27
** Time: 17:34:19
*******************************************************************************/
_CPU_NANOC_LIB_HW_YUV2RGB_READ_
READ API void send_UVY_data(uint32 *src)
{
    int i,j;
    for(i=0;i<(VOP_HEIGNT/2);i++)
    {
        for(j = 0; j <(VOP_WIDTH / 4);j++)
    	{			
    		VopSendData(0, *src++);    		
    	}    	
    	for(j = 0; j <(VOP_WIDTH/ 4);j++)
    	{
    		VopSendData(0,*src++);
    	}	    	
    	for(j = 0; j <(VOP_WIDTH/ 4);j++)
 {
    		VopSendData(0, *src++);
    	}
    }
}
/*******************************************************************************
** Name: send_Y_UV_data
** Input:uint32 *src
** Return: void
** Owner:WJR
** Date: 2014.11.27
** Time: 17:34:19
*******************************************************************************/
_CPU_NANOC_LIB_HW_YUV2RGB_READ_
READ API void send_Y_UV_data(uint32 *pst_y,uint32 *pst_uv)
{
   int i,j;
   for(i=0;i<(VOP_HEIGNT/2);i++) 
    {
        for(j = 0; j <(VOP_WIDTH/4);j++)
    	{			
    		VopSendData(0, *pst_uv++);    		
    	}	
    	for(j = 0; j <(VOP_WIDTH/4);j++)
    	{
    		VopSendData(0, *pst_y++);
    	}
        for(j = 0; j <(VOP_WIDTH/4);j++)
    	{
    		VopSendData(0, *pst_y++);
    	}
    }
}
/*******************************************************************************
** Name: send_rgb_data
** Input:uint32 *src
** Return: void
** Owner:WJR
** Date: 2014.11.27
** Time: 17:34:19
*******************************************************************************/
_CPU_NANOC_LIB_HW_YUV2RGB_READ_
READ API void send_rgb_data(uint16 *src)
{
   int i,j;
   for(i=0;i<(VOP_HEIGNT);i++) 
    {
        for(j = 0; j <(VOP_WIDTH /2);j++)
    	{			
    		VopSendData(0, *src++);    		
    	}	
    	for(j = 0; j <(VOP_WIDTH/2);j++)
    	{
    		VopSendData(0, *src++);
    	}	
    }
}
/*******************************************************************************
** Name: YuvSetMode
** Input:uint32 width, uint32 Height, uint32 Version
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 19:18:23
*******************************************************************************/
_CPU_NANOC_LIB_HW_YUV2RGB_READ_
READ API rk_err_t YuvSetMode(uint32 RawFormat, uint32 Version)
{
   
    VopSetDisplayMode(0,RawFormat); //8
    VopSetSplit(0, VOP_CON_SPLIT_TWO);//  1
    VopSetUvswap(0,VOP_CON_YUV420_UV_KEEP);//9
    VopSetFifoMode(0, VOP_CON_NOT_BYPASS_FIFO); //11
    VopSetMcuIdle(0,VOP_CON_IDLE_INPUT);//0
    VopEnableClk(0,VOP_CON_AUTO_CLK_DISABLE);//12
    
    if(Version ==  YUV_VERSION_0)
    {
        VopSetYUVType(0, VOP_CON_YUV_BT601_L);
    }
    else if(Version == YUV_VERSION_1)
    {
        VopSetYUVType(0, VOP_CON_YUV_BT701_L);        
    }
    else if(Version == YUV_VERSION_2)
    {
        VopSetYUVType(0, VOP_CON_YUV_BT601_F);
    }
}

_CPU_NANOC_LIB_HW_YUV2RGB_READ_
void YUV_DmaIsr()
{
     YUV_DmaFinish = 1;   
     //rk_print_string1("\r\n yuv_DmaIsr\n");         
     DmaDisableInt(YUV_DMA_CH); 
}
/*******************************************************************************
** Name:  RgbWrite
** Input:uint32  VopId,uint32 Addr, uint32 width, uint32 Height
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.29
** Time: 17:07:11
*******************************************************************************/
#if 1
_CPU_NANOC_LIB_HW_YUV2RGB_READ_
READ API rk_err_t Rgb565Write(uint32  VopId,uint32 Addr, uint32 width, uint32 Height)
{
    int i ,j,total_size;
    VOP * pVop = VopPort(VopId);
    uint32 YUV_ACC_addr =(uint32)&pVop->VopMcuData ;
	  DMA_CFGX DmaCfg = {DMA_CTLL_VOP_WR, DMA_CFGL_VOP_WR, DMA_CFGH_VOP_WR, 0}; 
//    ScuClockGateCtr(CLOCK_GATE_DMA, 1);     //open uart clk
    ScuSoftResetCtr(SYSDMA_SRST, 1); //open rst uart ip
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);
    
    IntRegister(INT_ID_DMA ,DmaInt);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);
    
    YUV_DMA_CH = 0;
    {
        YUV_DmaFinish = 0;
        DmaEnableInt(YUV_DMA_CH);        
        DmaConfig(YUV_DMA_CH, Addr, YUV_ACC_addr,Height*width*2/4, &DmaCfg,YUV_DmaIsr,DmaIsrCallBack, yuv_llpListn[YUV_DMA_CH]);
        while(!YUV_DmaFinish);
        DmaDisableInt(YUV_DMA_CH); 
    }
}

_CPU_NANOC_LIB_HW_YUV2RGB_READ_
READ API rk_err_t YuvWrite(uint32  VopId,uint32 Addr, uint32 width, uint32 Height)
{
    int i ,j,total_size;
    VOP * pVop = VopPort(VopId);
    uint32 YUV_ACC_addr =(uint32)&pVop->VopMcuData ;
    DMA_CFGX DmaCfg = {DMA_CTLL_VOP_WR, DMA_CFGL_VOP_WR, DMA_CFGH_VOP_WR, 0};
    //ScuClockGateCtr(CLOCK_GATE_DMA, 1);     //open uart clk
    ScuSoftResetCtr(SYSDMA_SRST, 1); //open rst uart ip
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);
    
    IntRegister(INT_ID_DMA ,DmaInt);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);
        
    YUV_DMA_CH = 0;
    {
        YUV_DmaFinish = 0;
        DmaEnableInt(YUV_DMA_CH);
        DmaConfig(YUV_DMA_CH, Addr, YUV_ACC_addr,Height*width*3/8, &DmaCfg,YUV_DmaIsr,DmaIsrCallBack, yuv_llpListn[YUV_DMA_CH]);
        while(!YUV_DmaFinish);
        DmaDisableInt(YUV_DMA_CH); 
    }
}
#endif


#ifdef USE_LLP
/*******************************************************************************
** Name: YuvLlpWrite
** Input:uint32  Yaddr, uint32 UvAddr, uint32 width, uint32 Height
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 19:10:21
*******************************************************************************/
_CPU_NANOC_LIB_HW_YUV2RGB_READ_
READ API rk_err_t YuvLlpWrite(uint32  VopId,uint32  Yaddr,uint32 UvAddr, uint32 width, uint32 Height)
{
    int i = 0,j;
    VOP * pVop = VopPort(VopId);
	 DMA_CFGX DmaCfg = {DMA_CTLL_VOP_WR, DMA_CFGL_VOP_WR, DMA_CFGH_VOP_WR, 0}; 
    uint32 YUV_ACC_addr = (uint32)&pVop->VopMcuData ;
    //ScuClockGateCtr(CLOCK_GATE_DMA, 1);
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);    
    IntRegister(INT_ID_DMA ,YUV_DmaIsr);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);
         
    YUV_DMA_CH = 0;    
    YUV_DmaFinish = 0;
    DmaEnableInt(YUV_DMA_CH);    
    for(i=0;i<Height/2;i++)
    {
            yuv_llpListn[YUV_DMA_CH][2*i].SAR = UvAddr;
            yuv_llpListn[YUV_DMA_CH][2*i].DAR = YUV_ACC_addr;
            yuv_llpListn[YUV_DMA_CH][2*i].SIZE = width/4; 
            UvAddr += width;
            yuv_llpListn[YUV_DMA_CH][2*i+1].SAR = Yaddr;
            yuv_llpListn[YUV_DMA_CH][2*i+1].DAR = YUV_ACC_addr ;
            yuv_llpListn[YUV_DMA_CH][2*i+1].SIZE = width/2;
            Yaddr += width*2;
    }
    DmaConfig_for_LLP(YUV_DMA_CH, width/2, Height,&DmaCfg, yuv_llpListn[YUV_DMA_CH]);
	while(!YUV_DmaFinish)
    {
        __WFI();
    };
    DmaDisableInt(YUV_DMA_CH);   
}
#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: hw_yuv2rgb_shell
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.12
** Time: 14:23:26
*******************************************************************************/
_CPU_NANOC_LIB_HW_YUV2RGB_SHELL_
uint32 yuv;

void YuvIsr()
{
    yuv = 1;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

