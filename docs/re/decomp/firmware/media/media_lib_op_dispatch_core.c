/**
 * media_lib_op_dispatch_core @ 0x03029246
 * Introduced in v3.5.0 (changelog: album sorting / Favorites)
 * Named via cross-version lineage; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4

media_lib_op_dispatch_core

          (int param_1,undefined4 param_2,undefined4 param_3,int param_4,int *param_5,

          undefined4 *param_6,undefined4 *param_7)



{

  undefined2 uVar1;

  undefined4 uVar2;

  int iVar3;

  int iVar4;

  int iVar5;

  int iVar6;

  uint uVar7;

  uint uVar8;

  int iVar9;

  int iVar10;

  uint uStack_50;

  uint uStack_4c;

  uint uStack_48;

  undefined4 uStack_44;

  undefined4 uStack_40;

  undefined4 uStack_3c;

  undefined4 uStack_38;

  

  iVar6 = param_5[6];

  if ((uint)param_6[4] < (uint)param_5[7]) {

    param_5[0xe] = 0;

    iVar3 = param_5[4];

    (*(code *)*param_7)(*param_5,1,param_5[3],*param_6);

    for (iVar10 = 0; iVar10 < param_5[6]; iVar10 = iVar10 + 1) {

      switch(param_3) {

      case 0:

        iVar4 = param_5[0xe];

        iVar9 = iVar4 >> 0x10;

        uVar2 = FUN_02ffb850(iVar9);

        FUN_02ffb4ec(uVar2,param_5[2]);

        iVar5 = FUN_02ffb89e();

        uVar7 = (uint)*(byte *)(*param_5 + iVar5);

        switch(iVar9 + ((int)(iVar9 + ((uint)(iVar4 >> 0x1f) >> 0x1d)) >> 3) * -8 & 0xff) {

        case 0:

          uVar7 = (uint)(*(byte *)(*param_5 + iVar5) >> 7);

          break;

        case 1:

          uVar7 = (uVar7 & 0x7f) >> 6;

          break;

        case 2:

          uVar7 = (uVar7 & 0x3f) >> 5;

          break;

        case 3:

          uVar7 = (uVar7 & 0x1f) >> 4;

          break;

        case 4:

          uVar7 = (uVar7 & 0xf) >> 3;

          break;

        case 5:

          uVar7 = (uVar7 & 7) >> 2;

          break;

        case 6:

          uVar7 = (uVar7 & 3) >> 1;

          break;

        case 7:

          uVar7 = uVar7 & 1;

        }

        uStack_50 = (uint)*(byte *)(*(int *)(param_4 + 0x38) + uVar7 * 4 + 2);

        uStack_4c = (uint)*(byte *)(*(int *)(param_4 + 0x38) + uVar7 * 4 + 1);

        uStack_48 = (uint)*(byte *)(*(int *)(param_4 + 0x38) + uVar7 * 4);

        break;

      case 1:

        iVar9 = param_5[0xe] >> 0x10;

        uVar7 = iVar9 + (iVar9 - (param_5[0xe] >> 0x1f) >> 1) * -2 & 0xff;

        uVar2 = FUN_02ffb850();

        FUN_02ffb4ec(uVar2,param_5[2]);

        iVar9 = FUN_02ffb89e();

        uVar8 = (uint)*(byte *)(*param_5 + iVar9);

        if (uVar7 == 0) {

          uVar8 = (uint)(*(byte *)(*param_5 + iVar9) >> 4);

        }

        else if (uVar7 == 1) {

          uVar8 = uVar8 & 0xf;

        }

        uStack_50 = (uint)*(byte *)(*(int *)(param_4 + 0x38) + uVar8 * 4 + 2);

        uStack_4c = (uint)*(byte *)(*(int *)(param_4 + 0x38) + uVar8 * 4 + 1);

        uStack_48 = (uint)*(byte *)(*(int *)(param_4 + 0x38) + uVar8 * 4);

        break;

      case 2:

        uVar7 = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10));

        uStack_50 = (uint)*(byte *)(*(int *)(param_4 + 0x38) + uVar7 * 4 + 2);

        uStack_4c = (uint)*(byte *)(*(int *)(param_4 + 0x38) + uVar7 * 4 + 1);

        uStack_48 = (uint)*(byte *)(*(int *)(param_4 + 0x38) + uVar7 * 4);

        break;

      case 3:

        uVar7 = (uint)*(ushort *)(*param_5 + (param_5[0xe] >> 0x10) * 2);

        uStack_50 = (int)(uVar7 & 0x7c00) >> 7;

        uStack_4c = (int)(uVar7 & 0x3e0) >> 2;

        uStack_48 = (uVar7 & 0x1f) << 3;

        break;

      case 4:

        uStack_50 = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10) * 3 + 2);

        uStack_4c = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10) * 3 + 1);

        uStack_48 = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10) * 3);

      default:

        break;

      case 5:

        uStack_50 = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10) * 4 + 2);

        uStack_4c = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10) * 4 + 1);

        uStack_48 = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10) * 4);

        break;

      case 8:

        uVar7 = (uint)*(ushort *)(*param_5 + (param_5[0xe] >> 0x10) * 2);

        uStack_50 = (int)(uVar7 & 0xf800) >> 8;

        uStack_4c = (int)(uVar7 & 0x7e0) >> 3;

        uStack_48 = (uVar7 & 0x1f) << 3;

        break;

      case 9:

        uStack_50 = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10) * 4 + 3);

        uStack_4c = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10) * 4 + 2);

        uStack_48 = (uint)*(byte *)(*param_5 + (param_5[0xe] >> 0x10) * 4 + 1);

      }

      uVar1 = (*(code *)param_7[2])

                        (uStack_50,uStack_4c,uStack_48,uStack_44,uStack_40,uStack_3c,uStack_38);

      *(undefined2 *)(iVar3 + iVar10 * 2 + ((param_1 - iVar6) / 2) * 2) = uVar1;

      param_5[0xe] = param_5[0xe] + param_5[8];

    }

    param_5[0xf] = param_5[0xf] + param_5[9];

    *(uint *)(param_4 + 0x58) =

         param_5[3] * (((param_5[0xf] >> 0x10) - ((uint)(param_5[0xf] - param_5[9]) >> 0x10)) + -1);

    uVar2 = FUN_02ffb850(*(undefined4 *)(param_4 + 0x12));

    FUN_02ffb4ec(uVar2,param_5[2]);

    iVar6 = FUN_02ffb8d0();

    *(int *)(param_4 + 0x58) = (param_5[3] - iVar6) + *(int *)(param_4 + 0x58);

    if (*(int *)(param_4 + 0x16) < 1) {

      (*(code *)param_7[1])(*param_6,(param_5[0xf] >> 0x10) * param_5[3] + *(int *)(param_4 + 10),0)

      ;

    }

    else {

      (*(code *)param_7[1])

                (*param_6,(param_5[1] - (param_5[0xf] >> 0x10)) * param_5[3] +

                          *(int *)(param_4 + 10),0);

    }

    uVar2 = 1;

  }

  else {

    uVar2 = 0;

  }

  return uVar2;

}
