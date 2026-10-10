/**
 * wma_cache_helper @ 0x03084b98
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 wma_cache_helper(int *param_1)



{

  int iVar1;

  int iVar2;

  undefined4 uVar3;

  uint uVar4;

  undefined1 auStack_24 [4];

  undefined4 local_20;

  undefined4 uStack_1c;

  

  iVar1 = DAT_03084c34;

  uVar4 = 0;

  do {

    iVar2 = FATDeleteClus(*(undefined4 *)(iVar1 + 0xc),auStack_24);

    if (iVar2 != 0) {

      if (iVar2 == 6 || iVar2 == 1) {

        uVar3 = 6;

      }

      else {

        uVar3 = 1;

      }

      return uVar3;

    }

    local_20 = 0;

    uStack_1c = 0;

    *(int *)(iVar1 + 0x18) = *(int *)(iVar1 + 0x18) + 1;

    iVar2 = FUN_030854b2(*(undefined4 *)(iVar1 + 0xc),*param_1 + uVar4 * 4,0x2000,

                         *(int *)(iVar1 + 0x10) - uVar4,&local_20);

    uVar4 = uVar4 + iVar2;

  } while (uVar4 < *(uint *)(iVar1 + 0x10));

  iVar2 = *(int *)(iVar1 + 0x14) + *(int *)(iVar1 + 0x10);

  *(int *)(iVar1 + 0x14) = iVar2;

  param_1[5] = iVar2;

  return 0;

}
