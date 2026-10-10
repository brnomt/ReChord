/**
 * APE_Codec_Open @ 0x030eef60
 * Tags: codec, ape
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 APE_Codec_Open(undefined4 param_1,uint *param_2,undefined4 *param_3)



{

  int iVar1;

  int iVar2;

  undefined4 *puVar3;

  uint uVar4;

  uint uVar5;

  uint uVar6;

  uint extraout_r2;

  int iVar7;

  uint extraout_r3;

  int *piVar8;

  bool bVar9;

  bool bVar10;

  char local_20;

  char local_1f;

  char local_1e;

  byte local_1c;

  byte local_1a;

  byte local_19;

  byte local_18;

  byte local_17;

  

  iVar1 = DAT_030ef174;

  puVar3 = DAT_030ef14c;

  iVar7 = DAT_030ef148;

  iVar2 = DAT_030ef140;

  switch(param_1) {

  case 3:

    uVar5 = *(int *)(DAT_030ef140 + 0x74) + 500;

    goto LAB_030ef10e;

  case 4:

    *param_2 = *(uint *)(DAT_030ef140 + 100);

    return 1;

  case 5:

    *param_2 = (uint)*(ushort *)(DAT_030ef140 + 0x62);

    break;

  case 6:

    uVar5 = *(uint *)(DAT_030ef140 + 0x70);

    goto LAB_030ef10e;

  case 7:

    uVar5 = *(uint *)(DAT_030ef140 + 100);

    if (uVar5 == 0) {

      return 1;

    }

    uVar4 = *(uint *)(DAT_030ef140 + 0x78) / uVar5;

    uVar5 = ((*(uint *)(DAT_030ef140 + 0x78) - uVar5 * uVar4) * 1000) / uVar5 + uVar4 * 1000;

    goto LAB_030ef10e;

  case 8:

    *(undefined4 *)(DAT_030ef148 + 0xc) = *DAT_030ef144;

    (*(code *)*puVar3)(0);

    (*(code *)*DAT_030ef150)(&local_20,0x14,*(undefined4 *)(iVar7 + 0xc));

    bVar9 = local_20 == 'I';

    if (bVar9) {

      local_20 = local_1f;

    }

    bVar10 = bVar9 && local_20 == 'D';

    if (bVar9 && local_20 == 'D') {

      bVar10 = local_1e == '3';

    }

    if (bVar10) {

      uVar4 = (uint)local_1c;

      bVar9 = 0xfe < uVar4;

      uVar5 = extraout_r2;

      if (uVar4 != 0xff) {

        uVar5 = (uint)local_1a;

        bVar9 = 0x7f < uVar5;

      }

      if (bVar9) goto LAB_030eefda;

      bVar9 = local_19 < 0x80;

      if (bVar9) {

        uVar4 = (uint)local_18;

      }

      uVar6 = extraout_r3;

      if (bVar9 && uVar4 < 0x80) {

        uVar6 = (uint)local_17;

      }

      if ((!bVar9 || uVar4 >= 0x80) || 0x7f < uVar6) goto LAB_030eefda;

      uVar5 = (uint)local_19 << 0xe | uVar4 << 7 | uVar6 | uVar5 << 0x15;

    }

    else {

LAB_030eefda:

      uVar5 = 0;

    }

    *(uint *)(iVar7 + 8) = uVar5;

    if (uVar5 == 0) {

      (*(code *)*puVar3)(0,0,*(undefined4 *)(iVar7 + 0xc));

    }

    else {

      *(uint *)(iVar7 + 8) = uVar5 + 10;

      (*(code *)*puVar3)(uVar5 + 10,0,*(undefined4 *)(iVar7 + 0xc));

    }

    FUN_030ef17c();

    iVar2 = APE_Set_CFG();

    mpi_mod_mpi();

    if (iVar2 < 0) {

      log_printf_ts(s_ape_open_FAIL__030ef154);

      return 0;

    }

    log_printf_ts(s_ape_open_OK_030ef164);

    break;

  default:

    return 0;

  case 10:

    puVar3 = (undefined4 *)(DAT_030ef174 + -0x10);

    *param_2 = *(int *)(DAT_030ef174 + *(int *)(DAT_030ef174 + -0xc) * 4) + *DAT_030ef178;

    *param_3 = *puVar3;

    *(uint *)(iVar1 + -0xc) = *(uint *)(iVar1 + -0xc) ^ 1;

    break;

  case 0xc:

    piVar8 = (int *)(DAT_030ef174 + -0x10);

    FUN_030f0360(*(int *)(DAT_030ef174 + *(int *)(DAT_030ef174 + -0xc) * 4) + *DAT_030ef178,piVar8);

    if (*piVar8 == 0) {

      return 0;

    }

    if (*(uint *)(iVar2 + 0x6c) <= *(uint *)(iVar2 + 0x78)) {

      return 0;

    }

    break;

  case 0xe:

    if (*(uint **)(DAT_030ef140 + 0x70) < param_2) {

      param_2 = *(uint **)(DAT_030ef140 + 0x70);

    }

    iVar7 = *(int *)(DAT_030ef140 + 100);

    uVar4 = *(uint *)(DAT_030ef140 + 0x54);

    uVar5 = (((uint)param_2 / 1000) * iVar7) / uVar4 +

            (iVar7 * ((uint)param_2 % 1000)) / (uVar4 * 1000);

    uVar6 = *(uint *)(DAT_030ef140 + 0x5c);

    if (uVar6 <= uVar5) {

      iVar7 = FUN_030ee87e((int)((ulonglong)uVar6 * (ulonglong)uVar4),

                           (int)((ulonglong)uVar6 * (ulonglong)uVar4 >> 0x20),iVar7,0);

      (*(code *)*DAT_030ef14c)(iVar7 * *(int *)(iVar2 + 0x74),0,*(undefined4 *)(DAT_030ef148 + 0xc))

      ;

      uVar5 = uVar6;

    }

    *(uint *)(iVar2 + 0x78) = uVar5 * *(int *)(iVar2 + 0x54);

    *(uint *)(iVar2 + 0x14) = uVar5;

    *(undefined4 *)(iVar2 + 0x18) = 0;

    break;

  case 0xf:

    FUN_030f0384();

    FUN_030ecc64();

    break;

  case 0x11:

    uVar5 = (uint)*(ushort *)(DAT_030ef140 + 0x60);

LAB_030ef10e:

    *param_2 = uVar5;

  }

  return 1;

}
