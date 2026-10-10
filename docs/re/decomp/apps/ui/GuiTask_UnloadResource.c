/**
 * GuiTask_UnloadResource @ 0x0301e3f2
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 GuiTask_UnloadResource(void)



{

  undefined4 uVar1;

  int iVar2;

  int iVar3;

  bool bVar4;

  undefined4 local_68;

  undefined4 local_64;

  undefined4 local_60;

  undefined4 local_5c;

  undefined4 local_58;

  undefined4 local_54;

  undefined4 local_50;

  undefined4 local_4c;

  undefined4 local_48;

  undefined4 local_44;

  undefined4 local_40;

  undefined4 local_3c;

  undefined4 local_38;

  undefined4 local_34;

  undefined4 local_30;

  undefined4 local_2c;

  undefined4 local_28;

  undefined4 local_24;

  undefined4 local_20;

  undefined4 local_1c;

  undefined4 local_18;

  undefined4 local_14;

  

  iVar2 = DAT_0301e50c;

  *(undefined4 *)(DAT_0301e50c + 8) = 0;

  *(undefined4 *)(iVar2 + 0xc) = 0;

  *(undefined4 *)(iVar2 + 0x18) = 0;

  *(undefined4 *)(iVar2 + 0x1c) = 0;

  iVar3 = GUITask_PingPongMsg();

  uVar1 = DAT_0301e508;

  if (iVar3 != -1) {

    if (iVar3 == -2) {

      *(int *)(iVar2 + 0x10) = *(int *)(iVar2 + 0x10) + 1;

      return 0;

    }

    do {

      local_64 = 0;

      local_54 = 0;

      local_60 = 0;

      local_5c = 0;

      local_58 = 0;

      local_50 = 0;

      local_40 = 0;

      local_4c = 0;

      local_48 = 0;

      local_44 = 0;

      local_3c = 0;

      local_2c = 0;

      local_38 = 0;

      local_34 = 0;

      local_28 = 0;

      local_1c = 0;

      local_18 = 0;

      local_24 = 0;

      local_20 = 0;

      local_68 = uVar1;

      local_30 = uVar1;

      local_14 = 0;

      iVar3 = Gpio_SetPortIntMode(*(undefined4 *)(iVar2 + 4),2,&local_68,&local_30);

      if (iVar3 < 0) break;

    } while (((*(int *)(iVar2 + 0xc) == 0 && *(int *)(iVar2 + 0x14) == 0) &&

             *(int *)(iVar2 + 0x18) == 0) && *(int *)(iVar2 + 0x1c) == 0);

    if (*(int *)(iVar2 + 0xc) != 0) {

      *(int *)(iVar2 + 0x10) = *(int *)(iVar2 + 0x10) + 1;

    }

    iVar3 = *(int *)(iVar2 + 0x18);

    bVar4 = iVar3 == 0;

    if (bVar4) {

      iVar3 = *(int *)(iVar2 + 0x1c);

    }

    if (!bVar4 || iVar3 != 0) {

      return 1;

    }

    if (*(int *)(iVar2 + 0x14) == 0) {

      return 0;

    }

  }

  return 2;

}
