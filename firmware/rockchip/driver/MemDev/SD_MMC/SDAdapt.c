/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SDAdapt.c
Desc:

Author: chenfen
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/

#include "driver/GRF/hw_grf.h"   /* IOMUX_* pinmux macros */
#include "driver/CRU/Hw_cru.h"  /* HCLK_EMMC_GATE & clock gate IDs */
#include "driver/GPIO/gpio.h"   /* GPIOPortX_PinY enums */
#include "driver/DMA/Hw_dma.h" /* pDMA_LLP, B_CTLL_* (before Dma.h) */
#include "driver/DMA/Dma.h"     /* eDMA_CHN, DMA_CFGX, DMA_CTLL_* (vendor) */
#include "SDConfig.h"


#if defined(SDMMC_DRIVER)

#include "SysInclude.h"

/*------------------------------------ Defines -------------------------------*/


/*----------------------------------- Typedefs -------------------------------*/


/*-------------------------- Forward Declarations ----------------------------*/


/* ------------------------------- Globals ---------------------------------- */


/*-------------------------------- Local Statics: ----------------------------*/
_ATTR_SD_BSS_  SDOAM_EVENT_T gEvent[SDC_MAX];
_ATTR_SD_DATA_ int32 SDA_DMAChannel[SDC_MAX];

/*--------------------------- Local Function Prototypes ----------------------*/


/*------------------------ Function Implement --------------------------------*/

