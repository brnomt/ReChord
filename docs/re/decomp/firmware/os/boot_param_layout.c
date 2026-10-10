/**
 * boot_param_layout @ 0x030000da
 * Tags: init
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void boot_param_layout(ushort *param_1)



{

  undefined1 *puVar1;

  uint uVar2;

  int iVar3;

  ushort uVar4;

  uint uVar5;

  

  *DAT_03000164 = 8;

  uVar4 = 0;

  switch(*param_1) {

  case 0:

  case 1:

  case 2:

  case 3:

  case 4:

  case 5:

  case 8:

  case 10:

    uVar4 = *param_1;

  }

  *DAT_03000168 = uVar4;

  puVar1 = DAT_03000164;

  uVar4 = 0;

  switch(*param_1) {

  case 0:

  case 1:

  case 2:

  case 3:

  case 4:

  case 5:

  case 8:

  case 10:

    uVar4 = *param_1;

  }

  DAT_03000164[5] = (char)uVar4;

  if (0xc < (uVar4 & 0xff)) {

    puVar1[5] = 0xd;

  }

  if (0xc < *DAT_03000168) {

    *DAT_03000168 = 0xd;

  }

  uVar2 = (uint)(byte)puVar1[5];

  uVar5 = ((uVar2 / 6) * 3 & 0x7f) * 2;

  puVar1[1] = (char)uVar5;

  if (uVar2 <= uVar5 && uVar5 - uVar2 != 0) {

    uVar5 = uVar5 - 6;

    puVar1[1] = (char)uVar5;

  }

  puVar1[2] = puVar1[5] - (char)uVar5;

  iVar3 = 8 - (uVar5 & 0xff);

  if (6 < iVar3) {

    iVar3 = 6;

  }

  puVar1[4] = (char)iVar3;

  puVar1[3] = 0;

  return;

}
