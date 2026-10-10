/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: mmc.c
Desc:

Author: chenfen
Date: 12-01-10
Notes:

$Log: $
 *
 *
*/

/*-------------------------------- Includes ----------------------------------*/

#include "SDConfig.h"
#include "PowerManager.h"

#if (CONFIG_EMMC_SPEC ==1)

/*------------------------------------ Defines -------------------------------*/


/*----------------------------------- Typedefs -------------------------------*/


/*-------------------------- Forward Declarations ----------------------------*/


/* ------------------------------- Globals ---------------------------------- */


/*-------------------------------- Local Statics: ----------------------------*/
/*
uint8 EMMC_MIDTbl[]=
{
    0x15,					    //����SAMSUNG
    0x11,					    //��֥TOSHIBA
    0x90,					    //����ʿHYNIX
    0xff,					    //Ӣ����INFINEON
    0x13,					    //����MICRON
    0xff,					    //����RENESAS
    0xff,					    //�ⷨ�뵼��ST
    0xff,					    //Ӣ�ض�intel
    0x45,                       //SanDisk
    0x70,                       //Kingston
    0xfe                        //���� Numonyx
};
*/
_ATTR_SYS_BSS_     uint16 eMMC_ID;

/*--------------------------- Local Function Prototypes ----------------------*/
_ATTR_SD_INIT_BSS_ uint16 MMC_RCA;

/*------------------------ Function Implement --------------------------------*/


/*
Name:       _GenerateRCA
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
uint16 _MMCGenerateRCA(void)
{
    //static uint16 max = 2;   //rca = 0001��MMC�ϵ���ʼ��ʱʹ�õ�Ĭ�ϵ�ַ�����Դ�2��ʼ

    return (MMC_RCA++);
}

/*
Name:       _DecodeCSD
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
static void _MMCDecodeCSD(uint32 *pCSD, pSDM_CARD_INFO pCard)
{
    uint32           c_size = 0;
    uint32           c_size_mult = 0;
    uint32           read_bl_len = 0;
    uint32           transfer_rate_unit[4] = {10, 100, 1000, 10000};
    uint32           time_value[16] = {10/*reserved*/, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
    uint8            erase_grp_size;
    uint8            erase_grp_mult;

    c_size      = (pCSD[1] >> 30) | ((pCSD[2] & 0x3FF) << 2);    //[73:62]
    c_size_mult = (pCSD[1] >> 15) & 0x7;                         //[49:47]
    read_bl_len = (pCSD[2] >> 16) & 0xF;                         //[83:80]

    erase_grp_size = (pCSD[1] >> 10) & 0x1F;                     //[46:42]
    erase_grp_mult = (pCSD[1] >> 5) & 0x1F;                     //[41:37]

    pCard->EraseSize =  (erase_grp_size + 1) * (erase_grp_mult + 1);

    pCard->SpecVer    = (CARD_SPEC_VER_E)(MMC_SPEC_VER_10 + ((pCSD[3] >> 26) & 0xF)); //[125:122]
    pCard->capability = (((c_size + 1)*(0x1 << (c_size_mult + 2))*(0x1 << read_bl_len)) >> 9);
    pCard->TranSpeed = transfer_rate_unit[pCSD[3] & 0x3]*time_value[(pCSD[3] >> 3) & 0x7]; //[103:96]
    return;
}

