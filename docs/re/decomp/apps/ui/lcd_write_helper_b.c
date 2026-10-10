/**
 * lcd_write_helper_b @ 0x03028628
 * Named via cross-version lineage (v3.4.0/v3.0.0 clusters); decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void lcd_write_helper_b(ushort *param_1,undefined2 *param_2)



{

  ushort uVar1;

  ushort uVar2;

  ushort uVar3;

  undefined2 uVar4;

  ushort *puVar5;

  int iVar6;

  uint uVar7;

  int iVar8;

  int iVar9;

  int iVar10;

  int iVar11;

  int iVar12;

  int iVar13;

  int iVar14;

  int iVar15;

  int iVar16;

  int iVar17;

  int iVar18;

  int iVar19;

  

  iVar14 = 7;

  puVar5 = param_1;

  do {

    iVar6 = (int)(short)puVar5[1];

    uVar1 = puVar5[2];

    iVar8 = (int)(short)puVar5[3];

    uVar2 = puVar5[4];

    iVar17 = (int)(short)puVar5[5];

    uVar3 = puVar5[6];

    iVar15 = (int)(short)puVar5[7];

    if (((((puVar5[1] == 0 && uVar1 == 0) && (puVar5[3] == 0 && uVar2 == 0)) && puVar5[5] == 0) &&

        uVar3 == 0) && puVar5[7] == 0) {

      uVar1 = *puVar5 << 2;

      *puVar5 = uVar1;

      puVar5[1] = uVar1;

      puVar5[2] = uVar1;

      puVar5[3] = uVar1;

      puVar5[4] = uVar1;

      puVar5[5] = uVar1;

      puVar5[6] = uVar1;

      puVar5[7] = uVar1;

    }

    else {

      iVar11 = ((int)(short)uVar1 + (int)(short)uVar3) * 0x1151;

      iVar10 = (short)uVar3 * DAT_030289d4 + iVar11;

      iVar11 = iVar11 + (short)uVar1 * 0x187e;

      iVar9 = ((uint)*puVar5 + (int)(short)uVar2) * 0x2000;

      iVar19 = ((uint)*puVar5 - (int)(short)uVar2) * 0x2000;

      iVar12 = iVar9 - iVar11;

      iVar13 = iVar19 + iVar10;

      iVar19 = iVar19 - iVar10;

      iVar9 = iVar9 + iVar11;

      iVar10 = (iVar15 + iVar8 + iVar17 + iVar6) * 0x25a1;

      iVar16 = (iVar15 + iVar6) * DAT_030289d8;

      iVar11 = (iVar17 + iVar6) * DAT_030289e0 + iVar10;

      iVar18 = (iVar17 + iVar8) * DAT_030289dc;

      iVar10 = (iVar15 + iVar8) * -0x3ec5 + iVar10;

      iVar15 = iVar15 * 0x98e + iVar16 + iVar10;

      iVar17 = iVar17 * 0x41b3 + iVar18 + iVar11;

      iVar8 = iVar8 * 0x6254 + iVar10 + iVar18;

      iVar6 = iVar6 * 0x300b + iVar11 + iVar16;

      *puVar5 = (ushort)((uint)((iVar9 + iVar6 + 0x400) * 0x20) >> 0x10);

      puVar5[7] = (ushort)((uint)(((iVar9 - iVar6) + 0x400) * 0x20) >> 0x10);

      puVar5[1] = (ushort)((uint)((iVar13 + iVar8 + 0x400) * 0x20) >> 0x10);

      puVar5[6] = (ushort)((uint)(((iVar13 - iVar8) + 0x400) * 0x20) >> 0x10);

      puVar5[2] = (ushort)((uint)((iVar19 + iVar17 + 0x400) * 0x20) >> 0x10);

      puVar5[5] = (ushort)((uint)(((iVar19 - iVar17) + 0x400) * 0x20) >> 0x10);

      puVar5[3] = (ushort)((uint)((iVar12 + iVar15 + 0x400) * 0x20) >> 0x10);

      puVar5[4] = (ushort)((uint)(((iVar12 - iVar15) + 0x400) * 0x20) >> 0x10);

    }

    puVar5 = puVar5 + 8;

    iVar14 = iVar14 + -1;

  } while (-1 < iVar14);

  iVar14 = 7;

  do {

    iVar6 = (int)(short)param_1[8];

    uVar1 = param_1[0x10];

    iVar8 = (int)(short)param_1[0x18];

    uVar2 = param_1[0x20];

    iVar17 = (int)(short)param_1[0x28];

    uVar3 = param_1[0x30];

    iVar15 = (int)(short)param_1[0x38];

    if (((((param_1[8] == 0 && uVar1 == 0) && (param_1[0x18] == 0 && uVar2 == 0)) &&

         param_1[0x28] == 0) && uVar3 == 0) && param_1[0x38] == 0) {

      iVar6 = ((short)*param_1 + 0x10 >> 5) + 0x80;

      if (iVar6 < 0) {

        iVar6 = 0;

      }

      else if (0xff < iVar6) {

        iVar6 = 0xff;

      }

      uVar4 = (undefined2)iVar6;

      *param_2 = uVar4;

      param_2[8] = uVar4;

      param_2[0x10] = uVar4;

      param_2[0x18] = uVar4;

      param_2[0x20] = uVar4;

      param_2[0x28] = uVar4;

      param_2[0x30] = uVar4;

      param_2[0x38] = uVar4;

    }

    else {

      iVar11 = ((int)(short)uVar1 + (int)(short)uVar3) * 0x1151;

      iVar10 = (short)uVar3 * DAT_030289d4 + iVar11;

      iVar11 = iVar11 + (short)uVar1 * 0x187e;

      iVar9 = ((int)(short)*param_1 + (int)(short)uVar2) * 0x2000;

      iVar18 = iVar9 + iVar11;

      iVar9 = iVar9 - iVar11;

      iVar12 = ((int)(short)*param_1 - (int)(short)uVar2) * 0x2000;

      iVar11 = iVar12 + iVar10;

      iVar12 = iVar12 - iVar10;

      iVar10 = (iVar15 + iVar8 + iVar17 + iVar6) * 0x25a1;

      iVar19 = (iVar15 + iVar6) * DAT_030289d8;

      iVar13 = (iVar17 + iVar6) * DAT_030289e0 + iVar10;

      iVar16 = (iVar17 + iVar8) * DAT_030289dc;

      iVar10 = (iVar15 + iVar8) * -0x3ec5 + iVar10;

      iVar15 = iVar15 * 0x98e + iVar19 + iVar10;

      iVar17 = iVar17 * 0x41b3 + iVar16 + iVar13;

      iVar8 = iVar8 * 0x6254 + iVar10 + iVar16;

      iVar6 = iVar13 + iVar19 + iVar6 * 0x300b;

      uVar7 = (iVar18 + iVar6 + 0x20000 >> 0x12) + 0x80;

      if ((uVar7 & 0xff00) != 0) {

        uVar7 = (~uVar7 & 0x7fffff) >> 0xf;

      }

      *param_2 = (short)uVar7;

      uVar7 = ((iVar18 - iVar6) + 0x20000 >> 0x12) + 0x80;

      if ((uVar7 & 0xff00) != 0) {

        uVar7 = (~uVar7 & 0x7fffff) >> 0xf;

      }

      param_2[0x38] = (short)uVar7;

      uVar7 = (iVar11 + iVar8 + 0x20000 >> 0x12) + 0x80;

      if ((uVar7 & 0xff00) != 0) {

        uVar7 = (~uVar7 & 0x7fffff) >> 0xf;

      }

      param_2[8] = (short)uVar7;

      uVar7 = ((iVar11 - iVar8) + 0x20000 >> 0x12) + 0x80;

      if ((uVar7 & 0xff00) != 0) {

        uVar7 = (~uVar7 & 0x7fffff) >> 0xf;

      }

      param_2[0x30] = (short)uVar7;

      uVar7 = (iVar12 + iVar17 + 0x20000 >> 0x12) + 0x80;

      if ((uVar7 & 0xff00) != 0) {

        uVar7 = (~uVar7 & 0x7fffff) >> 0xf;

      }

      param_2[0x10] = (short)uVar7;

      uVar7 = ((iVar12 - iVar17) + 0x20000 >> 0x12) + 0x80;

      if ((uVar7 & 0xff00) != 0) {

        uVar7 = (~uVar7 & 0x7fffff) >> 0xf;

      }

      param_2[0x28] = (short)uVar7;

      uVar7 = (iVar9 + iVar15 + 0x20000 >> 0x12) + 0x80;

      if ((uVar7 & 0xff00) != 0) {

        uVar7 = (~uVar7 & 0x7fffff) >> 0xf;

      }

      param_2[0x18] = (short)uVar7;

      uVar7 = ((iVar9 - iVar15) + 0x20000 >> 0x12) + 0x80;

      if ((uVar7 & 0xff00) != 0) {

        uVar7 = (~uVar7 & 0x7fffff) >> 0xf;

      }

      param_2[0x20] = (short)uVar7;

    }

    param_1 = param_1 + 1;

    param_2 = param_2 + 1;

    iVar14 = iVar14 + -1;

  } while (-1 < iVar14);

  return;

}
