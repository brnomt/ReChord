/**
 * AudioFileSeek2 @ 0x0306dd54
 * Tags: fs, audio
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 AudioFileSeek2(uint param_1,int param_2,int param_3)



{

  int iVar1;

  int *piVar2;

  uint uVar3;

  undefined4 uVar4;

  int iVar5;

  uint uVar6;

  bool bVar7;

  

  iVar1 = DAT_0306de04;

  if (*(int *)(DAT_0306de04 + 0x10) == 1) {

    hifi_busy_delay(1);

    iVar5 = DAT_0306de0c;

    do {

      if (*(int *)(iVar1 + 0x10) != 1) goto LAB_0306dd86;

      hifi_busy_delay(1);

      iVar5 = iVar5 + -1;

    } while (iVar5 != 0);

    hifi_debug_printf(s_AudioFileSeek2__timeout___0306de54);

  }

LAB_0306dd86:

  piVar2 = DAT_0306de10;

  uVar6 = *(uint *)(DAT_0306de50 + param_3 * 4);

  if (param_2 == 0) {

    uVar3 = uVar6 - (*DAT_0306de10 + DAT_0306de10[1]);

    bVar7 = param_1 <= uVar3;

    if (uVar3 <= param_1) {

      bVar7 = uVar6 <= param_1;

    }

    if (!bVar7) {

      iVar5 = *(int *)(iVar1 + 0x18);

      uVar3 = param_1 - uVar3;

      uVar6 = DAT_0306de10[iVar5];

      if (uVar3 < uVar6) goto LAB_0306de72;

      goto LAB_0306ddea;

    }

  }

  else if (param_2 == 1) {

    uVar3 = *(int *)(iVar1 + 0x14) + param_1;

    if ((uint)(*DAT_0306de10 + DAT_0306de10[1]) <= uVar3) {

      uVar3 = (uVar6 - (*DAT_0306de10 + DAT_0306de10[1])) + uVar3;

      uVar4 = HifiFileSeek(uVar3 & 0xfffffe00,0,param_3);

      FUN_0306da18(param_3);

      *(uint *)(iVar1 + 0x14) = (uVar3 & 0x1ff) + *(int *)(iVar1 + 0x14);

      return uVar4;

    }

    if ((int)param_1 < 1) {

      if (-1 < (int)param_1) {

        return 0;

      }

LAB_0306de72:

      *(uint *)(iVar1 + 0x14) = uVar3;

      return 0;

    }

    *(uint *)(iVar1 + 0x14) = uVar3;

    iVar5 = *(int *)(iVar1 + 0x18);

    uVar6 = piVar2[iVar5];

    if (uVar3 <= uVar6) {

      return 0;

    }

LAB_0306ddea:

    *(uint *)(iVar1 + 0x14) = uVar3 - uVar6;

    piVar2[iVar5] = 0;

    *(int *)(iVar1 + 0x1c) = iVar5;

    *(int *)(iVar1 + 0x18) = 1 - iVar5;

    event_set(0x115);

    return 0;

  }

  uVar4 = HifiFileSeek(((int)param_1 / 0x200) * 0x200,param_2,param_3);

  FUN_0306da18(param_3);

  *(int *)(iVar1 + 0x14) = (int)param_1 % 0x200 + *(int *)(iVar1 + 0x14);

  return uVar4;

}
