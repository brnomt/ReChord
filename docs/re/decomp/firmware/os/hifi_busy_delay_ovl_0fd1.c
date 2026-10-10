/**
 * hifi_busy_delay_ovl_0fd1 @ 0x030fd100
 * Tags: os, overlay (**body unsound**)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* BOUNDARY UNSOUND: entry 030fd100 but Ghidra body spans 030fc598-030fd123 (~0xb68 before entry).

   True delay body is ~030fd100-030fd123. Recreate via delete+create_function when approved. */



void hifi_busy_delay_ovl_0fd1(int param_1)



{

  uint uVar1;

  bool bVar2;

  

  uVar1 = (uint)((ulonglong)DAT_030fd184 *

                 (ulonglong)((*(uint *)(DAT_030fd17c + 0x20) / DAT_030fd180) * param_1) >> 0x22);

  if (uVar1 == 0) {

    return;

  }

  do {

    bVar2 = uVar1 != 0;

    uVar1 = uVar1 - 1;

  } while (bVar2 && uVar1 != 0);

  return;

}
