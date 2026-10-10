/**
 * hifi_busy_delay @ 0x0306c2e8
 * Tags: os, utility
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void hifi_busy_delay(int param_1)



{

  uint uVar1;

  bool bVar2;

  

  uVar1 = (uint)((ulonglong)DAT_0306c36c *

                 (ulonglong)((*(uint *)(DAT_0306c364 + 0x20) / DAT_0306c368) * param_1) >> 0x22);

  if (uVar1 == 0) {

    return;

  }

  do {

    bVar2 = uVar1 != 0;

    uVar1 = uVar1 - 1;

  } while (bVar2 && uVar1 != 0);

  return;

}
