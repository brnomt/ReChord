/**
 * wma_frame_dispatch @ 0x0309ff48
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 wma_frame_dispatch(uint param_1,int param_2,int param_3)



{

  int iVar1;

  int *piVar2;

  uint uVar3;

  undefined4 uVar4;

  int iVar5;

  uint uVar6;

  bool bVar7;

  

  iVar1 = DAT_0309fffc;

  if (*(int *)(DAT_0309fffc + 0x10) == 1) {

    hifi_busy_delay_ovl_09e3(1);

    iVar5 = DAT_030a0004;

    do {

      if (*(int *)(iVar1 + 0x10) != 1) goto LAB_0309ff7a;

      hifi_busy_delay_ovl_09e3(1);

      iVar5 = iVar5 + -1;

    } while (iVar5 != 0);

    hifi_debug_printf_ovl_09e0(s_AudioFileSeek2__timeout___030a004c);

  }

LAB_0309ff7a:

  piVar2 = DAT_030a0008;

  uVar6 = *(uint *)(DAT_030a0048 + param_3 * 4);

  if (param_2 == 0) {

    uVar3 = uVar6 - (*DAT_030a0008 + DAT_030a0008[1]);

    bVar7 = param_1 <= uVar3;

    if (uVar3 <= param_1) {

      bVar7 = uVar6 <= param_1;

    }

    if (!bVar7) {

      iVar5 = *(int *)(iVar1 + 0x18);

      uVar3 = param_1 - uVar3;

      uVar6 = DAT_030a0008[iVar5];

      if (uVar3 < uVar6) goto LAB_0309fff8;

      goto LAB_0309ffde;

    }

  }

  else if (param_2 == 1) {

    uVar3 = *(int *)(iVar1 + 0x14) + param_1;

    if ((uint)(*DAT_030a0008 + DAT_030a0008[1]) <= uVar3) {

      uVar3 = (uVar6 - (*DAT_030a0008 + DAT_030a0008[1])) + uVar3;

      uVar4 = wma_frame_helper_a(uVar3 & 0xfffffe00,0,param_3);

      FUN_0309fc18(param_3);

      *(uint *)(iVar1 + 0x14) = (uVar3 & 0x1ff) + *(int *)(iVar1 + 0x14);

      return uVar4;

    }

    if ((int)param_1 < 1) {

      if (-1 < (int)param_1) {

        return 0;

      }

LAB_0309fff8:

      *(uint *)(iVar1 + 0x14) = uVar3;

      return 0;

    }

    *(uint *)(iVar1 + 0x14) = uVar3;

    iVar5 = *(int *)(iVar1 + 0x18);

    uVar6 = piVar2[iVar5];

    if (uVar3 <= uVar6) {

      return 0;

    }

LAB_0309ffde:

    *(uint *)(iVar1 + 0x14) = uVar3 - uVar6;

    piVar2[iVar5] = 0;

    *(int *)(iVar1 + 0x1c) = iVar5;

    *(int *)(iVar1 + 0x18) = 1 - iVar5;

    FUN_030a3900(0xf2);

    return 0;

  }

  uVar4 = wma_frame_helper_a(((int)param_1 / 0x200) * 0x200,param_2,param_3);

  FUN_0309fc18(param_3);

  *(int *)(iVar1 + 0x14) = (int)param_1 % 0x200 + *(int *)(iVar1 + 0x14);

  return uVar4;

}
