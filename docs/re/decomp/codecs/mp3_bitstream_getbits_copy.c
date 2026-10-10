/**
 * mp3_bitstream_getbits_copy @ 0x03028338
 * Tags: codec, mp3, bitstream (similarity match)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void mp3_bitstream_getbits_copy(int param_1)



{

  uint uVar1;

  int iVar2;

  int iVar3;

  int iVar4;

  uint uVar5;

  int iVar6;

  

  iVar2 = DAT_030283e0;

  uVar5 = *(uint *)(DAT_030283e0 + 0x54);

  iVar6 = *(int *)(DAT_030283e0 + 0x58) - param_1;

  if (iVar6 < 1) {

    uVar1 = -(param_1 + iVar6) & 0x1f;

    iVar3 = PmuSetPowerMode();

    iVar4 = PmuSetPowerMode();

    uVar5 = (uVar5 >> uVar1 | uVar5 << 0x20 - uVar1) & 0xffff | iVar3 << 0x18 | iVar4 << 0x10;

    uVar5 = uVar5 << (-iVar6 & 0xffU) | uVar5 >> (iVar6 + 0x20U & 0xff);

    iVar6 = iVar6 + 0x10;

  }

  else {

    uVar5 = uVar5 >> (-param_1 & 0x1fU) | uVar5 << 0x20 - (-param_1 & 0x1fU);

  }

  *(uint *)(iVar2 + 0x54) = uVar5;

  *(int *)(iVar2 + 0x58) = iVar6;

  return;

}
