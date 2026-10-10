/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：  dma.h
*
* Description:
*
*
* History:      <author>          <time>        <version>
*               HuWeiGuo        2009-01-05         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _DMA_H_
#define _DMA_H_
#include "Hw_dma.h"   /* pDMA_LLP, B_CTLL_* (same dir) */

#undef  EXT
#ifdef _IN_DMA_
#define EXT
#else
#define EXT extern
#endif
/*
--------------------------------------------------------------------------------

                        Macro define

--------------------------------------------------------------------------------
*/
#define DMA_MAX_BLOCK_SIZE      4088        //do not changed.
#define LLP_BLOCK_SIZE  4088
#define LLP_MAX_NUM     8
//------------------------------------------------------------------------------
//M2M SINGLE
#define     DMA_CTLL_M2M_WORD (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                                |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                                |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                                |B_CTLL_MEM2MEM_DMAC \
                                |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_M2M_WORD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2M_WORD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//M2M LLP
#define     CTLL_M2M_LLP    (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0 \
                            |B_CTLL_LLP_DST_EN | B_CTLL_LLP_SRC_EN)

#define     CFGL_M2M_LLP (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     CFGH_M2M_LLP (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//M2LCD
#define     CTLL_M2LCD_HALF (B_CTLL_DST_TR_WIDTH_16 | B_CTLL_SRC_TR_WIDTH_16 \
                            |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     CTLL_M2LCD_WORD (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     CFGL_M2LCD_SINGLE (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     CFGH_M2LCD_SINGLE (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//M2IMDCT
#define     DMA_CTLL_M2IMDCT (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_M2IMDCT (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2IMDCT (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//IMDCT2M
#define     DMA_CTLL_IMDCT2M (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_IMDCT2M (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_IMDCT2M (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)


//M2SYNTH
#define     DMA_CTLL_M2SYNTH (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_M2SYNTH (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2SYNTH (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)


//SYNTH2M
#define     DMA_CTLL_SYNTH2M (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_SYNTH2M (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SYNTH2M (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//----------------------------------------------------------------------------------------------------//
//M2I2S_0 MSIZE不能使用16，否则I2S端接收错误?
#define     DMA_CTLL_I2S0_TX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                            |B_CTLL_MEM2PER_DMAC \
                            |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_I2S0_TX (B_CFGL_CH_PRIOR(2) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_I2S0_TX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(0))

//I2S_02M MSIZE不能使用16，否则I2S端接收错误?
#define     DMA_CTLL_I2S0_RX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                            |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                            |B_CTLL_PER2MEM_DMAC \
                            |B_CTLL_DMS_AM0 |B_CTLL_SMS_AM1)

#define     DMA_CFGL_I2S0_RX (B_CFGL_CH_PRIOR(2) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_I2S0_RX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(1))
//----------------------------------------------------------------------------------------------------//

//M2I2S_1 MSIZE不能使用16，否则I2S端接收错误?
#define     DMA_CTLL_I2S1_TX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                            |B_CTLL_MEM2PER_DMAC \
                            |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_I2S1_TX (B_CFGL_CH_PRIOR(2) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_I2S1_TX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(2))

//I2S_12M MSIZE不能使用16，否则I2S端接收错误?
#define     DMA_CTLL_I2S1_RX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                            |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                            |B_CTLL_PER2MEM_DMAC \
                            |B_CTLL_DMS_AM0 |B_CTLL_SMS_AM1)

#define     DMA_CFGL_I2S1_RX (B_CFGL_CH_PRIOR(2) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_I2S1_RX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(3))

//----------------------------------------------------------------------------------------------------//

//M2SPI_0 MASTER 8BIT
#define     DMA_CTLL_SPI0_8_MTX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8 \
                              |B_CTLL_MEM2PER_DMAC \
                              |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI0_8_MTX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI0_8_MTX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(4))


