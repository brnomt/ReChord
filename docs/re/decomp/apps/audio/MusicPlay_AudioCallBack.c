/**
 * MusicPlay_AudioCallBack @ 0x0301e7dc
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void MusicPlay_AudioCallBack(int param_1)



{

  int iVar1;

  uint uVar2;

  

  if (*(int *)(param_1 + 0x18) != 0) {

    rom_dma_or_copy(*(undefined4 *)(param_1 + 0x14),*(undefined4 *)(param_1 + 0x10));

  }

  iVar1 = rkos_queue_send(*(int *)(param_1 + 0x14) + *(int *)(param_1 + 0x18),0x2000,param_1 + 0x20)

  ;

  do {

  } while (iVar1 == -1);

  uVar2 = iVar1 + *(int *)(param_1 + 0x18);

  *(uint *)(param_1 + 0x18) = uVar2;

  if (uVar2 < 4) {

    *(undefined4 *)(param_1 + 0x18) = 4;

  }

  *(undefined4 *)(param_1 + 0x10) = *(undefined4 *)(param_1 + 0x14);

  return;

}
