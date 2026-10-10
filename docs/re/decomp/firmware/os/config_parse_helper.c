/**
 * config_parse_helper @ 0x0301ded2
 * Named via cross-version lineage (v3.4.0/v3.0.0 clusters); decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 config_parse_helper(uint param_1)



{

  int iVar1;

  uint uVar2;

  uint uVar3;

  undefined4 *unaff_r4;

  undefined4 *unaff_r5;

  int unaff_r6;

  uint uVar4;

  int unaff_r7;

  undefined4 unaff_r8;

  uint unaff_r9;

  uint unaff_r10;

  

  while( true ) {

    *(int *)(unaff_r7 + 8) = *(int *)(unaff_r7 + 8) + 1;

    if (*(ushort *)((int)unaff_r5 + 2) == unaff_r9) {

      iVar1 = unaff_r4[5];

      unaff_r4[5] = iVar1 + 1U;

      if (*(uint *)(unaff_r7 + 4) < iVar1 + 1U) {

        return 2;

      }

      uVar4 = *(uint *)(DAT_0301df60 + 0xc);

      uVar2 = FUN_0301d680(*unaff_r4);

      if (uVar2 < uVar4) {

        uVar2 = 0;

      }

      else {

        uVar2 = uVar2 - uVar4;

      }

      uVar4 = (uint)*(byte *)(unaff_r5 + 2) | *(int *)((int)unaff_r5 + 9) << 8;

      unaff_r4[6] = uVar4;

      uVar3 = (uint)*(byte *)(unaff_r5 + 3) | *(int *)((int)unaff_r5 + 0xd) << 8;

      unaff_r4[7] = uVar3;

      if (uVar4 < uVar2) {

        iVar1 = -8;

      }

      else {

        iVar1 = uVar4 - uVar2;

      }

      unaff_r4[3] = iVar1 + 8;

      unaff_r4[4] = uVar3;

      return 1;

    }

    if (*(ushort *)((int)unaff_r5 + 2) != unaff_r10) break;

    if (unaff_r4[5] == *(int *)(unaff_r7 + 4)) {

      return 2;

    }

    uVar2 = unaff_r4[8];

    if (uVar2 < param_1) {

      unaff_r5 = (undefined4 *)(uVar2 + unaff_r6);

      unaff_r4[8] = uVar2 + 0x10;

    }

    else {

      param_1 = ape_rom_read_thunk(unaff_r4 + 9,0x400,unaff_r4[1]);

      if (param_1 < 0x10) {

        return 2;

      }

      *(uint *)(unaff_r7 + 0x18) = param_1;

      unaff_r5 = unaff_r4 + 9;

      unaff_r4[8] = unaff_r8;

    }

  }

  return 0xffffffff;

}
