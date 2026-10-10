/**
 * DSD_BufferReady @ 0x030ff2d8
 * Tags: codec, dsd
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


bool DSD_BufferReady(void)



{

  return *(uint *)(DAT_030ff6d4 + 0x2c) <= *(uint *)(DAT_030ff6d4 + 0x30);

}
