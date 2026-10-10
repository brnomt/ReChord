/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                                    (c) Copyright 2013, RockChip.Ltd
*                                          All Rights Reserved
*File    : BSP.C
* By     : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/
//#include "typedef_rkos.h"
#include <stdarg.h>
#include "DriverInclude.h"
#include "device.h"



/*
*********************************************************************************************************
*                                        Macro Define
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        Variable Define
*********************************************************************************************************
*/

// �����豸����Ҫ�����豸���ڴ�����
enum RK_device_list
{
    UartDevNO = 0,
    ADCDevNO,
    KeyDevNO,
    MAX_DEVICE_NUM,
};
//�豸�����б�
char* DeviceName[] =
{
    "UartDevice",
    "ADCDevice",
    "KeyDevice"
};

/*
*********************************************************************************************************
*                                     int fputc(int ch, FILE *f)
* Description:  This function is for the rk_printf from uart debug.
*
* Argument(s) : int ch, FILE *f
* Return(s)   : int
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint32  SysTickCntSave;
uint32  SysTickSave;
    
void rk_count_clk_start()
{
    SysTickCntSave = SysTickCounter;
    SysTickSave = nvic->SysTick.Value;
}

void rk_count_clk_end()
{
    uint32 SysTick, SysTickCnt;
    uint32 clk;

    SysTickCnt = SysTickCounter;
    SysTick = nvic->SysTick.Value;

    if(SysTickCntSave == SysTickCnt)
    {
        clk = SysTickSave - SysTick;
    }
    else
    {
        clk = SysTickSave + (SysTickCnt - SysTickCntSave - 1) * nvic->SysTick.Reload
            + (nvic->SysTick.Reload - SysTick);
    }

    rk_printf("total clk = %u", clk);
    
}


void MemCpy(uint8 * trg, uint8 * src, uint32 len)
{
    uint32 i;
    
    for(i = 0; i < len; i++)
    {
        *trg++ = *src++;
    }    
}
void MemMov(uint8 * trg, uint8 * src, uint32 len)
{
    uint32 i;
    
    for(i = 0; i < len; i++)
    {
        *trg++ = *src++;
    }    
}
void MemSet(uint8 * trg, int  data, uint32 len)
{
    uint32 i;
    for(i = 0; i < len; i++)
    {
        *trg++ = data;
    }    
}


uint32 StrLenW(uint16 * pstr)
{
    uint32 i = 0;
    while (*pstr++ != 0)
        i++;

    return i;
}

uint32 StrLenA(uint8 * pstr)
{
    uint32 i = 0;
    while (*pstr++ != 0)
        i++;

    return i;
}

int32 StrCmpW(uint16 * pstr1, uint16 * pstr2, uint32 len)
{
    uint32 len1, len2;
    
    if(len == 0)
    {
        len1 = StrLenW(pstr1);
        len2 = StrLenW(pstr2);

        if(len1 > len2)
        {
            return 1;
        }
        else if(len1 < len2)
        {
            return -1;
        }
        else
        {
            len = len1;
        }
    }

    while (len--)
    {
        if ((*pstr1 == 0) && (*pstr2 == 0))
        {
            return 0;
        }
        else if (*pstr1 == 0)
        {
            return -1;
        }
        else if (*pstr2 == 0)
        {
            return 1;
        }
        if (*pstr1 > * pstr2)
        {
            return 1;
        }
        else if (*pstr1 < * pstr2)
        {
            return -1;
        }
        pstr1++;
        pstr2++;
    }

    return 0;

}

int32 StrCmpA(uint8 * pstr1, uint8 * pstr2, uint32 len)
{
    uint32 len1, len2;
    
    if(len == 0)
    {
        len1 = StrLenA(pstr1);
        len2 = StrLenA(pstr2);

        if(len1 > len2)
        {
            return 1;
        }
        else if(len1 < len2)
        {
            return -1;
        }
        else
        {
            len = len1;
        }
    }


    while (len--)
    {
        if ((*pstr1 == 0) && (*pstr2 == 0))
        {
            return 0;
        }
        else if (*pstr1 == 0)
        {
            return -1;
        }
        else if (*pstr2 == 0)
        {
            return 1;
        }
        if (*pstr1 > * pstr2)
        {
            return 1;
        }
        else if (*pstr1 < * pstr2)
        {
            return -1;
        }
        pstr1++;
        pstr2++;
    }

    return 0;

}

uint32 StringtoNum(uint8 * pstr)
{
    uint32 num;
    int32 i;

    num = 0;
    for (i = (StrLenA(pstr) - 1); i >= 0; i--)
    {
        num = num * 10;
        num += (pstr[i] - 0x30);
    }

    return num;
}

uint32 Unicode2Ascii(uint8 * pbAscii, uint16 * pwUnicode, uint32 len)
{
    uint32 i;
    uint8 * pbUnicode = (uint8 *)pwUnicode;

    for (i = 0; i < len;)
    {
        if ((*pbUnicode == 0) && (*(pbUnicode + 1) == 0))
        {
            pbAscii[i] = 0;
            i++;
            break;
        }

        if (*pbUnicode != 0)
        {
            pbAscii[i] = *pbUnicode;
            i++;
        }

        if (*(pbUnicode + 1) != 0)
        {
            pbAscii[i] = *(pbUnicode + 1);
            i++;
        }
        pbUnicode += 2;
    }

    return i;
}

rk_err_t ShellCheckCmd(uint8 * CmdStr[],  uint8 * Cmd,  uint32 len)
{

    int32 i = 0;

    while (1)   //compare the name
    {
        if (len != StrLenA(CmdStr[i]))
        {
            i++;
        }
        else
        {
            if (StrCmpA(CmdStr[i], Cmd, len) != 0)
            {
                i++;
            }
            else
            {
                break;
            }
        }

        if (*(CmdStr[i]) == '\b')
            return RK_ERROR;

    }

    return i;

}

uint32 ShellItemExtract(uint8 * pstr, uint8 ** pItem)
{
    uint32 i = 0;
    uint16 ItemSize = 0;

    (*pItem) = (uint8 *)(pstr + i);

    while ((*(pstr + i) != ' ') && (*(pstr + i) != '.') && (*(pstr + i) != NULL))
    {
        ItemSize++;
        i++;
    }

    return ItemSize;

} 