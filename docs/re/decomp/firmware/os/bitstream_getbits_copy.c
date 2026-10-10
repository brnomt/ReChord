/**
 * bitstream_getbits_copy @ 0x030b1682
 * Tags: os, bitstream (similarity match)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




void FUN_030b1682(int param_1,uint param_2)



{

  param_2 = param_2 & 0x1f;

  if (*(int *)(param_1 + 8) < (int)param_2) {

    param_2 = param_2 - *(int *)(param_1 + 8);

    FUN_030b1532(param_1);

  }

  *(int *)(param_1 + 4) = *(int *)(param_1 + 4) << (param_2 & 0xff);

  *(uint *)(param_1 + 8) = *(int *)(param_1 + 8) - param_2;

  return;

}




