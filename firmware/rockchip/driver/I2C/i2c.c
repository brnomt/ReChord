/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   I2c.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-11-20         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_I2C_
#include "typedef.h"
#include "DriverInclude.h"
#include "interrupt.h"


const uint32 I2C_GROUP[3] =
{
    I2C0_BASE,
    I2C1_BASE,
    I2C2_BASE,
};

#define I2CPORT(n)          ((I2CReg_t*)(I2C_GROUP[n]))

//#define RK_CEIL(x, y) \
//  ({ unsigned long __x = (x), __y = (y); (__x + __y - 1) / __y; })


static uint32 RK_CEIL(uint32 x, uint32 y)
{
    uint32 _x = x;
    uint32 _y = y;
    return ((_x + _y - 1) / _y);
}

static void I2C_show_regs(eI2C_CHANNEL i2cPort)
{
}

static int I2C_send_start_bit(eI2C_CHANNEL i2cPort);
static int I2C_send_stop_bit(eI2C_CHANNEL i2cPort);

void I2CStart(eI2C_CHANNEL i2cPort)
{
    I2C_send_start_bit(i2cPort);
}

/*
--------------------------------------------------------------------------------
  Function name : I2CStop(void)
  Author        : yangwenjie
  Description   : I2C stop

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void I2CStop(eI2C_CHANNEL i2cPort)
{
    I2C_send_stop_bit(i2cPort);
}


void I2CEnableInt(eI2C_CHANNEL i2cPort, uint32 IntType)
{
    I2CReg_t *  pReg;
    pReg = I2CPORT(i2cPort);
    pReg->I2C_IEN |= IntType;
}

void I2CDisableInt(eI2C_CHANNEL i2cPort, uint32 IntType)
{
    I2CReg_t *  pReg;
    pReg = I2CPORT(i2cPort);
    pReg->I2C_IEN &= ~IntType;
}

void I2CDisableIRQ(eI2C_CHANNEL i2cPort)
{
    I2CReg_t *  pReg;
    pReg = I2CPORT(i2cPort);
    pReg->I2C_IEN = IRQ_ALL_DISABLE ;
}

uint32 I2CGetInt(eI2C_CHANNEL i2cPort)
{
    I2CReg_t *  pReg;
    uint32 intstatus;

    pReg = I2CPORT(i2cPort);
    intstatus = pReg->I2C_IPD;

    if (intstatus & I2C_NAKRCVIPD)
    {
        pReg->I2C_IPD = I2C_NAKRCVIPD;
//        printf("I2CGetInt :Error! NAKRCV IPD\n");
    }

    if (intstatus & I2C_STARTIPD )
    {
        pReg->I2C_IPD = I2C_STARTIPD;
//        printf("I2CGetInt : start IPD\n");
    }

    if (intstatus & I2C_STOPIPD)
    {
        pReg->I2C_IPD = I2C_STOPIPD;
//        printf("I2CGetInt : stop IPD\n");
    }

    if (intstatus & I2C_BTFIPD )
    {
        pReg->I2C_IPD = I2C_BTFIPD;
//        printf("I2CGetInt :  BYTE transmit finish IPD\n");
    }

    if (intstatus & I2C_BRFIPD )
    {
        pReg->I2C_IPD = I2C_BRFIPD;
//        printf("I2CGetInt :  BYTE recive finish IPD\n");
    }

    if (intstatus & I2C_MBTFIPD )
    {
        pReg->I2C_IPD = I2C_MBTFIPD;
//        printf("I2CGetInt : MTXCNT data transmit finished IPD\n");
    }

    if (intstatus & I2C_MBRFIPD )
    {
        pReg->I2C_IPD = I2C_MBRFIPD;
//        printf("I2CGetInt : MRXCNT data received finished IPD\n");
    }
    return intstatus;
}


static int I2C_send_start_bit(eI2C_CHANNEL i2cPort)
{
    I2CReg_t *  pReg;
    pReg = I2CPORT(i2cPort);
    pReg->I2C_IPD = I2C_IPD_ALL_CLEAN;
    pReg->I2C_CON |= I2C_CON_EN | I2C_CON_START; //0x09
    pReg->I2C_IEN = I2C_STARTIEN;
    return OK;
}


static int I2C_send_stop_bit(eI2C_CHANNEL i2cPort)
{
    I2CReg_t *  pReg;
    pReg = I2CPORT(i2cPort);
    pReg->I2C_IPD = I2C_IPD_ALL_CLEAN;
    pReg->I2C_CON = I2C_CON_EN | I2C_CON_STOP;  // 0x11 b 10001
    pReg->I2C_IEN = I2C_STOPIEN;
    return OK;
}


int32 I2CSendData(eI2C_CHANNEL i2cPort,
                  UINT16 slaveaddr,
                  UINT16 regaddr,
                  eI2C_Address_Reg_t addr_reg_mode,
                  UINT8* Databuf,
                  uint32 size,
                  eI2C_RW_mode_t rw_mode)
{
    int err = OK;
    int TimeOut = I2C_TIMEOUT_US;
    uint8 *pbuf = Databuf;
    uint bytes_remain_len;
    uint bytes_tranfered_len = 0;
    uint words_tranfered_len = 0;
    uint con = 0;
    uint txData = 0;
    uint txdataCnt = 0;
    uint index = 0;
    I2CReg_t *  pReg;
    pReg = I2CPORT(i2cPort);
    pReg->I2C_CON &= (~I2C_CON_START);    //clear start signal

    txData = slaveaddr;
    if (addr_reg_mode == I2C_7BIT_ADDRESS_8BIT_REG
        || addr_reg_mode == I2C_7BIT_ADDRESS_16BIT_REG)
    {
        index += 1;
    }
    else if (addr_reg_mode == I2C_10BIT_ADDRESS_8BIT_REG
             || addr_reg_mode == I2C_10BIT_ADDRESS_16BIT_REG)
    {
        index += 2;
    }

    if (regaddr != 0)
    {
        txData |= regaddr << ((index % 4) * 8);    //index is slaveaddr length

        if (addr_reg_mode == I2C_7BIT_ADDRESS_8BIT_REG
            || addr_reg_mode == I2C_7BIT_ADDRESS_16BIT_REG)
        {
            index += 1;
        }
        else if (addr_reg_mode == I2C_10BIT_ADDRESS_8BIT_REG
                 || addr_reg_mode == I2C_10BIT_ADDRESS_16BIT_REG)
        {
            index += 2;
        }
    }
    else
    {
        //printf("Don't need to process regester!\n");
    }

    bytes_remain_len = size + index;//all data numbers( address and data) to be transmit   //j is register length
    //DEBUG("txData = 0x%08x, bytes_remain_len = %d", txData, bytes_remain_len);

    if (bytes_remain_len)
    {
        while (1)
        {
            txData |= (*Databuf << ((index % 4) * 8));
            pReg->I2C_TXDATA[index / 4] = txData;
//            printf("I2C Write TXDATA[%d] = 0x%08x\n",index/4 ,txData);
            index++;
            Databuf++;
            size--;

            if (index % 4 == 0 || index == 4)
            {
                txData = 0;
            }

            if ((index == 32) || (size == 0))
            {
                bytes_remain_len -= index;
                break;
            }
        }

        pReg->I2C_CON = I2C_CON_EN | I2C_CON_MOD(I2C_MODE_TX);
        pReg->I2C_MTXCNT = index;   //start transmit
        txdataCnt = index;
        index = 0;
    }

//    printf("I2C Write TXDATA[%d] = 0x%08x\n",index/4,pReg->I2C_TXDATA[index/4]);
    return txdataCnt;
}

int32 I2CReadData(eI2C_CHANNEL i2cPort,
                  UINT8 slaver_addr,
                  uint16 reg_addr,
                  eI2C_Address_Reg_t addr_reg_mode,
                  UINT8 *Databuf,
                  uint32 len,
                  eI2C_RW_mode_t rw_mode)
{
    int err = OK;
    int TimeOut ;
    uint8 *pbuf = Databuf;
    uint bytes_remain_len = len;
    uint bytes_tranfered_len = 0;
    uint words_tranfered_len = 0;
    uint con = 0;
    uint rxdata;
    uint rxSize = 0;
    uint i, j;
    I2CReg_t *  pReg;

    pReg = I2CPORT(i2cPort);
    TimeOut = 200;

    pReg->I2C_CON &= (~I2C_CON_START);//clear start signal
    pReg->I2C_CON &= (~I2C_CON_MASK);
    pReg->I2C_CON |= I2C_CON_MOD(I2C_MODE_TRX) /*| I2C_CON_LASTACK*/;
    pReg->I2C_IPD = I2C_IPD_ALL_CLEAN;//clear interrupt pending bit

    if ((addr_reg_mode == I2C_7BIT_ADDRESS_16BIT_REG)
        || (addr_reg_mode == I2C_7BIT_ADDRESS_8BIT_REG))
    {
        pReg->I2C_MRXADDR = I2C_MRXADDR_SET(1, slaver_addr);
    }
    else if ((addr_reg_mode == I2C_10BIT_ADDRESS_16BIT_REG)
             || (addr_reg_mode == I2C_10BIT_ADDRESS_8BIT_REG))
    {
        pReg->I2C_MRXADDR = I2C_MRXADDR_SET(3, slaver_addr);
    }

    if ( reg_addr != 0 )
    {
        //config device register address
        if ((addr_reg_mode == I2C_10BIT_ADDRESS_8BIT_REG)
            || (addr_reg_mode == I2C_7BIT_ADDRESS_8BIT_REG))
        {
            pReg->I2C_MRXRADDR = (0x1 << 24) | reg_addr; //config device register address,low byte valids
        }
        else if ((addr_reg_mode == I2C_10BIT_ADDRESS_16BIT_REG)
                 || (addr_reg_mode == I2C_7BIT_ADDRESS_16BIT_REG))
        {
            pReg->I2C_MRXRADDR = (0x3 << 24) | reg_addr; //config device register address,low and mid bytes valid
        }
    }
    else
    {
        //printf("Don't need to process regester!\n");
    }

    if (bytes_remain_len)
    {
        if (bytes_remain_len >= RK_I2C_FIFO_SIZE)
        {
            if ( bytes_remain_len == RK_I2C_FIFO_SIZE )
            {
                pReg->I2C_CON |= (I2C_CON_EN | I2C_CON_LASTACK);
            }

            bytes_tranfered_len = 32;
        }
        else
        {
            pReg->I2C_CON |= (I2C_CON_EN | I2C_CON_LASTACK);
            bytes_tranfered_len = bytes_remain_len;
        }

        pReg->I2C_MRXCNT = bytes_tranfered_len;  //set recive data count
        rxSize  = bytes_tranfered_len;
        words_tranfered_len = RK_CEIL(bytes_tranfered_len, 4);

        for (i = 0; i < words_tranfered_len; i++)
        {
            do
            {
                TimeOut--;
                __WFI();
            }
            while ((!(pReg->I2C_IPD & I2C_MBRFIPD)) && (TimeOut > 0));

            //pReg->I2C_IPD |= (I2C_MBRFIPD|I2C_BRFIPD);

            if ( TimeOut)
            {
                rxdata = pReg->I2C_RXDATA[i];

                for (j = 0; j < 4; j++)
                {
                    if ((i * 4 + j) == bytes_tranfered_len)
                    {
                        break;
                    }

                    *pbuf++ = (rxdata >> (j * 8)) & 0xff;
                }
            }
        }

        bytes_remain_len -= bytes_tranfered_len;

        if ( bytes_remain_len != 0)
        {
            //config recive mode
            pReg->I2C_CON = (pReg->I2C_CON & ~(I2C_CON_MASK)) | I2C_CON_MOD(I2C_MODE_RX);
        }

        //printf("I2C Read bytes_remain_len %d\n", bytes_remain_len);
    }

    return rxSize;
}

