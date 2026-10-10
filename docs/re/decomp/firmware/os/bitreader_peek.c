/**
 * bitreader_peek @ 0x0301e724
 * Tags: os, bitstream
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint bitreader_peek(uint *param_1,int param_2)



{

  uint uVar1;

  uint uVar2;

  uint uVar3;

  

  uVar3 = param_1[3];

  uVar1 = param_2 + -0x20 + uVar3;

  uVar2 = *param_1 & 0xffffffffU >> (uVar3 & 0xff);

  if (0 < (int)uVar1) {

    return param_1[1] >> (0x20 - uVar1 & 0xff) | uVar2 << (uVar1 & 0xff);

  }

  return uVar2 >> ((0x20 - uVar3) - param_2 & 0xff);

}
