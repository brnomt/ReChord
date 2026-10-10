/**
 * hifi_busy_delay_ovl_0e48 @ 0x030e4808
 * Tags: os, overlay (**body unsound**)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* BOUNDARY UNSOUND: entry 030e4808 but Ghidra body spans 030e3ba8-030e482b. True delay body is

   small at entry. Recreate via delete+create_function when approved. */



void hifi_busy_delay_ovl_0e48(int param_1)



{

  uint uVar1;

  bool bVar2;

  

  uVar1 = (uint)((ulonglong)DAT_030e488c *

                 (ulonglong)((*(uint *)(DAT_030e4884 + 0x20) / DAT_030e4888) * param_1) >> 0x22);

  if (uVar1 == 0) {

    return;

  }

  do {

    bVar2 = uVar1 != 0;

    uVar1 = uVar1 - 1;

  } while (bVar2 && uVar1 != 0);

  return;

}
