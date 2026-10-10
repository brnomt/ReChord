/**
 * APEV1_Parse @ 0x030cb34a
 * Tags: codec, ape, tags
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




int FUN_030cb34a(int *param_1)



{

  uint *puVar1;

  int *piVar2;

  int iVar3;

  int *piVar4;

  int iVar5;

  int iVar6;

  undefined1 *puVar7;

  undefined1 *puVar8;

  

  piVar4 = (int *)FUN_030cd9a8();

  *piVar4 = 0;

  if (*param_1 == -1) {

    return 0;

  }

  if (*DAT_030cb760 == 3) {

    FUN_030c66b8(*(undefined4 *)(param_1[6] + 8));

    puVar1 = DAT_030cb764;

    iVar5 = FUN_030c6d02(param_1[6],*DAT_030cb764);

    if (iVar5 != 0) {

      iVar6 = (*(code *)param_1[0x24])(iVar5,*puVar1,*param_1);

      iVar3 = DAT_030cb770;

      piVar2 = DAT_030cb768;

      if (0xff54 < *DAT_030cb768) {

        if (*DAT_030cb76c < 2) {

          if (0 < (int)(*puVar1 - 0x1a)) {

            puVar7 = (undefined1 *)(DAT_030cb770 + 0x19);

            puVar8 = (undefined1 *)(iVar5 + 0x19);

            if ((*puVar1 & 1) != 0) {

              puVar7 = (undefined1 *)(DAT_030cb770 + 0x1a);

              puVar8 = (undefined1 *)(iVar5 + 0x1a);

              *puVar8 = *puVar7;

            }

            for (iVar5 = (int)(*puVar1 - 0x1a) >> 1; iVar5 != 0; iVar5 = iVar5 + -1) {

              puVar8[1] = puVar7[1];

              puVar7 = puVar7 + 2;

              puVar8 = puVar8 + 2;

              *puVar8 = *puVar7;

            }

          }

        }

        else {

          if ((iVar5 - DAT_030cb770 < 1) || (3 < iVar5 - DAT_030cb770)) {

            FUN_030c0f56(iVar5 + 0x12,DAT_030cb770 + 0x12,4);

          }

          else {

            *(undefined1 *)(iVar5 + 0x12) = *(undefined1 *)(DAT_030cb770 + 0x12);

            *(undefined1 *)(iVar5 + 0x13) = *(undefined1 *)(iVar3 + 0x13);

            *(undefined1 *)(iVar5 + 0x14) = *(undefined1 *)(iVar3 + 0x14);

            *(undefined1 *)(iVar5 + 0x15) = *(undefined1 *)(iVar3 + 0x15);

          }

          if (0 < (int)(*puVar1 - 0x1a)) {

            puVar7 = (undefined1 *)(iVar3 + 0x19);

            puVar8 = (undefined1 *)(iVar5 + 0x19);

            if ((*puVar1 & 1) != 0) {

              puVar7 = (undefined1 *)(iVar3 + 0x1a);

              puVar8 = (undefined1 *)(iVar5 + 0x1a);

              *puVar8 = *puVar7;

            }

            for (iVar5 = (int)(*puVar1 - 0x1a) >> 1; iVar5 != 0; iVar5 = iVar5 + -1) {

              puVar8[1] = puVar7[1];

              puVar7 = puVar7 + 2;

              puVar8 = puVar8 + 2;

              *puVar8 = *puVar7;

            }

          }

        }

      }

      if (iVar6 < 1) {

        if ((iVar6 == 0) && (*piVar4 != 0)) {

          return -1;

        }

      }

      else {

        FUN_030c6d68(param_1[6],iVar6);

      }

      iVar5 = *piVar2;

      if (iVar5 < 0xff55) {

        (*(code *)param_1[0x25])

                  (*param_1,(code *)param_1[0x25],*DAT_030cb774,*DAT_030cb774 >> 0x1f,1);

      }

      else {

        (*(code *)param_1[0x25])(*param_1,(code *)param_1[0x25],iVar5,iVar5 >> 0x1f,0);

      }

      return iVar6;

    }

  }

  else {

    iVar5 = FUN_030c6d02(param_1[6],0x400);

    if (iVar5 != 0) {

      iVar5 = (*(code *)param_1[0x24])(iVar5,0x400,*param_1);

      if (iVar5 < 1) {

        if ((iVar5 == 0) && (*piVar4 != 0)) {

          return -1;

        }

      }

      else {

        FUN_030c6d68(param_1[6],iVar5);

      }

      return iVar5;

    }

  }

  return -1;

}




