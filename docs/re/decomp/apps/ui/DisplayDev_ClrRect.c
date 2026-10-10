/**
 * DisplayDev_ClrRect @ 0x0301c978
 * Tags: ui, display
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




void FUN_0301c978(undefined4 param_1,ushort *param_2,int param_3,undefined4 param_4)



{

  char cVar1;

  uint uVar2;

  ushort *puVar3;

  uint uVar4;

  uint uVar5;

  ushort *local_30;

  int iStack_2c;

  undefined4 uStack_28;

  

  uStack_28 = param_4;

  iStack_2c = param_3;

  local_30 = param_2;

  cVar1 = (char)param_2[1];

  if (cVar1 == '\0') {

    cVar1 = *(char *)((int)param_2 + 3);

    for (uVar2 = 0; uVar2 < 3; uVar2 = uVar2 + 1) {

      puVar3 = (ushort *)bitstream_getbits_be_ovl_01c7(param_1,cVar1);

      (&local_30)[uVar2] = puVar3;

    }

  }

  else {

    uVar2 = bitstream_getbits_be_ovl_01c7(param_1,*(undefined1 *)((int)param_2 + 3));

    uVar5 = (uint)*param_2;

    for (uVar4 = 0; uVar4 < 3; uVar4 = uVar4 + 1) {

      (&local_30)[uVar4] = (ushort *)(uVar2 - uVar5 * (uVar2 / uVar5));

      uVar2 = uVar2 / uVar5;

    }

  }

  for (uVar2 = 0; uVar2 < 3; uVar2 = uVar2 + 1) {

    uVar4 = (uint)(&local_30)[uVar2] ^ 1 << (uint)(byte)(cVar1 - 1);

    *(int *)(param_3 + uVar2 * 4) =

         *(int *)(param_2 + 2) *

         (((int)((uVar4 | -(1 << (uint)(byte)(cVar1 - 1) & uVar4)) <<

                (uint)(byte)(0x1c - (cVar1 + -1))) >> 0xc) + *(int *)(param_2 + 4) * 0x10);

  }

  return;

}




