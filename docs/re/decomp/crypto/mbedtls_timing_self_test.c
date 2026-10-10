/**
 * mbedtls_timing_self_test @ 0x030093f4
 * Tags: crypto, test
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




void FUN_030093f4(void)



{

  ushort uVar1;

  undefined2 uVar2;

  undefined2 uVar3;

  ushort *puVar4;

  char *pcVar5;

  uint uVar6;

  short *psVar7;

  byte *pbVar8;

  undefined2 *puVar9;

  undefined4 uVar10;

  undefined4 uVar11;

  int iVar12;

  uint uVar13;

  short sVar14;

  uint uVar15;

  uint uVar16;

  int iVar17;

  

  pbVar8 = DAT_030094bc;

  psVar7 = DAT_030094b8;

  uVar6 = DAT_030094b4;

  pcVar5 = DAT_030094b0;

  iVar12 = DAT_030094ac;

  puVar4 = DAT_0300949c;

  sVar14 = *(short *)(DAT_030094ac + 4);

  if ((sVar14 == 0) || (*(ushort *)(DAT_030094ac + 6) <= *(ushort *)(DAT_030094ac + 2)))

  goto LAB_030095f8;

  uVar15 = (*(ushort *)(DAT_030094ac + 6) & 0x7fff) << 1;

  uVar1 = *DAT_0300949c;

  iVar17 = (int)*DAT_030094b8;

  uVar13 = (uint)*DAT_030094bc;

  if (sVar14 == 1) {

    if (0xd1 < uVar15) {

      uVar15 = 0xd2;

    }

    if (uVar1 < 2) {

      uVar16 = uVar15 / 0x18;

      *DAT_030094b8 = 0;

      if (uVar16 == uVar13) {

        FUN_02ff73b4(0x96);

        FUN_03009156(0,(int)(short)-((short)uVar15 +

                                    (short)(uint)((ulonglong)uVar6 * (ulonglong)uVar15 >> 0x24) *

                                    -0x18),2);

      }

      else if (uVar13 < uVar16) {

        *puVar4 = uVar1 + 1;

        FUN_02ff73b4(0x96);

        FUN_03009156(0,0);

        *pbVar8 = (byte)uVar16;

      }

      else {

        FUN_02ff0c16();

      }

    }

    else {

      if (iVar17 == 0) {

        if (uVar15 != 0) {

          FUN_02ff0c16();

          *psVar7 = 1;

          uVar15 = 0;

        }

LAB_0300946e:

        uVar15 = (int)(uVar15 * 0x1c) / (int)*psVar7 & 0xffff;

      }

      else {

        if ((int)uVar15 <= iVar17) goto LAB_0300946e;

        *DAT_030094b8 = (short)uVar15;

      }

      sVar14 = (short)uVar15;

      if (0x1b < uVar15) {

        sVar14 = 0x1c;

      }

      FUN_02ff73b4(0x96);

      FUN_03009156(0,(int)-sVar14);

    }

    *pcVar5 = '\x02';

    FUN_02ff73f6(0x21);

    *(undefined2 *)(iVar12 + 4) = 0;

  }

  else if (sVar14 == 2) {

    if (0xd1 < uVar15) {

      uVar15 = 0xd2;

    }

    if (uVar1 == 0) {

      if (iVar17 == 0) {

        if (uVar15 != 0) {

          FUN_02ff0c16();

          *psVar7 = 1;

          uVar15 = 0;

        }

LAB_030095a0:

        uVar15 = (int)(uVar15 * 0x1c) / (int)*psVar7 & 0xffff;

      }

      else {

        if ((int)uVar15 <= iVar17) goto LAB_030095a0;

        *DAT_030094b8 = (short)uVar15;

      }

      sVar14 = (short)uVar15;

      if (0x1b < uVar15) {

        sVar14 = 0x1c;

      }

      FUN_02ff73b4(0x96);

      FUN_03009156(0,(int)sVar14);

    }

    else {

      uVar16 = uVar15 / 0x18;

      *DAT_030094b8 = 0;

      if (uVar16 == uVar13) {

        FUN_02ff73b4(0x96);

        FUN_03009156(0,(int)(short)((short)uVar15 +

                                   (short)(uint)((ulonglong)uVar6 * (ulonglong)uVar15 >> 0x24) *

                                   -0x18),1);

      }

      else if (uVar13 < uVar16) {

        *puVar4 = uVar1 - 1;

        FUN_02ff73b4(0x96);

        FUN_03009156(0,0);

        *pbVar8 = (byte)uVar16;

      }

      else {

        FUN_02ff0c16();

      }

    }

    *pcVar5 = '\x01';

    FUN_02ff73f6(0x21);

    *(undefined2 *)(iVar12 + 4) = 0;

  }

  FUN_02ff4124();

LAB_030095f8:

  puVar9 = DAT_03009744;

  if (*pcVar5 != '\0') {

    return;

  }

  uVar2 = *DAT_03009744;

  uVar3 = DAT_03009744[1];

  uVar10 = FUN_02ff2b38(1);

  uVar11 = FUN_02ff2b14(2);

  *puVar9 = 0xffff;

  puVar9[1] = 0xffff;

  iVar12 = FUN_02ff73d0(0x32);

  if (iVar12 != 0) {

    FUN_02ff73b4(0x95);

  }

  iVar12 = FUN_02ff73d0(0x95);

  if (((iVar12 == 1) || (iVar12 = FUN_02ff73d0(0x26), iVar12 == 1)) ||

     (iVar12 = FUN_02ff73d0(0x2f), iVar12 != 0)) {

    FUN_02ff33e6(0x7a);

    *puVar9 = *DAT_03009748;

    FUN_02ff2be4(0,3,0x140,0x10);

    *puVar9 = 0xffff;

    FUN_02ff73b4(0x21);

    FUN_02ff73b4(0x96);

    FUN_02ff73b4(0x9b);

    FUN_02ff73b4(0x1a9);

    FUN_02ff2eda(1);

  }

  FUN_02ff2eda(0);

  FUN_03009156(0,0);

  FUN_02ff2b38(uVar10);

  puVar9[1] = uVar3;

  *puVar9 = uVar2;

  FUN_02ff2b14(uVar11);

  return;

}




