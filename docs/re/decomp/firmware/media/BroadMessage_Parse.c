/**
 * BroadMessage_Parse @ 0x030124a4
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void BroadMessage_Parse(undefined4 param_1)



{

  ushort uVar1;

  byte bVar2;

  longlong lVar3;

  short sVar4;

  short sVar5;

  int iVar6;

  undefined4 *puVar7;

  byte *pbVar8;

  int iVar9;

  uint uVar10;

  int iVar11;

  uint uVar12;

  uint uVar13;

  int iVar14;

  undefined1 auStack_6c [72];

  

  iVar6 = DAT_03012608;

  iVar9 = FUN_030121bc(*(undefined1 *)(DAT_03012608 + 4));

  *DAT_0301261c = *(undefined4 *)(DAT_03012618 + 0xd);

  uVar10 = FUN_02ff2cd4(param_1,auStack_6c,*(undefined4 *)(iVar9 + 4),*(undefined1 *)(iVar6 + 8));

  pbVar8 = DAT_03012620;

  puVar7 = DAT_0301260c;

  *(ushort *)(DAT_0301260c + 0x73) = (ushort)*DAT_03012620;

  uVar13 = 0;

  do {

    iVar14 = uVar13 * 0x47 + DAT_03012624;

    FUN_02fede0e(puVar7 + uVar13 * 0xe + 6,iVar14 + 0x1d,0x2a);

    puVar7[uVar13 * 0xe + 5] = *(undefined4 *)(iVar14 + 0x15);

    iVar11 = MediaLib_GetTotalFiles_d(0x15d);

    if (iVar11 == 0) {

      uVar1 = *(ushort *)(iVar14 + 4);

      uVar12 = (uint)uVar1 + (uint)((ulonglong)DAT_03012628 * (ulonglong)(uint)uVar1 >> 0x22) * -6;

      sVar4 = (short)uVar12;

      *(short *)((int)puVar7 + uVar13 * 2 + 0x1de) = sVar4;

      if ((*(short *)(iVar6 + 8) == 3) && ((uVar12 & 0xffff) != 0)) {

        *(short *)((int)puVar7 + uVar13 * 2 + 0x1de) = sVar4 + -1;

      }

      *(ushort *)((int)puVar7 + uVar13 * 2 + 0x1ce) = (uVar1 / 6) * 6;

    }

    uVar13 = uVar13 + 1 & 0xffff;

  } while (uVar13 <= *pbVar8);

  MediaLib_GetTotalFiles_c(0x15d);

  BroadMessage_Parse(*(undefined4 *)(iVar9 + 4),puVar7 + (uint)*(ushort *)(puVar7 + 0x73) * 0xe + 3,

                     *(undefined1 *)(iVar6 + 8));

  bVar2 = *pbVar8;

  uVar13 = (uint)*(ushort *)((int)puVar7 + (uint)bVar2 * 0x38 + 0xe);

  if (uVar13 < uVar10) {

    uVar10 = uVar13;

  }

  lVar3 = (longlong)DAT_0301262c *

          (longlong)(int)((*(ushort *)(puVar7 + (uint)bVar2 * 0xe + 4) + uVar10 & 0xffff) - 1);

  sVar5 = (short)((ulonglong)lVar3 >> 0x20) - (short)(lVar3 >> 0x3f);

  sVar4 = (short)(*(ushort *)(puVar7 + (uint)bVar2 * 0xe + 4) + uVar10) + sVar5 * -6 + -1;

  *(short *)((int)puVar7 + (uint)*(ushort *)(puVar7 + 0x73) * 2 + 0x1ce) = sVar5 * 6;

  *(short *)((int)puVar7 + 0x1ee) = sVar4;

  *(short *)(puVar7 + 0x7c) = sVar4;

  FUN_030122a2(*(undefined4 *)(puVar7[0x7d] + 4),*puVar7,

               puVar7 + (uint)*(ushort *)(puVar7 + 0x73) * 0xe + 3,

               *(undefined2 *)((int)puVar7 + (uint)*(ushort *)(puVar7 + 0x73) * 2 + 0x1ce),

               *(undefined1 *)(iVar6 + 8));

  return;

}
