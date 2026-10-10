/**
 * FW_Ansi2Unicode @ 0x030c4dec
 * SDK-matched (structural+combined, documented in MULTI-VERSION-PLAN §10).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void FW_Ansi2Unicode(undefined1 *param_1,int param_2,uint param_3)



{

  int iVar1;

  uint uVar2;

  undefined1 *puVar3;

  uint uVar4;

  undefined1 *puVar5;

  int iVar6;

  int iVar7;

  undefined1 *puVar8;

  uint uVar9;

  uint uVar10;

  uint uVar11;

  undefined1 *puVar12;

  undefined1 *puVar13;

  bool bVar14;

  undefined1 local_430 [1024];

  undefined1 *local_30;

  int iStack_2c;

  uint uStack_28;

  

  uStack_28 = param_3;

  iStack_2c = param_2;

  local_30 = param_1;

  puVar8 = local_430;

  uVar9 = 1;

  puVar5 = param_1;

  puVar3 = local_430;

  if (1 < param_3) {

    do {

      param_1 = puVar3;

      puVar8 = puVar5;

      uVar2 = 0;

      uVar4 = uVar9;

      while (uVar4 < param_3) {

        iVar6 = uVar2 + uVar9;

        uVar10 = uVar2 + uVar9 * 2;

        iVar7 = iVar6;

        uVar4 = uVar2;

        if (param_3 <= uVar10) {

          uVar10 = param_3;

        }

        while( true ) {

          bVar14 = SBORROW4(uVar4,iVar6);

          iVar1 = uVar4 - iVar6;

          if ((int)uVar4 < iVar6) {

            bVar14 = SBORROW4(iVar7,uVar10);

            iVar1 = iVar7 - uVar10;

          }

          if (iVar1 < 0 == bVar14) break;

          if (*(ushort *)(param_2 + (uint)(byte)puVar8[uVar4] * 2) <

              *(ushort *)(param_2 + (uint)(byte)puVar8[iVar7] * 2)) {

            param_1[uVar2] = puVar8[uVar4];

            uVar2 = uVar2 + 1 & 0xffff;

            uVar4 = uVar4 + 1;

          }

          else {

            param_1[uVar2] = puVar8[iVar7];

            uVar2 = uVar2 + 1 & 0xffff;

            iVar7 = iVar7 + 1;

          }

        }

        uVar11 = iVar6 - uVar4;

        if (0 < (int)uVar11) {

          puVar12 = puVar8 + uVar4;

          puVar13 = param_1 + uVar2;

          puVar5 = puVar12 + -1;

          puVar3 = puVar13 + -1;

          if ((uVar11 & 1) != 0) {

            *puVar13 = *puVar12;

            puVar5 = puVar12;

            puVar3 = puVar13;

          }

          for (iVar6 = (int)uVar11 >> 1; iVar6 != 0; iVar6 = iVar6 + -1) {

            puVar3[1] = puVar5[1];

            puVar5 = puVar5 + 2;

            puVar3 = puVar3 + 2;

            *puVar3 = *puVar5;

          }

          uVar2 = uVar2 + uVar11 & 0xffff;

        }

        uVar10 = uVar10 - iVar7;

        if (0 < (int)uVar10) {

          puVar12 = puVar8 + iVar7;

          puVar13 = param_1 + uVar2;

          puVar3 = puVar13 + -1;

          puVar5 = puVar12 + -1;

          if ((uVar10 & 1) != 0) {

            *puVar13 = *puVar12;

            puVar3 = puVar13;

            puVar5 = puVar12;

          }

          for (iVar7 = (int)uVar10 >> 1; iVar7 != 0; iVar7 = iVar7 + -1) {

            puVar3[1] = puVar5[1];

            puVar5 = puVar5 + 2;

            puVar3 = puVar3 + 2;

            *puVar3 = *puVar5;

          }

          uVar2 = uVar2 + uVar10 & 0xffff;

        }

        uVar4 = uVar2 + uVar9;

      }

      uVar4 = param_3 - uVar2;

      if (0 < (int)uVar4) {

        puVar13 = puVar8 + uVar2;

        puVar12 = param_1 + uVar2;

        puVar3 = puVar12 + -1;

        puVar5 = puVar13 + -1;

        if ((uVar4 & 1) != 0) {

          *puVar12 = *puVar13;

          puVar3 = puVar12;

          puVar5 = puVar13;

        }

        for (uVar2 = (uVar4 & 0x1ffff) >> 1; uVar2 != 0; uVar2 = uVar2 - 1 & 0xffff) {

          puVar3[1] = puVar5[1];

          puVar5 = puVar5 + 2;

          puVar3 = puVar3 + 2;

          *puVar3 = *puVar5;

        }

      }

      uVar9 = (uVar9 & 0x7fff) << 1;

      puVar5 = param_1;

      puVar3 = puVar8;

    } while (uVar9 < param_3);

  }

  if (puVar8 == local_30) {

    if (param_3 != 0) {

      puVar3 = puVar8 + -1;

      puVar5 = param_1 + -1;

      if ((param_3 & 1) != 0) {

        *puVar8 = *param_1;

        puVar3 = puVar8;

        puVar5 = param_1;

      }

      for (param_3 = param_3 >> 1; param_3 != 0; param_3 = param_3 - 1) {

        puVar3[1] = puVar5[1];

        puVar5 = puVar5 + 2;

        puVar3 = puVar3 + 2;

        *puVar3 = *puVar5;

      }

    }

    return;

  }

  return;

}
