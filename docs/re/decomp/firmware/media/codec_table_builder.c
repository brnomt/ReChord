/**
 * codec_table_builder @ 0x03014af2
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void codec_table_builder(short param_1)



{

  undefined2 uVar1;

  undefined2 uVar2;

  undefined2 *puVar3;

  undefined4 uVar4;

  undefined4 uVar5;

  undefined1 auStack_f0 [200];

  short local_28 [3];

  undefined2 local_22;

  

  puVar3 = DAT_03014ef0;

  uVar1 = *DAT_03014ef0;

  uVar2 = DAT_03014ef0[1];

  uVar4 = FUN_02ffef2c(1);

  uVar5 = FUN_02ffef08(2);

  local_28[0] = param_1 + 9;

  *puVar3 = *DAT_03014ef4;

  puVar3[1] = *DAT_03014ef8;

  local_28[2] = param_1 + 0xc6;

  local_28[1] = 0x4c;

  local_22 = 0x5b;

  FUN_02ff0622(auStack_f0,u_FLAC_24bit_192kHz_03014efc,100);

  FUN_02ffe3dc(local_28,local_28,auStack_f0,0);

  local_28[1] = 100;

  local_22 = 0x73;

  FUN_02ff0622(auStack_f0,u_APE__24bit_192kHz_03014f24,100);

  FUN_02ffe3dc(local_28,local_28,auStack_f0,0);

  local_28[1] = 0x7c;

  local_22 = 0x8b;

  FUN_02ff0622(auStack_f0,u_MP3_OGG_M4A_WMA_03014f4c,100);

  FUN_02ffe3dc(local_28,local_28,auStack_f0,0);

  local_28[1] = 0x94;

  local_22 = 0xa3;

  FUN_02ff0622(auStack_f0,u_NO_support_SACD_and_DTS_03014f70,100);

  FUN_02ffe3dc(local_28,local_28,auStack_f0,0);

  local_28[1] = 0x1c;

  local_22 = 0x2b;

  FUN_02ff0622(auStack_f0,u_DSD64_DSD128_DSD256_03014fa4,100);

  FUN_02ffe3dc(local_28,local_28,auStack_f0,0);

  local_28[1] = 0x34;

  local_22 = 0x43;

  FUN_02ff0622(auStack_f0,u_WAV_24bit_192kHz_03014fd0,100);

  FUN_02ffe3dc(local_28,local_28,auStack_f0,0);

  func_0x030037a8(0x21);

  FUN_02ffef2c(uVar4);

  puVar3[1] = uVar2;

  *puVar3 = uVar1;

  FUN_02ffef08(uVar5);

  return;

}
