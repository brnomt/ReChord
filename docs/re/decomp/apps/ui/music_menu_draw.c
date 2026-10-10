/**
 * music_menu_draw @ 0x0300512a
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */



void music_menu_draw(uint param_1,int param_2,uint param_3)



{

  short sVar1;

  undefined4 uVar2;

  int iVar3;

  int iVar4;

  int iVar5;

  uint uVar6;

  uint uVar7;

  int iVar8;

  short local_a8 [60];

  undefined2 uStack_30;

  undefined2 uStack_2e;

  undefined2 uStack_2c;

  undefined2 uStack_2a;

  

  iVar3 = 0;

  if (DAT_0304ae3c == 0) {

    local_a8[0] = 0x4d;

    local_a8[1] = 0x50;

    local_a8[2] = 0x33;

LAB_030051d2:

    iVar3 = 3;

  }

  else if (DAT_0304ae3c == 6) {

    local_a8[0] = 0x46;

    local_a8[2] = 0x41;

    local_a8[1] = 0x4c;

    local_a8[3] = 0x43;

    iVar3 = 4;

  }

  else {

    if (DAT_0304ae3c != 7) {

      if (DAT_0304ae3c == 2) {

        local_a8[0] = 0x41;

        local_a8[1] = 0x41;

        local_a8[2] = 0x43;

      }

      else if (DAT_0304ae3c == 5) {

        local_a8[0] = 0x41;

        local_a8[1] = 0x50;

        local_a8[2] = 0x45;

      }

      else if (DAT_0304ae3c == 8) {

        local_a8[0] = 0x44;

        local_a8[1] = 0x53;

        local_a8[2] = 0x44;

      }

      else if (DAT_0304ae3c == 9) {

        local_a8[0] = 0x44;

        local_a8[1] = 0x53;

        local_a8[2] = 0x46;

      }

      else if (DAT_0304ae3c == 1) {

        local_a8[0] = 0x57;

        local_a8[1] = 0x4d;

        local_a8[2] = 0x41;

      }

      else if (DAT_0304ae3c == 4) {

        local_a8[0] = 0x4f;

        local_a8[2] = 0x47;

        local_a8[1] = 0x47;

      }

      else {

        if (DAT_0304ae3c != 3) goto LAB_030051d4;

        local_a8[0] = 0x57;

        local_a8[1] = 0x41;

        local_a8[2] = 0x56;

      }

      goto LAB_030051d2;

    }

    local_a8[0] = 0x41;

    local_a8[2] = 0x41;

    local_a8[1] = 0x4c;

    local_a8[3] = 0x43;

    iVar3 = 4;

  }

LAB_030051d4:

  local_a8[iVar3] = 0x7c;

  iVar5 = param_2 / 1000;

  iVar4 = iVar3 + 1;

  if (param_3 < 10) {

    if (param_1 - 0x5622c8 < 1000) {

LAB_030052dc:

      local_a8[iVar4] = 0x32;

      local_a8[iVar3 + 2] = 0x2e;

      local_a8[iVar3 + 3] = 0x38;

      local_a8[iVar3 + 4] = 0x4d;

      local_a8[iVar3 + 5] = 0x48;

      local_a8[iVar3 + 6] = 0x5a;

      iVar3 = iVar3 + 7;

    }

    else if (DAT_030055f4 + param_1 < 1000) {

      local_a8[iVar4] = 0x35;

      local_a8[iVar3 + 2] = 0x2e;

      local_a8[iVar3 + 3] = 0x36;

      local_a8[iVar3 + 4] = 0x4d;

      local_a8[iVar3 + 5] = 0x48;

      local_a8[iVar3 + 6] = 0x5a;

      iVar3 = iVar3 + 7;

    }

    else {

      if (999 < DAT_030055f8 + param_1) goto LAB_030052dc;

      local_a8[iVar4] = 0x31;

      local_a8[iVar3 + 2] = 0x31;

      local_a8[iVar3 + 3] = 0x2e;

      local_a8[iVar3 + 4] = 0x32;

      local_a8[iVar3 + 5] = 0x4d;

      local_a8[iVar3 + 6] = 0x48;

      local_a8[iVar3 + 7] = 0x5a;

      iVar3 = iVar3 + 8;

    }

  }

  else {

    if (99 < iVar5) {

      iVar8 = (int)((ulonglong)((longlong)DAT_030055fc * (longlong)iVar5) >> 0x20);

      iVar8 = (iVar8 >> 5) - (iVar8 >> 0x1f);

      local_a8[iVar4] =

           (short)iVar8 +

           (((short)(iVar8 / 10) + (short)(iVar8 >> 0x1f)) -

           (short)((longlong)iVar8 * 0x66666667 >> 0x3f)) * -10 + 0x30;

      iVar4 = iVar3 + 2;

    }

    iVar3 = iVar5 / 10;

    local_a8[iVar4] =

         (short)iVar3 +

         (((short)(iVar3 / 10) + (short)(iVar3 >> 0x1f)) -

         (short)((longlong)iVar3 * 0x66666667 >> 0x3f)) * -10 + 0x30;

    local_a8[iVar4 + 1] = (short)iVar5 + (short)iVar3 * -10 + 0x30;

    local_a8[iVar4 + 2] = 0x4b;

    local_a8[iVar4 + 3] = 0x48;

    local_a8[iVar4 + 4] = 0x5a;

    iVar3 = iVar4 + 5;

  }

  local_a8[iVar3] = 0x7c;

  iVar4 = iVar3 + 1;

  uVar6 = (uint)((ulonglong)DAT_03005600 * (ulonglong)param_3 >> 0x23);

  sVar1 = (short)uVar6;

  if (9 < param_3) {

    local_a8[iVar4] =

         sVar1 + (short)(uint)((ulonglong)DAT_03005600 * (ulonglong)uVar6 >> 0x23) * -10 + 0x30;

    iVar4 = iVar3 + 2;

  }

  local_a8[iVar4] = (short)param_3 + sVar1 * -10 + 0x30;

  local_a8[iVar4 + 1] = 0x62;

  local_a8[iVar4 + 2] = 0x69;

  local_a8[iVar4 + 3] = 0x74;

  local_a8[iVar4 + 4] = 0x7c;

  iVar3 = iVar4 + 5;

  if (9 >= param_3) {

    local_a8[iVar3] = 0x44;

    local_a8[iVar4 + 6] = 0x53;

    uVar6 = iVar4 + 7;

    local_a8[uVar6] = 0x44;

    FUN_02fef2b2(0x13c,0xfc,0x76);

    goto LAB_0300555a;

  }

  uVar6 = param_1 / 1000;

  if (999 < param_1) {

    local_a8[iVar3] = (short)(uVar6 / 1000) + (short)((uVar6 / 1000) / 10) * -10 + 0x30;

    iVar3 = iVar4 + 6;

  }

  if (99 < param_1) {

    iVar4 = (int)((ulonglong)((longlong)DAT_030055fc * (longlong)(int)uVar6) >> 0x20);

    iVar4 = (iVar4 >> 5) - (iVar4 >> 0x1f);

    local_a8[iVar3] =

         (short)iVar4 +

         (((short)(iVar4 / 10) + (short)(iVar4 >> 0x1f)) -

         (short)((longlong)iVar4 * 0x66666667 >> 0x3f)) * -10 + 0x30;

    iVar3 = iVar3 + 1;

  }

  sVar1 = (short)(uVar6 / 10);

  local_a8[iVar3] = sVar1 + (short)((uVar6 / 10) / 10) * -10 + 0x30;

  uVar7 = iVar3 + 1U & 0xff;

  local_a8[uVar7] = (short)uVar6 + sVar1 * -10 + 0x30;

  uVar6 = uVar7 + 1 & 0xff;

  local_a8[uVar6] = 0x6b;

  uVar6 = uVar6 + 1 & 0xff;

  local_a8[uVar6] = 0x62;

  uVar6 = uVar6 + 1 & 0xff;

  local_a8[uVar6] = 0x70;

  uVar6 = uVar6 + 1 & 0xff;

  local_a8[uVar6] = 0x73;

  if ((DAT_0304ae3c == 3 || DAT_0304ae3c == 5) || (DAT_0304ae3c == 6 || DAT_0304ae3c == 7)) {

    if (param_3 < 0x18) {

      if (_FUN_03005604 <= param_2) goto LAB_03005540;

      if (param_2 < 0xac44) goto LAB_03005530;

      uVar2 = 0x13a;

    }

    else {

      if (param_2 < 0xac44) goto LAB_03005530;

LAB_03005540:

      uVar2 = 0x13b;

    }

  }

  else {

LAB_03005530:

    uVar2 = 0x13d;

  }

  FUN_02fef2b2(uVar2,0xfc,0x76);

LAB_0300555a:

  uVar7 = uVar6 + 1 & 0xff;

  uVar6 = uVar7 + 1 & 0xff;

  local_a8[uVar7] = 0;

  local_a8[uVar6] = 0;

  uVar6 = uVar6 + 1 & 0xff;

  local_a8[uVar6] = 0;

  local_a8[uVar6 + 1 & 0xff] = 0;

  uStack_30 = 0x1e;

  uStack_2e = 0x76;

  uStack_2c = 0x122;

  uStack_2a = 0x85;

  FUN_02fee544(1);

  if (*(char *)(FUN_03005604 + 0x34f) == '\x04') {

    uRam0301d750 = 0xe162;

  }

  else {

    uRam0301d750 = 0x44de;

  }

                    /* WARNING: Read-only address (ram,0x0301d750) is written */

  FUN_02feda18(&uStack_30,&uStack_30,local_a8,2);

  FUN_02fee544(2);

  return;

}
