/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name��   PowerManager.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo         2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_POWER

#include "SysInclude.h"
#include "PowerManager.h"
#include "DriverInclude.h"
#include "interrupt.h"
#include "Msg.h"

/*
--------------------------------------------------------------------------------

                        Variable Define

--------------------------------------------------------------------------------
*/
//typedef __int64 uint64 ;

typedef void(*ExecFunPtr)(void) __irq;
extern UINT32 Image$$AP_MAIN_STACK$$ZI$$Limit;
extern uint32 BbSystemStartOK;
extern uint32 BBsystemHoldState;

void PmuResume();
void Freq_VopTiming_Update(uint32 Freq);

_ATTR_DRIVER_BSS_   CRU_CLK_INFO    g_chipClk;
_ATTR_DRIVER_BSS_   uint64          g_APPList;
_ATTR_DRIVER_BSS_   uint64          g_APPDisableList;
_ATTR_DRIVER_DATA_  uint8           FreqDisableFlag = 0;
_ATTR_DRIVER_DATA_  uint8           FreqDisableCnt = 0;



FREQ_APP_TABLE g_CruAPPTabel[FREQ_APP_MAX] =
{
    //          ID,      counter,       pll,  sys_hclk,  sys_stclk, sys_pclk,  cal_hclk,  cal_stclk
    {FREQ_IDLE,            0,         0,         0,         0,         0,         0,         0},
    {FREQ_MIN,             0,  24000000,  24000000,  24000000,  24000000,         0,         0},
    {FREQ_INIT,            0,  96000000,  96000000,  96000000,  96000000,         0,         0},
    {FREQ_BLON,            0,  24000000,  24000000,  24000000,  24000000,         0,         0},
    {FREQ_AUDIO_INIT,      0,  48000000,  48000000,  48000000,  48000000,  48000000,  48000000},
    {FREQ_MP3,             0,  24000000,  24000000,  24000000,  24000000,  24000000,  24000000},
    {FREQ_MP3H,            0,  36000000,  36000000,  36000000,  36000000,  36000000,  36000000},
    {FREQ_WAV,             0,  88000000,  88000000,  88000000,  88000000,  88000000,  88000000},
    {FREQ_AAC,             0,  80000000,  80000000,  80000000,  80000000,  80000000,  80000000},
    {FREQ_AACL,            0,  18000000,  18000000,  18000000,  18000000,  36000000,  36000000},
    {FREQ_APE,             0, 150000000, 150000000, 150000000,  75000000, 150000000, 150000000},
    {FREQ_FLAC,            0, 150000000, 150000000, 150000000,  75000000, 150000000, 150000000},
    {FREQ_OGG,             0,  80000000,  80000000,  80000000,  80000000, 120000000, 120000000},
    {FREQ_NOGG,            0, 120000000,  80000000,  80000000,  80000000, 120000000, 120000000},
    {FREQ_HOGG,            0, 180000000,  80000000,  80000000,  80000000, 180000000, 180000000},
    {FREQ_EHOGG,           0, 260000000,  80000000,  80000000,  80000000, 260000000, 260000000},
    {FREQ_HAPE,            0, 220000000,  80000000,  80000000,  80000000, 400000000, 400000000},
    {FREQ_HFLAC,           0, 220000000,  80000000,  80000000,  80000000, 220000000, 220000000},
    {FREQ_HALAC,           0, 220000000,  80000000,  80000000,  80000000, 400000000, 400000000},
    {FREQ_DSDIFF,          0, 260000000, 150000000,  80000000,  80000000, 400000000, 400000000},
    {FREQ_DSF,             0, 260000000, 150000000,  80000000,  80000000, 400000000, 400000000},
    {FREQ_SBC_ENCODING,    0,  20000000,  20000000,  20000000,  10000000,         0,         0},
    {FREQ_BLUETOOTH,       0,  30000000,  30000000,  30000000,  30000000,         0,         0},
    {FREQ_SSRC,            0,  40000000,  40000000,  40000000,  40000000,         0,         0},
    {FREQ_EQ_L,            0,  36000000,  36000000,  36000000,  36000000,  36000000,  36000000},
    {FREQ_EQ,              0,  72000000,  72000000,  72000000,  72000000,  72000000,  72000000},
    {FREQ_EQ_H,            0, 140000000, 140000000, 140000000, 140000000, 140000000, 140000000},
    {FREQ_JPG,             0, 250000000, 200000000, 200000000,  75000000,         0,         0},
    {FREQ_FM,              0,  36000000,  36000000,  36000000,  36000000,         0,         0},
    {FREQ_RECORDADPCM,     0,  20000000,  96000000,  96000000,  75000000, 192000000, 192000000},
    {FREQ_FMAUTOSEARCH,    0,  24000000,  24000000,  24000000,  24000000,         0,         0},
    {FREQ_MEDIAUPDATA,     0, 400000000, 200000000, 200000000,  75000000,         0,         0},
    {FREQ_USB,             0, 400000000, 200000000, 200000000,  75000000, 400000000, 400000000}, //USB ����Ƶ
    {FREQ_BEEP,            0,  24000000,  24000000,  24000000,  24000000,         0,         0},
    {FREQ_MEDIA_INIT,      0, 100000000, 100000000, 100000000,  75000000,         0,         0},
    {FREQ_AVI,             0, 100000000, 100000000, 100000000,  75000000,         0,         0},

    {FREQ_MAX,             0, 400000000, 200000000, 200000000,  75000000, 400000000, 400000000},
};

