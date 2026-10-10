/**
 * music_service_init_helper @ 0x03029e22
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void music_service_init_helper(void)



{

  byte bVar1;

  byte bVar2;

  undefined4 *puVar3;

  int iVar4;

  byte *pbVar5;

  byte *pbVar6;

  int iVar7;

  undefined4 uVar8;

  int iVar9;

  

  switch(*DAT_03029f00) {

  case 0:

    audio_stream_process(0x2a,7);

    FUN_02ff67a4(0x6e,1);

    FUN_02ff67a4(0x7e,1);

    rom_dma_config(0xe,0);

    rom_dma_config(0xf,0);

    FUN_03011ef4(0x49);

    pbVar5 = DAT_03029f6c;

    iVar9 = DAT_03029f64;

    *(undefined4 *)(DAT_03029f64 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_03029f70;

    *pbVar5 = (byte)*DAT_03029f68;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_03029f74 + (uint)*pbVar5 * 0x27e + 0x20);

    break;

  case 1:

    audio_stream_process(0x2b,7);

    audio_stream_process(0x2c,7);

    FUN_03011ef4(0x4a);

    pbVar5 = DAT_03029f6c;

    iVar9 = DAT_03029f64;

    *(undefined4 *)(DAT_03029f64 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_03029f70;

    *pbVar5 = (byte)*DAT_03029f68;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_0302a35c + (uint)*pbVar5 * 0x27e + 0x20);

    break;

  case 2:

    audio_stream_process(0x33,7);

    FUN_03011ef4(0x4b);

    pbVar5 = DAT_0302a368;

    iVar9 = DAT_0302a360;

    *(undefined4 *)(DAT_0302a360 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_0302a36c;

    *pbVar5 = (byte)*DAT_0302a364;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    pbVar6 = DAT_0302a374;

    iVar9 = DAT_0302a35c;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_0302a35c + (uint)*pbVar5 * 0x27e + 0x20);

    bVar2 = *DAT_0302a370;

    pbVar5[1] = bVar2;

    bVar1 = *pbVar6;

    pbVar5[2] = bVar1;

    *(undefined4 *)(pbVar5 + 0xc) = *(undefined4 *)(iVar9 + (uint)bVar2 * 0x27e + 0x20);

    *(undefined4 *)(pbVar5 + 0x10) = *(undefined4 *)(iVar9 + (uint)bVar1 * 0x27e + 0x20);

    break;

  case 3:

    audio_stream_process(0x34,7);

    FUN_03011ef4(0x4c);

    pbVar5 = DAT_0302a368;

    iVar9 = DAT_0302a360;

    *(undefined4 *)(DAT_0302a360 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_0302a36c;

    *pbVar5 = (byte)*DAT_0302a364;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_0302a35c + (uint)*pbVar5 * 0x27e + 0x20);

    break;

  case 4:

    audio_stream_process(0x35,7);

    FUN_03011ef4(0x4d);

    pbVar5 = DAT_0302a368;

    iVar9 = DAT_0302a360;

    *(undefined4 *)(DAT_0302a360 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_0302a36c;

    *pbVar5 = (byte)*DAT_0302a364;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_0302a35c + (uint)*pbVar5 * 0x27e + 0x20);

    break;

  case 5:

    audio_stream_process(0x38,7);

    FUN_03011ef4(0x50);

    pbVar5 = DAT_0302a368;

    iVar9 = DAT_0302a360;

    *(undefined4 *)(DAT_0302a360 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_0302a36c;

    *pbVar5 = (byte)*DAT_0302a364;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_0302a35c + (uint)*pbVar5 * 0x27e + 0x20);

    break;

  case 6:

    audio_stream_process(0x36,7);

    FUN_03011ef4(0x4e);

    pbVar5 = DAT_0302a368;

    iVar9 = DAT_0302a360;

    *(undefined4 *)(DAT_0302a360 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_0302a36c;

    *pbVar5 = (byte)*DAT_0302a364;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_0302a35c + (uint)*pbVar5 * 0x27e + 0x20);

    break;

  case 7:

    audio_stream_process(0x37,7);

    FUN_03011ef4(0x4f);

    pbVar5 = DAT_0302a368;

    iVar9 = DAT_0302a360;

    *(undefined4 *)(DAT_0302a360 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_0302a36c;

    *pbVar5 = (byte)*DAT_0302a364;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    pbVar6 = DAT_0302a374;

    iVar9 = DAT_0302a35c;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_0302a35c + (uint)*pbVar5 * 0x27e + 0x20);

    bVar2 = *DAT_0302a370;

    pbVar5[1] = bVar2;

    bVar1 = *pbVar6;

    pbVar5[2] = bVar1;

    *(undefined4 *)(pbVar5 + 0xc) = *(undefined4 *)(iVar9 + (uint)bVar2 * 0x27e + 0x20);

    *(undefined4 *)(pbVar5 + 0x10) = *(undefined4 *)(iVar9 + (uint)bVar1 * 0x27e + 0x20);

    break;

  case 8:

    audio_stream_process(0x39,7);

    FUN_03011ef4(0x51);

    pbVar5 = DAT_0302a368;

    iVar9 = DAT_0302a360;

    *(undefined4 *)(DAT_0302a360 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_0302a36c;

    *pbVar5 = (byte)*DAT_0302a364;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    iVar9 = DAT_0302a35c;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_0302a35c + (uint)*pbVar5 * 0x27e + 0x20);

    bVar2 = *DAT_0302a378;

    pbVar5[1] = bVar2;

    *(undefined4 *)(pbVar5 + 0xc) = *(undefined4 *)(iVar9 + (uint)bVar2 * 0x27e + 0x20);

    break;

  case 9:

    audio_stream_process(0x3a,7);

    FUN_03011ef4(0x52);

    pbVar5 = DAT_0302a368;

    iVar9 = DAT_0302a360;

    *(undefined4 *)(DAT_0302a360 + 0x348) = 0;

    *(undefined4 *)(iVar9 + 0x34c) = 0;

    *(undefined4 *)(iVar9 + 0x350) = 0;

    *(undefined4 *)(iVar9 + 0x354) = 0;

    *(undefined4 *)(iVar9 + 0x358) = 0;

    *(undefined4 *)(iVar9 + 0x35c) = 0;

    puVar3 = DAT_0302a36c;

    *pbVar5 = (byte)*DAT_0302a364;

    uVar8 = (*(code *)*puVar3)();

    *(undefined4 *)(pbVar5 + 4) = uVar8;

    iVar9 = DAT_0302a35c;

    *(undefined4 *)(pbVar5 + 8) = *(undefined4 *)(DAT_0302a35c + (uint)*pbVar5 * 0x27e + 0x20);

    bVar2 = *DAT_0302a37c;

    pbVar5[1] = bVar2;

    *(undefined4 *)(pbVar5 + 0xc) = *(undefined4 *)(iVar9 + (uint)bVar2 * 0x27e + 0x20);

    break;

  default:

    return;

  }

  iVar4 = DAT_03029f04;

  puVar3 = DAT_03029f00;

  *(undefined4 *)(DAT_03029f04 + 0x7c) = 0;

  *(undefined4 *)(iVar4 + 0x75c) = *puVar3;

  iVar9 = DAT_03029f60;

  func_0x030124a0(0x65,0,2);

  func_0x03012474(iVar4 + 0x748,0,2);

  iVar7 = *(int *)(iVar4 + 0x7c);

  while (iVar7 == 0) {

    FUN_030059bc();

    FadeIsFinished();

    FUN_030098c0(1);

    iVar9 = iVar9 + -1;

    if (iVar9 == 0) break;

    iVar7 = *(int *)(iVar4 + 0x7c);

  }

  *(undefined4 *)(iVar4 + 0x7c) = 0;

  return;

}
