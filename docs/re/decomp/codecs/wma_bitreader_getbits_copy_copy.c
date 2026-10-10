/**
 * wma_bitreader_getbits_copy_copy @ 0x03088390
 * Tags: codec, wma (similarity match)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined4 FUN_03088390(int param_1,uint param_2)



{

  undefined4 uVar1;

  int iVar2;

  byte *pbVar3;

  uint uVar4;

  

  uVar1 = 0;

  if ((uint)(*(int *)(param_1 + 0x28) + *(int *)(param_1 + 0x30) + *(int *)(param_1 + 0x20) * 8) <

      param_2) {

    while ((*(uint *)(param_1 + 0x28) < 0x19 && (*(int *)(param_1 + 0x20) != 0))) {

      uVar4 = *(int *)(param_1 + 0x24) << 8;

      *(uint *)(param_1 + 0x24) = uVar4;

      pbVar3 = *(byte **)(param_1 + 0x1c);

      *(byte **)(param_1 + 0x1c) = pbVar3 + 1;

      *(uint *)(param_1 + 0x24) = *pbVar3 | uVar4;

      *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) + -1;

      *(uint *)(param_1 + 0x28) = *(uint *)(param_1 + 0x28) + 8;

    }

    *(undefined4 *)(param_1 + 0x2c) = 0;

    iVar2 = *(int *)(param_1 + 0x20);

    while (iVar2 != 0) {

      uVar4 = *(int *)(param_1 + 0x2c) << 8;

      *(uint *)(param_1 + 0x2c) = uVar4;

      pbVar3 = *(byte **)(param_1 + 0x1c);

      *(byte **)(param_1 + 0x1c) = pbVar3 + 1;

      *(uint *)(param_1 + 0x2c) = *pbVar3 | uVar4;

      iVar2 = *(int *)(param_1 + 0x20) + -1;

      *(int *)(param_1 + 0x20) = iVar2;

      *(int *)(param_1 + 0x30) = *(int *)(param_1 + 0x30) + 8;

    }

    uVar1 = FUN_030847b2(param_1,0,param_2);

  }

  return uVar1;

}




