/**
 * hifi_debug_printf_ovl_09e0 @ 0x0309e078
 * Tags: os, overlay
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



void hifi_debug_printf_ovl_09e0

               (undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)



{

  char cVar1;

  byte bVar2;

  uint uVar3;

  int iVar4;

  int iVar5;

  uint uVar6;

  char *pcVar7;

  int iVar8;

  int iVar9;

  int iVar10;

  code *UNRECOVERED_JUMPTABLE;

  undefined1 local_130;

  undefined1 local_12f;

  undefined1 local_12e;

  undefined1 local_12d;

  undefined1 local_12c;

  undefined1 uStack_12b;

  char acStack_12a [249];

  char cStack_31;

  char local_30 [6];

  char local_2a [6];

  undefined4 uStack_c;

  undefined4 uStack_8;

  undefined4 uStack_4;

  

  uVar3 = *DAT_0309e2f4;

  pcVar7 = &cStack_31;

  iVar9 = 4;

  uStack_c = param_2;

  uStack_8 = param_3;

  uStack_4 = param_4;

  do {

    uVar6 = (uint)((ulonglong)DAT_0309e2f8 * (ulonglong)uVar3 >> 0x23);

    cVar1 = (char)uVar6;

    pcVar7[1] = (char)uVar3 + cVar1 * -10;

    uVar3 = (uint)((ulonglong)DAT_0309e2f8 * (ulonglong)uVar6 >> 0x23);

    pcVar7 = pcVar7 + 2;

    *pcVar7 = cVar1 + (char)uVar3 * -10;

    iVar9 = iVar9 + -1;

  } while (iVar9 != 0);

  local_130 = 0xd;

  local_12f = 10;

  local_12e = 0x5b;

  local_12c = 0x5d;

  uStack_12b = 0x5b;

  iVar9 = 0;

  local_12d = 0x42;

  pcVar7 = local_2a + 2;

  iVar8 = 0;

  iVar10 = 4;

  do {

    acStack_12a[iVar9] = pcVar7[-1] + '0';

    iVar4 = iVar9;

    if (iVar8 == 5) {

      iVar4 = iVar9 + 1;

      acStack_12a[iVar9 + 1] = '.';

    }

    pcVar7 = pcVar7 + -2;

    iVar5 = iVar4 + 1;

    acStack_12a[iVar4 + 1] = *pcVar7 + '0';

    if (iVar8 == 4) {

      iVar5 = iVar4 + 2;

      acStack_12a[iVar4 + 2] = '.';

    }

    iVar9 = iVar5 + 1;

    iVar8 = iVar8 + 2;

    iVar10 = iVar10 + -1;

  } while (iVar10 != 0);

  acStack_12a[iVar5 + 1] = ']';

  bVar2 = FUN_030a56b8(acStack_12a + iVar5 + 2,0x100,param_1,&uStack_c);

  FUN_0309f692(DAT_0309e2ec,&local_130,bVar2 + 0x12);

  pcVar7 = DAT_0309e2e8;

  *DAT_0309e2e8 = '\x01';

  FUN_030a5604(5,0,3);

  FUN_030a5630(DAT_0309e2ec,0,3);

  if (*pcVar7 == '\x01') {

    hifi_busy_delay_ovl_09e3(1);

    iVar9 = DAT_0309e2f0;

    do {

      if (*pcVar7 != '\x01') {

        return;

      }

      hifi_busy_delay_ovl_09e3();

      iVar9 = iVar9 + -1;

    } while (iVar9 != 0);

    return;

  }

                    /* WARNING: Could not recover jumptable at 0x0309e160. Too many branches */

                    /* WARNING: Treating indirect jump as call */

  (*UNRECOVERED_JUMPTABLE)();

  return;

}
