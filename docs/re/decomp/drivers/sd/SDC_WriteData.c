/**
 * SDC_WriteData @ 0x030f0384
 * SDK-matched (structural+combined, documented in MULTI-VERSION-PLAN §10).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void SDC_WriteData(void)



{

  int iVar1;

  int iVar2;

  int iVar3;

  

  iVar1 = DAT_030f046c;

  FUN_030f07f8(*(undefined4 *)(DAT_030f046c + 0x10));

  FUN_030f07f8(*(undefined4 *)(iVar1 + 0x7c));

  iVar2 = DAT_030f0470;

  iVar1 = DAT_030f0468;

  iVar3 = 0;

  do {

    if (*(int *)(iVar1 + iVar3 * 4 + 0x8954) == 0) {

      if (*(short *)(iVar2 + *(int *)(iVar1 + 0x10) * 6 + iVar3 * 2) == 0) {

        return;

      }

      FUN_030f07f8();

    }

    iVar3 = iVar3 + 1;

  } while (iVar3 < 3);

  return;

}
