/**
 * lcd_audio_helper @ 0x03027a7c
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void lcd_audio_helper(int param_1)



{

  uint uVar1;

  int iVar2;

  int iVar3;

  int iVar4;

  uint uVar5;

  uint uVar6;

  

  iVar2 = DAT_03027e38;

  uVar5 = *(uint *)(DAT_03027e38 + 0x54);

  uVar6 = *(int *)(DAT_03027e38 + 0x58) - param_1;

  if ((int)uVar6 < 1) {

    uVar1 = -(param_1 + uVar6) & 0x1f;

    iVar3 = power_mode_get();

    iVar4 = power_mode_get();

    uVar5 = (uVar5 >> uVar1 | uVar5 << 0x20 - uVar1) & 0xffff | iVar3 << 0x18 | iVar4 << 0x10;

    uVar5 = uVar5 >> (uVar6 & 0x1f) | uVar5 << 0x20 - (uVar6 & 0x1f);

    uVar6 = uVar6 + 0x10;

  }

  else {

    uVar5 = uVar5 >> (-param_1 & 0x1fU) | uVar5 << 0x20 - (-param_1 & 0x1fU);

  }

  *(uint *)(iVar2 + 0x54) = uVar5;

  *(uint *)(iVar2 + 0x58) = uVar6;

  return;

}
