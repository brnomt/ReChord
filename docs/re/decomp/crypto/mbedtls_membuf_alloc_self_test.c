/**
 * mbedtls_membuf_alloc_self_test @ 0x030087d6
 * Structural-matcher sweep (post-boundary-repair, module-validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 mbedtls_membuf_alloc_self_test(void)



{

  ushort *puVar1;

  undefined4 *puVar2;

  ushort uVar3;

  int iVar4;

  int iVar5;

  uint uVar6;

  int iVar7;

  int iVar8;

  undefined4 uVar9;

  uint extraout_r1;

  undefined4 extraout_r1_00;

  int __c;

  size_t extraout_r2;

  size_t extraout_r2_00;

  size_t extraout_r2_01;

  size_t extraout_r2_02;

  size_t extraout_r2_03;

  size_t extraout_r2_04;

  size_t sVar10;

  size_t extraout_r2_05;

  size_t extraout_r2_06;

  size_t extraout_r2_07;

  size_t extraout_r2_08;

  size_t __n;

  int iVar11;

  bool bVar12;

  longlong lVar13;

  undefined8 uVar14;

  undefined2 auStack_48 [18];

  

  puVar1 = DAT_0300897c;

  iVar11 = 0xff;

  *DAT_03008990 = *DAT_0300897c;

  lVar13 = FUN_02ff0ea6();

  iVar8 = DAT_03008998;

  iVar7 = DAT_03008994;

  iVar4 = (int)lVar13;

  sVar10 = extraout_r2;

  if (iVar4 == DAT_03008994 || iVar4 == DAT_03008998) {

    iVar5 = UartDev_Write(1);

    if (iVar5 == 0) {

      uVar14 = FUN_02ff0b76(0,0x7d,0xa0,0xaa,0);

      iVar5 = DAT_0300899c;

      uVar9 = (undefined4)((ulonglong)uVar14 >> 0x20);

      if ((int)uVar14 == 0) {

        uVar14 = FUN_02ff0b76(0xa0,0x7d,0x140,0xaa,0);

        uVar9 = (undefined4)((ulonglong)uVar14 >> 0x20);

        if ((int)uVar14 == 0) {

          iVar7 = FUN_02ff0b76(0,0x1b,0x140,0x7d,0);

          if (iVar7 == 0) {

            uVar14 = FUN_02ff0b76(0,0,0x3c,0x32,0);

            uVar6 = (uint)((ulonglong)uVar14 >> 0x20);

            sVar10 = extraout_r2_05;

            lVar13 = CONCAT44(uVar6,DAT_03008bf0);

            if ((int)uVar14 == 0) {

              lVar13 = (ulonglong)uVar6 << 0x20;

            }

          }

          else {

            iVar7 = FUN_02ff0b76(0,0x1b,0x140,0x33,0);

            if (iVar7 != 0) {

              iVar11 = 0;

            }

            iVar7 = FUN_02ff0b76(0,0x33,0x140,0x4b,0);

            if (iVar7 != 0) {

              iVar11 = 1;

            }

            iVar7 = FUN_02ff0b76(0,0x4b,0x140,99,0);

            if (iVar7 != 0) {

              iVar11 = 2;

            }

            iVar7 = FUN_02ff0b76(0,99,0x140,0x7b,0);

            if (iVar7 != 0) {

              iVar11 = 3;

            }

            uVar14 = FUN_02ff0b76(0,0x7b,0x140,0x93,0);

            bVar12 = (int)uVar14 != 0;

            if (bVar12) {

              iVar11 = 4;

            }

            sVar10 = extraout_r2_03;

            lVar13 = CONCAT44((int)((ulonglong)uVar14 >> 0x20),iVar4);

            if (bVar12 || iVar11 != 0xff) {

              uVar6 = iVar11 + (*puVar1 / 6) * 6;

              *puVar1 = (ushort)uVar6;

              lVar13 = CONCAT44(0xaaab,iVar4);

              if ((uVar6 & 0xffff) < 0x1e) {

                mbedtls_mpi_grow(0x98);

                mbedtls_mpi_grow(0xce);

                FUN_0300862e();

                FUN_02ff4124();

                FUN_02ff06bc();

                sVar10 = extraout_r2_04;

                lVar13 = CONCAT44(extraout_r1_00,DAT_030089b8);

              }

            }

          }

        }

        else {

          sVar10 = extraout_r2_02;

          lVar13 = CONCAT44(uVar9,DAT_030089ac);

          if (((iVar4 != iVar7) && (lVar13 = CONCAT44(uVar9,DAT_030089b0), iVar4 != iVar8)) &&

             (lVar13 = CONCAT44(uVar9,iVar4), iVar4 == iVar5)) {

            lVar13 = CONCAT44(uVar9,DAT_030089b4);

          }

        }

      }

      else {

        sVar10 = extraout_r2_01;

        lVar13 = CONCAT44(uVar9,DAT_030089a0);

        if (((iVar4 != iVar7) && (lVar13 = CONCAT44(uVar9,DAT_030089a4), iVar4 != iVar8)) &&

           (lVar13 = CONCAT44(uVar9,iVar4), iVar4 == DAT_0300899c)) {

          lVar13 = CONCAT44(uVar9,DAT_030089a8);

        }

      }

    }

    else {

      mbedtls_mpi_grow(0x2f);

      sVar10 = extraout_r2_00;

      lVar13 = (ulonglong)extraout_r1 << 0x20;

    }

  }

  iVar8 = (int)lVar13;

  memset((void *)0x1,(int)((ulonglong)lVar13 >> 0x20),sVar10);

  iVar11 = DAT_030089c4;

  puVar2 = DAT_030089c0;

  iVar7 = iVar8 - DAT_030089b8;

  if (iVar8 == DAT_030089b8) {

    uVar14 = FUN_02ff7414(1);

    iVar11 = (int)((ulonglong)uVar14 >> 0x20);

    sVar10 = extraout_r2_08;

    if ((int)uVar14 == 0) {

      if (*DAT_03008bfc != 2) {

        MBEDTLS_MPI_CHK(DAT_03008bf8);

        FUN_02ff0b46();

        memset((void *)0x9c,__c,__n);

        mbedtls_mpi_grow(0x9d);

        return 1;

      }

      iVar11 = *DAT_03008c00;

      iVar7 = iVar11 + (uint)*puVar1 * 2;

      if (*(short *)(iVar7 + 0x14) == 0) {

        return 0;

      }

      *(ushort *)(iVar11 + 0x54) = *puVar1;

      *(uint *)(iVar11 + 0xc) = (uint)*(ushort *)(iVar7 + 0x14);

      pbuf_free(0xac,1);

      return 0;

    }

    goto LAB_03008a56;

  }

  if (DAT_030089b8 <= iVar8) {

    if (iVar7 != 0x3fc0) {

      if (iVar7 < 0x3fc1) {

        if (iVar7 != 0x1c0) {

          if (iVar7 == 0x3c0) goto LAB_03008ba0;

          if (iVar7 == 0x7c0) goto LAB_03008b7a;

          if (iVar7 != 0xfc0) {

            return 0;

          }

        }

      }

      else {

        if (iVar7 == 0x7fc0) {

LAB_03008ba0:

          uVar6 = (*puVar1 / 5) * 5;

          uVar3 = (ushort)uVar6;

          *puVar1 = uVar3;

          if ((uVar6 & 0xffff) < 0x18) {

            uVar3 = uVar3 + 5;

          }

          else {

            uVar3 = 0x19;

          }

          *puVar1 = uVar3;

          mbedtls_mpi_grow(0x98);

          return 0;

        }

        if (iVar7 == 0xffffc0) {

          auStack_48[0] = 0;

          FUN_02fe62b0(DAT_03008bf8,DAT_03008c0c,auStack_48);

          return 0;

        }

        if (iVar7 == 0x1ffffc0) {

          auStack_48[0] = 1;

          FUN_02fe62b0(DAT_03008bf8,DAT_03008c0c,auStack_48);

          return 0;

        }

        if (iVar7 != 0x100001c0) {

          return 0;

        }

      }

      uVar14 = FUN_02ff7414(1);

      iVar11 = (int)((ulonglong)uVar14 >> 0x20);

      sVar10 = extraout_r2_06;

      if ((int)uVar14 == 0) {

        RKDeviceUnRegister();

        MBEDTLS_MPI_CHK(DAT_03008bf8);

        FUN_02ff0b46();

        mbedtls_mpi_grow(0xa1);

        return 1;

      }

LAB_03008a56:

      memset((void *)0x1,iVar11,sVar10);

      return 0;

    }

LAB_03008b7a:

    uVar6 = (*puVar1 / 5) * 5;

    uVar3 = (ushort)uVar6;

    *puVar1 = uVar3;

    if ((uVar6 & 0xffff) < 5) {

      *puVar1 = 0;

    }

    else {

      *puVar1 = uVar3 - 5;

    }

    uVar9 = 0x98;

    goto LAB_03008b9a;

  }

  iVar7 = iVar8 - DAT_030089bc;

  if (iVar8 == DAT_030089bc) {

LAB_03008b3a:

    iVar7 = pbuf_alloc(*DAT_030089c0,DAT_03008c04);

    if ((iVar7 == 1) && (*(char *)(iVar11 + 0x12) != '\0')) {

      *(char *)(iVar11 + 0x12) = *(char *)(iVar11 + 0x12) + -1;

      phybusif_output(0xff,0);

    }

    iVar7 = pbuf_alloc(*puVar2,DAT_03008c08);

    if ((iVar7 == 1) && (*(char *)(iVar11 + 0x12) != '\0')) {

      *(char *)(iVar11 + 0x12) = *(char *)(iVar11 + 0x12) + -1;

      pbuf_free(0xb6,0);

    }

  }

  else {

    if (DAT_030089bc <= iVar8) {

      if (iVar7 == 0x3e0) goto LAB_03008b7a;

      if (iVar7 == 0x7e0) goto LAB_03008ba0;

      if (iVar7 + DAT_03008bf4 == 0) goto LAB_03008aec;

      if (iVar7 + DAT_03008bf4 != 0x10) {

        return 0;

      }

      goto LAB_03008b3a;

    }

    iVar8 = iVar8 + -0x20000400;

    if (iVar8 == 0) goto LAB_03008ba0;

    if (iVar8 == 0x400) goto LAB_03008b7a;

    if (iVar8 == 0xc00) {

      uVar14 = FUN_02ff7414(1);

      iVar11 = (int)((ulonglong)uVar14 >> 0x20);

      sVar10 = extraout_r2_07;

      if ((int)uVar14 == 0) {

        RKDeviceUnRegister();

        MBEDTLS_MPI_CHK(DAT_03008bf8);

        FUN_02ff0b46();

        mbedtls_mpi_grow(0x93);

        return 1;

      }

      goto LAB_03008a56;

    }

    if (iVar8 + DAT_03008bec != 0) {

      return 0;

    }

LAB_03008aec:

    iVar7 = pbuf_alloc(*DAT_030089c0,DAT_03008c04);

    if ((iVar7 == 1) && (*(byte *)(iVar11 + 0x12) < *(byte *)(iVar11 + 0x1e))) {

      *(byte *)(iVar11 + 0x12) = *(byte *)(iVar11 + 0x12) + 1;

      phybusif_output(0xff,0);

    }

    iVar7 = pbuf_alloc(*puVar2,DAT_03008c08);

    if ((iVar7 == 1) && (*(byte *)(iVar11 + 0x12) < *(byte *)(iVar11 + 0x1e))) {

      *(byte *)(iVar11 + 0x12) = *(byte *)(iVar11 + 0x12) + 1;

      pbuf_free(0xb6,0);

    }

  }

  uVar9 = 0x1a9;

LAB_03008b9a:

  mbedtls_mpi_grow(uVar9);

  return 0;

}
