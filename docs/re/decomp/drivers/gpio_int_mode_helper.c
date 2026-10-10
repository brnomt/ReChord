/**
 * gpio_int_mode_helper @ 0x030202ce
 * Named via cross-version lineage (v3.4.0/v3.0.0 clusters); decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void gpio_int_mode_helper

               (int param_1,undefined4 param_2,int param_3,undefined4 param_4,uint param_5)



{

  undefined2 *puVar1;

  int iVar2;

  uint uVar3;

  undefined4 uVar4;

  int iVar5;

  uint extraout_r1;

  int unaff_r4;

  uint uVar6;

  uint uVar7;

  uint unaff_r8;

  int unaff_r9;

  int unaff_r10;

  int unaff_r11;

  undefined1 auStack_f0 [188];

  uint local_34;

  int local_30;

  undefined1 *local_2c;

  undefined4 local_28;

  

  local_28 = *DAT_03020430;

  uVar7 = *(uint *)(param_1 + 0x74);

  uVar6 = 0;

  if (uVar7 != 0) {

    do {

      iVar2 = bitreader_peek(param_2,9);

      if (iVar2 == 1) {

                    /* WARNING: Subroutine does not return */

        bitreader_refill(param_2,9);

      }

      iVar2 = SpiFlashDev_Read(param_2,0);

      if (iVar2 != 0) {

        *DAT_03020434 = 1;

      }

      local_2c = auStack_f0;

      uVar3 = FUN_0301f5aa(param_2);

      local_34 = uVar3 & 7;

      uVar4 = FUN_0301e8b2(param_2);

      iVar2 = FUN_0301f5ca(param_2,1);

      if (local_34 == 4) {

        iVar5 = FUN_0301e89a(param_2,2);

        param_3 = param_3 + *(int *)(DAT_03020438 + iVar5 * 4);

        if (param_3 < 0x20) {

          if (param_3 < 1) {

            param_3 = 1;

          }

        }

        else {

          param_3 = 0x1f;

        }

      }

      if (*(int *)(param_1 + 0x2c) != 0) {

        *DAT_03020434 = 1;

      }

      local_30 = param_3;

      FUN_030200f6(param_1,local_2c,uVar6,local_28,uVar4,uVar3 >> 4 | iVar2 << 2,param_2,param_3,

                   param_4,0);

      if (uVar6 != 0) {

        rom_dma_or_copy(DAT_0302043c + uVar6 * 0xc4 + -0xc4,DAT_0302043c + 0xf50,0xc4);

      }

      rom_dma_or_copy(DAT_03020440,auStack_f0,0xc4);

      uVar6 = uVar6 + 1;

    } while (uVar6 < uVar7);

  }

  rom_dma_or_copy(DAT_0302043c + uVar6 * 0xc4 + -0xc4,DAT_0302043c + 0xf50,0xc4);

  uVar7 = uVar7 & 0xfffffff;

  if ((int)unaff_r8 < unaff_r9) {

    uVar6 = unaff_r8;

    do {

      puVar1 = (undefined2 *)(unaff_r11 + (uVar6 - unaff_r8) * unaff_r10 * 2);

      for (uVar3 = param_5; (int)uVar3 < unaff_r4 + 1; uVar3 = uVar3 + 1 & 0xffff) {

        FUN_03005434(*puVar1,uVar7);

        puVar1 = puVar1 + 1;

        uVar7 = extraout_r1;

      }

      uVar6 = uVar6 + 1 & 0xffff;

    } while ((int)uVar6 < unaff_r9);

    return;

  }

  return;

}
