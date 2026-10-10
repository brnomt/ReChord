/**
 * event_clear @ 0x03073882
 * Tags: os, event
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void event_clear(uint param_1)



{

  *(uint *)(DAT_030738bc + (param_1 >> 5) * 4) =

       *(uint *)(DAT_030738bc + (param_1 >> 5) * 4) & ~(1 << (param_1 & 0x1f));

  return;

}
