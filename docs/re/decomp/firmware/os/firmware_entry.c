/**
 * firmware_entry @ 0x03000010
 * Tags: init, entry
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void firmware_entry(short *param_1)



{

  int iVar1;

  

  boot_param_layout();

  iVar1 = rom_alloc(0x1dc);

  if (iVar1 == 0) {

    rom_hw_init2(0x16f);

  }

  else {

    rom_hw_init(0x1dc);

    rom_hw_init(0x16f);

    rom_hw_init(0x16f);

    rom_hw_init2(0x171);

    rom_hw_init(0x170);

  }

  rom_early_init();

  if (*param_1 != 0xb) {

    rom_hw_init2(399);

    return;

  }

  rom_hw_init2(0x191);

  return;

}
