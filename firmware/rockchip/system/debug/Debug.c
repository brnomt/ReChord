/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:   Debug.c
*
* Description:
*
*
* History:      <author>          <time>        <version>
*    desc:    ORG.
********************************************************************************
*/

#include <stdio.h>
#include "DriverInclude.h"

#include  "SysInclude.h"
#include  "fsinclude.h"
#include  "File.h"
#include  "FDT.h"
#include "device.h"
#include <stdarg.h>

#define DEBUG_TIME_LEN  8

#define PRINTF_CHAR_DATA 0
#define PRINTF_HEX_DATA  1
#define PRINTF_DEC_DATA  2

uint8  DebugFlag = 0;
uint32 BcoreDebug;
uint8 * pBBDebugBuf;

static uint8 lastchar = 0;
void DebugDisable(void)
{
    DebugFlag = 1;
}

void DebugEnable(void)
{
    DebugFlag = 0;
}

int fputc_hook(char ch)
{
    if (DebugFlag)
        return;

    DebugFlag = 1;

    #ifdef _UART_DEBUG_
    {
        while (UARTWriteByte0(DEBUG_UART_PORT,(uint8*)&ch, 1) == 0);
    }
    #endif

    #ifdef _FILE_DEBUG_
    {
        if (pDebugHook != NULL)
        {
            pDebugHook(ch);
        }
    }
    #endif

    DebugFlag = 0;

    return OK;
}

int fputc(int ch, FILE *f)
{
    uint8 dgbBuffer[DEBUG_TIME_LEN];
    uint32 tmpcnt, i;

    if (ch == '\n')
    {
        if(lastchar == '\n')
        {
            return OK;
        }
        lastchar = ch;
        tmpcnt = SysTickCounter;

        for (i = 0; i < DEBUG_TIME_LEN; i++)
        {
            dgbBuffer[i] = tmpcnt % 10;
            tmpcnt = tmpcnt / 10;
        }

        fputc_hook('\r');
        fputc_hook('\n');
        fputc_hook('[');

        for (i = 0; i < DEBUG_TIME_LEN; i++)
        {
            fputc_hook(dgbBuffer[DEBUG_TIME_LEN - 1 - i] + 0x30);

            if (DEBUG_TIME_LEN - 1 - i == 2)
            {
                fputc_hook('.');
            }
        }

        fputc_hook(']');
        return OK;
    }
    lastchar = ch;
    fputc_hook(ch);
    return OK;
}

uint32 Rk_Printf_lock = 0;

void rk_printf(const char * fmt, ...)
{
    uint8 buf[256];
    uint8 size;
    uint32 tmpcnt, i, j;
    uint8 dgbBuffer[DEBUG_TIME_LEN];
    va_list arg_ptr;

    IntMasterDisable();
    if (Rk_Printf_lock > 0)
    {
        IntMasterEnable();
        return;
    }
    Rk_Printf_lock = 1;
    IntMasterEnable();

    tmpcnt = SysTickCounter;

    for (i = 0; i < DEBUG_TIME_LEN; i++)
    {
        dgbBuffer[i] = tmpcnt % 10;
        tmpcnt = tmpcnt / 10;
    }

    buf[0] = '\r';
    buf[1] = '\n';
    buf[2] = '[';
    buf[3] = 'A';
    buf[4] = ']';
    buf[5] = '[';

    for (i = 0 , j = 0; i < DEBUG_TIME_LEN; i++)
    {
        buf[j + 6] = dgbBuffer[DEBUG_TIME_LEN - 1 - i] + 0x30;

        if (DEBUG_TIME_LEN - 1 - i == 2)
        {
            j++;
            buf[j + 6] = '.';
        }

        j++;
    }

    buf[j + 6] = ']';
    va_start(arg_ptr, fmt);
    size =  vsnprintf((char*)(buf + j + 7), 256, (const char *)fmt, arg_ptr);
#ifdef _UART_DEBUG_
    {
        UartDev_Write(NULL, buf, size + 16,  SYNC_MODE, NULL);
    }
#endif

    IntMasterDisable();
    Rk_Printf_lock = 0;
    IntMasterEnable();
}

