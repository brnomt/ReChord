/**
 * AudioStateHandler @ 0x0302a74e
 * Named via cross-version lineage (v3.4.0/v3.0.0 clusters); decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 AudioStateHandler(short param_1)



{

  ushort *puVar1;

  int iVar2;

  uint *puVar3;

  int iVar4;

  uint uVar5;

  undefined4 uVar6;

  short *psVar7;

  

  uVar6 = 1;

  *DAT_0302a860 = 2;

  iVar4 = DAT_0302a888;

  *(undefined4 *)(DAT_0302a888 + 0x10) = 0;

  iVar2 = DAT_0302ac84;

  puVar1 = DAT_0302ac80;

  iVar4 = *(int *)(iVar4 + 0xc);

  switch(*(undefined4 *)(iVar4 + 0x1c)) {

  case 0:

  case 3:

    DAT_0302ac80[5] = 2;

    break;

  case 1:

  case 5:

    if (*(int *)(DAT_0302a888 + 8) != 1) goto LAB_0302aabe;

    break;

  case 2:

    DAT_0302ac80[5] = 2;

    uVar5 = (uint)puVar1[1];

    if (*(int *)(iVar4 + 0x20) == 1) {

      uVar5 = *(uint *)(iVar2 + 0x14);

    }

    if ((uVar5 < *puVar1) || (*(int *)(iVar2 + 8) == 1)) {

      func_0x03001ed4(DAT_0302ac80);

      if (*(int *)(*(int *)(iVar2 + 0xc) + 0x20) == 1) {

        iVar4 = *(int *)(iVar2 + 0x14) + (int)param_1;

        *(int *)(iVar2 + 0x14) = iVar4;

        if (iVar4 == 0) {

          *(uint *)(iVar2 + 0x14) = (uint)*puVar1;

        }

      }

      if ((*DAT_0302ac9c == 1) && (*DAT_0302ac98 == 1 || *DAT_0302ac98 == 2)) {

        *DAT_0302ac98 = 0;

      }

      goto LAB_0302aabe;

    }

    func_0x03001ed4(DAT_0302ac80,(int)param_1);

    if (*(int *)(*(int *)(iVar2 + 0xc) + 0x20) == 1) {

      *(undefined4 *)(iVar2 + 0x14) = 1;

    }

    *DAT_0302ac98 = 3;

    AudioPlayback_Start();

    if (*DAT_0302ac9c == 1) {

      MediaLib_thunk_GetFiles(s__________AudioCodec_Open_Error___0302aca8);

      func_0x03001ed4(DAT_0302ac80,(int)param_1);

      return 0;

    }

    FUN_0302c950(0);

    puVar3 = *(uint **)(iVar2 + 0xc);

    if (*puVar3 <= puVar3[1]) {

      puVar3[1] = *puVar3;

      iVar4 = DAT_0302ac8c + *DAT_0302ac88 * 0x27e;

      *(undefined4 *)(iVar4 + 0x20) = *(undefined4 *)(iVar4 + 0x10);

    }

    psVar7 = DAT_0302ac94;

    if (*(char *)(DAT_0302ac90 + 0x34a) == '\0') goto LAB_0302aa6e;

    if (*DAT_0302ac94 == 0 || *DAT_0302ac94 == 2) {

      FUN_0300afd8(0x87);

      iVar4 = FUN_0300c6d0(0x1ca);

joined_r0x0302a910:

      if (iVar4 != 0) {

        FUN_0300b00a(DAT_0302aca4,(int)*DAT_0302aca0);

      }

    }

    goto LAB_0302a912;

  case 4:

  case 7:

    DAT_0302ac80[5] = 3;

    break;

  case 6:

  case 9:

    uVar5 = (uint)DAT_0302ac80[1];

    if (*(int *)(iVar4 + 0x20) == 1) {

      uVar5 = *(uint *)(DAT_0302ac84 + 0x14);

    }

    if ((uVar5 < *DAT_0302ac80) || (*(int *)(DAT_0302ac84 + 8) == 1)) {

      func_0x03001ed4(DAT_0302ac80);

      if (*(int *)(*(int *)(iVar2 + 0xc) + 0x20) == 1) {

        iVar4 = *(int *)(iVar2 + 0x14) + (int)param_1;

        *(int *)(iVar2 + 0x14) = iVar4;

        if (iVar4 == 0) {

          *(uint *)(iVar2 + 0x14) = (uint)*puVar1;

        }

      }

      goto LAB_0302aabe;

    }

    func_0x03001ed4(DAT_0302ac80,(int)param_1);

    if (*(int *)(*(int *)(iVar2 + 0xc) + 0x20) == 1) {

      *(undefined4 *)(iVar2 + 0x14) = 1;

    }

    AudioPlayback_Start();

    FUN_0302c950(0);

    puVar3 = *(uint **)(iVar2 + 0xc);

    if (*puVar3 <= puVar3[1]) {

      puVar3[1] = *puVar3;

      iVar4 = DAT_0302ac8c + *DAT_0302ac88 * 0x27e;

      *(undefined4 *)(iVar4 + 0x20) = *(undefined4 *)(iVar4 + 0x10);

    }

    psVar7 = DAT_0302ac94;

    if (*(char *)(DAT_0302ac90 + 0x34a) == '\0') goto LAB_0302aa6e;

    if (*DAT_0302ac94 == 0 || *DAT_0302ac94 == 2) {

      FUN_0300afd8(0x87);

      iVar4 = FUN_0300c6d0(0x1ca);

      goto joined_r0x0302a910;

    }

LAB_0302a912:

    *psVar7 = 1;

LAB_0302aa6e:

    FUN_0302a69e();

    *(undefined4 *)(iVar2 + 0x38) = 0xffffffff;

    return 1;

  case 8:

    if (*(int *)(DAT_0302ac84 + 8) != 1) {

      AudioPlayback_Start();

      FUN_0302c950(0);

      puVar3 = *(uint **)(iVar2 + 0xc);

      if (*puVar3 <= puVar3[1]) {

        puVar3[1] = *puVar3;

        iVar4 = DAT_0302ac8c + *DAT_0302ac88 * 0x27e;

        *(undefined4 *)(iVar4 + 0x20) = *(undefined4 *)(iVar4 + 0x10);

      }

      psVar7 = DAT_0302ac94;

      if (*(char *)(DAT_0302ac90 + 0x34a) == '\0') goto LAB_0302aa6e;

      if (*DAT_0302ac94 == 0 || *DAT_0302ac94 == 2) {

        FUN_0300afd8(0x87);

        iVar4 = FUN_0300c6d0(0x1ca);

        goto joined_r0x0302a910;

      }

      goto LAB_0302a912;

    }

    break;

  default:

    uVar6 = 0;

    goto LAB_0302aabe;

  }

  func_0x03001ed4(DAT_0302a7a8,(int)param_1);

LAB_0302aabe:

  AudioPlayback_Start();

  return uVar6;

}
