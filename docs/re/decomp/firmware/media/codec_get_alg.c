/**
 * codec_get_alg @ 0x03014c1a
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void codec_get_alg(short param_1)



{

  undefined2 uVar1;

  undefined2 uVar2;

  short sVar3;

  undefined2 *puVar4;

  int iVar5;

  undefined4 uVar6;

  undefined1 auStack_100 [200];

  short local_38 [3];

  undefined2 local_32;

  undefined4 local_30;

  undefined4 local_2c;

  

  puVar4 = DAT_03014ef0;

  local_30 = 0;

  uVar1 = *DAT_03014ef0;

  uVar2 = DAT_03014ef0[1];

  uVar6 = FUN_02ffef2c(1);

  local_2c = FUN_02ffef08(2);

  iVar5 = DAT_03014ff8;

  *puVar4 = *DAT_03014ef4;

  puVar4[1] = *DAT_03014ef8;

  sVar3 = param_1 + 9;

  local_38[2] = param_1 + 0xb9;

  local_38[0] = sVar3;

  if (*DAT_03014ff4 == -1) {

    if (*(char *)(iVar5 + 0x10) == '\0') {

      FUN_02ff0622(auStack_100,&DAT_0301503c,0x50);

    }

    else {

      FUN_02ff0622(auStack_100,u_Hardware_1_2_0_0301501c,0x50);

    }

  }

  else if (*(char *)(iVar5 + 0x10) == '\0') {

    FUN_02ff0622(auStack_100,&DAT_03015054,0x50);

  }

  else {

    FUN_02ff0622(auStack_100,u_Hardware_1_3_0_03014ffc,0x50);

  }

  local_38[1] = 0x1c;

  local_32 = 0x2b;

  FUN_02ffe3dc(local_38,local_38,auStack_100,0);

  param_1 = param_1 + 0xc6;

  local_38[2] = param_1;

  if (*(char *)(iVar5 + 0x10) == '\0') {

    local_38[0] = sVar3;

    FUN_02ff0622(auStack_100,&DAT_0301508c,0x50);

  }

  else {

    local_38[0] = sVar3;

    FUN_02ff0622(auStack_100,u_Software_3_7_0_0301506c,0x50);

  }

  local_38[1] = 0x34;

  local_32 = 0x43;

  FUN_02ffe3dc(local_38,local_38,auStack_100,0);

  local_38[1] = 0x4c;

  local_32 = 0x5b;

  if (*(char *)(iVar5 + 0x10) == '\0') {

    local_38[0] = sVar3;

    local_38[2] = param_1;

    FUN_02ff0622(auStack_100,&DAT_030150c8,0x1e);

    FUN_02ffe3dc(local_38,local_38,auStack_100,0);

    local_30 = 0;

    VopGetInt(2,0,auStack_100,&local_30);

    VopGetInt(2,1,auStack_100,&local_30);

    local_38[1] = 100;

    local_32 = 0x73;

    FUN_02ffe3dc(local_38,local_38,auStack_100,0);

  }

  else {

    local_38[0] = sVar3;

    local_38[2] = param_1;

    FUN_02ff0622(auStack_100,u_Flash_Memory__030150a4,0x28);

    FUN_02ffe3dc(local_38,local_38,auStack_100,0);

    func_0x030037a8(0xae);

    local_30 = 0;

    VopGetInt(2,0,auStack_100,&local_30);

    VopGetInt(2,1,auStack_100,&local_30);

    local_38[1] = 100;

    local_32 = 0x73;

    FUN_02ffe3dc(local_38,local_38,auStack_100,0);

    func_0x030037a8(0xae);

  }

  if ((*DAT_030150c4 == '\0') && (*DAT_030150dc != 0)) {

    local_38[1] = 0x7c;

    local_32 = 0x8b;

    if (*(char *)(iVar5 + 0x10) == '\0') {

      FUN_02ff0622(auStack_100,&DAT_03015100,0x1e);

      FUN_02ffe3dc(local_38,local_38,auStack_100,0);

      local_30 = 0;

      VopGetInt(4,0,auStack_100,&local_30);

      VopGetInt(4,1,auStack_100,&local_30);

      local_38[1] = 0x94;

      local_32 = 0xa3;

      FUN_02ffe3dc(local_38,local_38,auStack_100,0);

    }

    else {

      FUN_02ff0622(auStack_100,u_Card_Memory__030150e0,0x24);

      FUN_02ffe3dc(local_38,local_38,auStack_100,0);

      local_30 = 0;

      VopGetInt(4,0,auStack_100,&local_30);

      VopGetInt(4,1,auStack_100,&local_30);

      local_38[1] = 0x94;

      local_32 = 0xa3;

      FUN_02ffe3dc(local_38,local_38,auStack_100,0);

      func_0x030037a8(0xae);

    }

  }

  FUN_02ffef2c(uVar6);

  puVar4[1] = uVar2;

  *puVar4 = uVar1;

  FUN_02ffef08(local_2c);

  return;

}
