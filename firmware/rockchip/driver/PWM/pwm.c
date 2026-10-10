/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   Pwm.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*             anzhiguo      2008-11-20         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_PWM_

#include "DriverInclude.h"


const uint32 PWM_GROUP[2] =
{
    PWM0_BASE,
    PWM1_BASE,
};

#define PWMPORT(n)          ((PWM_REG*)(PWM_GROUP[n]))

#define PWMPORT0_MAXCHN     4

_ATTR_DRIVER_BSS_
uint8 gPwmChnAlive[PWMPORT0_MAXCHN];
/*
--------------------------------------------------------------------------------
  Function name : void PWM_Start(int num)
  Author        : LinChongSen
  Description   : 启动PWM 输出

  Input         : num -- PWM号(0~4)

  Return        : *rate -- 当前PWM的占空比
                  *PWM_freq  -- 当前PWM的频率
--------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void PWM_Start(ePWM_CHN ch)
{
    PWM_REG* pPwmReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
    {
        ScuClockGateCtr(PCLK_PWM0_GATE, 1);

        ScuSoftResetCtr(PWM0_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(PWM0_SRST, 0);

        gPwmChnAlive[ch] = 1;
        pPwmReg = PWMPORT(0);
    }
    else
    {
        ScuClockGateCtr(PCLK_PWM1_GATE, 1);

        ScuSoftResetCtr(PWM1_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(PWM1_SRST, 0);

        pPwmReg = PWMPORT(1);
    }

    uReg = (uint32)&(pPwmReg->PWM0_CTRL);
    uReg = uReg + 16 * ch;

    *(uint32 volatile *)uReg |= PWM_ENABLE;

    PWM_GPIO_Init(BL_PWM_CH);

}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_Stop(int num)
  Author        : LinChongSen
  Description   : 关闭PWM 输出

  Input         : num -- PWM号(0~4)

  Return        :
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void PWM_Stop(ePWM_CHN ch)
{
    PWM_REG* pReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
    {
        pReg = PWMPORT(1);
        ch -= 4;
    }

    uReg = (uint32)&(pReg->PWM0_CTRL);
    uReg = uReg + 16 * ch;

    *(uint32 volatile *)uReg &= ~(PWM_ENABLE);

    if( ch < PWM_CHN4)
    {
        int i = 0;
        uint8  flag = 0;

        for(i = 0;i < PWMPORT0_MAXCHN; i++)
        {
            if(i != ch && gPwmChnAlive[i] == 1)
            {
                flag = 1;
                break;
            }
        }

        if(flag)    //have other pwm channels alive,here return.
        {
            return;
        }

        ScuClockGateCtr(PCLK_PWM0_GATE, 0);
        gPwmChnAlive[ch] = 0;
    }
    else //pwm port 1 channel 4
    {
        ScuClockGateCtr(PCLK_PWM1_GATE, 0);
    }

    PWM_GPIO_DeInit(ch);
}

/*
--------------------------------------------------------------------------------
  Function name : void PwmRegReset(int num)
  Author        : LinChongSen
  Description   : 复位PWM

  Input         : num -- PWM号(0~4)

  Return        :
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void PwmRegReset(ePWM_CHN ch)
{
    PWM_REG* pReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
        pReg = PWMPORT(1);

    memset(pReg,0, sizeof(PWM_REG));
}

/*
--------------------------------------------------------------------------------
  Function name : void PwmPrescalefFctorSet(int num,int mpwmPrescalefFctor)
  Author        : LinChongSen
  Description   : PWM 分频率设置

  Input         : num -- PWM号(0~4)
                  mpwmPrescalefFctor -- 0000: 1/2             0001: 1/4  (0~15)
                                        0010: 1/8             0011: 1/16
                                        0100: 1/32            0101: 1/64
                                        0110: 1/128           0111: 1/256
                                        1000: 1/512           1001: 1/1024
                                        1010: 1/2048          1011: 1/4096
                                        1100: 1/8192          1101: 1/16384
                                        1110: 1/32768         1111: 1/65536

  Return        :
  desc          : 不调用，默认为1/2分频
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
void PwmPrescalefFctorSet(ePWM_CHN ch, ePWM_PRESCALE mpwmPrescalefFctor)
{
    PWM_REG* pReg;
    uint32  uReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
        pReg = PWMPORT(1);

    uReg = (uint32)&(pReg->PWM0_CTRL);
    uReg = uReg + 16 * ch;

    *(uint32 volatile *)uReg |= (mpwmPrescalefFctor << 12);
}

/*
--------------------------------------------------------------------------------
  Function name : int PwmPrescalefFctorGet(int num)
  Author        : LinChongSen
  Description   : PWM 分频率读取

  Input         : num -- PWM号(0~4)
  Output        : mpwmPrescalefFctor  -- 返回2的次方数 如:2，4，8，16....65536
                                         可以参考PwmPrescalefFctorSet中的mpwmPrescalefFctor注释
  Return        :
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
uint32 PwmPrescalefFctorGet(ePWM_CHN ch)
{
    PWM_REG* pReg;
    uint32  uReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
        pReg = PWMPORT(1);

    uReg = (uint32)&(pReg->PWM0_CTRL);
    uReg = uReg + 16 * ch;

    return (*(uint32 volatile *)uReg & 0x00007000) >> 12;
}


/*
--------------------------------------------------------------------------------
  Function name : BOOL PwmRateSet(int num,UINT32 rate, UINT32 PWM_freq)
  Author        : LinChongSen
  Description   : PWM 占空比设置

  Input         : num -- PWM号(0~4)
                  rate -- 高与低的比值(0~100)
                  PWM_freq  -- 当前PWM的频率(<10k)

  Return        :
  desc          : data1 过小,则无法正常设置
--------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
int32 PwmRateSet(ePWM_CHN ch, UINT32 rate, UINT32 PWM_freq)
{
    UINT32 data0,data1;
    UINT32 pwmclk;
    UINT32 periodData;
    UINT32 pwmPrescalefFctor;
    PWM_REG* pReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
    {
        pReg = PWMPORT(1);
        ch -= 4;
    }

    if(rate > 100)
        return ERROR;

    if (rate == 100)
    {
        uReg = (uint32)&(pReg->PWM0_CTRL);
        uReg = uReg + 16 * ch;  //16 BYTES
        *(uint32 volatile *)uReg &= ~(PWM_ENABLE);

        return OK;
    }
    else if (rate == 0)
    {
        uReg = (uint32)&(pReg->PWM0_CTRL);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg &= ~(PWM_ENABLE);

        return OK;
    }

    if((rate > 0) && (rate < 100))
    {
        pwmclk = GetPWMFreq();
        periodData = pwmclk / PWM_freq;

        //DEBUG("periodData = %d",periodData);

        uReg = (uint32)&(pReg->PWM0_DUTY_LPR);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg = rate * periodData / 100; //duty in period

        uReg = (uint32)&(pReg->PWM0_CNT);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg = 0;

        uReg = (uint32)&(pReg->PWM0_PERIOD_HPR);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg = periodData;   //output pwm-clk period.

        //repeat counter[7:0]<<24 | scale factor[7:0]<<16 |
        //prescale factor[2:0]<<12 | clk_sel<<9 | lp_en<<8 |
        //output mode<<5 | inactive pol<<4 | duty pol<<3 |
        //pwm mode[1:0]<<1 | pwm enable<<0

//if clk_sel [non scaled clock],this value no effect
/*continuous mode*/
        uReg = (uint32)&(pReg->PWM0_CTRL);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg = PWM_RPT_CNT(0x0) \
                        |PWM_SCALE_FACTOR(1) \
                        |PWM_PRESCALE_FACTOR(0) \
                        |PWM_CLK_SEL(NON_SCALED_CLOCK) \
                        |PWM_LP_DISABLE \
                        |PWM_OUTPUT_MODE(OUTPUT_MODE_LEFT) \
                        |PWM_INACTIVE_POLARITY(POLARITY_NEG) \
                        |PWM_DUTY_POLARITY(POLARITY_NEG) \
                        |PWM_CON_MODE(PWM_MODE_CONTINUOUS) \
                        |PWM_ENABLE;


