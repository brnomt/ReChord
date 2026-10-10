/**
 * MusicService_Init @ 0x0302a3e0
 * Tags: audio, init
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void MusicService_Init(undefined4 param_1,int param_2)



{

  byte bVar1;

  undefined1 uVar2;

  undefined2 *puVar3;

  int *piVar4;

  undefined4 *puVar5;

  undefined4 *puVar6;

  undefined4 *puVar7;

  undefined4 *puVar8;

  ushort *puVar9;

  undefined1 *puVar10;

  int iVar11;

  int iVar12;

  int iVar13;

  int unaff_r4;

  undefined4 unaff_r5;

  bool bVar14;

  

  if ((*(int *)(param_2 + 0x34) == 0) &&

     ((bVar1 = *DAT_0302a7c4, bVar1 == 2 || ((1 < bVar1 && (bVar1 == 3)))))) {

    *(undefined4 *)(param_2 + 0x34) = *(undefined4 *)(DAT_0302a7c8 + 0xd);

  }

  if (*(int *)(param_2 + 0x44) != 1) {

    mpi_is_prime();

  }

  shared_frame_epilogue_c6b4(0xf7);

  FUN_03029e22();

  MediaLib_thunk_GetFiles(s_file_open_totally_over_0302a7cc);

  puVar3 = DAT_0302a7e4;

  *DAT_0302a7e4 = (short)unaff_r5;

  iVar11 = audiocodec_open();

  puVar9 = DAT_0302a828;

  if (iVar11 == -1) {

    *puVar3 = 1;

    if (*puVar9 < 3) {

      shared_frame_epilogue_c6b4(0x55);

    }

    MediaLib_thunk_GetFiles(s_Codec_Open_Error1_0302a82c);

    return;

  }

  MediaLib_thunk_GetFiles(s_audio_codec_open_success__0302a7e8);

  SdioDev_EnalbeFunc();

  FUN_03029b44();

  rom_dma_config(4,1);

  iVar11 = DAT_0302a80c;

  uVar2 = (undefined1)unaff_r5;

  *DAT_0302a804 = uVar2;

  *(undefined1 *)(iVar11 + 0x10) = *(undefined1 *)(DAT_0302a808 + 0x12);

  rom_i2s_master_config(0,0x17,0,1);

  FLAC_ThemeColor_Select(*(undefined4 *)(*(int *)(iVar11 + -0x1a0) + 8));

  FUN_030054b6(0,*(undefined4 *)(*(int *)(iVar11 + -0x1a0) + 8));

  FUN_0302ca40(DAT_0302a814,DAT_0302a810);

  puVar5 = DAT_0302a814;

  piVar4 = DAT_0302a810;

  rom_memzero(*DAT_0302a814,*DAT_0302a810 << 2);

  FUN_03009fe8(*puVar5,piVar4,*(undefined4 *)(*(int *)(iVar11 + -0x1a0) + 0x9c));

  if (*(int *)(*(int *)(iVar11 + -0x1a0) + 8) != 0x2ee00) {

    iVar13 = *DAT_0302a818;

    bVar14 = SBORROW4(iVar13,8);

    iVar12 = iVar13 + -8;

    if (iVar13 < 8) {

      iVar12 = *(int *)(*(int *)(iVar11 + -0x1a0) + 0x9c);

      bVar14 = SBORROW4(iVar12,0x18);

      iVar12 = iVar12 + -0x18;

    }

    if (iVar12 < 0 != bVar14) {

      FUN_0302ccea();

      FUN_0302d808();

      FUN_03029b00(*(undefined1 *)(iVar11 + 0x10));

    }

  }

  FUN_03005604(*(undefined1 *)(iVar11 + 0x10));

  puVar8 = DAT_0302a824;

  puVar7 = DAT_0302a820;

  puVar6 = DAT_0302a81c;

  *DAT_0302a81c = unaff_r5;

  *puVar7 = unaff_r5;

  *puVar8 = unaff_r5;

  iVar12 = rom_buffer_ready(5);

  while (iVar12 == 1) {

    dac_gain_curve_apply(1);

    unaff_r4 = unaff_r4 + -1;

    if (unaff_r4 == 0) break;

    iVar12 = rom_buffer_ready(5);

  }

  if (*DAT_0302a840 == '\0') {

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

  rom_playback_start(1,2,1,*(undefined4 *)(*(int *)(iVar11 + -0x1a0) + 8));

  rom_i2s_dma_start(1,4,0);

  rom_audio_path_route(1,4);

  rom_audio_path_disable(1,4);

  if (*DAT_0302a828 == 0) {

    *puVar7 = 1;

    FUN_03004608(5,*puVar5,DAT_0302a84c,*piVar4);

    *DAT_0302a850 = (short)unaff_r5;

    shared_mid_entry_c6f6(0x59);

    puVar10 = DAT_0302a854;

    *puVar6 = 1;

    *puVar10 = uVar2;

    httpup_test();

    FUN_030059bc();

    dac_gain_curve_apply(0x50);

    rom_dac_mute(1,4);

  }

  else if (*DAT_0302a828 == 3) {

    *(undefined4 *)(iVar11 + -0x174) = 0xffffffff;

  }

  FUN_030059bc();

  iVar11 = DAT_0302a808;

  if (*(char *)(DAT_0302a808 + 0x34a) != '\0') {

    shared_frame_epilogue_c6b4(0x1d5);

    shared_mid_entry_c6f6(0x1d4);

  }

  *DAT_0302a858 = *(undefined1 *)(iVar11 + 0x34a);

  *DAT_0302a85c = uVar2;

  shared_mid_entry_c6f6(0x1db);

  shared_frame_epilogue_c6b4(0x2b);

  *DAT_0302a860 = 2;

  shared_frame_epilogue_c6b4(0x23);

  *DAT_0302a868 = *DAT_0302a864;

  MediaLib_thunk_GetFiles(s_MusicService_start_ok____0302a86c);

  return;

}
