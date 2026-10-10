/**
 * shell_short_handler @ 0x03002048
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void shell_short_handler(short *param_1)



{

  short sVar1;

  ushort uVar2;

  uint uVar3;

  uint uVar4;

  int iVar5;

  uint uVar6;

  int iVar7;

  int iVar8;

  bool bVar9;

  

  sVar1 = *param_1;

  *DAT_03002180 = param_1[1];

  uVar3 = DAT_03002184;

  if (sVar1 == 0) {

    return;

  }

  *(short *)(DAT_03002184 + 10) = param_1[7];

  uVar4 = 0;

  do {

    iVar7 = uVar3 + uVar4 * 2;

    *(short *)(iVar7 + 2) = param_1[uVar4 + 3];

    *(short *)(iVar7 + 0xc) = param_1[uVar4 + 8];

    uVar4 = uVar4 + 1 & 0xffff;

  } while (uVar4 <= *(ushort *)(uVar3 + 10));

  sVar1 = *DAT_03002150;

  uVar4 = (uint)*(ushort *)(uVar3 + 10);

  if (sVar1 == 8 || sVar1 == 0xc) {

    uVar4 = uVar3 + uVar4 * 2;

  }

  if (sVar1 == 8 || sVar1 == 0xc) {

    *(short *)(uVar4 + 0xc) = *(short *)(DAT_03002188 + 6);

  }

  else {

    sVar1 = *(short *)(DAT_0300214c + uVar4 * 2 + 4);

    if (sVar1 == 0x600) {

      if (uVar4 != 0) {

        *(undefined2 *)(uVar3 + 0x16) = 0;

        *(undefined2 *)(uVar3 + uVar4 * 2 + 0xc) = 0;

        goto LAB_030020e8;

      }

    }

    else if (sVar1 == 0x500) {

      bVar9 = uVar4 == 1;

      if (bVar9) {

        uVar4 = DAT_03002184;

      }

      if (bVar9) {

        *(undefined2 *)(uVar4 + 0xe) = 0;

        goto LAB_030020e8;

      }

    }

    else if (sVar1 == 0x700) {

      bVar9 = uVar4 != 0;

      if (bVar9) {

        uVar4 = uVar3 + uVar4 * 2;

      }

      if (bVar9) {

        *(undefined2 *)(uVar4 + 0xc) = 0;

        goto LAB_030020e8;

      }

    }

    *(short *)(uVar3 + uVar4 * 2 + 0xc) = *(short *)(DAT_03002188 + 6) + -1;

  }

LAB_030020e8:

  iVar7 = DAT_0300218c;

  iVar8 = uVar3 + (uint)*(ushort *)(uVar3 + 10) * 2;

  uVar2 = *(ushort *)(iVar8 + 0xc);

  uVar4 = (uint)uVar2;

  uVar6 = *(uint *)(DAT_0300218c + 8);

  if (uVar4 == uVar6) {

LAB_03002124:

    if (5 < *(uint *)(iVar7 + 4)) {

      if (uVar4 < 5) {

        *(ushort *)(uVar3 + 0x16) = uVar2;

      }

      else {

        *(undefined2 *)(uVar3 + 0x16) = 5;

      }

      goto LAB_03002130;

    }

  }

  else if ((int)uVar4 < (int)uVar6) {

    iVar5 = *(int *)(DAT_0300218c + 4) - (uVar6 - uVar4);

    *(int *)(DAT_0300218c + 4) = iVar5;

    if (-1 < iVar5) goto LAB_03002124;

    *(undefined4 *)(iVar7 + 4) = 0;

  }

  else {

    iVar5 = *(int *)(DAT_0300218c + 4) + (uVar4 - uVar6);

    *(int *)(DAT_0300218c + 4) = iVar5;

    if (iVar5 < 6) goto LAB_03002124;

    *(undefined4 *)(iVar7 + 4) = 5;

  }

  *(undefined2 *)(uVar3 + 0x16) = *(undefined2 *)(iVar7 + 4);

LAB_03002130:

  *(uint *)(iVar7 + 8) = (uint)*(ushort *)(iVar8 + 0xc);

  return;

}