/*one shot mode*/
//        uReg = (uint32)&(pReg->PWM0_CTRL);
//        uReg = uReg + 16 * ch;
//        *(uint32 volatile *)uReg = PWM_RPT_CNT(10) \
//                                |PWM_SCALE_FACTOR(2) \
//                                |PWM_PRESCALE_FACTOR(1) \
//                                |PWM_CLK_SEL(NON_SCALED_CLOCK) \
//                                |PWM_LP_DISABLE \
//                                |PWM_OUTPUT_MODE(OUTPUT_MODE_LEFT) \
//                                |PWM_INACTIVE_POLARITY(POLARITY_NEG) \
//                                |PWM_DUTY_POLARITY(POLARITY_NEG) \
//                                |PWM_CON_MODE(PWM_MODE_ONESHOT) \
//                                |PWM_ENABLE;


        return OK;
    }

    return ERROR;
}


/*
--------------------------------------------------------------------------------
  Function name : void PWM_SetIntEnable(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM使能中断

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          :
--------------------------------------------------------------------
*/
void PWM_SetIntEnable(ePWM_CHN ch)
{
    PWM_REG* pPwmReg;

    if( ch < PWM_CHN4)
        pPwmReg = PWMPORT(0);
    else
        pPwmReg = PWMPORT(1);

    switch(ch)
    {
        case PWM_CHN0:
            pPwmReg->PWM_INT_EN = PWM_CHN0_INT_EN;
            break;

        case PWM_CHN1:
            pPwmReg->PWM_INT_EN = PWM_CHN1_INT_EN;
            break;

        case PWM_CHN2:
            pPwmReg->PWM_INT_EN = PWM_CHN2_INT_EN;
            break;

        case PWM_CHN3:
            pPwmReg->PWM_INT_EN = PWM_CHN3_INT_EN;
            break;

        case PWM_CHN4:
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_SetIntDisable(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM中断除能

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          :
--------------------------------------------------------------------
*/
void PWM_SetIntDisable(ePWM_CHN ch)
{
    PWM_REG* pPwmReg;

    if( ch < PWM_CHN4)
        pPwmReg = PWMPORT(0);
    else
        pPwmReg = PWMPORT(1);

    switch(ch)
    {
        case PWM_CHN0:
            pPwmReg->PWM_INT_EN = PWM_CHN0_INT_DISABLE;
            break;

        case PWM_CHN1:
            pPwmReg->PWM_INT_EN = PWM_CHN1_INT_DISABLE;
            break;

        case PWM_CHN2:
            pPwmReg->PWM_INT_EN = PWM_CHN2_INT_DISABLE;
            break;

        case PWM_CHN3:
            pPwmReg->PWM_INT_EN = PWM_CHN3_INT_DISABLE;
            break;

        case PWM_CHN4:
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_GetIntStatus(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM获取中断状态值

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          :
--------------------------------------------------------------------
*/
int PWM_GetIntStatus(ePWM_CHN ch)
{
    int intStatus;
    PWM_REG* pPwmReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pPwmReg = PWMPORT(0);
    else
        pPwmReg = PWMPORT(1);

    uReg = (uint32)pPwmReg;

    intStatus =  pPwmReg->PWM_INTSTS;

    if(intStatus & PWM_CH0INT_STAS == PWM_CH0INT_STAS)
    {
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x1;
    }
    else if(intStatus & PWM_CH1INT_STAS == PWM_CH1INT_STAS)
    {
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x2;
    }
    else if(intStatus & PWM_CH2INT_STAS == PWM_CH2INT_STAS)
    {
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x4;
    }
    else if(intStatus & PWM_CH3INT_STAS == PWM_CH3INT_STAS)
    {
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x8;
    }

    return intStatus;
}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_GetHPolarity(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM获取某一通道输入波形的高极性时长

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          :only in  capture mode
--------------------------------------------------------------------
*/
int PWM_GetHPolarity(ePWM_CHN ch)
{
    uint32  highCycle;  //effective high cycle
    uint32  intStatus;
    PWM_REG* pPwmReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pPwmReg = PWMPORT(0);
    else
        pPwmReg = PWMPORT(1);

    intStatus =  pPwmReg->PWM_INTSTS;

    if(intStatus & PWM_CH0_POL == PWM_CH0_POL)
    {
        highCycle = pPwmReg->PWM0_PERIOD_HPR;
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x1;
    }
    else if(intStatus & PWM_CH1_POL == PWM_CH1_POL)
    {
        highCycle = pPwmReg->PWM1_PERIOD_HPR;
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x2;
    }
    else if(intStatus & PWM_CH2_POL == PWM_CH2_POL)
    {
        highCycle = pPwmReg->PWM2_PERIOD_HPR;
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x4;
    }
    else if(intStatus & PWM_CH3_POL == PWM_CH3_POL)
    {
        highCycle = pPwmReg->PWM3_PERIOD_HPR;
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x8;
    }

    return highCycle;

}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_GetLPolarity(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM某一通道输入波形的低极性时长

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          : only in  capture mode
--------------------------------------------------------------------
*/
int PWM_GetLPolarity(ePWM_CHN ch)
{
    uint32  lowCycle;  //effective low cycle
    uint32 intStatus;
    PWM_REG* pPwmReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pPwmReg = PWMPORT(0);
    else
        pPwmReg = PWMPORT(1);

    intStatus =  pPwmReg->PWM_INTSTS;

    if(intStatus & PWM_CH0_POL != PWM_CH0_POL)
    {
        lowCycle = pPwmReg->PWM0_PERIOD_HPR;
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x1;
    }
    else if(intStatus & PWM_CH1_POL != PWM_CH1_POL)
    {
        lowCycle = pPwmReg->PWM1_PERIOD_HPR;
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x2;
    }
    else if(intStatus & PWM_CH2_POL != PWM_CH2_POL)
    {
        lowCycle = pPwmReg->PWM2_PERIOD_HPR;
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x4;
    }
    else if(intStatus & PWM_CH3_POL != PWM_CH3_POL)
    {
        lowCycle = pPwmReg->PWM3_PERIOD_HPR;
        //clear interrupt polarity flag
        pPwmReg->PWM_INTSTS = 0x8;
    }

    return lowCycle;

}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_GetIntStatus(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM获取中断状态值

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          :
--------------------------------------------------------------------
*/
void PWM_ClrInt(ePWM_CHN ch)
{
    int intStatus;

    PWM_GetIntStatus(ch);
}

void PWM_GPIO_Init(ePWM_CHN ch)
{
    switch( ch )
    {
        case PWM_CHN0:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_PWM0_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin4, GPIO_OUT);
            break;

        case PWM_CHN1:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_PWM1_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3, GPIO_OUT);
            break;

        case PWM_CHN2:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin2,IOMUX_GPIO2A2_PWM2_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin2, GPIO_OUT);
            break;

        case PWM_CHN3:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_PWM3_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin1, GPIO_OUT);
            break;

        case PWM_CHN4:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_PWM4_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin0, GPIO_OUT);
            break;

        default:
            break;
    }

}

void PWM_GPIO_DeInit(ePWM_CHN ch)
{
    switch( ch )
    {
        case PWM_CHN0:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin4,GPIO_OUT);
            Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin4,GPIO_LOW);
            break;

        case PWM_CHN1:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3, GPIO_OUT);
            Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin3,GPIO_LOW);
            break;

        case PWM_CHN2:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin2,IOMUX_GPIO2A2_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin2, GPIO_OUT);
            Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin2,GPIO_LOW);
            break;

        case PWM_CHN3:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin1, GPIO_OUT);
            Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin1,GPIO_LOW);
            break;

        case PWM_CHN4:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin0, GPIO_OUT);
            Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin0,GPIO_LOW);
            break;

        default:
            break;
    }

}


void PWM_LowPowerEnable(ePWM_CHN ch,bool enable)
{
    PWM_REG* pPwmReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pPwmReg = PWMPORT(0);
    else
    {
        pPwmReg = PWMPORT(1);
        ch -= 4;
    }

    uReg = (uint32)&(pPwmReg->PWM0_CTRL);
    uReg = uReg + 16 * ch;


    if(enable)
        *(uint32 volatile *)uReg  |= PWM_LP_ENABLE;
    else
        *(uint32 volatile *)uReg  |= PWM_LP_DISABLE;

}

/*
********************************************************************************
*
*                         End of pwm.c
*
********************************************************************************
*/

