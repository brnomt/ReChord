/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                                    (c) Copyright 2013, RockChip.Ltd
*                                          All Rights Reserved
*File    : NanoShell.C
*By      : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include "SysInclude.h"
#ifdef _USE_SHELL_
//#include "typedef_rkos.h"
#include "device.h"
#include "AudioControl.h"
#include "audio_main.h"
#include "mainmenu.h"
#include "MusicWinInterface.h"
#include "MediaBroWin.h"



/*
*********************************************************************************************************
*                                        Macro Define
*********************************************************************************************************
*/
#define EFFECT_TEST_NUM 0X00000FFF
#define EFFECT_TEST_BASE_NUM 3000

#define SHELL_RX_BUF_MAX_SIZE                128
#define SHELL_CMD_MAX_ITEM                   30
#define SHELL_CMD_MAX_SIZE                   10


typedef rk_err_t(* SHELL_PARASE_FUN)(HDC dev, uint8 *pStrBuff);

typedef struct _SHELL_CMD_ITEM
{
    uint32 useflag;
    uint8 * ShellCmdName;
    uint32 TaskID;
    SHELL_PARASE_FUN ShellCmdParaseFun;

}SHELL_CMD_ITEM;


typedef struct SHELL_TASK_DATA_BLOCK
{
    uint8* pData;
    uint8* pDataEnd;
    uint8* ShellRxBuffer;
    uint32 ShellRxStart;
    HDC hUart;
//    xTimerHandle IdleTime;
    uint32 IdleCnt;
    uint32 SysTick;
    uint16 * pbuf;
    uint32 temp;
    SHELL_CMD_ITEM * pCmdHeadItem;
    uint32 i;

}SHELL_TASK_DATA_BLOCK;

typedef struct _SHELL_CMD_INFO
{
    uint8 * ShellCmdName;
    uint32 TaskID;
    SHELL_PARASE_FUN ShellCmdParaseFun;

}SHELL_CMD_INFO;


/*
*********************************************************************************************************
*                                      Variable Define
*********************************************************************************************************
*/

const char* ShellRootName[] =
{
    "sd",
    "poweroff",
    "removeusb",
    "setting",
    "connect",
    "remove",
    "help",
    "\b"                          // the end
};


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
rk_err_t mailBoxTestCmdParsing(HDC dev, uint8 * pstr);
rk_err_t MemoryRW_Test(HDC dev, uint8 * pstr);
rk_err_t Time_test(HDC dev, uint8 * pstr);
rk_err_t WatchDogTestCmdParsing(HDC dev, uint8 * pstr);
rk_err_t EmmcTestCmdParsing(HDC dev, uint8 * pstr);
rk_err_t SdCardTestCmdParsing(HDC dev, uint8 * pstr);
rk_err_t Spi_test(HDC dev, uint8 * pstr);
rk_err_t UartTestCmdParse(HDC dev, uint8 * pstr);
rk_err_t PWMTestCmdParse(HDC dev, uint8 * pstr);
rk_err_t GPIOTestCmdParse(HDC dev, uint8 * pstr);
rk_err_t I2CTestCmdParse(HDC dev, uint8 * pstr);
rk_err_t ADCTestCmdParse(HDC dev, uint8 * pstr);
rk_err_t PVTMTestCmdParse(HDC dev, uint8 * pstr);
rk_err_t I2s_test(HDC dev, uint8 * pstr);
rk_err_t Acodec_test(HDC dev, uint8 * pstr);
rk_err_t SysTick_Test(HDC dev, uint8 * pstr);
rk_err_t VOP_test(HDC dev, uint8 * pstr);
rk_err_t MP3_test(HDC dev, uint8 * pstr);
rk_err_t hifi_test(HDC dev, uint8 * pstr);

