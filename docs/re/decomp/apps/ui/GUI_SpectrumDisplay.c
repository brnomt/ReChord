/**
 * GUI_SpectrumDisplay @ 0x03027e02
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


longlong GUI_SpectrumDisplay(int param_1)



{

  int iVar1;

  undefined4 uVar2;

  uint uVar3;

  int iVar4;

  int iVar5;

  int *piVar6;

  undefined4 uVar7;

  uint uVar8;

  

  iVar1 = DAT_03027e38;

  uVar7 = *(undefined4 *)(param_1 + 8);

  piVar6 = (int *)(DAT_03027e38 + 0x80);

  do {

    if (*(int *)(iVar1 + 0x14) == 0) {

      uVar8 = *(uint *)(iVar1 + 0x10);

      iVar5 = FUN_03027d08(uVar7,*(undefined4 *)(iVar1 + 0x7c),*piVar6,*(undefined4 *)(iVar1 + 0xc),

                           uVar8,*(undefined4 *)(iVar1 + 0x84),*(undefined4 *)(iVar1 + 0x18));

    }

    else {

      DisplayDev_Write();

      *(undefined4 *)(iVar1 + 0x14) = 0;

      uVar2 = *(undefined4 *)(iVar1 + 0x74);

      *(undefined4 *)(iVar1 + 0x7c) = uVar2;

      iVar5 = *(int *)(iVar1 + 0x78);

      *piVar6 = iVar5;

      *(undefined4 *)(iVar1 + 0x84) = 0;

      uVar8 = *(uint *)(iVar1 + 0x10);

      iVar5 = FUN_03027d08(uVar7,uVar2,iVar5,*(undefined4 *)(iVar1 + 0xc),uVar8,0,

                           *(undefined4 *)(iVar1 + 0x18));

      *(int *)(iVar1 + 0x18) = *(int *)(iVar1 + 0x18) + 1;

    }

    *(uint *)(iVar1 + 0x7c) = *(int *)(iVar1 + 0x7c) + *(uint *)(iVar1 + 0xc);

    uVar3 = *(uint *)(iVar1 + 0x84);

    if ((uVar3 & 1) != 0) {

      *piVar6 = (*(uint *)(iVar1 + 0xc) & 0xfffffffe) + *piVar6;

    }

    iVar4 = uVar3 + 1;

    *(int *)(iVar1 + 0x84) = iVar4;

    if (*(int *)(iVar1 + 0x40) == iVar4) {

      *(undefined4 *)(iVar1 + 0x14) = 1;

    }

    if ((*(int *)(iVar1 + 0x18) == *(int *)(iVar1 + 0x4c)) && (iVar4 == *(int *)(iVar1 + 0x40)))

    goto LAB_03027e9e;

  } while (iVar5 == 1);

  if (iVar5 == 0) {

    return CONCAT44(uVar8,1);

  }

LAB_03027e9e:

  return (ulonglong)uVar8 << 0x20;

}
