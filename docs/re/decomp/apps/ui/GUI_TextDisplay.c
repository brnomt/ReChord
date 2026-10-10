/**
 * GUI_TextDisplay @ 0x03013378
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 GUI_TextDisplay(void)



{

  byte bVar1;

  undefined2 uVar2;

  int iVar3;

  int *piVar4;

  undefined4 *puVar5;

  undefined1 *puVar6;

  undefined2 *puVar7;

  uint *puVar8;

  int iVar9;

  int iVar10;

  uint uVar11;

  ushort uVar12;

  undefined4 uVar13;

  undefined2 local_48;

  undefined2 local_46;

  undefined2 local_44;

  

  iVar9 = MediaLib_GetTotalFiles_d(0x134);

  puVar6 = DAT_030136a4;

  piVar4 = DAT_03013688;

  iVar3 = DAT_03013684;

  if (iVar9 == 0) {

    iVar9 = MediaLib_GetTotalFiles_d(0x155);

    if ((iVar9 == 0) && (iVar9 = MediaLib_GetTotalFiles_b(0xe0), iVar9 != 0)) {

      iVar9 = MediaLib_GetTotalFiles_b(0xe1);

      if (iVar9 == 0) {

        BroMemSelKeyMenu_Handler();

      }

      MediaLib_GetTotalFiles(0x14d);

    }

    else {

      iVar9 = MediaLib_GetTotalFiles_d(0x155);

      if ((iVar9 == 0) && (iVar9 = MediaLib_GetTotalFiles_b(0xdf), iVar9 != 0)) {

        BroMemSelKeyMenu_Handler();

        *(undefined2 *)(iVar3 + 0x42) = 1;

        *puVar6 = 0;

        FUN_02feffdc(DAT_03013694);

        *DAT_030136a8 = (short)piVar4[0x7e];

        local_48 = 9;

        FUN_030011be(0xd,&local_48);

        return 1;

      }

    }

  }

  iVar9 = MediaLib_GetTotalFiles_b(0xe3);

  if (iVar9 != 0) {

    ShellBspMemory();

    MediaLib_GetTotalFiles(0x14d);

  }

  iVar9 = MediaLib_GetTotalFiles_b(0x15c);

  if ((iVar9 != 0) && ((short)piVar4[(uint)*(ushort *)(piVar4 + 0x73) * 0xe + 3] == 0)) {

    MediaLib_GetTotalFiles(0x14d);

    wiced_ip_up();

  }

  iVar9 = MediaLib_GetTotalFiles_b(300);

  puVar5 = DAT_03013694;

  if (iVar9 != 0) {

    iVar9 = FUN_02ff0020(*DAT_03013694,DAT_03013690);

    if (iVar9 == 1) {

      *puVar6 = 0;

    }

    uVar12 = 0;

    iVar9 = *piVar4;

    if (*(ushort *)((int)piVar4 + 0x1ee) != 0) {

      do {

        uVar12 = uVar12 + 1;

        iVar9 = *(int *)(iVar9 + 4);

      } while (uVar12 < *(ushort *)((int)piVar4 + 0x1ee));

    }

    if ((*(int *)(iVar9 + 8) == 0) &&

       ((bVar1 = *DAT_030136ac, bVar1 == 2 || ((1 < bVar1 && (bVar1 == 3)))))) {

      *(undefined4 *)(iVar9 + 8) = *(undefined4 *)(DAT_030136b0 + 0xd);

    }

    FUN_02fedb3c(*(int *)(iVar9 + 0xc) + 1,*(undefined4 *)(iVar9 + 8),local_48,1);

    FUN_02fefea8(DAT_03013698);

    MediaLib_GetTotalFiles(0x14d);

  }

  uVar13 = 0;

  iVar9 = MediaLib_GetTotalFiles_d(0x134);

  if ((iVar9 != 0) &&

     ((iVar9 = MediaLib_GetTotalFiles_d(0xdf), iVar9 != 0 ||

      (iVar9 = MediaLib_GetTotalFiles_d(0xe0), iVar9 != 0)))) {

    iVar9 = MediaLib_GetTotalFiles_b(0xdf);

    if (iVar9 != 0) {

      local_48 = 0x38;

      local_46 = 0x67;

      local_44 = 3;

      FUN_02fefe90(DAT_0301369c,DAT_03013698,&local_48);

      MediaLib_GetTotalFiles(300);

      MediaLib_GetTotalFiles_c(0x134);

    }

    iVar9 = MediaLib_GetTotalFiles_b(0xe0);

    if (iVar9 != 0) {

      MediaLib_GetTotalFiles(0x14d);

      MediaLib_GetTotalFiles_c(0x134);

    }

  }

  iVar9 = MediaLib_GetTotalFiles_b(0x156);

  if (iVar9 != 0) {

    iVar9 = FUN_02ff0020(*puVar5,DAT_03013690);

    if (iVar9 == 1) {

      *puVar6 = 0;

      FUN_02feff78(DAT_03013694,DAT_03013690);

    }

    GUI_TextDisplayBuff();

    FUN_02fefea8(DAT_03013698);

    *(undefined2 *)(iVar3 + 0x42) = 1;

    MediaLib_GetTotalFiles(0x14d);

  }

  iVar9 = MediaLib_GetTotalFiles_d(0x155);

  if ((iVar9 != 0) &&

     ((iVar9 = MediaLib_GetTotalFiles_d(0xdf), iVar9 != 0 ||

      (iVar9 = MediaLib_GetTotalFiles_d(0xe0), iVar9 != 0)))) {

    iVar9 = MediaLib_GetTotalFiles_b(0xdf);

    if (iVar9 != 0) {

      local_48 = 0x38;

      local_46 = 0x4a;

      local_44 = 3;

      FUN_02fefe90(DAT_0301369c,DAT_03013698,&local_48);

      MediaLib_GetTotalFiles(0x156);

      MediaLib_GetTotalFiles_c(0x155);

    }

    iVar9 = MediaLib_GetTotalFiles_b(0xe0);

    if (iVar9 != 0) {

      MediaLib_GetTotalFiles(0x14d);

      MediaLib_GetTotalFiles_c(0x155);

    }

  }

  iVar9 = MediaLib_GetTotalFiles_b(0x14);

  if (iVar9 != 0) {

    FUN_0301283e();

  }

  iVar10 = MediaLib_GetTotalFiles_b(0x15e);

  puVar7 = DAT_030136b8;

  iVar9 = DAT_030136b4;

  if ((iVar10 == 1) && (*(char *)(iVar3 + 0x6c) != '\0')) {

    uVar2 = *(undefined2 *)(iVar3 + 0x91);

    *(undefined2 *)(DAT_030136b4 + 0xf2) = uVar2;

    *puVar7 = uVar2;

    iVar10 = DAT_030136bc;

    uVar11 = 0;

    do {

      *(undefined2 *)(iVar10 + uVar11 * 2) = *(undefined2 *)(iVar3 + uVar11 * 2 + 0x93);

      uVar11 = uVar11 + 1;

    } while (uVar11 < 0x80);

    *(undefined4 *)(iVar9 + 0x10) = DAT_030136c0;

    puVar8 = DAT_030136c4;

    *(undefined2 *)(iVar9 + 6) = *(undefined2 *)(iVar3 + 0x66);

    *puVar8 = (uint)*(ushort *)(iVar3 + 0x19f);

    FUN_02fed494();

    local_48 = *(undefined2 *)(iVar3 + 0x66);

    local_46 = 3;

    FUN_030011be(3,&local_48);

    uVar13 = 1;

  }

  return uVar13;

}