#ifdef IN_LOADER
/*
Name:       _MMCSetBootSize
Desc:        SAMSUMG ��һЩEMMC Ĭ�ϵ�BootSizeΪ0, �μ�SAMSUMG DATASHEET
Param:       boot_size ��λ��128K
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
static int32 _MMCSetBootSize(pSDM_CARD_INFO         pCard, uint32 boot_size)
{
    int32            ret = SDM_FALSE;
    uint32           status = 0;


    ret = SDC_SendCmd(pCard->SDCPort, (SD_CMD62 | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), 0xEFAC62EC, &status);
    if (SDC_SUCCESS != ret)
    {
        return ret;
    }

	ret = SDC_SendCmd(pCard->SDCPort, (SD_CMD62 | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), 0x00CBAEA7, &status);
    if (SDC_SUCCESS != ret)
    {
        return ret;
    }

    ret = SDC_SendCmd(pCard->SDCPort, (SD_CMD62 | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), boot_size, &status);
    if (SDC_SUCCESS != ret)
    {
        return ret;
    }

	return SDC_WaitBusy(pCard->SDCPort);

}

#endif

/****************************************************************/
//������:MMC_SwitchBoot
//����:�л�boot partition����user area
//����˵��:pCard �������  ����Ϣ��ָ��
//         enable     �������  ʹ��
//         partition  �������  ��������ĸ�boot partition
//����ֵ:
//���ȫ�ֱ���:
//ע��:
/****************************************************************/
_ATTR_SD_INIT_CODE_
int32 MMC_PartitionConfig(pSDM_CARD_INFO         pCard, uint8 value)
{
    uint32           status = 0;
    int32            ret = SDM_SUCCESS;

    ret = SDC_SendCmd(pCard->SDCPort, \
                         (MMC4_SWITCH_FUNC | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), \
                         ((0x3 << 24) | (179 << 16) | (value << 8)), \
                         &status);
    if ((SDC_SUCCESS == ret) && ((status & (0x1 << 7)) != 0x0))
    {
        ret = SDM_FALSE;
    }
    return ret;
}


