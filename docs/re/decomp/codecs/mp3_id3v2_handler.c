/**
 * mp3_id3v2_handler @ 0x0306fec4
 * Tags: codec, mp3
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 mp3_id3v2_handler(int param_1)



{

  char cVar1;

  byte bVar2;

  undefined4 *puVar3;

  undefined4 *puVar4;

  undefined4 *puVar5;

  undefined4 *puVar6;

  uint *puVar7;

  int *piVar8;

  byte bVar9;

  undefined1 uVar10;

  int iVar11;

  uint uVar12;

  uint uVar13;

  int iVar14;

  undefined4 uVar15;

  undefined4 uVar16;

  undefined4 uVar17;

  uint uVar18;

  uint extraout_r1;

  uint uVar19;

  uint extraout_r1_00;

  uint extraout_r1_01;

  uint extraout_r2;

  uint extraout_r2_00;

  uint extraout_r2_01;

  uint uVar20;

  uint extraout_r3;

  uint extraout_r3_00;

  uint uVar21;

  uint *puVar22;

  uint uVar23;

  int iVar24;

  bool bVar25;

  bool bVar26;

  undefined4 local_2c;

  

  puVar3 = DAT_030701b0;

  puVar22 = DAT_030701b0 + 1;

  *DAT_030701b0 = 0;

  puVar4 = DAT_030701b4;

  iVar24 = 0x19000;

  iVar11 = (*(code *)*DAT_030701b8)(*DAT_030701b4);

  (*(code *)*DAT_030701bc)(iVar11 + -0x20,0,*puVar4);

  puVar6 = DAT_030701c0;

  (*(code *)*DAT_030701c0)(puVar22,0x20,*puVar4);

  iVar11 = FUN_0306d3a2(puVar22,DAT_030701c4,8);

  if (((iVar11 != 0) || (2000 < ((uint)*(byte *)(puVar3 + 3) | *(int *)((int)puVar3 + 0xd) << 8)))

     || (uVar18 = (uint)*(byte *)(puVar3 + 4) | *(int *)((int)puVar3 + 0x11) << 8,

        0x1000000 < uVar18 - 0x20)) {

    uVar18 = 0;

  }

  DAT_030701c8[1] = uVar18;

  hifi_debug_printf(s_apetag____d_030701cc);

  (*(code *)*DAT_030701bc)(0,0,*puVar4);

  (*(code *)*puVar6)(puVar22,0x800,*puVar4);

  puVar7 = DAT_030701c8;

  puVar5 = DAT_030701bc;

  cVar1 = (char)*puVar22;

  bVar25 = cVar1 == 'I';

  if (bVar25) {

    cVar1 = *(char *)((int)puVar3 + 5);

  }

  bVar26 = bVar25 && cVar1 == 'D';

  if (bVar25 && cVar1 == 'D') {

    bVar26 = *(char *)((int)puVar3 + 6) == '3';

  }

  if (bVar26) {

    uVar12 = (uint)*(byte *)(puVar3 + 2);

    bVar25 = 0xfe < uVar12;

    uVar18 = extraout_r2;

    if (uVar12 != 0xff) {

      uVar18 = (uint)*(byte *)((int)puVar3 + 10);

      bVar25 = 0x7f < uVar18;

    }

    if (!bVar25) {

      uVar20 = (uint)*(byte *)((int)puVar3 + 0xb);

      bVar25 = uVar20 < 0x80;

      if (bVar25) {

        uVar12 = (uint)*(byte *)(puVar3 + 3);

      }

      uVar19 = extraout_r1;

      if (bVar25 && uVar12 < 0x80) {

        uVar19 = (uint)*(byte *)((int)puVar3 + 0xd);

      }

      if ((bVar25 && uVar12 < 0x80) && uVar19 < 0x80) {

        uVar19 = uVar19 | uVar18 << 0x15;

        uVar18 = uVar20 << 0xe;

        uVar12 = uVar19 | uVar18 | uVar12 << 7;

        if (uVar12 != 0) {

          do {

            uVar23 = uVar12 + 10;

            uVar13 = *puVar7;

            *puVar7 = uVar13 + uVar23;

            if (uVar23 < 0xc01) {

              if (uVar23 < 0x601) {

                puVar22 = (uint *)((int)puVar22 + uVar23);

              }

              else {

                hifi_memmove(puVar22,puVar22 + 0x180,0x200);

                (*(code *)*puVar6)(puVar22 + 0x80,0x600,*puVar4);

                puVar22 = (uint *)((int)puVar22 + (uVar12 - 0x5f6));

                uVar18 = extraout_r1_01;

                uVar19 = extraout_r2_01;

                uVar20 = extraout_r3_00;

              }

            }

            else {

              (*(code *)*puVar5)(uVar13 + uVar23 & 0xfffffe00,0,*puVar4);

              (*(code *)*puVar6)(puVar22,0x800,*puVar4);

              puVar22 = (uint *)((int)puVar22 + ((ushort)*puVar7 & 0x1ff));

              uVar18 = extraout_r1_00;

              uVar19 = extraout_r2_00;

              uVar20 = extraout_r3;

            }

            cVar1 = (char)*puVar22;

            bVar25 = cVar1 == 'I';

            if (bVar25) {

              cVar1 = *(char *)((int)puVar22 + 1);

            }

            bVar26 = bVar25 && cVar1 == 'D';

            if (bVar25 && cVar1 == 'D') {

              bVar26 = *(char *)((int)puVar22 + 2) == '3';

            }

            if (!bVar26) break;

            bVar25 = 0xfe < (byte)puVar22[1];

            if ((byte)puVar22[1] != 0xff) {

              uVar18 = (uint)*(byte *)((int)puVar22 + 6);

              bVar25 = 0x7f < uVar18;

            }

            if (bVar25) break;

            bVar25 = 0x7f < *(byte *)((int)puVar22 + 7);

            if (!bVar25) {

              uVar20 = (uint)(byte)puVar22[2];

            }

            if (!bVar25 && 0x7f >= uVar20) {

              uVar19 = (uint)*(byte *)((int)puVar22 + 9);

            }

            if ((bVar25 || 0x7f < uVar20) || 0x7f < uVar19) break;

            uVar18 = uVar19 | uVar18 << 0x15;

            uVar12 = uVar18 | (uint)*(byte *)((int)puVar22 + 7) << 0xe | uVar20 << 7;

          } while (uVar12 != 0);

          uVar18 = *puVar7;

          iVar14 = FUN_0306f944(puVar22);

          iVar11 = DAT_030701f8;

          if (iVar14 == 0) {

            iVar11 = 0;

            if ((int)(*DAT_030701c8 - 0x400) < 1) {

              (*(code *)*puVar5)(0,0,*puVar4);

            }

            else {

              (*(code *)*puVar5)(*DAT_030701c8 - 0x400);

            }

            (*(code *)*puVar6)(puVar22,0x800,*puVar4);

            puVar7 = DAT_030701c8;

            iVar14 = 0x7fb;

            do {

              if (*(char *)((int)puVar22 + iVar11) == 'X') {

                cVar1 = *(char *)((int)puVar22 + iVar11 + 1);

                bVar25 = cVar1 == 'i';

                if (bVar25) {

                  cVar1 = *(char *)((int)puVar22 + iVar11 + 2);

                }

                bVar26 = bVar25 && cVar1 == 'n';

                if (bVar25 && cVar1 == 'n') {

                  bVar26 = *(char *)((int)puVar22 + iVar11 + 3) == 'g';

                }

                if (bVar26) {

LAB_030700f2:

                  uVar12 = *DAT_030701c8;

                  iVar14 = uVar12 - 0x400;

                  if (uVar12 != 0x400 && -1 < iVar14) {

                    uVar12 = (uVar12 + iVar11) - 0x424;

                  }

                  if (iVar14 < 1) {

                    uVar12 = iVar11 - 0x24;

                  }

                  *DAT_030701c8 = uVar12;

                  (*(code *)*puVar5)(*puVar7,0,*puVar4);

                  (*(code *)*puVar6)(puVar22,0x800,*puVar4);

                  hifi_debug_printf(s_id3_contain_half_VBR_frame_030701dc);

                  iVar11 = DAT_030701f8;

                  goto LAB_03070138;

                }

              }

              else if (*(char *)((int)puVar22 + iVar11) == 'V') {

                cVar1 = *(char *)((int)puVar22 + iVar11 + 1);

                bVar25 = cVar1 == 'B';

                if (bVar25) {

                  cVar1 = *(char *)((int)puVar22 + iVar11 + 2);

                }

                bVar26 = bVar25 && cVar1 == 'R';

                if (bVar25 && cVar1 == 'R') {

                  bVar26 = *(char *)((int)puVar22 + iVar11 + 3) == 'I';

                }

                if (bVar26) goto LAB_030700f2;

              }

              iVar11 = iVar11 + 1;

              bVar25 = iVar14 != 0;

              iVar14 = iVar14 + -1;

            } while (bVar25);

            (*(code *)*puVar5)(*DAT_030701c8,0,*puVar4);

            (*(code *)*puVar6)(puVar22,0x800,*puVar4);

            iVar11 = DAT_030701f8;

          }

          goto LAB_03070138;

        }

      }

    }

  }

  uVar18 = 0;

  *DAT_030701c8 = 0;

  iVar11 = DAT_030701f8;

LAB_03070138:

  do {

    if ((char)*puVar22 == -1) {

      uVar12 = (uint)*(byte *)((int)puVar22 + 1);

      if ((((~uVar12 & 0xe0) == 0) &&

          ((uVar12 & 0x18) != 8 && (*(byte *)((int)puVar22 + 1) & 6) != 0)) &&

         ((bVar9 = *(byte *)((int)puVar22 + 2), (bVar9 & 0xf0) != 0 &&

          (((~bVar9 & 0xf0) != 0 && ((~bVar9 & 0xc) != 0)))))) {

        uVar12 = (uVar12 & 7) >> 1;

        if (uVar12 == 1) {

LAB_03070192:

          *DAT_030701a0 = 2;

        }

        else if (uVar12 == 2) {

          *DAT_030701a0 = 1;

        }

        else {

          if (uVar12 != 3) goto LAB_03070192;

          *DAT_03070624 = 0;

        }

        bVar9 = ~*(byte *)((int)puVar22 + 3) & 0xc0;

        if ((~*(byte *)((int)puVar22 + 1) & 0x18) == 0) {

          if (bVar9 == 0) {

            iVar14 = 0x15;

          }

          else {

            iVar14 = 0x24;

          }

          local_2c = 0x480;

          *DAT_03070620 = 0x480;

        }

        else {

          if (bVar9 == 0) {

            iVar14 = 0xd;

          }

          else {

            iVar14 = 0x15;

          }

          local_2c = 0x240;

          *DAT_03070620 = 0x240;

        }

        if (*(char *)((int)puVar22 + iVar14) == 'X') {

          cVar1 = *(char *)((int)puVar22 + iVar14 + 1);

          bVar25 = cVar1 == 'i';

          if (bVar25) {

            cVar1 = *(char *)((int)puVar22 + iVar14 + 2);

          }

          bVar26 = bVar25 && cVar1 == 'n';

          if (bVar25 && cVar1 == 'n') {

            bVar26 = *(char *)((int)puVar22 + iVar14 + 3) == 'g';

          }

          if ((!bVar26) || ((*(byte *)((int)puVar22 + iVar14 + 7) & 1) == 0)) goto LAB_03070270;

          uVar12 = *(uint *)((int)puVar22 + iVar14 + 8);

          bVar9 = *(byte *)((int)puVar22 + iVar14 + 0xb);

          uVar12 = uVar12 << 0x18 | (uVar12 >> 8 & 0xff) << 0x10 | (uVar12 >> 0x10 & 0xff) << 8;

LAB_0307029e:

          if ((bVar9 | uVar12) != 0) {

            *(undefined1 *)(param_1 + 0x14) = 1;

            if (*(byte *)((int)puVar22 + 3) >> 6 == 3) {

              uVar10 = 1;

            }

            else {

              uVar10 = 2;

            }

            *(undefined1 *)(param_1 + 0x15) = uVar10;

            puVar3 = DAT_0307062c;

            bVar2 = *(byte *)((int)puVar22 + 1) >> 1;

            uVar20 = bVar2 & 0xc;

            if ((bVar2 & 0xc) != 0) {

              uVar20 = uVar20 - 4 & 0xff;

            }

            *(undefined2 *)(param_1 + 0x16) =

                 *(undefined2 *)

                  (DAT_03070628 + ((*(byte *)((int)puVar22 + 2) & 0xf) >> 2 | uVar20) * 2);

            *(uint *)(param_1 + 0x18) = uVar18;

            iVar11 = (*(code *)*puVar3)(*puVar4);

            iVar11 = (iVar11 - *(int *)(param_1 + 0x18)) - DAT_03070630[1];

            *(int *)(param_1 + 0x1c) = iVar11;

            uVar15 = FUN_0306d4c4(*(undefined2 *)(param_1 + 0x16));

            uVar16 = FUN_0306d4c4(local_2c);

            uVar17 = FUN_0306d4c4(bVar9 | uVar12);

            uVar16 = FUN_0306d3c0(uVar17,uVar16);

            uVar15 = FUN_0306d42c(uVar16,uVar15);

            uVar16 = FUN_0306d4c4(iVar11);

            uVar16 = FUN_0306d4ac(uVar16,3);

            FUN_0306d42c(uVar16,uVar15);

            iVar11 = FUN_0306d4ce();

            *(int *)(param_1 + 0x20) = iVar11;

            if (iVar11 != 0) {

              return 1;

            }

            hifi_debug_printf(s_FILE___s__LINE___d__03070668,

                              s_______Common_Codec_Audio_Mp3_lib_03070634,0x2fe);

            return 0;

          }

        }

        else {

LAB_03070270:

          cVar1 = (char)puVar22[9];

          bVar25 = cVar1 == 'V';

          if (bVar25) {

            cVar1 = *(char *)((int)puVar22 + 0x25);

          }

          if (bVar25 && cVar1 == 'B') {

            cVar1 = *(char *)((int)puVar22 + 0x26);

            bVar25 = cVar1 == 'R';

            if (bVar25) {

              cVar1 = *(char *)((int)puVar22 + 0x27);

            }

            if (bVar25 && cVar1 == 'I') {

              uVar12 = *(uint *)((int)puVar22 + 0x32);

              bVar9 = *(byte *)((int)puVar22 + 0x35);

              uVar12 = uVar12 << 0x18 | (uVar12 >> 8 & 0xff) << 0x10 | (uVar12 >> 0x10 & 0xff) << 8;

              goto LAB_0307029e;

            }

          }

        }

        uVar12 = *puVar22;

        uVar12 = uVar12 << 0x18 | (uVar12 >> 8 & 0xff) << 0x10 | (uVar12 >> 0x10 & 0xff) << 8 |

                 (uint)*(byte *)((int)puVar22 + 3);

        bVar9 = *(byte *)((int)puVar22 + 1) >> 1;

        uVar20 = bVar9 & 0xc;

        uVar19 = (*(byte *)((int)puVar22 + 1) & 7) >> 1;

        if (uVar19 == 1) {

          *DAT_03070624 = 2;

          if ((bVar9 & 0xc) != 0 && uVar20 != 8) goto LAB_03070376;

          *DAT_03070620 = 0x240;

        }

        else {

          if (uVar19 == 2) {

            *DAT_03070624 = 1;

          }

          else {

            if (uVar19 == 3) {

              *DAT_03070624 = 0;

              *DAT_03070620 = 0x180;

              goto LAB_0307037e;

            }

            *DAT_03070624 = 2;

          }

LAB_03070376:

          *DAT_03070620 = 0x480;

        }

LAB_0307037e:

        uVar23 = (uint)*(byte *)((int)puVar22 + 2);

        uVar13 = (uint)(*(byte *)((int)puVar22 + 2) >> 4);

        uVar21 = (uVar23 & 0xf) >> 2;

        if (uVar19 == 1) {

          if ((bVar9 & 0xc) != 0) {

            uVar20 = uVar20 - 4 & 0xff;

          }

          uVar20 = (uint)*(ushort *)(DAT_03070694 + (uVar20 | uVar21) * 0x20 + uVar13 * 2) -

                   ((int)(uVar23 << 0x1e) >> 0x1f) & 0xffff;

        }

        else if (uVar19 == 2) {

          if ((bVar9 & 0xc) != 0) {

            uVar20 = uVar20 - 8 & 0xff;

          }

          uVar20 = (uint)*(ushort *)(DAT_03070698 + (uVar20 | uVar21) * 0x20 + uVar13 * 2) -

                   ((int)(uVar23 << 0x1e) >> 0x1f) & 0xffff;

        }

        else {

          if ((bVar9 & 0xc) != 0) {

            uVar20 = uVar20 - 8 & 0xff;

          }

          uVar20 = (uint)*(ushort *)(DAT_03070680 + (uVar21 | uVar20) * 0x20 + uVar13 * 2) -

                   ((int)(uVar23 << 0x1e) >> 0x1f) & 0xffff;

        }

        uVar19 = (*(byte *)((int)puVar22 + 1) & 7) >> 1;

        if ((*(byte *)((int)puVar22 + 1) >> 1 & 0xc) == 0xc) {

          iVar14 = DAT_0307069c;

          if ((uVar19 != 3) && (uVar13 = uVar13 | 0x10, iVar14 = DAT_03070688, uVar19 != 2)) {

            iVar14 = DAT_0307068c;

          }

        }

        else {

          iVar14 = DAT_03070684;

          if ((uVar19 != 3) && (iVar14 = DAT_03070688, uVar19 != 2)) {

            iVar14 = DAT_0307068c;

          }

        }

        uVar19 = (uint)*(ushort *)(iVar14 + uVar13 * 2);

        puVar22 = (uint *)((int)puVar22 + uVar20);

        *(uint *)(param_1 + 0x20) = uVar19 * 1000;

        puVar7 = DAT_03070690;

        if (0x7fb < (int)puVar22 - iVar11) {

          *DAT_03070690 = DAT_03070690[0x1ff];

          (*(code *)*puVar6)(puVar7 + 1,0x600,*puVar4);

          puVar22 = (uint *)((int)puVar7 + ((int)puVar22 - iVar11) + -0x7fc);

        }

        puVar7 = DAT_03070690;

        if ((((((char)*puVar22 == -1) && (bVar9 = *(byte *)((int)puVar22 + 1), (~bVar9 & 0xe0) == 0)

              ) && ((bVar9 & 0x18) != 8 && (bVar9 & 6) != 0)) &&

            ((bVar9 = *(byte *)((int)puVar22 + 2), (bVar9 & 0xf0) != 0 && ((~bVar9 & 0xf0) != 0))))

           && (((~bVar9 & 0xc) != 0 &&

               (uVar20 = *puVar22,

               (uVar12 & DAT_030706a0) ==

               ((uVar20 << 0x18 | (uVar20 >> 8 & 0xff) << 0x10 | (uVar20 >> 0x10 & 0xff) << 8 |

                (uint)*(byte *)((int)puVar22 + 3)) & DAT_030706a0))))) {

          if (*(byte *)((int)puVar22 + 3) >> 6 == 3) {

            uVar10 = 1;

          }

          else {

            uVar10 = 2;

          }

          *(undefined1 *)(param_1 + 0x15) = uVar10;

          puVar3 = DAT_0307062c;

          bVar9 = *(byte *)((int)puVar22 + 1) >> 1;

          uVar20 = bVar9 & 0xc;

          if ((bVar9 & 0xc) != 0) {

            uVar20 = uVar20 - 4 & 0xff;

          }

          *(undefined2 *)(param_1 + 0x16) =

               *(undefined2 *)

                (DAT_03070628 + (uVar20 | (*(byte *)((int)puVar22 + 2) & 0xf) >> 2) * 2);

          *(uint *)(param_1 + 0x18) = uVar18;

          iVar11 = (*(code *)*puVar3)(*puVar4);

          puVar3 = DAT_030706a4;

          piVar8 = DAT_03070630;

          uVar18 = 0;

          uVar20 = 0;

          *(int *)(param_1 + 0x1c) = (iVar11 - *(int *)(param_1 + 0x18)) - DAT_03070630[1];

          do {

            puVar22 = DAT_03070690;

            (*(code *)*puVar3)(((uint)((ulonglong)DAT_030706a8 *

                                       (ulonglong)((uVar20 + 5) * *(int *)(param_1 + 0x1c)) >> 0x24)

                               & 0xfffffe00) + *piVar8,0,*puVar4);

            iVar11 = (*(code *)*puVar6)(puVar22,0x800,*puVar4);

            if ((iVar11 < 1) || (iVar11 = FUN_0306fc20(puVar22,uVar12), iVar11 < 0)) {

              return 0xffffffff;

            }

            if (iVar11 < 1) {

              hifi_debug_printf(s_FILE___s__LINE___d__03070668,

                                s_______Common_Codec_Audio_Mp3_lib_03070634,0x350);

            }

            else {

              puVar22[-1] = iVar11 + puVar22[-1];

              uVar18 = uVar18 + 2 & 0xff;

            }

            uVar20 = uVar20 + 1 & 0xff;

          } while (uVar20 < 0x14);

          if (uVar18 != 0) {

            uVar18 = *DAT_030706ac / uVar18;

            *DAT_030706ac = uVar18;

            if (uVar19 == uVar18) {

              *(undefined1 *)(param_1 + 0x14) = 0;

              *(uint *)(param_1 + 0x20) = uVar19 * 1000;

            }

            else {

              *(undefined1 *)(param_1 + 0x14) = 1;

              *(uint *)(param_1 + 0x20) = uVar18 * 1000;

            }

            return 1;

          }

          hifi_debug_printf(s_FILE___s__LINE___d__03070668,

                            s_______Common_Codec_Audio_Mp3_lib_03070634,0x35e);

          return 0;

        }

        (*(code *)*DAT_030706a4)(uVar18 & 0xfffffe00,0,*puVar4);

        (*(code *)*puVar6)(puVar7,0x800,*puVar4);

        puVar22 = (uint *)((int)puVar7 + (uVar18 & 0x1ff));

      }

    }

    puVar7 = DAT_03070690;

    iVar24 = iVar24 + -1;

    if (0x5ff < (int)puVar22 - iVar11) {

      hifi_memmove(DAT_03070690,DAT_03070690 + 0x180,0x200);

      iVar14 = (*(code *)*puVar6)(puVar7 + 0x80,0x600,*puVar4);

      puVar22 = puVar7;

      if (iVar14 < 1) {

        return 0xffffffff;

      }

    }

    uVar18 = uVar18 + 1;

    puVar22 = (uint *)((int)puVar22 + 1);

    if (iVar24 == 0) {

      return 0;

    }

  } while( true );

}
