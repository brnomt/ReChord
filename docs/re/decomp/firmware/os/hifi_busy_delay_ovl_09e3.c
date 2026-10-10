/**
 * hifi_busy_delay_ovl_09e3 @ 0x0309e334
 * Tags: os, overlay (**body unsound**)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* BOUNDARY UNSOUND: entry 0309e334 but Ghidra body spans 0309d748-0309e357 (~0xbec before entry).

   True delay body is ~0309e334-0309e357. Do not trust decompile of the oversized range; recreate

   via delete+create_function when approved. */



void hifi_busy_delay_ovl_09e3(int param_1)



{

  uint uVar1;

  bool bVar2;

  

  uVar1 = (uint)((ulonglong)DAT_0309e3b8 *

                 (ulonglong)((*(uint *)(DAT_0309e3b0 + 0x20) / DAT_0309e3b4) * param_1) >> 0x22);

  if (uVar1 == 0) {

    return;

  }

  do {

    bVar2 = uVar1 != 0;

    uVar1 = uVar1 - 1;

  } while (bVar2 && uVar1 != 0);

  return;

}
