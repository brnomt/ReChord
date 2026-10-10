/**
 * event_test_and_clear @ 0x0307385c
 * Tags: os, event
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 event_test_and_clear(uint param_1)



{

  uint uVar1;

  uint uVar2;

  

  uVar2 = *(uint *)(DAT_030738bc + (param_1 >> 5) * 4);

  uVar1 = 1 << (param_1 & 0x1f);

  if ((uVar2 & uVar1) != 0) {

    *(uint *)(DAT_030738bc + (param_1 >> 5) * 4) = uVar2 & ~uVar1;

    return 1;

  }

  return 0;

}
