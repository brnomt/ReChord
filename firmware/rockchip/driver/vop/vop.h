/*
********************************************************************************************
*
*				 Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*							   All rights reserved.
*
* FileName: Cpu\NanoC\lib\vop.h
* Owner: aaron.sun
* Date: 2014.11.21
* Time: 14:43:48
* Desc: vop bsp driver
* History:
*    <author>	 <date> 	  <time>	 <version>	   <Desc>
*    aaron.sun     2014.11.21     14:43:48   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOC_LIB_VOP_H__
#define __CPU_NANOC_LIB_VOP_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define									 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define						 
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define VOP_CON_IDLE_INPUT          (uint32)(0x00 << 0)
#define VOP_CON_IDLE_OUTPUT         (uint32)(0x01 << 0)

#define VOP_CON_SPLIT_ONE           (uint32)(0x00 << 1)
#define VOP_CON_SPLIT_TWO           (uint32)(0x01 << 1)
#define VOP_CON_SPLIT_THREE         (uint32)(0x02 << 1)
#define VOP_CON_SPLIT_FOUR          (uint32)(0x03 << 1)
#define VOP_CON_SPLIT_MASK          (uint32)(0x03 << 1)

#define VOP_CON_DATA_WIDTH_8        (uint32)(0x00 << 3)
#define VOP_CON_DATA_WIDTH_16       (uint32)(0x01 << 3)

#define VOP_CON_HWORD_NO_SWAP       (uint32)(0x00 << 4)
#define VOP_CON_HWORD_SWAP          (uint32)(0x01 << 4)

#define VOP_CON_BYTE_NO_SWAP        (uint32)(0x00 << 5)
#define VOP_CON_BYTE_SWAP           (uint32)(0x01 << 5)

#define VOP_CON_YUV_BT601_L         (uint32)(0x00 << 6)
#define VOP_CON_YUV_BT701_L         (uint32)(0x01 << 6)
#define VOP_CON_YUV_BT601_F         (uint32)(0x02 << 6)
#define VOP_CON_YUV_MASK            (uint32)(0x03 << 6)

#define VOP_CON_FORMAT_RGB565       (uint32)(0x00 << 8)
#define VOP_CON_FORMAT_YUV420       (uint32)(0x01 << 8)
#define VOP_CON_FORMAT_MASK         (uint32)(0x01 << 8)

#define VOP_CON_YUV420_UV_KEEP      (uint32)(0x00 << 9)
#define VOP_CON_YUV420_UV_SWAP      (uint32)(0x01 << 9)

#define VOP_CON_DITHER_UP           (uint32)(0x00 << 10)
#define VOP_CON_DITHER_DWON         (uint32)(0x01 << 10)

#define VOP_CON_NOT_BYPASS_FIFO     (uint32)(0x00 << 11)
#define VOP_CON_BYPASS_FIFO         (uint32)(0x01 << 11)
#define VOP_CON_FIFO_MODE_MASK      (uint32)(0x01 << 11)

#define VOP_CON_AUTO_CLK_DISABLE    (uint32)(0x00 << 12)
#define VOP_CON_AUTO_CLK_EN         (uint32)(0x01 << 12)


#define VOP_INT_TYPE_FIFO_FULL      (uint32)(0x01 << 2)
#define VOP_INT_TYPE_FIFO_EMPTY     (uint32)(0x01 << 1)
#define VOP_INT_TYPE_FRAME_DONE     (uint32)(0x01 << 0)

#define VOP_INT_TYPE_RAW_FIFO_FULL  (uint32)(0x01 << 5)
#define VOP_INT_TYPE_RAW_FIFO_EMPTY (uint32)(0x01 << 4)
#define VOP_INT_TYPE_RAW_FRAME_DONE (uint32)(0x01 << 3)






/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare							 
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare          							 
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t VopSetMcu_FIFO_WATERMARK(uint32 VopId,int full_watermark,int empty_watermark);
extern rk_err_t VopSetStart(uint32 VopId ,uint32 mode);
extern rk_err_t VopEnableClk(uint32 VopId,uint32 IntType);
extern rk_err_t VopSetMcuIdle(uint32 VopId,uint32 idleFlag);
extern uint32 VopIntClear(uint32 VopId);
extern rk_err_t VopReset(uint32 VopId);
extern rk_err_t VopSetUvswap(uint32 VopId,uint32 mode);
extern rk_err_t VopSetFifoMode(uint32 VopId, uint32 fifomode);
extern rk_err_t VopDisableInt(uint32 VopId, uint32 IntType);
extern rk_err_t VopEnableInt(uint32 VopId, uint32 IntType);
extern uint32  VopGetInt(uint32 VopId);
extern rk_err_t VopSetYUVType(uint32 VopId, uint32 YuvType);
extern rk_err_t VopSetDisplayMode(uint32 VopId, uint32 DisplayMode);
extern rk_err_t VopSetSplit(uint32 VopId, uint32 mode);
extern rk_err_t VopSetWidthHeight(uint32 VopId, uint32 Width, uint32 Height);
extern rk_err_t VopSetTiming(uint32 VopId, uint32 csrw, uint32 rwpw, uint32 rwcs);
extern rk_err_t VopReadData(uint32 VopId, uint32 * data);
extern rk_err_t VopReadCmd(uint32 VopId, uint32 * cmd);
extern rk_err_t VopSendData(uint32 VopId, uint32 data);
extern rk_err_t VopSendCmd(uint32 VopId, uint32 cmd);
extern rk_err_t VopSetMode(uint32 VopId, uint32 mode);
extern rk_err_t VopSetDitherMode(uint32 VopId, uint32 DitherMode);
extern rk_err_t VopSetHWMode(uint32 VopId, uint32 HWMode);

#endif