/*
Name:       SDA_Delay
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
void SDA_Delay(uint32 us)
{
    DelayUs(us);
}


/*
Name:       SDA_CreateEvent
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
pEVENT SDA_CreateEvent(uint8 nSDCPort)
{
    gEvent[nSDCPort].event = 0;
    return (&gEvent[nSDCPort].event);
}

/*
Name:       SDA_SetEvent
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
void SDA_SetEvent(pEVENT handle, uint32 event)
{
    *handle |= event;
}

/*
Name:       SDOAM_GetEvent
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
int32 SDOAM_GetEvent(pEVENT handle, uint32 event, uint32 timeout)
{
    if (NO_TIMEOOUT == timeout)
    {
        while (!(*((volatile uint32 *)handle) & event))
        {
             SDA_Delay(1);
        }
    }
    else if (0 == timeout)
    {
        if (!(*((volatile uint32 *)handle) & event))
        {
            return SDC_NO_EVENT;
        }
    }
    else
    {
        while (!(*((volatile uint32 *)handle) & event))
        {
            if (timeout == 0)
            {
                printf("\nSDOAM_GetEvent(TIMEOOUT): event=%d\n", event);
                return SDC_NO_EVENT;
            }
            timeout--;
            SDA_Delay(1);
        }
    }

    DisableIntMaster();
    *handle &= ~(event);
    EnableIntMaster();

    return SDC_SUCCESS;
}


/*
Name:       SDA_DMAStart
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
bool SDA_DMAStart(SDMMC_PORT_E nSDCPort, uint32 DstAddr, uint32 SrcAddr, uint32 size, bool rw, pFunc CallBack)
{
    uint32 timeout = 0;
    eDMA_CHN channel;

    if (rw)
    {
        //write
        DMA_CFGX DmaCfg_SD = {DMA_CTLL_SDMMC_WR, DMA_CFGL_SDMMC_WR, DMA_CFGH_SDMMC_WR,0};
        DMA_CFGX DmaCfg_eMMC = {DMA_CTLL_EMMC_WR, DMA_CFGL_EMMC_WR, DMA_CFGH_EMMC_WR,0};
        DMA_CFGX *pDmaCfg;
        if(nSDCPort == SDC0)
        {
            pDmaCfg = &DmaCfg_SD;
        }
        else if (nSDCPort == SDC1)
        {
            pDmaCfg = &DmaCfg_eMMC;
        }

        channel = DmaGetChannel();
        if ((channel != DMA_FALSE)/* && (channel < DMA_CHN2)*/)
        {
            SDA_DMAChannel[nSDCPort] = DmaStart((uint32)(channel),(uint32)SrcAddr,(uint32)DstAddr, size,pDmaCfg, CallBack);
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        //read
        DMA_CFGX DmaCfg_SD   = {DMA_CTLL_SDMMC_RD, DMA_CFGL_SDMMC_RD, DMA_CFGH_SDMMC_RD,0};
        DMA_CFGX DmaCfg_eMMC = {DMA_CTLL_EMMC_RD, DMA_CFGL_EMMC_RD, DMA_CFGH_EMMC_RD,0};
        DMA_CFGX *pDmaCfg;
        if(nSDCPort == SDC0)
        {
            pDmaCfg = &DmaCfg_SD;
        }
        else if (nSDCPort == SDC1)
        {
            pDmaCfg = &DmaCfg_eMMC;
        }

        channel = DmaGetChannel();
        if ((channel != DMA_FALSE)/* && (channel < DMA_CHN2)*/)
        {
            SDA_DMAChannel[nSDCPort] = DmaStart((uint32)(channel),(uint32)SrcAddr,(uint32)DstAddr,size,pDmaCfg, CallBack);
        }
        else
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*
Name:       SDA_DMAStop
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
bool SDA_DMAStop(SDMMC_PORT_E nSDCPort)
{
    uint32 timeout = 20000;

    if (SDA_DMAChannel[nSDCPort] != -1)
    {
        while(timeout--)
        {
            if (DMA_SUCCESS == DmaGetState(SDA_DMAChannel[nSDCPort]))
            {
                break;
            }
            DelayUs(10);
        }
    }
    SDA_DMAChannel[nSDCPort] = -1;

    return TRUE;
}

/*
Name:       SDA_GetSrcClkFreq
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
uint32 SDA_GetSrcClkFreq(SDMMC_PORT_E nSDCPort, uint32 FreqKHz, uint32 *SrcSel)
{
    uint32 SrcFreq;
    uint32 PllFreq = GetPllFreq()/1000;
    uint32 HclkFreq = GetHclkSysCoreFreq()/1000;

    if ((FreqKHz <= 24000) || (HclkFreq <= 24000))
    {
        *SrcSel = 1;//select 24MHZ
        SrcFreq = 24000;
    }
    else
    {
        *SrcSel = 0;//select PLL
        SrcFreq = PllFreq;
    }

    return SrcFreq;
}


/*
Name:       SDA_SetSrcClkDiv
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
int32 SDA_SetSrcClkDiv(SDMMC_PORT_E nSDCPort, uint32 div, uint32 SrcSel)
{
    if (div > 0)
    {
        if(nSDCPort == SDC0)
        {
             CRU->CRU_CLKSEL_CON[7] = ((SrcSel&0x1)<<7) | ((div-1)<<0)| (0x9f<<16);
        }
        else if(nSDCPort == SDC1)   //EMMC
        {
             CRU->CRU_CLKSEL_CON[7] = (SrcSel&0x1) << 15 | ((div-1)<<8)| (0x9f<<24);
        }
    }
	return SDC_SUCCESS;
}

/*
Name:       SDA_SetSrcClk
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_CODE_
void SDA_SetSrcClk(SDMMC_PORT_E nSDCPort, bool enable)
{
    if(nSDCPort == SDC0)
    {
         CRU->CRU_CLKGATE_CON[1] = (((TRUE == enable)? 0:1)<<8) | (1<<(8+16));
    }
    else if(nSDCPort == SDC1)
    {
         CRU->CRU_CLKGATE_CON[1] = (((TRUE == enable)? 0:1)<<9) | (1<<(9+16));
    }
}

/*
Name:       SDA_RegISR
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
bool SDA_RegISR(SDMMC_PORT_E nSDCPort, pFunc Routine)
{
    if(nSDCPort == SDC0)
    {
        IntRegister(INT_ID_SDMMC, (void*)Routine);
    }
    else if(nSDCPort == SDC1)
    {
        IntRegister(INT_ID_EMMC, (void*)Routine);
    }
    else
    {

    }
    return TRUE;

}

/*
Name:       SDA_EnableIRQ
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
void SDA_EnableIRQ(SDMMC_PORT_E nSDCPort)
{
    if(nSDCPort == SDC0)
    {
        IntEnable(INT_ID_SDMMC);
    }
    else if(nSDCPort == SDC1)
    {
        IntEnable(INT_ID_EMMC);
    }
    else
    {

    }

}


/*
Name:       SDA_SetIOMux
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
bool SDA_SetIOMux(SDMMC_PORT_E SDCPort, HOST_BUS_WIDTH width)
{
    return TRUE;
}

/*
Name:       SDA_SetIOMux
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_DRIVERLIB_CODE_
bool SDA_CheckCard(SDMMC_PORT_E         SDCPortd)
{
#if(RKNANOD_CHIP_TYPE == RKNANOD_N)
    if (SDCPortd == SDC0)
    {
        return (0 == Gpio_GetPinLevel(GPIO_CH2,GPIOPortB_Pin6))? TRUE:FALSE; // sdio -> tf
    }
    else //if (SDCPortd == SDC1)
    {
        return (0 == Gpio_GetPinLevel(GPIO_CH2,GPIOPortB_Pin6))? TRUE:FALSE; // emmc -> tf
    }
#elif(RKNANOD_CHIP_TYPE == RKNANOD_G || RKNANOD_CHIP_TYPE == RKNANOD_L)
    if (SDCPortd == SDC0)
    {
        return (0 == Gpio_GetPinLevel(GPIO_CH0,GPIOPortB_Pin3))? TRUE:FALSE; // sdio -> tf
    }
    else //if (SDCPortd == SDC1)
    {
        return (0 == Gpio_GetPinLevel(GPIO_CH0,GPIOPortB_Pin3))? TRUE:FALSE; // emmc -> tf
    }
#endif
}

/******************************************************************************
 * SDA_Init -
 * DESCRIPTION: - when nSDCPort == 0,func can only use for sd card.
 *                 when nSDCPort == 1,func can only use for sd card and emmc.
 *
 * Input: nSDCPort:  0 use the  sdmmc control, 1 use the emmc control
 *        func    :  0 use the sd card funtion, 1 the the emmc funtion
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.2.27  written
 * Time : 14:30:17
 * --------------------
 ******************************************************************************/
_ATTR_SD_INIT_CODE_
void SDA_Init(SDMMC_PORT_E nSDCPort, SDMMC_PORT_FUNCTION func)
{
    if (SDC0 == nSDCPort)
    {
        ScuClockGateCtr(CLK_SDMMC_GATE, 1);
        ScuClockGateCtr(HCLK_SDMMC_GATE, 1);
        ScuSoftResetCtr(SDMMC_SRST,0);

            //select 24m 24MHz/32=750KHz
        //CRUReg->CRU_CLKSEL_CON[7] = (((0x1F<<0)|(1<<7))<<16)|(0x1F<<0)|(1<<7);
        SetSdmmcFreq(XIN24M,750*1000);

        //sd_cmd,sd_clk,sd_d0 iomux
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin5,IOMUX_GPIO1A5_SDMMC_CMD);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin6,IOMUX_GPIO1A6_SDMMC_CLK);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin7,IOMUX_GPIO1A7_SDMMC_D0);

        //sd_d1~d3 iomux
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin0,IOMUX_GPIO1B0_SDMMC_D1);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_SDMMC_D2);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin2,IOMUX_GPIO1B2_SDMMC_D3);
        DelayUs(2);
	}
    else    // emmc controller
    {
        ScuClockGateCtr(CLK_EMMC_GATE, 1);
        ScuClockGateCtr(HCLK_EMMC_GATE, 1);
        ScuSoftResetCtr(SDMMC_SRST,0);

    #ifdef _EMMC_   //eMMC Flash
            //select 24m 24MHz/32=750KHz
        //CRUReg->CRU_CLKSEL_CON[7] = (((0x1F<<8)|(1<<15))<<16)|(0x1F<<8)|(1<<15);
        SetSdioFreq(XIN24M,750*1000);

        //emmc_pwren,emmc_clk,emmc_cmd,emmc_d0~d4 iomux
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin0,IOMUX_GPIO0A0_EMMC_PWREN);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_EMMC_CLK);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_EMMC_CMD);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_EMMC_D0);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_EMMC_D1);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_EMMC_D2);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_EMMC_D3);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7,IOMUX_GPIO0A7_EMMC_D4);

        //emmc_d5~d7 iomux
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0,IOMUX_GPIO0B0_EMMC_D5);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1,IOMUX_GPIO0B1_EMMC_D6);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2,IOMUX_GPIO0B2_EMMC_D7);

        //emmc_rstn iomux
        //Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin3,IOMUX_GPIO1B3_EMMC_RSTN);

        //GRFReg->GRF_GPIO0A_IOMUX = (0xFFFF<<16)| 0x5555;   //emmc_pwren,emmc_clk,emmc_cmd,emmc_d0~d4
        //GRFReg->GRF_GPIO0B_IOMUX = ((0x3F<<0)<<16)|(0x15<<0);    //emmc_d5~d7
        //GRFReg->GRF_GPIO1B_IOMUX = ((0x3<<6)<<16)|(0x1<<6);      //emmc_rstn
    #else   //SDC1
        //CRUReg->CRU_CLKSEL_CON[7] = (((0x1F<<8)|(1<<15))<<16)|(0x1F<<8)|(1<<15);
        SetSdioFreq(XIN24M,750*1000);

        //GRFReg->GRF_GPIO0A_IOMUX = (0x3FFF<<16)| 0x1555;   //emmc_pwren,emmc_clk,emmc_cmd,emmc_d0~d3
        //emmc_pwren,emmc_clk,emmc_cmd,emmc_d0~d3 iomux
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin0,IOMUX_GPIO0A0_EMMC_PWREN);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_EMMC_CLK);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_EMMC_CMD);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_EMMC_D0);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_EMMC_D1);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_EMMC_D2);
        Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_EMMC_D3);
    #endif
    }
    SDA_DMAChannel[nSDCPort] = -1;
}

#endif