/*
--------------------------------------------------------------------------------
  Function name : void FreqSetPrevProcess(uint32 nMHz)
  Author        :
  Description   : Prepare for Freq Change.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                                2009-3-24         Ver1.0
--------------------------------------------------------------------------------
*/


void FreqSetPrevProcess(void)
{
    uint32 timeout;

    UserIsrDisable();

    if (BbSystemStartOK)
    {
        BBsystemHoldState = 0;
        MailBoxWriteA2BCmd(MSGBOX_CMD_BB_HOLD,  MAILBOX_ID_0, MAILBOX_CHANNEL_0);
        MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_0);

        timeout = 50000;
        while(!BBsystemHoldState)
        {
#ifdef _WATCH_DOG_
            WatchDogReload();
#endif

            BBDebug();
            //__WFI();
            DelayUs(1);
            if (--timeout == 0)
            {
                DEBUG("FreqSetPrevProcess: timeout!!!");
                break;
            }
        }
    }

    DmaSuspend(DMA_CHN0);
    DmaSuspend(DMA_CHN1);
    DmaSuspend(DMA_CHN2);
    DmaSuspend(DMA_CHN3);
    DmaSuspend(DMA_CHN4);
}

/*
--------------------------------------------------------------------------------
  Function name : void FreqSetPostProcess(uint32 nMHz)
  Author        :
  Description   : Prepare for Freq Change.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                                2009-3-24         Ver1.0
--------------------------------------------------------------------------------
*/


void FreqSetPostProcess(void)
{
    if (BbSystemStartOK)
    {
        MailBoxWriteA2BCmd(MSGBOX_CMD_BB_HOLD_EXIT,  MAILBOX_ID_0, MAILBOX_CHANNEL_0);
        MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    }
    DmaResume(DMA_CHN0);
    DmaResume(DMA_CHN1);
    DmaResume(DMA_CHN2);
    DmaResume(DMA_CHN3);
    DmaResume(DMA_CHN4);
    UserIsrEnable();
}

/*
--------------------------------------------------------------------------------
  Function name : void PMUSetARMFreq(uint32 nMHz)
  Author        :
  Description   : set arm main frequency,HCLK,PCLK.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                                2009-3-24         Ver1.0
  desc:         ARM, HCLK, PCLK's frequency should in valid range
--------------------------------------------------------------------------------
*/


