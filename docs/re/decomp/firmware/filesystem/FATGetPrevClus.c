/**
 * FATGetPrevClus @ 0x03086358
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int FATGetPrevClus(int *param_1,ushort *param_2,short *param_3)



{

  ushort uVar1;

  short sVar2;

  undefined1 uVar3;

  ushort uVar4;

  int iVar5;

  short *psVar6;

  uint uVar7;

  int iVar8;

  int iVar9;

  int *piVar10;

  uint uVar11;

  int iVar12;

  int iVar13;

  bool bVar14;

  bool bVar15;

  undefined8 uVar16;

  short local_38 [2];

  short local_34 [2];

  short local_30 [2];

  int local_2c [2];

  

  iVar12 = *param_1;

  iVar13 = 0;

  iVar9 = DAT_03086748;

  if (param_1 == (int *)0x0) goto LAB_03086756;

  if (param_2 == (ushort *)0x0) goto LAB_03086772;

  if (iVar12 != 0) {

    *param_2 = 0;

    if (param_3 != (short *)0x0) {

      *param_3 = 0;

    }

switchD_030863a0_default:

    switch(*(undefined1 *)((int)param_1 + 0x1e)) {

    case 0:

      *(undefined1 *)((int)param_1 + 0x1e) = 2;

      *(undefined1 *)((int)param_1 + 0x23) = 2;

      if (0 < param_1[0x17]) {

        param_1[0x17] = param_1[0x17] + -1;

      }

      goto switchD_030863a0_default;

    case 1:

      goto switchD_030863a0_default;

    case 2:

      *(int *)(*param_1 + 0x148) = *(int *)(*param_1 + 0x148) + 1;

      param_1[0x1f] = 0;

      *(undefined2 *)(param_1 + 0x20) = 0;

      *(undefined4 *)(iVar12 + 0x110) = 0;

      param_1[0x31] = 0;

      iVar5 = *(int *)(iVar12 + 0x118);

      iVar9 = *(int *)(iVar12 + 0x11c);

      if (iVar9 <= iVar5) {

        iVar9 = *(int *)(iVar12 + 0x154);

        iVar8 = iVar9 - (iVar9 >> 0x1f);

        if (iVar9 / 2 <= iVar5) {

          *(undefined4 *)(iVar12 + 0x11c) = 0;

          *(undefined4 *)(iVar12 + 0x118) = 0;

        }

        uVar1 = *(ushort *)(iVar12 + 0x26);

        bVar15 = uVar1 != 0;

        uVar7 = 0;

        if (bVar15) {

          uVar7 = uVar1 & 1;

        }

        bVar14 = (uVar1 & 1) != 0;

        if (bVar15 && bVar14) {

          iVar8 = *(int *)(iVar12 + 0xd8);

        }

        if (bVar15 && bVar14) {

          *(undefined2 *)(iVar8 + 0x7c) = 0;

        }

        if (uVar7 < *(ushort *)(iVar12 + 0x26)) {

          do {

            iVar9 = uVar7 * 0xfc;

            *(undefined2 *)(*(int *)(iVar12 + 0xd8) + uVar7 * 0xfc + 0x7c) = 0;

            uVar7 = (uint)(short)((short)uVar7 + 2);

            *(undefined2 *)(iVar9 + *(int *)(iVar12 + 0xd8) + 0x178) = 0;

          } while ((int)uVar7 < (int)(uint)*(ushort *)(iVar12 + 0x26));

        }

        uVar3 = 3;

        goto LAB_0308673e;

      }

      goto LAB_03086682;

    case 3:

      if ((short)param_1[4] == 0) goto LAB_03086754;

      uVar16 = p2p_xfer_helper(param_1);

      iVar5 = (int)((ulonglong)uVar16 >> 0x20);

      iVar13 = (int)uVar16;

      iVar9 = iVar13;

      if (iVar13 < 0) goto LAB_03086756;

      uVar1 = *(ushort *)(iVar12 + 0x26);

      bVar15 = uVar1 != 0;

      uVar7 = 0;

      if (bVar15) {

        uVar7 = uVar1 & 1;

      }

      bVar14 = (uVar1 & 1) != 0;

      if (bVar15 && bVar14) {

        iVar5 = *(int *)(iVar12 + 0xd8);

      }

      if (bVar15 && bVar14) {

        *(undefined2 *)(iVar5 + 0x7e) = 0;

      }

      if (uVar7 < *(ushort *)(iVar12 + 0x26)) {

        do {

          iVar9 = uVar7 * 0xfc;

          *(undefined2 *)(*(int *)(iVar12 + 0xd8) + uVar7 * 0xfc + 0x7e) = 0;

          uVar7 = (uint)(short)((short)uVar7 + 2);

          *(undefined2 *)(iVar9 + *(int *)(iVar12 + 0xd8) + 0x17a) = 0;

        } while ((int)uVar7 < (int)(uint)*(ushort *)(iVar12 + 0x26));

      }

      uVar3 = 4;

LAB_0308673e:

      *(undefined1 *)((int)param_1 + 0x1e) = uVar3;

      goto switchD_030863a0_default;

    case 4:

      if (param_1[0x1f] == 0) {

        iVar9 = 0;

        if (0 < *(short *)(iVar12 + 0x188)) {

          do {

            iVar13 = iVar9 * 2;

            iVar9 = (int)(short)((short)iVar9 + 1);

            *(undefined2 *)

             (*(int *)(iVar12 + 0xd8) + *(short *)(*(int *)(iVar12 + 0x18c) + iVar13) * 0xfc + 0x52)

                 = 0x7fff;

          } while (iVar9 < *(short *)(iVar12 + 0x188));

        }

        *(undefined1 *)((int)param_1 + 0x1e) = 5;

        *(undefined1 *)((int)param_1 + 0x1f) = 0;

        *(undefined1 *)(param_1 + 8) = 0;

        *(undefined2 *)(param_1 + 0xd) = 0;

        *(undefined2 *)((int)param_1 + 0x36) = 0;

      }

      uVar1 = *(ushort *)(iVar12 + 0x188);

      if ((short)uVar1 < 1) {

        uVar4 = 0;

      }

      else {

        uVar4 = uVar1 & 1;

      }

      for (; (short)uVar4 < (short)uVar1; uVar4 = uVar4 + 2) {

      }

      break;

    case 6:

      goto switchD_030863a0_caseD_6;

    case 7:

      goto switchD_030863a0_caseD_7;

    case 8:

      goto switchD_030863a0_caseD_8;

    case 9:

      goto switchD_030863a0_caseD_9;

    default:

      goto switchD_030863a0_default;

    }

    if ((*(int *)(iVar12 + 0xb4) == 0) || (*(int *)(iVar12 + 0x2c) != 0)) {

      iVar13 = FATGetNextClus(param_1);

    }

    else {

      iVar13 = FUN_0308876e(param_1);

    }

    iVar9 = iVar13;

    if (-1 < iVar13) {

      if (*(char *)((int)param_1 + 0x1f) == '\b') goto LAB_030864ac;

      goto switchD_030863a0_caseD_6;

    }

    goto LAB_03086756;

  }

  goto LAB_0308675c;

LAB_030864ac:

  if (param_1[0x1f] == 0) {

    uVar3 = 8;

  }

  else {

    uVar3 = 6;

  }

  *(undefined1 *)((int)param_1 + 0x1e) = uVar3;

  if (param_1[0x1f] == 0) goto switchD_030863a0_default;

switchD_030863a0_caseD_6:

  *(undefined1 *)((int)param_1 + 0x1e) = 7;

switchD_030863a0_caseD_7:

  *(short *)(param_1 + 4) = (short)param_1[4] + -1;

  *(undefined1 *)((int)param_1 + 0x1e) = 8;

switchD_030863a0_caseD_8:

  if (param_3 == (short *)0x0) {

    param_3 = local_30;

  }

  iVar9 = FUN_03087b5c(iVar12);

  if (iVar9 == 1) {

    iVar9 = -0xf;

    goto LAB_03086756;

  }

  *(short *)(param_1 + 0x20) = (short)param_1[0x20] + 1;

  iVar5 = 0;

  if (0 < *(short *)(iVar12 + 0x188)) {

    do {

      iVar8 = *(int *)(iVar12 + 0xd8) + *(short *)(*(int *)(iVar12 + 0x18c) + iVar5 * 2) * 0xfc;

      iVar9 = DAT_03086750;

      if (**(short **)(iVar8 + 0xc0) <= *(short *)(iVar8 + 0x7c)) goto LAB_03086756;

      *(short *)(iVar8 + 0x7c) = *(short *)(iVar8 + 0x7c) + 1;

      iVar5 = (int)(short)((short)iVar5 + 1);

    } while (iVar5 < *(short *)(iVar12 + 0x188));

  }

switchD_030863a0_caseD_9:

  if (*(char *)(iVar12 + 0x48) == '\x03') {

    if ((char)param_1[0x76] != '\0') {

      *(undefined4 *)(*param_1 + 0x108) = 1;

    }

    FUN_03088174(iVar12,param_1[0x75]);

  }

  if (param_1[0x1f] == 0) {

    *(undefined1 *)((int)param_1 + 0x1e) = 4;

    iVar9 = iVar13;

    goto LAB_03086756;

  }

  iVar5 = *(int *)(iVar12 + 0x118);

  iVar9 = *(int *)(iVar12 + 0x11c);

  if (iVar5 < iVar9) {

LAB_03086682:

    *param_2 = (short)iVar9 - (short)iVar5;

    goto switchD_030863a0_default;

  }

  iVar9 = DAT_0308674c;

  if ((-1 < iVar5) && (iVar9 = DAT_0308674c, iVar5 < *(int *)(iVar12 + 0x154) / 2)) {

    FUN_03085c50(iVar12,local_2c);

    *(int *)(iVar12 + 0x11c) = *(int *)(iVar12 + 0x118) + local_2c[0];

    piVar10 = *(int **)(iVar12 + 0xfc);

    iVar9 = *piVar10;

    if ((0 < iVar9) && (param_1[0x75] == 0)) {

      if (iVar9 < local_2c[0]) {

        iVar5 = iVar9 + *(int *)(iVar12 + 0x118);

      }

      else {

        *piVar10 = iVar9 - local_2c[0];

        iVar5 = *(int *)(iVar12 + 0x11c);

      }

      *(int *)(iVar12 + 0x118) = iVar5;

      if (iVar9 < local_2c[0]) {

        *piVar10 = 0;

      }

      besl_p2p_host_find_device(iVar12);

    }

    iVar9 = DAT_03086750;

    if (*(int *)(iVar12 + 0x118) <= *(int *)(iVar12 + 0x11c)) {

      *param_2 = (short)*(int *)(iVar12 + 0x11c) - (short)*(int *)(iVar12 + 0x118);

      if (*(int *)(iVar12 + 0x4c) != 0) {

        if (param_3 == (short *)0x0) {

          param_3 = local_30;

        }

        *param_3 = 0;

        if (*(int *)(iVar12 + 0x40) < 3) {

          psVar6 = *(short **)(*(int *)(*(int *)(iVar12 + 0xd8) + 0xc0) + 8);

          FUN_03087f6e(iVar12,1,(int)psVar6[-1],(int)*psVar6,local_38,local_34);

          *param_3 = (local_34[0] - local_38[0]) + *param_3;

        }

        *(undefined4 *)(iVar12 + 0x4c) = 0;

        if (param_3 != (short *)0x0) {

          uVar11 = param_1[0x14];

          uVar7 = (uint)*param_3;

          param_1[0x14] = uVar11 + uVar7;

          param_1[0x15] = param_1[0x15] + ((int)uVar7 >> 0x1f) + (uint)CARRY4(uVar11,uVar7);

        }

      }

      if ((*(int *)(iVar12 + 0x70) != 0) && (iVar9 = 0, *(short *)(iVar12 + 0x26) != 0)) {

        do {

          psVar6 = *(short **)(*(int *)(iVar12 + 0xd8) + iVar9 * 0xfc + 0xc0);

          sVar2 = *psVar6;

          if (*(int *)(iVar12 + 0x40) < 3) {

            *(undefined2 *)(*(int *)(psVar6 + 4) + -2) =

                 *(undefined2 *)(*(int *)(psVar6 + 4) + sVar2 * 2 + -2);

            **(undefined2 **)(psVar6 + 4) = (*(undefined2 **)(psVar6 + 4))[sVar2];

            **(undefined2 **)(psVar6 + 6) = 0;

            *psVar6 = 1;

          }

          iVar9 = (int)(short)((short)iVar9 + 1);

        } while (iVar9 < (int)(uint)*(ushort *)(iVar12 + 0x26));

      }

      *(undefined1 *)((int)param_1 + 0x1e) = 2;

      *(undefined1 *)((int)param_1 + 0x23) = 1;

      iVar9 = iVar13;

      if ((short)param_1[4] < 1) {

LAB_03086754:

        iVar9 = 4;

      }

    }

  }

LAB_03086756:

  if (param_2 == (ushort *)0x0) goto LAB_03086772;

LAB_0308675c:

  if (*param_2 != 0) {

    *(undefined1 *)(param_1 + 0x72) = 3;

    param_1[0x74] = (uint)*param_2;

    return iVar9;

  }

LAB_03086772:

  *(undefined1 *)(param_1 + 0x72) = 2;

  return iVar9;

}
