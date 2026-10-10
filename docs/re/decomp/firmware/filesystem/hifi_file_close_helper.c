/**
 * hifi_file_close_helper @ 0x0306c6c4
 * Named via cross-version lineage (v3.4.0/v3.0.0 clusters); decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void hifi_file_close_helper(int param_1,undefined4 param_2)



{

  int *piVar1;

  undefined4 *puVar2;

  undefined4 uVar3;

  int iVar4;

  uint uVar5;

  

  puVar2 = DAT_0306ca10;

  piVar1 = DAT_0306ca0c;

  if ((undefined4 *)*DAT_0306ca0c != DAT_0306ca10) {

    *DAT_0306ca10 = DAT_0306ca14;

    puVar2[1] = DAT_0306ca18;

    uVar3 = DAT_0306ca1c;

    uVar5 = 1;

    puVar2[2] = DAT_0306ca1c;

    iVar4 = 1;

    do {

      puVar2[iVar4 + 2] = uVar3;

      uVar5 = uVar5 + 2;

      puVar2[iVar4 + 3] = uVar3;

      iVar4 = iVar4 + 2;

    } while (uVar5 < 0x37);

    *piVar1 = (int)puVar2;

  }

  puVar2[param_1] = param_2;

  return;

}