//SPI_02M MASTER 8BIT
#define     DMA_CTLL_SPI0_8_MRX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                              |B_CTLL_PER2MEM_DMAC \
                              |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI0_8_MRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI0_8_MRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(5))
//----------------------------------------------------------------------------------------------------//

//M2SPI_0 SLAVE 8BIT
#define     DMA_CTLL_SPI0_8_STX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_32 \
                              |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4 \
                              |B_CTLL_MEM2PER_DMAC \
                              |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI0_8_STX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI0_8_STX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(4))


//SPI_02M SLAVE 8BIT
#define     DMA_CTLL_SPI0_8_SRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                              |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                              |B_CTLL_PER2MEM_DMAC \
                              |B_CTLL_DMS_AM0 |B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI0_8_SRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI0_8_SRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(5))

//----------------------------------------------------------------------------------------------------//

//M2SPI_0 MASTER 16BIT
#define     DMA_CTLL_SPI0_16_MTX (B_CTLL_DST_TR_WIDTH_16 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI0_16_MTX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI0_16_MTX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(4))

//SPI_02M MASTER 16BIT
#define     DMA_CTLL_SPI0_16_MRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_16 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI0_16_MRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI0_16_MRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(5))
//----------------------------------------------------------------------------------------------------//

//M2SPI_0 SLAVE 16BIT
#define     DMA_CTLL_SPI0_16_STX (B_CTLL_DST_TR_WIDTH_16 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI0_16_STX   (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI0_16_STX   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(4))

//SPI_02M SLAVE 16BIT
#define     DMA_CTLL_SPI0_16_SRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_16 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI0_16_SRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI0_16_SRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(5))
//----------------------------------------------------------------------------------------------------//

//M2SPI_1 MASTER 8BIT
#define     DMA_CTLL_SPI1_8_MTX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8 \
                              |B_CTLL_MEM2PER_DMAC \
                              |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI1_8_MTX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI1_8_MTX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(6))


//SPI_12M MASTER 8BIT
#define     DMA_CTLL_SPI1_8_MRX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                              |B_CTLL_PER2MEM_DMAC \
                              |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI1_8_MRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI1_8_MRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(7))
//----------------------------------------------------------------------------------------------------//

//M2SPI_1 SLAVE 8BIT
#define     DMA_CTLL_SPI1_8_STX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_32 \
                              |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4 \
                              |B_CTLL_MEM2PER_DMAC \
                              |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI1_8_STX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI1_8_STX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(6))


//SPI_12M SLAVE 8BIT
#define     DMA_CTLL_SPI1_8_SRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                              |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                              |B_CTLL_PER2MEM_DMAC \
                              |B_CTLL_DMS_AM0 |B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI1_8_SRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI1_8_SRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(7))

//----------------------------------------------------------------------------------------------------//

//M2SPI_1 MASTER 16BIT
#define     DMA_CTLL_SPI1_16_MTX (B_CTLL_DST_TR_WIDTH_16 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI1_16_MTX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI1_16_MTX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(6))

//SPI_12M MASTER 16BIT
#define     DMA_CTLL_SPI1_16_MRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_16 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI1_16_MRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI1_16_MRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(7))
//----------------------------------------------------------------------------------------------------//

//M2SPI_1 SLAVE 16BIT
#define     DMA_CTLL_SPI1_16_STX (B_CTLL_DST_TR_WIDTH_16 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI1_16_STX   (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI1_16_STX   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(6))

//SPI_12M SLAVE 16BIT
#define     DMA_CTLL_SPI1_16_SRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_16 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI1_16_SRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI1_16_SRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(7))
//----------------------------------------------------------------------------------------------------//
//M2UART_02
#define     DMA_CTLL_M2UART_02 (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                            |B_CTLL_DINC_UNC| B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_4 \
                            |B_CTLL_MEM2PER_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_M2UART_02 (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2UART_02   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(8))


