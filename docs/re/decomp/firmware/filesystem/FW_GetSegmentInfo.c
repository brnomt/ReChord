/**
 * FW_GetSegmentInfo @ 0x030c910a
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 FW_GetSegmentInfo(int *param_1,int *param_2)



{

  uint uVar1;

  int iVar2;

  undefined4 uVar3;

  byte *pbVar4;

  int iVar5;

  int iVar6;

  int iVar7;

  int *piVar8;

  int iVar9;

  int iVar10;

  int iVar11;

  byte *pbVar12;

  uint uVar13;

  undefined1 auStack_188 [292];

  uint local_64;

  int local_60 [6];

  int local_48 [4];

  undefined4 local_38 [5];

  

  iVar10 = *param_1;

  iVar9 = *(int *)(iVar10 + 0x1c);

  local_64 = *(uint *)(iVar9 + param_1[0xc] * 4);

  iVar11 = 0;

  if (0 < *(int *)(iVar10 + 4)) {

    local_60[2] = (local_64 & 0x3fffffff) << 1;

    do {

      uVar1 = 0;

      if (1 < *param_2) {

        uVar1 = (uint)*(byte *)((int)param_2 + iVar11 + 4);

      }

      uVar1 = (uint)*(byte *)((int)param_2 + uVar1 * 2 + 6);

      if (*(char *)(*(int *)(iVar9 + 0x24) + uVar1) == '\0') {

        iVar2 = FUN_030c4c48(*(undefined4 *)(*(int *)(iVar9 + 0x28) + uVar1 * 4));

        if (0x24 < iVar2) {

          uVar3 = FUN_030c4c48(*(undefined4 *)(*(int *)(iVar9 + 0x28) + uVar1 * 4));

          FUN_030cd958(s_floormemo_need__d_030c93c4,uVar3);

        }

        local_48[iVar11] = (int)(auStack_188 + iVar11 * 0x90);

        iVar2 = SDSwitchFunction(param_1,*(undefined4 *)(*(int *)(iVar9 + 0x28) + uVar1 * 4));

      }

      else {

        iVar2 = FUN_030c5f38();

        if (0x24 < iVar2) {

          uVar3 = FUN_030c5f38(*(undefined4 *)(*(int *)(iVar9 + 0x28) + uVar1 * 4));

          FUN_030cd958(s_floormemo_need__d_030c93c4,uVar3);

        }

        local_48[iVar11] = (int)(auStack_188 + iVar11 * 0x90);

        iVar2 = FUN_030c5f3e(param_1,*(undefined4 *)(*(int *)(iVar9 + 0x28) + uVar1 * 4));

      }

      local_48[iVar11] = iVar2;

      if (iVar2 == 0) {

        local_60[iVar11] = 0;

      }

      else {

        local_60[iVar11] = 1;

      }

      memset_byte(*(undefined4 *)(param_1[7] + iVar11 * 4),0,local_60[2]);

      iVar11 = iVar11 + 1;

    } while (iVar11 < *(int *)(iVar10 + 4));

  }

  iVar11 = param_2[3];

  if (0 < iVar11) {

    pbVar12 = (byte *)(param_2 + 4);

    pbVar4 = (byte *)((int)param_2 + 0x11);

    do {

      if ((local_60[*pbVar12] != 0) || (local_60[*pbVar4] != 0)) {

        local_60[*pbVar12] = 1;

        local_60[*pbVar4] = 1;

      }

      pbVar4 = pbVar4 + 2;

      pbVar12 = pbVar12 + 2;

      iVar11 = iVar11 + -1;

    } while (iVar11 != 0);

  }

  uVar1 = 0;

  if (0 < *param_2) {

    do {

      iVar2 = *(int *)(iVar10 + 4);

      iVar11 = 0;

      if (0 < iVar2) {

        piVar8 = local_60;

        pbVar4 = (byte *)(param_2 + 1);

        iVar6 = 0;

        do {

          uVar13 = 0;

          if (param_2 != (int *)0xfffffffc) {

            uVar13 = (uint)*pbVar4;

          }

          if (param_2 == (int *)0xfffffffc || uVar13 == uVar1) {

            if (*piVar8 == 0) {

              local_38[iVar11] = 0;

            }

            else {

              local_38[iVar11] = 1;

            }

            local_38[iVar11 + 2] = *(undefined4 *)(param_1[7] + iVar6 * 4);

            iVar11 = iVar11 + 1;

          }

          piVar8 = piVar8 + 1;

          iVar6 = iVar6 + 1;

          pbVar4 = pbVar4 + 1;

          iVar2 = iVar2 + -1;

        } while (iVar2 != 0);

      }

      FUN_030caeda(param_1,*(int *)(iVar9 + 0x2c) +

                           (uint)*(byte *)((int)param_2 + uVar1 * 2 + 7) * 0x1c,local_38 + 2,

                   local_38,iVar11);

      uVar1 = uVar1 + 1;

    } while ((int)uVar1 < *param_2);

  }

  uVar1 = param_2[3] - 1;

  if (-1 < (int)uVar1) {

    do {

      iVar6 = *(int *)(param_1[7] + (uint)*(byte *)((int)param_2 + uVar1 * 2 + 0x10) * 4);

      iVar2 = *(int *)(param_1[7] + (uint)*(byte *)((int)param_2 + uVar1 * 2 + 0x11) * 4);

      for (iVar11 = 0; iVar11 < (int)local_64 / 2; iVar11 = iVar11 + 1) {

        iVar5 = *(int *)(iVar6 + iVar11 * 4);

        iVar7 = *(int *)(iVar2 + iVar11 * 4);

        if (iVar5 < 1) {

          if (iVar7 < 1) {

            *(int *)(iVar2 + iVar11 * 4) = iVar5;

            iVar7 = -iVar7;

            goto LAB_030c9320;

          }

LAB_030c930c:

          *(int *)(iVar2 + iVar11 * 4) = iVar5 + iVar7;

        }

        else {

          if (0 < iVar7) {

            iVar7 = -iVar7;

            goto LAB_030c930c;

          }

          *(int *)(iVar2 + iVar11 * 4) = iVar5;

LAB_030c9320:

          *(int *)(iVar6 + iVar11 * 4) = iVar5 + iVar7;

        }

      }

      uVar1 = uVar1 - 1;

    } while (uVar1 < 0x80000000);

  }

  iVar11 = *(int *)(iVar10 + 4);

  iVar2 = 0;

  if (0 < iVar11) {

    do {

      uVar1 = 0;

      if (1 < *param_2) {

        uVar1 = (uint)*(byte *)((int)param_2 + iVar2 + 4);

      }

      uVar1 = (uint)*(byte *)((int)param_2 + uVar1 * 2 + 6);

      if (*(char *)(*(int *)(iVar9 + 0x24) + uVar1) == '\0') {

        FUN_030c4d30(param_1,*(undefined4 *)(*(int *)(iVar9 + 0x28) + uVar1 * 4),local_48[iVar2],

                     *(undefined4 *)(param_1[7] + iVar2 * 4));

      }

      else {

        FUN_030c614a();

      }

      iVar11 = *(int *)(iVar10 + 4);

      iVar2 = iVar2 + 1;

    } while (iVar2 < iVar11);

  }

  iVar9 = 0;

  if (0 < iVar11) {

    do {

      FW_ReadFirmwaveByByte(local_64,*(undefined4 *)(param_1[7] + iVar9 * 4));

      iVar9 = iVar9 + 1;

    } while (iVar9 < *(int *)(iVar10 + 4));

  }

  iVar10 = *(int *)(iVar10 + 4);

  iVar9 = 0;

  if (0 < iVar10) {

    do {

      local_48[iVar9] = 0;

      iVar9 = iVar9 + 1;

    } while (iVar9 < iVar10);

  }

  return 0;

}