/*
Name:       _MMC_SwitchFunction
Desc:       ��ȡEXT_CSD�Ĵ��������ݿ��Ƿ�֧�ֿ������ߣ��ı������ߵĿ���
            �Լ����ݿ��Ƿ�֧�ָ���ģʽ���л�������ģʽ
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
static void _MMCSwitchFunction(pSDM_CARD_INFO pCard)
{
    HOST_BUS_WIDTH   wide = BUS_WIDTH_INVALID;
    uint32           value = 0;
    uint32           status = 0;
    int32            ret = SDM_SUCCESS;
    uint32           DataBuf[512/4];
    uint8           *pBuf = NULL;

    if(pCard->SpecVer < MMC_SPEC_VER_40)
    {
        return;
    }

    pBuf = (uint8*)DataBuf;
    do
    {
        ret = SDC_Request(pCard->SDCPort,
                        (MMC4_SEND_EXT_CSD | SD_READ_OP | SD_RSP_R1 | WAIT_PREV),
                        0,
                        &status,
                        512,
                        512,
                        DataBuf);
        if (SDC_SUCCESS != ret)
        {
            break;
        }

        pBuf = (uint8 *)DataBuf;

        pCard->BootSize = pBuf[226]*256;  // *128K
        #ifdef IN_LOADER
        if (pCard->BootSize ==0)
        {
            _MMCSetBootSize(pCard, 1);
        }
        #endif
        value = ((pBuf[215] << 24) | (pBuf[214] << 16) | (pBuf[213] << 8) | pBuf[212]);//[215--212]  sector count
        if(value)
        {
            pCard->capability = value;
        }
        if (pBuf[196] & 0x3) //֧�ָ���ģʽ
        {
            ret = SDC_SendCmd(pCard->SDCPort, \
                             (MMC4_SWITCH_FUNC | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), \
                             ((0x3 << 24) | (185 << 16) | (0x1 << 8)), \
                             &status);
            if ((SDC_SUCCESS == ret) && ((status & (0x1 << 7)) == 0x0))
            {
                ret = SDC_SendCmd(pCard->SDCPort, (SD_SEND_STATUS | SD_NODATA_OP | SD_RSP_R1 | NO_WAIT_PREV), (pCard->rca << 16), &status);
                if ((SDC_SUCCESS == ret) && ((status & (0x1 << 7)) == 0x0))
                {
                    if (pBuf[196] & 0x2) // 52M
                    {
                        ret = SDC_UpdateFreq(pCard->SDCPort, MMCHS_52_FPP_FREQ);
                        if (SDC_SUCCESS == ret)
                        {
                            pCard->TranSpeed = MMCHS_52_FPP_FREQ;
                            pCard->WorkMode |= SDM_HIGH_SPEED_MODE;
                        }
                    }
                    else  // 26M
                    {
                        ret = SDC_UpdateFreq(pCard->SDCPort, MMCHS_26_FPP_FREQ);
                        if (SDC_SUCCESS == ret)
                        {
                            pCard->TranSpeed = MMCHS_26_FPP_FREQ;
                            pCard->WorkMode |= SDM_HIGH_SPEED_MODE;
                        }
                    }
                }
            }
        }

        //�л�����ģʽ�в��ɹ���ֱ��return���߿����л����Լ���
        //�л��߿����ڸ���ģʽ�л�֮��������������˳����һ���ڸ���ģʽ���ýϿ��������߻᲻�����
        ret = SDC_GetBusWidth(pCard->SDCPort, &wide);
        if((wide == BUS_WIDTH_INVALID) || (wide == BUS_WIDTH_MAX))
        {
            ret = SDC_SDC_ERROR;
            break;
        }
        if (wide == BUS_WIDTH_4_BIT)
        {
            value = 0x1;
            ret = SDC_SetBusWidth(pCard->SDCPort, wide);
            if (SDC_SUCCESS != ret)
            {
                break;
            }
            //�������������Ҫ��鷵��ֵ�Ƿ�ɹ�����Ϊ���ǵ�CRC���
            pBuf[0] = 0x5A;
            pBuf[1] = 0x5A;
            pBuf[2] = 0x5A;
            pBuf[3] = 0x5A;
            ret = SDC_Request(pCard->SDCPort, \
                             (MMC4_BUSTEST_W | SD_WRITE_OP | SD_RSP_R1 | WAIT_PREV), \
                             0, \
                             &status, \
                             4, \
                             4, \
                             pBuf);
            pBuf[0] = 0x00;
            pBuf[1] = 0x00;
            pBuf[2] = 0x00;
            pBuf[3] = 0x00;
            ret = SDC_Request(pCard->SDCPort, \
                                 (MMC4_BUSTEST_R | SD_READ_OP | SD_RSP_R1 | WAIT_PREV), \
                                 0, \
                                 &status, \
                                 4, \
                                 4, \
                                 pBuf);
            if (pBuf[0] != 0xA5)
            {
                break;
            }
        }
        else if (wide == BUS_WIDTH_8_BIT)
        {
            value = 0x2;
            ret = SDC_SetBusWidth(pCard->SDCPort, wide);
            if (SDC_SUCCESS != ret)
            {
                break;
            }
            //�������������Ҫ��鷵��ֵ�Ƿ�ɹ�����Ϊ���ǵ�CRC���
            pBuf[0] = 0x55;
            pBuf[1] = 0xAA;
            pBuf[2] = 0x55;
            pBuf[3] = 0xAA;
            pBuf[4] = 0x55;
            pBuf[5] = 0xAA;
            pBuf[6] = 0x55;
            pBuf[7] = 0xAA;
            ret = SDC_Request(pCard->SDCPort, \
                                 (MMC4_BUSTEST_W | SD_WRITE_OP | SD_RSP_R1 | WAIT_PREV), \
                                 0, \
                                 &status, \
                                 8, \
                                 8, \
                                 pBuf);
            pBuf[0] = 0x00;
            pBuf[1] = 0x00;
            pBuf[2] = 0x00;
            pBuf[3] = 0x00;
            pBuf[4] = 0x00;
            pBuf[5] = 0x00;
            pBuf[6] = 0x00;
            pBuf[7] = 0x00;
            ret = SDC_Request(pCard->SDCPort, \
                                 (MMC4_BUSTEST_R | SD_READ_OP | SD_RSP_R1 | WAIT_PREV), \
                                 0, \
                                 &status, \
                                 4, \
                                 4, \
                                 pBuf);
            if ((pBuf[0] != 0xAA)
                 || (pBuf[1] != 0x55))
            {
                break;
            }
        }
        else
        {
            break;
        }

        ret = SDC_SendCmd(pCard->SDCPort, \
                             (MMC4_SWITCH_FUNC | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), \
                             ((0x3 << 24) | (183 << 16) | (value << 8)), \
                             &status);
        if ((SDC_SUCCESS != ret) || (status & (0x1 << 7)))
        {
            break;
        }
        ret = SDC_SendCmd(pCard->SDCPort, (SD_SEND_STATUS | SD_NODATA_OP | SD_RSP_R1 | NO_WAIT_PREV), (pCard->rca << 16), &status);
        if ((SDC_SUCCESS != ret) || (status & (0x1 << 7)))
        {
            break;
        }
        pCard->WorkMode |= SDM_WIDE_BUS_MODE;
    }while(0);

    return;
}

/*
Name:        _SDMMC_Read
Desc:
Param:
Return:
Global:
Note:
Author:
Log:
*/
_ATTR_SD_INIT_CODE_
int32 MMC_Init(pSDM_CARD_INFO    pCard)
{
    uint32           type = UNKNOW_CARD;
    uint32           LongResp[4];
    uint32           status = 0;
    uint16           rca = 0;
    uint32           i = 0;
    int32            ret = SDC_SUCCESS;

    MMC_RCA = 2;

    /**************************************************/
    // �ÿ�����Ready State
    /**************************************************/
    for (i=0; i<((FOD_FREQ*1000)/(48+2)); i++)
    {
        ret = SDC_SendCmd(pCard->SDCPort, (MMC_SEND_OP_COND | SD_NODATA_OP | SD_RSP_R3 | WAIT_PREV), 0x40ff8000, &status);
        if (SDC_SUCCESS == ret)
        {
            if (status & 0x80000000)
            {
                if ((0x80ff8000 == status) || (0x80ff8080 == status))
                {
                    type = MMC;
                    break;
                }
                else if((0xc0ff8000 == status) || (0xc0ff8080 == status))
                {
                    type = eMMC2G;
                    break;
                }
                else
                {
                    ret = SDM_UNKNOWABLECARD;
                    break;
                }
            }
        }
        else
        {
            /* error occured */
            ret = SDM_UNKNOWABLECARD;
            break;
        }
    }

    if (ret != SDC_SUCCESS)
    {
        return ret;
    }
    //��ʱ��busy
    if (((FOD_FREQ*1000)/(48+2)) == i)
    {
        return SDM_VOLTAGE_NOT_SUPPORT;
    }
    /**************************************************/
    // �ÿ�����Stand-by State
    /**************************************************/
    memset(LongResp, 0, sizeof(LongResp));
    ret = SDC_SendCmd(pCard->SDCPort, (SD_ALL_SEND_CID | SD_NODATA_OP | SD_RSP_R2 | WAIT_PREV), 0, LongResp);
    if (SDC_SUCCESS != ret)
    {
        return ret;
    }
    //decode CID
    //_MMC_DecodeCID(LongResp, pCard);
    eMMC_ID = (uint16)((LongResp[3] >> 24) & 0xFF);

    //generate a RCA
    rca = _MMCGenerateRCA();
    ret = SDC_SendCmd(pCard->SDCPort, (MMC_SET_RELATIVE_ADDR | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), (rca << 16), &status);
    if (SDC_SUCCESS != ret)
    {
        return ret;
    }
    pCard->rca = rca;

    memset(LongResp, 0, sizeof(LongResp));
    ret = SDC_SendCmd(pCard->SDCPort, (SD_SEND_CSD | SD_NODATA_OP | SD_RSP_R2 | WAIT_PREV), (rca << 16), LongResp);
    if (SDC_SUCCESS != ret)
    {
        return ret;
    }
    //decode CSD
    _MMCDecodeCSD(LongResp, pCard);

    pCard->TranSpeed = (pCard->TranSpeed > MMC_FPP_FREQ) ? MMC_FPP_FREQ : (pCard->TranSpeed);
    ret = SDC_UpdateFreq(pCard->SDCPort, pCard->TranSpeed);
    if (SDC_SUCCESS != ret)
    {
        return ret;
    }
    /**************************************************/
    // �ÿ�����Transfer State
    /**************************************************/
    ret = SDC_SendCmd(pCard->SDCPort, (SD_SELECT_DESELECT_CARD | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), (rca << 16), &status);
    if (SDC_SUCCESS != ret)
    {
        return ret;
    }

    /* Э��涨������SD1.X����SD2.0����SDHC������֧��block��СΪ512, ��������һ��Ҳֻ��512���������ֱ����Ϊ512 */
    ret = SDC_SendCmd(pCard->SDCPort, (SD_SET_BLOCKLEN | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), 512, &status);
    if (SDC_SUCCESS != ret)
    {
        return ret;
    }

    pCard->WriteProt = FALSE;  //MMC����û��д����
    //�����뿪��������������
    if (status & CARD_IS_LOCKED)
    {
        pCard->bPassword = TRUE;
    }
    else
    {
        pCard->bPassword = FALSE;
        _MMCSwitchFunction(pCard);
    }

    pCard->type |= type;
    return SDC_SUCCESS ;
}
#endif
