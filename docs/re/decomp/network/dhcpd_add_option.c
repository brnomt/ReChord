/**
 * dhcpd_add_option @ 0x0301a30e
 * Tags: network, dhcp
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined4 FUN_0301a30e(void)



{

  char cVar1;

  int iVar2;

  uint uVar3;

  undefined2 unaff_r4;

  char *unaff_r5;

  char unaff_r6;

  undefined4 unaff_r7;

  short *unaff_r8;

  undefined1 *unaff_r9;

  undefined1 unaff_r10;

  undefined3 uStack00000000;

  undefined2 uStack00000004;

  

  _uStack00000000 = 0x440038;

  uStack00000004 = 3;

  FUN_03008e98(0x134);

  FUN_02ff7d94(DAT_0301a4ac,DAT_0301a4a8);

  iVar2 = FUN_03008eb4(0xe3);

  if (iVar2 == 1) {

    uVar3 = *DAT_0301a464;

    *DAT_0301a464 = uVar3 + 1;

    if ((uint)*DAT_0301a4b0 == uVar3 + 1) {

      FUN_02ff7e7c(DAT_0301a49c,DAT_0301a498);

      iVar2 = FUN_03008ef8(0x157);

      if (iVar2 == 0) {

        uStack00000000 = CONCAT21(*(undefined2 *)(DAT_0301a4a0 + 0x1b1),unaff_r10);

      }

      else {

        uStack00000000 = CONCAT21(unaff_r4,6);

      }

      _uStack00000000 = CONCAT13((char)unaff_r4,uStack00000000);

      FUN_030090c2(9);

      return 1;

    }

    cVar1 = *DAT_0301a45c;

    *unaff_r5 = unaff_r6;

    *unaff_r9 = (char)unaff_r4;

    if (cVar1 == '\0') {

      FUN_0301ab96(0x122,1);

      FUN_03008e98(0x128);

    }

    else {

      FUN_0301ab96(0x122,0xffffffff);

      FUN_03008e98(0x128);

    }

  }

  if ((*unaff_r8 == 0) && (*unaff_r5 != '\0')) {

    FUN_03008e98(0x12d);

    FUN_03008e98(0x12a);

    FUN_03008e98(0x127);

  }

  iVar2 = FUN_03008ef8(0x21);

  cVar1 = '\0';

  if (iVar2 != 0) {

    cVar1 = *unaff_r5;

  }

  if (iVar2 != 0 && cVar1 != '\0') {

    FUN_03008e98(0x129);

  }

  return unaff_r7;

}




