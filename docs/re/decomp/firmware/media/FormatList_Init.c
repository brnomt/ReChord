/**
 * FormatList_Init @ 0x03013c10
 * Tags: media, init
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */



undefined4 FormatList_Init(void)



{

  char cVar1;

  int *piVar2;

  int iVar3;

  char *pcVar4;

  int iVar5;

  undefined4 *puVar6;

  ushort uVar7;

  ushort uVar8;

  undefined2 uVar9;

  short sVar10;

  int iVar11;

  int iVar12;

  uint uVar13;

  int iVar14;

  bool bVar15;

  char local_74 [4];

  char acStack_70 [4];

  char acStack_6c [4];

  char acStack_68 [4];

  char local_64 [4];

  char acStack_60 [4];

  char acStack_5c [4];

  char acStack_58 [4];

  char local_54 [4];

  char local_50 [4];

  undefined4 uStack_4c;

  ushort local_48 [18];

  

  iVar3 = DAT_03014028;

  iVar12 = DAT_03014024;

  piVar2 = DAT_03014020;

  local_74 = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._0_4_;

  acStack_70 = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._4_4_;

  acStack_6c = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._8_4_;

  acStack_68 = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._12_4_;

  local_64 = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._16_4_;

  acStack_60 = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._20_4_;

  acStack_5c = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._24_4_;

  acStack_58 = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._28_4_;

  local_50 = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._36_4_;

  uStack_4c = ram0x0301401c;

  local_54 = (char  [4])s_MP1MP2MP3WMAWAVAPEFLAAACM4AOGGMP_03013ff4._32_4_;

  uVar8 = *(ushort *)((int)DAT_03014020 + 0x1ee);

  uVar7 = 0;

  iVar14 = *DAT_03014020;

  if (uVar8 != 0) {

    do {

      uVar7 = uVar7 + 1;

      iVar14 = *(int *)(iVar14 + 4);

    } while (uVar7 < uVar8);

  }

  cVar1 = *(char *)(iVar14 + 0x4e);

  if (cVar1 == '\0') {

    uVar13 = (uint)*(ushort *)(DAT_03014020 + 0x73);

    if ((short)DAT_03014020[uVar13 * 0xe + 3] != 0) {

      sVar10 = *(short *)(DAT_03014024 + 8);

      if ((*(short *)((int)DAT_03014020 + uVar13 * 2 + 0x1ce) == 0 && uVar8 == 0) && sVar10 == 1) {

        FUN_030121bc(1);

        local_48[0] = FUN_02ff41f0(1,*(undefined4 *)(iVar14 + 8),local_74,

                                   *(undefined1 *)(iVar12 + 8));

        FUN_02ffc66a(3,DAT_03014030,0x80);

        iVar12 = FUN_02ff0020(*DAT_03014038,DAT_03014034);

        if (iVar12 == 1) {

          FUN_02feff78(DAT_03014038,DAT_03014034);

        }

        *(undefined2 *)(iVar3 + 0xf2) = 10;

        *DAT_0301403c = piVar2[(uint)*(ushort *)(piVar2 + 0x73) * 0xe + 5];

        *(undefined1 *)(DAT_03014040 + 0x6c) = 0;

        MediaLib_GetTotalFiles_c(0x18c);

        MediaLib_GetTotalFiles_c(0x18b);

        MediaLib_GetTotalFiles(0x15d);

        FUN_030011be(3,local_48);

        return 1;

      }

      if (uVar13 < 7) {

        *(ushort *)((int)DAT_03014020 + uVar13 * 2 + 0x1de) = uVar8;

        uVar8 = (short)piVar2[0x73] + 1;

        *(ushort *)(piVar2 + 0x73) = uVar8;

        *(undefined2 *)((int)piVar2 + (uint)uVar8 * 2 + 0x1ce) = 0;

        pcVar4 = DAT_0301402c;

        *(undefined2 *)((int)piVar2 + 0x1ee) = 0;

        cVar1 = *pcVar4;

        piVar2[(uint)*(ushort *)(piVar2 + 0x73) * 0xe + 5] =

             CONCAT22(*(undefined2 *)(iVar14 + 99),*(undefined2 *)(iVar14 + 0x69));

        piVar2[(uint)*(ushort *)(piVar2 + 0x73) * 0xe + 6] =

             CONCAT22(*(undefined2 *)(iVar14 + 99),*(undefined2 *)(iVar14 + 0x69));

        if (cVar1 == '\x03') {

          *(undefined1 *)(piVar2 + (uint)*(ushort *)(piVar2 + 0x73) * 0xe + 9) =

               *(undefined1 *)(iVar14 + 0x5c);

          piVar2[(uint)*(ushort *)(piVar2 + 0x73) * 0xe + 7] = *(int *)(iVar14 + 0x6b);

        }

        else {

          *(undefined1 *)(piVar2 + (uint)*(ushort *)(piVar2 + 0x73) * 0xe + 9) = 1;

        }

        BroadMessage_Parse(*(undefined4 *)(piVar2[0x7d] + 4),

                           piVar2 + (uint)*(ushort *)(piVar2 + 0x73) * 0xe + 3,(char)sVar10);

        FUN_030122a2(*(undefined4 *)(piVar2[0x7d] + 4),*piVar2,

                     piVar2 + (uint)*(ushort *)(piVar2 + 0x73) * 0xe + 3,

                     *(undefined2 *)((int)piVar2 + (uint)*(ushort *)(piVar2 + 0x73) * 2 + 0x1ce),

                     *(undefined1 *)(iVar12 + 8));

        if ((short)piVar2[(uint)*(ushort *)(piVar2 + 0x73) * 0xe + 3] == 0) {

          MediaLib_GetTotalFiles(0x15c);

        }

        else {

          MediaLib_GetTotalFiles(0x14f);

          MediaLib_GetTotalFiles_c(0x154);

        }

      }

    }

  }

  else {

    iVar11 = FUN_030121bc(cVar1);

    if (*(short *)(iVar12 + 8) == 3) {

      if (cVar1 != '\x01' && cVar1 != '\x04') {

        uVar9 = parse_v23_frame(iVar14,0);

        uVar9 = FUN_02ff41f0(uVar9,*(undefined4 *)(iVar14 + 8),&DAT_03014044,

                             *(undefined1 *)(iVar12 + 8));

        *DAT_03014048 = uVar9;

      }

      sVar10 = parse_v23_frame(iVar14,1);

    }

    else {

      sVar10 = ((*(short *)((int)piVar2 + (uint)*(ushort *)(piVar2 + 0x73) * 2 + 0x1ce) +

                *(short *)((int)piVar2 + 0x1ee)) -

               (short)piVar2[(uint)*(ushort *)(piVar2 + 0x73) * 0xe + 4]) + 1;

    }

    if ((sVar10 != 0) &&

       (*DAT_0301404c =

             (uint)*(ushort *)((int)piVar2 + (uint)*(ushort *)(piVar2 + 0x73) * 2 + 0x1ce),

       cVar1 == '\x01')) {

      FUN_02feffdc(DAT_03014038);

      iVar5 = DAT_03014040;

      if (*(char *)(DAT_03014040 + 0x360) != '\x01') {

        uVar9 = parse_v23_frame(iVar14,0);

        uVar9 = FUN_02ff41f0(uVar9,*(undefined4 *)(iVar14 + 8),&DAT_03014044,

                             *(undefined1 *)(iVar12 + 8));

        *DAT_03014048 = uVar9;

        sVar10 = parse_v23_frame(iVar14,1);

      }

      local_48[0] = FUN_02ff41f0(sVar10,*(undefined4 *)(iVar14 + 8),*(undefined4 *)(iVar11 + 4),

                                 *(undefined1 *)(iVar12 + 8));

      FUN_02ffc66a(3,DAT_03014030,0x80);

      iVar14 = FUN_02ff0020(*DAT_03014038,DAT_03014034);

      if (iVar14 == 1) {

        sVar10 = *(short *)(iVar3 + 0xf2);

        if (sVar10 == 99) {

          if (*(short *)(iVar12 + 8) == 3) {

            uVar13 = (uint)*(ushort *)(iVar3 + 2);

            if ((*(short *)(iVar3 + 10) == 2) &&

               (uVar13 = FUN_02ff41f0(uVar13,*(undefined4 *)(iVar3 + 0x34),

                                      *(undefined4 *)(iVar11 + 4),3), uVar13 == 0)) {

              uVar13 = (uint)*(ushort *)(iVar3 + 2);

            }

            if (local_48[0] == uVar13) goto LAB_03013f50;

            *(ushort *)(iVar3 + 2) = local_48[0];

          }

        }

        else {

          bVar15 = sVar10 == 10;

          if (bVar15) {

            sVar10 = *(short *)(iVar12 + 8);

          }

          if (bVar15 && sVar10 == 1) {

            uVar13 = (uint)*(ushort *)(iVar3 + 2);

            if ((*(short *)(iVar3 + 10) == 2) &&

               (uVar13 = FUN_02ff41f0(uVar13,*(undefined4 *)(iVar3 + 0x34),

                                      *(undefined4 *)(iVar11 + 4),1), uVar13 == 0)) {

              uVar13 = (uint)*(ushort *)(iVar3 + 2);

            }

            if (local_48[0] == uVar13) {

              MediaLib_GetTotalFiles_c(0x18c);

LAB_03013f50:

              MediaLib_GetTotalFiles(0x15d);

              FUN_030011be(3,0);

              return 1;

            }

          }

        }

      }

      *(undefined1 *)(iVar5 + 0x6c) = 0;

      *(undefined4 *)(iVar5 + 0x68) = 0;

      FUN_02feff78(DAT_03014038,DAT_03014034);

      *(undefined1 *)(iVar5 + 0x6c) = 0;

      puVar6 = DAT_03014050;

      if (*(short *)(iVar12 + 8) == 3) {

        *(undefined2 *)(iVar3 + 0xf2) = 99;

        *DAT_03014444 = 0;

        *puVar6 = 0;

      }

      else {

        *DAT_0301403c = 0;

        *puVar6 = 0;

        *(undefined2 *)(iVar3 + 0xf2) = 10;

      }

      MediaLib_GetTotalFiles_c(0x18c);

      MediaLib_GetTotalFiles(0x15d);

      MediaLib_GetTotalFiles_c(0x18b);

      FUN_030011be(3,local_48);

      return 1;

    }

  }

  return 0;

}
