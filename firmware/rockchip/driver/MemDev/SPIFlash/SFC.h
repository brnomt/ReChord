/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SFC.h
Desc: 

Author: 
Date: 14-02-25
Notes:

$Log: $
 *
 *
*/

#ifndef _SFC_H
#define _SFC_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/

/*configs*/

#define SFC_MAX_IOSIZE              (1024*8)    //8K byte


#define SFC_EN_INT                  (0)         //enable interrupt
#define SFC_EN_DMA                  (0)         //enable dma

#define SFC_FIFO_DEPTH              (0x10)      // 16 words

/* FIFO watermark */
#define SFC_RX_WMARK                (SFC_FIFO_DEPTH)      //RX watermark level
#define SFC_TX_WMARK                (SFC_FIFO_DEPTH)       //TX watermark level
#define SFC_RX_WMARK_SHIFT          (8)
#define SFC_TX_WMARK_SHIFT          (0)

/*return value*/
#define SFC_OK                      (0)
#define SFC_ERROR                   (-1)
#define SFC_PARAM_ERR               (-2)
#define SFC_TX_TIMEOUT              (-3)
#define SFC_RX_TIMEOUT              (-4)
#define SFC_WAIT_TIMEOUT            (-5)
#define SFC_BUSY_TIMEOUT            (-6)
#define SFC_ECC_FAIL                (-7)
#define SFC_PROG_FAIL               (-8)
#define SFC_ERASE_FAIL              (-9)

/* SFC_CMD Register */
#define SFC_ADDR_0BITS              (0)
#define SFC_ADDR_24BITS             (1)
#define SFC_ADDR_32BITS             (2)
#define SFC_ADDR_XBITS              (3)

#define SFC_WRITE                   (1)
#define SFC_READ                    (0)

/* SFC_CTRL Register */
#define SFC_1BITS_LINE              (0)
#define SFC_2BITS_LINE              (1)
#define SFC_4BITS_LINE              (2)

#define SFC_ENABLE_DMA              (1<<14)

/*------------------------------ Global Typedefs -----------------------------*/

typedef enum
{
    DATA_LINES_X1 = 0,
    DATA_LINES_X2,
    DATA_LINES_X4
} SFC_DATA_LINES;


typedef union tagSFCCTRL_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
        /*spi mode select*/
        unsigned mode : 1;
        /*Shift in phase selection
        0: shift in the flash data at posedge sclk_out
        1: shift in the flash data at negedge sclk_out*/
        unsigned sps : 1;

        unsigned reserved3_2 : 2;
        /*sclk_idle_level_cycles*/
        unsigned scic : 4;
        /*Cmd bits number*/
        unsigned cmdlines : 2;
        /*Address bits number*/
        unsigned addrlines : 2;
        /*Data bits number*/
        unsigned datalines : 2;
        /*this bit is not exit in regiseter, just use for code param*/
        unsigned enbledma : 1;

        unsigned reserved15 : 1;

        unsigned addrbits : 5;

        unsigned reserved31_21 : 11;
    } b;
}SFCCTRL_DATA;

typedef union tagSFCCMD_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
        /*Command that will send to Serial Flash*/
        unsigned cmd : 8;               //
        /*Dummy bits number*/
        unsigned dummybits : 4;
        /*0:read, 1: write*/
        unsigned rw : 1;                
        /*Continuous read mode*/
        unsigned readmode : 1;
        /*Address bits number*/
        unsigned addrbits : 2;
        /*Transferred bytes number*/
        unsigned datasize : 14;
        /*Chip select*/
        unsigned cs : 2;
    } b;
}SFCCMD_DATA;

/*----------------------------- External Variables ---------------------------*/


/*------------------------- Global Function Prototypes -----------------------*/
extern int32 SFC_Init(void);

extern int32 SFC_Request(uint32 sfcmd, uint32 sfctrl, uint32 addr, void *data);

extern void SFC_HandleIRQ(void);

extern void SFC_Delay(uint32 us);


#endif
