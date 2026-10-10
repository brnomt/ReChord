/**
 * flac_decode_frame @ 0x030df9fa
 * Tags: codec, flac, decoder
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 flac_decode_frame(int param_1,int param_2,int param_3)



{

  undefined4 uVar1;

  int iVar2;

  int iVar3;

  int iVar4;

  int iVar5;

  int iVar6;

  undefined4 local_ac;

  undefined1 auStack_a8 [132];

  

  iVar4 = param_1 + param_2 * 0x4800 + 0x48;

  if (*(int *)(DAT_030dfa94 + 4) == 0) {

    *(undefined4 *)(DAT_030dfa94 + 4) = 1;

  }

  hifi_flac_helper(param_1 + 0x20,0x809);

  iVar5 = 0;

  if (0 < param_3) {

    do {

      uVar1 = flac_bitstream_getbits_s(param_1 + 0x20,*(undefined4 *)(param_1 + 0x34));

      *(undefined4 *)(iVar4 + iVar5 * 4) = uVar1;

      iVar5 = iVar5 + 1;

    } while (iVar5 < param_3);

  }

  iVar5 = flac_bitstream_getbits_u(param_1 + 0x20,4);

  if (iVar5 + 1 == 0x10) {

    hifi_debug_printf_ovl_0dc5(DAT_030dfe88 + -0x20,DAT_030dfe88,0x1d7);

    return 0xffffffff;

  }

  iVar2 = flac_bitstream_getbits_s(param_1 + 0x20,5);

  if (iVar2 < 0) {

    hifi_debug_printf_ovl_0dc5(DAT_030dfa98 + -0x20,DAT_030dfa98,0x1df);

    return 0xffffffff;

  }

  iVar6 = 0;

  if (0 < param_3) {

    do {

      uVar1 = flac_bitstream_getbits_s(param_1 + 0x20,iVar5 + 1);

      iVar3 = param_3 - iVar6;

      iVar6 = iVar6 + 1;

      *(undefined4 *)(auStack_a8 + iVar3 * 4 + -4) = uVar1;

    } while (iVar6 < param_3);

  }

  iVar5 = hifi_flac_dec(param_1,param_2,param_3);

  if (-1 < iVar5) {

    if (param_3 == 1 || param_3 == 2) {

      local_ac = 0;

      flac_decode_subframe

                (iVar4,iVar4,*(undefined4 *)(param_1 + 0x30),*(undefined4 *)(param_1 + 8),auStack_a8

                 ,0,param_3,iVar2,0);

    }

    else {

      FUN_030e0bbc(0,*(undefined4 *)(param_1 + 0x30),0xa0000000);

      flac_hifi_ctrl_set(0);

      flac_hifi_cfg_write(0,param_3,iVar2);

      FUN_030e094c(auStack_a8,DAT_030dfe8c,param_3);

      FUN_030e0ba4(0,*(undefined4 *)(param_1 + 0x30),0xa0000000);

      FUN_030e0b12(iVar4,DAT_030dfe94,*(undefined4 *)(param_1 + 0x30),DAT_030dfe90,iVar4);

      do {

        iVar4 = FUN_030e097c(0,0x8000);

      } while (iVar4 != 0x8000);

      flac_hifi_ctrl_clear(0);

    }

    return 0;

  }

  hifi_debug_printf_ovl_0dc5(DAT_030dfe88 + -0x20,DAT_030dfe88,0x1f6);

  return 0xffffffff;

}
