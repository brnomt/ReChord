/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:
*
* Description:
*
* History:      <author>          <time>        <version>
*    desc:    ORG.
********************************************************************************
*/
#include "stdint.h"
#include "FontUseSetting.h"

#ifdef USEFONT_12dot
uint8_t Font12_8859Width(uint16_t code)
{
    uint32_t CharInNFAddr;
    uint8_t Temp;

    CharInNFAddr = FontLogicAddress + code * Font12_8859_Width_LEN + Font12_8859_Width_START;
    ReadResourceData(CharInNFAddr, &Temp, Font12_8859_Width_LEN);
    return Temp;
};
#endif // USEFONT_12dot
