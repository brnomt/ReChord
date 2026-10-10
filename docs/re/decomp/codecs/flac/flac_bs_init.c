/**
 * flac_bs_init @ 0x030dd888
 * Tags: codec, flac, bitstream
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 flac_bs_init(int *param_1,int param_2,int param_3)



{

  int iVar1;

  

  iVar1 = param_3 + 7 >> 3;

  if (-1 < iVar1 && -1 < param_3) {

    *param_1 = param_2;

    param_1[1] = param_2 + iVar1;

    param_1[2] = 0;

    param_1[3] = param_3;

    return 0;

  }

  return 0xffffffff;

}
