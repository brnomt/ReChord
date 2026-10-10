/**
 * flac_crc_update @ 0x030e0ba4
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void flac_crc_update(int param_1,int param_2,uint param_3)



{

  *(uint *)(param_1 * 0x3c + 0x106000c) = param_3 | param_2 << 0xf | 3;

  return;

}
