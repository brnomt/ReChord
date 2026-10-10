/**
 * flac_bs_bswap32 @ 0x030dd6d0
 * Tags: codec, flac, bitstream
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint flac_bs_bswap32(uint *param_1)



{

  uint uVar1;

  

  uVar1 = *param_1;

  return uVar1 >> 0x18 | (uVar1 >> 0x10 & 0xff) << 8 | ((uVar1 & 0xffff) >> 8 | uVar1 << 8) << 0x10;

}
