/**
 * flac_lpc_compute @ 0x030ddd0a
 * Tags: codec, flac, lpc
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined8 flac_lpc_compute(uint param_1,uint param_2,uint param_3,uint param_4)



{

  uint uVar1;

  uint uVar2;

  uint uVar3;

  uint uVar4;

  uint uVar5;

  uint uVar6;

  int iVar7;

  int iVar8;

  uint uVar9;

  bool bVar10;

  

  if ((param_1 != 0 || (param_2 & 0x7fffffff) != 0) && (param_3 != 0 || (param_4 & 0x7fffffff) != 0)

     ) {

    uVar5 = param_4 & 0xfffff | 0x100000;

    uVar4 = param_2 & 0xfffff | 0x100000;

    iVar7 = ((param_2 & 0x7fffffff) >> 0x14) - ((param_4 & 0x7fffffff) >> 0x14);

    iVar8 = iVar7 + 0x3fd;

    if (uVar5 < uVar4 || uVar4 - uVar5 < (uint)(param_3 <= param_1)) {

      iVar8 = iVar7 + 0x3fe;

    }

    else {

      bVar10 = CARRY4(param_1,param_1);

      param_1 = param_1 * 2;

      uVar4 = uVar4 * 2 + (uint)bVar10;

    }

    if (-1 < iVar8) {

      uVar3 = 0x100000;

      uVar6 = 0;

      uVar9 = 0;

      for (uVar2 = 0; uVar2 != 0 || uVar3 != 0; uVar2 = (uint)(uVar1 != 0) << 0x1f | uVar2 >> 1) {

        if (uVar5 < uVar4 || uVar4 - uVar5 < (uint)(param_3 <= param_1)) {

          bVar10 = param_1 < param_3;

          param_1 = param_1 - param_3;

          uVar4 = (uVar4 - uVar5) - (uint)bVar10;

          uVar6 = uVar6 | uVar2;

          uVar9 = uVar9 | uVar3;

        }

        uVar1 = uVar3 & 1;

        uVar3 = uVar3 >> 1;

        bVar10 = CARRY4(param_1,param_1);

        param_1 = param_1 * 2;

        uVar4 = uVar4 * 2 + (uint)bVar10;

      }

      if (param_1 != 0 || uVar4 != 0) {

        if (param_1 == param_3 && uVar4 == uVar5) {

          param_1 = 0;

          uVar4 = 0x80000000;

        }

        else if (uVar5 < uVar4 || uVar4 - uVar5 < (uint)(param_3 <= param_1)) {

          param_1 = 0xfffffffe;

          uVar4 = 0xffffffff;

        }

        else {

          param_1 = 1;

          uVar4 = 0;

        }

      }

      iVar7 = uVar9 + iVar8 * 0x100000 + ((param_2 ^ param_4) & 0x80000000);

      if ((int)uVar4 < 0) {

        bVar10 = 0xfffffffe < uVar6;

        uVar6 = uVar6 + 1;

        iVar7 = iVar7 + (uint)bVar10;

        if ((param_1 & 0x7fffffff) == 0 && uVar4 * 2 + (uint)CARRY4(param_1,param_1) == 0) {

          uVar6 = uVar6 & 0xfffffffe;

        }

      }

      return CONCAT44(iVar7,uVar6);

    }

  }

  return 0;

}
