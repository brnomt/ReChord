/**
 * mbedtls_x509_dn_gets @ 0x03003df4
 * Tags: crypto, tls
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




void FUN_03003df4(void)



{

  ushort uVar1;

  ushort uVar2;

  int iVar3;

  int unaff_r6;

  undefined2 *unaff_r7;

  undefined2 *unaff_r8;

  undefined2 unaff_r10;

  bool bVar4;

  undefined2 in_stack_00000008;

  undefined4 in_stack_00000018;

  undefined4 in_stack_0000001c;

  

  uVar2 = 0;

  do {

    iVar3 = (int)(short)(uVar2 * 0x18 + 0x17);

    uVar1 = (ushort)*(byte *)(unaff_r6 + 3);

    bVar4 = uVar1 != uVar2;

    if (bVar4) {

      uVar1 = (ushort)*(byte *)(unaff_r6 + 4);

    }

    if (!bVar4 || uVar1 == uVar2) {

      if (*(byte *)(unaff_r6 + 4) == uVar2) {

        *unaff_r7 = *unaff_r8;

        FUN_02fecb06(0x7a,0,0,iVar3);

      }

      if (*(byte *)(unaff_r6 + 3) == uVar2) {

        *unaff_r7 = *DAT_03003ee4;

        FUN_02fecf6a(0x85,4,iVar3);

      }

      *unaff_r7 = *unaff_r8;

      FUN_02fec2cc(0x1c,iVar3 + 5U & 0xffff,in_stack_00000008,0x10);

    }

    uVar2 = uVar2 + 1;

  } while (uVar2 < 3);

  FUN_02fec5c2(0);

  iVar3 = FUN_02ff0afc(1);

  if ((iVar3 != 0) && (iVar3 = FUN_02ff0ab8(0x3e), iVar3 == 1)) {

    FUN_02fecfc0(*(byte *)(DAT_03003ed0 + 0x12) + 0xd2,0,0x50);

  }

  FUN_02fec220(in_stack_00000018);

  *unaff_r7 = unaff_r10;

  FUN_02fec1fc(in_stack_0000001c);

  return;

}