const SHELL_CMD_INFO ShellRegisterName[] =
{
#ifdef _MAILBOX_TEST_
    "1", MODULE_TEST_MAILBOX, mailBoxTestCmdParsing,
#endif

#ifdef _M2M_TEST_
    "2", MODULE_TEST_MEMORY,  MemoryRW_Test,
#endif

#ifdef _TIME_TEST_
    "3", MODULE_TEST_TIMER,   Time_test,
#endif

#ifdef _WATCHDOG_TEST_
    "4", MODULE_TEST_WDT,     WatchDogTestCmdParsing,
#endif

#ifdef _EMMC_TEST_
    "5", MODULE_TEST_EMMC,    EmmcTestCmdParsing,
#endif

#ifdef _SD_TEST_
    "6", MODULE_TEST_SDMMC,   SdCardTestCmdParsing,
#endif

#ifdef _SPI_TEST_
    "7", MODULE_TEST_SPI,     Spi_test,
#endif

#ifdef _UART_TEST_
    "8", MODULE_TEST_UART,    UartTestCmdParse,
#endif

#ifdef _PWM_TEST_
    "9", MODULE_TEST_PWM,     PWMTestCmdParse,
#endif

#ifdef _GPIO_TEST_
    "a", MODULE_TEST_GPIO,    GPIOTestCmdParse,
#endif

#ifdef _I2C_TEST_
    "b", MODULE_TEST_I2C,     I2CTestCmdParse,
#endif

#ifdef _ADC_TEST_
    "c", MODULE_TEST_ADC,     ADCTestCmdParse,
#endif

#ifdef _PVTM_TEST_
    "d", MODULE_TEST_PVTM,    PVTMTestCmdParse,
#endif

#ifdef _I2S_TEST_
    "e", MODULE_TEST_I2S,     I2s_test,
#endif

#ifdef _ACODEC_TEST_
    "f", MODULE_TEST_ACODEC,  Acodec_test,
#endif

#ifdef _SYSTICK_TEST_
    "g", MODULE_TEST_SYSTICK, SysTick_Test,
#endif

#ifdef _MP3_TEST_
    "h", NULL, MP3_test,
#endif

#ifdef _VOP_TEST_
    "i", NULL, VOP_test,
#endif

#ifdef _HIFI_TEST_
    "j", NULL, hifi_test,
#endif

    "\b",NULL, NULL
};

static SHELL_TASK_DATA_BLOCK*   gpstShellTaskDataBlock;
static SHELL_TASK_DATA_BLOCK stShellTaskDataBlock;
static uint8 ShellRxBuffer[SHELL_RX_BUF_MAX_SIZE];
static uint8 CmdHeadItem[sizeof(SHELL_CMD_ITEM) * SHELL_CMD_MAX_ITEM];

static SHELL_PARASE_FUN currentParaseFun = NULL;

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void ShellPrintHelpInfo(void)
{
    rk_print_stringA("\r\n\r\n");

    rk_print_stringA("\r\n================================================================================");
    rk_print_stringA("\r\n Help Infomation                                                                ");
    rk_print_stringA("\r\n 1. mailbox test                                                                ");
    rk_print_stringA("\r\n 2. M2M test                                                                    ");
    rk_print_stringA("\r\n 3. Time test                                                                   ");
    rk_print_stringA("\r\n 4. watch dog test                                                              ");
    rk_print_stringA("\r\n 5. emmc test                                                                   ");
    rk_print_stringA("\r\n 6. sd card test                                                                ");
    rk_print_stringA("\r\n 7. Spi test                                                                    ");
    rk_print_stringA("\r\n 8. uart test                                                                   ");
    rk_print_stringA("\r\n 9. pwm test                                                                    ");
    rk_print_stringA("\r\n a. gpio test                                                                   ");
    rk_print_stringA("\r\n b. i2c test                                                                    ");
    rk_print_stringA("\r\n c. adc test                                                                    ");
    rk_print_stringA("\r\n d. pvtm test                                                                   ");
    rk_print_stringA("\r\n e. I2S test                                                                    ");
    rk_print_stringA("\r\n f. Acodec test                                                                 ");
    rk_print_stringA("\r\n g. SysTick Test                                                                ");
    rk_print_stringA("\r\n h. MP3_test                                                                    ");
    rk_print_stringA("\r\n i. VOP_test                                                                    ");
    rk_print_stringA("\r\n j. hifi_test                                                                   ");
    rk_print_stringA("\r\n================================================================================");

    rk_print_stringA("\r\n\r\n");
}

rk_err_t ShellCmdRemove(HDC dev, uint8 * pItemName)
{
    SHELL_CMD_ITEM * pCmdItem;

    uint32 i = 0, j = 0;

    if (gpstShellTaskDataBlock->pCmdHeadItem == NULL)
    {
        goto CmdRegErrExit;
    }
    else
    {
        pCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;
        for (j = 0; j < SHELL_CMD_MAX_ITEM; j++)
        {
            if (pCmdItem->useflag == 1)
            {
                if (StrCmpA((uint8*)pCmdItem->ShellCmdName, pItemName, 0) == 0)
                {
                    // already register
                    pCmdItem->useflag = 0;
                    rk_print_stringA("cmd remove success");
                    return RK_SUCCESS;
                }
            }
            pCmdItem++;
        }
    }

    rk_print_stringA("not find cmd");

CmdRegErrExit:

    return RK_SUCCESS;
}


