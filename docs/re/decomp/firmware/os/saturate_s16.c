/**
 * saturate_s16 @ 0x030b38e0
 * Tags: os, math
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int saturate_s16(int param_1)



{

  ushort uVar1;

  

  uVar1 = (ushort)param_1;

  if (param_1 >> 0x1f != param_1 >> 0xf) {

    uVar1 = (ushort)(param_1 >> 0x1f) ^ 0x7fff;

  }

  return (int)(short)uVar1;

}
