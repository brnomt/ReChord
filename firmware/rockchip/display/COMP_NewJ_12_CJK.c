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
uint8_t Font12_CJK_CompData(uint8_t * pBuf, uint16_t offset) 
{
    uint32_t CharInNFAddr;
    
    CharInNFAddr = FontLogicAddress + offset * Font12_CJK_CompData_LEN + Font12_CJK_CompData_START;	
    LcdGetResourceData(CharInNFAddr, pBuf, Font12_CJK_CompData_LEN);
    return 1;
};
#endif // USEFONT_12dot
