/**
 * dict_frame_helper @ 0x0304f376
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 dict_frame_helper(int param_1,int param_2,int param_3,int *param_4)



{

  int *piVar1;

  int *piVar2;

  int iVar3;

  int iVar4;

  int iVar5;

  int iVar6;

  int *piVar7;

  int *piVar8;

  int iVar9;

  int iVar10;

  int iVar11;

  bool bVar12;

  

  piVar8 = DAT_0304f574;

  piVar7 = (int *)0x0;

  *DAT_0304f574 = param_2;

  piVar8[1] = param_3;

  iVar3 = param_4[1];

  iVar11 = param_4[2];

  iVar5 = param_4[3];

  piVar8[2] = *param_4;

  piVar8[3] = iVar3;

  piVar8[4] = iVar11;

  piVar8[5] = iVar5;

  iVar3 = *piVar8;

  iVar5 = piVar8[1] + iVar3 + -1;

  if (*(int **)(param_1 + 4) != (int *)0x0) {

    piVar2 = *(int **)(param_1 + 4);

    do {

      piVar1 = DAT_0304f578;

      iVar6 = *piVar2;

      iVar9 = piVar2[1];

      iVar4 = iVar6 + iVar9 + -1;

      bVar12 = SBORROW4(iVar6,iVar3);

      iVar11 = iVar6 - iVar3;

      if (iVar3 <= iVar6) {

        bVar12 = SBORROW4(iVar5,iVar4);

        iVar11 = iVar5 - iVar4;

      }

      if (iVar11 < 0 == bVar12) {

        if (piVar7 == (int *)0x0) {

          *(int *)(param_1 + 4) = piVar2[6];

        }

        else {

          piVar7[6] = piVar2[6];

        }

        piVar1 = (int *)piVar2[6];

      }

      else {

        bVar12 = SBORROW4(iVar3,iVar6);

        iVar11 = iVar3 - iVar6;

        if (iVar6 <= iVar3) {

          bVar12 = SBORROW4(iVar4,iVar5);

          iVar11 = iVar4 - iVar5;

        }

        if (iVar11 < 0 == bVar12) {

          if (iVar3 == iVar6 || iVar5 == iVar4) {

            if (iVar3 == iVar6) {

              iVar4 = iVar4 - iVar5;

              goto LAB_0304f424;

            }

LAB_0304f432:

            piVar2[1] = (iVar9 - (iVar4 - iVar3)) + -1;

          }

          else {

            iVar11 = piVar2[2];

            iVar10 = piVar2[3];

            *DAT_0304f578 = iVar6;

            piVar1[1] = iVar9;

            piVar1[2] = iVar11;

            piVar1[3] = iVar10;

            iVar11 = piVar2[4];

            iVar6 = piVar2[5];

            piVar1[6] = piVar2[6];

            piVar1[4] = iVar11;

            piVar1[5] = iVar6;

            piVar2[1] = (piVar2[1] - (iVar4 - iVar3)) + -1;

            *piVar1 = iVar5 + 1;

            piVar1[1] = iVar4 - iVar5;

            piVar2[6] = (int)piVar1;

            piVar2 = piVar1;

          }

        }

        else {

          bVar12 = SBORROW4(iVar6,iVar3);

          iVar11 = iVar6 - iVar3;

          if (iVar3 <= iVar6) {

            bVar12 = SBORROW4(iVar5,iVar6);

            iVar11 = iVar5 - iVar6;

          }

          if (iVar11 < 0 == bVar12) {

            iVar4 = (iVar9 - (iVar5 - iVar6)) + -1;

LAB_0304f424:

            *piVar2 = iVar5 + 1;

            piVar2[1] = iVar4;

          }

          else {

            bVar12 = SBORROW4(iVar4,iVar3);

            iVar11 = iVar4 - iVar3;

            if (iVar3 <= iVar4) {

              bVar12 = SBORROW4(iVar5,iVar4);

              iVar11 = iVar5 - iVar4;

            }

            if (iVar11 < 0 == bVar12) goto LAB_0304f432;

          }

        }

        piVar1 = (int *)piVar2[6];

        piVar7 = piVar2;

      }

      piVar2 = piVar1;

    } while (piVar1 != (int *)0x0);

    piVar7 = *(int **)(param_1 + 4);

    if (piVar7 != (int *)0x0) {

      if (*piVar8 < *piVar7) {

        piVar8[6] = (int)piVar7;

        *(int **)(param_1 + 4) = piVar8;

      }

      else {

        do {

          piVar2 = piVar7;

          piVar7 = (int *)piVar2[6];

          if (piVar7 == (int *)0x0) break;

        } while (*piVar7 <= *piVar8);

        piVar8[6] = (int)piVar7;

        piVar2[6] = (int)piVar8;

      }

      goto LAB_0304f470;

    }

  }

  piVar8[6] = 0;

  *(int **)(param_1 + 4) = piVar8;

LAB_0304f470:

  piVar8 = *(int **)(param_1 + 4);

  while (piVar7 = piVar8, piVar7 != (int *)0x0) {

    piVar8 = (int *)piVar7[6];

    if (((piVar8 != (int *)0x0) && (*piVar7 + piVar7[1] + -1 == *piVar8)) &&

       (iVar3 = FUN_0301a3b4(piVar7 + 2,piVar8 + 2,0x10), iVar3 == 0)) {

      piVar7[1] = piVar7[1] + piVar8[1];

      piVar8 = (int *)piVar8[6];

      piVar7[6] = (int)piVar8;

    }

  }

  return 1;

}
