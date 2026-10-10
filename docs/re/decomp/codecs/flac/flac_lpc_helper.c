/**
 * flac_lpc_helper @ 0x030dd9d0
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined8 flac_lpc_helper(uint param_1,uint param_2,uint param_3)



{

  uint uVar1;

  int iVar2;

  

  if ((int)param_3 < 0x20) {

    iVar2 = (int)param_2 >> (param_3 & 0xff);

    uVar1 = param_1 >> (param_3 & 0xff) | param_2 << (0x20 - param_3 & 0xff);

  }

  else {

    uVar1 = (int)param_2 >> (param_3 - 0x20 & 0xff);

    iVar2 = (int)(param_2 | uVar1) >> 0x1f;

  }

  return CONCAT44(iVar2,uVar1);

}
