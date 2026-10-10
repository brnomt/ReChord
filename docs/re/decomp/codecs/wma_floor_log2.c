/**
 * wma_floor_log2 @ 0x030842cc
 * Tags: codec, wma
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void wma_floor_log2(uint param_1)



{

  uint uVar1;

  

  uVar1 = 0;

  if (param_1 < 2) {

    return;

  }

  do {

    uVar1 = uVar1 + 1;

  } while (1 < param_1 >> (uVar1 & 0xff));

  return;

}