static  void I2C_Get_div(int div, int *divh, int *divl)
{
    if (div % 2 == 0)
    {
        *divh = div / 2;
        *divl = div / 2;
    }
    else
    {
        *divh = RK_CEIL(div, 2);
        *divl = div / 2;
    }
}

BOOL I2CSetSpeed(eI2C_CHANNEL i2cPort, UINT16 speed)
{
    UINT32 exp;
    UINT32 rem;
    UINT32 scalespeed;
    UINT32 freqbase;
    uint32 i2c_rate;
    int div, divl, divh;
    I2CReg_t *  pReg;
    pReg = I2CPORT(i2cPort);
    i2c_rate = GetI2CFreq() / 1000;  //KHz
    div = RK_CEIL(i2c_rate, speed << 3) - 2;

    if (div < 0 )
    {
        divh = divl = 0;
    }
    else
    {
        I2C_Get_div(div, &divh, &divl);
    }

//     printf("divl = %d,divh = %d \n",divl,divh);
    pReg->I2C_CLKDIV = I2C_CLKDIV_VAL(divl, divh);
//    printf("I2CSetSpeed %d \n",speed);
    return TRUE;
}

BOOL I2C_Read_SendACK(eI2C_CHANNEL i2cPort, BOOL flg)
{
    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : I2C_Init(UINT8 SlaveAddress, UINT16 nKHz)
  Author        : yangwenjie
  Description   :

  Input         : SlaveAddress :I2C device address
                  nKHz         :I2C max speed

  Return        :TRUE -- ok
                 FALSE -- fail

  History:     <author>         <time>         <version>
             yangwenjie     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
BOOL i2c_init(eI2C_CHANNEL i2cPort, UINT8 SlaveAddress, uint16 speed)
{
    I2CReg_t *  pReg;
    uint32 i2c_rate;
    pReg = I2CPORT(i2cPort);
    pReg->I2C_CON = 0;
    I2CSetSpeed(i2cPort, speed);
    pReg->I2C_CON = I2C_CON_EN;
    return TRUE;
}

/*
--------------------------------------------------------------------------------
  Function name : i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
  Description   : Write to i2c memory

  Input         : i2cPort: i2c channel
                  slaveaddr:target i2c address
                  regaddr:  address to send to
                  reglen:
                  buffer:   buffer for send data
                  len:  length of bytes to be send

  Return        :TRUE -- ok
                 FALSE --fail

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
int i2c_write(eI2C_CHANNEL i2cPort,
              uint16 slaveaddr,
              uint16 regaddr,
              eI2C_Address_Reg_t addr_reg_mode,
              uint8 *buffer,
              int len,
              eI2C_RW_mode_t rw_mode)
{
    if ((buffer == NULL) && (len != 0))
    {
        printf ("i2c_write: buf == NULL ,error return\n");
        return ERROR;
    }

    if (len == 0)
    {
        printf("i2c_write: len = 0 ,error return\n");
        return ERROR;
    }

    return I2CSendData(i2cPort, slaveaddr, regaddr, addr_reg_mode, buffer, len, rw_mode);
}

/*
--------------------------------------------------------------------------------
  Function name : i2c_read(uchar slaveaddr, uint regaddr, int reglen, uchar *buf, int len)
  Author        :
  Description   : Read from i2c memory

  Input         : i2cPort: i2c channel
                  slaveaddr:    target i2c address
                  regaddr:  address to read from
                  reglen:
                  buffer:   buffer for read data
                  len:  length of bytes to be read

  Return        :TRUE -- ok
                 FALSE --fail

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
int i2c_read(eI2C_CHANNEL i2cPort,
             uint8 slaveaddr,
             uint16 regaddr,
             eI2C_Address_Reg_t addr_reg_mode,
             uint8 *buf,
             int len,
             eI2C_RW_mode_t rw_mode)
{
    if ((buf == NULL) && (len != 0))
    {
        printf("i2c_read: buf == NULL\n");
        return ERROR;
    }

    if (len == 0)
    {
        printf("i2c_read: len = 0 \n");
        return ERROR;
    }

    return I2CReadData(i2cPort, slaveaddr, regaddr, addr_reg_mode, buf, len, rw_mode);
}


/*
--------------------------------------------------------------------------------
  Function name : I2C_Read(UINT8 RegAddr, UINT8 *pData, UINT16 size, eI2C_mode_t mode)
  Author        : yangwenjie
  Description   :

  Input         : RegAddr
                  size -- read size
                  mode -- NormalMode/DirectMode

  Return        :TRUE -- ok
                 FALSE --fail

  History:     <author>         <time>         <version>
             yangwenjie     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
BOOL I2C_ReadConfig(eI2C_CHANNEL i2cPort, BOOL StartBit)
{
    BOOL ret = TRUE;
    I2CReg_t *  pReg;
    pReg = I2CPORT(i2cPort);
    return ret;
}

BOOL i2c_deinit(eI2C_CHANNEL i2cPort )
{
    I2CReg_t *  pReg;
    pReg = I2CPORT(i2cPort);
    pReg->I2C_CON = 0;
    return TRUE;
}

void I2CIntIsr(uint32 i2cport)
{
    gI2C_flag[i2cport] = 1;
    gI2C_intType = I2CGetInt(i2cport);
}

void I2CIntIsr2(void)
{
    I2CIntIsr(I2C_CH2);
}

void I2CIntIsr1(void)
{
    I2CIntIsr(I2C_CH1);
}

void I2CIntIsr0(void)
{
    I2CIntIsr(I2C_CH0);
}

static
void I2C_GPIO_init(eI2C_IOMUX i2cPort)
{
    switch ( i2cPort )
    {
        case I2C_CH0_PA:
            Grf_I2C0_Sel(I2C_SEL_A);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin2, IOMUX_GPIO2B2_I2C0A_SCL);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin3, IOMUX_GPIO2B3_I2C0A_SDA);
            break;

        case I2C_CH0_PB:
            Grf_I2C0_Sel(I2C_SEL_B);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin1, IOMUX_GPIO2C1_I2C0B_SCL);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin0, IOMUX_GPIO2C0_I2C0B_SDA);
            break;

        case I2C_CH0_PC:
            Grf_I2C0_Sel(I2C_SEL_C);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin6, IOMUX_GPIO0A6_I2C0C_SCL);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin5, IOMUX_GPIO0A5_I2C0C_SDA);
            break;

        case I2C_CH1_PA:
            Grf_I2C1_Sel(I2C_SEL_A);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin1, IOMUX_GPIO2B1_I2C1A_SCL);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin0, IOMUX_GPIO2B0_I2C1A_SDA);
            break;

        case I2C_CH1_PB:
            Grf_I2C1_Sel(I2C_SEL_B);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin1, IOMUX_GPIO1B1_I2C1B_SCL);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin2, IOMUX_GPIO1B2_I2C1B_SDA);
            break;

        case I2C_CH1_PC:
            Grf_I2C1_Sel(I2C_SEL_C);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin5, IOMUX_GPIO2B5_I2C1C_SCL);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin4, IOMUX_GPIO2B4_I2C1C_SDA);
            break;

        case I2C_CH2_PA:
            Grf_I2C2_Sel(I2C_SEL_A);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortA_Pin1, IOMUX_GPIO2A1_I2C2A_SCL);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortA_Pin0, IOMUX_GPIO2A0_I2C2A_SDA);
            break;

        case I2C_CH2_PB:
            Grf_I2C2_Sel(I2C_SEL_B);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortD_Pin0, IOMUX_GPIO0D0_I2C2B_SCL);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortD_Pin1, IOMUX_GPIO0D1_I2C2B_SDA);
            break;

        case I2C_CH2_PC:
            Grf_I2C2_Sel(I2C_SEL_C);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortA_Pin5, IOMUX_GPIO2A5_I2C2C_SCL);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortA_Pin6, IOMUX_GPIO2A6_I2C2C_SDA);
            break;

        default:
            break;
    }
}

_ATTR_SYS_CODE_
BOOL I2C_Init(eI2C_IOMUX i2cPort, UINT8 SlaveAddress, uint16 speed)
{
    int i2cChannel;
    //UserIsrDisable();
    i2cChannel = i2cPort / 3;
    I2C_GPIO_init(i2cPort);

    //open rst I2C ip
    if ( i2cPort < I2C_CH1_PA)
    {
        ScuClockGateCtr(PCLK_I2C0_GATE, 1);
        ScuSoftResetCtr(I2C0_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(I2C0_SRST, 0);
    }
    else if ( i2cPort < I2C_CH2_PA)
    {
        ScuClockGateCtr(PCLK_I2C1_GATE, 1);
        ScuSoftResetCtr(I2C1_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(I2C1_SRST, 0);
    }
    else if ( i2cPort < I2C_CH_PORT_MAX )
    {
        ScuClockGateCtr(PCLK_I2C2_GATE, 1);
        ScuSoftResetCtr(I2C2_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(I2C2_SRST, 0);
    }

    gI2C_slaveaddr = SlaveAddress;
    gI2C_oldspeed = speed;
    gI2C_flag[i2cChannel] = 0;
    gI2C_intType = 0;
    i2c_init(i2cChannel, SlaveAddress, speed);
    //DEBUG("I2C CHN %d port %d SetSpeed %d \n",i2cChannel,i2cPort%3,speed);

    switch (i2cChannel)
    {
        case I2C_CH0 :
            IntRegister(INT_ID_I2C0 , I2CIntIsr0);
            IntPendingClear(INT_ID_I2C0);
            IntEnable(INT_ID_I2C0);
            break;

        case I2C_CH1:
            IntRegister(INT_ID_I2C1 , I2CIntIsr1);
            IntPendingClear(INT_ID_I2C1);
            IntEnable(INT_ID_I2C1);
            break;

        case I2C_CH2:
            IntRegister(INT_ID_I2C2 , I2CIntIsr2);
            IntPendingClear(INT_ID_I2C2);
            IntEnable(INT_ID_I2C2);
            break;

        default:
            break;
    }

    return TRUE;
}

_ATTR_SYS_CODE_
BOOL I2C_Deinit( eI2C_IOMUX i2cPort  )
{
//    UserIsrEnable();
    int i2cChannel;
    i2cChannel = i2cPort / 3;

    switch ( i2cPort )
    {
        case I2C_CH0_PA:
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin2, IOMUX_GPIO2B2_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin3, IOMUX_GPIO2B3_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin2, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin3, GPIO_IN);
            break;

        case I2C_CH0_PB:
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin1, IOMUX_GPIO2C1_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin0, IOMUX_GPIO2C0_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortC_Pin1, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortC_Pin0, GPIO_IN);
            break;

        case I2C_CH0_PC:
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin6, IOMUX_GPIO0A6_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin5, IOMUX_GPIO0A5_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin6, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin5, GPIO_IN);
            break;

        case I2C_CH1_PA:
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin1, IOMUX_GPIO2B1_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin0, IOMUX_GPIO2B0_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin1, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin0, GPIO_IN);
            break;

        case I2C_CH1_PB:
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin1, IOMUX_GPIO1B1_IO);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin2, IOMUX_GPIO1B2_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin1, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin2, GPIO_IN);
            break;

        case I2C_CH1_PC:
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin5, IOMUX_GPIO2B5_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin4, IOMUX_GPIO2B4_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin5, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin4, GPIO_IN);
            break;

        case I2C_CH2_PA:
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortA_Pin1, IOMUX_GPIO2A1_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortA_Pin0, IOMUX_GPIO2A0_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin1, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin0, GPIO_IN);
            break;

        case I2C_CH2_PB:
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortD_Pin0, IOMUX_GPIO0D0_IO);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortD_Pin1, IOMUX_GPIO0D1_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortD_Pin0, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortD_Pin1, GPIO_IN);
            break;

        case I2C_CH2_PC:
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortA_Pin5, IOMUX_GPIO2A5_IO);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortA_Pin6, IOMUX_GPIO2A6_IO);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin5, GPIO_IN);
            Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin6, GPIO_IN);
            break;
    }

    switch ( i2cChannel )
    {
        case I2C_CH0:
            ScuSoftResetCtr(I2C0_SRST, 1);
            ScuClockGateCtr(PCLK_I2C0_GATE, 0);
            break;

        case I2C_CH1:
            ScuSoftResetCtr(I2C1_SRST, 1);
            ScuClockGateCtr(PCLK_I2C1_GATE, 0);
            break;

        case I2C_CH2:
            ScuSoftResetCtr(I2C1_SRST, 1);
            ScuClockGateCtr(PCLK_I2C1_GATE, 0);
            break;
    }

    switch (i2cChannel)
    {
        case I2C_CH0 :
            IntPendingClear(INT_ID_I2C0);
            IntDisable(INT_ID_I2C0);
            IntUnregister(INT_ID_I2C0);
            break;

        case I2C_CH1:
            IntPendingClear(INT_ID_I2C1);
            IntDisable(INT_ID_I2C1);
            IntUnregister(INT_ID_I2C1);
            break;

        case I2C_CH2:
            IntPendingClear(INT_ID_I2C2);
            IntDisable(INT_ID_I2C2);
            IntUnregister(INT_ID_I2C2);
            break;

        default:
            break;
    }
}

_ATTR_SYS_CODE_
int I2C_Write(eI2C_CHANNEL i2cPort, UINT8 RegAddr, UINT8 *pData, UINT16 size, eI2C_RW_mode_t mode)
{
    int sendedSize;
    int needSend, deltaSize;
    uint32 intpending;
    int TimeOut;
    deltaSize = 1;  //because include 7bit slaveraddr and 8bit register addr
    needSend = size;
    sendedSize = 0;
    TimeOut = I2C_TIMEOUT_MS;   //system tick 10ms,here 2's timeout.

    if (RegAddr != 0)
    {
        deltaSize += 1;     //no matter RW_mode is normal or direct
    }
    else
    {
        deltaSize += 0;
    }

    I2CDisableIRQ(i2cPort); //clear all irq
    I2CStart(i2cPort);

    do
    {
        TimeOut--;
//        __WFI();
        DelayUs(1);
    }
    while ((!gI2C_flag[i2cPort]) && (TimeOut > 0));

    if (TimeOut)
    {
        gI2C_flag[i2cPort] = 0;
        TimeOut = I2C_TIMEOUT_MS;
    }
    else
    {
        i2c_deinit(i2cPort);
        return TIMEOUT;
    }

    I2CEnableInt(i2cPort, I2C_MBTFIEN | I2C_NAKRCVIEN);
    sendedSize = i2c_write(i2cPort,
                           gI2C_slaveaddr | WriteMode,
                           RegAddr,
                           I2C_7BIT_ADDRESS_8BIT_REG,
                           pData,
                           size,
                           mode);

    if ( sendedSize - deltaSize == needSend)
    {
        needSend = 0;   //send over
    }
    else
    {
        needSend -= sendedSize - deltaSize;
        pData    += sendedSize - deltaSize;
    }

//    I2CEnableInt(i2cPort, I2C_MBTFIEN | I2C_NAKRCVIEN);
    do
    {
        TimeOut--;
//        __WFI();
        DelayUs(1);
    }
    while ((!gI2C_flag[i2cPort]) && (TimeOut > 0));

    if (TimeOut)
    {
        gI2C_flag[i2cPort] = 0;
        TimeOut = I2C_TIMEOUT_MS;

        if ( gI2C_intType & I2C_NAKRCVIPD ) //ACK未收到，出错返回，重新再发
        {
            printf("\n------I2C_NAKRCVIPD------\n");
            I2CStop(i2cPort );

            while (!gI2C_flag[i2cPort])
                __WFI();

            gI2C_flag[i2cPort] = 0;
            gI2C_intType = 0;
            i2c_deinit(i2cPort);
            I2CDisableInt(i2cPort, I2C_MBTFIEN | I2C_NAKRCVIEN);
            return ERROR;   //need retry to send again.
        }
    }
    else
    {
        i2c_deinit(i2cPort);
        I2CDisableInt(i2cPort, I2C_MBTFIEN | I2C_NAKRCVIEN);
        return TIMEOUT;
    }

    while (needSend)
    {
        if ( gI2C_intType & I2C_MBTFIPD )
        {
            int realsize = 0;
            /*Note:in interrupt handle,set slave address 0x0 as continus Tx flag*/
            realsize = i2c_write(i2cPort,
                                 0,
                                 RegAddr,
                                 I2C_7BIT_ADDRESS_8BIT_REG,
                                 pData,
                                 needSend,
                                 mode);

            do
            {
                TimeOut--;
//                __WFI();
                DelayUs(1);
            }
            while ((!gI2C_flag[i2cPort]) && (TimeOut > 0));

            if (TimeOut)
            {
                gI2C_flag[i2cPort] = 0;
                TimeOut = I2C_TIMEOUT_MS;
                pData       += realsize;
                needSend    -= realsize;
                sendedSize  += realsize;
            }
            else
            {
                break;
            }
        }

        if ( gI2C_intType & I2C_NAKRCVIPD ) //ACK未收到，出错返回，重新再发
        {
            printf("I2C_NAKRCVIPD------\n");
            I2CStop(i2cPort );

            while (!gI2C_flag[i2cPort])
                __WFI();

            gI2C_flag[i2cPort] = 0;
            i2c_deinit(i2cPort);
            I2CDisableInt(i2cPort, I2C_MBTFIEN | I2C_NAKRCVIEN);
            gI2C_intType = 0;
            return ERROR;   //need retry to send again.
        }
    }//while( needSend );  //是否继续发送

    I2CStop(i2cPort );

    do
    {
        TimeOut--;
//        __WFI();
        DelayUs(1);
    }
    while ((!gI2C_flag[i2cPort]) && (TimeOut > 0));

    if (TimeOut)
    {
        gI2C_flag[i2cPort] = 0;
        i2c_deinit(i2cPort);
        I2CDisableInt(i2cPort, I2C_MBTFIEN | I2C_NAKRCVIEN);
        gI2C_intType = 0;
        //printf(" \n  ###### Write OK ######\n");
        return sendedSize;
    }
    else
    {
        i2c_deinit(i2cPort);
        I2CDisableInt(i2cPort, I2C_MBTFIEN | I2C_NAKRCVIEN);
        return TIMEOUT;
    }
}

