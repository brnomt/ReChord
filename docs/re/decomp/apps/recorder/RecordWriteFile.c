/**
 * RecordWriteFile @ 0x0307112c
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

/* WARNING: Restarted to delay deadcode elimination for space: stack */



undefined4 RecordWriteFile(uint *param_1,int param_2,int param_3,uint param_4)



{

  byte bVar1;

  ushort uVar2;

  longlong lVar3;

  undefined1 uVar4;

  undefined1 uVar5;

  undefined1 uVar6;

  uint uVar7;

  int iVar8;

  int iVar9;

  uint uVar10;

  int *piVar11;

  int iVar12;

  byte *pbVar13;

  undefined4 *puVar14;

  int iVar15;

  byte *pbVar16;

  uint uVar17;

  int iVar18;

  ushort *puVar19;

  uint uVar20;

  uint uVar21;

  uint *puVar22;

  uint uVar23;

  int iVar24;

  byte bVar25;

  int iVar26;

  uint *puVar27;

  uint *puVar28;

  uint *puVar29;

  int iVar30;

  uint uVar31;

  int *piVar32;

  bool bVar33;

  uint auStack_ab0 [575];

  ushort uStack_1b2;

  undefined4 local_1b0;

  uint local_1ac;

  uint local_1a8;

  uint *local_1a4;

  uint *puStack_19c;

  uint local_198;

  uint *local_194;

  uint uStack_190;

  uint *apuStack_18c [5];

  uint local_178;

  uint local_174;

  int iStack_14c;

  uint auStack_138 [4];

  int local_128;

  uint uStack_124;

  uint *puStack_f8;

  uint *puStack_f4;

  uint auStack_f0 [17];

  uint **ppuStack_ac;

  uint local_a8;

  int aiStack_a4 [4];

  uint *local_94;

  uint *local_90;

  uint uStack_8c;

  int local_88 [4];

  uint uStack_78;

  uint uStack_74;

  uint uStack_70;

  uint uStack_6c;

  uint uStack_68;

  byte *local_64;

  int local_60;

  int local_5c;

  int local_58;

  ushort *local_54;

  int iStack_4c;

  int iStack_44;

  uint local_40;

  uint *local_34;

  int local_30;

  int local_2c;

  uint local_28;

  

  auStack_f0[0x10] = param_2;

  local_34 = param_1;

  local_30 = param_2;

  local_2c = param_3;

  local_28 = param_4;

  FUN_0306e300();

  uVar7 = *(uint *)(auStack_f0[0x10] + 0x10);

  bVar33 = (*(uint *)(auStack_f0[0x10] + 0x18) & 0x4000) != 0;

  if (bVar33) {

    uVar7 = uVar7 << 1;

  }

  iVar8 = ((uVar7 & 0x7ff) >> 7) - ((int)(uVar7 << 0x10) >> 0x1f);

  iVar9 = iVar8 + -8;

  if (bVar33) {

    iVar9 = iVar8 + -5;

  }

  if ((*(uint *)(auStack_f0[0x10] + 0x18) & 0x1000) == 0) {

    aiStack_a4[3] = 2;

  }

  else {

    aiStack_a4[3] = 1;

  }

  local_a8 = 0;

  local_58 = iVar9 * 3;

  local_5c = DAT_0307157c + iVar9 * 0xc;

  do {

    puVar14 = DAT_03071580;

    local_60 = local_2c + local_a8 * 0x74 + 10;

    *DAT_03071580 = _DAT_e000e018;

    puVar14[1] = _DAT_e000e014;

    local_198 = 0;

    if (local_28 != 0) {

      do {

        iVar8 = DAT_0307158c;

        puVar19 = (ushort *)(local_60 + local_198 * 0x3a);

        local_88[local_198] = *(int *)(DAT_0307157c + local_58 * 4);

        if (*(char *)((int)puVar19 + 9) == '\x02') {

          if ((puVar19[4] & 8) == 0) {

            iVar9 = *(int *)(local_5c + 4);

          }

          else {

            iVar9 = *(int *)(local_5c + 8);

          }

          local_88[local_198] = iVar9;

        }

        puVar22 = local_34;

        if ((*(ushort *)(auStack_f0[0x10] + 0x18) & 0x1000) == 0) {

          iVar8 = local_2c + local_198 * 0x3a;

          if (local_a8 == 0) {

            bVar25 = 0;

          }

          else {

            bVar25 = *(byte *)(local_2c + local_198 + 8);

          }

          uStack_190 = *local_34;

          apuStack_18c[0] = (uint *)local_34[1];

          uVar6 = *(undefined1 *)(DAT_03071594 + (uint)puVar19[3] * 2);

          uVar4 = *(undefined1 *)(DAT_03071594 + (uint)puVar19[3] * 2 + 1);

          if (*(char *)((int)puVar19 + 9) == '\x02') {

            if ((puVar19[4] & 8) == 0) {

              iVar9 = 0x11;

              iVar8 = 0;

            }

            else {

              iVar9 = 0x10;

              iVar8 = 0;

            }

            do {

              uVar5 = bitstream_getbits_be(puVar22,uVar6);

              iVar15 = iVar8 + 1;

              *(undefined1 *)((int)puVar19 + iVar8 + 0x12) = uVar5;

              bVar33 = iVar9 != 0;

              iVar9 = iVar9 + -1;

              iVar8 = iVar15;

            } while (bVar33);

            iVar8 = 0x11;

            do {

              iVar9 = iVar15;

              uVar6 = bitstream_getbits_be(puVar22,uVar4);

              iVar15 = iVar9 + 1;

              *(undefined1 *)((int)puVar19 + iVar9 + 0x12) = uVar6;

              bVar33 = iVar8 != 0;

              iVar8 = iVar8 + -1;

            } while (bVar33);

            iVar12 = 1;

            *(undefined1 *)((int)puVar19 + iVar9 + 0x13) = 0;

            iVar8 = 1;

            do {

              iVar9 = iVar15 + iVar8;

              *(undefined1 *)((int)puVar19 + iVar15 + iVar8 + 0x12) = 0;

              iVar8 = iVar8 + 2;

              iVar12 = iVar12 + 2;

              *(undefined1 *)((int)puVar19 + iVar9 + 0x13) = 0;

            } while (iVar12 < 3);

          }

          else {

            if ((bVar25 & 8) == 0) {

              uVar7 = 0;

              do {

                uVar5 = bitstream_getbits_be(puVar22,uVar6);

                uVar10 = uVar7 + 1;

                *(undefined1 *)((int)puVar19 + uVar7 + 0x12) = uVar5;

                uVar7 = uVar10;

              } while (uVar10 < 6);

            }

            else {

              *(undefined1 *)(puVar19 + 9) = *(undefined1 *)(iVar8 + 0x1c);

              *(undefined1 *)((int)puVar19 + 0x13) = *(undefined1 *)(iVar8 + 0x1d);

              *(undefined1 *)(puVar19 + 10) = *(undefined1 *)(iVar8 + 0x1e);

              *(undefined1 *)((int)puVar19 + 0x15) = *(undefined1 *)(iVar8 + 0x1f);

              *(undefined1 *)(puVar19 + 0xb) = *(undefined1 *)(iVar8 + 0x20);

              *(undefined1 *)((int)puVar19 + 0x17) = *(undefined1 *)(iVar8 + 0x21);

            }

            if ((bVar25 & 4) == 0) {

              uVar7 = 6;

              do {

                uVar5 = bitstream_getbits_be(puVar22,uVar6);

                uVar10 = uVar7 + 1;

                *(undefined1 *)((int)puVar19 + uVar7 + 0x12) = uVar5;

                uVar7 = uVar10;

              } while (uVar10 < 0xb);

            }

            else {

              *(undefined1 *)(puVar19 + 0xc) = *(undefined1 *)(iVar8 + 0x22);

              *(undefined1 *)((int)puVar19 + 0x19) = *(undefined1 *)(iVar8 + 0x23);

              *(undefined1 *)(puVar19 + 0xd) = *(undefined1 *)(iVar8 + 0x24);

              *(undefined1 *)((int)puVar19 + 0x1b) = *(undefined1 *)(iVar8 + 0x25);

              *(undefined1 *)(puVar19 + 0xe) = *(undefined1 *)(iVar8 + 0x26);

            }

            if ((bVar25 & 2) == 0) {

              uVar7 = 0xb;

              do {

                uVar6 = bitstream_getbits_be(puVar22,uVar4);

                uVar10 = uVar7 + 1;

                *(undefined1 *)((int)puVar19 + uVar7 + 0x12) = uVar6;

                uVar7 = uVar10;

              } while (uVar10 < 0x10);

            }

            else {

              *(undefined1 *)((int)puVar19 + 0x1d) = *(undefined1 *)(iVar8 + 0x27);

              *(undefined1 *)(puVar19 + 0xf) = *(undefined1 *)(iVar8 + 0x28);

              *(undefined1 *)((int)puVar19 + 0x1f) = *(undefined1 *)(iVar8 + 0x29);

              *(undefined1 *)(puVar19 + 0x10) = *(undefined1 *)(iVar8 + 0x2a);

              *(undefined1 *)((int)puVar19 + 0x21) = *(undefined1 *)(iVar8 + 0x2b);

            }

            if ((bVar25 & 1) == 0) {

              uVar7 = 0x10;

              do {

                uVar6 = bitstream_getbits_be(puVar22,uVar4);

                uVar10 = uVar7 + 1;

                *(undefined1 *)((int)puVar19 + uVar7 + 0x12) = uVar6;

                uVar7 = uVar10;

              } while (uVar10 < 0x15);

            }

            else {

              *(undefined1 *)(puVar19 + 0x11) = *(undefined1 *)(iVar8 + 0x2c);

              *(undefined1 *)((int)puVar19 + 0x23) = *(undefined1 *)(iVar8 + 0x2d);

              *(undefined1 *)(puVar19 + 0x12) = *(undefined1 *)(iVar8 + 0x2e);

              *(undefined1 *)((int)puVar19 + 0x25) = *(undefined1 *)(iVar8 + 0x2f);

              *(undefined1 *)(puVar19 + 0x13) = *(undefined1 *)(iVar8 + 0x30);

            }

            *(undefined1 *)((int)puVar19 + 0x27) = 0;

          }

          puVar27 = &uStack_190;

        }

        else {

          local_194 = local_34;

          if (local_198 == 0) {

            uVar7 = 0;

          }

          else {

            uVar7 = local_2c + 0xb8;

          }

          local_178 = *local_34;

          local_174 = local_34[1];

          uVar2 = puVar19[3];

          uVar10 = (uint)uVar2;

          if (*(char *)((int)puVar19 + 9) == '\x02') {

            if ((puVar19[4] & 8) == 0) {

              iVar9 = 1;

            }

            else {

              iVar9 = 2;

            }

          }

          else {

            iVar9 = 0;

          }

          if ((-(*(uint *)(auStack_f0[0x10] + 4) & 1) & uVar7) == 0) {

            if (uVar10 < 400) {

              local_1b0 = (uint *)((ulonglong)DAT_03071590 * (ulonglong)(uint)(uVar2 >> 4) >> 0x22);

              local_1ac = (int)local_1b0 * -5 + (uint)(uVar2 >> 4);

              local_1a4 = (uint *)(uVar10 & 3);

              local_1a8 = (uVar10 & 0xf) >> 2;

              iVar8 = DAT_0307158c + iVar9 * 4;

            }

            else if (uVar10 < 500) {

              uVar7 = uVar10 - 400 >> 2;

              local_1b0 = (uint *)((ulonglong)DAT_03071590 * (ulonglong)uVar7 >> 0x22);

              local_1ac = uVar7 + (int)local_1b0 * -5;

              local_1a8 = uVar10 - 400 & 3;

              local_1a4 = (uint *)0x0;

              iVar8 = DAT_0307158c + iVar9 * 4 + 0xc;

            }

            else {

              local_1b0 = (uint *)((ulonglong)DAT_03071588 * (ulonglong)(uVar10 - 500) >> 0x21);

              local_1ac = (uVar10 - 500) + (int)local_1b0 * -3;

              local_1a8 = 0;

              local_1a4 = (uint *)0x0;

              *(byte *)(puVar19 + 4) = (byte)puVar19[4] | 4;

              iVar8 = iVar8 + iVar9 * 4 + 0x18;

            }

            uVar7 = 0;

            uVar10 = 0;

            do {

              uVar23 = 0;

              uVar20 = uVar7;

              if (*(char *)(iVar8 + uVar10) != '\0') {

                do {

                  uVar4 = bitstream_getbits_be(local_194,(&local_1b0)[uVar10]);

                  uVar7 = uVar20 + 1;

                  *(undefined1 *)((int)puVar19 + uVar20 + 0x12) = uVar4;

                  uVar23 = uVar23 + 1;

                  uVar20 = uVar7;

                } while (uVar23 < *(byte *)(iVar8 + uVar10));

              }

              uVar10 = uVar10 + 1;

            } while (uVar10 < 4);

            if ((uVar7 < 0x80000000) && (uVar10 = 0x27 - uVar7, 0 < (int)uVar10)) {

              uVar20 = uVar10 & 1;

              if (uVar20 != 0) {

                *(undefined1 *)((int)puVar19 + uVar7 + 0x12) = 0;

              }

              uVar23 = (uint)(uVar20 != 0);

              for (; (int)uVar20 < (int)uVar10; uVar20 = uVar20 + 2) {

                iVar8 = uVar7 + uVar23;

                *(undefined1 *)((int)puVar19 + uVar7 + uVar23 + 0x12) = 0;

                uVar23 = uVar23 + 2;

                *(undefined1 *)((int)puVar19 + iVar8 + 0x13) = 0;

              }

            }

          }

          else {

            uVar10 = (uint)(uVar2 >> 1);

            if (uVar10 < 0xb4) {

              local_1b0 = (uint *)((ulonglong)DAT_03071584 * (ulonglong)uVar10 >> 0x23);

              uVar10 = uVar10 + (int)local_1b0 * -0x24;

              local_1ac = (uint)((ulonglong)DAT_03071588 * (ulonglong)uVar10 >> 0x22);

              local_1a8 = uVar10 + (uint)((ulonglong)DAT_03071588 * (ulonglong)uVar10 >> 0x22) * -6;

              local_1a4 = (uint *)0x0;

              iVar8 = DAT_0307158c + iVar9 * 4 + 0x24;

            }

            else if (uVar10 < 0xf4) {

              uVar10 = uVar10 - 0xb4;

              local_1b0 = (uint *)((uVar10 & 0x3f) >> 4);

              local_1a8 = uVar10 & 3;

              local_1a4 = (uint *)0x0;

              local_1ac = (uVar10 & 0xf) >> 2;

              iVar8 = DAT_0307158c + iVar9 * 4 + 0x30;

            }

            else {

              local_1b0 = (uint *)((ulonglong)DAT_03071588 * (ulonglong)(uVar10 - 0xf4) >> 0x21);

              local_1ac = (uVar10 - 0xf4) + (int)local_1b0 * -3;

              local_1a8 = 0;

              local_1a4 = (uint *)0x0;

              iVar8 = DAT_0307158c + iVar9 * 4 + 0x3c;

            }

            local_1a4 = (uint *)0x0;

            uVar10 = 0;

            uVar20 = 0;

            do {

              bVar25 = *(byte *)(&local_1b0 + uVar20);

              uVar23 = 0;

              if (*(char *)(iVar8 + uVar20) != '\0') {

                do {

                  iVar9 = bitstream_getbits_be(local_194,(&local_1b0)[uVar20]);

                  *(char *)((int)puVar19 + uVar10 + 0x12) = (char)iVar9;

                  iVar15 = uVar7 + uVar10;

                  uVar10 = uVar10 + 1;

                  *(bool *)(iVar15 + 0x12) = iVar9 == (1 << (uint)bVar25) + -1;

                  uVar23 = uVar23 + 1;

                } while (uVar23 < *(byte *)(iVar8 + uVar20));

              }

              uVar20 = uVar20 + 1;

            } while (uVar20 < 4);

            if (uVar10 < 0x80000000) {

              iVar8 = 1;

              iVar9 = 0x28 - uVar10;

              if (1 < iVar9) {

                do {

                  iVar8 = iVar8 + 1;

                  *(undefined1 *)((int)puVar19 + uVar10 + 0x12) = 0;

                  iVar15 = uVar7 + uVar10;

                  uVar10 = uVar10 + 1;

                  *(undefined1 *)(iVar15 + 0x12) = 0;

                } while (iVar8 < iVar9);

              }

            }

          }

          puVar27 = &local_178;

          puVar22 = local_194;

        }

        iVar8 = FUN_03070b72(puVar27,puVar22);

        local_90 = local_34;

        local_94 = (uint *)(DAT_03071aa8 + local_198 * 0x900);

        local_54 = puVar19;

        pbVar16 = (byte *)local_88[local_198];

        if ((int)((uint)*puVar19 - iVar8) < 0) {

          return 0x236;

        }

        bVar25 = (byte)puVar19[4];

        iVar8 = puVar19[2] - 0xd2;

        if ((bVar25 & 2) == 0) {

          iVar9 = 1;

        }

        else {

          iVar9 = 2;

        }

        local_64 = pbVar16;

        if (*(char *)((int)puVar19 + 9) == '\x02') {

          uVar7 = 0;

          iVar15 = 0;

          if ((bVar25 & 8) != 0) {

            uVar10 = bVar25 & 4;

            if ((bVar25 & 4) != 0) {

              uVar10 = 0xffffffff;

            }

            do {

              (&local_194)[iVar15] =

                   (uint *)(iVar8 - ((uint)*(byte *)((int)puVar19 + iVar15 + 0x12) +

                                     (*(byte *)(DAT_03071ab0 + iVar15) & uVar10) << iVar9));

              pbVar13 = pbVar16 + iVar15;

              iVar15 = iVar15 + 1;

              uVar7 = uVar7 + *pbVar13;

            } while (uVar7 < 0x24);

          }

          bVar25 = *(byte *)((int)puVar19 + 0xd);

          uVar2 = puVar19[7];

          bVar1 = *(byte *)((int)puVar19 + 0xf);

          for (; uVar7 < 0x240; uVar7 = uVar7 + (uint)*pbVar13 * 3) {

            (&local_194)[iVar15] =

                 (uint *)((iVar8 + (uint)bVar25 * -8) -

                         ((uint)*(byte *)((int)puVar19 + iVar15 + 0x12) << iVar9));

            (&uStack_190)[iVar15] =

                 (iVar8 + (uint)(byte)uVar2 * -8) -

                 ((uint)*(byte *)((int)puVar19 + iVar15 + 0x13) << iVar9);

            apuStack_18c[iVar15] =

                 (uint *)((iVar8 + (uint)bVar1 * -8) -

                         ((uint)*(byte *)((int)puVar19 + iVar15 + 0x14) << iVar9));

            pbVar13 = pbVar16 + iVar15;

            iVar15 = iVar15 + 3;

          }

        }

        else if ((bVar25 & 4) == 0) {

          puVar22 = &local_198;

          pbVar16 = (byte *)((int)puVar19 + 0x11);

          iVar15 = 0xb;

          do {

            puVar22[1] = iVar8 - ((uint)pbVar16[1] << iVar9);

            pbVar16 = pbVar16 + 2;

            puVar22 = puVar22 + 2;

            *puVar22 = iVar8 - ((uint)*pbVar16 << iVar9);

            iVar15 = iVar15 + -1;

          } while (iVar15 != 0);

        }

        else {

          puVar22 = &local_198;

          pbVar16 = (byte *)((int)puVar19 + 0x11);

          iVar15 = 0xb;

          pbVar13 = DAT_03071aac;

          do {

            puVar22[1] = iVar8 - ((uint)pbVar13[1] + (uint)pbVar16[1] << iVar9);

            pbVar16 = pbVar16 + 2;

            pbVar13 = pbVar13 + 2;

            puVar22 = puVar22 + 2;

            *puVar22 = iVar8 - ((uint)*pbVar16 + (uint)*pbVar13 << iVar9);

            iVar15 = iVar15 + -1;

          } while (iVar15 != 0);

        }

        puVar22 = local_94;

        uVar7 = 0;

        uStack_70 = *local_90;

        uStack_6c = local_90[1];

        local_1a4 = local_94;

        FUN_03070b92(local_90);

        FUN_0306e300();

        iStack_44 = 1;

        ppuStack_ac = &local_194;

        iStack_4c = -1;

        puStack_19c = (uint *)0x3210;

        uVar10 = 0;

        uStack_8c = 0;

        uVar20 = 0;

        iVar8 = 0;

        puStack_f8 = puVar22 + (uint)local_54[1] * 2;

        if (puVar22 < puStack_f8) {

          do {

            iVar9 = iStack_4c;

            local_1a4 = local_1a4 + *local_64;

            local_64 = local_64 + 1;

            puStack_f4 = local_1a4;

            if (puStack_f8 < local_1a4) {

              puStack_f4 = puStack_f8;

            }

            iStack_44 = iStack_44 + -1;

            if (iStack_44 == 0) {

              if (puStack_19c == (uint *)0x3210) {

                iStack_44 = (byte)local_54[8] + 1;

              }

              else if (iStack_4c == 0) {

                iStack_44 = *(byte *)((int)local_54 + 0x11) + 1;

              }

              iStack_4c = iStack_4c + 1;

              piVar11 = (int *)(DAT_03071ab4 + (uint)*(byte *)((int)local_54 + iVar9 + 0xb) * 8);

              iVar8 = *piVar11;

              uStack_8c = (uint)*(ushort *)(piVar11 + 1);

              uVar20 = (uint)*(ushort *)((int)piVar11 + 6);

              if (iVar8 == 0) {

                return 0x237;

              }

            }

            if (*ppuStack_ac != puStack_19c) {

              auStack_f0[0] = 0;

              auStack_f0[1] = 0;

              auStack_f0[2] = 0;

              auStack_f0[3] = 0;

              auStack_f0[4] = 0;

              auStack_f0[5] = 0;

              auStack_f0[6] = 0;

              auStack_f0[7] = 0;

              auStack_f0[8] = 0;

              auStack_f0[9] = 0;

              auStack_f0[10] = 0;

              auStack_f0[0xb] = 0;

              auStack_f0[0xc] = 0;

              auStack_f0[0xd] = 0;

              auStack_f0[0xe] = 0;

              auStack_f0[0xf] = 0;

              puStack_19c = *ppuStack_ac;

            }

            ppuStack_ac = ppuStack_ac + 1;

            if (uStack_8c == 0) {

              if (puVar22 < puStack_f4) {

                uStack_68 = (1 << (uVar20 & 0xff)) - 1;

                do {

                  if ((int)uVar7 < 6) {

                    if ((int)uVar7 < 0) {

                      return 0x238;

                    }

                    uVar21 = 0x1f - uVar7;

                    uVar23 = bitstream_getbits_be(&uStack_70,uVar21);

                    uVar10 = uVar23 | uVar10 << (uVar21 & 0xff);

                    uVar7 = uVar7 + uVar21;

                  }

                  puVar19 = (ushort *)(iVar8 + (uStack_68 & uVar10 >> (uVar7 - uVar20 & 0xff)) * 2);

                  bVar25 = (byte)*puVar19;

                  uVar23 = uVar20;

                  while ((bVar25 & 1) == 0) {

                    uVar7 = uVar7 - uVar23;

                    if ((int)uVar7 < 6) {

                      if ((int)uVar7 < 0) {

                        return 0x238;

                      }

                      uVar21 = 0x1f - uVar7;

                      uVar23 = bitstream_getbits_be(&uStack_70,uVar21);

                      uVar10 = uVar23 | uVar10 << (uVar21 & 0xff);

                      uVar7 = uVar7 + uVar21;

                    }

                    uVar23 = (*puVar19 & 0xf) >> 1;

                    puVar19 = (ushort *)

                              (iVar8 + (((1 << uVar23) - 1U & uVar10 >> (uVar7 - uVar23 & 0xff)) +

                                       (uint)(*puVar19 >> 4)) * 2);

                    bVar25 = (byte)*puVar19;

                  }

                  uVar7 = uVar7 - (((byte)*puVar19 & 0xf) >> 1);

                  uVar23 = (uint)(byte)((byte)*puVar19 >> 4);

                  if (uVar23 == 0) {

                    *puVar22 = 0;

                  }

                  else {

                    uVar21 = auStack_f0[uVar23];

                    if (uVar21 == 0) {

                      uVar21 = FUN_03070cb8(uVar23,puStack_19c);

                      auStack_f0[uVar23] = uVar21;

                    }

                    uVar23 = uVar7 - 1;

                    uVar7 = uVar7 - 1;

                    if ((1 << (uVar23 & 0xff) & uVar10) != 0) {

                      uVar21 = -uVar21;

                    }

                    *puVar22 = uVar21;

                  }

                  uVar23 = (*puVar19 & 0xfff) >> 8;

                  if (uVar23 == 0) {

                    puVar22[1] = 0;

                  }

                  else {

                    uVar21 = auStack_f0[uVar23];

                    if (uVar21 == 0) {

                      uVar21 = FUN_03070cb8(uVar23,puStack_19c);

                      auStack_f0[uVar23] = uVar21;

                    }

                    uVar23 = uVar7 - 1;

                    uVar7 = uVar7 - 1;

                    if ((1 << (uVar23 & 0xff) & uVar10) != 0) {

                      uVar21 = -uVar21;

                    }

                    puVar22[1] = uVar21;

                  }

                  puVar22 = puVar22 + 2;

                } while (puVar22 < puStack_f4);

              }

            }

            else if (puVar22 < puStack_f4) {

              uStack_74 = (1 << (uStack_8c & 0xff)) - 1;

              local_40 = (1 << (uVar20 & 0xff)) - 1;

              do {

                if ((int)uVar7 < 0x13) {

                  if ((int)uVar7 < 0) {

                    return 0x238;

                  }

                  uVar21 = 0x1f - uVar7;

                  uVar23 = bitstream_getbits_be(&uStack_70,uVar21);

                  uVar10 = uVar23 | uVar10 << (uVar21 & 0xff);

                  uVar7 = uVar7 + uVar21;

                }

                puVar19 = (ushort *)(iVar8 + (local_40 & uVar10 >> (uVar7 - uVar20 & 0xff)) * 2);

                bVar25 = (byte)*puVar19;

                uVar23 = uVar20;

                while ((bVar25 & 1) == 0) {

                  uVar7 = uVar7 - uVar23;

                  uVar23 = (*puVar19 & 0xf) >> 1;

                  puVar19 = (ushort *)

                            (iVar8 + (((1 << uVar23) - 1U & uVar10 >> (uVar7 - uVar23 & 0xff)) +

                                     (uint)(*puVar19 >> 4)) * 2);

                  bVar25 = (byte)*puVar19;

                }

                uVar7 = uVar7 - (((byte)*puVar19 & 0xf) >> 1);

                uVar23 = (uint)(byte)((byte)*puVar19 >> 4);

                if (uVar23 == 0) {

                  *puVar22 = 0;

                }

                else {

                  if (uVar23 == 0xf) {

                    if ((int)uVar7 < 0xf) {

                      if ((int)uVar7 < 0) {

                        return 0x238;

                      }

                      uVar21 = 0x1f - uVar7;

                      uVar23 = bitstream_getbits_be(&uStack_70,uVar21);

                      uVar10 = uVar23 | uVar10 << (uVar21 & 0xff);

                      uVar7 = uVar7 + uVar21;

                    }

                    uVar7 = uVar7 - uStack_8c;

                    uVar21 = FUN_03070cb8((uStack_74 & uVar10 >> (uVar7 & 0xff)) + 0xf,puStack_19c);

                  }

                  else {

                    uVar21 = auStack_f0[uVar23];

                    if (uVar21 == 0) {

                      uVar21 = FUN_03070cb8(uVar23,puStack_19c);

                      auStack_f0[uVar23] = uVar21;

                    }

                  }

                  uVar23 = uVar7 - 1;

                  uVar7 = uVar7 - 1;

                  if ((1 << (uVar23 & 0xff) & uVar10) != 0) {

                    uVar21 = -uVar21;

                  }

                  *puVar22 = uVar21;

                }

                uVar23 = (*puVar19 & 0xfff) >> 8;

                if (uVar23 == 0) {

                  puVar22[1] = 0;

                }

                else {

                  if (uVar23 == 0xf) {

                    if ((int)uVar7 < 0xe) {

                      if ((int)uVar7 < 0) {

                        return 0x238;

                      }

                      uVar21 = 0x1f - uVar7;

                      uVar23 = bitstream_getbits_be(&uStack_70,uVar21);

                      uVar10 = uVar23 | uVar10 << (uVar21 & 0xff);

                      uVar7 = uVar7 + uVar21;

                    }

                    uVar7 = uVar7 - uStack_8c;

                    uVar21 = FUN_03070cb8((uStack_74 & uVar10 >> (uVar7 & 0xff)) + 0xf,puStack_19c);

                  }

                  else {

                    uVar21 = auStack_f0[uVar23];

                    if (uVar21 == 0) {

                      uVar21 = FUN_03070cb8(uVar23,puStack_19c);

                      auStack_f0[uVar23] = uVar21;

                    }

                  }

                  uVar23 = uVar7 - 1;

                  uVar7 = uVar7 - 1;

                  if ((1 << (uVar23 & 0xff) & uVar10) != 0) {

                    uVar21 = -uVar21;

                  }

                  puVar22[1] = uVar21;

                }

                puVar22 = puVar22 + 2;

              } while (puVar22 < puStack_f4);

            }

          } while (puVar22 < puStack_f8);

        }

        FUN_0306e302();

        iVar8 = local_90[1] - uStack_6c;

        if ((int)(iVar8 + uVar7) < 0) {

          iVar8 = 0;

        }

        iVar9 = *(int *)(DAT_03072070 + ((byte)local_54[4] & 1) * 4);

        uVar20 = FUN_03070cb8(1,puStack_19c);

        puVar27 = local_94 + 0x23c;

        while ((puVar22 <= puVar27 && (0 < (int)(iVar8 + uVar7)))) {

          if ((int)uVar7 < 10) {

            if ((int)uVar7 < 0) {

              return 0x238;

            }

            uVar21 = 0x1f - uVar7;

            uVar23 = bitstream_getbits_be(&uStack_70,uVar21);

            uVar10 = uVar23 | uVar10 << (uVar21 & 0xff);

            uVar7 = uVar7 + uVar21;

            iVar8 = iVar8 - uVar21;

          }

          puVar19 = (ushort *)(iVar9 + (uVar10 >> (uVar7 - 4 & 0xff) & 0xf) * 2);

          uVar2 = *puVar19;

          if ((uVar2 & 1) == 0) {

            uVar7 = uVar7 - 4;

            uVar23 = (uVar2 & 0xf) >> 1;

            puVar19 = (ushort *)

                      (iVar9 + (((1 << uVar23) - 1U & uVar10 >> (uVar7 - uVar23 & 0xff)) +

                               (uint)(uVar2 >> 4)) * 2);

          }

          uVar7 = uVar7 - (((byte)*puVar19 & 0xf) >> 1);

          if (puVar22 == local_1a4) {

            local_1a4 = local_1a4 + *local_64;

            local_64 = local_64 + 1;

            if (*ppuStack_ac != puStack_19c) {

              puStack_19c = *ppuStack_ac;

              uVar20 = FUN_03070cb8(1);

            }

            ppuStack_ac = ppuStack_ac + 1;

          }

          uVar23 = (byte)*puVar19 & 0x10;

          if ((((byte)*puVar19 & 0x10) != 0) &&

             (uVar7 = uVar7 - 1, uVar23 = uVar20, (1 << (uVar7 & 0xff) & uVar10) != 0)) {

            uVar23 = -uVar20;

          }

          *puVar22 = uVar23;

          uVar23 = (byte)*puVar19 & 0x20;

          if ((((byte)*puVar19 & 0x20) != 0) &&

             (uVar7 = uVar7 - 1, uVar23 = uVar20, (1 << (uVar7 & 0xff) & uVar10) != 0)) {

            uVar23 = -uVar20;

          }

          puVar22[1] = uVar23;

          if (puVar22 + 2 == local_1a4) {

            local_1a4 = local_1a4 + *local_64;

            local_64 = local_64 + 1;

            if (*ppuStack_ac != puStack_19c) {

              puStack_19c = *ppuStack_ac;

              uVar20 = FUN_03070cb8(1);

            }

            ppuStack_ac = ppuStack_ac + 1;

          }

          uVar23 = (byte)*puVar19 & 0x40;

          if ((((byte)*puVar19 & 0x40) != 0) &&

             (uVar7 = uVar7 - 1, uVar23 = uVar20, (1 << (uVar7 & 0xff) & uVar10) != 0)) {

            uVar23 = -uVar20;

          }

          puVar22[2] = uVar23;

          uVar23 = (byte)*puVar19 & 0x80;

          if ((((byte)*puVar19 & 0x80) != 0) &&

             (uVar7 = uVar7 - 1, uVar23 = uVar20, (1 << (uVar7 & 0xff) & uVar10) != 0)) {

            uVar23 = -uVar20;

          }

          puVar22[3] = uVar23;

          puVar22 = puVar22 + 4;

        }

        if ((int)(iVar8 + uVar7) < 0) {

          puVar22 = puVar22 + -4;

        }

        if ((int)local_94 - (int)puVar22 == -0x900) {

          FUN_0306e302();

        }

        else {

          printf(puVar22);

        }

        local_198 = local_198 + 1;

      } while (local_198 < local_28);

    }

    iVar9 = local_88[0];

    iVar8 = DAT_03072074;

    if ((*(char *)(auStack_f0[0x10] + 1) == '\x02') && (*(int *)(auStack_f0[0x10] + 4) != 0)) {

      local_128 = local_60;

      if ((*(char *)(local_60 + 9) != *(char *)(local_60 + 0x43)) ||

         ((*(byte *)(local_60 + 8) & 8) != (*(byte *)(local_60 + 0x42) & 8))) {

        return 0x239;

      }

      puVar14 = &local_1b0;

      local_1b0 = (uint *)CONCAT22(local_1b0._2_2_,*(undefined2 *)(auStack_f0[0x10] + 4));

      iVar15 = 0x13;

      do {

        iVar15 = iVar15 + -1;

        *(undefined2 *)((int)puVar14 + 2) = *(undefined2 *)(auStack_f0[0x10] + 4);

        puVar14 = puVar14 + 1;

        *(undefined2 *)puVar14 = *(undefined2 *)(auStack_f0[0x10] + 4);

      } while (iVar15 != 0);

      uStack_124 = *(uint *)(auStack_f0[0x10] + 4);

      if ((uStack_124 & 1) != 0) {

        iStack_14c = local_128 + 0x3a;

        iVar12 = DAT_03072074 + 0x900;

        *(uint *)(auStack_f0[0x10] + 0x18) = *(uint *)(auStack_f0[0x10] + 0x18) | 0x100;

        iVar15 = DAT_030724f0;

        if (*(char *)(local_128 + 0x43) == '\x02') {

          uVar17 = 0;

          auStack_138[0] = 0;

          auStack_138[2] = 0;

          auStack_138[1] = 0;

          uVar10 = 0;

          uVar20 = 0;

          uVar7 = 0;

          uVar21 = 0;

          uVar23 = uVar7;

          if ((*(byte *)(local_128 + 0x42) & 8) == 0) {

LAB_03071f80:

            iVar24 = 0;

            uVar23 = uVar20;

            do {

              uVar31 = (uint)*(byte *)(iVar9 + uVar23);

              uVar23 = uVar23 + 1;

              uVar17 = 0;

              if (uVar31 != 0) {

                do {

                  if (*(int *)(iVar12 + uVar17 * 4) != 0) {

                    auStack_138[iVar24] = uVar23;

                    uVar10 = uVar23;

                    break;

                  }

                  uVar17 = uVar17 + 1;

                } while (uVar17 < uVar31);

              }

              uVar21 = uVar21 + uVar31;

              iVar12 = iVar12 + uVar31 * 4;

              iVar24 = (uint)((ulonglong)DAT_03072078 * (ulonglong)(iVar24 + 1U) >> 0x21) * -3 +

                       iVar24 + 1U;

            } while (uVar21 < 0x240);

          }

          else {

            do {

              uVar31 = (uint)*(byte *)(iVar9 + uVar17);

              uVar17 = uVar17 + 1;

              uVar20 = 0;

              uVar7 = uVar23;

              if (uVar31 != 0) {

                do {

                  uVar7 = uVar17;

                  if (*(int *)(iVar12 + uVar20 * 4) != 0) break;

                  uVar20 = uVar20 + 1;

                  uVar7 = uVar23;

                } while (uVar20 < uVar31);

              }

              uVar21 = uVar21 + uVar31;

              iVar12 = iVar12 + uVar31 * 4;

              uVar23 = uVar7;

            } while (uVar21 < 0x24);

            uVar20 = uVar17;

            if (uVar21 < 0x240) goto LAB_03071f80;

          }

          if (uVar10 != 0) {

            uVar7 = uVar20;

          }

          if (uVar7 != 0) {

            puVar19 = &uStack_1b2;

            if ((uVar7 & 1) != 0) {

              puVar19 = (ushort *)&local_1b0;

              local_1b0 = (uint *)(CONCAT22(local_1b0._2_2_,(short)uStack_124) & 0xfffffffe);

            }

            for (uVar7 = uVar7 >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {

              puVar19[1] = *(ushort *)(auStack_f0[0x10] + 4) & 0xfffe;

              puVar19 = puVar19 + 2;

              *puVar19 = *(ushort *)(auStack_f0[0x10] + 4) & 0xfffe;

            }

          }

          iVar12 = 0;

          if (uVar20 < uVar10) {

            puVar19 = (ushort *)((int)&local_1b0 + uVar20 * 2);

            iVar24 = 0;

            iVar18 = uVar10 - uVar20;

            do {

              if (uVar20 + iVar24 < auStack_138[iVar12]) {

                *puVar19 = *(ushort *)(auStack_f0[0x10] + 4) & 0xfffe;

              }

              puVar19 = puVar19 + 1;

              iVar24 = iVar24 + 1;

              iVar12 = iVar12 + 1U +

                       (uint)((ulonglong)DAT_03072078 * (ulonglong)(iVar12 + 1U) >> 0x21) * -3;

              iVar18 = iVar18 + -1;

            } while (iVar18 != 0);

          }

        }

        else {

          uVar10 = 0;

          uVar20 = 0;

          uVar7 = 0;

          do {

            uVar21 = 0;

            uVar17 = (uint)*(byte *)(iVar9 + uVar20);

            uVar20 = uVar20 + 1;

            uVar23 = uVar7;

            if (uVar17 != 0) {

              do {

                uVar23 = uVar20;

                if (*(int *)(iVar12 + uVar21 * 4) != 0) break;

                uVar21 = uVar21 + 1;

                uVar23 = uVar7;

              } while (uVar21 < uVar17);

            }

            uVar10 = uVar10 + uVar17;

            iVar12 = iVar12 + uVar17 * 4;

            uVar7 = uVar23;

          } while (uVar10 < 0x240);

          if (uVar23 != 0) {

            puVar19 = &uStack_1b2;

            if ((uVar23 & 1) != 0) {

              puVar19 = (ushort *)&local_1b0;

              local_1b0 = (uint *)(CONCAT22(local_1b0._2_2_,(short)uStack_124) & 0xfffffffe);

            }

            for (uVar23 = uVar23 >> 1; uVar23 != 0; uVar23 = uVar23 - 1) {

              puVar19[1] = *(ushort *)(auStack_f0[0x10] + 4) & 0xfffe;

              puVar19 = puVar19 + 2;

              *puVar19 = *(ushort *)(auStack_f0[0x10] + 4) & 0xfffe;

            }

          }

        }

        if ((*(ushort *)(auStack_f0[0x10] + 0x18) & 0x1000) == 0) {

          iVar12 = 0;

          uVar7 = 0;

          do {

            uVar10 = (uint)*(byte *)(iVar9 + iVar12);

            uVar2 = *(ushort *)((int)&local_1b0 + iVar12 * 2);

            if ((uVar2 & 1) != 0) {

              uVar20 = (uint)*(byte *)(iStack_14c + iVar12 + 0x12);

              if (uVar20 < 7) {

                if (uVar10 != 0) {

                  piVar11 = (int *)(iVar8 + uVar7 * 4);

                  uVar23 = uVar10;

                  piVar32 = piVar11 + 0x240;

                  do {

                    iVar24 = *piVar11 + 0x800 >> 0xc;

                    *piVar11 = iVar24 * *(int *)(iVar15 + uVar20 * 4);

                    uVar23 = uVar23 - 1;

                    *piVar32 = iVar24 * *(int *)(iVar15 + (6 - uVar20) * 4);

                    piVar11 = piVar11 + 1;

                    piVar32 = piVar32 + 1;

                  } while (uVar23 != 0);

                }

              }

              else {

                *(ushort *)((int)&local_1b0 + iVar12 * 2) = uVar2 & 0xfffe;

              }

            }

            iVar12 = iVar12 + 1;

            uVar7 = uVar7 + uVar10;

          } while (uVar7 < 0x240);

        }

        else {

          auStack_138[3] = local_128 + 0xc0;

          iVar15 = 0;

          iVar12 = DAT_030724ec + (*(byte *)(iStack_14c + 6) & 1) * 0x3c;

          uVar7 = 0;

          do {

            uVar10 = (uint)*(byte *)(iVar9 + iVar15);

            uVar2 = *(ushort *)((int)&local_1b0 + iVar15 * 2);

            if ((uVar2 & 1) != 0) {

              if (*(char *)(auStack_138[3] + iVar15) == '\0') {

                bVar25 = *(byte *)(iStack_14c + iVar15 + 0x12);

                if (uVar10 != 0) {

                  iVar24 = 0;

                  uVar20 = uVar10;

                  do {

                    iVar26 = uVar7 + iVar24;

                    iVar18 = *(int *)(iVar8 + iVar26 * 4);

                    if (bVar25 == 0) {

                      *(int *)(iVar8 + iVar26 * 4 + 0x900) = iVar18;

                    }

                    else {

                      iVar30 = (iVar18 + 0x800 >> 0xc) * *(int *)(iVar12 + (bVar25 - 1 >> 1) * 4);

                      if ((bVar25 & 1) == 0) {

                        *(int *)(iVar8 + iVar26 * 4 + 0x900) = iVar30;

                      }

                      else {

                        *(int *)(iVar8 + iVar26 * 4) = iVar30;

                        *(int *)(iVar8 + iVar26 * 4 + 0x900) = iVar18;

                      }

                    }

                    iVar24 = iVar24 + 1;

                    uVar20 = uVar20 - 1;

                  } while (uVar20 != 0);

                }

              }

              else {

                *(ushort *)((int)&local_1b0 + iVar15 * 2) = uVar2 & 0xfffe;

              }

            }

            iVar15 = iVar15 + 1;

            uVar7 = uVar7 + uVar10;

          } while (uVar7 < 0x240);

        }

      }

      if ((*(byte *)(auStack_f0[0x10] + 4) & 2) != 0) {

        *(uint *)(auStack_f0[0x10] + 0x18) = *(uint *)(auStack_f0[0x10] + 0x18) | 0x200;

        iVar15 = 0;

        uVar7 = 0;

        iVar12 = *(int *)(DAT_030724f4 + 4);

        do {

          uVar10 = (uint)*(byte *)(iVar9 + iVar15);

          if ((*(short *)((int)&local_1b0 + iVar15 * 2) == 2) && (uVar10 != 0)) {

            piVar11 = (int *)(iVar8 + ((uVar7 - 1) + uVar10) * 4);

            uVar20 = uVar10;

            piVar32 = piVar11 + 0x240;

            do {

              iVar18 = *piVar11;

              iVar24 = *piVar32;

              *piVar11 = (iVar18 + iVar24 + 0x800 >> 0xc) * iVar12;

              *piVar32 = ((iVar18 - iVar24) + 0x800 >> 0xc) * iVar12;

              uVar20 = uVar20 - 1;

              piVar11 = piVar11 + -1;

              piVar32 = piVar32 + -1;

            } while (uVar20 != 0);

          }

          iVar15 = iVar15 + 1;

          uVar7 = uVar7 + uVar10;

        } while (uVar7 < 0x240);

      }

    }

    FUN_0306e300();

    uVar7 = 0;

    if (local_28 != 0) {

      do {

        iVar9 = local_60 + uVar7 * 0x3a;

        iVar8 = *(int *)(local_30 + 0x24);

        local_1b0 = (uint *)(DAT_030724f8 + uVar7 * 0x900);

        if (*(char *)(iVar9 + 9) == '\x02') {

          pbVar16 = (byte *)local_88[uVar7];

          uStack_190 = 0;

          if ((*(byte *)(iVar9 + 8) & 8) != 0) {

            uStack_190 = 2;

            uVar10 = 0;

            pbVar13 = pbVar16;

            do {

              pbVar16 = pbVar13 + 1;

              uVar10 = uVar10 + *pbVar13;

              pbVar13 = pbVar16;

            } while (uVar10 < 0x24);

          }

          aiStack_a4[1] = 0;

          uStack_78 = uStack_190;

          aiStack_a4[0] = 0;

          local_88[2] = uStack_190;

          local_88[3] = uStack_190;

          aiStack_a4[2] = 0;

          local_1ac = uStack_190 * 9;

          pbVar13 = pbVar16 + 1;

          uVar10 = (uint)*pbVar16;

          iVar15 = 0;

          local_194 = local_1b0;

          if (uStack_190 * 0x12 < 0x240) {

            iVar12 = uStack_190 * -0x12 + 0x240;

            puVar22 = local_1b0 + uStack_190 * 0x12;

            do {

              if (uVar10 == 0) {

                uVar10 = (uint)*pbVar13;

                iVar15 = iVar15 + 1U +

                         (uint)((ulonglong)DAT_03072504 * (ulonglong)(iVar15 + 1U) >> 0x21) * -3;

                pbVar13 = pbVar13 + 1;

              }

              uVar10 = uVar10 - 1;

              iVar18 = local_88[iVar15 + 2];

              iVar24 = aiStack_a4[iVar15];

              auStack_ab0[iVar18 * 0x12 + iVar15 * 6 + iVar24] = *puVar22;

              iVar24 = iVar24 + 1;

              aiStack_a4[iVar15] = iVar24;

              if (iVar24 == 6) {

                aiStack_a4[iVar15] = 0;

                local_88[iVar15 + 2] = iVar18 + 1;

              }

              iVar12 = iVar12 + -1;

              puVar22 = puVar22 + 1;

            } while (iVar12 != 0);

          }

          hifi_memmove(local_1b0 + local_1ac * 2,auStack_ab0 + local_1ac * 2,

                       uStack_190 * -0x48 + 0x900);

          if ((*(byte *)(iVar9 + 8) & 8) != 0) {

            apuStack_18c[0] = local_194 + 0x24;

            local_1a4 = local_194;

            while (piVar32 = DAT_03072500, piVar11 = DAT_030724fc, local_1a4 = local_1a4 + 0x12,

                  DAT_030724fc = piVar11, DAT_03072500 = piVar32, local_1a4 < apuStack_18c[0]) {

              iVar15 = 8;

              puVar27 = local_1a4;

              puVar22 = local_1a4;

              do {

                puVar22 = puVar22 + -1;

                uVar20 = *puVar22;

                uVar10 = *puVar27;

                if (uVar20 != 0 || uVar10 != 0) {

                  lVar3 = ((longlong)(int)uVar20 * (longlong)*piVar11 -

                          (longlong)(int)uVar10 * (longlong)*piVar32) + 0x800;

                  *puVar22 = (uint)lVar3 >> 0xc | (int)((ulonglong)lVar3 >> 0x20) * 0x100000;

                  lVar3 = (longlong)(int)uVar10 * (longlong)*piVar11 +

                          ((longlong)

                           (int)((ulonglong)((longlong)(int)uVar20 * (longlong)*piVar32) >> 0x20) <<

                           0x20 | (longlong)(int)uVar20 * (longlong)*piVar32 & 0xffffffffU) + 0x800;

                  *puVar27 = (uint)lVar3 >> 0xc | (int)((ulonglong)lVar3 >> 0x20) * 0x100000;

                }

                puVar27 = puVar27 + 1;

                piVar11 = piVar11 + 1;

                piVar32 = piVar32 + 1;

                iVar15 = iVar15 + -1;

              } while (iVar15 != 0);

            }

          }

        }

        else {

          apuStack_18c[1] = local_1b0 + 0x240;

          while (piVar32 = DAT_03072500, piVar11 = DAT_030724fc, local_1b0 = local_1b0 + 0x12,

                DAT_030724fc = piVar11, DAT_03072500 = piVar32, local_1b0 < apuStack_18c[1]) {

            iVar15 = 8;

            puVar27 = local_1b0;

            puVar22 = local_1b0;

            do {

              puVar22 = puVar22 + -1;

              uVar20 = *puVar22;

              uVar10 = *puVar27;

              if (uVar20 != 0 || uVar10 != 0) {

                lVar3 = ((longlong)(int)uVar20 * (longlong)*piVar11 -

                        (longlong)(int)uVar10 * (longlong)*piVar32) + 0x800;

                *puVar22 = (int)((ulonglong)lVar3 >> 0x20) * 0x100000 | (uint)lVar3 >> 0xc;

                lVar3 = (longlong)(int)uVar10 * (longlong)*piVar11 +

                        ((longlong)

                         (int)((ulonglong)((longlong)(int)uVar20 * (longlong)*piVar32) >> 0x20) <<

                         0x20 | (longlong)(int)uVar20 * (longlong)*piVar32 & 0xffffffffU) + 0x800;

                *puVar27 = (uint)lVar3 >> 0xc | (int)((ulonglong)lVar3 >> 0x20) * 0x100000;

              }

              puVar27 = puVar27 + 1;

              piVar11 = piVar11 + 1;

              piVar32 = piVar32 + 1;

              iVar15 = iVar15 + -1;

            } while (iVar15 != 0);

          }

        }

        iVar15 = DAT_03072960;

        iVar12 = 0;

        uVar10 = 0x240;

        iVar24 = DAT_030724f8 + uVar7 * 0x900;

        do {

          if (*(int *)(iVar24 + uVar10 * 4 + -4) != 0) break;

          uVar10 = uVar10 - 1;

        } while (0x24 < uVar10);

        iVar18 = -(uint)((ulonglong)DAT_03072954 * (ulonglong)(0x240 - uVar10) >> 0x22);

        puVar22 = (uint *)(iVar18 + 0x20);

        if (*(char *)(iVar9 + 9) == '\x02') {

          if ((*(byte *)(iVar9 + 8) & 8) == 0) {

            puVar27 = (uint *)0x0;

            if (puVar22 != (uint *)0x0) {

              do {

                FUN_03070ec4(iVar24 + iVar12 * 4,DAT_0307295c);

                FUN_03071042(DAT_0307295c,iVar15 + (int)(puVar27 + uVar7 * 8) * 0x48,iVar8,puVar27);

                if (((uint)puVar27 & 1) != 0) {

                  iVar9 = iVar8 + (int)puVar27 * 4;

                  *(int *)(iVar9 + 0x80) = -*(int *)(iVar9 + 0x80);

                  iVar9 = iVar9 + 0x80;

                  iVar18 = 4;

                  do {

                    *(int *)(iVar9 + 0x100) = -*(int *)(iVar9 + 0x100);

                    *(int *)(iVar9 + 0x200) = -*(int *)(iVar9 + 0x200);

                    iVar9 = iVar9 + 0x200;

                    iVar18 = iVar18 + -1;

                  } while (iVar18 != 0);

                }

                puVar27 = (uint *)((int)puVar27 + 1);

                iVar12 = iVar12 + 0x12;

              } while (puVar27 < puVar22);

            }

          }

          else {

            uVar10 = 0;

            do {

              FUN_03070ec4(iVar24 + iVar12 * 4,DAT_0307295c);

              FUN_03071042(DAT_0307295c,iVar15 + (uVar10 + uVar7 * 0x20) * 0x48,iVar8,uVar10);

              uVar10 = uVar10 + 1;

              iVar12 = iVar12 + 0x12;

            } while (uVar10 < 2);

            puStack_19c = (uint *)(iVar18 + 0x21);

            puVar27 = (uint *)0x2;

            if ((uint *)0x2 < puStack_19c) {

              do {

                *DAT_03072958 = 1;

                while( true ) {

                  puVar29 = puVar27;

                  if (puVar29 != puVar22) {

                    do {

                    } while (*DAT_03072958 == 0);

                    iVar18 = iVar24 + iVar12 * 4;

                    *DAT_03072964 = DAT_0307295c;

                    iVar15 = FUN_0306e39c(iVar18);

                    if (iVar15 == 0) {

                      hifi_memmove(DAT_03072968,iVar18,0x48);

                    }

                  }

                  puVar27 = (uint *)((int)puVar29 + 1);

                  iVar12 = iVar12 + 0x12;

                  if (puStack_19c <= puVar27) goto LAB_030727ba;

                  if (puVar27 == (uint *)0x2) break;

                  do {

                  } while (*DAT_03072958 == 0);

                  FUN_03070d34(iVar24 + iVar12 * 4,DAT_0307295c,*(undefined1 *)(iVar9 + 9));

                  FUN_03071042(DAT_03072960 + 0x2400,

                               DAT_03072960 + (int)(puVar29 + uVar7 * 8) * 0x48,iVar8,puVar29);

                  if (((uint)puVar29 & 1) != 0) {

                    iVar15 = iVar8 + (int)puVar29 * 4;

                    *(int *)(iVar15 + 0x80) = -*(int *)(iVar15 + 0x80);

                    iVar15 = iVar15 + 0x80;

                    iVar18 = 4;

                    do {

                      *(int *)(iVar15 + 0x100) = -*(int *)(iVar15 + 0x100);

                      *(int *)(iVar15 + 0x200) = -*(int *)(iVar15 + 0x200);

                      iVar15 = iVar15 + 0x200;

                      iVar18 = iVar18 + -1;

                    } while (iVar18 != 0);

                  }

                }

              } while( true );

            }

          }

        }

        else {

          puVar27 = (uint *)0x0;

          puVar29 = (uint *)(iVar18 + 0x21);

          if ((uint *)(iVar18 + 0x21) != (uint *)0x0) {

            do {

              apuStack_18c[0] = puVar29;

              *DAT_03072958 = 1;

              while( true ) {

                puVar28 = puVar27;

                if (puVar28 != puVar22) {

                  do {

                  } while (*DAT_03072958 == 0);

                  *DAT_03072958 = 0;

                  iVar18 = iVar24 + iVar12 * 4;

                  *DAT_03072964 = DAT_0307295c;

                  iVar15 = FUN_0306e39c(iVar18);

                  if (iVar15 == 0) {

                    hifi_memmove(DAT_03072968,iVar18,0x48);

                  }

                }

                puVar27 = (uint *)((int)puVar28 + 1);

                iVar12 = iVar12 + 0x12;

                if (apuStack_18c[0] <= puVar27) goto LAB_030727ba;

                puVar29 = apuStack_18c[0];

                if (puVar27 == (uint *)0x0) break;

                do {

                } while (*DAT_03072958 == 0);

                FUN_03070d34(iVar24 + iVar12 * 4,DAT_0307295c,*(undefined1 *)(iVar9 + 9));

                FUN_03071042(DAT_03072960 + 0x2400,DAT_03072960 + (int)(puVar28 + uVar7 * 8) * 0x48,

                             iVar8,puVar28);

                if (((uint)puVar28 & 1) != 0) {

                  iVar15 = iVar8 + (int)puVar28 * 4;

                  *(int *)(iVar15 + 0x80) = -*(int *)(iVar15 + 0x80);

                  iVar15 = iVar15 + 0x80;

                  iVar18 = 4;

                  do {

                    *(int *)(iVar15 + 0x100) = -*(int *)(iVar15 + 0x100);

                    *(int *)(iVar15 + 0x200) = -*(int *)(iVar15 + 0x200);

                    iVar15 = iVar15 + 0x200;

                    iVar18 = iVar18 + -1;

                  } while (iVar18 != 0);

                }

              }

            } while( true );

          }

        }

LAB_030727ba:

        iVar9 = DAT_03072960;

        if (puVar22 < &Reserved2) {

          do {

            puVar14 = (undefined4 *)(iVar9 + (int)(puVar22 + uVar7 * 8) * 0x48);

            *(undefined4 *)(iVar8 + (int)puVar22 * 4) = *puVar14;

            iVar15 = iVar8 + (int)puVar22 * 4;

            *(undefined4 *)(iVar15 + 0x80) = puVar14[1];

            *(undefined4 *)(iVar15 + 0x100) = puVar14[2];

            *(undefined4 *)(iVar15 + 0x180) = puVar14[3];

            *(undefined4 *)(iVar15 + 0x200) = puVar14[4];

            *(undefined4 *)(iVar15 + 0x280) = puVar14[5];

            *(undefined4 *)(iVar15 + 0x300) = puVar14[6];

            *(undefined4 *)(iVar15 + 0x380) = puVar14[7];

            *(undefined4 *)(iVar15 + 0x400) = puVar14[8];

            *(undefined4 *)(iVar15 + 0x480) = puVar14[9];

            *(undefined4 *)(iVar15 + 0x500) = puVar14[10];

            *(undefined4 *)(iVar15 + 0x580) = puVar14[0xb];

            *(undefined4 *)(iVar15 + 0x600) = puVar14[0xc];

            *(undefined4 *)(iVar15 + 0x680) = puVar14[0xd];

            *(undefined4 *)(iVar15 + 0x700) = puVar14[0xe];

            *(undefined4 *)(iVar15 + 0x780) = puVar14[0xf];

            *(undefined4 *)(iVar15 + 0x800) = puVar14[0x10];

            *(undefined4 *)(iVar15 + 0x880) = puVar14[0x11];

            *puVar14 = 0;

            puVar14[1] = 0;

            puVar14[2] = 0;

            puVar14[3] = 0;

            puVar14[4] = 0;

            puVar14[5] = 0;

            puVar14[6] = 0;

            puVar14[7] = 0;

            puVar14[8] = 0;

            puVar14[9] = 0;

            puVar14[10] = 0;

            puVar14[0xb] = 0;

            puVar14[0xc] = 0;

            puVar14[0xd] = 0;

            puVar14[0xe] = 0;

            puVar14[0xf] = 0;

            puVar14[0x10] = 0;

            puVar14[0x11] = 0;

            if (((uint)puVar22 & 1) != 0) {

              *(int *)(iVar15 + 0x80) = -*(int *)(iVar15 + 0x80);

              iVar15 = iVar15 + 0x80;

              iVar12 = 4;

              do {

                iVar12 = iVar12 + -1;

                *(int *)(iVar15 + 0x100) = -*(int *)(iVar15 + 0x100);

                *(int *)(iVar15 + 0x200) = -*(int *)(iVar15 + 0x200);

                iVar15 = iVar15 + 0x200;

              } while (iVar12 != 0);

            }

            puVar22 = (uint *)((int)puVar22 + 1);

          } while (puVar22 < &Reserved2);

        }

        do {

        } while (*DAT_0307296c == 1);

        RecordCopyEncDataToBuf(local_30,uVar7,0x12,local_a8);

        FUN_0306f366(local_30);

        uVar7 = uVar7 + 1;

      } while (uVar7 < local_28);

    }

    local_a8 = local_a8 + 1;

    if ((uint)aiStack_a4[3] <= local_a8) {

      FUN_0306e302();

      return 0;

    }

  } while( true );

}
