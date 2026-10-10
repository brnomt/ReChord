/**
 * wma_bitreader_getbits_copy @ 0x030848dc
 * Tags: codec, wma (similarity match)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int wma_bitreader_getbits_copy(int param_1,uint param_2)



{

  byte bVar1;

  uint uVar2;

  uint uVar3;

  int iVar4;

  byte *pbVar5;

  uint uVar6;

  

  uVar6 = *(uint *)(param_1 + 0x28);

  iVar4 = 0;

  if (uVar6 < param_2) {

    uVar3 = *(uint *)(param_1 + 0x30);

    if (uVar3 != 0) {

      uVar2 = 0x20 - uVar6;

      if (uVar3 <= 0x20 - uVar6) {

        uVar2 = uVar3;

      }

      uVar3 = uVar3 - uVar2;

      *(uint *)(param_1 + 0x30) = uVar3;

      *(uint *)(param_1 + 0x24) =

           *(int *)(param_1 + 0x24) << (uVar2 & 0xff) | *(uint *)(param_1 + 0x2c) >> (uVar3 & 0xff);

      *(uint *)(param_1 + 0x2c) = (1 << (uVar3 & 0xff)) - 1U & *(uint *)(param_1 + 0x2c);

      *(uint *)(param_1 + 0x28) = uVar6 + uVar2;

    }

    while ((uVar6 = *(uint *)(param_1 + 0x28), uVar6 < 0x19 && (*(int *)(param_1 + 0x20) != 0))) {

      uVar3 = *(int *)(param_1 + 0x24) << 8;

      *(uint *)(param_1 + 0x24) = uVar3;

      pbVar5 = *(byte **)(param_1 + 0x1c);

      *(byte **)(param_1 + 0x1c) = pbVar5 + 1;

      bVar1 = *pbVar5;

      *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) + -1;

      *(uint *)(param_1 + 0x24) = bVar1 | uVar3;

      *(uint *)(param_1 + 0x28) = uVar6 + 8;

    }

    if ((uVar6 < param_2) && (iVar4 = SNOR_SetDLines(param_1,2,param_2), iVar4 < 0)) {

      return iVar4;

    }

  }

  *(uint *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) - param_2;

  return iVar4;

}
