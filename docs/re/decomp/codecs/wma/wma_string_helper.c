/**
 * wma_string_helper @ 0x0309e264
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void wma_string_helper(char *param_1)



{

  char *pcVar1;

  undefined4 uVar2;

  int iVar3;

  int iVar4;

  

  iVar4 = DAT_0309e2ec;

  if (*param_1 == '\r') {

    FUN_030a56dc(DAT_0309e2ec,s__B__0309e2fc);

    iVar3 = FUN_0309f6da(param_1);

    FUN_0309f692(iVar4 + 5,param_1 + 2,iVar3 + -2);

  }

  else {

    uVar2 = FUN_0309f6da(param_1);

    FUN_0309f692(iVar4 + 5,param_1,uVar2);

  }

  pcVar1 = DAT_0309e2e8;

  *DAT_0309e2e8 = '\x01';

  FUN_030a5604(5,0,3);

  FUN_030a5630(DAT_0309e2ec,0,3);

  if (*pcVar1 != '\x01') {

    return;

  }

  hifi_busy_delay_ovl_09e3(1);

  iVar4 = DAT_0309e2f0;

  do {

    if (*pcVar1 != '\x01') {

      return;

    }

    hifi_busy_delay_ovl_09e3(1);

    iVar4 = iVar4 + -1;

  } while (iVar4 != 0);

  return;

}
