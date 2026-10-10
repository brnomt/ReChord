/**
 * AudioPlayback_Start @ 0x0302a398
 * Tags: audio
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void AudioPlayback_Start(void)



{

  byte bVar1;

  undefined4 *puVar2;

  undefined2 *puVar3;

  int *piVar4;

  undefined4 *puVar5;

  undefined4 *puVar6;

  undefined4 *puVar7;

  ushort *puVar8;

  undefined4 uVar9;

  undefined4 uVar10;

  undefined1 *puVar11;

  int iVar12;

  int iVar13;

  int iVar14;

  int iVar15;

  bool bVar16;

  

  puVar2 = DAT_0302a790;

  DAT_0302a790[2] = 0;

  *puVar2 = 0;

  iVar15 = 200;

  MediaLib_thunk_GetFiles(s_Audio_Start_0302a794);

  FUN_0300cd00(0x22,7);

  FUN_0302da30();

  rom_usb_disconnect(0xe);

  FUN_02ffb29c(0xe,0xe0);

  FUN_0300b960(0xe,DAT_0302a7a4);

  rom_usb_connect(0xe);

  iVar12 = AudioCodec();

  iVar13 = DAT_0302a7a8;

  if (iVar12 == -1) {

    shared_frame_epilogue_c6b4(0x55);

    MediaLib_thunk_GetFiles(s_Audio_File_Open_Error_0302a7ac);

    return;

  }

  if ((*(int *)(DAT_0302a7a8 + 0x34) == 0) &&

     ((bVar1 = *DAT_0302a7c4, bVar1 == 2 || ((1 < bVar1 && (bVar1 == 3)))))) {

    *(undefined4 *)(DAT_0302a7a8 + 0x34) = *(undefined4 *)(DAT_0302a7c8 + 0xd);

  }

  if (*(int *)(iVar13 + 0x44) != 1) {

    mpi_is_prime();

  }

  shared_frame_epilogue_c6b4(0xf7);

  FUN_03029e22();

  MediaLib_thunk_GetFiles(s_file_open_totally_over_0302a7cc);

  puVar3 = DAT_0302a7e4;

  *DAT_0302a7e4 = 0;

  iVar13 = audiocodec_open();

  puVar8 = DAT_0302a828;

  if (iVar13 == -1) {

    *puVar3 = 1;

    if (*puVar8 < 3) {

      shared_frame_epilogue_c6b4(0x55);

    }

    MediaLib_thunk_GetFiles(s_Codec_Open_Error1_0302a82c);

    return;

  }

  MediaLib_thunk_GetFiles(s_audio_codec_open_success__0302a7e8);

  SdioDev_EnalbeFunc();

  FUN_03029b44();

  rom_dma_config(4,1);

  iVar13 = DAT_0302a80c;

  *DAT_0302a804 = 0;

  *(undefined1 *)(iVar13 + 0x10) = *(undefined1 *)(DAT_0302a808 + 0x12);

  rom_i2s_master_config(0,0x17,0,1);

  FLAC_ThemeColor_Select(*(undefined4 *)(*(int *)(iVar13 + -0x1a0) + 8));

  FUN_030054b6(0,*(undefined4 *)(*(int *)(iVar13 + -0x1a0) + 8));

  FUN_0302ca40(DAT_0302a814,DAT_0302a810);

  puVar2 = DAT_0302a814;

  piVar4 = DAT_0302a810;

  rom_memzero(*DAT_0302a814,*DAT_0302a810 << 2);

  FUN_03009fe8(*puVar2,piVar4,*(undefined4 *)(*(int *)(iVar13 + -0x1a0) + 0x9c));

  if (*(int *)(*(int *)(iVar13 + -0x1a0) + 8) != 0x2ee00) {

    iVar14 = *DAT_0302a818;

    bVar16 = SBORROW4(iVar14,8);

    iVar12 = iVar14 + -8;

    if (iVar14 < 8) {

      iVar12 = *(int *)(*(int *)(iVar13 + -0x1a0) + 0x9c);

      bVar16 = SBORROW4(iVar12,0x18);

      iVar12 = iVar12 + -0x18;

    }

    if (iVar12 < 0 != bVar16) {

      FUN_0302ccea();

      FUN_0302d808();

      FUN_03029b00(*(undefined1 *)(iVar13 + 0x10));

    }

  }

  FUN_03005604(*(undefined1 *)(iVar13 + 0x10));

  puVar7 = DAT_0302a824;

  puVar6 = DAT_0302a820;

  puVar5 = DAT_0302a81c;

  *DAT_0302a81c = 0;

  *puVar6 = 0;

  *puVar7 = 0;

  iVar12 = rom_buffer_ready(5);

  while (iVar12 == 1) {

    dac_gain_curve_apply(1);

    iVar15 = iVar15 + -1;

    if (iVar15 == 0) break;

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

  rom_playback_start(1,2,1,*(undefined4 *)(*(int *)(iVar13 + -0x1a0) + 8),0,0x17,0);

  rom_i2s_dma_start(1,4,0);

  rom_audio_path_route(1,4);

  rom_audio_path_disable(1,4);

  uVar10 = DAT_0302a848;

  uVar9 = DAT_0302a844;

  if (*DAT_0302a828 == 0) {

    *puVar6 = 1;

    FUN_03004608(5,*puVar2,DAT_0302a84c,*piVar4,uVar10,uVar9);

    *DAT_0302a850 = 0;

    shared_mid_entry_c6f6(0x59);

    puVar11 = DAT_0302a854;

    *puVar5 = 1;

    *puVar11 = 0;

    httpup_test();

    FUN_030059bc();

    dac_gain_curve_apply(0x50);

    rom_dac_mute(1,4);

  }

  else if (*DAT_0302a828 == 3) {

    *(undefined4 *)(iVar13 + -0x174) = 0xffffffff;

  }

  FUN_030059bc();

  iVar15 = DAT_0302a808;

  if (*(char *)(DAT_0302a808 + 0x34a) != '\0') {

    shared_frame_epilogue_c6b4(0x1d5);

    shared_mid_entry_c6f6(0x1d4);

  }

  *DAT_0302a858 = *(undefined1 *)(iVar15 + 0x34a);

  *DAT_0302a85c = 0;

  shared_mid_entry_c6f6(0x1db);

  shared_frame_epilogue_c6b4(0x2b);

  *DAT_0302a860 = 2;

  shared_frame_epilogue_c6b4(0x23);

  *DAT_0302a868 = *DAT_0302a864;

  MediaLib_thunk_GetFiles(s_MusicService_start_ok____0302a86c);

  return;

}
