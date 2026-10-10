/**
 * event_set @ 0x03073840
 * Tags: os, event
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void event_set(uint param_1)



{

  *(uint *)(DAT_030738bc + (param_1 >> 5) * 4) =

       1 << (param_1 & 0x1f) | *(uint *)(DAT_030738bc + (param_1 >> 5) * 4);

  return;

}