void FREQSetARMFreq(FREQ_APP_TABLE *FreqTab, chip_freq_t *pChipFreq)
{
    uint32 i,j,k;
    uint32 pllclk;
    uint32 syshclk_div;
    uint32 sysstclk_div;
    uint32 pclk_dkv;
    uint32 calhclk_div;
    uint32 calstclk_div;
    uint32 osc_24m_div;

    if ((FreqTab->syshclk <= 24000000) && (FreqTab->calhclk <= 24000000))
    {
        pllclk = 24000000;
        if (FreqTab->syshclk == 0)
        {
            FreqTab->syshclk = 24000000 / 64;
            FreqTab->sysstclk = FreqTab->syshclk;
            FreqTab->syspclk  = FreqTab->syshclk;
        }
        if (FreqTab->calhclk == 0)
        {
            FreqTab->calhclk = 24000000 / 64;
            FreqTab->calstclk = FreqTab->calhclk;
        }
        if (FreqTab->syshclk >= FreqTab->calhclk)
        {
            osc_24m_div = 24000000 / FreqTab->syshclk;
            pllclk = 24000000 / osc_24m_div;
        }
        else if (FreqTab->syshclk < FreqTab->calhclk)
        {
            osc_24m_div = 24000000 / FreqTab->calhclk;
            pllclk = 24000000 / osc_24m_div;
        }

        syshclk_div = pllclk / FreqTab->syshclk;
        calhclk_div = pllclk / FreqTab->calhclk;
    }
    else
    {
        if (FreqTab->syshclk == 0)
        {
            FreqTab->syshclk = FreqTab->calhclk;
        }
        else if (FreqTab->calhclk == 0)
        {
            FreqTab->calhclk = FreqTab->syshclk;
        }

        if (((FreqTab->PllFreq % FreqTab->syshclk) == 0) && ((FreqTab->PllFreq % FreqTab->calhclk) == 0))
        {
            pllclk       = FreqTab->PllFreq;
            syshclk_div  = FreqTab->PllFreq / FreqTab->syshclk;
            calhclk_div  = FreqTab->PllFreq / FreqTab->calhclk;
        }
        else if (FreqTab->syshclk > FreqTab->calhclk)
        {
            for (i = 1; i < 9; i++)
            {
                pllclk = FreqTab->calhclk * i;
                if (pllclk > 1600000000)
                {
                    pllclk = FreqTab->calhclk * (i - 1);
                    if (pllclk < FreqTab->syshclk)
                    {
                        pllclk = FreqTab->syshclk;
                    }
                    syshclk_div = pllclk / FreqTab->syshclk;
                    pllclk      = FreqTab->syshclk * syshclk_div;     //PLL ��ϵͳƵ�ʶ����趨
                    calhclk_div = pllclk / FreqTab->calhclk;
                    if (calhclk_div > 8)
                    {
                        calhclk_div = 8;
                    }
                    break;
                }
                if ((pllclk % FreqTab->syshclk) == 0)
                {
                    calhclk_div = i;
                    syshclk_div = pllclk / FreqTab->syshclk;
                    break;
                }
            }

            if (i >= 9)
            {
                pllclk      = FreqTab->calhclk * 8;
                if (pllclk < FreqTab->syshclk)
                {
                    pllclk = FreqTab->syshclk;
                }
                syshclk_div = pllclk / FreqTab->syshclk;
                pllclk      = FreqTab->syshclk * syshclk_div;     //PLL ��ϵͳƵ�ʶ����趨
                calhclk_div = pllclk / FreqTab->calhclk;
                if (calhclk_div > 8)
                {
                    calhclk_div = 8;
                }
            }
        }
        else // if (FreqTab->syshclk < FreqTab->calhclk)
        {
            for (i = 1; i < 9; i++)
            {
                pllclk = FreqTab->syshclk * i;
                if (pllclk > 1600000000)
                {
                    pllclk      = FreqTab->syshclk * (i - 1);
                    if (pllclk < FreqTab->calhclk)
                    {
                        pllclk = FreqTab->calhclk;
                    }
                    calhclk_div = pllclk / FreqTab->calhclk;
                    pllclk      = FreqTab->calhclk * calhclk_div;
                    syshclk_div = pllclk / FreqTab->syshclk;
                    if (pllclk / syshclk_div > 300000000)
                    {
                        syshclk_div++;
                    }
                    break;
                }
                if ((pllclk % FreqTab->calhclk) == 0)
                {
                    syshclk_div = i;
                    calhclk_div = pllclk / FreqTab->calhclk;
                    if (pllclk / syshclk_div > 300000000)
                    {
                        syshclk_div++;
                    }
                    if (syshclk_div > 8)
                    {
                        syshclk_div = 8;
                    }
                    break;
                }
            }
            if (i >= 9)
            {
                pllclk      = FreqTab->syshclk * 8;
                if (pllclk < FreqTab->calhclk)
                {
                    pllclk = FreqTab->calhclk;
                }
                calhclk_div = pllclk / FreqTab->calhclk;
                pllclk      = FreqTab->calhclk * calhclk_div;
                syshclk_div = pllclk / FreqTab->syshclk;
                if (pllclk / syshclk_div > 300000000)
                {
                    syshclk_div++;
                }
                if (syshclk_div > 8)
                {
                    syshclk_div = 8;
                }
            }
        }
    }

    //if(pllclk != pChipFreq->pll)
    {
        pChipFreq->pll = pllclk;
        pChipFreq->armclk = pllclk;
        pChipFreq->armclk2 = pllclk;

        if (pChipFreq->pll > 24000000)
        {
            Get_PLL_Post_Div(pChipFreq->pll, &PllArg);
            Get_PLL_Pre_Div(PllArg.VCO, &PllArg);
        }
        else
        {
            memset(&PllArg, 0, sizeof(PLL_ARG_t));
            PllArg.div_con_24m = osc_24m_div;
        }

        //sys core hclk/fclk
        PllArg.sys_core_div = syshclk_div;
        pChipFreq->hclk_sys_core = pChipFreq->pll / syshclk_div;
        pChipFreq->fclk_sys_core  = pChipFreq->hclk_sys_core;
        //sys core stclk
        PllArg.sys_stclk_div = pChipFreq->hclk_sys_core / FreqTab->sysstclk;
        if (PllArg.sys_stclk_div == 0) PllArg.sys_stclk_div = 1;
        pChipFreq->stclk_sys_core = pChipFreq->hclk_sys_core / PllArg.sys_stclk_div;
        //sys core pclk
        for (i = 0; i < 4; i++)
        {
            if ((FreqTab->syspclk * (0x0001 << i)) >= pChipFreq->hclk_sys_core)
            {
                PllArg.pclk_logic_div = i;
                //DEBUG("PllArg.pclk_logic_div = %d", PllArg.pclk_logic_div);
                pChipFreq->pclk_logic_pre = pChipFreq->hclk_sys_core / (0x0001 << i);
                break;
            }
        }

        //cal core hclk/fclk
        PllArg.cal_core_div = calhclk_div;
        pChipFreq->hclk_cal_core = pChipFreq->pll / calhclk_div;
        pChipFreq->fclk_cal_core = pChipFreq->hclk_cal_core;
        //cal core stclk
        PllArg.cal_stclk_div = pChipFreq->hclk_cal_core / FreqTab->calstclk;
        if (PllArg.cal_stclk_div == 0) PllArg.cal_stclk_div = 1;
        pChipFreq->stclk_cal_core = pChipFreq->hclk_cal_core / PllArg.cal_stclk_div;

        FreqSetPrevProcess();
        SetPllFreq(pllclk, &PllArg);
        FreqSetPostProcess();

        SetHclkLogicPreFreq();
        SetPclkPmuPreFreq();
        SetHclkPmuPreFreq();


        SysTickPeriodSet(10);
    }
    #if 0
    else
    {
        if(FreqTab->syshclk != pChipFreq->hclk_sys_core)
        {
            pChipFreq->hclk_sys_core  = SetHclkSysCoreFreq(PLL_MUX_CLK, FreqTab->syshclk);
            pChipFreq->fclk_sys_core  = pChipFreq->hclk_sys_core;
            pChipFreq->stclk_sys_core = SetStclkSysCoreFreq(FreqTab->sysstclk);
            pChipFreq->pclk_logic_pre = SetPclkLogicPreFreq(FreqTab->syspclk);
            SysTickPeriodSet(10);
        }

        if(FreqTab->sysstclk != pChipFreq->stclk_sys_core)
        {
            pChipFreq->stclk_sys_core = SetStclkSysCoreFreq(FreqTab->sysstclk);
        }

        if(FreqTab->syspclk != pChipFreq->pclk_logic_pre)
        {
            pChipFreq->pclk_logic_pre = SetPclkLogicPreFreq(FreqTab->syspclk);
        }

        if(FreqTab->calhclk != pChipFreq->hclk_cal_core)
        {
            pChipFreq->hclk_cal_core  = SetHclkCalCoreFreq(PLL_MUX_CLK, FreqTab->calhclk);
            pChipFreq->fclk_cal_core  = pChipFreq->hclk_cal_core;
            pChipFreq->stclk_cal_core = SetStclkCalCoreFreq(FreqTab->calstclk);
        }

        if(FreqTab->calstclk != pChipFreq->stclk_cal_core)
        {
            pChipFreq->stclk_cal_core = SetStclkCalCoreFreq(FreqTab->calstclk);
        }
        SetHclkLogicPreFreq();
        SetPclkPmuPreFreq();
        SetHclkPmuPreFreq();
    }
    #endif

    #ifdef  _EMMC_
    SDC_UpdateClkHook(1);    //Notify emmc driver that pll has changed
    #endif
    #ifdef  _SDCARD_
    SDC_UpdateClkHook(0);   //Notify sdio driver that pll has changed
    #endif

    //printf("\nVCO=%d, pll=%d, shclk=%d, stck=%d, splk=%d, hhclk=%d, htck=%d\n",
    //      PllArg.VCO, pChipFreq->pll, pChipFreq->hclk_sys_core, pChipFreq->stclk_sys_core, pChipFreq->pclk_logic_pre,
    //      pChipFreq->hclk_cal_core, pChipFreq->stclk_cal_core);

}

