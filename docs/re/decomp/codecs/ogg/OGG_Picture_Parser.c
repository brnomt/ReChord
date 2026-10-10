/**
 * OGG_Picture_Parser @ 0x030241f8
 * Introduced in v3.5.0 (changelog: album sorting / Favorites)
 * Named via cross-version lineage; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 OGG_Picture_Parser(uint param_1,uint param_2)



{

  uint *puVar1;

  uint *puVar2;

  int iVar3;

  int iVar4;

  uint uVar5;

  

  FUN_03021c54();

  if (*DAT_030243d8 != 0) {

    param_1 = FUN_02ffc6f6(&DAT_03023ae4,s_PICTURE_OGG_03023ad8,DAT_030243dc);

    *DAT_030243e0 = (char)param_1;

    if ((param_1 == 0xffffffff) || (8 < param_1)) {

      return 0;

    }

  }

  if (param_2 < 2) {

    FUN_0300c0e0(0x13,7);

    puVar1 = DAT_030243e4;

    puVar2 = DAT_030243e4 + -0x4a0;

    *DAT_030243e4 = param_1;

    puVar1[1] = 0;

    puVar1[2] = (uint)puVar2;

    puVar1[9] = 0;

    puVar1[10] = (uint)(puVar1 + -0xa0);

    if ((param_1 != 0xffffffff) && (iVar3 = FUN_03026ccc(puVar1), puVar2 = DAT_030243ec, iVar3 == 0)

       ) {

      uVar5 = *DAT_030243e8;

      puVar1[3] = uVar5;

      puVar1[4] = *puVar2;

      if (((int)uVar5 < 0x141) && (iVar3 = my_bui_clz(), iVar3 == 0)) {

        iVar4 = (int)*DAT_030243f0;

        iVar3 = (int)*DAT_030243f4;

        if (iVar3 < iVar4) {

          *DAT_030243f8 = *DAT_030243f8 + (0x7e - (iVar3 * 0x7e) / iVar4) / 2;

        }

        else if (iVar4 < iVar3) {

          *DAT_030243fc = *DAT_030243fc + (0x7e - (iVar4 * 0x7e) / iVar3) / 2;

        }

        iVar3 = DAT_03024400;

        if (*(int *)(DAT_03024400 + 8) == 0) {

          uVar5 = (int)(puVar1[4] - puVar1[6]) >> 1;

          puVar1[0xb] = uVar5;

        }

        else {

          uVar5 = 0;

          puVar1[0xb] = 0;

        }

        *(uint *)(iVar3 + 0xc) = uVar5;

        return 1;

      }

    }

  }

  return 0;

}
