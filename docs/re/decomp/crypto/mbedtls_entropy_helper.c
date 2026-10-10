/**
 * mbedtls_entropy_helper @ 0x0308c764
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 mbedtls_entropy_helper(int param_1,undefined1 *param_2)



{

  undefined4 uVar1;

  

  *param_2 = *(undefined1 *)(param_1 + 0x3c);

  param_2[8] = *(undefined1 *)(param_1 + 0x4c);

  *(undefined4 *)(param_2 + 4) = *(undefined4 *)(param_1 + 0x40);

  *(undefined4 *)(param_2 + 0xc) = *(undefined4 *)(param_1 + 0x20);

  *(undefined4 *)(param_2 + 0x10) = *(undefined4 *)(param_1 + 0x14);

  uVar1 = *(undefined4 *)(param_1 + 0x2c);

  *(undefined4 *)(param_2 + 0x18) = *(undefined4 *)(param_1 + 0x28);

  *(undefined4 *)(param_2 + 0x1c) = uVar1;

  uVar1 = *(undefined4 *)(param_1 + 0x34);

  *(undefined4 *)(param_2 + 0x20) = *(undefined4 *)(param_1 + 0x30);

  *(undefined4 *)(param_2 + 0x24) = uVar1;

  *(undefined4 *)(param_2 + 0x28) = *(undefined4 *)(param_1 + 0x18c);

  *(undefined4 *)(param_2 + 0x2c) = *(undefined4 *)(param_1 + 0xd8);

  *(int *)(param_2 + 0x30) = *(int *)(param_1 + 0x44) << 3;

  *(undefined2 *)(param_2 + 0x34) = *(undefined2 *)(param_1 + 0x56);

  *(undefined2 *)(param_2 + 0x36) = *(undefined2 *)(param_1 + 0x58);

  *(undefined2 *)(param_2 + 0x38) = *(undefined2 *)(param_1 + 0x5a);

  *(undefined4 *)(param_2 + 0x4c) = *(undefined4 *)(param_1 + 0x5c);

  *(undefined4 *)(param_2 + 0x3c) = *(undefined4 *)(param_1 + 0x60);

  *(undefined2 *)(param_2 + 0x40) = *(undefined2 *)(param_1 + 100);

  *(undefined2 *)(param_2 + 0x42) = *(undefined2 *)(param_1 + 0x66);

  uVar1 = *(undefined4 *)(param_1 + 0x6c);

  *(undefined4 *)(param_2 + 0x44) = *(undefined4 *)(param_1 + 0x68);

  *(undefined4 *)(param_2 + 0x48) = uVar1;

  return 0;

}
