/**
 * MusicInit @ 0x0302b9d8
 * Tags: audio, init
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void MusicInit(void)



{

  int iVar1;

  int iVar2;

  short sVar3;

  int iVar4;

  undefined2 *puVar5;

  int iVar6;

  ushort *puVar7;

  int iVar8;

  uint *puVar9;

  int iVar10;

  int iVar11;

  uint uVar12;

  undefined4 unaff_r4;

  short *unaff_r7;

  undefined4 unaff_r8;

  

  rom_audio_path_disable();

  FUN_030059bc();

  rom_audio_clock_off();

  rom_playback_start(1,2,1,48000);

  rom_dac_mute(1,4);

  dac_gain_curve_apply(100);

  rom_dac_unmute(1,4);

  rom_dma_config(4,1);

  MediaLib_thunk_GetFiles(s_MusicInit___0302be10);

  shared_mid_entry_c6f6(0x2a);

  puVar5 = DAT_0302be20;

  iVar4 = DAT_0302be1c;

  *(short *)(DAT_0302be1c + 4) = (short)unaff_r4;

  *puVar5 = 4;

  shared_mid_entry_c6f6(0x114);

  iVar6 = DAT_0302be24;

  if (unaff_r7 == (short *)0x0) {

LAB_0302ba76:

    *(short *)(iVar6 + 0x66) = (short)unaff_r8;

  }

  else {

    sVar3 = *unaff_r7;

    *(short *)(DAT_0302be24 + 0x66) = sVar3;

    if (sVar3 == -1 || sVar3 == 0) goto LAB_0302ba76;

  }

  iVar10 = FUN_0300c714(0x159);

  puVar7 = DAT_0302be28;

  if (iVar10 != 0) {

    DAT_0302be28[0x79] = 99;

  }

  *(uint *)(iVar4 + 0x18) = *(ushort *)(iVar6 + 0x66) - 1;

  if (puVar7[0x79] == 99) {

    iVar10 = USB_DAC_OpenStream(3);

    if (iVar10 != 0) {

LAB_0302bb18:

      RockCodecDev_ExitMode(0x23);

      return;

    }

  }

  else if (puVar7[0x79] == 10) {

    iVar10 = FUN_02ffe3a0(DAT_0302be2c,1);

    *puVar7 = (ushort)iVar10;

    if (iVar10 == 0) {

      shared_frame_epilogue_c6b4(0xf5);

      goto LAB_0302bb18;

    }

  }

  puVar5 = DAT_0302be34;

  iVar10 = DAT_0302be30;

  *(int *)(iVar4 + 0xc) = DAT_0302be30;

  *puVar5 = (short)unaff_r4;

  *(uint *)(iVar10 + 0x1c) = (uint)*(byte *)(iVar6 + 0x6d);

  *(uint *)(iVar10 + 0x20) = (uint)*(byte *)(iVar6 + 0x6f);

  iVar8 = DAT_0302be38;

  *(undefined1 *)(iVar10 + 0x28) = *(undefined1 *)(iVar10 + -8 + (uint)*(byte *)(iVar6 + 0x74));

  *(undefined1 *)(iVar8 + 0xc) = *(undefined1 *)(iVar10 + -8 + (uint)*(byte *)(iVar6 + 0x74));

  iVar11 = 0;

  do {

    iVar1 = iVar11 * 2;

    iVar2 = iVar11 * 2;

    iVar11 = iVar11 + 1;

    *(undefined2 *)(iVar10 + iVar2 + 0x2b) =

         *(undefined2 *)(iVar10 + 0xa0 + *(short *)(iVar6 + iVar1 + 0x77) * 2);

  } while (iVar11 < 5);

  rom_memcpy(iVar8 + 0xc,iVar10 + 0x28,0xd);

  uVar12 = *(uint *)(iVar10 + 0x1c);

  if ((uVar12 == 2 || uVar12 == 3) || (uVar12 < 2)) {

    *(undefined4 *)(iVar10 + 0x24) = 2;

  }

  if (puVar7[0x79] == 99 || puVar7[0x79] == 10) {

    FUN_03001694(DAT_0302be28,*(undefined2 *)(iVar6 + 0x66),*(undefined2 *)(iVar10 + 0x24),

                 *(undefined2 *)(iVar10 + 0x20));

    if (puVar7[0x79] != 99) goto LAB_0302bb7c;

  }

  else {

    puVar7[6] = (ushort)*(byte *)(iVar6 + 0x6f);

    *(undefined4 *)(puVar7 + 8) = DAT_0302be3c;

    puVar9 = DAT_0302be4c;

    *DAT_0302be40 = unaff_r8;

    *DAT_0302be44 = unaff_r8;

    *DAT_0302be48 = unaff_r8;

    *puVar9 = (uint)puVar7[1];

LAB_0302bb7c:

    if (0x2000 < *puVar7) {

      *puVar7 = 0x2000;

      if (0x2000 < puVar7[1]) {

        puVar7[1] = 0x1fff;

      }

    }

  }

  puVar9 = DAT_0302be50;

  if (*(int *)(*(int *)(iVar4 + 0xc) + 0x20) != 1) goto LAB_0302bbfe;

  *(undefined4 *)(iVar4 + 0x14) = unaff_r8;

  uVar12 = (uint)*puVar7;

  if (uVar12 == 1) {

LAB_0302bcce:

    *(undefined4 *)(iVar4 + 0x50) = unaff_r4;

  }

  else if (uVar12 == 2) {

    if (puVar7[1] != 1) goto LAB_0302bcce;

    *(undefined4 *)(iVar4 + 0x50) = unaff_r8;

  }

  else {

    iVar10 = 0;

    *(uint *)(iVar4 + 0x50) = *puVar9 - uVar12 * (*puVar9 / uVar12);

    do {

      if ((*(int *)(iVar6 + 0x70) != *(int *)(iVar4 + 0x50)) &&

         (puVar7[1] - 1 != *(int *)(iVar4 + 0x50))) break;

      *(uint *)(iVar4 + 0x50) = *puVar9 - (uint)*puVar7 * (*puVar9 / (uint)*puVar7);

      dac_gain_curve_apply(0xf);

      iVar10 = iVar10 + 1;

    } while (iVar10 < 200);

  }

  FUN_030299d2(*puVar7,*(undefined4 *)(iVar4 + 0x50));

  *(undefined4 *)(iVar6 + 0x70) = *(undefined4 *)(iVar4 + 0x50);

  *(undefined4 *)(iVar4 + 0x54) = unaff_r8;

LAB_0302bbfe:

  RockCodecDev_ExitMode(0x23);

  dac_gain_curve_apply(0xf);

  FUN_0300a926();

  rom_dsp_start();

  rom_dsp_bypass_disable();

  rk_printf();

  FUN_0300b960(0x1a,DAT_0302be54);

  rom_usb_disconnect(0x1a);

  FUN_03012358(0,2);

  FUN_0301231c(0,2);

  rom_usb_connect(0x1a);

  FUN_0300b960(0x1b,0x302add1);

  rom_usb_disconnect(0x1b);

  FUN_03012358(0,4);

  FUN_0301231c(0,4);

  rom_usb_connect(0x1b);

  FUN_0300c558(4);

  MusicService(0xec,0);

  if (*DAT_0302be58 == '\0') {

    rom_i2s_dma_start(2,0);

    rom_audio_path_route(2,0,1);

    rom_audio_path_disable(2,0);

    rom_i2s_dma_start(2,0xe,0);

    rom_audio_path_route(2,0xe,1);

    rom_audio_path_disable(2,0xe,1);

  }

  else {

    rom_i2s_dma_start(2,0);

    rom_audio_path_route(2,0,1);

    rom_audio_path_disable(2,0,1);

    rom_i2s_dma_start(2,0xe,0);

    rom_audio_path_route(2,0xe,1);

    rom_audio_path_disable(2,0xe,0);

  }

  shared_frame_epilogue_c6b4(0x23);

  *DAT_0302be5c = 0x50;

  *DAT_0302be60 = *puVar9;

  *DAT_0302be64 = 0xff;

  return;

}
