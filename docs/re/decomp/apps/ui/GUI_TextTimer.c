/**
 * GUI_TextTimer @ 0x030281e4
 * Structural-matcher sweep (post-boundary-repair, module-validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


bool GUI_TextTimer(uint param_1)



{

  byte bVar1;

  byte bVar2;

  int iVar3;

  undefined4 uVar4;

  int iVar5;

  int iVar6;

  int iVar7;

  int iVar8;

  int unaff_r4;

  int iVar9;

  int unaff_r5;

  int unaff_r6;

  int unaff_r8;

  uint uVar10;

  int unaff_r9;

  int unaff_r10;

  int unaff_r11;

  bool bVar11;

  undefined8 uVar12;

  int in_stack_00000000;

  int in_stack_00000004;

  uint in_stack_00000008;

  uint in_stack_0000000c;

  uint in_stack_00000010;

  uint in_stack_00000014;

  uint in_stack_00000018;

  uint in_stack_0000001c;

  uint uStack00000020;

  int in_stack_00000024;

  uint in_stack_00000028;

  uint in_stack_0000002c;

  int in_stack_00000030;

  int in_stack_00000038;

  uint uStack0000003c;

  undefined4 in_stack_00000040;

  undefined4 in_stack_00000044;

  int in_stack_00000048;

  int in_stack_0000004c;

  int in_stack_00000050;

  int in_stack_00000058;

  

  while( true ) {

    uStack00000020 = param_1 & 0xff;

    uVar4 = FUN_03011cac(unaff_r5,unaff_r6);

    iVar5 = FUN_03011cac(in_stack_0000002c << 0x10,uVar4);

    uVar4 = FUN_03011cac(unaff_r10,unaff_r6);

    iVar6 = FUN_03011cac(in_stack_00000028 << 0x10,uVar4);

    uVar4 = FUN_03011cac(unaff_r5,unaff_r9);

    iVar7 = FUN_03011cac(in_stack_0000001c << 0x10,uVar4);

    uVar4 = FUN_03011cac(unaff_r10,unaff_r9);

    iVar8 = FUN_03011cac(in_stack_00000018 << 0x10,uVar4);

    uVar12 = FUN_02ff8e94(iVar8 + iVar5 + iVar6 + iVar7);

    FUN_02ff8e32((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),0xfffffff0);

    FUN_02ff8f6c();

    bVar1 = FUN_02ff8ee0();

    uVar4 = FUN_03011cac(unaff_r5,unaff_r6);

    iVar5 = FUN_03011cac(in_stack_00000014 << 0x10,uVar4);

    uVar4 = FUN_03011cac(unaff_r10,unaff_r6);

    iVar6 = FUN_03011cac(in_stack_00000010 << 0x10,uVar4);

    uVar4 = FUN_03011cac(unaff_r5,unaff_r9);

    iVar7 = FUN_03011cac(in_stack_0000000c << 0x10,uVar4);

    uVar4 = FUN_03011cac(unaff_r10,unaff_r9);

    iVar8 = FUN_03011cac(in_stack_00000008 << 0x10,uVar4);

    uVar12 = FUN_02ff8e94(iVar8 + iVar5 + iVar6 + iVar7);

    FUN_02ff8e32((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),0xfffffff0);

    FUN_02ff8f6c();

    bVar2 = FUN_02ff8ee0();

    if ((((-1 < unaff_r8) && (unaff_r8 <= in_stack_00000050 * 2)) && (-1 < unaff_r4)) &&

       (unaff_r4 <= in_stack_00000000 * 2)) {

      *(ushort *)(in_stack_00000048 + unaff_r11 * 2) =

           (bVar2 & 0xf8) << 8 | (bVar1 & 0xfc) << 3 | (ushort)(uStack00000020 >> 3);

    }

    unaff_r11 = unaff_r11 + 1;

    if (in_stack_0000004c <= unaff_r11) break;

    uVar4 = FUN_02ff8e60();

    uStack00000020 = FUN_02ff8b68(uVar4,in_stack_00000040);

    unaff_r4 = FUN_02ff8eae();

    iVar6 = unaff_r4 * 4;

    iVar5 = DAT_030282b8[3];

    if (iVar5 < in_stack_00000024) {

      iVar7 = 0;

      if (in_stack_00000024 - iVar5 < 4) {

        iVar8 = iVar5 + 1;

        iVar7 = 4 - (in_stack_00000024 - iVar5);

      }

      else {

        iVar8 = in_stack_00000024 + -3;

      }

      for (iVar9 = 0; iVar9 < (iVar8 - iVar5) + -1; iVar9 = iVar9 + 1) {

        if ((uint)(DAT_030282b8[3] + iVar9) <= *(int *)(DAT_030282b0 + 0x10) - 1U) {

          GUI_SpectrumDisplay(in_stack_00000058);

        }

      }

      for (iVar5 = 0; iVar5 < iVar7; iVar5 = iVar5 + 1) {

        iVar9 = (int)*(short *)(DAT_030282b0 + 6);

        FUN_02ff89c2(DAT_030282b4 + iVar5 * iVar9 * 4,

                     DAT_030282b4 + iVar9 * ((iVar5 - iVar7) + 4) * 4,iVar9 << 2);

        DAT_030282b8[iVar5] = (iVar8 - iVar7) + iVar5;

      }

      for (iVar5 = 0; iVar9 = DAT_030282b0, iVar5 < (in_stack_00000024 - iVar8) + 1;

          iVar5 = iVar5 + 1) {

        if (*(int *)(DAT_030282b0 + 0x10) - 1U < (uint)(iVar8 + iVar5)) {

          if ((iVar7 < 1) && (iVar5 < 1)) goto LAB_0302830e;

          iVar9 = (int)*(short *)(DAT_030282b0 + 6);

          FUN_02ff89c2(DAT_030282b4 + iVar9 * (iVar7 + iVar5) * 4,

                       DAT_030282b4 + iVar9 * (iVar7 + iVar5 + -1) * 4,iVar9 << 2);

        }

        else {

          GUI_SpectrumDisplay(in_stack_00000058);

          iVar3 = (int)*(short *)(iVar9 + 6);

          FUN_02ff89c2(DAT_030282b4 + iVar3 * (iVar7 + iVar5) * 4,

                       *(undefined4 *)(*(int *)(iVar9 + 0x20) + 8),iVar3 << 2);

        }

        DAT_030282b8[iVar7 + iVar5] = iVar8 + iVar5;

      }

    }

    iVar5 = *DAT_030282b8 * in_stack_00000000 * -4 + DAT_030282b4;

    iVar8 = iVar5 + in_stack_00000038 + iVar6;

    bVar1 = *(byte *)(iVar8 + 3);

    iVar7 = iVar5 + in_stack_00000038 + iVar6 + 4;

    bVar11 = in_stack_00000004 + -1 < unaff_r4 + 1;

    uVar10 = (uint)*(byte *)(iVar7 + 3);

    if (bVar11) {

      uVar10 = (uint)bVar1;

    }

    iVar9 = iVar6 + in_stack_00000030 + iVar5;

    bVar2 = *(byte *)(iVar9 + 3);

    iVar5 = iVar5 + iVar6 + 4 + in_stack_00000030;

    uStack0000003c = (uint)*(byte *)(iVar5 + 3);

    if (bVar11) {

      uStack0000003c = (uint)bVar2;

    }

    in_stack_0000002c = (uint)*(byte *)(iVar8 + 2);

    in_stack_00000028 = (uint)*(byte *)(iVar7 + 2);

    if (bVar11) {

      in_stack_00000028 = in_stack_0000002c;

    }

    in_stack_0000001c = (uint)*(byte *)(iVar9 + 2);

    in_stack_00000018 = (uint)*(byte *)(iVar5 + 2);

    if (bVar11) {

      in_stack_00000018 = in_stack_0000001c;

    }

    in_stack_00000014 = (uint)*(byte *)(iVar8 + 1);

    in_stack_00000010 = (uint)*(byte *)(iVar7 + 1);

    if (bVar11) {

      in_stack_00000010 = in_stack_00000014;

    }

    in_stack_0000000c = (uint)*(byte *)(iVar9 + 1);

    in_stack_00000008 = (uint)*(byte *)(iVar5 + 1);

    if (bVar11) {

      in_stack_00000008 = in_stack_0000000c;

    }

    FUN_02ff8be8(in_stack_00000044,0x10);

    iVar5 = FUN_02ff8ee0();

    unaff_r6 = (unaff_r8 * 0x10000 - iVar5) + 0x10000;

    FUN_02ff8be8(uStack00000020,0x10);

    iVar6 = FUN_02ff8ee0();

    unaff_r5 = (unaff_r4 * 0x10000 - iVar6) + 0x10000;

    uVar4 = FUN_03011cac(unaff_r5,unaff_r6);

    iVar7 = FUN_03011cac((uint)bVar1 << 0x10,uVar4);

    unaff_r10 = iVar6 + unaff_r4 * -0x10000;

    uVar4 = FUN_03011cac(unaff_r10,unaff_r6);

    iVar6 = FUN_03011cac(uVar10 << 0x10,uVar4);

    unaff_r9 = iVar5 + unaff_r8 * -0x10000;

    uVar4 = FUN_03011cac(unaff_r5,unaff_r9);

    iVar5 = FUN_03011cac((uint)bVar2 << 0x10,uVar4);

    uVar4 = FUN_03011cac(unaff_r10,unaff_r9);

    iVar8 = FUN_03011cac(uStack0000003c << 0x10,uVar4);

    uVar12 = FUN_02ff8e94(iVar8 + iVar7 + iVar6 + iVar5);

    FUN_02ff8e32((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),0xfffffff0);

    FUN_02ff8f6c();

    param_1 = FUN_02ff8ee0();

  }

LAB_0302830e:

  iVar5 = *(int *)(in_stack_00000058 + 0x24) + 1;

  *(int *)(in_stack_00000058 + 0x24) = iVar5;

  bVar11 = iVar5 != *(int *)(in_stack_00000058 + 0x10);

  if (!bVar11) {

    *(undefined4 *)(DAT_030283e0 + 0x1c) = 0;

  }

  return bVar11;

}
