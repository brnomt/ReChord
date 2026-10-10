/**
 * sdio_memcpy_toio @ 0x030cbebc
 * Structural-matcher sweep (post-boundary-repair, module-validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int sdio_memcpy_toio(int *param_1,int param_2,uint param_3,uint param_4)



{

  int iVar1;

  uint *puVar2;

  uint uVar3;

  uint uVar4;

  uint uVar5;

  uint uVar6;

  int iVar7;

  uint uVar8;

  bool bVar9;

  

  iVar1 = param_1[0x1a];

  if (iVar1 == 2 || iVar1 == 3) {

    uVar8 = param_1[7];

    uVar6 = 0;

    uVar5 = param_4;

    if ((int)uVar8 < 1) {

      uVar4 = 0;

    }

    else {

      uVar4 = uVar8 & 1;

      if (uVar4 != 0) {

        param_3 = param_1[2];

        uVar5 = param_1[3];

        do {

          puVar2 = (uint *)(uVar6 * 8 + 8 + param_1[8]);

          uVar3 = puVar2[1];

          bVar9 = *puVar2 < param_3;

          if ((int)((uVar3 - uVar5) - (uint)bVar9) < 0 ==

              (SBORROW4(uVar3,uVar5) != SBORROW4(uVar3 - uVar5,(uint)bVar9))) goto LAB_030cbf50;

          uVar6 = uVar6 + 1;

        } while ((int)uVar6 < (int)uVar4);

      }

    }

    uVar6 = uVar4;

    if ((int)uVar4 < (int)uVar8) {

      uVar5 = param_1[2];

      iVar1 = param_1[3];

      param_3 = param_1[8];

      do {

        puVar2 = (uint *)(uVar6 * 8 + 8 + param_3);

        uVar4 = puVar2[1];

        bVar9 = *puVar2 < uVar5;

        if ((int)((uVar4 - iVar1) - (uint)bVar9) < 0 ==

            (SBORROW4(uVar4,iVar1) != SBORROW4(uVar4 - iVar1,(uint)bVar9))) break;

        puVar2 = (uint *)(param_3 + uVar6 * 8 + 0x10);

        uVar4 = puVar2[1];

        bVar9 = *puVar2 < uVar5;

        if ((int)((uVar4 - iVar1) - (uint)bVar9) < 0 ==

            (SBORROW4(uVar4,iVar1) != SBORROW4(uVar4 - iVar1,(uint)bVar9))) {

          uVar6 = uVar6 + 1;

          break;

        }

        uVar6 = uVar6 + 2;

      } while ((int)uVar6 < (int)uVar8);

    }

LAB_030cbf50:

    if (uVar8 == uVar6) {

      return -1;

    }

    uVar8 = param_1[2];

    iVar7 = param_1[3];

    iVar1 = FUN_030cbb76(param_1,uVar6,param_3,uVar5,param_4);

    if (iVar1 != 0) {

      return iVar1;

    }

    if (*param_1 != -1) {

      (*(code *)param_1[0x25])(*param_1,(code *)param_1[0x25],uVar8,iVar7,0);

      param_1[3] = iVar7;

      param_1[2] = uVar8;

      sdio_reset_func(param_1[6]);

    }

    puVar2 = (uint *)(param_1[8] + uVar6 * 8);

    uVar5 = puVar2[1];

    bVar9 = uVar8 < *puVar2;

    if ((int)((iVar7 - uVar5) - (uint)bVar9) < 0 ==

        (SBORROW4(iVar7,uVar5) != SBORROW4(iVar7 - uVar5,(uint)bVar9))) {

      puVar2 = (uint *)(param_1[8] + uVar6 * 8 + 8);

      uVar5 = *puVar2;

      uVar6 = puVar2[1];

      param_2 = uVar8 - uVar5;

      if ((int)((iVar7 - uVar6) - (uint)(uVar8 < uVar5)) < 0 !=

          (SBORROW4(iVar7,uVar6) != SBORROW4(iVar7 - uVar6,(uint)(uVar8 < uVar5))))

      goto LAB_030cbfb2;

    }

    param_2 = 3;

    param_1[0x1a] = 3;

  }

  else if (iVar1 != 4) {

    if (iVar1 == 5) {

      return 0;

    }

    return -1;

  }

LAB_030cbfb2:

  iVar1 = FUN_030c3f18(param_1 + 0xc,param_2);

  param_1[0x23] = iVar1;

  param_1[0x1a] = 5;

  param_1[0x1e] = 0;

  param_1[0x1f] = 0;

  param_1[0x20] = 0;

  param_1[0x21] = 0;

  return 0;

}
