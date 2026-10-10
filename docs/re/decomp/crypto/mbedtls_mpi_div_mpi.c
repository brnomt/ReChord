/**
 * mbedtls_mpi_div_mpi @ 0x030094e4
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void mbedtls_mpi_div_mpi(undefined1 *param_1,short param_2,undefined4 param_3,short *param_4)



{

  undefined2 uVar1;

  undefined2 uVar2;

  undefined2 *puVar3;

  undefined4 uVar4;

  undefined4 uVar5;

  int iVar6;

  uint unaff_r4;

  undefined2 unaff_r6;

  undefined1 unaff_r7;

  char *unaff_r8;

  uint unaff_r10;

  int unaff_r11;

  char in_ZR;

  bool in_CY;

  

  if (in_ZR == '\0') {

    if (in_CY) {

      *param_4 = param_2 + 1;

      mbedtls_mpi_grow(0x96);

      FUN_03009156(0,0);

      *param_1 = unaff_r7;

    }

    else {

      FUN_02ff0c16();

    }

  }

  else {

    mbedtls_mpi_grow(0x96);

    FUN_03009156(0,(int)(short)-((short)unaff_r4 +

                                (short)(uint)((ulonglong)unaff_r10 * (ulonglong)unaff_r4 >> 0x24) *

                                -0x18),2);

  }

  *unaff_r8 = '\x02';

  memset(0x21);

  *(undefined2 *)(unaff_r11 + 4) = unaff_r6;

  FUN_02ff4124();

  puVar3 = DAT_03009744;

  if (*unaff_r8 == '\0') {

    uVar1 = *DAT_03009744;

    uVar2 = DAT_03009744[1];

    uVar4 = FUN_02ff2b38(1);

    uVar5 = FUN_02ff2b14(2);

    *puVar3 = 0xffff;

    puVar3[1] = 0xffff;

    iVar6 = UartDev_Write(0x32);

    if (iVar6 != 0) {

      mbedtls_mpi_grow(0x95);

    }

    iVar6 = UartDev_Write(0x95);

    if (((iVar6 == 1) || (iVar6 = UartDev_Write(0x26), iVar6 == 1)) ||

       (iVar6 = UartDev_Write(0x2f), iVar6 != 0)) {

      FUN_02ff33e6(0x7a);

      *puVar3 = *DAT_03009748;

      FUN_02ff2be4(0,3,0x140,0x10);

      *puVar3 = 0xffff;

      mbedtls_mpi_grow(0x21);

      mbedtls_mpi_grow(0x96);

      mbedtls_mpi_grow(0x9b);

      mbedtls_mpi_grow(0x1a9);

      FUN_02ff2eda(1);

    }

    FUN_02ff2eda(0);

    FUN_03009156(0,0);

    FUN_02ff2b38(uVar4);

    puVar3[1] = uVar2;

    *puVar3 = uVar1;

    FUN_02ff2b14(uVar5);

    return;

  }

  return;

}
