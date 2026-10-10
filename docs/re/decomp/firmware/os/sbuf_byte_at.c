/**
 * sbuf_byte_at @ 0x030c6984
 * Tags: os, bitstream, sbuf
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined1 sbuf_byte_at(int *param_1,int param_2)



{

  int *piVar1;

  

  if (param_2 < param_1[3]) {

    piVar1 = (int *)*param_1;

    param_1[3] = 0;

    param_1[1] = (int)piVar1;

    param_1[4] = piVar1[2];

    param_1[2] = piVar1[1] + *piVar1;

  }

  FUN_030c68b8(param_1,param_2);

  return *(undefined1 *)(param_1[2] + (param_2 - param_1[3]));

}
