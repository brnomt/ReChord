/**
 * sdio_obj_calc_offset @ 0x030cb73a
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


longlong sdio_obj_calc_offset(int *param_1,undefined4 param_2)



{

  longlong lVar1;

  undefined4 uVar2;

  int iVar3;

  int extraout_r1;

  uint uVar4;

  uint uVar5;

  uint uVar6;

  uint uVar7;

  longlong lVar8;

  int *piVar9;

  

  uVar6 = param_1[2];

  uVar4 = param_1[3];

  lVar8 = -1;

  uVar5 = uVar4;

  uVar7 = uVar6;

  piVar9 = param_1;

  do {

    uVar5 = (uVar5 - 1) + (uint)(0x3ff < uVar7);

    uVar7 = uVar7 - 0x400;

    if ((int)uVar5 < 0) {

      uVar5 = 0;

      uVar7 = uVar5;

    }

    if (*param_1 != -1) {

      (*(code *)param_1[0x25])(*param_1,(code *)param_1[0x25],uVar7,uVar5,0,piVar9);

      param_1[2] = uVar7;

      param_1[3] = uVar5;

      sdio_reset_func(param_1[6]);

    }

    iVar3 = param_1[3];

    lVar1 = lVar8;

    if ((int)((iVar3 - uVar4) - (uint)((uint)param_1[2] < uVar6)) < 0 !=

        (SBORROW4(iVar3,uVar4) != SBORROW4(iVar3 - uVar4,(uint)((uint)param_1[2] < uVar6)))) {

      do {

        lVar8 = FwCheck(param_1,param_2,uVar6 - param_1[2],

                        (uVar4 - param_1[3]) - (uint)(uVar6 < (uint)param_1[2]));

        if (lVar8 == -0x80) {

          uVar2 = 0xffffff80;

          goto LAB_030cb7d8;

        }

        if (lVar8 < 0) {

          *DAT_030cbbe0 = *DAT_030cbbe0 + 1;

          lVar8 = lVar1;

          break;

        }

        iVar3 = param_1[3];

        lVar1 = lVar8;

      } while ((int)((iVar3 - uVar4) - (uint)((uint)param_1[2] < uVar6)) < 0 !=

               (SBORROW4(iVar3,uVar4) != SBORROW4(iVar3 - uVar4,(uint)((uint)param_1[2] < uVar6))));

    }

    if (lVar8 != -1) {

      if (*param_1 != -1) {

        (*(code *)param_1[0x25])

                  (*param_1,(code *)param_1[0x25],(int)lVar8,(int)((ulonglong)lVar8 >> 0x20),0);

        *(longlong *)(param_1 + 2) = lVar8;

        sdio_reset_func(param_1[6]);

      }

      FwCheck(param_1,param_2,0x400,0);

      if (-1 < extraout_r1) {

        return lVar8;

      }

      uVar2 = 0xffffff7f;

LAB_030cb7d8:

      return CONCAT44(0xffffffff,uVar2);

    }

  } while( true );

}
