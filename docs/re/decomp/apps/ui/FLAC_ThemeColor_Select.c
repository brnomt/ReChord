/**
 * FLAC_ThemeColor_Select @ 0x030054ba
 * Tags: ui, theme, flac
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */



void FLAC_ThemeColor_Select(short param_1,int param_2,uint param_3,int param_4)



{

  undefined4 uVar1;

  uint uVar2;

  uint uVar3;

  short unaff_r4;

  int unaff_r5;

  undefined2 unaff_r6;

  undefined4 unaff_r7;

  short unaff_r8;

  int in_r12;

  undefined2 in_stack_00000078;

  undefined2 uStack0000007a;

  undefined2 in_stack_0000007c;

  undefined2 uStack0000007e;

  

  *(short *)(unaff_r5 + param_4 * 2) =

       param_1 + (unaff_r8 - (short)((int)unaff_r7 >> 0x1f)) * -10 + 0x30;

  uVar3 = param_4 + 1U & 0xff;

  *(short *)(unaff_r5 + uVar3 * 2) = unaff_r4 + param_1 * -10 + 0x30;

  uVar3 = uVar3 + 1 & 0xff;

  *(undefined2 *)(unaff_r5 + uVar3 * 2) = 0x6b;

  uVar3 = uVar3 + 1 & 0xff;

  *(undefined2 *)(unaff_r5 + uVar3 * 2) = unaff_r6;

  uVar3 = uVar3 + 1 & 0xff;

  *(undefined2 *)(unaff_r5 + uVar3 * 2) = 0x70;

  uVar3 = uVar3 + 1 & 0xff;

  *(undefined2 *)(unaff_r5 + uVar3 * 2) = 0x73;

  uVar3 = uVar3 + 1 & 0xff;

  if ((in_r12 == 3 || in_r12 == 5) || (in_r12 == 6 || in_r12 == 7)) {

    if (param_3 < 0x18) {

      if (_FUN_03005604 <= param_2) goto LAB_03005540;

      if (0xac43 < param_2) {

        uVar1 = 0x13a;

        goto LAB_03005556;

      }

    }

    else if (0xac43 < param_2) {

LAB_03005540:

      uVar1 = 0x13b;

      goto LAB_03005556;

    }

  }

  uVar1 = 0x13d;

LAB_03005556:

  FUN_02fef2b2(uVar1,0xfc,0x76);

  uVar2 = uVar3 + 1 & 0xff;

  *(undefined2 *)(unaff_r5 + uVar3 * 2) = 0;

  *(undefined2 *)(unaff_r5 + uVar2 * 2) = 0;

  uVar3 = uVar2 + 1 & 0xff;

  *(undefined2 *)(unaff_r5 + uVar3 * 2) = 0;

  *(undefined2 *)(unaff_r5 + (uVar3 + 1 & 0xff) * 2) = 0;

  in_stack_00000078 = 0x1e;

  uStack0000007a = 0x76;

  in_stack_0000007c = 0x122;

  uStack0000007e = 0x85;

  FUN_02fee544(1);

  if (*(char *)(FUN_03005604 + 0x34f) == '\x04') {

    uRam0301d750 = 0xe162;

  }

  else {

    uRam0301d750 = 0x44de;

  }

                    /* WARNING: Read-only address (ram,0x0301d750) is written */

  FUN_02feda18(&stack0x00000078);

  FUN_02fee544(2);

  return;

}
