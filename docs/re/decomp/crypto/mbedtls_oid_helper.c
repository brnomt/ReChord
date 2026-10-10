/**
 * mbedtls_oid_helper @ 0x03015318
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void mbedtls_oid_helper(int param_1)



{

  longlong lVar1;

  short sVar2;

  ushort *puVar3;

  int iVar4;

  undefined4 uVar5;

  int iVar6;

  undefined4 uVar7;

  uint uVar8;

  uint uVar9;

  int iVar10;

  undefined1 auStack_58 [16];

  undefined1 auStack_48 [16];

  uint local_38;

  undefined4 local_34;

  undefined4 local_30;

  uint local_2c;

  

  puVar3 = DAT_03015570;

  local_2c = (uint)*DAT_03015570;

  local_38 = (uint)DAT_03015570[1];

  local_34 = FUN_02ffef2c(1);

  local_30 = FUN_02ffef08(2);

  *puVar3 = *DAT_0301556c;

  puVar3[1] = *DAT_03015584;

  FUN_02fff7da(0x7a);

  FUN_02fff4de(0x8c,auStack_58);

  FUN_02fff4de(0x9e,auStack_48);

  iVar4 = DAT_03015588;

  if (*(char *)(DAT_03015560 + 0x357) == 'U') {

LAB_03015374:

    uVar9 = 1;

  }

  else {

    if (*(short *)(DAT_03015568 + 2) == 0xfe) {

      if (param_1 != 0) {

        uVar9 = 4;

        goto LAB_0301538a;

      }

    }

    else if (param_1 != 0) goto LAB_03015374;

    uVar9 = 6;

  }

LAB_0301538a:

  uVar8 = 0;

  iVar10 = DAT_03015588 + -0x30;

  do {

    if (uVar9 <= uVar8) break;

    sVar2 = (short)uVar8;

    lVar1 = (ulonglong)DAT_0301558c * (ulonglong)*(uint *)(DAT_03015568 + 0x14);

    uVar7 = (undefined4)lVar1;

    if (*(uint *)(DAT_03015568 + 0x14) + (uint)((ulonglong)lVar1 >> 0x22) * -6 == uVar8) {

      if (uVar8 == 0) {

        FUN_02fffc76(0x86,4,0x17,uVar7);

      }

      else {

        if (uVar8 == 5) {

          uVar5 = 0x88;

        }

        else {

          uVar5 = 0x85;

        }

        FUN_02fffc76(uVar5,4,(int)(short)(sVar2 * 0x18 + 0x17),uVar7);

      }

      if (*(char *)(DAT_03015560 + 0x357) == 'U') {

        FUN_02fffc76(*(undefined2 *)(iVar10 + uVar8 * 4 + 4),5,(int)(short)(sVar2 * 0x18 + 0x18));

      }

      else if (*(short *)(DAT_03015568 + 2) == 0xfe) {

        iVar6 = (int)(short)(sVar2 * 0x18 + 0x19);

        if (param_1 == 0) {

          FUN_02fffc76(*(undefined2 *)(iVar4 + uVar8 * 4),5,iVar6);

        }

        else {

          FUN_02fffc76(*(undefined2 *)(iVar4 + uVar8 * 4 + 0x18),5,iVar6);

        }

      }

      else {

        FUN_02fffc76(*(undefined2 *)(iVar10 + uVar8 * 4),5,(int)(short)(sVar2 * 0x18 + 0x18));

      }

      *DAT_03015570 = *DAT_03015590;

    }

    else {

      if (*(char *)(DAT_03015560 + 0x357) == 'U') {

        FUN_02fffc76(*(short *)(iVar10 + uVar8 * 4 + 4) + 1,5,(int)(short)(sVar2 * 0x18 + 0x18),

                     uVar7);

      }

      else if (*(short *)(DAT_03015568 + 2) == 0xfe) {

        iVar6 = (int)(short)(sVar2 * 0x18 + 0x19);

        if (param_1 == 0) {

          FUN_02fffc76(*(short *)(iVar4 + uVar8 * 4) + 1,5,iVar6,uVar7);

        }

        else {

          FUN_02fffc76(*(short *)(iVar4 + uVar8 * 4 + 0x18) + 1,5,iVar6,uVar7);

        }

      }

      else {

        FUN_02fffc76(*(short *)(iVar10 + uVar8 * 4) + 1,5,(int)(short)(sVar2 * 0x18 + 0x18),uVar7);

      }

      *DAT_03015570 = *DAT_0301556c;

    }

    if (*(char *)(DAT_03015560 + 0x357) == 'U') {

      FUN_02ffefd8(0x1c,uVar8 * 0x18 + 0x1c & 0xffff,100,0x10,0,

                   *(undefined2 *)(iVar10 + uVar8 * 4 + 6));

    }

    else if (*(short *)(DAT_03015568 + 2) == 0xfe) {

      iVar6 = iVar4 + uVar8 * 4;

      if (param_1 == 0) {

        FUN_02ffefd8(0x1c,uVar8 * 0x18 + 0x1c & 0xffff,100,0x10,0,*(undefined2 *)(iVar6 + 2));

      }

      else {

        FUN_02ffefd8(0x1c,uVar8 * 0x18 + 0x1c & 0xffff,100,0x10,0,*(undefined2 *)(iVar6 + 0x1a));

      }

    }

    else {

      FUN_02ffefd8(0x1c,uVar8 * 0x18 + 0x1c & 0xffff,100,0x10,0,

                   *(undefined2 *)(iVar10 + uVar8 * 4 + 2));

    }

    uVar8 = uVar8 + 1 & 0xffff;

  } while (uVar8 < 6);

  FUN_02ffef2c(local_34);

  puVar3 = DAT_03015570;

  DAT_03015570[1] = (ushort)local_38;

  *puVar3 = (ushort)local_2c;

  FUN_02ffef08(local_30);

  return;

}