void bt_printf(const char * fmt,...)
{
#ifdef _BT_DEBUG_
    rk_printf(fmt);
#endif
}

void rk_printf_no_time(const char * fmt,...)
{
    uint8 buf[256];
    uint8 size;
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
#ifdef _UART_DEBUG_
    {
        UartDev_Write(NULL, "\r\n[A]", 5, SYNC_MODE, NULL);
    }
#endif
    size =  vsnprintf((char*)buf, 256, (const char *)fmt, arg_ptr);
#ifdef _UART_DEBUG_
    {
        UartDev_Write(NULL, buf, size,  SYNC_MODE, NULL);
    }
#endif
}

void rk_print_string(const uint8 * pstr)
{
#ifdef _UART_DEBUG_
    {
        if (pstr[0] == '\r')
        {
            UartDev_Write(NULL, "\r\n[A]", 5, SYNC_MODE, NULL);
            UartDev_Write(NULL, pstr + 2, strlen(pstr) - 2, SYNC_MODE, NULL);
        }
        else
        {
            UartDev_Write(NULL, pstr, strlen(pstr), SYNC_MODE, NULL);
        }
    }
#endif
}

void dumpMemoryCharA(uint8* addr, uint32 size, uint8 data_type)
{
#ifdef _UART_DEBUG_
    uint32 index = 0;
    uint8 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        if (index == 0)
            printf("\r\n");

        if (index % 16 == 0 && index != 0)
            printf("\n");

        switch ( data_type )
        {
            case PRINTF_HEX_DATA:
                printf("%02x ", *ptemp);
                break;

            case PRINTF_DEC_DATA:
                printf("%d ", *ptemp);
                break;

            case PRINTF_CHAR_DATA:
                printf("%c ", *ptemp);
                break;
        }

        ptemp++;
    }
#endif
}

void dumpMemoryShortA(uint16* addr, uint32 size, uint8 data_type)
{
#ifdef _UART_DEBUG_
    uint32 index = 0;
    uint8 data[2];
    uint16 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        if (index == 0)
            printf("\r\n");

        if (index % 8 == 0 && index != 0)
            printf("\n");

        data[1] = *ptemp >> 8 & 0xff;
        data[0] = *ptemp & 0xff;

        switch ( data_type )
        {
            case PRINTF_HEX_DATA:
                //printf("%02x ", data[1] );
                //printf("%02x ", data[0] );
                printf("%04x ", *ptemp);
                break;

            case PRINTF_DEC_DATA:
                printf("%d ", data[1] );
                printf("%d ", data[0] );
                break;

            case PRINTF_CHAR_DATA:
                if(data[1] == 0)
                {
                    printf("%c ", data[0] );
                }
                else
                {
                    printf("%d ", data[1] );
                    printf("%d ", data[0] );
                }
                break;

        }

        ptemp++;
    }
#endif
}

void dumpMemoryShortA2String(uint16* addr,uint32 size)
{
#ifdef _UART_DEBUG_
    uint32 index = 0;
    uint8 data[2];
    char strBuf[256];
    uint16 *ptemp;
    ptemp = addr;

    if(size > 256)
    {
        printf("Error!!! Too long short buffer.\n");
        return;
    }

    for(index = 0; index < size;index++)
    {
        if(*ptemp == 0)
            break;

        if(index == 0)
            printf("\r\n");

        data[1] = *ptemp >> 8 & 0xff;
        data[0] = *ptemp & 0xff;

        if(data[1] != 0)
        {
            printf("Error!! Can't translate to char string.\n");
            break;
        }

        strBuf[index] = data[0];
        ptemp++;
    }

    strBuf[index] = 0;

    printf(" %s\n",strBuf);
#endif
}

void dumpMemoryIntA(uint32* addr, uint32 size)
{
#ifdef _UART_DEBUG_
    uint32 index = 0,i;
    uint8 data[4];
    uint32 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        if (index == 0)
            printf("\r\n");

        if (index % 4 == 0 && index != 0)
            printf("\n");

        data[3] = *ptemp >> 24 & 0xff;
        data[2] = *ptemp >> 16 & 0xff;
        data[1] = *ptemp >> 8  & 0xff;
        data[0] = *ptemp & 0xff;

        for (i = 4; i > 0; i--)
        {
            printf("%02x ", data[i - 1] );
        }

        ptemp++;
    }
