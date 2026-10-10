/**
 * bitreader_get_u32_be @ 0x030f068c
 * Tags: os, bitstream
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint bitreader_get_u32_be(undefined4 param_1)



{

  int iVar1;

  int iVar2;

  uint uVar3;

  

  uVar3 = 0x20;

  iVar1 = 0;

  do {

    iVar2 = SNOR_WriteStatus(param_1,8);

    uVar3 = uVar3 - 8 & 0xff;

    iVar1 = iVar2 + iVar1 * 0x100;

  } while (7 < uVar3);

  iVar2 = SNOR_WriteStatus(param_1,uVar3);

  uVar3 = iVar2 + (iVar1 << uVar3);

  return uVar3 >> 0x18 | ((uVar3 & 0xffffff) >> 0x10) << 8 | ((uVar3 & 0xffff) >> 8) << 0x10 |

         uVar3 * 0x1000000;

}
