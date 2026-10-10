/**
 * flac_decode_subframe @ 0x030ded62
 * Tags: codec, flac, decoder
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



void flac_decode_subframe

               (undefined4 param_1,int param_2,int param_3,undefined4 param_4,int *param_5,

               undefined4 param_6,uint param_7,undefined4 param_8)



{

  longlong lVar1;

  longlong lVar2;

  int *piVar3;

  int *piVar4;

  int *piVar5;

  uint uVar6;

  int iVar7;

  int iVar8;

  int iVar9;

  

  if ((int)param_7 < param_3) {

    uVar6 = param_7;

    do {

      piVar3 = (int *)(param_2 + (uVar6 - param_7) * 4);

      lVar1 = 0;

      lVar2 = 0;

      if (0 < (int)(param_7 - 1)) {

        piVar4 = piVar3 + -1;

        piVar5 = param_5 + -1;

        if ((param_7 & 1) == 0) {

          lVar2 = (longlong)*param_5 * (longlong)*piVar3;

          piVar4 = piVar3;

          piVar5 = param_5;

        }

        iVar8 = piVar5[1];

        iVar9 = piVar4[1];

        lVar1 = 0;

        for (iVar7 = (int)(param_7 - 1) >> 1; iVar7 != 0; iVar7 = iVar7 + -1) {

          lVar2 = (longlong)iVar8 * (longlong)iVar9 + lVar2;

          lVar1 = (longlong)piVar5[2] * (longlong)piVar4[2] + lVar1;

          iVar8 = piVar5[3];

          iVar9 = piVar4[3];

          piVar4 = piVar4 + 2;

          piVar5 = piVar5 + 2;

        }

        lVar1 = lVar2 + lVar1;

      }

      iVar7 = param_7 - 1;

      if (-1 < iVar7) {

        lVar1 = (longlong)param_5[iVar7] * (longlong)piVar3[iVar7] + lVar1;

      }

      iVar8 = *(int *)(param_2 + uVar6 * 4);

      iVar7 = FUN_030dd9d0((int)lVar1,(int)((ulonglong)lVar1 >> 0x20),param_8);

      *(int *)(param_2 + uVar6 * 4) = iVar8 + iVar7;

      uVar6 = uVar6 + 1;

    } while ((int)uVar6 < param_3);

    return;

  }

  return;

}
