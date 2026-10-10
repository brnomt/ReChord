/**
 * sbuf_read_bits @ 0x030ce930
 * Tags: os, bitstream, sbuf
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 sbuf_read_bits(uint *param_1,int param_2)



{

  int iVar1;

  undefined4 uVar2;

  uint uVar3;

  uint uVar4;

  int *piVar5;

  uint uVar6;

  

  uVar2 = FUN_030ce7f0();

  uVar3 = *param_1;

  uVar6 = uVar3 + param_2 & 7;

  *param_1 = uVar6;

  iVar1 = (int)(uVar3 + param_2) >> 3;

  uVar3 = param_1[2] - iVar1;

  param_1[2] = uVar3;

  param_1[1] = param_1[1] + iVar1;

  while( true ) {

    if (0 < (int)uVar3) {

      return uVar2;

    }

    uVar3 = param_1[2];

    *param_1 = uVar6;

    param_1[2] = uVar3;

    uVar4 = param_1[3];

    if (*(int *)(uVar4 + 0xc) == 0) break;

    param_1[5] = param_1[5] + *(int *)(uVar4 + 8);

    piVar5 = *(int **)(uVar4 + 0xc);

    param_1[3] = (uint)piVar5;

    if (0 < (int)(piVar5[2] + uVar3)) {

      param_1[1] = (*piVar5 + piVar5[1]) - uVar3;

    }

    uVar3 = piVar5[2] + uVar3;

    param_1[2] = uVar3;

  }

  if ((int)uVar6 <= (int)(uVar3 * 8)) {

    return uVar2;

  }

  param_1[2] = 0xffffffff;

  return uVar2;

}
