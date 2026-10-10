/**
 * OGGInfo_Parse @ 0x0301930e
 * Tags: codec, ogg
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined4 FUN_0301930e(void)



{

  int iVar1;

  ushort uVar2;

  short sVar3;

  ushort *puVar4;

  int iVar5;

  undefined4 uVar6;

  int iVar7;

  uint uVar8;

  int iVar9;

  undefined2 local_38 [18];

  

  iVar5 = FUN_02ffd29a();

  puVar4 = DAT_03019644;

  iVar7 = DAT_0301963c;

  iVar9 = iVar5 - DAT_03019654;

  if (iVar5 == DAT_03019654) {

LAB_030193ea:

    if (4 < *DAT_03019644) {

      return 0;

    }

    iVar7 = DAT_0301963c + (uint)*DAT_03019644 * 2;

    sVar3 = *(short *)(iVar7 + 0x77);

    if (sVar3 == 0) {

      return 0;

    }

    *(short *)(iVar7 + 0x77) = sVar3 + -1;

    func_0x030037a8(0x103);

    uVar6 = 0x1b2;

  }

  else {

    if (iVar5 < DAT_03019654) {

      iVar9 = iVar5 - DAT_03019658;

      if (iVar5 != DAT_03019658) {

        if (iVar5 < DAT_03019658) {

          if (iVar9 != -6) {

            if (iVar9 != -4) {

              return 0;

            }

            goto LAB_030193ea;

          }

        }

        else {

          if (iVar9 + DAT_0301965c == 0) goto LAB_0301940e;

          if (iVar9 + DAT_0301965c + DAT_03019660 != 0) {

            return 0;

          }

        }

        uVar2 = *DAT_03019644;

        *DAT_03019644 = uVar2 + 1;

        if (4 < (ushort)(uVar2 + 1)) {

          *puVar4 = 0;

        }

        uVar6 = 0xbb;

        goto LAB_030193e4;

      }

    }

    else {

      if (iVar9 == DAT_03019664) {

LAB_0301940e:

        iVar9 = ui_refresh_and_redraw(1);

        iVar5 = DAT_03019640;

        if (iVar9 == 0) {

          uVar8 = 0;

          do {

            iVar9 = uVar8 * 4;

            iVar1 = uVar8 * 2;

            uVar8 = uVar8 + 1 & 0xffff;

            *(undefined2 *)(iVar7 + iVar1 + 0x77) = *(undefined2 *)(iVar5 + iVar9);

          } while (uVar8 < 5);

          func_0x030037a8(0xba);

          *puVar4 = 0;

          FUN_02ff26bc(DAT_03019668);

          func_0x030037a8(199);

          return 0;

        }

LAB_0301937e:

        func_0x030037a8(0x2f);

        ui_post_redraw(1);

        return 0;

      }

      if (DAT_03019664 <= iVar9) {

        if (iVar9 - DAT_03019664 == 0xfff000) {

          local_38[0] = 0;

          func_0x030037a8(0xd1);

          FUN_02ff26a4(DAT_03019668,DAT_0301966c,local_38);

          return 0;

        }

        if (iVar9 - DAT_03019664 != 0x1fff000) {

          return 0;

        }

        local_38[0] = 1;

        func_0x030037a8(0xd1);

        FUN_02ff26a4(DAT_03019668,DAT_0301966c,local_38);

        return 0;

      }

      if (iVar9 != 4) {

        if (iVar9 != 0x1000003c) {

          return 0;

        }

        iVar5 = ui_refresh_and_redraw(1);

        if (iVar5 != 0) goto LAB_0301937e;

        func_0x030037a8(0xba);

        *(undefined1 *)(iVar7 + 0x74) = *DAT_03019638;

        func_0x030037a8(0x102);

        *puVar4 = 0;

        FUN_02ff26bc(DAT_03019668);

        uVar6 = 199;

        goto LAB_030193e4;

      }

    }

    if (4 < *DAT_03019644) {

      return 0;

    }

    iVar7 = DAT_0301963c + (uint)*DAT_03019644 * 2;

    sVar3 = *(short *)(iVar7 + 0x77);

    if (0x13 < sVar3) {

      return 0;

    }

    *(short *)(iVar7 + 0x77) = sVar3 + 1;

    func_0x030037a8(0x103);

    uVar6 = 0x1b2;

  }

LAB_030193e4:

  func_0x030037a8(uVar6);

  return 0;

}




