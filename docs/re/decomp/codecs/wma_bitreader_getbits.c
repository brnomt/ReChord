/**
 * wma_bitreader_getbits @ 0x03084970
 * Tags: codec, wma
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int wma_bitreader_getbits(int param_1,uint param_2,uint *param_3)



{

  byte bVar1;

  uint uVar2;

  uint uVar3;

  uint uVar4;

  int iVar5;

  int iVar6;

  byte *pbVar7;

  

  uVar4 = *(uint *)(param_1 + 0x28);

  iVar5 = 0;

  iVar6 = *DAT_03084a28;

  if (uVar4 < param_2) {

    uVar3 = *(uint *)(param_1 + 0x30);

    if (uVar3 != 0) {

      uVar2 = 0x20 - uVar4;

      if (uVar3 <= 0x20 - uVar4) {

        uVar2 = uVar3;

      }

      uVar3 = uVar3 - uVar2;

      *(uint *)(param_1 + 0x30) = uVar3;

      *(uint *)(param_1 + 0x24) =

           *(uint *)(param_1 + 0x2c) >> (uVar3 & 0xff) | *(int *)(param_1 + 0x24) << (uVar2 & 0xff);

      *(uint *)(param_1 + 0x28) = uVar2 + uVar4;

      *(uint *)(param_1 + 0x2c) = *(uint *)(param_1 + 0x2c) & (1 << (uVar3 & 0xff)) - 1U;

    }

    while ((uVar4 = *(uint *)(param_1 + 0x28), uVar4 < 0x19 && (*(int *)(param_1 + 0x20) != 0))) {

      uVar3 = *(int *)(param_1 + 0x24) << 8;

      *(uint *)(param_1 + 0x24) = uVar3;

      pbVar7 = *(byte **)(param_1 + 0x1c);

      *(byte **)(param_1 + 0x1c) = pbVar7 + 1;

      bVar1 = *pbVar7;

      *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) + -1;

      *(uint *)(param_1 + 0x24) = bVar1 | uVar3;

      *(uint *)(param_1 + 0x28) = uVar4 + 8;

    }

    if ((uVar4 < param_2) && (iVar5 = SNOR_SetDLines(param_1,2,param_2), iVar5 < 0)) {

      return iVar5;

    }

  }

  uVar4 = *(int *)(param_1 + 0x28) - param_2;

  *(uint *)(param_1 + 0x28) = uVar4;

  *param_3 = *(uint *)(param_1 + 0x24) >> (uVar4 & 0xff) & *(uint *)(iVar6 + param_2 * 4);

  return iVar5;

}
