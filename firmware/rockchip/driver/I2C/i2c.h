/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   I2c.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _I2C_H_
#define _I2C_H_

#undef  EXT
#ifdef _IN_I2C_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------

                        Macro define

--------------------------------------------------------------------------------
*/

/* Control register */
/*#define I2C_CON               0x000*/
#define I2C_CON_EN              (1 << 0)    //i2c module enable
#define I2C_CON_MOD(mod)        ((mod) << 1)
#define I2C_MODE_TX             0x00
#define I2C_MODE_TRX            0x01
#define I2C_MODE_RX             0x02
#define I2C_MODE_RRX            0x03
#define I2C_CON_MASK            (3 << 1)

#define I2C_CON_START           (1 << 3)
#define I2C_CON_STOP            (1 << 4)
#define I2C_CON_LASTACK         (1 << 5)
#define I2C_CON_ACTACK          (1 << 6)


/* Clock dividor register */
/*#define I2C_CLKDIV            0x004*/
#define I2C_CLKDIV_VAL(divl, divh)  (((divl) & 0xffff) | (((divh) << 16) & 0xffff0000))


/* the slave address accessed  for master rx mode */
/*#define I2C_MRXADDR           0x008*/
#define HIGH_BYTE_VALID         4
#define MID_BYTE_VALID          2
#define LOW_BYTE_VALID          1
#define I2C_MRXADDR_SET(vld, addr)  (((vld) << 24) | (addr))

/* the slave register address accessed  for master rx mode */
/*#define I2C_MRXRADDR          0x00c*/
#define I2C_MRXRADDR_SET(vld, raddr)    (((vld) << 24) | (raddr))

/* master tx count */
//#define I2C_MTXCNT        0x010
/* master rx count */
//#define I2C_MRXCNT        0x014

/* interrupt enable register */
/*#define I2C_IEN               0x018*/
#define I2C_BTFIEN              ((UINT32)(1 << 0))        //BYTE transmit finish interrupt enable
#define I2C_BRFIEN              ((UINT32)(1 << 1))        //BYTE receive finish interrupt enable
#define I2C_MBTFIEN             ((UINT32)(1 << 2))        //MTXCNT data transmit finish interrupt enable
#define I2C_MBRFIEN             ((UINT32)(1 << 3))        //MRXCNT data receive finish interrupt enable
#define I2C_STARTIEN            ((UINT32)(1 << 4))        //start operate finish interrupt enable
#define I2C_STOPIEN             ((UINT32)(1 << 5))        //stop operate finish interrupt enable
#define I2C_NAKRCVIEN           ((UINT32)(1 << 6))        //NAK handshake receive interrupt enable

#define IRQ_MST_ENABLE          (I2C_MBTFIEN | I2C_MBRFIEN | I2C_NAKRCVIEN | I2C_STARTIEN | I2C_STOPIEN)
#define IRQ_ALL_DISABLE         0

/* interrupt pending register */
/*#define I2C_IPD                 0x01c*/
#define I2C_BTFIPD              ((UINT32)(1 << 0))    //BYTE transmit finish interrupt pending bit
#define I2C_BRFIPD              ((UINT32)(1 << 1))    //BYTE receive finish interrupt pending bit
#define I2C_MBTFIPD             ((UINT32)(1 << 2))    //MTXCNT data transmit finished interrupt pending bit
#define I2C_MBRFIPD             ((UINT32)(1 << 3))    //MRXCNT data received finished interrupt pending bit
#define I2C_STARTIPD            ((UINT32)(1 << 4))    //start operation finished interrupt pending bit
#define I2C_STOPIPD             ((UINT32)(1 << 5))    //stop operation finished interrupt pending bit
#define I2C_NAKRCVIPD           ((UINT32)(1 << 6))    //NAK handshake received interrupt pending bit
#define I2C_IPD_ALL_CLEAN       ((UINT32)0x7f) //

/* I2C tx data register */
#define I2C_TXDATA_BASE         0X100
/* I2C rx data register */
#define I2C_RXDATA_BASE         0x200


