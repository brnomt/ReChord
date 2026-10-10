/**
 * hifi_memmove @ 0x0306d330
 * Tags: os, utility
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void hifi_memmove(undefined4 *param_1,undefined4 *param_2,uint param_3)



{

  undefined1 *puVar1;

  undefined1 *puVar2;

  undefined4 uVar3;

  bool bVar4;

  

  if (param_3 <= (uint)((int)param_1 - (int)param_2)) {

    if ((((uint)param_1 | (uint)param_2) & 3) == 0) {

      for (; 3 < param_3; param_3 = param_3 - 4) {

        uVar3 = *param_2;

        param_2 = param_2 + 1;

        *param_1 = uVar3;

        param_1 = param_1 + 1;

      }

    }

    while (bVar4 = param_3 != 0, param_3 = param_3 - 1, bVar4) {

      *(undefined1 *)param_1 = *(undefined1 *)param_2;

      param_1 = (undefined4 *)((int)param_1 + 1);

      param_2 = (undefined4 *)((int)param_2 + 1);

    }

    return;

  }

  puVar2 = (undefined1 *)((int)param_1 + param_3);

  puVar1 = (undefined1 *)((int)param_2 + param_3);

  while (bVar4 = param_3 != 0, param_3 = param_3 - 1, bVar4) {

    puVar1 = puVar1 + -1;

    puVar2 = puVar2 + -1;

    *puVar2 = *puVar1;

  }

  return;

}
