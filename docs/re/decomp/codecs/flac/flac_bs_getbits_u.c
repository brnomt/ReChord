/**
 * flac_bs_getbits_u @ 0x030dd842
 * Tags: codec, flac, bitstream
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int * flac_bs_getbits_u(int *param_1,int param_2)



{

  uint uVar1;

  uint uVar2;

  

  if (0x20 < param_2) {

    return param_1;

  }

  uVar2 = param_1[2];

  uVar1 = *(uint *)(*param_1 + ((int)uVar2 >> 3));

  param_1[2] = param_2 + uVar2;

  return (int *)(((uVar1 >> 0x18 | (uVar1 >> 0x10 & 0xff) << 8 |

                  ((uVar1 & 0xffff) >> 8 | uVar1 << 8) << 0x10) << (uVar2 & 7)) >>

                (0x20U - param_2 & 0xff));

}
