/**
 * FatDev_FileSeek @ 0x030c3a52
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 FatDev_FileSeek(uint *param_1,int param_2,uint *param_3,int param_4)



{

  uint uVar1;

  ushort *puVar2;

  uint uVar3;

  uint uVar4;

  int iVar5;

  uint uVar6;

  uint *puVar7;

  uint uVar8;

  uint uVar9;

  int iVar10;

  

  uVar1 = FatCheckSecContinue();

  if (*(int *)(param_2 + 8) < 0) {

    return 0xffffffff;

  }

  uVar4 = param_1[7];

  uVar3 = *param_1;

  iVar10 = (int)uVar3 >> 1;

  if (uVar4 == 1) {

    uVar4 = param_1[0xd];

    uVar8 = (1 << (uVar4 & 0xff)) - 1;

    if (0 < (int)uVar3) {

      puVar7 = param_3 + -1;

      iVar5 = iVar10;

      if ((uVar3 & 1) != 0) {

        *param_3 = uVar1 & uVar8;

        uVar1 = uVar1 >> (uVar4 & 0xff);

        puVar7 = param_3;

      }

      for (; iVar5 != 0; iVar5 = iVar5 + -1) {

        uVar9 = uVar1 >> (uVar4 & 0xff);

        puVar7[1] = uVar1 & uVar8;

        uVar1 = uVar9 >> (uVar4 & 0xff);

        puVar7[2] = uVar9 & uVar8;

        puVar7 = puVar7 + 2;

      }

    }

  }

  else if (uVar4 == 2) {

    uVar9 = param_1[0xe];

    uVar8 = param_1[0xf];

    uVar4 = (1 << (uVar9 & 0xff)) - 1;

    if ((int)param_1[0xd] < 9) {

      if (0 < (int)uVar3) {

        puVar7 = param_3 + -1;

        iVar5 = iVar10;

        if ((uVar3 & 1) != 0) {

          uVar6 = uVar1 & uVar4;

          uVar1 = uVar1 >> (uVar9 & 0xff);

          *param_3 = (uint)*(byte *)(uVar8 + uVar6);

          puVar7 = param_3;

        }

        for (; iVar5 != 0; iVar5 = iVar5 + -1) {

          uVar6 = uVar1 >> (uVar9 & 0xff);

          puVar7[1] = (uint)*(byte *)(uVar8 + (uVar1 & uVar4));

          uVar1 = uVar6 >> (uVar9 & 0xff);

          puVar7[2] = (uint)*(byte *)(uVar8 + (uVar6 & uVar4));

          puVar7 = puVar7 + 2;

        }

      }

    }

    else if (0 < (int)uVar3) {

      puVar7 = param_3 + -1;

      iVar5 = iVar10;

      if ((uVar3 & 1) != 0) {

        uVar6 = uVar1 & uVar4;

        uVar1 = uVar1 >> (uVar9 & 0xff);

        *param_3 = (uint)*(ushort *)(uVar8 + uVar6 * 2);

        puVar7 = param_3;

      }

      for (; iVar5 != 0; iVar5 = iVar5 + -1) {

        uVar6 = uVar1 >> (uVar9 & 0xff);

        puVar7[1] = (uint)*(ushort *)(uVar8 + (uVar1 & uVar4) * 2);

        uVar1 = uVar6 >> (uVar9 & 0xff);

        puVar7[2] = (uint)*(ushort *)(uVar8 + (uVar6 & uVar4) * 2);

        puVar7 = puVar7 + 2;

      }

    }

  }

  else {

    if (uVar4 != 3) {

      return 0xffffffff;

    }

    puVar2 = (ushort *)(param_1[0xf] + param_1[0xe] * uVar1 * 4);

    iVar5 = 0;

    if ((int)param_1[0xd] < 9) {

      puVar7 = param_3;

      if (0 < (int)uVar3) {

        do {

          iVar5 = iVar5 + 1;

          *puVar7 = (uint)(byte)*puVar2;

          puVar2 = (ushort *)((int)puVar2 + 1);

          puVar7 = puVar7 + 1;

        } while (iVar5 < (int)uVar3);

      }

    }

    else {

      puVar7 = param_3;

      if (0 < (int)uVar3) {

        do {

          iVar5 = iVar5 + 1;

          *puVar7 = (uint)*puVar2;

          puVar2 = puVar2 + 1;

          puVar7 = puVar7 + 1;

        } while (iVar5 < (int)uVar3);

      }

    }

  }

  uVar1 = param_4 - param_1[0xb];

  uVar4 = param_4 - param_1[9];

  if ((int)uVar4 < 1) {

    iVar5 = param_1[8] << (-uVar4 & 0xff);

  }

  else {

    iVar5 = (int)param_1[8] >> (uVar4 & 0xff);

  }

  if ((int)uVar1 < 1) {

    uVar1 = -uVar1;

    uVar4 = param_1[10];

    if (0 < (int)uVar3) {

      puVar7 = param_3 + -1;

      if ((uVar3 & 1) != 0) {

        *param_3 = (*param_3 * uVar4 << (uVar1 & 0xff)) + iVar5;

        puVar7 = param_3;

      }

      for (; iVar10 != 0; iVar10 = iVar10 + -1) {

        puVar7[1] = (puVar7[1] * uVar4 << (uVar1 & 0xff)) + iVar5;

        puVar7[2] = (puVar7[2] * uVar4 << (uVar1 & 0xff)) + iVar5;

        puVar7 = puVar7 + 2;

      }

    }

  }

  else {

    uVar4 = param_1[10];

    if (0 < (int)uVar3) {

      puVar7 = param_3 + -1;

      if ((uVar3 & 1) != 0) {

        *param_3 = ((int)(*param_3 * uVar4) >> (uVar1 & 0xff)) + iVar5;

        puVar7 = param_3;

      }

      for (; iVar10 != 0; iVar10 = iVar10 + -1) {

        puVar7[1] = ((int)(uVar4 * puVar7[1]) >> (uVar1 & 0xff)) + iVar5;

        puVar7[2] = ((int)(uVar4 * puVar7[2]) >> (uVar1 & 0xff)) + iVar5;

        puVar7 = puVar7 + 2;

      }

    }

  }

  if (param_1[0xc] != 0) {

    puVar7 = param_3 + 1;

    uVar1 = *param_3;

    if (0 < (int)(uVar3 - 1)) {

      if ((uVar3 & 1) == 0) {

        *puVar7 = *puVar7 + uVar1;

        uVar1 = *puVar7;

        param_3 = puVar7;

      }

      for (iVar10 = (int)(uVar3 - 1) >> 1; iVar10 != 0; iVar10 = iVar10 + -1) {

        uVar3 = param_3[1];

        param_3[1] = uVar1 + uVar3;

        puVar7 = param_3 + 2;

        param_3 = param_3 + 2;

        *param_3 = uVar1 + uVar3 + *puVar7;

        uVar1 = *param_3;

      }

    }

  }

  return 0;

}
