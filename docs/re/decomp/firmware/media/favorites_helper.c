/**
 * favorites_helper @ 0x0300a970
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void favorites_helper(int param_1)



{

  int iVar1;

  int *piVar2;

  int iVar3;

  int iVar4;

  undefined4 uVar5;

  uint uVar6;

  int iVar7;

  undefined4 uVar8;

  uint uVar9;

  bool bVar10;

  

  *DAT_0300aa68 = 0;

  FUN_02ff9742(0xe3);

  FUN_02ff2e92();

  iVar4 = DAT_0300aa6c;

  if (*(int *)(param_1 + 4) == 1) {

    *(undefined1 *)(DAT_0300aa6c + 1) = 1;

  }

  else {

    *(undefined1 *)(DAT_0300aa6c + 1) = 3;

  }

  uVar5 = FUN_02ff4e60(2);

  iVar3 = DAT_0300aa64;

  piVar2 = DAT_0300aa60;

  iVar1 = DAT_0300aa5c;

  uVar9 = 0;

  do {

    uVar6 = uVar9 + 0x30 & 0xff;

    bVar10 = *(short *)(iVar1 + 0x24) != 1;

    if (bVar10) {

      uVar6 = uVar6 * 0x21;

    }

    if (bVar10) {

      uVar8 = 0x20;

    }

    else {

      uVar6 = uVar6 * 0x20;

      uVar8 = 0x18;

    }

    FUN_02ff5ee0(*piVar2 + uVar6,iVar3 + uVar9 * 0x40,uVar8);

    uVar9 = uVar9 + 1;

  } while (uVar9 < 10);

  iVar7 = *piVar2;

  if (*(short *)(iVar1 + 0x24) == 1) {

    iVar7 = iVar7 + 0x740;

    uVar8 = 0x18;

  }

  else {

    iVar7 = iVar7 + 0x77a;

    uVar8 = 0x20;

  }

  FUN_02ff5ee0(iVar7,iVar3 + uVar9 * 0x40,uVar8);

  FUN_02ff4e60(uVar5);

  uVar5 = DAT_0300aa74;

  *DAT_0300aa70 = 0;

  *(undefined4 *)(iVar4 + 4) = 1;

  FUN_02ff6268(0x66,uVar5);

  FUN_02ff623e(0x66,DAT_0300aa78,0x18);

  FUN_02ff623e(0x67,DAT_0300aa7c,0x18);

  FUN_02fff884(s_Record_Start_0300aa80);

  RecordWinSvc_Start(param_1);

  FUN_02fff884(s_Record_Start11111111111_0300aa94);

  return;

}
