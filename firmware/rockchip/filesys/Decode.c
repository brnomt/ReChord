/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Decode.c
* 
* Description:  
*
* History:      <author>          <time>        <version>       
*             ZhengYongzhi      2008-10-21          1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_DECODE_

#include "FsInclude.h"

/*
--------------------------------------------------------------------------------
  Function name : void RC4(UINT8 *buf, UINT16 len)
  Author        : ZHengYongzhi
  Description   : RC4 encryption decryption algorithm
                  
  Input         : *buf: encryption data
                   len: the length of encryption/decryption data.
  Return        : 

  History:     <author>         <time>         <version>       
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
#if 1
_ATTR_FLASH_DATA_ UINT8 const key[16] = {124,78,3,4,85,5,9,7,45,44,123,56,23,13,23,17};

_ATTR_FLASH_CODE_
void RC4(UINT8 *buf, UINT16 len)
{
	UINT16 i,j,t,x;
	UINT8  S[256], K[256], temp;
	
	j = 0;
	for (i = 0; i < 256; i++) {
	    
		S[i] = (UINT8)i;
		
		j   &= 0x0f;
		K[i] = key[j];
		
		j++;
	}
	
	j = 0;
	for (i = 0; i < 256; i++) {
	    
		j = (j + S[i] + K[i]) % 256;
		
		temp = S[i];
		S[i] = S[j];
		S[j] = temp;
	}
	
	i = j = 0;
	for (x = 0; x < len; x++) {
	    
		i = (i+1) % 256;
		j = (j + S[i]) % 256;
		
		temp = S[i];
		S[i] = S[j];
		S[j] = temp;
		
		t = (S[i] + (S[j] % 256)) % 256;
		buf[x] = buf[x] ^ S[t];
	}
}
#else
void RC4(UINT8 *buf, UINT16 len)
{
    void(*pRC4)(UINT8 *buf, UINT16 len);
    
    pRC4 = (void *)0x000000b1;
    (*pRC4)(buf, len);
}
#endif
/*
********************************************************************************
*
*                         End of Decode.c
*
********************************************************************************
*/
