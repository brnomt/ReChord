/**
 * leds_cgi_helper @ 0x0300bc92
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void leds_cgi_helper(uint param_1)



{

  short sVar1;

  short sVar2;

  ushort *puVar3;

  ushort *puVar4;

  int iVar5;

  undefined4 *puVar6;

  ushort uVar7;

  int iVar8;

  int iVar9;

  uint uVar10;

  undefined1 auStack_a00 [2048];

  char local_200 [16];

  undefined4 local_1f0;

  int local_1e8;

  undefined1 auStack_1e0 [102];

  undefined1 auStack_17a [12];

  undefined1 auStack_16e [246];

  undefined2 auStack_78 [20];

  undefined4 local_50;

  undefined4 local_4c;

  undefined2 local_48;

  undefined2 local_46;

  undefined2 local_44;

  undefined2 local_42;

  undefined2 local_40;

  undefined2 local_3e;

  undefined2 local_3c;

  undefined2 local_3a;

  undefined4 local_34;

  undefined4 local_30;

  uint local_2c;

  

  local_34 = FUN_02ff5df4(2);

  local_4c = 0x2c0080;

  local_48 = 0x28;

  local_46 = 0x45;

  local_44 = 0x6d;

  local_42 = 0x70;

  local_40 = 0x74;

  local_3e = 0x79;

  local_50 = 0x1c001c;

  local_3c = 0x29;

  local_3a = 0;

  local_30 = FUN_02ff5e18(1);

  puVar4 = DAT_0300c0f4;

  puVar3 = DAT_0300c0f0;

  local_2c = (uint)*DAT_0300c0f0;

  *DAT_0300c0f0 = 0xffff;

  puVar3[1] = *puVar4;

  FUN_02ff66fe(0x7a,0,0,0x17,0x8f);

  uVar10 = 0;

  do {

    *DAT_0300c0f0 = 0xffff;

    sVar2 = (short)uVar10;

    opp_WriteFileTail(1,auStack_a00,0x800);

    RKBT_FRead(local_200,auStack_a00 + (short)(sVar2 + 1U) * 0x188,0x188);

    iVar9 = DAT_0300c0f8;

    if (local_200[0] == '\0') {

      iVar8 = FUN_02fe7598(DAT_0300c0f8 + 0x10,auStack_1e0);

      if (iVar8 == 0) {

        FUN_02fe7598(iVar9 + 0x76,auStack_17a);

      }

      iVar9 = FUN_0300d5ae(local_1e8 + 1,local_1f0,auStack_78);

      auStack_78[iVar9] = 0;

      iVar8 = FUN_0300ced6(auStack_78,auStack_16e,iVar9);

      auStack_78[iVar8] = 0;

      sVar1 = sVar2 * 0x18 + 0x1c;

      local_50 = CONCAT22(sVar1,(short)local_50);

      local_4c = CONCAT22(sVar2 * 0x18 + 0x2b,(short)local_4c);

      if (uVar10 == param_1) {

        uVar7 = FUN_02ff4f56(auStack_78);

        if ((int)(short)local_4c - (int)(short)local_50 < (int)(uint)uVar7) {

          FUN_02fe755c(DAT_0300c0fc,0x3a);

          iVar5 = DAT_0300c100;

          if (*(char *)(DAT_0300c100 + 5) == '\0') {

            *(undefined2 *)(DAT_0300c100 + 0x10) = 0;

            *(short *)(iVar5 + 0x12) = sVar1;

            *(undefined2 *)(iVar5 + 0x14) = 0;

            *(undefined2 *)(iVar5 + 0x16) = 0x11;

            *(undefined2 *)(iVar5 + 0x18) = 0x85;

            FUN_02ff5b58(&local_50,*(undefined4 *)(iVar5 + 0x10),*(undefined4 *)(iVar5 + 0x14),

                         *(undefined4 *)(iVar5 + 0x18),auStack_78,0x1e);

            RKBT_FRead(iVar5 + 0x50,auStack_78,iVar8 << 1);

            RKBT_FRead(iVar5 + 0x22,auStack_16e,(iVar8 - iVar9) * 2);

            *(int *)(iVar5 + 0xc) = local_1e8;

            *(undefined4 *)(iVar5 + 0x1a) = local_50;

            puVar6 = DAT_0300c104;

            *(undefined4 *)(iVar5 + 0x1e) = local_4c;

            *(undefined4 *)(iVar5 + 0x4c) = *puVar6;

            *(undefined1 *)(iVar5 + 5) = 1;

          }

        }

        FUN_02ff6b62(0x85,0,(int)(short)(sVar2 * 0x18 + 0x14));

      }

      FUN_02ff52c8(&local_50,&local_50,auStack_78,0);

    }

    else if (uVar10 == param_1) {

      FUN_02ff6b62(0x85,0,(int)(short)(sVar2 * 0x18 + 0x14));

      FUN_02ff51b8(0x1c,uVar10 * 0x18 + 0x1c,&local_48);

    }

    else {

      FUN_02ff51b8(0x1c,uVar10 * 0x18 + 0x1c,&local_48);

    }

    uVar10 = (uint)(ushort)(sVar2 + 1U);

  } while (uVar10 < 3);

  FUN_02ff5e18(local_30);

  *DAT_0300c0f0 = (ushort)local_2c;

  FUN_02ff5df4(local_34);

  return;

}