//UART_022M
#define     DMA_CTLL_UART_022M (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC\
                            |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_4 \
                            |B_CTLL_PER2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_UART_022M (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_UART_022M   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(9))
//----------------------------------------------------------------------------------------------------//
//M2UART_1
#define     DMA_CTLL_M2UART_1 (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                            |B_CTLL_DINC_UNC| B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_4 \
                            |B_CTLL_MEM2PER_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_M2UART_1 (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2UART_1   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(10))


//UART_12M
#define     DMA_CTLL_UART_12M (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC\
                            |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_4 \
                            |B_CTLL_PER2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_UART_12M (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_UART_12M   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(11))
//----------------------------------------------------------------------------------------------------//
//M2SDMMC
#define     DMA_CTLL_SDMMC_WR  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SDMMC_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SDMMC_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(12))

//SDMMC2M
#define     DMA_CTLL_SDMMC_RD  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_SDMMC_RD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SDMMC_RD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(12))
//----------------------------------------------------------------------------------------------------//
//M2EMMC
#define     DMA_CTLL_EMMC_WR  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_EMMC_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_EMMC_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(13))

//EMMC2M
#define     DMA_CTLL_EMMC_RD  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_EMMC_RD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_EMMC_RD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(13))
//----------------------------------------------------------------------------------------------------//
//M2VOP
#define     DMA_CTLL_VOP_WR  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_16 | B_CTLL_SRC_MSIZE_16\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_VOP_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_VOP_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(14))
//----------------------------------------------------------------------------------------------------//
//M2HIFI
#define     DMA_CTLL_HIFI_WR  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_HIFI_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_HIFI_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(1))

//HIFI2M
#define     DMA_CTLL_HIFI_RD  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_HIFI_RD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)
#define     DMA_CFGH_HIFI_RD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(0))
//M2NAND
#define     DMA_CTLL_NNAD_WR  (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_MEM2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_NAND_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_NAND_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//NANDC2M
#define     DMA_CTLL_NNAD_RD  (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_MEM2MEM_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_NAND_RD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_NAND_RD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)


//M2USB DEVICE
#define     DMA_CTLL_USB_TX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_UNC| B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_USB_TX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_USB_TX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//USB2M DEVICE
#define     DMA_CTLL_USB_RX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC\
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_USB_RX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_USB_RX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

/*
--------------------------------------------------------------------------------

                        Struct Define

--------------------------------------------------------------------------------
*/
typedef enum
{
    DMA_CH0     = (int32)(0),
    DMA_CH1     = (int32)(1),
    DMA_CH2     = (int32)(2),

    DMA_SUCCESS = (int32)(0),
    DMA_BUSY    = (int32)(1),
    DMA_IDLE    = (int32)(2),
    DMA_FALSE   = (int32)(-1)

}DMAState;


typedef enum _DMA_CHN
{
    DMA_CHN0 = (uint32)(0),
    DMA_CHN1,
    DMA_CHN2,
    DMA_CHN3,
    DMA_CHN4,
    DMA_CHN5,
    DMA_CHN_MAX
}eDMA_CHN;

typedef struct tagDMA_CFGX
{
    uint32   CTLL;
    uint32   CFGL;
    uint32   CFGH;
    void    *pLLP;         //llp list addr

}DMA_CFGX,*pDMA_CFGX;

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
extern DMAState DmaGetState(uint32 ch);
//extern int32    DmaGetChannel(void);
extern DMAState DmaConfig(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size,
                              pDMA_CFGX g_dmaPar, pFunc CallBack, pFunc *pCallBackBuf,
                              pDMA_LLP pllplist);


extern int32    DmaGetChannel(void);
extern DMAState DmaStart(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pFunc CallBack);
extern void     DmaInt(void);
extern void     DmaSuspend(uint32 ch);
extern void     DmaResume(uint32 ch);

/*
********************************************************************************
*
*                         End of dma.h
*
********************************************************************************
*/
#endif
