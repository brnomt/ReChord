/**
 * flac_hifi_ctrl_set @ 0x030e0936
 * Tags: codec, flac, hifi
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void flac_hifi_ctrl_set(int param_1)



{

  *(uint *)(param_1 * 0x3c + 0x1060004) = *(uint *)(param_1 * 0x3c + 0x1060004) | 0x8000;

  return;

}
