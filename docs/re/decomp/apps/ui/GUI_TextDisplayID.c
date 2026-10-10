/**
 * GUI_TextDisplayID @ 0x030200f6
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void GUI_TextDisplayID(int param_1,int param_2,int param_3,undefined4 param_4,int param_5,

                      uint param_6,undefined4 param_7,uint param_8,uint param_9,undefined4 param_10)



{

  undefined2 uVar1;

  uint uVar2;

  int iVar3;

  undefined1 uVar4;

  uint uVar5;

  undefined4 uVar6;

  uint uVar7;

  undefined2 *puVar8;

  undefined1 *puVar9;

  undefined2 local_660 [384];

  undefined1 auStack_360 [512];

  undefined1 auStack_160 [256];

  undefined1 auStack_60 [16];

  int local_50;

  int local_4c;

  int local_48;

  int local_44;

  uint local_40;

  int local_3c;

  int local_38;

  int iStack_34;

  int iStack_30;

  int local_2c;

  undefined4 local_28;

  

  uVar2 = *(uint *)(param_1 + 0x74);

  local_50 = uVar2 << 4;

  local_4c = (uVar2 & 0xfffffff) << 3;

  local_38 = uVar2 << 7;

  uVar7 = *(uint *)(param_2 + 0xc0);

  local_3c = DAT_03020424 + param_3 * 0x10;

  local_44 = DAT_03020428 + param_3 * 8;

  local_48 = DAT_0302042c + param_3 * 8;

  iStack_34 = param_1;

  iStack_30 = param_2;

  local_2c = param_3;

  local_28 = param_4;

  AudioDev_I2S_Acodec_Mode(local_660,0x300);

  uVar2 = 0;

  local_40 = uVar7 + 0xd >> 1;

  do {

    if (uVar7 < 5) {

      uVar5 = 8;

    }

    else if (((0x18 < uVar7) || (uVar5 = local_40, uVar2 < 4)) && (uVar5 = uVar7 * 2, 8 < uVar7)) {

      if (uVar7 < 0x19) {

        uVar5 = uVar7 + 8;

      }

      else if (uVar2 < 4) {

        uVar5 = uVar7 * 2 - 0x10;

      }

      else {

        uVar5 = uVar7 - 6;

      }

    }

    puVar8 = local_660 + uVar2 * 0x40;

    FUN_0301fd00(param_2,local_2c,local_28,*(undefined4 *)(param_1 + 0x74),uVar2,puVar8,uVar7,uVar5,

                 auStack_60,param_10);

    if (param_5 == 0) {

      *(undefined1 *)(param_2 + uVar2 + 0xb4) = 0;

    }

    if (param_8 < param_9) {

      if (uVar2 < 4) {

        iVar3 = GUI_TextExtractString();

      }

      else {

        iVar3 = FUN_0301f650(param_7);

      }

      uVar1 = 0;

      if (iVar3 != 0) {

        uVar1 = FUN_0301f5f8(param_7,iVar3);

      }

      if (8 < iVar3) {

                    /* WARNING: Subroutine does not return */

        bitreader_refill(param_7,1);

      }

      *puVar8 = uVar1;

      uVar6 = 1;

    }

    else {

      uVar6 = 0;

    }

    if ((1 << (5 - uVar2 & 0xff) & param_6) != 0) {

      if (*(int *)(param_1 + 0x34) == 0) {

        uVar4 = *(undefined1 *)(param_2 + uVar2 + 0xb4);

      }

      else {

        uVar4 = 2;

      }

      FUN_0301f860(param_7,puVar8,uVar4,uVar6);

    }

    FUN_0301ff10(param_2,uVar2,puVar8,uVar5,auStack_60,*(undefined4 *)(param_1 + 0x5c));

    puVar9 = auStack_360 + uVar2 * 0x80;

    FUN_0301f8bc(puVar9,puVar8,uVar7,uVar5,*(undefined4 *)(param_1 + 0x1c));

    if (uVar2 < 4) {

      FUN_0301f938(puVar9,(uVar2 >> 1) * local_38 + local_3c + (uVar2 & 1) * 8,local_50);

    }

    else {

      iVar3 = local_48;

      if (uVar2 == 4) {

        puVar9 = auStack_160;

        iVar3 = local_44;

      }

      FUN_0301f938(puVar9,iVar3,local_4c);

    }

    uVar2 = uVar2 + 1;

  } while (uVar2 < 6);

  return;

}
