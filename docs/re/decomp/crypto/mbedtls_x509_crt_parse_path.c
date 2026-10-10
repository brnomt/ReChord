/**
 * mbedtls_x509_crt_parse_path @ 0x0300d184
 * Tags: crypto, tls
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined4 FUN_0300d184(void)



{

  uint *puVar1;

  int *piVar2;

  undefined1 *puVar3;

  undefined4 *puVar4;

  uint *puVar5;

  char *pcVar6;

  undefined1 uVar7;

  undefined2 uVar8;

  ushort uVar9;

  int iVar10;

  int iVar11;

  int iVar12;

  uint uVar13;

  undefined4 uVar14;

  int iVar15;

  uint uVar16;

  int iVar17;

  undefined1 local_48 [2];

  byte bStack_46;

  undefined1 local_45;

  

  iVar10 = FUN_02ff4186();

  iVar12 = DAT_0300d358;

  iVar15 = DAT_0300d354;

  iVar11 = 0;

  if (iVar10 != 0x40000080) {

    iVar11 = iVar10 + -0x50000000;

  }

  iVar17 = iVar10;

  if ((iVar10 == 0x40000080 || iVar11 == 0x80) || iVar10 == DAT_0300d354) {

    iVar11 = FUN_02ff3e56(0,0,0x3c,0x3c,0);

    if (iVar11 == 0) {

      iVar11 = FUN_02ff3e56(0x3c,0,0x6e,0x3c,0);

      iVar17 = DAT_0300d360;

      if ((iVar11 == 0) && (iVar11 = FUN_02ff3e56(0xbc,0,0xf0,0x3c,0), iVar17 = iVar10, iVar11 != 0)

         ) {

        iVar17 = iVar12;

      }

    }

    else if (iVar10 != iVar15) {

      iVar17 = DAT_0300d35c;

    }

  }

  FUN_02ffa6d6(0x1bb);

  puVar5 = DAT_0300d78c;

  puVar4 = DAT_0300d374;

  iVar11 = DAT_0300d370;

  puVar3 = DAT_0300d36c;

  piVar2 = DAT_0300d34c;

  puVar1 = DAT_0300d340;

  uVar8 = stack0xffffffb9;

  iVar15 = iVar17 - DAT_0300d364;

  if (iVar17 == DAT_0300d364) {

LAB_0300d406:

    FUN_0300b8d4(0);

    _local_48 = CONCAT12(bStack_46,5);

    FUN_02ffa8be(10,local_48);

    return 1;

  }

  if (iVar17 < DAT_0300d364) {

    iVar15 = iVar15 + 0xfe;

    if (iVar17 == iVar12) goto LAB_0300d406;

    if (iVar17 < iVar12) {

      if (iVar17 == DAT_0300d378) {

LAB_0300d2e0:

        iVar15 = FUN_02fe9720(*DAT_0300d374,DAT_0300d38c);

        if ((iVar15 == 1) && (*(byte *)(iVar11 + 0x12) < *(byte *)(iVar11 + 0x1e))) {

          *(byte *)(iVar11 + 0x12) = *(byte *)(iVar11 + 0x12) + 1;

          FUN_03017adc(0xff,0);

        }

        iVar15 = FUN_02fe9720(*puVar4,DAT_0300d390);

        if (iVar15 != 1) {

          return 0;

        }

        if (*(byte *)(iVar11 + 0x1e) <= *(byte *)(iVar11 + 0x12)) {

          return 0;

        }

        *(byte *)(iVar11 + 0x12) = *(byte *)(iVar11 + 0x12) + 1;

        FUN_03024952(0xb6,0);

        return 0;

      }

      if (DAT_0300d378 <= iVar17) {

        if (iVar17 - DAT_0300d378 == 0x10) {

LAB_0300d396:

          iVar15 = FUN_02fe9720(*DAT_0300d374,DAT_0300d784);

          if ((iVar15 == 1) && (*(char *)(iVar11 + 0x12) != '\0')) {

            *(char *)(iVar11 + 0x12) = *(char *)(iVar11 + 0x12) + -1;

            FUN_03017adc(0xff,0);

          }

          iVar15 = FUN_02fe9720(*puVar4,DAT_0300d788);

          if (iVar15 != 1) {

            return 0;

          }

          if (*(char *)(iVar11 + 0x12) == '\0') {

            return 0;

          }

          *(char *)(iVar11 + 0x12) = *(char *)(iVar11 + 0x12) + -1;

          FUN_03024952(0xb6,0);

          return 0;

        }

        if ((iVar17 - DAT_0300d378) + DAT_0300d380 != 0) {

          return 0;

        }

LAB_0300d3de:

        if (DAT_0300d340[2] == 0) {

          DAT_0300d340[2] = 1;

          puVar1[4] = *puVar5;

          FUN_02ff7942();

          FUN_02ff8906();

        }

        else {

          DAT_0300d340[2] = 0;

          FUN_02ff7930();

          FUN_02ff8364();

        }

        uVar14 = 0x13e;

        goto LAB_0300d50e;

      }

      if (iVar17 + -0x20001000 == 0) goto LAB_0300d406;

      iVar11 = iVar17 + -0x20001000 + DAT_0300d37c;

      if (iVar11 == 0) goto LAB_0300d474;

      if (iVar11 != 4) {

        return 0;

      }

    }

    else {

      if (iVar15 == 0xe) goto LAB_0300d2e0;

      if (0xe < iVar15) {

        if (iVar15 == 0x1e) goto LAB_0300d396;

        if (iVar15 != 0x3e) {

          return 0;

        }

        goto LAB_0300d3de;

      }

      if (iVar15 == 2) goto LAB_0300d474;

      if (iVar15 != 6) {

        return 0;

      }

    }

  }

  else {

    iVar12 = iVar15 - DAT_0300d384;

    if (iVar15 == DAT_0300d384) {

      _local_48 = CONCAT12(bStack_46,1);

      FUN_02fe9590(DAT_0300d7a0,DAT_0300d79c,local_48);

      return 0;

    }

    if (iVar15 < DAT_0300d384) {

      if (iVar15 == 0xf00) {

LAB_0300d43a:

        FUN_0300b8d4(0);

        iVar15 = FUN_02ffa6f4(0x157);

        if (iVar15 == 0) {

          uVar7 = 3;

          uVar8 = *(undefined2 *)(iVar11 + 0x21d);

        }

        else {

          uVar7 = 6;

          uVar8 = *DAT_0300d790;

        }

        _local_48 = CONCAT21(uVar8,uVar7);

        local_45 = 0;

        FUN_02ffa8be(9,local_48);

        return 1;

      }

      if (0xf00 < iVar15) {

        if (iVar15 != 0xff00) {

          if (iVar15 != 0xffff00) {

            return 0;

          }

          _local_48 = (uint3)bStack_46 << 0x10;

          FUN_02fe9590(DAT_0300d7a0,DAT_0300d79c,local_48);

          return 0;

        }

LAB_0300d420:

        unique0x100002c2 = uVar8;

        FUN_0300b8d4(0);

        _local_48 = CONCAT21(stack0xffffffb9,0xe);

        FUN_02ffa8be(0,local_48);

        return 1;

      }

      if (iVar15 == 0x100) goto LAB_0300d43a;

      if (iVar15 == 0x300) goto LAB_0300d474;

      if (iVar15 != 0x700) {

        return 0;

      }

    }

    else {

      iVar15 = iVar12 - DAT_0300d388;

      if (iVar12 == DAT_0300d388) {

LAB_0300d474:

        *DAT_0300d368 = 0;

        if ((uint)piVar2[1] <= puVar1[1]) {

          return 0;

        }

        *puVar3 = 0;

        uVar9 = 0;

        puVar1[8] = *puVar1;

        do {

          FUN_0300c65a();

          uVar9 = uVar9 + 1;

        } while (uVar9 < 0xc);

        puVar1[7] = puVar1[7] + 1;

        FUN_02ffa694(0x13d);

        FUN_02ffa694(0x13b);

        return 0;

      }

      if (iVar12 < DAT_0300d388) {

        if (iVar15 == -0x300) goto LAB_0300d406;

        if (iVar15 != -0x200) {

          return 0;

        }

        goto LAB_0300d43a;

      }

      if (iVar15 != 0x400) {

        if (iVar15 != 0xfc00) {

          return 0;

        }

        goto LAB_0300d420;

      }

    }

  }

  *DAT_0300d368 = 0;

  pcVar6 = DAT_0300d798;

  if (*piVar2 == 1) {

    uVar13 = 2;

  }

  else if (*piVar2 == 3) {

    uVar13 = (uint)*DAT_0300d794;

  }

  else {

    uVar13 = 0;

  }

  if (puVar1[8] < uVar13) {

    return 0;

  }

  uVar16 = *puVar1;

  if (uVar16 == uVar13) {

    return 0;

  }

  *puVar3 = 0;

  uVar13 = FUN_0300d090(uVar16,(int)*pcVar6);

  puVar1[8] = uVar13;

  *puVar1 = uVar13;

  uVar9 = 0;

  puVar1[1] = uVar13;

  do {

    *(undefined2 *)piVar2[3] = 0;

    FUN_0300c65a();

    uVar9 = uVar9 + 1;

  } while (uVar9 < 0xc);

  puVar1[7] = puVar1[7] - 1;

  FUN_02ffa694(0x13d);

  uVar14 = 0x13b;

LAB_0300d50e:

  FUN_02ffa694(uVar14);

  return 0;

}