rk_err_t ShellCmdRegister(HDC dev, uint8 * pItemName)
{
    SHELL_CMD_ITEM * pCmdItem;

    uint32 i = 0, j = 0;

    if (gpstShellTaskDataBlock->pCmdHeadItem == NULL)
    {
        goto CmdRegErrExit;
    }
    else
    {
        pCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;

        for (j = 0; j < SHELL_CMD_MAX_ITEM; j++)
        {
            if (pCmdItem->useflag == 1)
            {
                if (StrCmpA((uint8*)pCmdItem->ShellCmdName, pItemName, 0) == 0)
                {
                    // already register
                    rk_print_stringA("device already connected");
                    return RK_SUCCESS;
                }
            }
            pCmdItem++;
        }
    }

    while (StrCmpA((uint8*)ShellRegisterName[i].ShellCmdName,pItemName, 0) != 0)
    {
        i++;
        if (*((uint8*)ShellRegisterName[i].ShellCmdName) == '\b')
        {
            rk_print_stringA("device not exist");
            goto CmdRegErrExit;
        }
    }


    pCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;
    for (j = 0; j < SHELL_CMD_MAX_ITEM; j++)
    {
        if (pCmdItem->useflag == 0)
        {
            pCmdItem->ShellCmdParaseFun  = ShellRegisterName[i].ShellCmdParaseFun;
            pCmdItem->ShellCmdName = ShellRegisterName[i].ShellCmdName;
            pCmdItem->TaskID = ShellRegisterName[i].TaskID;
            pCmdItem->useflag = 1;
            rk_print_stringA("device connect success");
            return RK_SUCCESS;
        }
        pCmdItem++;
    }

    rk_print_stringA("cmd full");
CmdRegErrExit:

    return RK_SUCCESS;
}


rk_err_t ShellCmdRegisterALL(void)
{
    SHELL_CMD_ITEM * pCmdItem;
    int j,i;
    uint CmdCnt = sizeof(ShellRegisterName)/sizeof(SHELL_CMD_INFO) -1;

    pCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;

    if (CmdCnt > SHELL_CMD_MAX_ITEM)
    {
        rk_print_stringA("shell cmd can't register");
        return RK_ERROR;
    }
    i = 0;
    for (j = 0; j < CmdCnt; j++)
    {
        if (pCmdItem->useflag == 0)
        {
            pCmdItem->ShellCmdParaseFun  = ShellRegisterName[i].ShellCmdParaseFun;
            pCmdItem->ShellCmdName = ShellRegisterName[i].ShellCmdName;
            pCmdItem->TaskID = ShellRegisterName[i].TaskID;
            pCmdItem->useflag = 1;
            i++;

        }
        pCmdItem++;
    }

    rk_print_stringA("device connect success");
    return RK_SUCCESS;

}

