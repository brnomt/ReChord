/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   hw_dma.c
*
* Description:从RK27移植过来
*
*
* History:      <author>          <time>        <version>
*               HuWeiGuo        2009-01-05         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_DMA_

#include "DriverInclude.h"

#include "interrupt.h"


DMA_LLP g_llpListn[DMA_CHN_MAX][LLP_MAX_NUM];
pFunc   DmaIsrCallBack[DMA_CHN_MAX]={0,0,0,0,0,0};
uint32 DmaTransSizeBack[DMA_CHN_MAX];

/*
--------------------------------------------------------------------------------
  Function name : DMAStart(UINT32 ch, UINT32 src, UINT32 dst, UINT32 size, UINT32 mode)
  Author        :
  Description   :

  Input         : [ch]       --  the channel number that DMA use for data transfer.
                  [src]      --  source address,WORD alignment
                  [dst]      --  destination，WORD alignment
                  [size]     --  transfer length.WORD(4 byte) is unit
                  [g_dmaPar] --  parameter.


  Return        : DMAState

  History:     <author>         <time>         <version>
               yangwenjie    2009-1-5        Ver1.0
  desc:         the unit of DMA transfer must be word.
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
DMAState DmaGetState(uint32 ch)
{
    uint32 tmp;

    tmp = (((uint32)(DmaReg->ChEnReg)) & ((uint32)(0x01) << ch));
    if (tmp != 0)
    {
        return DMA_BUSY;
    }

    return DMA_SUCCESS;
}

/*
--------------------------------------------------------------------------------
  Function name : DMAStart(UINT32 ch, UINT32 src, UINT32 dst, UINT32 size, UINT32 mode)
  Author        :
  Description   :

  Input         : [ch]       --  DMA传输时所用的通道
                  [src]      --  源地址，需要WORD对齐
                  [dst]      --  目的地址，需要WORD对齐
                  [size]     --  传输的长度，以WORD(4 byte)为单位
                  [g_dmaPar] --  配置参数表


  Return        : DMAState

  History:     <author>         <time>         <version>
               yangwenjie    2009-1-5        Ver1.0
  desc:         DMA传入的参数size必须是按word来计算
--------------------------------------------------------------------------------
*/
#if 0
_ATTR_DRIVERLIB_CODE_
int32 DmaGetChannel(void)
{
    uint32 ch;
    uint32 tmp;

    for (ch = 0; ch < DMA_CHN_MAX; ch++)
    {
        tmp = (((uint32)(DmaReg->ChEnReg)) & ((uint32)(0x01) << ch));
        if (tmp == 0)
        {
            return (int32)(ch);
        }
    }

    return DMA_FALSE;
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : DmaConfig(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size,
                       pDMA_CFGX g_dmaPar, pFunc CallBack, pFunc *pCallBackBuf,
                       pDMA_LLP pllplist)
  Author        :
  Description   : DmaStart call this function, main destination is used for saving to rom or flash memory

  Input         :

  Return        : DMAState

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
DMAState DmaConfig(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size,
                       pDMA_CFGX g_dmaPar, pFunc CallBack, pFunc *pCallBackBuf,
                       pDMA_LLP pllplist)
{
    uint32 i;
    int32  llpSrcOffset;
    int32  llpDstOffset;
    uint32 llpNum;
    uint32 remaind;

    //int status clear
    DmaReg->ClearTfr = (0x01u  << ch);

    //int enable
    if ((uint32)CallBack != NULL)
    {
        g_dmaPar->CTLL |= B_CTLL_INT_EN;
        DmaReg->MaskTfr  = (0x101ul << ch);
    }
    else
    {
        DmaReg->MaskTfr  = (0x100ul << ch);
    }

    pCallBackBuf[ch] = (pFunc)CallBack;

    //dma channel configer
    DmaReg->CHANNEL[ch].SAR   = srcAddr;
    DmaReg->CHANNEL[ch].DAR   = dstAddr;
    DmaReg->CHANNEL[ch].CFG_L = g_dmaPar->CFGL;
    DmaReg->CHANNEL[ch].CFG_H = g_dmaPar->CFGH;

    if (size > DMA_MAX_BLOCK_SIZE)
    {
        llpNum  = (size + (LLP_BLOCK_SIZE - 1))/LLP_BLOCK_SIZE;
        remaind = size - LLP_BLOCK_SIZE*(llpNum - 1);
        if (remaind == 0) remaind = LLP_BLOCK_SIZE;

        llpSrcOffset = 0x0;
        if((g_dmaPar->CTLL & B_CTLL_SINC_MASK) == B_CTLL_SINC_INC)  //incr
        {
            llpSrcOffset = LLP_BLOCK_SIZE << ((g_dmaPar->CTLL & B_CTLL_SRC_TR_WIDTH_MASK) >> 4);
        }
        else if((g_dmaPar->CTLL & B_CTLL_SINC_MASK) == B_CTLL_SINC_DEC)  //dec
        {
            llpSrcOffset = -(LLP_BLOCK_SIZE << ((g_dmaPar->CTLL & B_CTLL_SRC_TR_WIDTH_MASK) >> 4));
        }

        llpDstOffset = 0x0;
        if((g_dmaPar->CTLL & B_CTLL_DINC_MASK) == B_CTLL_DINC_INC)  //incr
        {
            llpDstOffset = LLP_BLOCK_SIZE << ((g_dmaPar->CTLL & B_CTLL_DST_TR_WIDTH_MASK) >> 1);
        }
        else if((g_dmaPar->CTLL & B_CTLL_DINC_MASK) == B_CTLL_DINC_DEC) //decr
        {
            llpDstOffset = -(LLP_BLOCK_SIZE << ((g_dmaPar->CTLL & B_CTLL_DST_TR_WIDTH_MASK) >> 1));
        }

        for (i = 0; i < llpNum ; i++)
        {
            pllplist[i].SAR = srcAddr + llpSrcOffset * i;
            pllplist[i].DAR = dstAddr + llpDstOffset * i;
            pllplist[i].LLP = (struct tagDMA_LLP *)((uint32)(&pllplist[i+1]));
            pllplist[i].CTLL = g_dmaPar->CTLL | (B_CTLL_LLP_SRC_EN | B_CTLL_LLP_DST_EN);
            pllplist[i].SIZE = LLP_BLOCK_SIZE;
        }

        pllplist[i-1].LLP  = 0;
        pllplist[i-1].SIZE = remaind;

        DmaReg->CHANNEL[ch].CTL_L = g_dmaPar->CTLL | (B_CTLL_LLP_SRC_EN | B_CTLL_LLP_DST_EN);
        DmaReg->CHANNEL[ch].LLP   = (uint64)(((uint32)(&pllplist[0])));
    }
    else
    {
        DmaReg->CHANNEL[ch].CTL_L = g_dmaPar->CTLL;
        DmaReg->CHANNEL[ch].SIZE  = size;
        DmaReg->CHANNEL[ch].LLP   = 0;
    }

    //DMA Transmit Enable
    DmaReg->DmaCfgReg = 1;
    DmaReg->ChEnReg  |= (0x101 << ch);       //DMA Channel Enable

    return(DMA_SUCCESS);
}

/*
--------------------------------------------------------------------------------
  Function name : rk_err_t DmaDisableInt(uint32 ch)
  Author        :
  Description   :

  Input         :

  Return        : DMAState

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
rk_err_t DmaDisableInt(uint32 ch)
{
    DmaReg->ClearTfr = (0x01u  << ch);
    DmaReg->MaskTfr  = (0x0100ul << ch);
    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 DmaGetIntType(void)
  Author        :
  Description   :

  Input         :

  Return        : DMAState

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
uint32 DmaGetIntType(void)
{
    uint32 rawStatus;
    rawStatus = (uint32)(DmaReg->StatusTfr);
    DmaReg->ClearTfr = rawStatus;
    return rawStatus;
}

/*
--------------------------------------------------------------------------------
  Function name :rk_err_t DmaEnableInt(uint32 ch)
  Author        :
  Description   :

  Input         :

  Return        : DMAState

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVERLIB_CODE_
rk_err_t DmaEnableInt(uint32 ch)
{
    DmaReg->ClearTfr = (0x01u  << ch);
    DmaReg->MaskTfr  = (0x0101ul << ch);
    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : DmaConfig(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size,
                       pDMA_CFGX g_dmaPar, pFunc CallBack, pFunc *pCallBackBuf,
                       pDMA_LLP pllplist)
  Author        :
  Description   : DmaStart call this function, main destination is used for saving to rom or flash memory

  Input         :

  Return        : DMAState

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
rk_err_t DmaConfig_for_LLP(uint32 ch,uint32 size ,uint32 block_size,pDMA_CFGX g_dmaPar, pDMA_LLP pllplist)
{
    uint32 i;
    int32  llpSrcOffset;
    int32  llpDstOffset;
    uint32 llpNum;
    uint32 remaind;

    DmaReg->CHANNEL[ch].CFG_L = g_dmaPar->CFGL;
    DmaReg->CHANNEL[ch].CFG_H = g_dmaPar->CFGH;
    g_dmaPar->CTLL |= B_CTLL_INT_EN;

    if (size < LLP_BLOCK_SIZE)
    {
        if(pllplist)
        {
            for (i = 0; i < block_size ; i++)
            {
                pllplist[i].LLP = (struct tagDMA_LLP *)((uint32)(&pllplist[i+1]));
                pllplist[i].CTLL = g_dmaPar->CTLL | (B_CTLL_LLP_SRC_EN | B_CTLL_LLP_DST_EN);
            }

            pllplist[i-1].LLP  = NULL;

            DmaReg->CHANNEL[ch].CTL_L = g_dmaPar->CTLL | (B_CTLL_LLP_SRC_EN | B_CTLL_LLP_DST_EN);
            DmaReg->CHANNEL[ch].LLP   = (uint64)(((uint32)(&pllplist[0])));
        }
        else
        {
            DmaReg->CHANNEL[ch].CTL_L = g_dmaPar->CTLL;
            DmaReg->CHANNEL[ch].SIZE  = size;
            DmaReg->CHANNEL[ch].LLP   = 0;
        }
    }

    //DMA Transmit Enable
    DmaReg->DmaCfgReg = 1;
    DmaReg->ChEnReg  |= (0x101 << ch);       //DMA Channel Enable

    return(RK_SUCCESS);

}


/*
--------------------------------------------------------------------------------
  Function name : DMAStart(UINT32 ch, UINT32 src, UINT32 dst, UINT32 size, UINT32 mode)
  Author        :
  Description   :

  Input         : [ch]       --  DMA传输时所用的通道
                  [src]      --  源地址，需要WORD对齐
                  [dst]      --  目的地址，需要WORD对齐
                  [size]     --  传输的长度，以传输宽度为单位(1,2,4 byte);
                  [g_dmaPar] --  配置参数表
                                 如果不支持LLP模式，g_dmaPar->pLLP为0，同时CTLL禁止llp功能，参数size为整个传输长度
                                 中断使能控制，有外部参数g_dmaPar->CTLL控制

  Return        : DMAState

  History:     <author>         <time>         <version>
  desc:         对于无法确定传输大小的,可以使用该函数
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
int32 DmaGetChannel(void)
{
    uint32 ch;
    uint32 tmp;
    uint8  ch_max = DMA_CHN_MAX - 1;

    for (ch = 0; ch < ch_max; ch++)
    {
        tmp = (((uint32)(DmaReg->ChEnReg)) & ((uint32)(0x01) << ch));
        if (tmp == 0)
        {
            return (int32)(ch);
        }
    }

    return DMA_FALSE;
}

/*
--------------------------------------------------------------------------------
  Function name : DMAStart(UINT32 ch, UINT32 src, UINT32 dst, UINT32 size, UINT32 mode)
  Author        :
  Description   :

  Input         : [ch]       --  the channel number that DMA use for data transfer.
                  [src]      --  source address,WORD alignment
                  [dst]      --  destination，WORD alignment
                  [size]     --  transfer length.WORD(4 byte) is unit
                  [g_dmaPar] --  parameter.


  Return        : DMAState

  History:     <author>         <time>         <version>
               yangwenjie    2009-1-5        Ver1.0
  desc:         the unit of DMA transfer must be word.
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
DMAState DmaStart(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pFunc CallBack)
{
    int32    ret;
    uint32   llpNum;
    uint32   temp_ch;
    pFunc   *pCallBackBuf = DmaIsrCallBack;
    DMAState retval = DMA_SUCCESS;

    //IntMasterDisable();
    DisableIntMaster();

    temp_ch = ch;
    if (temp_ch >= DMA_CHN_MAX)
    {
        printf("Dma Channel error: ch = %d!\n", ch);
        retval = DMA_FALSE;
        goto out;
    }

    if ((DmaReg->ChEnReg) & (0x01 << temp_ch))
    {
        ret = DmaGetChannel();
        if (ret == DMA_FALSE)
        {
            temp_ch = (uint32)ret;
            printf("Dma Channel Busy: ch = %d!\n", temp_ch);
            retval = DMA_FALSE;
            goto out;
        }
        temp_ch = (uint32)ret;
    }

    if(size <= 0)
    {
        printf("###ERROR DMA size == 0\n");
    }

    if (size > DMA_MAX_BLOCK_SIZE)
    {
        llpNum  = (size + (LLP_BLOCK_SIZE - 1))/LLP_BLOCK_SIZE;

        if (llpNum > LLP_MAX_NUM)
        {
            printf("Dma Ch%d size so large!\n", temp_ch);
            retval = DMA_FALSE;
            goto out;
        }
    }
    retval = temp_ch;

    DmaTransSizeBack[temp_ch] = size;
    DmaConfig(temp_ch, srcAddr, dstAddr, size, g_dmaPar, CallBack, DmaIsrCallBack, g_llpListn[temp_ch]);

out:

    //IntMasterEnable();
    EnableIntMaster();

    return(retval);
}

/*
--------------------------------------------------------------------------------
  Function name : DMAStart(UINT32 ch, UINT32 src, UINT32 dst, UINT32 size, UINT32 mode)
  Author        :
  Description   :

  Input         : [ch]       --  DMA传输时所用的通道
                  [src]      --  源地址，需要WORD对齐
                  [dst]      --  目的地址，需要WORD对齐
                  [size]     --  传输的长度，以传输宽度为单位(1,2,4 byte);
                  [g_dmaPar] --  配置参数表
                                 如果不支持LLP模式，g_dmaPar->pLLP为0，同时CTLL禁止llp功能，参数size为整个传输长度
                                 中断使能控制，有外部参数g_dmaPar->CTLL控制

  Return        : DMAState

  History:     <author>         <time>         <version>
  desc:         对于无法确定传输大小的,可以使用该函数
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
//DMAState DmaReStart(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pFunc CallBack)
DMAState DmaReStart(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pFunc CallBack)
{
    //dma channel configer

    //IntMasterDisable();
    DisableIntMaster();

    if (DmaTransSizeBack[ch] == size)
    {

        DmaIsrCallBack[ch] = (pFunc)CallBack;

        DmaReg->CHANNEL[ch].SAR   = srcAddr;
        DmaReg->CHANNEL[ch].DAR   = dstAddr;

        if (size > DMA_MAX_BLOCK_SIZE)
        {
            DmaReg->CHANNEL[ch].LLP = (uint64)(((uint32)(g_llpListn[ch])));
        }
        else
        {
            DmaReg->CHANNEL[ch].LLP = 0;
        }

        DmaReg->ChEnReg  |= (0x101 << ch);
    }
    else
    {
        DmaConfig(ch, srcAddr, dstAddr, size, g_dmaPar, CallBack, DmaIsrCallBack, g_llpListn[ch]);
    }

    //IntMasterEnable();
    EnableIntMaster();

}

/*
--------------------------------------------------------------------------------
  Function name : void DmaSuspend(uint32 ch)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void DmaSuspend(uint32 ch)
{
  uint32 delaycnt = 800;

  if (DMA_BUSY == DmaGetState(ch))
    {
		DmaReg->CHANNEL[ch].CFG_L |= B_CFGL_CH_SUSP;

		while(delaycnt)
		{
		    delaycnt--;
		    if ((DmaReg->CHANNEL[ch].CFG_L & B_CFGL_FIFO_EMPTY))
		    {
		        break;
		    }
		}
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void DmaResume(uint32 ch)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void DmaResume(uint32 ch)
{
    if (DMA_BUSY == DmaGetState(ch))
    {
        DmaReg->CHANNEL[ch].CFG_L &= ~B_CFGL_CH_SUSP;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : DMAStart(UINT32 ch, UINT32 src, UINT32 dst, UINT32 size, UINT32 mode)
  Author        :
  Description   :

  Input         : [ch]       --  DMA传输时所用的通道
                  [src]      --  源地址，需要WORD对齐
                  [dst]      --  目的地址，需要WORD对齐
                  [size]     --  传输的长度，以WORD(4 byte)为单位
                  [g_dmaPar] --  配置参数表


  Return        : DMAState

  History:     <author>         <time>         <version>
               yangwenjie    2009-1-5        Ver1.0
  desc:         DMA传入的参数size必须是按word来计算
--------------------------------------------------------------------------------
*/
_ATTR_DRIVER_CODE_
void DmaInt(void)
{
    uint32 channel;
    uint32 rawStatus;
    pFunc CallBack;

    do
    {
        rawStatus = (uint32)(DmaReg->StatusTfr);
        DmaReg->ClearTfr = rawStatus;

        channel = 0;
        do
        {
            for (; channel < DMA_CHN_MAX; channel++)
            {
                if (((uint32)(rawStatus)) & ((uint32)(0x01) << channel))
                {
                    rawStatus &= ~(0x01u << channel);
                    break;
                }
            }


            if(channel < DMA_CHN_MAX)
            {
                CallBack = DmaIsrCallBack[channel];

                if (CallBack)
                {
                    DmaIsrCallBack[channel] = NULL;
                    CallBack();
                }
            }

        }while (rawStatus & 0x03f);

    }while((uint32)(DmaReg->StatusTfr) & 0x03f);
}

/*
********************************************************************************
*
*                         End of dma.c
*
********************************************************************************
*/

