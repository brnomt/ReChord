/**
 * mbedtls_sha_helper @ 0x03005bd2
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void mbedtls_sha_helper(void)



{

  short *psVar1;

  int iVar2;

  uint uVar3;

  uint uVar4;

  uint extraout_r3;

  uint extraout_r3_00;

  uint extraout_r3_01;

  short local_30;

  short local_2e;

  short local_2c;

  undefined2 local_2a;

  

  *DAT_03005e90 = 1;

  *DAT_03005e98 = *DAT_03005e94;

  FUN_02fee544(1);

  psVar1 = DAT_03005e9c;

  uVar4 = 0;

  do {

    uVar3 = uVar4 + 1 & 0xffff;

    psVar1[uVar4] = 0;

    iVar2 = DAT_03005ea0;

    uVar4 = uVar3;

  } while (uVar3 < 0x40);

  uVar4 = 0;

  do {

    uVar3 = uVar4 + 1 & 0xffff;

    psVar1[uVar4] = *(short *)(iVar2 + uVar4 * 2 + 0x15e);

    uVar4 = uVar3;

  } while (uVar3 < 0x50);

  psVar1[uVar3] = 0;

  local_30 = 0xc6;

  local_2e = 0x3b;

  local_2c = 0x13e;

  local_2a = 0x46;

  if (*psVar1 == 0) {

    FUN_02fee4ba(0x18,DAT_03005e9c,0x50);

  }

  FUN_02fef2b2(0x149,(int)local_30,(int)local_2e);

  uVar4 = FUN_02fed6a6(DAT_03005e9c);

  if (uVar4 < (uint)((int)local_2c - (int)local_30)) {

    FUN_02ff2e26(0xae);

  }

  else {

    FUN_02fee326(&local_30,CONCAT22(local_2e,local_30),0xc0000,extraout_r3 & 0xffff0000 | 0x149,

                 DAT_03005e9c,0x1e);

    FUN_02ff2de4(0xae);

  }

  FUN_02feda18(&local_30,&local_30,DAT_03005e9c,0);

  psVar1 = DAT_03005ea4;

  uVar4 = 0;

  do {

    uVar3 = uVar4 + 1 & 0xffff;

    psVar1[uVar4] = 0;

    uVar4 = uVar3;

  } while (uVar3 < 0x40);

  uVar4 = 0;

  do {

    uVar3 = uVar4 + 1 & 0xffff;

    psVar1[uVar4] = *(short *)(iVar2 + uVar4 * 2 + 0x25c);

    uVar4 = uVar3;

  } while (uVar3 < 0x50);

  psVar1[uVar3] = 0;

  local_30 = 0xc6;

  local_2e = 0x51;

  local_2c = 0x125;

  local_2a = 0x5c;

  if (*psVar1 == 0) {

    FUN_02fee4ba(0x18,DAT_03005ea4,0x50);

  }

  FUN_02fef2b2(0x14a,(int)local_30,(int)local_2e);

  uVar4 = FUN_02fed6a6(DAT_03005ea4);

  if (uVar4 < (uint)((int)local_2c - (int)local_30)) {

    FUN_02ff2e26(0xaf);

  }

  else {

    FUN_02fee3a4(&local_30,CONCAT22(local_2e,local_30),0xc0000,extraout_r3_00 & 0xffff0000 | 0x14a,

                 DAT_03005ea4,0x1e);

    FUN_02ff2de4(0xaf);

  }

  FUN_02feda18(&local_30,&local_30,DAT_03005ea4,0);

  psVar1 = DAT_03005ea8;

  uVar4 = 0;

  do {

    uVar3 = uVar4 + 1 & 0xffff;

    psVar1[uVar4] = 0;

    uVar4 = uVar3;

  } while (uVar3 < 0x40);

  uVar4 = 0;

  do {

    uVar3 = uVar4 + 1 & 0xffff;

    psVar1[uVar4] = *(short *)(iVar2 + uVar4 * 2 + 0x35a);

    uVar4 = uVar3;

  } while (uVar3 < 0x50);

  psVar1[uVar3] = 0;

  local_30 = 0xc6;

  local_2e = 0x68;

  local_2c = 0x125;

  local_2a = 0x73;

  if (*psVar1 == 0) {

    FUN_02fee4ba(0x18,DAT_03005ea8,0x50);

  }

  FUN_02fef2b2(0x14b,(int)local_30,(int)local_2e);

  uVar4 = FUN_02fed6a6(DAT_03005ea8);

  if (uVar4 < (uint)((int)local_2c - (int)local_30)) {

    FUN_02ff2e26(0xb0);

  }

  else {

    FUN_02fee43c(&local_30,CONCAT22(local_2e,local_30),0xc0000,extraout_r3_01 & 0xffff0000 | 0x14b,

                 DAT_03005ea8,0x1e);

    FUN_02ff2de4(0xb0);

  }

  FUN_02feda18(&local_30,&local_30,DAT_03005ea8,0);

  FUN_02fee544(2);

  return;

}
