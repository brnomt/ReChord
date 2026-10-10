/**
 * genre_play_helper @ 0x0301e26c
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 genre_play_helper(void)



{

  longlong lVar1;

  int iVar2;

  uint uVar3;

  undefined4 uVar4;

  uint uVar5;

  undefined4 *puVar6;

  int iVar7;

  int iVar8;

  

  iVar2 = DAT_0301e318;

  iVar7 = DAT_0301e314;

  puVar6 = (undefined4 *)(DAT_0301e318 + -0x424);

  iVar8 = 0;

  if (*(uint *)(DAT_0301e314 + 4) <= *(uint *)(DAT_0301e318 + -0x834)) {

    return 2;

  }

  lVar1 = (ulonglong)*(uint *)(DAT_0301e318 + -0x834) * 0x660a;

  uVar3 = FUN_02ff6336((int)lVar1,(int)((ulonglong)lVar1 >> 0x20),*(undefined4 *)(DAT_0301e318 + 4),

                       0);

  uVar5 = *(uint *)(iVar2 + -0x410);

  if (*(uint *)(iVar7 + 0xc) < uVar3) {

    uVar3 = *(uint *)(iVar7 + 0xc);

  }

  if (uVar3 < uVar5 + 3) {

    if (uVar5 < uVar3 + 3) {

      return 2;

    }

    iVar7 = 0;

    if (0 < (int)(uVar5 - uVar3)) {

      do {

        iVar8 = FUN_0301dd9e(puVar6);

        iVar7 = iVar7 + 1;

      } while (iVar7 < (int)(uVar5 - uVar3));

    }

    *(undefined4 *)(iVar2 + -0x414) = 0;

    ape_rom_seek_thunk(-*(int *)(iVar2 + -0x418),1,*puVar6);

  }

  else {

    iVar7 = 0;

    if (0 < (int)(uVar3 - uVar5)) {

      do {

        iVar8 = ape_scan_cuesheet(puVar6);

        iVar7 = iVar7 + 1;

      } while (iVar7 < (int)(uVar3 - uVar5));

    }

    ape_rom_seek_thunk(*(undefined4 *)(iVar2 + -0x418),1,*puVar6);

  }

  if (iVar8 == 1) {

    return 0;

  }

  if (iVar8 == 2) {

    uVar4 = 1;

  }

  else {

    uVar4 = 0xffffffff;

  }

  return uVar4;

}
