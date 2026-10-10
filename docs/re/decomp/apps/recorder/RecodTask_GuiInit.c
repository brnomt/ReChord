/**
 * RecodTask_GuiInit @ 0x030648c2
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void RecodTask_GuiInit(void)



{

  int iVar1;

  undefined4 uVar2;

  undefined4 uVar3;

  undefined4 uVar4;

  int iVar5;

  int *piVar6;

  

  iVar1 = DAT_03064cb0;

  if (*(int *)(DAT_03064cb0 + 0x20) != 0) {

    uVar2 = FUN_03021ffe(0);

    uVar3 = func_0x03021fea(0);

    *(undefined4 *)(iVar1 + 0x20) = 0;

    FUN_03022242(1);

    FUN_03064098(*(undefined2 *)(iVar1 + 8));

    if (*(ushort *)(iVar1 + 8) < 2) {

      if (*(short *)(iVar1 + 10) == 0) {

        uVar4 = 0x480;

      }

      else {

        uVar4 = 0x3f4;

      }

      *(undefined4 *)(iVar1 + 0x1c) = uVar4;

      func_0x030245e8(3,0);

      FUN_030281e4(0,2,0,*(undefined4 *)(iVar1 + 0x18));

    }

    else {

      *(undefined4 *)(iVar1 + 0x1c) = 0xc00;

      func_0x030245e8(3,0);

      FUN_030281e4(0,2,0,*(undefined4 *)(iVar1 + 0x18));

    }

    FUN_030281b6(5);

    FUN_030281b6(4);

    piVar6 = (int *)(DAT_03064cb4 + -0x344);

    iVar5 = DAT_03064cb4 + 0x10;

    FUN_030323dc(5,uVar3,iVar5 + *piVar6 * 0x3000,*(undefined4 *)(iVar1 + 0x1c));

    FUN_030376b8(10);

    FUN_030323dc(4,iVar5 + (1 - *piVar6) * 0x3000,uVar2,*(undefined4 *)(iVar1 + 0x1c));

    FUN_03022060(0,6);

    FUN_030222a2(0,6);

    if (*(ushort *)(iVar1 + 8) < 2) {

      ID3_Picture_Parser(0,0xf,0,1);

    }

    else {

      ID3_Picture_Parser(0,0x17,0,1);

    }

    uVar2 = *(undefined4 *)(iVar1 + 0x18);

    FUN_0303328e(uVar2);

    FUN_0303328a(2,uVar2);

    FUN_0303328a(0,uVar2);

    FUN_03033580();

    GUITask_CreateWidget(0xb0,0);

    return;

  }

  return;

}