_ATTR_SYS_CODE_
int I2C_Read(eI2C_CHANNEL i2cPort, UINT8 RegAddr, UINT8 *pData, UINT16 size, eI2C_RW_mode_t mode)
{
    int readedSize;
    int needRev;
    uint32 intpending;
    int ret;
    int TimeOut;
    needRev = size;
    TimeOut = I2C_TIMEOUT_MS;   //system tick 10ms,here 2's timeout.
    I2CDisableIRQ(i2cPort); //clear all irq
    I2CStart(i2cPort);

    do
    {
        TimeOut--;
//        __WFI();
        DelayUs(1);
    }
    while ((!gI2C_flag[i2cPort]) && (TimeOut > 0));

    if (TimeOut)
    {
        gI2C_flag[i2cPort] = 0;
        TimeOut = I2C_TIMEOUT_MS;
    }
    else
    {
        i2c_deinit(i2cPort);
        return TIMEOUT;
    }

//    I2CEnableInt(i2cPort, I2C_MBRFIEN | I2C_NAKRCVIEN);
    readedSize = i2c_read(i2cPort,
                          gI2C_slaveaddr | ReadMode,
                          RegAddr,
                          I2C_7BIT_ADDRESS_8BIT_REG,
                          pData,
                          size,
                          mode);

    if (readedSize == needRev)
    {
        needRev = 0;
    }
    else
    {
        pData   += readedSize;
        needRev -= readedSize;
    }

    I2CEnableInt(i2cPort, I2C_MBRFIEN | I2C_NAKRCVIEN);

    do
    {
        TimeOut--;
//        __WFI();
        DelayUs(1);
    }
    while ((!gI2C_flag[i2cPort]) && (TimeOut > 0));

    if ( TimeOut )
    {
        gI2C_flag[i2cPort] = 0;
        TimeOut = I2C_TIMEOUT_MS;

        if ( gI2C_intType & I2C_NAKRCVIPD ) //ACK未收到，出错返回，重新再发
        {
            printf("I2C_NAKRCVIPD------\n");
            I2CStop(i2cPort );

            while (!gI2C_flag[i2cPort])
                __WFI();

            gI2C_flag[i2cPort] = 0;
            i2c_deinit(i2cPort);
            I2CDisableInt(i2cPort, I2C_MBRFIEN | I2C_NAKRCVIEN);
            return ERROR;   //need retry to send again.
        }
    }
    else
    {
        i2c_deinit(i2cPort);
        I2CDisableInt(i2cPort, I2C_MBRFIEN | I2C_NAKRCVIEN);
        return TIMEOUT;
    }

    while ( needRev ) //是否继续接收
    {
        if ( gI2C_intType & I2C_MBRFIPD )
        {
            int realsize = 0;
            /*Note:in interrupt handle,set slave address 0x0 as continus Rv flag*/
            realsize = i2c_read(i2cPort,
                                0,
                                RegAddr,
                                I2C_7BIT_ADDRESS_8BIT_REG,
                                pData,
                                needRev,
                                mode);

            do
            {
                TimeOut--;
//                __WFI();
                DelayUs(1);
            }
            while ((!gI2C_flag[i2cPort]) && (TimeOut > 0));

            if (TimeOut)
            {
                gI2C_flag[i2cPort] = 0;
                TimeOut = I2C_TIMEOUT_MS;
                pData       += realsize;
                needRev     -= realsize;
                readedSize  += realsize;
            }
            else
            {
                break;
            }
        }
    }

    I2CStop(i2cPort);

    do
    {
        TimeOut--;
//        __WFI();
        DelayUs(1);
    }
    while ((!gI2C_flag[i2cPort]) && (TimeOut > 0));

    if ( TimeOut )
    {
        gI2C_flag[i2cPort] = 0;
        i2c_deinit(i2cPort);
        I2CDisableInt(i2cPort, I2C_MBRFIEN | I2C_NAKRCVIEN);
        //printf(" \n  ###### Read OK ######\n");
        return readedSize;
    }
    else
    {
        i2c_deinit(i2cPort);
        I2CDisableInt(i2cPort, I2C_MBRFIEN | I2C_NAKRCVIEN);
        return TIMEOUT;
    }
}
/*
********************************************************************************
*
*                         End of I2S.c
*
********************************************************************************
*/

