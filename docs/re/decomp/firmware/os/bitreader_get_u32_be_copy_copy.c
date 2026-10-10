/**
 * bitreader_get_u32_be_copy_copy @ 0x030f061a
 * Tags: os, bitstream (similarity match)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int bitreader_get_u32_be_copy_copy(undefined4 param_1,uint param_2)



{

  int iVar1;

  int iVar2;

  

  iVar1 = 0;

  for (; 7 < param_2; param_2 = param_2 - 8 & 0xff) {

    iVar2 = SNOR_WriteStatus(param_1,8);

    iVar1 = iVar2 + iVar1 * 0x100;

  }

  iVar2 = SNOR_WriteStatus(param_1,param_2);

  return iVar2 + (iVar1 << (param_2 & 0xff));

}
