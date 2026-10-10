/**
 * RecordWriteFile2 @ 0x030730bc
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 RecordWriteFile2(int param_1,int param_2)



{

  char cVar1;

  undefined2 uVar2;

  undefined4 *puVar3;

  int *piVar4;

  byte bVar5;

  int iVar6;

  uint uVar7;

  undefined4 uVar8;

  undefined4 *puVar9;

  undefined4 *puVar10;

  uint uVar11;

  byte *pbVar12;

  uint uVar13;

  int iVar14;

  undefined4 *extraout_r1;

  uint uVar15;

  int iVar16;

  int iVar17;

  undefined4 *puVar18;

  undefined4 *puVar19;

  undefined4 *puVar20;

  undefined4 *puVar21;

  undefined4 *puVar22;

  ushort *puVar23;

  bool bVar24;

  byte abStack_1d9 [197];

  undefined4 *local_114;

  byte local_110 [32];

  byte abStack_f0 [32];

  uint local_d0 [4];

  undefined4 *local_c0;

  byte *local_bc;

  undefined4 *local_b8;

  undefined4 *local_b4;

  byte local_b0 [64];

  undefined4 *local_70;

  int local_6c;

  undefined4 local_68;

  undefined4 uStack_64;

  uint local_60 [3];

  undefined4 *local_54;

  uint local_4c;

  undefined4 *local_48;

  byte *local_40;

  int local_3c;

  int local_2c;

  int local_28;

  

  local_c0 = *(undefined4 **)(param_2 + 0x24);

  local_70 = local_c0 + 0x240;

  if (*(char *)(param_2 + 1) == '\0') {

    puVar20 = (undefined4 *)0x1;

  }

  else {

    puVar20 = (undefined4 *)0x2;

  }

  local_2c = param_1;

  local_28 = param_2;

  FUN_0306e300();

  uVar11 = *(uint *)(param_2 + 0x18);

  if ((uVar11 & 0x1000) != 0) {

    iVar6 = 4;

    goto LAB_0307311e;

  }

  if ((uVar11 & 0x400) == 0) {

    uVar7 = *(uint *)(param_2 + 0xc);

    if (puVar20 == (undefined4 *)0x2) {

      uVar7 = uVar7 >> 1;

    }

    if (uVar7 < 0xbb81) {

      if (*(int *)(param_2 + 0x10) == 32000) {

        iVar6 = 3;

      }

      else {

        iVar6 = 2;

      }

      goto LAB_0307311e;

    }

    if (DAT_03073304 < uVar7) goto LAB_03073112;

  }

  else {

LAB_03073112:

    if (*(int *)(param_2 + 0x10) != 48000) {

      iVar6 = 1;

      goto LAB_0307311e;

    }

  }

  iVar6 = 0;

LAB_0307311e:

  local_b8 = *(undefined4 **)(DAT_03073308 + iVar6 * 0x24);

  local_6c = DAT_03073308 + iVar6 * 0x24 + 4;

  local_114 = (undefined4 *)&Reserved2;

  if (*(char *)(param_2 + 1) == '\x02') {

    *(uint *)(param_2 + 0x18) = uVar11 | 0x100;

    local_114 = (undefined4 *)(*(int *)(param_2 + 4) * 4 + 4);

  }

  iVar6 = DAT_0307330c;

  if (local_b8 < local_114) {

    local_114 = local_b8;

  }

  puVar21 = (undefined4 *)0x0;

  local_68 = *(undefined4 *)(local_2c + 0x1c);

  uStack_64 = *(undefined4 *)(local_2c + 0x20);

  if (local_114 != (undefined4 *)0x0) {

    do {

      puVar18 = (undefined4 *)0x0;

      uVar2 = *(undefined2 *)(iVar6 + (uint)*(byte *)(local_6c + (int)puVar21) * 4);

      if (puVar20 != (undefined4 *)0x0) {

        do {

          bVar5 = bitstream_getbits_be(local_2c + 0x1c,uVar2);

          iVar14 = (int)puVar18 * 0x20;

          puVar18 = (undefined4 *)((int)puVar18 + 1);

          (local_110 + iVar14)[(int)puVar21] = bVar5;

        } while (puVar18 < puVar20);

      }

      puVar21 = (undefined4 *)((int)puVar21 + 1);

    } while (puVar21 < local_114);

  }

  iVar6 = DAT_0307330c;

  puVar21 = local_114;

  if (local_114 < local_b8) {

    do {

      bVar5 = bitstream_getbits_be

                        (local_2c + 0x1c,

                         *(undefined2 *)(iVar6 + (uint)*(byte *)(local_6c + (int)puVar21) * 4));

      abStack_f0[(int)puVar21] = bVar5;

      local_110[(int)puVar21] = bVar5;

      puVar21 = (undefined4 *)((int)puVar21 + 1);

    } while (puVar21 < local_b8);

  }

  puVar21 = (undefined4 *)0x0;

  if (local_b8 != (undefined4 *)0x0) {

    do {

      puVar18 = (undefined4 *)0x0;

      if (puVar20 != (undefined4 *)0x0) {

        do {

          if ((local_110 + (int)puVar18 * 0x20)[(int)puVar21] != 0) {

            bVar5 = bitstream_getbits_be(local_2c + 0x1c,2);

            (local_b0 + (int)puVar18 * 0x20)[(int)puVar21] = bVar5;

          }

          puVar18 = (undefined4 *)((int)puVar18 + 1);

        } while (puVar18 < puVar20);

      }

      puVar21 = (undefined4 *)((int)puVar21 + 1);

    } while (puVar21 < local_b8);

  }

  if ((*(byte *)(param_2 + 0x18) & 0x10) != 0) {

    uVar8 = FUN_03070b72(&local_68,local_2c + 0x1c);

    uVar11 = i2c_receive_helper(local_68,uStack_64,uVar8,*(undefined2 *)(param_2 + 0x14));

    *(short *)(param_2 + 0x14) = (short)uVar11;

    if ((uVar11 != *(ushort *)(param_2 + 0x16)) && ((*(byte *)(local_28 + 0x20) & 1) == 0)) {

      *(undefined2 *)(local_2c + 0x3c) = 0x201;

      return 0xffffffff;

    }

  }

  puVar21 = (undefined4 *)0x0;

  if (local_b8 != (undefined4 *)0x0) {

    do {

      puVar18 = (undefined4 *)0x0;

      if (puVar20 != (undefined4 *)0x0) {

        iVar6 = (int)puVar21 * 3;

        do {

          if ((local_110 + (int)puVar18 * 0x20)[(int)puVar21] != 0) {

            bVar5 = bitstream_getbits_be(local_2c + 0x1c,6);

            iVar14 = (int)puVar18 * 0x60 + -0x1d8;

            abStack_1d9[iVar6 + iVar14 + 0x1d9] = bVar5;

            pbVar12 = (byte *)(uint)(local_b0 + (int)puVar18 * 0x20)[(int)puVar21];

            if (pbVar12 == (byte *)0x0) {

              bVar5 = bitstream_getbits_be(local_2c + 0x1c,6);

              abStack_1d9[iVar6 + iVar14 + 0x1da] = bVar5;

LAB_030732be:

              bVar5 = bitstream_getbits_be(local_2c + 0x1c,6);

              pbVar12 = abStack_1d9 + iVar6 + iVar14 + 0x1d9;

LAB_030732cc:

              pbVar12[2] = bVar5;

            }

            else {

              if (pbVar12 == (byte *)0x1) goto LAB_030732be;

              bVar24 = pbVar12 == (byte *)0x2;

              if (bVar24) {

                pbVar12 = abStack_1d9 + iVar6 + iVar14 + 0x1d9;

              }

              if (bVar24) {

                pbVar12[1] = bVar5;

                goto LAB_030732cc;

              }

              if (pbVar12 == (byte *)0x3) goto LAB_030732be;

            }

            bVar5 = (local_b0 + (int)puVar18 * 0x20)[(int)puVar21];

            if ((bVar5 & 1) != 0) {

              abStack_1d9[iVar6 + iVar14 + 0x1da] =

                   abStack_1d9[iVar6 + (int)puVar18 * 0x60 + (uint)bVar5];

            }

          }

          puVar18 = (undefined4 *)((int)puVar18 + 1);

        } while (puVar18 < puVar20);

      }

      puVar21 = (undefined4 *)((int)puVar21 + 1);

    } while (puVar21 < local_b8);

  }

  uVar11 = 0;

  local_4c = 0x20 - (int)local_b8;

  do {

    puVar18 = (undefined4 *)0x0;

    puVar21 = (undefined4 *)((ulonglong)DAT_0307370c * (ulonglong)uVar11);

    local_48 = (undefined4 *)

               (uVar11 + (uint)((ulonglong)DAT_0307370c * (ulonglong)uVar11 >> 0x22) * -6);

    if (local_114 != (undefined4 *)0x0) {

      local_54 = local_70 + (int)local_48 * 0x60;

      local_b4 = local_c0 + (int)local_48 * 0x60;

      puVar21 = local_c0;

      do {

        puVar22 = (undefined4 *)0x0;

        if (puVar20 != (undefined4 *)0x0) {

          local_3c = (int)puVar18 * 3;

          local_40 = abStack_1d9 + (uVar11 >> 2) + (int)puVar18 * 3 + 1;

          do {

            if ((local_110 + (int)puVar22 * 0x20)[(int)puVar18] == 0) {

              puVar21 = (undefined4 *)0x0;

              if (puVar22 == (undefined4 *)0x0) {

                local_b4[(int)puVar18] = 0;

                puVar9 = local_b4 + (int)puVar18;

                puVar9[0x20] = 0;

                goto LAB_030735b0;

              }

              local_54[(int)puVar18] = 0;

              local_54[(int)(puVar18 + 8)] = 0;

              local_54[(int)(puVar18 + 0x10)] = 0;

            }

            else {

              local_d0[2] = local_2c + 0x1c;

              puVar23 = (ushort *)

                        (DAT_03073718 +

                        (uint)*(byte *)((uint)(local_110 + (int)puVar22 * 0x20)[(int)puVar18] +

                                        DAT_03073714 +

                                        (uint)*(ushort *)

                                               (DAT_03073710 +

                                                (uint)*(byte *)(local_6c + (int)puVar18) * 4 + 2) *

                                        0xf + -1) * 0xc);

              cVar1 = (char)puVar23[1];

              if (cVar1 == '\0') {

                cVar1 = *(char *)((int)puVar23 + 3);

                uVar7 = 0;

                do {

                  uVar13 = bitstream_getbits_be(local_d0[2],cVar1);

                  local_60[uVar7] = uVar13;

                  uVar7 = uVar7 + 1;

                } while (uVar7 < 3);

              }

              else {

                uVar7 = bitstream_getbits_be(local_d0[2],*(undefined1 *)((int)puVar23 + 3));

                uVar13 = (uint)*puVar23;

                uVar15 = uVar7 / uVar13;

                local_60[0] = uVar7 - uVar13 * uVar15;

                uVar7 = uVar15 / uVar13;

                local_60[1] = uVar15 - uVar13 * uVar7;

                local_60[2] = uVar7 - uVar13 * (uVar7 / uVar13);

              }

              puVar19 = DAT_0307371c;

              uVar7 = 1 << (uint)(byte)(cVar1 - 1);

              bVar5 = 0x1d - cVar1;

              iVar14 = *(int *)(puVar23 + 4);

              iVar16 = *(int *)(puVar23 + 2);

              iVar6 = (((int)((local_60[0] ^ uVar7 | -(uVar7 & (local_60[0] ^ uVar7))) <<

                             (uint)bVar5) >> 0xc) + iVar14 * 0x10) * iVar16 >> 0xc;

              iVar17 = iVar16 * (((int)((-(uVar7 & (local_60[1] ^ uVar7)) | local_60[1] ^ uVar7) <<

                                       (uint)bVar5) >> 0xc) + iVar14 * 0x10) >> 0xc;

              iVar14 = (((int)((-(uVar7 & (local_60[2] ^ uVar7)) | local_60[2] ^ uVar7) <<

                              (uint)bVar5) >> 0xc) + iVar14 * 0x10) * iVar16 >> 0xc;

              if (puVar22 == (undefined4 *)0x0) {

                local_b4[(int)puVar18] = iVar6 * DAT_0307371c[*local_40];

                puVar21 = local_b4 + (int)puVar18;

                puVar21[0x20] = iVar17 * puVar19[*local_40];

                puVar21[0x40] = puVar19[*local_40] * iVar14;

              }

              else {

                pbVar12 = abStack_1d9 + (uVar11 >> 2) + local_3c + (int)puVar22 * 0x60 + 1;

                local_54[(int)puVar18] = DAT_0307371c[*pbVar12] * iVar6;

                puVar9 = local_54 + (int)puVar18;

                puVar9[0x20] = puVar19[*pbVar12] * iVar17;

                puVar21 = (undefined4 *)(iVar14 * puVar19[*pbVar12]);

LAB_030735b0:

                puVar9[0x40] = puVar21;

              }

            }

            puVar22 = (undefined4 *)((int)puVar22 + 1);

          } while (puVar22 < puVar20);

        }

        puVar18 = (undefined4 *)((int)puVar18 + 1);

      } while (puVar18 < local_114);

    }

    puVar9 = local_48;

    puVar22 = local_70;

    puVar18 = local_c0;

    puVar19 = local_114;

    if (local_114 < local_b8) {

      do {

        if (local_110[(int)puVar19] == 0) {

          puVar21 = (undefined4 *)0x0;

          if (puVar20 != (undefined4 *)0x0) {

            do {

              puVar18[(int)(puVar19 + (int)puVar9 * 0x18)] = 0;

              puVar18[(int)(puVar19 + (int)puVar9 * 0x18 + 8)] = 0;

              puVar18[(int)(puVar19 + (int)puVar9 * 0x18 + 0x10)] = 0;

              while( true ) {

                puVar21 = (undefined4 *)((int)puVar21 + 1);

                if (puVar20 <= puVar21) goto LAB_0307373a;

                if (puVar21 == (undefined4 *)0x0) break;

                puVar22[(int)(puVar19 + (int)puVar9 * 0x18)] = 0;

                puVar22[(int)(puVar19 + (int)puVar9 * 0x18 + 8)] = 0;

                puVar22[(int)(puVar19 + (int)puVar9 * 0x18 + 0x10)] = 0;

              }

            } while( true );

          }

        }

        else {

          local_bc = (byte *)(local_2c + 0x1c);

          puVar23 = (ushort *)

                    (DAT_03073718 +

                    (uint)*(byte *)((uint)local_110[(int)puVar19] +

                                    DAT_03073714 +

                                    (uint)*(ushort *)

                                           (DAT_03073710 +

                                            (uint)*(byte *)(local_6c + (int)puVar19) * 4 + 2) * 0xf

                                   + -1) * 0xc);

          local_60[0] = (uint)(byte)puVar23[1];

          if (local_60[0] == 0) {

            local_60[0] = (uint)*(byte *)((int)puVar23 + 3);

            uVar7 = 0;

            do {

              uVar13 = bitstream_getbits_be(local_bc,local_60[0]);

              local_d0[uVar7] = uVar13;

              uVar7 = uVar7 + 1;

            } while (uVar7 < 3);

          }

          else {

            uVar7 = bitstream_getbits_be(local_bc,*(undefined1 *)((int)puVar23 + 3));

            uVar13 = (uint)*puVar23;

            uVar15 = uVar7 / uVar13;

            local_d0[0] = uVar7 - uVar13 * uVar15;

            uVar7 = uVar15 / uVar13;

            local_d0[1] = uVar15 - uVar13 * uVar7;

            local_d0[2] = uVar7 - uVar13 * (uVar7 / uVar13);

          }

          puVar3 = DAT_0307371c;

          uVar7 = 1 << (local_60[0] - 1 & 0xff);

          iVar6 = *(int *)(puVar23 + 4);

          uVar13 = 0x1d - local_60[0];

          puVar21 = *(undefined4 **)(puVar23 + 2);

          puVar10 = (undefined4 *)0x0;

          if (puVar20 != (undefined4 *)0x0) {

            iVar14 = (((int)((local_d0[0] ^ uVar7 | -(uVar7 & (local_d0[0] ^ uVar7))) <<

                            (uVar13 & 0xff)) >> 0xc) + iVar6 * 0x10) * (int)puVar21 >> 0xc;

            iVar17 = (((int)((-(uVar7 & (local_d0[2] ^ uVar7)) | local_d0[2] ^ uVar7) <<

                            (uVar13 & 0xff)) >> 0xc) + iVar6 * 0x10) * (int)puVar21 >> 0xc;

            local_bc = abStack_1d9 + (uVar11 >> 2) + (int)puVar19 * 3 + 1;

            iVar6 = (((int)((-(uVar7 & (local_d0[1] ^ uVar7)) | local_d0[1] ^ uVar7) <<

                           (uVar13 & 0xff)) >> 0xc) + iVar6 * 0x10) * (int)puVar21 >> 0xc;

            pbVar12 = local_bc;

            do {

              puVar18[(int)(puVar19 + (int)puVar9 * 0x18)] = puVar3[*pbVar12] * iVar14;

              puVar18[(int)(puVar19 + (int)puVar9 * 0x18 + 8)] = puVar3[*local_bc] * iVar6;

              puVar18[(int)(puVar19 + (int)puVar9 * 0x18 + 0x10)] = puVar3[*local_bc] * iVar17;

              while( true ) {

                puVar10 = (undefined4 *)((int)puVar10 + 1);

                puVar21 = puVar3;

                if (puVar20 <= puVar10) goto LAB_0307373a;

                pbVar12 = local_bc;

                if (puVar10 == (undefined4 *)0x0) break;

                pbVar12 = abStack_1d9 + (uVar11 >> 2) + (int)puVar19 * 3 + (int)puVar10 * 0x60 + 1;

                puVar22[(int)(puVar19 + (int)puVar9 * 0x18)] = puVar3[*pbVar12] * iVar14;

                puVar22[(int)(puVar19 + (int)puVar9 * 0x18 + 8)] = puVar3[*pbVar12] * iVar6;

                puVar22[(int)(puVar19 + (int)puVar9 * 0x18 + 0x10)] = iVar17 * puVar3[*pbVar12];

              }

            } while( true );

          }

        }

LAB_0307373a:

        puVar19 = (undefined4 *)((int)puVar19 + 1);

      } while (puVar19 < local_b8);

    }

    puVar18 = (undefined4 *)0x0;

    if (puVar20 != (undefined4 *)0x0) {

      uVar7 = 0;

      do {

        iVar6 = (int)local_4c >> 1;

        puVar21 = local_b8;

        if (local_b8 < &Reserved2) {

          puVar22 = local_c0 + (int)(local_b8 + ((int)local_48 * 3 + uVar7) * 8);

          puVar21 = puVar22 + -1;

          iVar14 = iVar6;

          if ((local_4c & 1) != 0) {

            *puVar22 = 0;

            puVar21 = puVar22;

          }

          for (; iVar14 != 0; iVar14 = iVar14 + -1) {

            puVar21[1] = 0;

            puVar21[2] = 0;

            puVar21 = puVar21 + 2;

          }

        }

        while( true ) {

          uVar7 = uVar7 + 1;

          if (2 < uVar7) {

            puVar18 = (undefined4 *)((int)puVar18 + 1);

            if (puVar20 <= puVar18) goto LAB_030737a2;

            uVar7 = 0;

          }

          if (puVar18 == (undefined4 *)0x0) break;

          puVar21 = local_b8;

          if (local_b8 < &Reserved2) {

            puVar22 = local_70 + (int)(local_b8 + ((int)local_48 * 3 + uVar7) * 8);

            puVar21 = puVar22 + -1;

            iVar14 = iVar6;

            if ((local_4c & 1) != 0) {

              *puVar22 = 0;

              puVar21 = puVar22;

            }

            for (; iVar14 != 0; iVar14 = iVar14 + -1) {

              puVar21[1] = 0;

              puVar21[2] = 0;

              puVar21 = puVar21 + 2;

            }

          }

        }

      } while( true );

    }

LAB_030737a2:

    piVar4 = DAT_0307383c;

    puVar18 = local_48;

    if (local_48 == (undefined4 *)0x5) {

      do {

      } while (*DAT_0307383c == 1);

      RecordCopyEncDataToBuf(local_28,0,0x12);

      if ((undefined4 *)0x1 < puVar20) {

        do {

        } while (*piVar4 == 1);

        hifi_memmove(local_c0,local_70,0x900);

        RecordCopyEncDataToBuf(local_28,1,0x12,0);

      }

      FUN_0306f366(local_28);

      local_c0 = *(undefined4 **)(local_28 + 0x24);

      local_70 = *(undefined4 **)(local_28 + 0x24) + 0x240;

      puVar18 = local_70;

      puVar21 = extraout_r1;

    }

    uVar11 = uVar11 + 1;

    if (0xb < uVar11) {

      FUN_0306e302(puVar18,puVar21);

      return 0;

    }

  } while( true );

}
