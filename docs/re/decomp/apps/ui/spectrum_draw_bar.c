/**
 * spectrum_draw_bar @ 0x03027d08
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4

spectrum_draw_bar(int param_1,int param_2,int param_3,int param_4,int param_5,short param_6,

                 short param_7)



{

  ushort uVar1;

  undefined4 uVar2;

  int iVar3;

  uint uVar4;

  int iVar5;

  uint uVar6;

  uint uVar7;

  int iVar8;

  uint *puVar9;

  int iVar10;

  

  iVar3 = (int)(short)((short)*(undefined4 *)(DAT_03027e38 + 0x40) * param_7 + param_6);

  iVar5 = (uint)(param_5 << 0x10) / (uint)(int)*(short *)(DAT_03027e38 + 8) + 1;

  uVar7 = (uint)(iVar3 * iVar5) >> 0x10;

  puVar9 = (uint *)(param_1 + ((uint)(*(int *)(DAT_03027e38 + 0xc) - param_4) >> 1) * 4);

  if ((uVar7 == (uint)(iVar5 * (iVar3 + 1)) >> 0x10) &&

     (uVar7 != *(int *)(DAT_03027e38 + 0x10) - 1U)) {

    uVar2 = FUN_03027dfa(1);

    return uVar2;

  }

  for (uVar7 = 0; (int)uVar7 < param_4; uVar7 = uVar7 + 1) {

    uVar1 = *(ushort *)(param_3 + (uVar7 & 0xfffffffe));

    iVar10 = *(byte *)(param_2 + uVar7) - 0x10;

    iVar3 = (uVar1 >> 8) - 0x80;

    iVar5 = (byte)uVar1 - 0x80;

    iVar8 = (iVar5 * 0x409 + iVar10 * 0x254 >> 9) + 4;

    uVar6 = (iVar3 * 0x331 + iVar10 * 0x254 >> 9) + 4;

    uVar4 = (iVar10 * 0x254 + iVar3 * -0x1a0 + iVar5 * -200 >> 9) + 4;

    if (iVar8 < 0x100) {

      if (iVar8 < 0) {

        iVar8 = 0;

      }

    }

    else {

      iVar8 = 0xff;

    }

    if ((int)uVar4 < 0x100) {

      if ((int)uVar4 < 0) {

        uVar4 = 0;

      }

    }

    else {

      uVar4 = 0xff;

    }

    if ((int)uVar6 < 0x100) {

      if ((int)uVar6 < 0) {

        uVar6 = 0;

      }

    }

    else {

      uVar6 = 0xff;

    }

    *puVar9 = iVar8 << 0x18 | (uVar4 & 0xff) << 0x10 | (uVar6 & 0xff) << 8;

    puVar9 = puVar9 + 1;

  }

  return 0;

}
