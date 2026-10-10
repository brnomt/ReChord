/**
 * softfloat_dmul_b @ 0x030fe3c2
 * Tags: os, softfloat
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void softfloat_dmul_b(uint param_1,uint param_2,uint param_3,uint param_4)



{

  longlong lVar1;

  uint uVar2;

  uint uVar3;

  uint uVar4;

  int iVar5;

  uint uVar6;

  uint uVar7;

  uint uVar8;

  

  if ((param_1 != 0 || (param_2 & 0x7fffffff) != 0) && (param_3 != 0 || (param_4 & 0x7fffffff) != 0)

     ) {

    uVar6 = (uint)((ulonglong)param_1 * (ulonglong)param_3);

    uVar2 = param_2 & 0xfffff | 0x100000;

    uVar3 = param_4 & 0xfffff | 0x100000;

    uVar4 = param_1 >> 10 | param_2 << 0x16;

    uVar8 = param_3 >> 10 | param_4 << 0x16;

    lVar1 = (ulonglong)uVar4 * (ulonglong)uVar8;

    uVar7 = (uint)lVar1;

    iVar5 = uVar4 * (uVar3 >> 10) + (uVar2 >> 10) * uVar8 + (int)((ulonglong)lVar1 >> 0x20);

    uVar8 = ((param_1 * uVar3 +

             uVar2 * param_3 + (int)((ulonglong)param_1 * (ulonglong)param_3 >> 0x20)) -

            (iVar5 * 0x100000 | uVar7 >> 0xc)) - (uint)(uVar6 < uVar7 * 0x100000);

    uVar4 = param_1 >> 0x1a | uVar2 << 6;

    uVar2 = param_3 >> 0x1a | uVar3 << 6;

    uVar3 = uVar6 + uVar7 * -0x100000 >> 0x14 | uVar8 * 0x1000;

    lVar1 = (ulonglong)uVar4 * (ulonglong)uVar2 +

            (ulonglong)((iVar5 - uVar4 * uVar2) + (uVar8 >> 0x14) + (uint)CARRY4(uVar3,uVar7));

    FUN_030fe762((int)lVar1,(int)((ulonglong)lVar1 >> 0x20),uVar6 * 0x1000,uVar3 + uVar7,0,

                 (param_2 ^ param_4) & 0x80000000,

                 ((param_4 & 0x7fffffff) >> 0x14) + ((param_2 & 0x7fffffff) >> 0x14) + -0x3ff);

  }

  return;

}
