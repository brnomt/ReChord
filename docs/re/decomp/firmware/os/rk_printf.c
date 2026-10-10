/**
 * rk_printf @ 0x03000e08
 * Tags: os, logging
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined4 FUN_03000e08(uint param_1,char param_2,uint param_3)



{

  short sVar1;

  undefined2 unaff_r4;

  undefined1 unaff_r7;

  undefined2 *unaff_r8;

  undefined4 unaff_r9;

  int unaff_r10;

  int unaff_r11;

  

  *(char *)(unaff_r10 + 1) = param_2;

  if (param_3 < param_1) {

    *(undefined1 *)(unaff_r10 + 4) = unaff_r7;

  }

  else {

    *(char *)(unaff_r10 + 4) = (char)param_1 - param_2;

  }

  rom_hw_init(0x1d8);

  rom_hw_init(0x1d9);

  *unaff_r8 = unaff_r4;

  rom_hw_init2(0x170);

  *(char *)(unaff_r10 + 5) = *(char *)(unaff_r10 + 5) + '\x01';

  rom_hw_init2(399);

  rom_hw_init(0x154);

  sVar1 = *(short *)(unaff_r11 + (uint)*(byte *)(unaff_r10 + 5) * 6 + 4);

  if ((sVar1 == 0 || sVar1 == 10) || sVar1 == 6) {

    rom_hw_init2(0x16f);

  }

  else {

    rom_hw_init2(0x1dc);

    unaff_r9 = FUN_0300016c(*(undefined2 *)(unaff_r11 + (uint)*(byte *)(unaff_r10 + 5) * 6 + 4));

  }

  return unaff_r9;

}




