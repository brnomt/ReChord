/**
 * hifi_busy_delay_ovl_0bff @ 0x030bff80
 * Tags: os, overlay
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void hifi_busy_delay_ovl_0bff(int param_1)



{

  uint uVar1;

  bool bVar2;

  

  uVar1 = (uint)((ulonglong)DAT_030c0004 *

                 (ulonglong)((*(uint *)(DAT_030bfffc + 0x20) / DAT_030c0000) * param_1) >> 0x22);

  if (uVar1 == 0) {

    return;

  }

  do {

    bVar2 = uVar1 != 0;

    uVar1 = uVar1 - 1;

  } while (bVar2 && uVar1 != 0);

  return;

}
