/**
 * AudioPause @ 0x0302a69e
 * Tags: audio, sdk match
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined4 FUN_0302a69e(void)



{

  int iVar1;

  short *psVar2;

  undefined2 *puVar3;

  int iVar4;

  int iVar5;

  

  psVar2 = DAT_0302a828;

  iVar1 = DAT_0302a80c;

  iVar5 = 20000;

  if (*DAT_0302a828 == 0) {

    *(undefined4 *)(DAT_0302a888 + 0x24) = 0;

    FUN_03005604(0);

    puVar3 = DAT_0302a850;

    *psVar2 = 3;

    *puVar3 = 4;

    iVar4 = rom_buffer_ready(5);

    while (iVar4 == 1) {

      dac_gain_curve_apply(1);

      iVar5 = iVar5 + -1;

      if (iVar5 == 0) break;

      iVar4 = rom_buffer_ready(5);

    }

    *(undefined1 *)(iVar1 + 0x11) = 0;

    iVar5 = FUN_0300c714(0x58);

    if (iVar5 != 0) {

      shared_mid_entry_c6f6(0x58);

    }

    *DAT_0302a81c = 0;

    *DAT_0302a820 = 0;

  }

  else {

    iVar4 = rom_buffer_ready(5);

    while (iVar4 == 1) {

      dac_gain_curve_apply(1);

      iVar5 = iVar5 + -1;

      if (iVar5 == 0) break;

      iVar4 = rom_buffer_ready(5);

    }

    *(undefined1 *)(iVar1 + 0x11) = 0;

  }

  iVar5 = FUN_02ff789c();

  if (iVar5 == 0) {

    func_0x0300ad1c();

  }

  else {

    FUN_0300a384();

  }

  if (*(char *)(DAT_0302a808 + 0x34a) != '\0') {

    shared_mid_entry_c6f6(0x1d5);

    shared_frame_epilogue_c6b4(0x1d4);

  }

  return 1;

}




