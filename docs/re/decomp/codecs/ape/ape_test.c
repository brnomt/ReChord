/**
 * ape_test @ 0x0301e122
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 ape_test(int param_1,int param_2)



{

  undefined4 *puVar1;

  int iVar2;

  int iVar3;

  

  puVar1 = DAT_0301e31c;

  iVar3 = -1;

  if (param_2 == 1) {

    iVar2 = 0;

    if (0 < param_1) {

      do {

        iVar3 = ape_scan_cuesheet(puVar1);

        iVar2 = iVar2 + 1;

      } while (iVar2 < param_1);

    }

    puVar1[4] = 0;

    ape_rom_seek_thunk(puVar1[3],1,*puVar1);

  }

  else {

    if (param_2 != -1) {

      return 0xffffffff;

    }

    iVar2 = 0;

    if (0 < param_1) {

      do {

        iVar3 = FUN_0301dd9e(puVar1);

        iVar2 = iVar2 + 1;

      } while (iVar2 < param_1);

    }

    puVar1[4] = 0;

    ape_rom_seek_thunk(-puVar1[3],1,*puVar1);

  }

  if (iVar3 != 1) {

    if (iVar3 != 2) {

      return 0xffffffff;

    }

    return 1;

  }

  return 0;

}
