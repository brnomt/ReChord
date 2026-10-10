/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   i2s.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _I2S_H_
#define _I2S_H_

#undef  EXT
#ifdef _IN_I2S_
#define EXT
#else
#define EXT extern
#endif


/*
--------------------------------------------------------------------------------

                        Struct Define

--------------------------------------------------------------------------------
*/

typedef enum
{
    I2S_CH0 = 0,
    I2S_CH1,
    I2S_CH_MAX
}I2S_CHannel;

typedef enum
{
    I2S_CH0_PA,
    I2S_CH0_ACODEC,
    I2S_CH1_PA,
    I2S_CH1_PB
}I2S_PORTs;

typedef enum
{
    I2S_INT_TX = 0,
    I2S_INT_RX,
    I2S_INT_RTX
}I2S_INT_t;

typedef enum I2S_FS
{
    I2S_FS_8000Hz   = 8000,
    I2S_FS_11025Hz  = 11025,
    I2S_FS_12KHz    = 12000,
    I2S_FS_16KHz    = 16000,
    I2S_FS_22050Hz  = 22050,
    I2S_FS_24KHz    = 24000,
    I2S_FS_32KHz    = 32000,
    I2S_FS_44100Hz  = 44100,
    I2S_FS_48KHz    = 48000,
    I2S_FS_64KHz    = 64000,
    I2S_FS_88200Hz   = 88200,
    I2S_FS_96KHz    = 96000,
    I2S_FS_128KHz   = 128000,
    I2S_FS_1764KHz  = 176400,
    I2S_FS_192KHz   = 192000,
    I2S_FSSTOP      = 192000
} I2sFS_en_t;

typedef enum
{
    I2S_FORMAT = 0,
    PCM_FORMAT  = 1,
}eI2sFormat_t;

typedef enum
{
    I2S_START_NULL = 0,
    I2S_START_PIO_RX,
    I2S_START_DMA_RX,
    I2S_START_PIO_TX,
    I2S_START_DMA_TX,
    I2S_START_PIO_RTX,
    I2S_START_DMA_RTX
}I2S_Start_t;

typedef enum
{
    I2S_NORMAL_MODE = 0,
    I2S_LEFT_MODE
}I2S_BUS_MODE_t;

typedef enum
{
    PCM_EARLY_MODE = 0,
    PCM_LATE_MODE
}PCM_BUS_MODE_t;

typedef enum
{
    I2S_BUS_MODE = 0,
    PCM_BUS_MODE
}I2S_PCM_SEL_t;


typedef enum
{
    I2S_SLAVE_MODE = 0,
    I2S_MASTER_MODE,
    I2S_MASTER_MODE_LOCK,
    I2S_MASTER_MODE_UNLOCK
}I2S_mode_t;

typedef enum
{
    I2S0_EXT     = 0,
    I2S0_ACODEC  = 1,
    I2S1_PA      = 2,
    I2S1_PB      = 3,

}eI2sCs_t;

typedef enum
{
    // Vailid Data width = n+1 bit
    I2S_DATA_WIDTH16 = 0xF,
    I2S_DATA_WIDTH24  = 0x17,
}eI2sDATA_WIDTH_t;

typedef enum
{
    TX_interrupt_active = 1,
    RX_interrupt_active = 2,
}eI2sINT_SR_Event;

typedef enum I2S_Source
{
    Source_For_12,
    Source_For_12288,
    Source_For_112896,
} I2S_Source_t;

/*
--------------------------------------------------------------------------------

                        I2S I/F Config

--------------------------------------------------------------------------------
*/

#if ( CODEC_CONFIG == CODEC_ROCKC)
#define I2S_CH          I2S_CH0
#define I2S_PORT        I2S_CH0_ACODEC  //actually i2s_ch0 port B
#define I2S_MODE        I2S_SLAVE_MODE
#else   //If You use other codec as main codec,and use i2s channel 1,you must use i2s master mode.
#define I2S_CH          I2S_CH1
#define I2S_PORT        I2S_CH1_PA
#define I2S_MODE        I2S_MASTER_MODE
#endif

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
extern int32 I2SStart(I2S_CHannel nCH,I2S_Start_t TxOrRx);
extern void  I2SStop(I2S_CHannel nCH,I2S_Start_t TxOrRx);
extern int32 I2SInit(I2S_CHannel nCH,
              I2S_PORTs I2S_Port,
              I2S_mode_t I2S_mode,
              I2sFS_en_t I2S_FS,
              eI2sFormat_t BUS_FORMAT,
              eI2sDATA_WIDTH_t I2S_Data_width,
              I2S_BUS_MODE_t I2S_Bus_mode);
extern void  I2SDeInit(I2S_CHannel nCH,I2S_PORTs I2S_Port);
extern void I2s0Cs(eI2sCs_t data);

/*
********************************************************************************
*
*                         End of i2s.h
*
********************************************************************************
*/
#endif
