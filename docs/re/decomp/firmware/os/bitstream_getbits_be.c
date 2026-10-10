/**
 * bitstream_getbits_be @ 0x03070b0c
 * Tags: os, bitstream
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint bitstream_getbits_be(int *param_1,int param_2)



{

  uint uVar1;

  uint uVar2;

  uint uVar3;

  uint *puVar4;

  

  if (param_2 == 0) {

    return 0;

  }

  uVar3 = param_1[1];

  param_1[1] = uVar3 + param_2;

  puVar4 = (uint *)(*param_1 + (uVar3 >> 5) * 4);

  uVar1 = *puVar4;

  uVar1 = (uVar1 << 0x18 | (uVar1 >> 8 & 0xff) << 0x10 | (uVar1 >> 0x10 & 0xff) << 8 | uVar1 >> 0x18

          ) << (uVar3 & 0x1f);

  if (0x20 < (uVar3 & 0x1f) + param_2) {

    uVar2 = puVar4[1];

    return uVar1 + ((uVar2 << 0x18 | (uVar2 >> 8 & 0xff) << 0x10 | (uVar2 >> 0x10 & 0xff) << 8 |

                    uVar2 >> 0x18) >> (-uVar3 & 0x1f)) >> (0x20U - param_2 & 0xff);

  }

  return uVar1 >> (0x20U - param_2 & 0xff);

}
