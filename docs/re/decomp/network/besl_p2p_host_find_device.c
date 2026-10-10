/**
 * besl_p2p_host_find_device @ 0x03085ce2
 * Tags: network, p2p
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




void FUN_03085ce2(int param_1)



{

  int iVar1;

  int iVar2;

  int iVar3;

  int iVar4;

  

  if (*(int *)(param_1 + 0x118) < *(int *)(param_1 + 0x154)) {

    return;

  }

  iVar4 = *(int *)(param_1 + 0x154) >> 1;

  iVar3 = 0;

  iVar2 = *(int *)(param_1 + 0x9c);

  if (*(short *)(param_1 + 0x26) != 0) {

    do {

      iVar1 = ((iVar2 * 3) / 2) * iVar3 * 4 + (iVar2 >> 1) * 4 + *(int *)(param_1 + 0xdc) +

              iVar4 * -4;

      wma_memmove(iVar1,iVar1 + *(int *)(param_1 + 0x154) * 4,iVar4 << 2);

      iVar3 = iVar3 + 1;

    } while (iVar3 < (int)(uint)*(ushort *)(param_1 + 0x26));

  }

  *(int *)(param_1 + 0x118) = *(int *)(param_1 + 0x118) - *(int *)(param_1 + 0x154);

  *(int *)(param_1 + 0x11c) = *(int *)(param_1 + 0x11c) - *(int *)(param_1 + 0x154);

  return;

}




