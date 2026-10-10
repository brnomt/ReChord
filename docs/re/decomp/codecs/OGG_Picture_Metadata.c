/**
 * OGG_Picture_Metadata @ 0x030059d6
 * Tags: codec, media
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void OGG_Picture_Metadata(void)



{

  short *psVar1;

  int iVar2;

  undefined4 *puVar3;

  int unaff_r5;

  undefined4 unaff_r6;

  int unaff_r7;

  int *unaff_r8;

  int unaff_r9;

  

  if (*unaff_r8 != -1) {

    FUN_02fe3c0c((int)(char)*unaff_r8);

    *unaff_r8 = unaff_r5;

  }

  FUN_02fe75e6(&DAT_03005a70,s_PICTURE_OGG_03005a64);

  iVar2 = FUN_02fe373e(unaff_r9 + 0x14,*(undefined4 *)(unaff_r9 + 0x34),

                       *(int *)(unaff_r9 + 0x38) + -1);

  puVar3 = DAT_03005a78;

  *unaff_r8 = iVar2;

  if (iVar2 != -1) {

    iVar2 = FUN_0300af38(iVar2,DAT_03005a7c);

    if ((iVar2 == 0) ||

       (iVar2 = FUN_0300b548(*unaff_r8,*(undefined4 *)(unaff_r7 + 0x20)), psVar1 = DAT_03005e80,

       iVar2 == 0)) {

      *puVar3 = unaff_r6;

      FUN_02fe3c0c((int)(char)*unaff_r8);

      *unaff_r8 = unaff_r5;

      return;

    }

    *puVar3 = 1;

    puVar3 = DAT_03005e84;

    if ((*psVar1 < 0x3e9) && (*DAT_03005e88 < 0x3e9)) {

      FUN_02fef2b2(0x15c,0x11,0x16);

      FUN_02ff2c88(0x1a);

      FUN_0300b63c(*(undefined4 *)(unaff_r7 + 0x20));

      FUN_02ff2cd4(0x1a);

      FUN_02fe3c0c((int)(char)*unaff_r8);

      *unaff_r8 = unaff_r5;

      *puVar3 = 1;

      return;

    }

  }

  *puVar3 = unaff_r6;

  return;

}
