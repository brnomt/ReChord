/**
 * FileDev_FileSeek2 @ 0x030501dc
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



uint FileDev_FileSeek2(short *param_1,int param_2,int *param_3)



{

  short sVar1;

  bool bVar2;

  int iVar3;

  int iVar4;

  int iVar5;

  undefined4 uVar6;

  uint uVar7;

  uint uVar8;

  uint uVar9;

  bool bVar10;

  uint auStack_80 [2];

  undefined2 auStack_78 [2];

  undefined1 *puStack_74;

  undefined1 auStack_70 [4];

  undefined4 uStack_6c;

  uint uStack_68;

  uint uStack_64;

  undefined1 auStack_60 [32];

  uint uStack_40;

  uint uStack_3c;

  uint uStack_38;

  uint uStack_34;

  short *local_30;

  int local_2c;

  int *piStack_28;

  

  piStack_28 = param_3;

  local_2c = param_2;

  local_30 = param_1;

  bVar2 = false;

  do {

    if (param_1 == (short *)0x0) {

      return 0x7fffffff;

    }

    if (*(int *)(param_1 + 4) < 1) {

      return 0x7fffffff;

    }

    if (local_2c == -1 || param_3 == (int *)0x0) {

      return 0x7fffffff;

    }

    if (*param_3 != 0) {

      return param_3[1];

    }

    if (param_3[1] == 0) {

      return 0x7fffffff;

    }

    sVar1 = *param_1;

    bVar10 = sVar1 == 0x70 || sVar1 == 0x71;

    if (sVar1 == 0x70 || sVar1 == 0x71) {

      bVar10 = param_1[1] == 0x62;

    }

    if (bVar10) {

      auStack_80[0] = (uint)(ushort)param_1[0xb];

      auStack_80[1] = (*(int *)(param_1 + 4) + -1) * (uint)(ushort)param_1[8] + auStack_80[0];

    }

    else {

      iVar3 = FUN_0301a38a();

      if (iVar3 < 1) {

        return 0x7fffffff;

      }

      iVar5 = 0;

      do {

        uVar7 = (uint)*(byte *)(param_3[1] + iVar5);

        uVar9 = uVar7 - 0x41;

        if (0x19 < uVar9) {

          uVar9 = uVar7 - 0x61;

        }

      } while ((0x19 < uVar9) && (iVar5 = iVar5 + 1, iVar5 < iVar3));

      if (iVar3 <= iVar5) {

        return 0x7fffffff;

      }

      uVar9 = (uint)*(byte *)(param_3[1] + iVar5);

      if (uVar9 - 0x41 < 0x1a) {

        uVar9 = uVar9 + 0x20 & 0xff;

      }

      auStack_80[0] = (uVar9 - 0x61) * 0x1a;

      uVar9 = (uint)*(byte *)(param_3[1] + iVar5 + 1);

      if (uVar9 - 0x41 < 0x1a) {

        uVar9 = uVar9 + 0x20 & 0xff;

      }

      if (uVar9 - 0x61 < 0x1a) {

        auStack_80[0] = (uVar9 - 0x61) + auStack_80[0];

      }

      uVar9 = (*(int *)(param_1 + 4) + -1) * (uint)(ushort)param_1[8] +

              (uint)(ushort)param_1[7] * (uint)(ushort)param_1[6] + (uint)(ushort)param_1[0xb];

      if ((int)(auStack_80[0] + 1) < (int)(uint)(ushort)param_1[7]) {

        iVar3 = 2;

      }

      else {

        iVar3 = 1;

        auStack_80[1] = uVar9;

      }

      auStack_80[0] = (ushort)param_1[6] * auStack_80[0] + (uint)(ushort)param_1[0xb];

      if (iVar3 == 2) {

        auStack_80[1] = (ushort)param_1[6] + auStack_80[0];

      }

      auStack_70[0] = 6;

      uStack_6c = 1;

      iVar5 = 0;

      uStack_68 = (uint)*(byte *)(param_1 + 2);

      if (iVar3 != 0) {

        do {

          uVar7 = auStack_80[iVar5];

          iVar4 = FatDev_FileSeek(local_2c,0,uVar7);

          if (iVar4 != 0) {

            return 0x7fffffff;

          }

          uStack_64 = uVar7;

          iVar4 = FUN_0304f550(local_2c,auStack_70);

          if (iVar4 == 0) {

            return 0x7fffffff;

          }

          auStack_80[iVar5] = 0;

          uVar7 = dict_stream_read(local_2c,param_1[6],auStack_80 + iVar5);

          if (uVar7 != (ushort)param_1[6]) {

            return 0x7fffffff;

          }

          iVar5 = iVar5 + 1;

        } while (iVar5 < iVar3);

      }

      if ((int)uVar9 < (int)auStack_80[1]) {

        auStack_80[1] = uVar9;

      }

      if ((int)auStack_80[1] < (int)auStack_80[0]) {

        auStack_80[0] = auStack_80[1];

      }

    }

    auStack_78[0] = 0x1e;

    puStack_74 = auStack_60;

    if ((*param_1 == 0x70 || *param_1 == 0x71) && (param_1[1] == 0x62)) {

      uVar6 = 0;

    }

    else {

      uVar6 = 1;

    }

    uVar8 = 0;

    uStack_40 = 0;

    uStack_38 = auStack_80[0];

    iVar3 = 1;

    uVar9 = 0;

    uStack_3c = auStack_80[1];

    uStack_34 = 0;

    uVar7 = auStack_80[0];

    if ((int)auStack_80[0] <= (int)auStack_80[1]) {

      do {

        iVar3 = (uint)(ushort)param_1[7] * (uint)(ushort)param_1[6] + (uint)(ushort)param_1[0xb];

        uVar7 = (((int)(auStack_80[0] + auStack_80[1]) / 2 - iVar3) / (int)(uint)(ushort)param_1[8])

                * (uint)(ushort)param_1[8] + iVar3;

        iVar3 = FileDev_FileSeek(local_30,local_2c,uVar7,auStack_78);

        if (iVar3 == 0) {

          return 0x7fffffff;

        }

        if (bVar2) {

          iVar3 = FUN_0305006e(param_3[1],puStack_74,uVar6,1);

        }

        else {

          iVar3 = FUN_0305006e(param_3[1],puStack_74,uVar6,0);

        }

        if ((uVar9 & 0xffff) <= (uStack_40 & 0xffff)) {

          uVar8 = uVar7;

          uVar9 = uStack_40;

        }

        if (iVar3 == 0) break;

        if (iVar3 < 0) {

          auStack_80[1] = uVar7 - (ushort)param_1[8];

        }

        else {

          auStack_80[0] = (ushort)param_1[8] + uVar7;

        }

      } while ((int)auStack_80[0] <= (int)auStack_80[1]);

      if (uStack_34 != 0) {

        iVar3 = 0;

        uVar7 = uStack_34;

        goto LAB_03050484;

      }

      if (iVar3 == 0) goto LAB_03050484;

    }

    if (bVar2) {

      if ((uVar8 != uVar7) &&

         (iVar5 = FileDev_FileSeek(local_30,local_2c,uVar8,auStack_78), uVar7 = uVar8, iVar5 == 0))

      {

        return 0x7fffffff;

      }

      FUN_0301a38a(param_3[1]);

      iVar5 = FUN_0305006e(param_3[1],puStack_74,uVar6,0);

      if ((iVar5 != 0) &&

         (((int)uVar7 < (int)uStack_38 || (uStack_38 = uStack_3c, (int)uStack_3c < (int)uVar7)))) {

        uVar7 = uStack_38;

      }

LAB_03050484:

      if (iVar3 == 0) {

        uVar9 = 0;

      }

      else {

        uVar9 = 0x400000;

      }

      return uVar9 | (int)((uVar7 - (ushort)param_1[0xb]) -

                          (uint)(ushort)param_1[6] * (uint)(ushort)param_1[7]) /

                     (int)(uint)(ushort)param_1[8];

    }

    bVar2 = true;

  } while( true );

}
