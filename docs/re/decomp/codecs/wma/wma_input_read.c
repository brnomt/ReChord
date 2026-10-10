/**
 * wma_input_read @ 0x0308c9a4
 * Introduced in v3.5.0 (changelog: album sorting / Favorites)
 * Named via cross-version lineage; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



undefined4 wma_input_read(uint *param_1,int *param_2,uint *param_3)



{

  int iVar1;

  uint uVar2;

  undefined4 uVar3;

  byte *local_18 [2];

  

  local_18[0] = (byte *)0x0;

  iVar1 = wma_input_cache_read(param_1,local_18,*param_1,param_1[1],0x18,local_18);

  if (iVar1 == 0x18) {

    uVar2 = *param_1;

    *param_1 = uVar2 + 0x18;

    param_1[1] = param_1[1] + (uint)(0xffffffe7 < uVar2);

    *param_2 = (uint)*local_18[0] + (uint)local_18[0][3] * 0x1000000 +

               (uint)local_18[0][2] * 0x10000 + (uint)local_18[0][1] * 0x100;

    *(ushort *)(param_2 + 1) = (ushort)local_18[0][4] + (ushort)local_18[0][5] * 0x100;

    *(ushort *)((int)param_2 + 6) = (ushort)local_18[0][6] + (ushort)local_18[0][7] * 0x100;

    *(byte *)(param_2 + 2) = local_18[0][8];

    *(byte *)((int)param_2 + 9) = local_18[0][9];

    *(byte *)((int)param_2 + 10) = local_18[0][10];

    *(byte *)((int)param_2 + 0xb) = local_18[0][0xb];

    *(byte *)(param_2 + 3) = local_18[0][0xc];

    *(byte *)((int)param_2 + 0xd) = local_18[0][0xd];

    *(byte *)((int)param_2 + 0xe) = local_18[0][0xe];

    *(byte *)((int)param_2 + 0xf) = local_18[0][0xf];

    uVar2 = (uint)local_18[0][0x10] + (uint)local_18[0][0x13] * 0x1000000 +

            (uint)local_18[0][0x12] * 0x10000 + (uint)local_18[0][0x11] * 0x100;

    *param_3 = uVar2;

    param_3[1] = (uint)local_18[0][0x14] + (uint)local_18[0][0x17] * 0x1000000 +

                 (uint)local_18[0][0x16] * 0x10000 + (uint)local_18[0][0x15] * 0x100;

    if (uVar2 < 0x18) {

      uVar3 = 4;

    }

    else {

      uVar3 = 0;

    }

    return uVar3;

  }

  return 3;

}
