/**
 * power_state_reset @ 0x0301861a
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void power_state_reset(void)



{

  byte bVar1;

  undefined2 uVar2;

  undefined2 uVar3;

  byte *pbVar4;

  undefined2 *puVar5;

  undefined2 *puVar6;

  char cVar7;

  undefined4 uVar8;

  undefined4 uVar9;

  int iVar10;

  undefined4 uVar11;

  int iVar12;

  uint uVar13;

  ushort uVar14;

  char cVar15;

  uint uVar16;

  

  puVar5 = DAT_03018964;

  pbVar4 = DAT_03018960;

  uVar14 = 0;

  if (*DAT_03018960 == 0) {

    FUN_030183d6();

    return;

  }

  uVar2 = *DAT_03018964;

  uVar3 = DAT_03018964[1];

  uVar8 = FUN_02ffef2c(1);

  uVar9 = FUN_02ffef08(2);

  puVar6 = DAT_03018968;

  *puVar5 = *DAT_03018968;

  puVar5[1] = *DAT_0301896c;

  iVar10 = func_0x030037c4(0xbd);

  if ((iVar10 != 0) || (iVar10 = ui_refresh_and_redraw(0x26), iVar10 != 0)) {

    if (*pbVar4 == 6) {

      func_0x030037a8(0x1c3);

    }

    if (*pbVar4 < 3) {

      FUN_02fff7da(0x42);

    }

    else if (*pbVar4 == 4) {

      FUN_03001576(1);

    }

    bVar1 = *pbVar4;

    if (bVar1 == 3) {

      uVar11 = 0x1c3;

LAB_030186e0:

      func_0x030037a8(uVar11);

    }

    else if (bVar1 == 0) {

      FUN_030183d6();

    }

    else if ((bVar1 != 1) && (bVar1 == 2)) {

      *puVar5 = *DAT_03018970;

      FUN_02ffefd8(0,3,0x140,0xc,0,0x179);

      *puVar5 = *puVar6;

      func_0x030037a8(0x1c3);

      mbedtls_asn1_get_tag(1);

      uVar11 = 0x21;

      goto LAB_030186e0;

    }

  }

  iVar12 = func_0x030037c4(0x1c3);

  iVar10 = DAT_03018978;

  uVar13 = DAT_03018974;

  if (iVar12 == 1) {

    bVar1 = *pbVar4;

    if (bVar1 == 3) {

      FUN_02fff7da(0xd2);

      if (*(char *)(iVar10 + 0x35a) == '\x01') {

        FUN_02fffc76(0xda,0x6e,0x81);

      }

      FUN_02fff812(0x4f,0,0,0,0x14);

      if (*(char *)(iVar10 + 0x1e) != '\0') {

        FUN_02fff812(0xd2,0,0,0x2a,10);

        if (*(byte *)(iVar10 + 0x1e) >> 1 != 0) {

          do {

            if (*(char *)(iVar10 + 0x35a) == '\0') {

              iVar12 = (int)(short)(uVar14 * -4 + 0x115);

              if ((uVar14 & 1) == 0) {

                uVar11 = 0xd9;

              }

              else {

                uVar11 = 0xd8;

              }

            }

            else {

              iVar12 = (int)(short)(uVar14 * 4 + 0x29);

              if ((uVar14 & 1) == 0) {

                uVar11 = 0xd8;

              }

              else {

                uVar11 = 0xd9;

              }

            }

            FUN_02fffc76(uVar11,iVar12);

            uVar14 = uVar14 + 1;

          } while (uVar14 < *(byte *)(iVar10 + 0x1e) >> 1);

        }

        iVar12 = (uint)*(byte *)(iVar10 + 0x1e) +

                 (uint)((ulonglong)uVar13 * (ulonglong)(uint)*(byte *)(iVar10 + 0x1e) >> 0x22) * -5;

        if (*(char *)(iVar10 + 0x35a) == '\0') {

          FUN_02fffc76(iVar12 + 0xd3U & 0xffff,0,0x38);

        }

        else {

          FUN_02fffc76(0xd7U - iVar12 & 0xffff,0,0x38);

        }

      }

      *puVar5 = *DAT_03018970;

      FUN_02ffefd8(0,3,0x140,0x10,2,0xec);

      FUN_02fffc76(((uint)*(byte *)(iVar10 + 0x1e) * 0x29 >> 0xc) + 0x162,0x8c,0x4c);

      uVar16 = *(byte *)(iVar10 + 0x1e) / 10;

      FUN_02fffc76(uVar16 + (uint)((ulonglong)uVar13 * (ulonglong)uVar16 >> 0x23) * -10 + 0x162 &

                   0xffff,0x9a,0x4c);

      FUN_02fffc76((uint)*(byte *)(iVar10 + 0x1e) +

                   (uint)((ulonglong)uVar13 * (ulonglong)(uint)*(byte *)(iVar10 + 0x1e) >> 0x23) *

                   -10 + 0x162 & 0xffff,0xa8,0x4c);

      mbedtls_asn1_get_tag(1);

      goto LAB_03018a4c;

    }

    if (bVar1 != 2) {

      if (bVar1 == 6) {

        bVar1 = *(byte *)(DAT_03018978 + 0x35d);

        cVar7 = '\0';

        cVar15 = '\0';

        if (bVar1 < 0xd) {

          cVar7 = '\f' - bVar1;

        }

        else {

          cVar15 = bVar1 - 0xc;

        }

        FUN_02fffc76(0x158,0x89,0x17);

        FUN_02ffefd8(0x89,0x2a,0xb4,0x10,2,0x112);

        if (cVar15 != '\0') {

          FUN_02fffc76(0x157,0xd7,0x45);

        }

        if (cVar7 != '\0') {

          FUN_02fffc76(0x156,0xd7,0x45);

        }

        uVar16 = (uint)(byte)(cVar15 + cVar7);

        if (uVar16 == 0) {

          FUN_02fffc76(0x14c,0xdc,0x45);

          uVar13 = 0x14c;

          uVar11 = 0xe3;

        }

        else {

          FUN_02fffc76(uVar16 / 10 + 0x14c,0xde,0x45);

          uVar13 = uVar16 + (uint)((ulonglong)uVar13 * (ulonglong)uVar16 >> 0x23) * -10 + 0x14c &

                   0xffff;

          uVar11 = 0xe5;

        }

        FUN_02fffc76(uVar13,uVar11,0x45);

        uVar13 = 0;

        if (uVar16 != 0) {

          do {

            if (cVar15 == '\0') {

              if (cVar7 != '\0') {

                iVar10 = uVar13 * -5;

                FUN_02fffc76(0x15b,iVar10 + 0xe2,0x5a);

                FUN_02fffc76(0x15b,iVar10 + 0xe1,0x5a);

                FUN_02fffc76(0x15b,iVar10 + 0xe0,0x5a);

                FUN_02fffc76(0x15b,iVar10 + 0xdf,0x5a);

                FUN_02fffc76(0x15b,iVar10 + 0xde,0x5a);

              }

            }

            else {

              iVar10 = uVar13 * 5;

              FUN_02fffc76(0x15b,iVar10 + 0xe2,0x5a);

              FUN_02fffc76(0x15b,iVar10 + 0xe3,0x5a);

              FUN_02fffc76(0x15b,iVar10 + 0xe4,0x5a);

              FUN_02fffc76(0x15b,iVar10 + 0xe5,0x5a);

              FUN_02fffc76(0x15b,iVar10 + 0xe6,0x5a);

            }

            uVar13 = uVar13 + 1 & 0xff;

          } while (uVar13 < uVar16);

        }

        if (cVar15 == '\0') {

          if (cVar7 == '\0') {

            FUN_02fffc76(0x15a,0xdf,0x56);

          }

          else {

            FUN_02fffc76(0x15a,uVar13 * -5 + 0xdf,0x56);

          }

        }

        else {

          FUN_02fffc76(0x15a,uVar13 * 5 + 0xdf,0x56);

        }

      }

      goto LAB_03018a4c;

    }

  }

  else {

LAB_03018a4c:

    if (*pbVar4 == 4) {

      FUN_03001576(0);

      goto LAB_03018a60;

    }

  }

  mbedtls_asn1_get_tag(0);

LAB_03018a60:

  FUN_02ffef2c(uVar8);

  puVar5[1] = uVar3;

  *puVar5 = uVar2;

  FUN_02ffef08(uVar9);

  return;

}
