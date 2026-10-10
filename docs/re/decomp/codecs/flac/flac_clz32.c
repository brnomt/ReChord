/**
 * flac_clz32 @ 0x030dd956
 * Tags: codec, flac, util
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void flac_clz32(uint param_1)



{

  int iVar1;

  uint uVar2;

  

  uVar2 = 0x80000000;

  if (param_1 == 0) {

    return;

  }

  iVar1 = 0;

  do {

    if ((uVar2 & ~param_1) == 0) {

      return;

    }

    uVar2 = uVar2 >> 1;

    iVar1 = iVar1 + 1;

  } while (iVar1 < 0x20);

  return;

}
