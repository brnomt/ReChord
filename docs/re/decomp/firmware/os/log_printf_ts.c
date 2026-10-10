/**
 * log_printf_ts @ 0x030ed3e6
 * Tags: os, logging
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void log_printf_ts(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)



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

  undefined1 local_130;

  undefined1 local_12f;

  undefined1 local_12e;

  undefined1 local_12d;

  undefined1 local_12c;

  undefined1 local_12b;

  char local_12a [249];

  char cStack_31;

  char local_30 [6];

  char local_2a [6];

  undefined4 uStack_c;

  undefined4 uStack_8;

  undefined4 uStack_4;

  

  uVar3 = *DAT_030ed608;

  pcVar7 = &cStack_31;

  iVar9 = 4;

  uStack_c = param_2;

  uStack_8 = param_3;

  uStack_4 = param_4;

  do {

    uVar6 = (uint)((ulonglong)DAT_030ed60c * (ulonglong)uVar3 >> 0x23);

    cVar1 = (char)uVar6;

    pcVar7[1] = (char)uVar3 + cVar1 * -10;

    uVar3 = (uint)((ulonglong)DAT_030ed60c * (ulonglong)uVar6 >> 0x23);

    pcVar7 = pcVar7 + 2;

    *pcVar7 = cVar1 + (char)uVar3 * -10;

    iVar9 = iVar9 + -1;

  } while (iVar9 != 0);

  local_130 = 0xd;

  local_12f = 10;

  local_12e = 0x5b;

  local_12c = 0x5d;

  local_12b = 0x5b;

  iVar9 = 0;

  local_12d = 0x42;

  pcVar7 = local_2a + 2;

  iVar8 = 0;

  iVar10 = 4;

  do {

    local_12a[iVar9] = pcVar7[-1] + '0';

    iVar4 = iVar9;

    if (iVar8 == 5) {

      iVar4 = iVar9 + 1;

      local_12a[iVar9 + 1] = '.';

    }

    pcVar7 = pcVar7 + -2;

    iVar5 = iVar4 + 1;

    local_12a[iVar4 + 1] = *pcVar7 + '0';

    if (iVar8 == 4) {

      iVar5 = iVar4 + 2;

      local_12a[iVar4 + 2] = '.';

    }

    iVar9 = iVar5 + 1;

    iVar8 = iVar8 + 2;

    iVar10 = iVar10 + -1;

  } while (iVar10 != 0);

  local_12a[iVar5 + 1] = ']';

  bVar2 = FUN_030f199c(local_12a + iVar5 + 2,0x100,param_1,&uStack_c);

  FUN_030ee8d8(DAT_030ed610,&local_130,bVar2 + 0x12);

  return;

}
