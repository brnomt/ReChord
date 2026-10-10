/**
 * dir_playlist_helper @ 0x0308ca7c
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



undefined4 dir_playlist_helper(uint *param_1,int *param_2,uint *param_3,uint param_4)



{

  int iVar1;

  undefined4 uVar2;

  uint uVar3;

  byte *local_10;

  

  local_10 = (byte *)0x0;

  iVar1 = wma_input_cache_read

                    (param_1,0,*param_1 + param_4,param_1[1] + (uint)CARRY4(*param_1,param_4),0x18,

                     &local_10);

  if (iVar1 == 0x18) {

    *param_2 = (uint)*local_10 + (uint)local_10[3] * 0x1000000 +

               (uint)local_10[2] * 0x10000 + (uint)local_10[1] * 0x100;

    *(ushort *)(param_2 + 1) = (ushort)local_10[4] + (ushort)local_10[5] * 0x100;

    *(ushort *)((int)param_2 + 6) = (ushort)local_10[6] + (ushort)local_10[7] * 0x100;

    *(byte *)(param_2 + 2) = local_10[8];

    *(byte *)((int)param_2 + 9) = local_10[9];

    *(byte *)((int)param_2 + 10) = local_10[10];

    *(byte *)((int)param_2 + 0xb) = local_10[0xb];

    *(byte *)(param_2 + 3) = local_10[0xc];

    *(byte *)((int)param_2 + 0xd) = local_10[0xd];

    *(byte *)((int)param_2 + 0xe) = local_10[0xe];

    *(byte *)((int)param_2 + 0xf) = local_10[0xf];

    uVar3 = (uint)local_10[0x10] + (uint)local_10[0x13] * 0x1000000 +

            (uint)local_10[0x12] * 0x10000 + (uint)local_10[0x11] * 0x100;

    *param_3 = uVar3;

    param_3[1] = (uint)local_10[0x14] + (uint)local_10[0x17] * 0x1000000 +

                 (uint)local_10[0x16] * 0x10000 + (uint)local_10[0x15] * 0x100;

    if (uVar3 < 0x18) {

      uVar2 = 4;

    }

    else {

      uVar2 = 0;

    }

    return uVar2;

  }

  return 3;

}
