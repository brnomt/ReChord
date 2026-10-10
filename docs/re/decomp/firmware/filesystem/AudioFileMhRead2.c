/**
 * AudioFileMhRead2 @ 0x0306dc72
 * Tags: fs, audio
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int AudioFileMhRead2(int param_1,uint param_2,int param_3)



{

  undefined4 uVar1;

  uint uVar2;

  int iVar3;

  int iVar4;

  uint uVar5;

  int iVar6;

  int *piVar7;

  

  iVar6 = DAT_0306de10;

  iVar3 = DAT_0306de04;

  if (param_3 != 0) {

    if (*(int *)(DAT_0306de04 + 0x10) == 1) {

      hifi_busy_delay(1);

      iVar6 = DAT_0306de0c;

      do {

        if (*(int *)(iVar3 + 0x10) != 1) goto LAB_0306dcaa;

        hifi_busy_delay(1);

        iVar6 = iVar6 + -1;

      } while (iVar6 != 0);

      hifi_debug_printf(s_AudioFileMhRead2__timeout___0306de34);

    }

LAB_0306dcaa:

    iVar3 = HifiFileRead(param_1,param_2,(int)(char)param_3);

    return iVar3;

  }

  iVar3 = 0;

  if (param_2 == 0) {

    return 0;

  }

  piVar7 = (int *)(DAT_0306de10 + -0x20);

  while( true ) {

    iVar4 = *(int *)(iVar6 + -0xc);

    uVar5 = *(int *)(iVar6 + *(int *)(iVar6 + -8) * 4) - iVar4;

    if (param_2 < uVar5) break;

    hifi_memmove(param_1,*(int *)(iVar6 + -8) * *(int *)(iVar6 + -0x14) + iVar4 + *piVar7,uVar5);

    param_2 = param_2 - uVar5;

    uVar2 = *(int *)(iVar6 + -0xc) + uVar5;

    *(uint *)(iVar6 + -0xc) = uVar2;

    iVar3 = iVar3 + uVar5;

    param_1 = param_1 + uVar5;

    if (*(uint *)(iVar6 + *(int *)(iVar6 + -8) * 4) <= uVar2) {

      *(undefined4 *)(iVar6 + -0xc) = 0;

      if (*(int *)(iVar6 + -0x10) == 1) {

        hifi_busy_delay(1);

        iVar4 = DAT_0306de0c;

        do {

          if (*(int *)(iVar6 + -0x10) != 1) goto LAB_0306dc1e;

          hifi_busy_delay(1);

          iVar4 = iVar4 + -1;

        } while (iVar4 != 0);

        hifi_debug_printf(DAT_0306de14);

      }

LAB_0306dc1e:

      *(undefined4 *)(iVar6 + -0xc) = 0;

      iVar4 = *(int *)(iVar6 + -8);

      *(undefined4 *)(iVar6 + iVar4 * 4) = 0;

      *(int *)(iVar6 + -4) = iVar4;

      iVar4 = 1 - iVar4;

      *(int *)(iVar6 + -8) = iVar4;

      if (*(int *)(iVar6 + iVar4 * 4) == 0) {

        *(undefined1 *)(DAT_0306de08 + 1) = 1;

        uVar1 = HifiFileRead(iVar4 * *(int *)(iVar6 + -0x14) + *piVar7,*(int *)(iVar6 + -0x14),0);

        *(undefined4 *)(iVar6 + *(int *)(iVar6 + -8) * 4) = uVar1;

      }

      event_set(0x115);

      if (*(int *)(iVar6 + *(int *)(iVar6 + -8) * 4) == 0) {

        return iVar3;

      }

    }

  }

  hifi_memmove(param_1,*(int *)(iVar6 + -8) * *(int *)(iVar6 + -0x14) + *piVar7 + iVar4,param_2);

  uVar5 = *(int *)(iVar6 + -0xc) + param_2;

  *(uint *)(iVar6 + -0xc) = uVar5;

  if (uVar5 < *(uint *)(iVar6 + *(int *)(iVar6 + -8) * 4)) {

    return iVar3 + param_2;

  }

  *(undefined4 *)(iVar6 + -0xc) = 0;

  if (*(int *)(iVar6 + -0x10) == 1) {

    hifi_busy_delay(1);

    iVar4 = DAT_0306de0c;

    do {

      if (*(int *)(iVar6 + -0x10) != 1) goto LAB_0306db8c;

      hifi_busy_delay(1);

      iVar4 = iVar4 + -1;

    } while (iVar4 != 0);

    hifi_debug_printf(DAT_0306de14);

  }

LAB_0306db8c:

  *(undefined4 *)(iVar6 + -0xc) = 0;

  iVar4 = *(int *)(iVar6 + -8);

  *(undefined4 *)(iVar6 + iVar4 * 4) = 0;

  *(int *)(iVar6 + -4) = iVar4;

  iVar4 = 1 - iVar4;

  *(int *)(iVar6 + -8) = iVar4;

  if (*(int *)(iVar6 + iVar4 * 4) == 0) {

    *(undefined1 *)(DAT_0306de08 + 1) = 1;

    uVar1 = HifiFileRead(iVar4 * *(int *)(iVar6 + -0x14) + *piVar7,*(int *)(iVar6 + -0x14),0);

    *(undefined4 *)(iVar6 + *(int *)(iVar6 + -8) * 4) = uVar1;

  }

  event_set(0x115);

  return iVar3 + param_2;

}
