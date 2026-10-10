/**
 * config_parse_helper2 @ 0x030e6246
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



void config_parse_helper2

               (int param_1,int param_2,int param_3,int param_4,int param_5,undefined4 param_6,

               int param_7,uint param_8)



{

  int iVar1;

  undefined4 uVar2;

  int iVar3;

  int iVar4;

  int extraout_r3;

  int iVar5;

  int iVar6;

  int iVar7;

  int iVar8;

  int iVar9;

  int iVar10;

  bool bVar11;

  int aiStack_b8 [32];

  int iStack_38;

  int iStack_34;

  int iStack_30;

  int local_2c;

  int iStack_28;

  

  iVar5 = param_7 + 1;

  if (iVar5 < param_3) {

    iStack_38 = 1 << (param_8 - 1 & 0xff);

    iStack_34 = param_1;

    iStack_30 = param_2;

    local_2c = param_3;

    iStack_28 = param_4;

    do {

      iVar4 = *(int *)(param_1 + iVar5 * 4);

      iVar10 = 0;

      if (iVar4 < 0) {

        param_4 = -1;

      }

      if (0 < iVar4) {

        param_4 = 1;

      }

      if (iVar4 == 0) {

        param_4 = 0;

      }

      iVar9 = param_1 + (iVar5 - param_7) * 4;

      iVar3 = 0;

      if (0 < param_7) {

        do {

          iVar6 = *(int *)(iVar9 + -4);

          bVar11 = param_4 != 0;

          iVar1 = *(int *)(param_1 + ((iVar5 - param_7) + iVar3) * 4) - iVar6;

          aiStack_b8[iVar3] = iVar1;

          iVar8 = *(int *)(param_5 + iVar3 * 4);

          iVar10 = iVar1 * iVar8 + iVar10;

          iVar7 = 0;

          if (bVar11) {

            iVar7 = iVar1;

          }

          if (bVar11 && iVar1 != 0) {

            iVar7 = iVar4 * param_4;

            iVar6 = iVar7;

          }

          if (((bVar11 && iVar1 != 0) && iVar6 != 0) && -1 < iVar7) {

            iVar7 = -iVar1;

            if (iVar7 < 0) {

              iVar1 = -1;

            }

            if (0 < iVar7) {

              iVar1 = 1;

            }

            if (iVar7 == 0) {

              iVar1 = 0;

            }

            iVar4 = iVar4 - (iVar7 * iVar1 * param_4 >> (param_8 & 0xff)) * (iVar3 + 1);

            *(int *)(param_5 + iVar3 * 4) = iVar8 - iVar1 * param_4;

          }

          iVar3 = iVar3 + 1;

        } while (iVar3 < param_7);

      }

      uVar2 = FUN_030e576a((iStack_38 + iVar10 >> (param_8 & 0xff)) +

                           *(int *)(iVar9 + -4) + *(int *)(param_1 + iVar5 * 4),iStack_28);

      *(undefined4 *)(param_1 + iVar5 * 4) = uVar2;

      *(undefined4 *)(iStack_30 + iVar5 * 4) = uVar2;

      iVar5 = iVar5 + 1;

      param_4 = extraout_r3;

    } while (iVar5 < local_2c);

    return;

  }

  return;

}
