/**
 * FATDelClusChain @ 0x030644c4
 * Tags: fs, fat
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined1 FUN_030644c4(undefined4 param_1)



{

  int iVar1;

  undefined1 uVar2;

  uint uVar3;

  uint uVar4;

  uint uVar5;

  

  FUN_0306444c(param_1,10);

  iVar1 = DAT_0306487c;

  uVar5 = (uint)*(ushort *)(DAT_0306487c + 6);

  uVar4 = *(uint *)(*(int *)(DAT_0306487c + 0x14) + 0xc);

  uVar3 = uVar5 - 10;

  if (uVar4 == uVar3) {

    FUN_0304ce2c();

    return 0;

  }

  if (uVar5 <= uVar4) {

    uVar3 = (uint)*(ushort *)(DAT_0306487c + 4);

  }

  if (uVar5 <= uVar4 && uVar4 <= uVar3) {

    FUN_0304ce2c(uVar3);

    uVar2 = (**(code **)(DAT_03064884 + 0x14))(param_1,*(int *)(iVar1 + 0x14) + 0xc);

    return uVar2;

  }

  return 1;

}