void Freq_VopTiming_Update(uint32 Freq)
{
    uint32 temp = Freq/8000000;
    uint32 csrw = temp / 2;
    uint32 rwpw = temp / 1;
    uint32 rwcs = temp / 2;

    if (csrw > 63) csrw = 15;
    if (rwpw == 0) rwpw = 1;
    if (rwpw > 63) rwpw = 63;
    if (rwcs == 0) rwcs = 1;
    if (rwcs > 31) rwcs = 31;

    VopSetTiming(0, csrw, rwpw, rwcs );     //LCD timing
}

/*
--------------------------------------------------------------------------------
  Function name : int32 PMUSetCodecFreq(uint32 nKHz)
  Author        :
  Description   : set CODEC frequency

  Input         : freq:codec frequency
  Return        :

  History:     <author>         <time>         <version>
                                2009-3-24         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/


int32 FREQSetCalCoreFreq(uint32 nMhz)
{

}

/*
--------------------------------------------------------------------------------
  Function name : int32 PMUSetFreq(uint64 appList)
  Author        :
  Description   : set each PLL output frequency and bus frequency.

  Input         : appList:application list.
  Return        :

  History:     <author>         <time>         <version>
                                2009-3-24         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/


int32 FREQSetFreq(uint64 appList)
{
    uint32 i;
    uint32 FreqIdleDisable = 0;
    FREQ_APP_TABLE FreqAppTabl;

    memset(&FreqAppTabl, 0,sizeof(FREQ_APP_TABLE));

    if(appList & (((uint64)0x01) << FREQ_IDLE))
    {
        appList = ((uint64)0x01) << FREQ_IDLE;
        FreqIdleDisable = 1;

    }
    else if(appList == 0)
    {
        appList = ((uint64)0x01) << FREQ_MIN;
        FreqIdleDisable = 0;
    }

    for(i = 0; i < FREQ_APP_MAX; i++)
    {
        if(appList & ((uint64)0x01 << i))
        {
            FreqAppTabl.PllFreq  += g_CruAPPTabel[i].PllFreq;

            FreqAppTabl.syshclk  += g_CruAPPTabel[i].syshclk;
            FreqAppTabl.sysstclk += g_CruAPPTabel[i].sysstclk;
            FreqAppTabl.syspclk  += g_CruAPPTabel[i].syspclk;

            FreqAppTabl.calhclk  += g_CruAPPTabel[i].calhclk;
            FreqAppTabl.calstclk += g_CruAPPTabel[i].calstclk;
        }
    }

    //pll min:mix
    if((FreqAppTabl.PllFreq < g_CruAPPTabel[FREQ_MIN].PllFreq) && (FreqIdleDisable == 0))
    {
        FreqAppTabl.PllFreq = g_CruAPPTabel[FREQ_MIN].PllFreq;
    }
    if(FreqAppTabl.PllFreq > g_CruAPPTabel[FREQ_MAX].PllFreq)
    {
        FreqAppTabl.PllFreq = g_CruAPPTabel[FREQ_MAX].PllFreq;
    }

    //syshclk min:mix
    if((FreqAppTabl.syshclk < g_CruAPPTabel[FREQ_MIN].syshclk) && (FreqIdleDisable == 0))
    {
        FreqAppTabl.syshclk = g_CruAPPTabel[FREQ_MIN].syshclk;
    }
    if(FreqAppTabl.syshclk > g_CruAPPTabel[FREQ_MAX].syshclk)
    {
        FreqAppTabl.syshclk = g_CruAPPTabel[FREQ_MAX].syshclk;
    }

    //sysstclk min:mix
    if((FreqAppTabl.sysstclk < g_CruAPPTabel[FREQ_MIN].sysstclk) && (FreqIdleDisable == 0))
    {
        FreqAppTabl.sysstclk = g_CruAPPTabel[FREQ_MIN].sysstclk;
    }
    if(FreqAppTabl.sysstclk > g_CruAPPTabel[FREQ_MAX].sysstclk)
    {
        FreqAppTabl.sysstclk = g_CruAPPTabel[FREQ_MAX].sysstclk;
    }

    //syspclk min:mix
    if((FreqAppTabl.syspclk < g_CruAPPTabel[FREQ_MIN].syspclk) && (FreqIdleDisable == 0))
    {
        FreqAppTabl.syspclk = g_CruAPPTabel[FREQ_MIN].syspclk;
    }
    if(FreqAppTabl.syspclk > g_CruAPPTabel[FREQ_MAX].syspclk)
    {
        FreqAppTabl.syspclk = g_CruAPPTabel[FREQ_MAX].syspclk;
    }

    //calhclk min:mix
    if((FreqAppTabl.calhclk < g_CruAPPTabel[FREQ_MIN].calhclk) && (FreqIdleDisable == 0))
    {
        FreqAppTabl.calhclk = g_CruAPPTabel[FREQ_MIN].calhclk;
    }
    if(FreqAppTabl.calhclk > g_CruAPPTabel[FREQ_MAX].calhclk)
    {
        FreqAppTabl.calhclk = g_CruAPPTabel[FREQ_MAX].calhclk;
    }

    //pll min:mix
    if((FreqAppTabl.calstclk < g_CruAPPTabel[FREQ_MIN].calstclk) && (FreqIdleDisable == 0))
    {
        FreqAppTabl.calstclk = g_CruAPPTabel[FREQ_MIN].calstclk;
    }
    if(FreqAppTabl.calstclk > g_CruAPPTabel[FREQ_MAX].calstclk)
    {
        FreqAppTabl.calstclk = g_CruAPPTabel[FREQ_MAX].calstclk;
    }

#if (FRAME_SUB_BUFFER_NUM > 1)
    //calhclk min:mix
    if(FreqAppTabl.calhclk < FreqAppTabl.syshclk)
    {
        FreqAppTabl.calhclk = FreqAppTabl.syshclk;
    }

    //pll min:mix
    if(FreqAppTabl.calstclk < FreqAppTabl.sysstclk)
    {
        FreqAppTabl.calstclk = FreqAppTabl.sysstclk;
    }
#endif

    FREQSetARMFreq(&FreqAppTabl, &chip_freq);

    return(0);
}

/*
--------------------------------------------------------------------------------
  Function name : int32 PMUStartAPP(ePMU_APP appId)
  Author        :
  Description   : start one application

  Input         : appId: application serial number.
  Return        :

  History:     <author>         <time>         <version>
                                2009-3-24         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/


int32 FREQStartAPP(eFREQ_APP appId)
{
    if(g_APPList & ((uint64)0x01<< appId))//this application had enter.
    {
        return(0);
    }
    else
    {
        g_APPList |= ((uint64)0x01<< appId);
        FREQSetFreq(g_APPList);
        return(0);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : PMUStopAPP
  Author        :
  Description   : stop one application

  Input         : appId:application serial number.
  Return        :

  History:     <author>         <time>         <version>
                                2009-3-24         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/


int32 FREQStopAPP(eFREQ_APP appId)
{
    if(appId >= FREQ_APP_MAX)
    {
        return (-1);
    }
    if(g_APPList & ((uint64)0x01<< appId))
    {
        g_APPList &= ~((uint64)0x01<< appId);
        FREQSetFreq(g_APPList);
    }
    else//this application had exit.
    {
        return(-1);
    }
    return(0);
}

/*
--------------------------------------------------------------------------------
  Function name : int32 PMU_EnterModule(ePMU_APP modulename)
  Author        :
  Description   : set pll frequency.(arm freq)

  Input         :  nMHz  PLL 's frequency

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


int32 FREQ_EnterModule(eFREQ_APP modulename)
{
    if (FreqDisableFlag == 0)
    {
        FREQStartAPP(modulename);
    }
    else
    {
         g_APPDisableList &= ~((uint64)0x01<< modulename);
    }

    return(0);
}

/*
--------------------------------------------------------------------------------
  Function name : int32 PMU_ExitModule(ePMU_APP modulename)
  Author        :
  Description   :

  Input         :  nMHz  PLL 's frequency

  Return        : null

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


int32 FREQ_ExitModule(eFREQ_APP modulename)
{
    if (FreqDisableFlag == 0)
    {
        FREQStopAPP(modulename);
    }
    else
    {
         g_APPDisableList |= ((uint64)0x01<< modulename);
    }

    return 0;
}

/*
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
*/


int32 FREQ_Control_Init(void)
{
    FreqDisableFlag = 0;
    FreqDisableCnt = 0;
}




int32 FREQ_Disable(void)
{
    FreqDisableFlag = 1;
    FreqDisableCnt++;
}

/*
--------------------------------------------------------------------------------
  Function name : int32 PMU_ExitModule(ePMU_APP modulename)
  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/


int32 FREQ_Enable(void)
{
    uint32 i;

    if(FreqDisableCnt >= 1)
    {
        FreqDisableCnt--;
    }

    if(FreqDisableCnt == 0)
    {
        FreqDisableFlag = 0;
        for (i = 0; i < 64; i++)
        {
            if(g_APPDisableList & ((uint64)0x01<< i))
            {
                g_APPDisableList &= ~((uint64)0x01 << i);
                FREQ_ExitModule(i);
            }
        }
    }
}


/*
********************************************************************************
*
*                         End of Pll.c
*
********************************************************************************
*/

