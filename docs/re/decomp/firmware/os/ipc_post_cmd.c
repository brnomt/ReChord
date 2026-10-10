/**
 * ipc_post_cmd @ 0x03073c7c
 * Tags: os, ipc
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 ipc_post_cmd(undefined4 param_1,int param_2,int param_3)



{

  int iVar1;

  

  iVar1 = DAT_03073d2c + param_2 * 0x50;

  if (param_3 == 0) {

    *(undefined4 *)(iVar1 + 0x30) = param_1;

  }

  else if (param_3 == 1) {

    *(undefined4 *)(iVar1 + 0x38) = param_1;

  }

  else if (param_3 == 2) {

    *(undefined4 *)(iVar1 + 0x40) = param_1;

  }

  else if (param_3 == 3) {

    *(undefined4 *)(iVar1 + 0x48) = param_1;

  }

  return 0;

}