/* i2c timerout */
#define I2C_TIMEOUT_US          200000
#define I2C_TIMEOUT_MS          200
#define I2C_RETRY_COUNT         3

/* i2c fifo max transfer bytes */
#define RK_I2C_FIFO_SIZE        32



/*
--------------------------------------------------------------------------------

                        Struct Define

--------------------------------------------------------------------------------
*/
 typedef enum I2C_slaveaddr
{
    tvp5145     = 0x5C,
    WM8987codec = 0x34,
    RDA5820AD   = 0x22,
    FM5767      = 0xC0,
    FM5800      = 0x20,
    RTCM41      = 0xD0,
    InterCodec  = 0x4E,
    AR1010      = 0x20,
    FM5802      = 0x20
}eI2C_slaveaddr_t;

typedef enum I2C_mode
{
    WriteMode,
    ReadMode,
}eI2C_mode_t;

typedef enum I2C_RW_mode
{
    NormalMode,
    DirectMode,
    RDA_5820Mode,
    YDA_174Mode     //mlc add for yamaha yda174
}eI2C_RW_mode_t;

typedef enum _EI2C_CHANNEL
{
    I2C_CH0,
    I2C_CH1,
    I2C_CH2,

    I2C_CH_MAX,
}eI2C_CHANNEL;

typedef enum _I2C_IO_MUX
{
    I2C_CH0_PA,   //0
    I2C_CH0_PB,
    I2C_CH0_PC,
    I2C_CH1_PA,
    I2C_CH1_PB,
    I2C_CH1_PC,
    I2C_CH2_PA,
    I2C_CH2_PB,
    I2C_CH2_PC,

    I2C_CH_PORT_MAX,
}eI2C_IOMUX;


typedef enum I2C_Address_Reg
{
    I2C_10BIT_ADDRESS_16BIT_REG = 0,
    I2C_10BIT_ADDRESS_8BIT_REG,
    I2C_7BIT_ADDRESS_16BIT_REG,
    I2C_7BIT_ADDRESS_8BIT_REG,
}eI2C_Address_Reg_t;
/*
--------------------------------------------------------------------------------

                        Variable Define

--------------------------------------------------------------------------------
*/
_ATTR_SYS_BSS_
 EXT UINT8 gI2C_slaveaddr ;
_ATTR_SYS_BSS_
 EXT UINT16 gI2C_oldspeed ;
_ATTR_SYS_BSS_
 EXT UINT8 gI2C_flag[I2C_CH_MAX];
_ATTR_SYS_BSS_
 EXT UINT8 gI2C_intType;

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/

extern void  I2CStart(eI2C_CHANNEL i2cPort);
extern void  I2CStop(eI2C_CHANNEL i2cPort);

extern int32 I2CSendData(eI2C_CHANNEL i2cPort,
                            UINT16 slaveaddr,
                            UINT16 regaddr,
                            eI2C_Address_Reg_t addr_reg_mode,
                            UINT8* Databuf,
                            uint32 size,
                            eI2C_RW_mode_t rw_mode);
extern int32 I2CReadData(eI2C_CHANNEL i2cPort,
                        UINT8 slaver_addr,
                        uint16 reg_addr,
                        eI2C_Address_Reg_t addr_reg_mode,
                        UINT8 *Databuf,
                        uint32 len,
                        eI2C_RW_mode_t rw_mode);
extern BOOL I2CSetSpeed(eI2C_CHANNEL i2cPort,UINT16 speed);
extern BOOL I2C_Init(eI2C_IOMUX i2cPort,UINT8 SlaveAddress,uint16 speed);
extern BOOL I2C_Deinit(eI2C_IOMUX i2cPort);

extern int I2C_Write(eI2C_CHANNEL i2cPort,UINT8 RegAddr, UINT8 *pData, UINT16 size, eI2C_RW_mode_t mode);
extern int I2C_Read(eI2C_CHANNEL i2cPort,UINT8 RegAddr, UINT8 *pData, UINT16 size, eI2C_RW_mode_t mode);

/*
********************************************************************************
*
*                         End of Example.h
*
********************************************************************************
*/
#endif
