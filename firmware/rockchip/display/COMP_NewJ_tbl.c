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

uint16_t FontCompTableAsia(uint16_t code)
{
    uint32_t CharInNFAddr;
    uint16_t Temp;
    
    CharInNFAddr = FontLogicAddress + code * Font_Comp_Table_Asia_LEN + Font_Comp_Table_Asia_START;	
    ReadResourceData(CharInNFAddr, &Temp, Font_Comp_Table_Asia_LEN);
    return Temp;
};

