/**
 * hifi_busy_delay_ovl_0ed6 @ 0x030ed650
 * Tags: os, overlay
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void hifi_busy_delay_ovl_0ed6(int param_1)



{

  uint uVar1;

  bool bVar2;

  

  uVar1 = (uint)((ulonglong)DAT_030ed6d4 *

                 (ulonglong)((*(uint *)(DAT_030ed6cc + 0x20) / DAT_030ed6d0) * param_1) >> 0x22);

  if (uVar1 == 0) {

    return;

  }

  do {

    bVar2 = uVar1 != 0;

    uVar1 = uVar1 - 1;

  } while (bVar2 && uVar1 != 0);

  return;

}
