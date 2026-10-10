/**
 * wma_memcmp @ 0x0308283c
 * Tags: codec, wma
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void wma_memcmp(int param_1,int param_2,uint param_3)



{

  uint uVar1;

  

  for (uVar1 = 0; (uVar1 < param_3 && (*(char *)(param_1 + uVar1) == *(char *)(param_2 + uVar1)));

      uVar1 = uVar1 + 1) {

  }

  return;

}
