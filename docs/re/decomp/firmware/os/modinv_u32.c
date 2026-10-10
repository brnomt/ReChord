/**
 * modinv_u32 @ 0x0308fb94
 * Tags: os, math
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int modinv_u32(uint param_1)



{

  int iVar1;

  uint uVar2;

  int iVar3;

  uint uVar4;

  uint *puVar5;

  int iVar6;

  uint uVar7;

  uint auStack_198 [97];

  

  if (param_1 < 2) {

    return 1;

  }

  auStack_198[0x30] = 0xffffffff / param_1;

  uVar7 = -(param_1 * auStack_198[0x30]);

  if (uVar7 == param_1) {

    uVar4 = 1;

    auStack_198[0x30] = auStack_198[0x30] + 1;

  }

  else {

    uVar4 = 1;

    if (uVar7 != 0) {

      while( true ) {

        auStack_198[uVar4 + 0x30] = param_1 / uVar7;

        uVar2 = param_1 - uVar7 * (param_1 / uVar7);

        uVar4 = (uint)(short)((short)uVar4 + 1);

        if (uVar2 == 0) break;

        param_1 = uVar7;

        uVar7 = uVar2;

        if (uVar4 == 0x31) {

          return 0;

        }

      }

    }

  }

  iVar3 = 1;

  iVar1 = 1;

  if (0 < (int)(uVar4 - 1)) {

    puVar5 = auStack_198 + uVar4 + 0x30;

    if ((uVar4 & 1) == 0) {

      puVar5 = auStack_198 + uVar4 + 0x2f;

      iVar1 = 1 - *puVar5;

    }

    uVar7 = puVar5[-1];

    for (iVar6 = (int)(uVar4 - 1) >> 1; iVar6 != 0; iVar6 = iVar6 + -1) {

      iVar3 = iVar3 - iVar1 * uVar7;

      uVar7 = puVar5[-3];

      iVar1 = iVar1 - iVar3 * puVar5[-2];

      puVar5 = puVar5 + -2;

    }

  }

  if (-1 < (int)(uVar4 - 1)) {

    return iVar3 - iVar1 * auStack_198[(uVar4 - (uVar4 - 1)) + 0x2f];

  }

  return iVar1;

}
