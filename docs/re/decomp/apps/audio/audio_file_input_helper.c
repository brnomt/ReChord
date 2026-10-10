/**
 * audio_file_input_helper @ 0x0300d090
 * Named via cross-version lineage (v3.4.0/v3.0.0 clusters); decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint audio_file_input_helper(uint param_1,undefined4 param_2)



{

  ushort uVar1;

  uint uVar2;

  int iVar3;

  uint uVar4;

  uint uVar5;

  uint uVar6;

  undefined1 auStack_460 [1054];

  ushort auStack_42 [13];

  undefined2 local_28;

  undefined2 local_26;

  undefined2 local_24;

  undefined2 local_22;

  

  if (*DAT_0300d34c == 1) {

    uVar5 = 2;

  }

  else if (*DAT_0300d34c == 3) {

    uVar5 = (uint)*DAT_0300d350;

  }

  else {

    uVar5 = 0;

  }

  if ((uVar5 < param_1) && (param_1 <= (uint)DAT_0300d34c[1])) {

    uVar2 = audio_file_input_read(param_1,auStack_460);

    local_28 = 6;

    local_26 = 0x22;

    local_24 = 0xed;

    local_22 = 0x140;

    uVar1 = 0;

    uVar5 = uVar2;

    uVar6 = uVar2;

    do {

      iVar3 = wwd_bus_sdio_set_oob_interrupt(auStack_460,uVar5,&local_28,param_2);

      if (iVar3 == 0) {

        return uVar2;

      }

      uVar1 = uVar1 + 1;

      if (0xc < uVar1) {

        uVar6 = uVar6 + auStack_42[1];

      }

      uVar4 = 1;

      do {

        auStack_42[uVar4] = auStack_42[uVar4 + 1];

        uVar4 = uVar4 + 1 & 0xffff;

      } while (uVar4 < 0xc);

      auStack_42[0xc] = (ushort)iVar3;

      uVar5 = uVar5 + iVar3;

    } while (uVar5 < param_1);

    return uVar6;

  }

  return uVar5;

}
