/**
 * sleep_timer_helper @ 0x0304e108
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 sleep_timer_helper(undefined4 param_1,undefined4 *param_2,undefined4 *param_3)



{

  short *psVar1;

  int *piVar2;

  undefined4 *puVar3;

  char *pcVar4;

  int *piVar5;

  undefined1 uVar6;

  undefined4 uVar7;

  int iVar8;

  int iVar9;

  int *piVar10;

  undefined4 local_30;

  undefined4 uStack_2c;

  undefined4 local_28;

  undefined4 uStack_24;

  

  piVar5 = DAT_0304e394;

  piVar10 = DAT_0304e38c;

  puVar3 = DAT_0304e374;

  piVar2 = DAT_0304e368;

  switch(param_1) {

  case 3:

    uVar7 = DAT_0304e374[2];

    goto LAB_0304e320;

  case 4:

    *param_2 = DAT_0304e374[4];

    return 1;

  case 5:

    *param_2 = DAT_0304e374[5];

    return 1;

  case 6:

    uVar7 = DAT_0304e374[1];

LAB_0304e320:

    *param_2 = uVar7;

    break;

  case 7:

    *param_2 = *DAT_0304e374;

    return 1;

  case 8:

    *DAT_0304e368 = 0;

    FUN_03032760(1,0);

    FUN_03032734(0,0,1);

    iVar8 = *piVar2;

    iVar9 = DAT_0304e36c;

    while (iVar8 == 0) {

      USBMSCDevShellTestRead();

      FUN_030176b0();

      FUN_03029b80(1);

      iVar9 = iVar9 + -1;

      if (iVar9 == 0) break;

      iVar8 = *piVar2;

    }

    piVar10 = DAT_0304e370;

    *piVar2 = 0;

    if (*piVar10 != 0) {

      return 0;

    }

    break;

  case 9:

  case 0xb:

  case 0xd:

  case 0x10:

    return 0;

  case 10:

    if (DAT_0304e374[9] == 1) {

      if (DAT_0304e374[0xb] == 1) {

        return 0;

      }

      *param_3 = DAT_0304e374[8];

      puVar3[9] = 0;

      puVar3[10] = 1;

      FUN_03032760(4,0,1);

      FUN_03032734(0,0,1);

      pcVar4 = DAT_0304e378;

      psVar1 = DAT_0304e364;

      if (*DAT_0304e378 != '\0') {

        local_30 = DAT_0304e37c[4];

        uStack_2c = DAT_0304e37c[5];

        local_28 = DAT_0304e37c[6];

        uStack_24 = DAT_0304e37c[7];

        *DAT_0304e364 = 0;

        uVar6 = dumpMemoryShortA2String();

        FUN_030248c8(uVar6,puVar3[7],DAT_0304e384,(uint)(puVar3[3] * puVar3[8]) >> 4,&local_30,

                     DAT_0304e380);

        do {

          thunk_FUN_0301a37c();

        } while (*psVar1 != 1);

        *param_2 = DAT_0304e384;

        *pcVar4 = '\0';

        return 1;

      }

      local_30 = *DAT_0304e37c;

      uStack_2c = DAT_0304e37c[1];

      local_28 = DAT_0304e37c[2];

      uStack_24 = DAT_0304e37c[3];

      *DAT_0304e364 = 0;

      uVar6 = dumpMemoryShortA2String();

      FUN_030248c8(uVar6,puVar3[7],DAT_0304e388,(uint)(puVar3[3] * puVar3[8]) >> 4,&local_30,

                   DAT_0304e380);

      do {

        thunk_FUN_0301a37c();

      } while (*psVar1 != 1);

      *param_2 = DAT_0304e388;

      *pcVar4 = '\x01';

      return 1;

    }

    if (DAT_0304e374[10] == 0) {

      DAT_0304e374[10] = 1;

      FUN_03032760(4,0,1);

      FUN_03032734(0,0,1);

      return 2;

    }

    if (DAT_0304e374[10] != 1) {

      return 0;

    }

    return 2;

  case 0xc:

    FUN_03032760(4,0,1);

    FUN_03032734(0,0,1);

    break;

  case 0xe:

    *DAT_0304e38c = 0;

    FUN_03032760(0xb,0,1);

    FUN_03032734(param_2,0,1);

    iVar8 = *piVar10;

    iVar9 = DAT_0304e390;

    while (iVar8 == 0) {

      USBMSCDevShellTestRead();

      FUN_030176b0();

      FUN_03029b80(1);

      iVar9 = iVar9 + -1;

      if (iVar9 == 0) break;

      iVar8 = *piVar10;

    }

    goto LAB_0304e2d6;

  case 0xf:

    *DAT_0304e394 = 0;

    FUN_03032760(0xd,0,1);

    FUN_03032734(0,0,1);

    iVar8 = *piVar5;

    iVar9 = DAT_0304e398;

    while (piVar10 = piVar5, iVar8 == 0) {

      USBMSCDevShellTestRead();

      FUN_030176b0();

      FUN_03029b80(1);

      iVar9 = iVar9 + -1;

      if (iVar9 == 0) break;

      iVar8 = *piVar5;

    }

LAB_0304e2d6:

    *piVar10 = 0;

    break;

  case 0x11:

    *param_2 = DAT_0304e374[3];

    break;

  default:

    return 0;

  }

  return 1;

}