#endif
}


/*
--------------------------------------------------------------------------------
  Function name : FileDebug
  Author        :
  Description   : Debug information save to filesystem

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
#ifdef _FILE_DEBUG_

#define DEBUG_BUF_SIZE 2048
HANDLE DgbHandle = -1;
char   SysDgbBuf[DEBUG_BUF_SIZE];
uint32 SysDgbBufCnt = 0;

int SysDebugHook(char ch)
{
    if ((SysDgbBufCnt >= DEBUG_BUF_SIZE) && (DgbHandle >= 0))
    {
        FileWrite(SysDgbBuf, FileInfo[DgbHandle].FileSize, DEBUG_BUF_SIZE, DgbHandle);
        SysDgbBufCnt = 0;
        memset(SysDgbBuf, 0x20, DEBUG_BUF_SIZE);
    }

    SysDgbBuf[SysDgbBufCnt] = ch;
    SysDgbBufCnt++;
}

int SysDebugHookInitMem(void)
{
    DgbHandle = -1;
    SysDgbBufCnt = 0;
    memset(SysDgbBuf, 0x20, DEBUG_BUF_SIZE);
    pDebugHook = SysDebugHook;
}

int SysDebugHookInit(void)
{
    if ((DgbHandle = FileOpenA("\\", "SYSDEBUGLOG", "WR")) == NOT_OPEN_FILE) //after open file ok,mount disk.
    {
        DgbHandle = FileCreate("\\", "SYSDEBUGLOG");
    }

    FileSeek(FileInfo[DgbHandle].FileSize, SEEK_SET, DgbHandle);
}

int SysDebugHookDeInit(void)
{
    pDebugHook = NULL;
    FileWrite(SysDgbBuf, FileInfo[DgbHandle].FileSize, DEBUG_BUF_SIZE, DgbHandle);
    FileClose(DgbHandle);
    DgbHandle = -1;
}

#endif


void BBDebug(void)
{
    if (BcoreDebug)
    {
#ifdef _UART_DEBUG_
        {
            UartDev_Write(NULL, pBBDebugBuf, StrLenA(pBBDebugBuf), SYNC_MODE, NULL);
        }
#else
        {
            DelayMs(5);
        }
#endif

        MailBoxWriteA2BCmd(MSGBOX_CMD_SYSTEM_PRINT_LOG_OK, MAILBOX_ID_0, MAILBOX_CHANNEL_3);
        MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_3);
        BcoreDebug = 0;
    }
}

void BBReqDebug(uint8 * buf)
{
    pBBDebugBuf = buf;
    BcoreDebug = 1;
}

void DebugInit(void)
{
    BcoreDebug = 0;
#ifdef _UART_DEBUG_
    {
#if(DEBUG_UART_PORT == 1)
        UARTInit(UART_CH1_PA, 115200, UART_DATA_8B, UART_ONE_STOPBIT, UART_PARITY_DISABLE);
#elif(DEBUG_UART_PORT == 0)
        UARTInit(UART_CH0_PA, 115200,UART_DATA_8B,UART_ONE_STOPBIT, UART_PARITY_DISABLE);
        //usb serial
        //UARTInit(UART_CH0_PB, 115200, UART_DATA_8B, UART_ONE_STOPBIT, UART_PARITY_DISABLE);
#elif(DEBUG_UART_PORT == 2)
        UARTInit(UART_CH2_PA,115200,UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
#endif
        {
#include "device.h"
#include "UartDevice.h"
            UART_DEV_ARG stUartArg;
            stUartArg.dwBitWidth = UART_DATA_8B;
#ifdef UART_USE_921600
            stUartArg.dwBitRate = UART_BR_921600;
#endif
#ifdef UART_USE_115200
            //stUartArg.dwBitRate = UART_BR_115200;
            stUartArg.dwBitRate = UART_BR_1500000;
#endif
#ifdef _CMD_TO_USB_SERIAL_
            UartDev_Create(1, &stUartArg);
#else
            UartDev_Create(0, &stUartArg);
#endif
        }
    }
#endif
}