/*
*********************************************************************************************************
*                                      voidShellTaskDeInit(void)
*
* Description: ��Ŀ¼Ԫ����ȡ
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/

rk_err_t ShellRootParsing(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    SHELL_CMD_ITEM * pTempCmdItem = NULL;

    StrCnt = ShellItemExtract(pstr,&pItem);

    if (StrCnt == 0)
    {
        rk_print_stringA("error cmd\r\nrkos://");
        return RK_ERROR;
    }

    pstr[StrCnt] = 0;

    ret = ShellCheckCmd(ShellRootName, pItem, StrCnt);
    if (ret < 0)
    {
        pTempCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;

        if (currentParaseFun)
        {
            ret = currentParaseFun(dev,pstr);
            if (ret == RK_ERROR)
            {
                rk_print_stringA("error cmd");
            }
            else if (ret == RK_EXIT)
            {
                ShellPrintHelpInfo();
                currentParaseFun = NULL;
            }

            return ret;
        }
        else
        {
            for (i = 0; i < SHELL_CMD_MAX_ITEM; i++)
            {
                if (pTempCmdItem->useflag == 1)
                {
                    if (StrCmpA(pTempCmdItem->ShellCmdName, pstr, 0) == 0)
                    {
                        if (pTempCmdItem->ShellCmdParaseFun != NULL)
                        {
                            pItem += StrCnt;
                            pItem++;

                            if (currentParaseFun == NULL)
                            {
                                currentParaseFun = pTempCmdItem->ShellCmdParaseFun;

                                if (pTempCmdItem->TaskID != NULL)
                                {
                                    ModuleOverlay(pTempCmdItem->TaskID, MODULE_OVERLAY_ALL);
                                }
                                ret = pTempCmdItem->ShellCmdParaseFun(dev,"I");

                                if (ret == RK_ERROR)
                                {
                                    rk_print_stringA("error cmd");
                                }
                            }
                            else
                            {
                                ret = pTempCmdItem->ShellCmdParaseFun(dev,pItem);

                                if (ret == RK_ERROR)
                                {
                                    rk_print_stringA("error cmd");
                                }
                            }

                            return ret;


                        }
                    }
                }
                pTempCmdItem++;
            }
        }


        rk_print_stringA("error cmd");
        return RK_ERROR;

    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    switch (i)
    {
        case 0x00:
            ShellPrintHelpInfo();
            break;

        case 0x01:

            break;

        case 0x02:

            break;

        case 0x03:

            break;

        case 0x04:
            break;

        case 0x05:

            break;

        case 0x06:

            break;

        case 0x07:

            break;

        default:
            ret = RK_ERROR;
            break;
    }

    if (ret == RK_ERROR)
    {
        rk_print_stringA("error cmd");
    }

    return ret;
}

/*
*********************************************************************************************************
*                                      void ShellTaskInit(void)
*
* Description:  This function is the Timer Task.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
rk_err_t ShellTaskInit(void *pvParameters)
{
    //RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)pvParameters;
    SHELL_TASK_DATA_BLOCK*  pShellTaskDataBlock;
    SHELL_CMD_ITEM * pShellCmdItem;

    uint32 i;

    pShellTaskDataBlock = &stShellTaskDataBlock;

    memset(pShellTaskDataBlock,NULL,sizeof(SHELL_TASK_DATA_BLOCK));
    pShellTaskDataBlock->ShellRxBuffer = ShellRxBuffer;
    pShellTaskDataBlock->pCmdHeadItem = (SHELL_CMD_ITEM *)CmdHeadItem;
    pShellCmdItem = (void *)pShellTaskDataBlock->pCmdHeadItem;
    for (i = 0; i < SHELL_CMD_MAX_ITEM; i++)
    {
        pShellCmdItem->useflag = 0;
        pShellCmdItem++;
    }

    gpstShellTaskDataBlock = pShellTaskDataBlock;
    ShellCmdRegisterALL();
    ShellPrintHelpInfo();
    {
        uint8* pstr;
        pstr = gpstShellTaskDataBlock->ShellRxBuffer;
        memset(pstr,0,SHELL_RX_BUF_MAX_SIZE);

        pstr[0] = 'r';
        pstr[1] = 'k';
        pstr[2] = 'o';
        pstr[3] = 's';
        pstr[4] = ':';
        pstr[5] = '/';
        pstr[6] = '/';

        gpstShellTaskDataBlock->ShellRxStart = 7;
        gpstShellTaskDataBlock->i = 7;

        rk_print_stringA("\r\n\r\n\r\n");
        rk_print_stringA(pstr);
    }


    return RK_SUCCESS;

exit:
    return RK_ERROR;
}


/*
*********************************************************************************************************
*                                      voidShellTaskDeInit(void)
*
* Description:  This function is the Timer Task.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
rk_err_t ShellTaskDeInit(void *pvParameters)
{
    return RK_SUCCESS;

exit:

    return RK_ERROR;

}

/*
*********************************************************************************************************
*                                      void ShellTaskDeInit(void)
*
* Description:  This function is the Timer Task.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void ShellTask(void)
{
    uint32 i,j;
    uint8* pstr;
    rk_size_t size;

    pstr = gpstShellTaskDataBlock->ShellRxBuffer;

    i = gpstShellTaskDataBlock->i;

    if (i <= SHELL_RX_BUF_MAX_SIZE)
    {
        size = UartDev_Read(gpstShellTaskDataBlock->hUart, pstr + i, 1);
        if (size == 0)
        {
            return;
        }

        if (pstr[i] == 0x0d)
        {
            UartDev_Write(gpstShellTaskDataBlock->hUart,"\r\n",2,SYNC_MODE,NULL);
            goto process;
        }
        else if ((pstr[i] < 32) && (pstr[i] != '\b'))
        {
            return;
        }
        else if ((pstr[i] >= 127))
        {
            return;
        }

        UartDev_Write(gpstShellTaskDataBlock->hUart,pstr+i,1,SYNC_MODE,NULL);

        if (pstr[i] == '\b')
        {
            if (i == gpstShellTaskDataBlock->ShellRxStart)
            {
                if (gpstShellTaskDataBlock->ShellRxStart == 7)
                {
                    UartDev_Write(gpstShellTaskDataBlock->hUart,"/",1,SYNC_MODE,NULL);
                }
                else
                {
                    UartDev_Write(gpstShellTaskDataBlock->hUart,".",1,SYNC_MODE,NULL);
                }
            }
            else
            {
                UartDev_Write(gpstShellTaskDataBlock->hUart," \b", 2, SYNC_MODE, NULL);
                i--;
                gpstShellTaskDataBlock->i = i;
            }

        }
        else
        {
            i++;
            gpstShellTaskDataBlock->i = i;
        }

    }

    return;

process:

    if (pstr[i] == 0x0d)
    {
        if (i == gpstShellTaskDataBlock->ShellRxStart)
        {
            memset(pstr + gpstShellTaskDataBlock->ShellRxStart,0,(SHELL_RX_BUF_MAX_SIZE - gpstShellTaskDataBlock->ShellRxStart));
            rk_print_stringA("\r\n\r\n");
            rk_print_stringA(pstr);

        }
        else if ((pstr[gpstShellTaskDataBlock->ShellRxStart] == 'c')
                 && (pstr[gpstShellTaskDataBlock->ShellRxStart + 1] == 'd')
                 && (pstr[gpstShellTaskDataBlock->ShellRxStart + 2] == 0x0d))
        {
            pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
        }
        else if ((pstr[gpstShellTaskDataBlock->ShellRxStart] == 'c')
                 && (pstr[gpstShellTaskDataBlock->ShellRxStart + 1] == 'd')
                 && (pstr[gpstShellTaskDataBlock->ShellRxStart + 2] == ' '))
        {


            if (pstr[gpstShellTaskDataBlock->ShellRxStart + 3] == 0x0d)
            {
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
            }
            else if ((pstr[gpstShellTaskDataBlock->ShellRxStart + 3] == '.')
                     && (pstr[gpstShellTaskDataBlock->ShellRxStart + 4] == '.')
                     &&(pstr[gpstShellTaskDataBlock->ShellRxStart + 5] == 0x0d))
            {
                if (gpstShellTaskDataBlock->ShellRxStart > 8)
                {
                    for (j = gpstShellTaskDataBlock->ShellRxStart - 2; j >= 7; j--)
                    {
                        if (pstr[j] == '.')
                        {
                            break;
                        }
                    }
                    gpstShellTaskDataBlock->ShellRxStart = j + 1;
                }
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
            }
            else if ((pstr[gpstShellTaskDataBlock->ShellRxStart + 3] == '.')
                     && (pstr[gpstShellTaskDataBlock->ShellRxStart + 4] == 0x0d))
            {
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
            }
            else if (memcmp(pstr + gpstShellTaskDataBlock->ShellRxStart + 3, "rkos://", 7) == 0)
            {

                pstr[i] = 0;
                for (j = 0; j < strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3); j++)
                {
                    pstr[j] = pstr[gpstShellTaskDataBlock->ShellRxStart + 3 + j];
                }

                if (j > 7)
                {
                    pstr[j++] = '.';
                    gpstShellTaskDataBlock->ShellRxStart = strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3) + 1;
                }
                else
                {
                    gpstShellTaskDataBlock->ShellRxStart = strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3);
                }
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;

            }
            else
            {
                pstr[i] = 0;
                for (j = gpstShellTaskDataBlock->ShellRxStart; j <(gpstShellTaskDataBlock->ShellRxStart + strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3)); j++)
                {
                    pstr[j] = pstr[3 + j];
                }
                pstr[j++] = '.';

                gpstShellTaskDataBlock->ShellRxStart += strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3) + 1;
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
            }


        }
        else
        {
            pstr[i] = 0;
            ShellRootParsing(gpstShellTaskDataBlock->hUart, pstr + 7);
            memset(pstr + gpstShellTaskDataBlock->ShellRxStart,0,(SHELL_RX_BUF_MAX_SIZE - gpstShellTaskDataBlock->ShellRxStart));
            rk_print_stringA("\r\n\r\n");
            rk_print_stringA(pstr);
        }

        gpstShellTaskDataBlock->i = gpstShellTaskDataBlock->ShellRxStart;

    }

}
#endif

