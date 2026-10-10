/**
 * flac_hifi_cfg_write @ 0x030de824
 * Tags: codec, flac, hifi
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void flac_hifi_cfg_write(int param_1,int param_2,int param_3)



{

  *(int *)(param_1 * 0x3c + 0x106002c) = param_2 << 0x10 | param_3 << 8;

  return;

}
