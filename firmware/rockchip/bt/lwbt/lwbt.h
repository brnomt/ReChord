#ifndef _LWBT_H_
#define _LWBT_H_
#include "SysConfig.h"

#ifdef _A2DP_SOUCRE_
#define _ATTR_LWBT_CODE_         
#define _ATTR_LWBT_DATA_         
#define _ATTR_LWBT_BSS_          

#define _ATTR_LWBT_UARTIF_CODE_         
#define _ATTR_LWBT_UARTIF_DATA_         
#define _ATTR_LWBT_UARTIF_BSS_          

#define _ATTR_LWBT_INIT_CODE_         
#define _ATTR_LWBT_INIT_DATA_         
#define _ATTR_LWBT_INIT_BSS_          
#else

#define _ATTR_LWBT_CODE_         
#define _ATTR_LWBT_DATA_         
#define _ATTR_LWBT_BSS_          

#define _ATTR_LWBT_UARTIF_CODE_         
#define _ATTR_LWBT_UARTIF_DATA_         
#define _ATTR_LWBT_UARTIF_BSS_          

#define _ATTR_LWBT_INIT_CODE_         
#define _ATTR_LWBT_INIT_DATA_         
#define _ATTR_LWBT_INIT_BSS_          
#endif
#endif