/**
 * flac_decode_channelassign @ 0x030e0b12
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void flac_decode_channelassign

               (undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,

               undefined4 param_5)



{

  undefined4 *puVar1;

  

  puVar1 = DAT_030e0c0c;

  DAT_030e0c0c[1] = 0;

  *puVar1 = 0;

  FUN_030dc7f2(puVar1[3]);

  FUN_030dc93c(puVar1[2],param_1,param_2,param_3);

  FUN_030dc93c(puVar1[3],param_4,param_5,param_3);

  if (puVar1[1] == 0) {

    do {

      FUN_030de630();

    } while (puVar1[1] == 0);

    return;

  }

  return;

}
