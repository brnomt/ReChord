/**
 * hifi_busy_delay_ovl_0f53 @ 0x030f5360
 * Tags: os, overlay
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void hifi_busy_delay_ovl_0f53(int param_1)



{

  uint uVar1;

  bool bVar2;

  

  uVar1 = (uint)((ulonglong)DAT_030f53e4 *

                 (ulonglong)((*(uint *)(DAT_030f53dc + 0x20) / DAT_030f53e0) * param_1) >> 0x22);

  if (uVar1 == 0) {

    return;

  }

  do {

    bVar2 = uVar1 != 0;

    uVar1 = uVar1 - 1;

  } while (bVar2 && uVar1 != 0);

  return;

}
