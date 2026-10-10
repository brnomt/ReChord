/**
 * bitstream_getbits @ 0x030b15ca
 * Tags: os, bitstream
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint bitstream_getbits(int param_1,uint param_2)



{

  int iVar1;

  uint uVar2;

  

  param_2 = param_2 & 0x1f;

  uVar2 = *(uint *)(param_1 + 4);

  *(uint *)(param_1 + 4) = uVar2 << param_2;

  uVar2 = (uVar2 >> (0x1f - param_2 & 0xff)) >> 1;

  iVar1 = *(int *)(param_1 + 8) - param_2;

  *(int *)(param_1 + 8) = iVar1;

  if (iVar1 < 0) {

    FUN_030b1532(param_1);

    uVar2 = uVar2 | *(uint *)(param_1 + 4) >> (iVar1 + 0x20U & 0xff);

    *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) << (-iVar1 & 0xffU);

    *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + iVar1;

  }

  return uVar2;

}
