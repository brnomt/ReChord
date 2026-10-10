/**
 * SDC_UpdateFreq @ 0x03082bfa
 * Tags: sd, config
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




/* WARNING: Removing unreachable block (ram,0x03082d76) */

/* WARNING: Removing unreachable block (ram,0x03082d84) */

/* WARNING: Removing unreachable block (ram,0x03082d86) */



uint FUN_03082bfa(uint param_1)



{

  int iVar1;

  uint uVar2;

  uint uVar3;

  int iVar4;

  

  uVar3 = (int)param_1 >> 0x1f;

  iVar1 = (param_1 ^ uVar3) - uVar3;

  iVar4 = LZCOUNT(iVar1);

  uVar2 = iVar1 << iVar4;

  if (uVar2 == 0) {

    return 0;

  }

  iVar4 = 0x9d - iVar4;

  if (-1 < iVar4) {

    uVar3 = (uVar2 >> 8) + iVar4 * 0x800000 + uVar3 * -0x80000000;

    if ((int)(uVar2 * 0x1000000) < 0) {

      uVar3 = uVar3 + 1;

      if ((uVar2 & 0x7f) == 0) {

        uVar3 = uVar3 & 0xfffffffe;

      }

      return uVar3;

    }

    return uVar3;

  }

  return 0;

}




