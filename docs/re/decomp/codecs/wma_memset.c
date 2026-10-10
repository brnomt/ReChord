/**
 * wma_memset @ 0x0308280a
 * Tags: codec, wma
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void wma_memset(undefined1 *param_1,int param_2,undefined1 param_3)



{

  bool bVar1;

  

  while (bVar1 = param_2 != 0, param_2 = param_2 + -1, bVar1) {

    *param_1 = param_3;

    param_1 = param_1 + 1;

  }

  return;

}
