/**
 * GUI_TextDisplayBuff @ 0x030128bc
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 GUI_TextDisplayBuff(void)



{

  ushort uVar1;

  ushort uVar2;

  ushort uVar3;

  int *piVar4;

  byte bVar5;

  uint uVar6;

  uint uVar7;

  undefined4 in_r3;

  int iVar8;

  

  piVar4 = DAT_03012a20;

  uVar7 = (uint)*(ushort *)((int)DAT_03012a20 + 0x1ee);

  iVar8 = *DAT_03012a20;

  uVar1 = *(ushort *)(DAT_03012a20 + (uint)*(ushort *)(DAT_03012a20 + 0x73) * 0xe + 3);

  uVar6 = 0;

  if (uVar7 != 0) {

    do {

      iVar8 = *(int *)(iVar8 + 4);

      uVar6 = uVar6 + 1;

    } while (uVar6 < uVar7);

  }

  if (*(char *)(iVar8 + 0x4e) != '\0') {

    FUN_030015e0(2,7,uVar7,in_r3,in_r3);

    func_0x03000e38(0x23);

    if (*DAT_03012a40 == '\x03') {

      bVar5 = FUN_0301218a(iVar8 + 0x24,*(undefined4 *)(iVar8 + 0xc));

      uVar6 = (uint)bVar5;

    }

    else {

      uVar6 = GUI_BmpFronDisplay(*(undefined4 *)(iVar8 + 8),iVar8 + 0x4f);

    }

    iVar8 = DAT_03012a34;

    if (uVar6 == 0) {

      FUN_02ff0852(*(undefined1 *)(DAT_03012a34 + 0xe));

      *(undefined1 *)(iVar8 + 0x6c) = 0;

      iVar8 = DAT_03012a24;

      BroadMessage_Parse(*(undefined4 *)(piVar4[0x7d] + 4),

                         piVar4 + (uint)*(ushort *)(piVar4 + 0x73) * 0xe + 3,

                         *(undefined1 *)(DAT_03012a24 + 8));

      if (*(short *)((int)piVar4 + 0x1ee) != 0) {

        *(short *)((int)piVar4 + 0x1ee) = *(short *)((int)piVar4 + 0x1ee) + -1;

      }

      uVar2 = *(ushort *)(piVar4 + 0x73);

      uVar3 = *(ushort *)((int)piVar4 + (uint)uVar2 * 2 + 0x1ce);

      if ((uint)uVar3 == uVar1 - 1) {

        if (uVar3 < 6) {

          *(undefined2 *)((int)piVar4 + (uint)uVar2 * 2 + 0x1ce) = 0;

        }

        else {

          *(ushort *)((int)piVar4 + (uint)uVar2 * 2 + 0x1ce) = (uVar3 / 6) * 6 + -6;

        }

      }

      FUN_030122a2(*(undefined4 *)(piVar4[0x7d] + 4),*piVar4,

                   piVar4 + (uint)*(ushort *)(piVar4 + 0x73) * 0xe + 3,

                   *(undefined2 *)((int)piVar4 + (uint)*(ushort *)(piVar4 + 0x73) * 2 + 0x1ce),

                   *(undefined1 *)(iVar8 + 8));

      FUN_03000e84(0x23);

      return 0;

    }

    FUN_03000e84(0x23);

    return 0xffffffff;

  }

  return 0;

}
