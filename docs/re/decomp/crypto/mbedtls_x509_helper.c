/**
 * mbedtls_x509_helper @ 0x03017adc
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void mbedtls_x509_helper(undefined4 param_1,undefined4 param_2)



{

  undefined4 *puVar1;

  undefined4 *puVar2;

  undefined1 uVar3;

  undefined1 uVar4;

  int iVar5;

  int unaff_r4;

  undefined4 *unaff_r5;

  undefined4 unaff_r6;

  undefined4 unaff_r7;

  undefined4 *puVar6;

  longlong lVar7;

  

  puVar1 = (undefined4 *)0x3f8;

  puVar2 = (undefined4 *)0x400;

  lVar7 = 2;

  puVar6 = unaff_r5;

  while( true ) {

    *puVar1 = *puVar6;

    puVar6 = puVar6 + 1;

    *puVar2 = *puVar6;

    puVar6 = puVar6 + 1;

    lVar7 = lVar7 + -1;

    if (lVar7 == 0) break;

    puVar1 = puVar1 + 1;

    puVar2 = puVar2 + 1;

  }

  iVar5 = (int)unaff_r5 * 2;

  *(undefined4 *)(unaff_r4 + 0x18) = param_2;

  uVar3 = (undefined1)unaff_r6;

  *(undefined1 *)(iVar5 + 0x1bd) = uVar3;

  *(undefined1 *)(iVar5 + 0x2b) = uVar3;

  uVar4 = (undefined1)unaff_r7;

  *(undefined1 *)(iVar5 + 0x34f) = uVar4;

  *(undefined1 *)(iVar5 + 0x352) = uVar4;

  *(undefined1 *)(iVar5 + 0x353) = uVar3;

  *DAT_03017f58 = 0xffff;

  *DAT_03017f5c = 0xffff;

  *DAT_03017f60 = 0x2945;

  *(undefined1 *)(iVar5 + 0x1be) = uVar4;

  *(undefined1 *)(iVar5 + 0x1bf) = uVar3;

  *(undefined1 *)(iVar5 + 0x350) = 0xb;

  *(undefined1 *)(iVar5 + 0x356) = uVar3;

  *(undefined1 *)(iVar5 + 0x355) = uVar3;

  *(undefined1 *)(iVar5 + 0x357) = uVar3;

  *(undefined1 *)(iVar5 + 0x359) = uVar4;

  *(undefined1 *)(iVar5 + 0x34c) = uVar3;

  *(undefined1 *)(iVar5 + 0x35a) = uVar3;

  *(undefined1 *)(iVar5 + 0x35e) = uVar4;

  return;

}
