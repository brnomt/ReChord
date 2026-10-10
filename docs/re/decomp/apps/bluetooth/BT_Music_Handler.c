/**
 * BT_Music_Handler @ 0x0300ddea
 * Tags: bluetooth
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void BT_Music_Handler(void)



{

  char cVar1;

  longlong lVar2;

  ushort *puVar3;

  ushort *puVar4;

  ushort *puVar5;

  char *pcVar6;

  ushort *puVar7;

  int iVar8;

  int iVar9;

  undefined4 uVar10;

  undefined4 uVar11;

  int iVar12;

  int iVar13;

  uint uVar14;

  uint uVar15;

  bool bVar16;

  short local_240 [256];

  short local_40;

  short local_3e;

  short local_3c;

  short local_3a;

  undefined1 auStack_38 [4];

  undefined4 local_34;

  undefined4 local_30;

  uint local_2c;

  uint local_28;

  

  puVar3 = DAT_0300dec0;

  local_28 = (uint)*DAT_0300dec0;

  local_2c = (uint)DAT_0300dec0[1];

  local_34 = FUN_02ff8028(1);

  local_30 = FUN_02ff8004(2);

  local_40 = 0x1c;

  local_3c = 0x81;

  *puVar3 = *DAT_0300dec4;

  puVar3[1] = 0xffff;

  iVar8 = FUN_02ffc8c0(0x1c6);

  if (iVar8 == 0) goto LAB_0300e3c6;

  uVar14 = 0;

  FUN_02ff88d6(0x7a);

  FUN_02ff83ca(1);

  puVar4 = DAT_0300dec8;

  iVar8 = DAT_0300dea0;

  *puVar3 = *DAT_0300dec8;

  if (*(char *)(iVar8 + 0x34a) == '\0') {

    cVar1 = *DAT_0300dea8;

    if (cVar1 == '\0') {

LAB_0300decc:

      FUN_02ff80d4(0,3,0x140,0xc,2,0x12d);

    }

    else if (cVar1 == '\x01') {

      FUN_02ff80d4(0,3,0x140,0xc,2,0x7b);

    }

    else {

      if (cVar1 != '\x02') goto LAB_0300decc;

      FUN_02ff80d4(0,3,0x140,0xc,2,0x7c);

    }

  }

  else {

    FUN_02ff80d4(0,3,0x140,0xc,2,0x12e);

  }

  iVar8 = DAT_0300e2c8;

  *puVar3 = *DAT_0300e2c0;

  if (*(char *)(DAT_0300e2c4 + 0x34a) == '\0') {

    iVar9 = FUN_02ffc904(0x1cf);

    puVar7 = DAT_0300e710;

    iVar13 = DAT_0300e70c;

    pcVar6 = DAT_0300e2e8;

    puVar5 = DAT_0300e2c0;

    if (iVar9 == 0) {

      if (*DAT_0300e2e8 < '\a') {

        do {

          local_40 = 0x1c;

          local_3a = (short)uVar14 * 0x18;

          local_3e = local_3a + 0x1c;

          local_3c = 0x81;

          local_3a = local_3a + 0x2b;

          AmrFunction(DAT_0300e2c8 + uVar14 * 0x26 + 6,local_240,auStack_38);

          uVar15 = uVar14 + 1;

          if ((int)pcVar6[1] == uVar15) {

            *puVar3 = *DAT_0300e2ec;

            if (uVar14 == 0) {

              FUN_02ff8d72(0x86,4,0x17);

            }

            else {

              if (uVar14 == 5) {

                iVar8 = 0x8f;

                uVar10 = 0x88;

              }

              else {

                iVar8 = (int)(short)((short)uVar14 * 0x18 + 0x17);

                uVar10 = 0x87;

              }

              FUN_02ff8d72(uVar10,4,iVar8);

            }

            uVar14 = FUN_02ff7166(local_240);

            iVar8 = DAT_0300e2cc;

            if (uVar14 < (uint)((int)local_3c - (int)local_40)) {

              FUN_02ffc8e6(0xae);

            }

            else {

              uVar14 = 0;

              do {

                if (local_240[uVar14] == 0) break;

                *(short *)(iVar8 + uVar14 * 2) = local_240[uVar14];

                uVar14 = uVar14 + 1 & 0xff;

              } while (uVar14 < 0x80);

              *(undefined2 *)(iVar8 + uVar14 * 2) = 0;

              FUN_0300dd76(&local_40,0x8d,DAT_0300e2cc,0x1e);

              FUN_02ffc8a4(0xae);

            }

          }

          else {

            *puVar3 = *puVar5;

          }

          FUN_02ff74d8(&local_40,&local_40,local_240,0);

        } while (((int)uVar15 < (int)*pcVar6) && (uVar14 = uVar15 & 0xffff, uVar14 < 6));

      }

      else if (DAT_0300e2e8[1] < '\a') {

        do {

          AmrFunction(iVar8 + uVar14 * 0x26 + 6,local_240,auStack_38);

          local_40 = 0x1c;

          local_3a = (short)uVar14 * 0x18;

          local_3e = local_3a + 0x1c;

          local_3c = 0x81;

          local_3a = local_3a + 0x2b;

          if ((int)DAT_0300e2e8[1] == uVar14 + 1) {

            *puVar3 = *DAT_0300e2ec;

            if (uVar14 == 0) {

              FUN_02ff8d72(0x86,4,0x17);

            }

            else {

              if (uVar14 == 5) {

                uVar10 = 0x88;

                iVar13 = 0x8f;

              }

              else {

                iVar13 = (int)(short)((short)uVar14 * 0x18 + 0x17);

                uVar10 = 0x87;

              }

              FUN_02ff8d72(uVar10,4,iVar13);

            }

            uVar15 = FUN_02ff7166(local_240);

            iVar13 = DAT_0300e2cc;

            if (uVar15 < (uint)((int)local_3c - (int)local_40)) {

              FUN_02ffc8e6(0xae);

            }

            else {

              uVar15 = 0;

              do {

                if (local_240[uVar15] == 0) break;

                *(short *)(iVar13 + uVar15 * 2) = local_240[uVar15];

                uVar15 = uVar15 + 1 & 0xff;

              } while (uVar15 < 0x80);

              *(undefined2 *)(iVar13 + uVar15 * 2) = 0;

              FUN_0300dd76(&local_40,0x8d,DAT_0300e2cc,0x1e);

              FUN_02ffc8a4(0xae);

            }

          }

          else {

            *puVar3 = *puVar5;

          }

          FUN_02ff74d8(&local_40,&local_40,local_240,0);

          uVar14 = uVar14 + 1 & 0xffff;

        } while (uVar14 < 6);

      }

      else {

        for (; iVar8 = (int)((ulonglong)((longlong)iVar13 * (longlong)(int)*pcVar6) >> 0x20),

            (int)uVar14 < (int)*pcVar6 + (iVar8 - (iVar8 >> 0x1f)) * -6;

            uVar14 = uVar14 + 1 & 0xffff) {

          iVar8 = (pcVar6[1] + -1) * 0xab;

          AmrFunction(DAT_0300e714 + (uVar14 + ((iVar8 >> 10) - (iVar8 >> 0x1f)) * 6) * 0x26 + 6,

                      local_240,auStack_38);

          local_40 = 0x1c;

          local_3a = (short)uVar14 * 0x18;

          local_3e = local_3a + 0x1c;

          local_3c = 0x81;

          local_3a = local_3a + 0x2b;

          lVar2 = (longlong)iVar13 * (longlong)(int)pcVar6[1];

          uVar10 = (undefined4)lVar2;

          iVar8 = (int)((ulonglong)lVar2 >> 0x20);

          if (((int)pcVar6[1] + (iVar8 - (iVar8 >> 0x1f)) * -6) - uVar14 == 1) {

            *puVar3 = *DAT_0300e718;

            if (uVar14 == 0) {

              FUN_02ff8d72(0x86,4,0x17,uVar10);

            }

            else {

              if (uVar14 == 5) {

                iVar8 = 0x8f;

                uVar11 = 0x88;

              }

              else {

                iVar8 = (int)(short)((short)uVar14 * 0x18 + 0x17);

                uVar11 = 0x87;

              }

              FUN_02ff8d72(uVar11,4,iVar8,uVar10);

            }

            uVar15 = FUN_02ff7166(local_240);

            iVar8 = DAT_0300e71c;

            if (uVar15 < (uint)((int)local_3c - (int)local_40)) {

              FUN_02ffc8e6(0xae);

            }

            else {

              uVar15 = 0;

              do {

                if (local_240[uVar15] == 0) break;

                *(short *)(iVar8 + uVar15 * 2) = local_240[uVar15];

                uVar15 = uVar15 + 1 & 0xff;

              } while (uVar15 < 0x80);

              *(undefined2 *)(iVar8 + uVar15 * 2) = 0;

              FUN_0300dd76(&local_40,0x8d,DAT_0300e71c,0x1e);

              FUN_02ffc8a4(0xae);

            }

          }

          else {

            *puVar3 = *puVar7;

          }

          FUN_02ff74d8(&local_40,&local_40,local_240,0);

        }

      }

    }

    else {

      FUN_02ff8d72(0x86,4,0x17);

      iVar13 = DAT_0300e2c4;

      *puVar3 = *puVar4;

      cVar1 = *(char *)(iVar13 + 0x328);

      bVar16 = cVar1 == '\0';

      if (bVar16) {

        cVar1 = *(char *)(iVar13 + 0x329);

      }

      if (bVar16 && cVar1 == '\0') {

        FUN_02fe971e(local_240,u_BT_Music_0300e2d0,0x50);

        *(undefined1 *)(iVar13 + 0x23e) = 0;

      }

      else {

        AmrFunction(iVar13 + 0x328,local_240,auStack_38);

      }

      local_40 = 0x1c;

      local_3c = 0x81;

      local_3e = 0x1c;

      local_3a = 0x2b;

      uVar14 = FUN_02ff7166(local_240);

      iVar9 = DAT_0300e2cc;

      if (uVar14 < (uint)((int)local_3c - (int)local_40)) {

        FUN_02ffc8e6(0xae);

      }

      else {

        uVar14 = 0;

        do {

          if (local_240[uVar14] == 0) break;

          *(short *)(iVar9 + uVar14 * 2) = local_240[uVar14];

          uVar14 = uVar14 + 1 & 0xff;

        } while (uVar14 < 0x80);

        *(undefined2 *)(iVar9 + uVar14 * 2) = 0;

        FUN_0300dd76(&local_40,0x8d,DAT_0300e2cc,0x1e);

        FUN_02ffc8a4(0xae);

      }

      FUN_02ff74d8(&local_40,&local_40,local_240,0);

      pcVar6 = DAT_0300e2e8;

      DAT_0300e2e8[1] = '\x01';

      if ('\x01' < *pcVar6) {

        *pcVar6 = '\x01';

        uVar14 = 0;

        do {

          iVar9 = iVar13 + uVar14;

          iVar12 = iVar8 + uVar14;

          uVar14 = uVar14 + 1 & 0xffff;

          *(undefined1 *)(iVar12 + 6) = *(undefined1 *)(iVar9 + 0x328);

        } while (uVar14 < 0x20);

        uVar14 = 0;

        do {

          *(undefined1 *)(iVar8 + uVar14) = *(undefined1 *)(iVar13 + uVar14 + 0x23e);

          uVar14 = uVar14 + 1 & 0xffff;

        } while (uVar14 < 6);

      }

    }

  }

  else {

    FUN_02ff8d72(0x86,4,0x17);

    iVar13 = DAT_0300e2c4;

    *puVar3 = *puVar4;

    cVar1 = *(char *)(iVar13 + 0x328);

    bVar16 = cVar1 == '\0';

    if (bVar16) {

      cVar1 = *(char *)(iVar13 + 0x329);

    }

    if (bVar16 && cVar1 == '\0') {

      FUN_02fe971e(local_240,u_BT_Music_0300e2d0,0x50);

      *(undefined1 *)(iVar13 + 0x23e) = 0;

    }

    else {

      AmrFunction(iVar13 + 0x328,local_240,auStack_38);

    }

    local_40 = 0x1c;

    local_3c = 0x81;

    local_3e = 0x1c;

    local_3a = 0x2b;

    uVar14 = FUN_02ff7166(local_240);

    iVar9 = DAT_0300e2cc;

    if (uVar14 < (uint)((int)local_3c - (int)local_40)) {

      FUN_02ffc8e6(0xae);

    }

    else {

      uVar14 = 0;

      do {

        if (local_240[uVar14] == 0) break;

        *(short *)(iVar9 + uVar14 * 2) = local_240[uVar14];

        uVar14 = uVar14 + 1 & 0xff;

      } while (uVar14 < 0x80);

      *(undefined2 *)(iVar9 + uVar14 * 2) = 0;

      FUN_0300dd76(&local_40,0x8d,DAT_0300e2cc,0x1e);

      FUN_02ffc8a4(0xae);

    }

    FUN_02ff74d8(&local_40,&local_40,local_240,0);

    pcVar6 = DAT_0300e2e8;

    DAT_0300e2e8[1] = '\x01';

    if ('\x01' < *pcVar6) {

      *pcVar6 = '\x01';

      uVar14 = 0;

      do {

        *(undefined1 *)(iVar8 + uVar14 + 6) = *(undefined1 *)(iVar13 + uVar14 + 0x328);

        uVar14 = uVar14 + 1 & 0xffff;

      } while (uVar14 < 0x20);

      uVar14 = 0;

      do {

        *(undefined1 *)(iVar8 + uVar14) = *(undefined1 *)(iVar13 + uVar14 + 0x23e);

        uVar14 = uVar14 + 1 & 0xffff;

      } while (uVar14 < 6);

    }

  }

  FUN_02ffc8a4(0x21);

LAB_0300e3c6:

  iVar8 = FUN_02ffc904(0xae);

  if (iVar8 == 1) {

    *puVar3 = *DAT_0300e710;

    FUN_02ff7c40(DAT_0300e71c);

    *puVar3 = 0xffff;

  }

  FUN_02ff83ca(0);

  FUN_02ff8028(local_34);

  puVar3[1] = (ushort)local_2c;

  *puVar3 = (ushort)local_28;

  FUN_02ff8004(local_30);

  return;

}
