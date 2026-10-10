/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SDConfig.h
Desc:

Author:
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

#ifndef _SDCONFIG_H
#define _SDCONFIG_H

/*-------------------------------- Includes ----------------------------------*/

#include "MDConfig.h"


/*------------------------------ Global Defines ------------------------------*/

#if defined(EMMC_DRIVER)
#define CONFIG_EMMC_SPEC        (1)     //����֧��EMMCЭ��
//#define CONFIG_SD_SPEC          (0)     //����֧��SDЭ��
#endif

#if defined(SDC0_DRIVER)
//#define CONFIG_EMMC_SPEC        (0)
#define CONFIG_SD_SPEC          (1)
#endif

#define CONFIG_EMMC_BUS_WIDTH        8
#define CONFIG_SD_BUS_WIDTH          4

#define MAX_SDC_NUM             2

#define MAX_SCU_CLK_DIV         32       //ʱ���ⲿ��Ƶ������Ƶֵ

//#define PRE_DEFINE_BLKCNT                 //Ԥ���峤�ȶ�д
#define ASYNC_WAIT_IO                       //�첽�ȴ�IO���

#define EN_SD_DMA             (0)//(1)      //�Ƿ���DMA���������ݴ��䣬1:DMA��ʽ��0:�жϷ�ʽ

#define EN_SD_INT             (1)      //�Ƿ�����жϷ�������ѯһЩSDMMC����������Ҫλ��1:���жϷ�ʽ��0:����ѯ��ʽ��Ŀǰ��������ѯ��ʽ������⻹������жϵ�
#define EN_SD_BUSY_INT        (0)      //ʹ��busy����жϣ����ֿ�����֧��
#define EN_SD_PRINTF          (0)      //�Ƿ�����SD�����ڲ�������Ϣ��ӡ��1:������ӡ��0:�رմ�ӡ
#define DEBOUNCE_TIME         (25)     //���β��������ʱ��,��λms


#define FOD_FREQ              (400)    //��ʶ��׶�ʹ�õ�Ƶ��,��λKHz,Э��涨���400KHz
//���������������Ƶ��ΪFREQ_HCLK_MAX/8
#define SD_FPP_FREQ           (24000)  //��׼SD����������Ƶ�ʣ���λKHz��Э��涨���25MHz
#define SDHC_FPP_FREQ         (48000)  //SDHC���ڸ���ģʽ�µĹ���Ƶ�ʣ���λKHz��Э��涨���50MHz
#define MMC_FPP_FREQ          (24000)  //��׼MMC����������Ƶ�ʣ���λKHz��Э��涨���20MHz
#define MMCHS_26_FPP_FREQ     (25000)  //����ģʽֻ֧�����26M��HS-MMC�����ڸ���ģʽ�µĹ���Ƶ�ʣ���λKHz��Э��涨���26MHz
#define MMCHS_52_FPP_FREQ     (50000)  //����ģʽ��֧�����52M��HS-MMC�����ڸ���ģʽ�µĹ���Ƶ�ʣ���λKHz��Э��涨���52MHz
#define MAX_CLKIN_FREQ        (50000)


#include "SDCommon.h"
#include "SDAdapt.h"

#include "SDM.h"
#include "SDCtrl.h"

#include "SD.h"
#include "MMC.h"
#include "eMMC.h"


/*------------------------------ Global Typedefs -----------------------------*/

/*----------------------------- External Variables ---------------------------*/

/*------------------------- Global Function Prototypes -----------------------*/

#endif
