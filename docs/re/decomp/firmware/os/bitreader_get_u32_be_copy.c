/**
 * bitreader_get_u32_be_copy @ 0x030f064c
 * Tags: os, bitstream (similarity match)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint bitreader_get_u32_be_copy(undefined4 param_1)



{

  uint uVar1;

  int iVar2;

  uint uVar3;

  

  uVar3 = 0x10;

  uVar1 = 0;

  do {

    iVar2 = SNOR_WriteStatus(param_1,8);

    uVar3 = uVar3 - 8 & 0xff;

    uVar1 = iVar2 + (uVar1 & 0xff) * 0x100 & 0xffff;

  } while (7 < uVar3);

  iVar2 = SNOR_WriteStatus(param_1,uVar3);

  uVar1 = iVar2 + (uVar1 << uVar3);

  return (uVar1 & 0xff) << 8 | (uVar1 & 0xffff) >> 8;

}
