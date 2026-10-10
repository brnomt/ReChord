/**
 * softfloat_dadd_a @ 0x030f6a66
 * Tags: os, softfloat
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint softfloat_dadd_a(uint param_1,uint param_2,uint param_3,uint param_4)



{

  int iVar1;

  int iVar2;

  uint uVar3;

  int iVar4;

  uint uVar5;

  uint uVar6;

  uint uVar7;

  uint uVar8;

  uint uVar9;

  bool bVar10;

  ulonglong uVar11;

  longlong lVar12;

  

  iVar1 = (int)(param_2 ^ param_4) >> 0x1f;

  iVar2 = -iVar1;

  uVar7 = param_3;

  uVar9 = param_4;

  if ((param_2 & 0x7fffffff) <= (param_4 & 0x7fffffff) &&

      (uint)(param_3 <= param_1) <= (param_2 & 0x7fffffff) - (param_4 & 0x7fffffff)) {

    uVar7 = param_1;

    uVar9 = param_2;

    param_2 = param_4;

    param_1 = param_3;

  }

  if ((uVar9 & 0x7fffffff) == 0 && uVar7 == 0) {

    return param_1;

  }

  uVar8 = param_2 >> 0x14;

  uVar3 = uVar8 & 0x7ff;

  iVar4 = uVar3 - ((uVar9 & 0x7fffffff) >> 0x14);

  if (iVar4 < 0x40) {

    uVar9 = uVar9 & 0xfffff | 0x100000;

    if (iVar2 != 0) {

      bVar10 = uVar7 != 0;

      uVar7 = -uVar7;

      uVar9 = -(uint)bVar10 - uVar9;

    }

    uVar11 = FUN_030f6d28(uVar7,uVar9,0x40 - iVar4);

    uVar6 = (uint)(uVar11 >> 0x20);

    uVar5 = (uint)uVar11;

    lVar12 = FUN_030f6d46(uVar7,uVar9,iVar4);

    lVar12 = lVar12 + CONCAT44(param_2,param_1);

    param_1 = (uint)lVar12;

    uVar7 = (uint)((ulonglong)lVar12 >> 0x20);

    if ((param_2 ^ uVar7) >> 0x14 != 0) {

      if (iVar2 == 0) {

        uVar11 = CONCAT44(uVar6 >> 1 | param_1 * -0x80000000,

                          (uint)((uVar11 & 0x100000000) != 0) << 0x1f | uVar5 >> 1);

        param_1 = (uint)((uVar7 + 0x100000 & 1) != 0) << 0x1f | param_1 >> 1;

      }

      else {

        if (iVar4 < 2) {

          uVar7 = FUN_030f69c2(param_1,uVar7 + uVar8 * -0x100000 + 0x100000,uVar5,uVar6,0,

                               (uVar8 & 0x800) << 0x14,uVar3);

          return uVar7;

        }

        param_1 = param_1 * 2 | uVar6 >> 0x1f;

        uVar11 = CONCAT44(uVar6 * 2 + (uint)CARRY4(uVar5,uVar5),uVar5 * 2);

      }

    }

  }

  else {

    uVar11 = CONCAT44(-(uint)(1 < (uint)(iVar1 * -2)),iVar1 * 2 + 1);

    param_1 = param_1 + iVar1;

  }

  if (((longlong)uVar11 < 0) &&

     (param_1 = param_1 + 1,

     (uVar11 & 0x7fffffff) == 0 &&

     (int)(uVar11 >> 0x20) * 2 + (uint)CARRY4((uint)uVar11,(uint)uVar11) == 0)) {

    param_1 = param_1 & 0xfffffffe;

  }

  return param_1;

}
