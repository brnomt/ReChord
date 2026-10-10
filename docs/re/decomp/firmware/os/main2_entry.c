/**
 * main2_entry @ 0x03000aba
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 main2_entry(void)



{

  short sVar1;

  int iVar2;

  byte *pbVar3;

  undefined4 *puVar4;

  undefined2 *puVar5;

  int iVar6;

  undefined4 *puVar7;

  undefined4 *puVar8;

  int iVar9;

  int iVar10;

  uint uVar11;

  undefined4 uVar12;

  byte bVar13;

  uint uVar14;

  bool bVar15;

  undefined1 local_48 [36];

  

  iVar9 = FUN_02fe896e();

  puVar8 = DAT_03000e68;

  puVar7 = DAT_03000e64;

  iVar6 = DAT_03000e58;

  puVar5 = DAT_03000e54;

  puVar4 = DAT_03000e50;

  pbVar3 = DAT_03000e44;

  iVar2 = DAT_03000e40;

  iVar10 = iVar9 - DAT_03000e4c;

  if (iVar9 == DAT_03000e4c) goto LAB_03000d8e;

  if (iVar9 < DAT_03000e4c) {

    iVar10 = iVar9 - DAT_03000e5c;

    if (iVar9 == DAT_03000e5c) goto LAB_03000c68;

    if (iVar9 < DAT_03000e5c) {

      iVar9 = iVar9 + -0x20000400;

      if (iVar9 == 0) goto LAB_03000c68;

      if (iVar9 != 0x400) {

        if (iVar9 == 0xc00) goto LAB_03000b76;

        if (iVar9 + DAT_03000e60 != 0) {

          return 0;

        }

      }

    }

    else {

      if (iVar10 == 8) {

LAB_03000bc4:

        if (*(byte *)(DAT_03000e40 + 0x12) < *(byte *)(DAT_03000e40 + 0x1e)) {

          *(byte *)(DAT_03000e40 + 0x12) = *(byte *)(DAT_03000e40 + 0x12) + 1;

        }

        iVar10 = func_0x02fddf08(*puVar4,DAT_03000e6c);

        goto joined_r0x03000c22;

      }

      if (iVar10 == 0x18) {

LAB_03000be0:

        if (*(char *)(DAT_03000e40 + 0x12) != '\0') {

          *(char *)(DAT_03000e40 + 0x12) = *(char *)(DAT_03000e40 + 0x12) + -1;

        }

        iVar10 = func_0x02fddf08(*puVar4,DAT_03000e6c);

        goto joined_r0x03000c22;

      }

      if (iVar10 == 0x3f8) goto LAB_03000c68;

      if (iVar10 != 0x7f8) {

        return 0;

      }

    }

LAB_03000d8e:

    iVar10 = MscSendCSW(1);

    if (iVar10 != 0) {

      rom_hw_init2(0x2f);

      return 0;

    }

    bVar13 = pbVar3[2];

    pbVar3[3] = bVar13;

    uVar11 = (uint)*pbVar3;

    if ((uint)pbVar3[5] == uVar11 - 1) {

      pbVar3[2] = 0;

      pbVar3[5] = 0;

      pbVar3[1] = 0;

      if (uVar11 < 7) {

        uVar12 = 0x171;

      }

      else {

        pbVar3[4] = 6;

        rom_hw_init(0x1d8);

        rom_hw_init(0x1d9);

        *puVar5 = 0;

        uVar12 = 0x170;

      }

      rom_hw_init2(uVar12);

    }

    else {

      if ((uint)bVar13 == pbVar3[4] - 1) {

        pbVar3[2] = 0;

        bVar13 = pbVar3[1] + 6;

        pbVar3[1] = bVar13;

        if (bVar13 + 5 < uVar11) {

          pbVar3[4] = 6;

        }

        else {

          pbVar3[4] = *pbVar3 - bVar13;

        }

        rom_hw_init(0x1d8);

        rom_hw_init(0x1d9);

        *puVar5 = 0;

        uVar12 = 0x170;

      }

      else {

        pbVar3[2] = bVar13 + 1;

        uVar12 = 0x171;

      }

      rom_hw_init2(uVar12);

      pbVar3[5] = pbVar3[5] + 1;

    }

    rom_hw_init2(399);

    rom_hw_init(0x154);

    sVar1 = *(short *)(iVar6 + (uint)pbVar3[5] * 6 + 4);

    if ((sVar1 != 0 && sVar1 != 10) && sVar1 != 6) {

LAB_03000ccc:

      rom_hw_init2(0x1dc);

LAB_03000cd4:

      uVar12 = WriteData_To_Flash(*(undefined2 *)(iVar6 + (uint)pbVar3[5] * 6 + 4));

      return uVar12;

    }

  }

  else {

    if (iVar10 == 0xffc) {

LAB_03000b76:

      iVar10 = MscSendCSW(0x1da);

      if ((iVar10 != 0) && (iVar10 = func_0x02fddf08(*puVar4,DAT_03000e6c), iVar10 == 1)) {

        MscEpData_Delete(3,0);

        return 1;

      }

LAB_03000b8c:

      iVar10 = rom_alloc(1);

      if (iVar10 != 0) {

        rom_hw_init2(0x2f);

        rom_hw_init(1);

        return 0;

      }

      local_48[0] = 0;

      MscEpData_Delete(0,local_48);

      return 1;

    }

    if (0xffc < iVar10) {

      if (iVar10 == 0x3ffc) {

        rom_hw_init(0x11c);

        *puVar8 = *puVar7;

        iVar10 = rom_alloc(1);

        if (iVar10 == 0) {

LAB_03000c64:

          uVar12 = 1;

          goto LAB_03000c76;

        }

        if (*(char *)(iVar2 + 0x12) != '\0') {

          *(char *)(iVar2 + 0x12) = *(char *)(iVar2 + 0x12) + -1;

        }

        iVar10 = func_0x02fddf08(*puVar4,DAT_03000e6c);

      }

      else {

        if (iVar10 != 0x7ffc) {

          bVar15 = iVar10 != 0xfffc;

          if (bVar15) {

            iVar10 = iVar10 + -0x10000ffc;

          }

          if (bVar15 && iVar10 != 0xf000) {

            return 0;

          }

          goto LAB_03000b8c;

        }

        rom_hw_init(0x11c);

        *puVar8 = *puVar7;

        iVar10 = rom_alloc(1);

        if (iVar10 == 0) goto LAB_03000c64;

        if (*(byte *)(iVar2 + 0x12) < *(byte *)(iVar2 + 0x1e)) {

          *(byte *)(iVar2 + 0x12) = *(byte *)(iVar2 + 0x12) + 1;

        }

        iVar10 = func_0x02fddf08(*puVar4,DAT_03000e6c);

      }

joined_r0x03000c22:

      if (iVar10 == 1) {

        GetFileType(0xff,0);

      }

      uVar12 = 0x3e;

      goto LAB_03000c76;

    }

    if (iVar10 != 4) {

      if (iVar10 != 0xc) {

        if (iVar10 != 0x1c) {

          if (iVar10 != 0x3c) {

            return 0;

          }

          goto LAB_03000cd4;

        }

        goto LAB_03000be0;

      }

      goto LAB_03000bc4;

    }

LAB_03000c68:

    iVar10 = MscSendCSW(1);

    if (iVar10 != 0) {

      uVar12 = 0x2f;

      goto LAB_03000c76;

    }

    bVar13 = pbVar3[2];

    pbVar3[3] = bVar13;

    if (pbVar3[5] == 0) {

      uVar11 = (uint)*pbVar3;

      uVar14 = uVar11 - 1 & 0xff;

      pbVar3[5] = (byte)(uVar11 - 1);

      if (uVar11 < 7) {

        uVar12 = 0x171;

      }

      else {

        uVar11 = ((uVar11 / 6) * 3 & 0x7f) * 2;

        pbVar3[1] = (byte)uVar11;

        if (uVar14 <= uVar11 && uVar11 - uVar14 != 0) {

          uVar11 = uVar11 - 6;

          pbVar3[1] = (byte)uVar11;

        }

        if ((int)(uVar14 - (uVar11 & 0xff)) < 7) {

          pbVar3[4] = *pbVar3 - (char)uVar11;

        }

        else {

          pbVar3[4] = 6;

        }

        rom_hw_init(0x1d8);

        rom_hw_init(0x1d9);

        *puVar5 = 0;

        uVar12 = 0x170;

      }

      rom_hw_init2(uVar12);

      pbVar3[2] = pbVar3[4] - 1;

    }

    else {

      if (bVar13 == 0) {

        pbVar3[2] = 5;

        pbVar3[1] = pbVar3[1] - 6;

        pbVar3[4] = 6;

        rom_hw_init(0x1d8);

        rom_hw_init(0x1d9);

        *puVar5 = 0;

        uVar12 = 0x170;

      }

      else {

        pbVar3[2] = bVar13 - 1;

        uVar12 = 0x171;

      }

      rom_hw_init2(uVar12);

      pbVar3[5] = pbVar3[5] - 1;

    }

    rom_hw_init(0x154);

    rom_hw_init2(399);

    sVar1 = *(short *)(iVar6 + (uint)pbVar3[5] * 6 + 4);

    if ((sVar1 != 0 && sVar1 != 10) && sVar1 != 6) goto LAB_03000ccc;

  }

  uVar12 = 0x16f;

LAB_03000c76:

  rom_hw_init2(uVar12);

  return 0;

}
