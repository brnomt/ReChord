/**
 * MscTestUnitReady @ 0x0300024e
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 MscTestUnitReady(void)



{

  undefined2 uVar1;

  int iVar2;

  uint *puVar3;

  uint uVar4;

  int iVar5;

  undefined1 unaff_r5;

  undefined4 unaff_r6;

  undefined2 *unaff_r7;

  undefined2 unaff_r8;

  int unaff_r9;

  char in_ZR;

  

  if (in_ZR == '\0') {

    rom_hw_init(0x17c);

    iVar2 = DAT_030005c8;

    iVar5 = DAT_030005c4;

    if (*(char *)(unaff_r9 + 0x6c) != '\0') {

      uVar1 = *(undefined2 *)(unaff_r9 + 0x91);

      *(undefined2 *)(DAT_030005c4 + 0xf2) = uVar1;

      *unaff_r7 = uVar1;

      uVar4 = 0;

      do {

        *(undefined2 *)(iVar2 + uVar4 * 2) = *(undefined2 *)(unaff_r9 + uVar4 * 2 + 0x93);

        uVar4 = uVar4 + 1 & 0xffff;

      } while (uVar4 < 0x80);

      *(undefined4 *)(iVar5 + 0x10) = DAT_030005cc;

      puVar3 = DAT_030005d0;

      *(undefined2 *)(iVar5 + 6) = *(undefined2 *)(unaff_r9 + 0x66);

      *puVar3 = (uint)*(ushort *)(unaff_r9 + 0x19f);

      func_0x02fdb37c();

      MscEpData_Delete(3);

      rom_hw_init(0x18c);

      return 1;

    }

  }

  else {

    *unaff_r7 = unaff_r8;

    if (*(short *)(unaff_r9 + 0x2e) == 0) {

      iVar5 = MscSendCSW(0x1dc);

      if ((iVar5 == 0) && (iVar5 = MscSendCSW(0x18c), iVar5 == 0)) {

        func_0x02fddd78(DAT_030005e0,DAT_030005dc);

      }

      else {

        rom_hw_init2(0x16f);

      }

    }

    else {

      *DAT_030005d8 = unaff_r5;

      MscEpData_Delete(2);

      unaff_r6 = 1;

    }

    rom_hw_init(0x18c);

  }

  return unaff_r6;

}
