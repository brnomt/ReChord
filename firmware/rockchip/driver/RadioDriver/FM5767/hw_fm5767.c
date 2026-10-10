/******************************************************************/
#define _IN_HWRAD5767_

#include "SysInclude.h"

#ifdef _RADIO_

#include "hw_fm5767.h"

/**************************************************************************************
*   Name    : Tuner_ReadData
*   Function: Read data form FM chip (TEA5767).
*   Input   : pdata -- the pointer from which read out data.
*             size --  the size want to read.
*   Output  : NULL
*   Return  : TRUE --  Read success
*             FALSE -- Read failure
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
BOOL FM5767_Tuner_ReadData(UINT8 *pdata, UINT8 size)
{
    BOOL ret;
    I2C_Init(0,FM5767, 200);
    ret = I2C_Read(0,0, pdata, size, DirectMode);
    I2C_Deinit(0);
    return ret;
}

/**************************************************************************************
*   Name    : Tuner_SendCommand
*   Function: send data to FM chip (TEA5767)
*   Input   : pdata -- the pointer of would send data.
*             size  -- the size of would send data.
*   Output  : NULL
*   Return  : TRUE  -- send seccess
*             FALSE -- send failure
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_

BOOL FM5767_Tuner_SendCommand(UINT8 *pdata, UINT8 size)
{
    BOOL ret,ret1;

    I2C_Init(0,FM5767, 200);
    ret = I2C_Write(0,0, pdata, size, DirectMode);

    I2C_Deinit(0);
    return ret;
}

/**************************************************************************************
*   Name    : Tuner_Enable
*   Function: Enable FM chip (TEA5767).
*   Input   : null
*   Output  : NULL
*   Return  : null
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
void FM5767_Tuner_Enable(INT8U area)
{
    TEA5767ControlData[0] = 0x24;
    TEA5767ControlData[1] = 0x22;
    TEA5767ControlData[2] = 0x20;
    TEA5767ControlData[3] = 0x37;
    TEA5767ControlData[4] = 0x00;
    FM5767_Tuner_SendCommand(TEA5767ControlData, 5);
}

/**************************************************************************************
*   Name    : Tuner_Disable
*   Function: Disable FM chip (TEA5767).
*   Input   : NULL
*   Output  : NULL
*   Return  : null
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
void FM5767_Tuner_Disable(void)
{
    TEA5767ControlData[0] = 0xa4;
    TEA5767ControlData[1] = 0x22;
    TEA5767ControlData[2] = 0x20;
    TEA5767ControlData[3] = 0x77;
    TEA5767ControlData[4] = 0x00;
    FM5767_Tuner_SendCommand(TEA5767ControlData, 5);
}

/**************************************************************************************
*   Name    : Tuner_PowerOnInit
*   Function: FM chip (TEA5767) power on initialization.
*   Input   : NULL
*   Output  : NULL
*   Return  : null
*   explain ：close the FM chip(TEA5767) before power on.
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
void FM5767_Tuner_PowerOnInit(void)
{
    FM5767_Tuner_Disable();
}

/**************************************************************************************
*   Name    : Tuner_SetStereo
*   Function: Fm Radio Stereo Setting
*   Input   : NULL
*   Output  : NULL
*   Return  : null
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
void FM5767_Tuner_SetStereo(BOOL bStereo)
{
    TEA5767ControlData[2] &= 0xF7; //~TUNER_MS 0x08
    TEA5767ControlData[3] = 0x00 | TUNER_XTAL | TUNER_HCC ;//| TUNER_SNC;
    if (bStereo)
    {
        TEA5767ControlData[2] |= TUNER_MS;
    }
    else
    {
        TEA5767ControlData[3] |= TUNER_SNC;
    }
    FM5767_Tuner_SendCommand(TEA5767ControlData, 5);
}
/**************************************************************************************
*   Name    : GetStereoStatus
*   Function: get stereo status
*   Input   : NULL
*   Output  : NULL
*   Return  : TRUE  -- stereo status
*             FALSE -- non stero status
*   explain :
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
BOOLEAN FM5767_GetStereoStatus(void)
{
    UINT8 TempBuffer[3];
    FM5767_Tuner_ReadData(TempBuffer, 3);
    return (TempBuffer[2] & 0x80) ? TRUE : FALSE;
}
/**************************************************************************************
*   Name    : Tuner_PowerOffDeinit
*   Function: Fm Radio Power Off Deinit
*   Input   : null
*   Output  : NULL
*   Return  : null
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
void FM5767_Tuner_PowerOffDeinit(void)
{

    TEA5767ControlData[3] |= TUNER_STBY;
    FM5767_Tuner_SendCommand(TEA5767ControlData, 5);

}
/**************************************************************************************
*   Name    : Tuner_SetFrequency
*   Function: set frequency for FM chip(TEA5767).
*   Input   : n10KHz -- 频率(单位10KHz)
*             HILO -- 1(偏高设置) 0(偏低设置)
*             ForceMono -- 是否强制为单声道
*   Output  : NULL
*   Return  : null
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_

void FM5767_Tuner_SetFrequency(UINT32 n10KHz, UINT8 HILO, BOOL ForceMono,UINT16 Area)
{
    if (HILO)
    {
        n10KHz = (n10KHz * 10000 + 225000 + (1 << 12)) >> 13;
        HILO = 0x30;
    }
    else
    {
        n10KHz = (n10KHz * 10000 - 225000 + (1 << 12)) >> 13;
        HILO = 0x20;
    }

    if (ForceMono)
        HILO |= 0x08;

    TEA5767ControlData[0] = n10KHz >> 8;
    TEA5767ControlData[1] = n10KHz;
    TEA5767ControlData[2] = HILO;
    TEA5767ControlData[3] = 0x00 | TUNER_XTAL | TUNER_HCC ;

    FM5767_Tuner_SendCommand(TEA5767ControlData, 4);

}
/**************************************************************************************
*   Name    : void Tuner_MuteControl(bool active)
*   Function: 让FM静音
*   Input   :
*   Output  : NULL
*   Return  : null
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
void FM5767_Tuner_MuteControl(bool active)
{

    if(active)
         TEA5767ControlData[0] |= TUNER_MUTE;
    else
        TEA5767ControlData[0] &= (unsigned char)~TUNER_MUTE;
    
    if(FM5767_Tuner_SendCommand(TEA5767ControlData,1) != 0)
        FM5767_Tuner_SendCommand(TEA5767ControlData,1);
}

/**************************************************************************************
*   Name    : void Tuner_PowerDown(void)
*   Function: 关闭FM
*   Input   :
*   Output  : NULL
*   Return  : null
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
void FM5767_Tuner_PowerDown(void)
{
    TEA5767ControlData[3] |= TUNER_STBY;
    if(FM5767_Tuner_SendCommand(TEA5767ControlData,5) != 0) {
        FM5767_Tuner_SendCommand(TEA5767ControlData,5);
    }
}
/**************************************************************************************
*   Name    : FMSearchByHand
*   Function: FM 5767芯片手动搜索频点
*   Input   : direct -- 1(递增搜索), 0(递减搜索)
*   Output  : FM_FoundStation 搜到一个有效台
*   Return  : 搜索结果
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
UINT16 FM5767_Tuner_SearchByHand(UINT16 direct, UINT32 *FmFreq)
{
    INT8 temp, cnt, i;
    UINT8 *P2Reg_5767;
    UINT8 hilo = direct;
    UINT32 HwFreq = *FmFreq;
    
    for (i = 0;i < 2;i++)
    {
        cnt = 0;
        temp = 0;
        hilo = (i == 0) ? direct : (!((BOOLEAN)direct));
        P2Reg_5767 = (hilo ? IFReadDataH : IFReadDataL);

        FM5767_Tuner_SetFrequency(HwFreq, hilo, TRUE,NULL);

        DelayMs(40);

        while (!(temp&0x80))
        {
            DelayMs(4);
            if (++cnt > 25)
            {
                return 1;//FM_SearchFail;
            }
            FM5767_Tuner_ReadData(&temp, 1);
        }

        FM5767_Tuner_ReadData(P2Reg_5767, 5);
        temp = P2Reg_5767[2] & 0x7f;

        if ((FM_VS_LLIMIT > temp) || (temp > FM_VS_HLIMIT))
        {
            return 0;//FM_InvalidStation;  //not
        }

    }

    if (((IFReadDataH[3] >> 4) <= FM_SENSI) || ((IFReadDataL[3] >> 4) <= FM_SENSI))
    {
        return 0;//FM_InvalidStation;
    }

    temp = ((IFReadDataH[3] >> 4) & 0x000f) - ((IFReadDataL[3] >> 4) & 0x000f);

    if ((-FM_VS_WLIMIT > temp) || (temp > FM_VS_WLIMIT))
    {
        return 0;//FM_InvalidStation;//not
    }

    return 2;//FM_FoundStation;//search one valid station.
}



/**************************************************************************************
*   Name    : Tuner_SetInitArea
*   Function: set frequency for FM chip(TEA5767).
*   Input   :
*   Output  : NULL
*   Return  : null
*   explain :
**************************************************************************************/
_ATTR_FMDRIVER_FM5767_TEXT_
void FM5767_Tuner_SetInitArea(UINT8 area)
{

    if(2==area)
        FM5767_Tuner_Enable(1);
    else
        FM5767_Tuner_Enable(0);
}

//#endif
#endif 
