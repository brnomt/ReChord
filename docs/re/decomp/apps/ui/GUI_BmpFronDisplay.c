/**
 * GUI_BmpFronDisplay @ 0x03012128
 * Tags: ui, gui
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined4 FUN_03012128(undefined4 param_1,undefined4 *param_2)



{

  int iVar1;

  undefined4 uVar2;

  undefined4 local_40;

  undefined4 local_3c;

  undefined2 local_38;

  undefined1 local_36;

  undefined1 auStack_30 [11];

  byte local_25;

  ushort local_1c;

  ushort local_16;

  

  local_40 = *param_2;

  local_3c = param_2[1];

  local_38 = *(undefined2 *)(param_2 + 2);

  local_36 = *(undefined1 *)((int)param_2 + 10);

  uVar2 = 0x13;

  iVar1 = FUN_02ff1166(auStack_30,param_1,&local_40);

  if ((iVar1 == 0) && ((local_25 & 0x10) == 0)) {

    uVar2 = 0x12;

    iVar1 = rom_i2s_master_config(param_1,param_2);

    if (7 < iVar1) {

      FUN_02ff42a0((uint)local_16 + (uint)local_1c * 0x10000);

      FUN_02ff4ed6(param_1,&local_40);

      uVar2 = 0;

    }

  }

  return uVar2;

}




