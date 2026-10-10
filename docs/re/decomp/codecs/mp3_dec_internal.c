/**
 * mp3_dec_internal @ 0x0306e5de
 * Tags: codec, mp3
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 mp3_dec_internal(int param_1)



{

  ushort uVar1;

  longlong lVar2;

  undefined4 *puVar3;

  int *piVar4;

  int *piVar5;

  int iVar6;

  undefined4 *puVar7;

  undefined4 *puVar8;

  undefined4 *puVar9;

  undefined4 *puVar10;

  int iVar11;

  int iVar12;

  undefined4 uVar13;

  uint uVar14;

  int iVar15;

  int iVar16;

  bool bVar17;

  

  iVar6 = DAT_0306e8b0;

  *DAT_0306e8b4 = 0;

  puVar7 = DAT_0306e8b8;

  puVar3 = DAT_0306e898;

  *DAT_0306e8b8 = 0;

  puVar7[3] = 0;

  puVar3[0x10] = 1;

  *(undefined2 *)(iVar6 + 0x16) = 0;

  *(undefined4 *)(iVar6 + 0x20) = 0;

  *(undefined1 *)(iVar6 + 0x15) = 0;

  *(undefined4 *)(iVar6 + 0x1c) = 0;

  piVar5 = DAT_0306e8a4;

  piVar4 = DAT_0306e8a0;

  *(undefined4 *)(iVar6 + 0x24) = 0;

  printf(*piVar4 + *piVar5 * 2,0x1200);

  printf(piVar4[1] + *piVar5 * 2,0x1200);

  puVar9 = DAT_0306e8c0;

  puVar8 = DAT_0306e8bc;

  iVar11 = *piVar4 + *piVar5 * 2;

  *DAT_0306e8a8 = iVar11;

  puVar3[3] = iVar11;

  (*(code *)*puVar9)(0,0,*puVar8);

  if ((param_1 == 1) && (iVar11 = mp3_id3v2_handler(iVar6), iVar11 == 0)) {

    hifi_debug_printf(s_FILE___s__LINE___d__0306e928,s_______Common_Codec_Audio_Mp3_lib_0306e8f0,

                      0x16d);

    return 0;

  }

  puVar10 = DAT_0306e8c4;

  hifi_debug_printf(s_mp3_start_pos___d_0306e8c8,*DAT_0306e8c4);

  (*(code *)*puVar9)(*puVar10,0,*puVar8);

  *puVar3 = *puVar8;

  puVar9 = DAT_0306e8e0;

  *DAT_0306e8e0 = DAT_0306e8dc;

  puVar9[1] = 0x800;

  FUN_0306f184(DAT_0306e8e0 + 2,DAT_0306e8e0,0x306e499,0,0,&LAB_0306e50e_1,&LAB_0306e534_1,0);

  iVar11 = DAT_0306e8e4;

  if (*(int *)(DAT_0306e8e4 + 0x1c) == 0) {

    return 0;

  }

  iVar12 = *(int *)(DAT_0306e8e4 + 0x2c);

  bVar17 = iVar12 == 0;

  if (bVar17) {

    iVar12 = 0x306e513;

  }

  puVar3[9] = iVar12;

  uVar13 = DAT_0306e8e8;

  if (!bVar17) {

    uVar13 = *(undefined4 *)(iVar11 + 0x18);

  }

  puVar3[4] = uVar13;

  iVar12 = DAT_0306e8ec;

  *(int *)(iVar11 + 0x14) = DAT_0306e8ec;

  puVar3[6] = iVar12;

  puVar3[7] = iVar12 + 0x40;

  puVar3[8] = iVar12 + 0x84;

  FUN_030706e8();

  PowerOn_Reset(puVar3[7]);

  FUN_03070ab2(puVar3[8]);

  *(undefined4 *)(puVar3[6] + 0x38) = *(undefined4 *)(iVar11 + 4);

  (**(code **)(iVar11 + 0x1c))(*(undefined4 *)(iVar11 + 0x18));

  iVar16 = 0x400;

  iVar12 = *DAT_0306e8a0 + *piVar5 * 2;

  puVar3[1] = iVar12;

  puVar3[2] = iVar12;

  iVar15 = puVar3[7];

  *(int *)(iVar15 + 0x2c) = iVar12;

  *(int *)(iVar15 + 0x30) = iVar12;

  *(undefined4 *)(iVar15 + 0x34) = 0;

  *(undefined4 *)(iVar15 + 0x38) = 0;

  do {

    while ((*(short *)(puVar3[6] + 0x3c) != 1 ||

           (iVar12 = (**(code **)(iVar11 + 0x1c))(*(undefined4 *)(iVar11 + 0x18)), iVar12 == 0))) {

LAB_0306e760:

      while( true ) {

        iVar16 = iVar16 + -1;

        if (iVar16 == 0) goto LAB_0306e970;

        iVar12 = FUN_0306f87e(puVar3[7],puVar3[6]);

        if (iVar12 != -1) {

          puVar3[5] = 0;

          goto LAB_0306e7a4;

        }

        uVar1 = *(ushort *)(puVar3[6] + 0x3c);

        if (uVar1 == 1) break;

        if ((uVar1 & 0xff00) == 0) goto LAB_0306e970;

        iVar12 = (*(code *)puVar3[9])(puVar3[4],puVar3[6],puVar3[7]);

        if (iVar12 != 0) {

          if (iVar12 == 0x10) goto LAB_0306e7a4;

          if (iVar12 == 0x11) goto LAB_0306e970;

          if (iVar12 != 0x20) goto LAB_0306e7a4;

        }

      }

    }

    if (iVar12 == 0x10) break;

    if (iVar12 == 0x11) {

LAB_0306e970:

      PmuSetPowerMode();

      return 0;

    }

    if (iVar12 != 0x20) goto LAB_0306e760;

  } while (*(short *)(puVar3[6] + 0x3c) == 1);

LAB_0306e7a4:

  iVar11 = puVar3[7];

  puVar3[10] = *(undefined4 *)(iVar11 + 0x10);

  iVar12 = *(int *)(iVar11 + 0xc);

  puVar3[0xb] = iVar12;

  puVar9 = DAT_0306e940;

  if (*(char *)(iVar11 + 1) == '\0') {

    uVar13 = 1;

  }

  else {

    uVar13 = 2;

  }

  puVar3[0xc] = uVar13;

  if (iVar12 < 1) {

    puVar3[0xd] = 60000;

  }

  else {

    uVar14 = (*(code *)*puVar9)(*puVar8);

    uVar13 = FUN_0306d2d6((int)((ulonglong)uVar14 * 8000),(int)((ulonglong)uVar14 * 8000 >> 0x20),

                          puVar3[0xb],(int)puVar3[0xb] >> 0x1f);

    puVar3[0xd] = uVar13;

  }

  uVar13 = (*(code *)*puVar9)(*puVar8);

  puVar7[1] = uVar13;

  if (*(short *)(iVar6 + 0x16) == 0) {

    *(undefined2 *)(iVar6 + 0x16) = *(undefined2 *)(puVar3 + 10);

  }

  if (*(int *)(iVar6 + 0x20) == 0) {

    *(undefined4 *)(iVar6 + 0x20) = puVar3[0xb];

  }

  else {

    hifi_debug_printf();

  }

  if (*(char *)(iVar6 + 0x15) == '\0') {

    *(undefined1 *)(iVar6 + 0x15) = *(undefined1 *)(puVar3 + 0xc);

  }

  if (*(int *)(iVar6 + 0x20) == 0) {

    puVar3[0xd] = 60000;

    *(undefined4 *)(iVar6 + 0x28) = 60000;

  }

  else if (*(uint *)(iVar6 + 0x1c) == 0) {

    uVar14 = (*(code *)*puVar9)(*puVar8);

    uVar13 = FUN_0306d2d6((int)((ulonglong)uVar14 * 8000),(int)((ulonglong)uVar14 * 8000 >> 0x20),

                          *(undefined4 *)(iVar6 + 0x20),0);

    *(undefined4 *)(iVar6 + 0x28) = uVar13;

  }

  else {

    lVar2 = (ulonglong)*(uint *)(iVar6 + 0x1c) * 8000;

    uVar13 = FUN_0306d2d6((int)lVar2,(int)((ulonglong)lVar2 >> 0x20),*(int *)(iVar6 + 0x20),0);

    *(undefined4 *)(iVar6 + 0x28) = uVar13;

  }

  puVar7[2] = *(undefined4 *)(iVar6 + 0x1c);

  uVar13 = (*(code *)*puVar9)(*puVar8);

  hifi_debug_printf(s_length__d__fileLen__d__bitraate__0306e948,*(undefined4 *)(iVar6 + 0x1c),uVar13

                    ,*(undefined4 *)(iVar6 + 0x20));

  if (*DAT_0306e96c == 0) {

    *DAT_0306e96c = 0x480;

  }

  puVar3[0xe] = 0x480;

  *(undefined4 *)(iVar6 + 0x2c) = 0x480;

  *(undefined4 *)(iVar6 + 0x24) = 0x480;

  puVar3[10] = (uint)*(ushort *)(iVar6 + 0x16);

  puVar3[0xb] = *(undefined4 *)(iVar6 + 0x20);

  puVar3[0xc] = (uint)*(byte *)(iVar6 + 0x15);

  puVar3[0xd] = *(undefined4 *)(iVar6 + 0x28);

  puVar3[0xf] = 0x480;

  return 1;

}
