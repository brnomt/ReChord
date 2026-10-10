/**
 * flac_decode_blocksize @ 0x030e094c
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void flac_decode_blocksize(undefined4 *param_1,undefined4 *param_2,uint param_3)



{

  undefined4 *puVar1;

  undefined4 *puVar2;

  int iVar3;

  

  if ((int)param_3 < 1) {

    return;

  }

  puVar1 = param_1 + -1;

  puVar2 = param_2 + -1;

  if ((param_3 & 1) != 0) {

    *param_2 = *param_1;

    puVar1 = param_1;

    puVar2 = param_2;

  }

  iVar3 = (int)param_3 >> 1;

  if (iVar3 == 0) {

    return;

  }

  do {

    puVar2[1] = puVar1[1];

    puVar1 = puVar1 + 2;

    puVar2 = puVar2 + 2;

    *puVar2 = *puVar1;

    iVar3 = iVar3 + -1;

  } while (iVar3 != 0);

  return;

}
