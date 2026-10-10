/**
 * fw_segment_helper_b @ 0x030e6dcc
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 fw_segment_helper_b(undefined4 param_1,int param_2,int param_3)



{

  int iVar1;

  uint uVar2;

  int iVar3;

  uint local_18;

  

  local_18 = 0;

  buffered_fseek_ovl_0e64(param_1,4,1);

  uVar2 = buffered_fread_ovl_0e65(&local_18,1,4,param_1);

  iVar1 = DAT_030e7148;

  if (uVar2 < 4) {

    return 0xfffffffb;

  }

  local_18 = local_18 << 0x18 | (local_18 >> 8 & 0xff) << 0x10 | (local_18 >> 0x10 & 0xff) << 8 |

             local_18 >> 0x18;

  if (*(int *)(DAT_030e7148 + 0x18) == 0) {

    *(uint *)(param_2 + 0x88) = local_18;

    iVar3 = BackupSysInformation(local_18 * 0xc);

    *(int *)(iVar1 + 0x24) = iVar3;

    if (iVar3 == 0) {

      return 0xfffffff9;

    }

    uVar2 = buffered_fread_ovl_0e65(iVar3,1,local_18 * 0xc,param_1);

    if (uVar2 < local_18 * 0xc) {

      return 0xfffffffb;

    }

  }

  else if (*(int *)(DAT_030e7148 + 0x18) == 1) {

    *(uint *)(param_2 + 0x8c) = local_18;

    iVar3 = BackupSysInformation(local_18 * 0xc);

    *(int *)(iVar1 + 0x28) = iVar3;

    if (iVar3 == 0) {

      return 0xfffffff9;

    }

    uVar2 = buffered_fread_ovl_0e65(iVar3,1,local_18 * 0xc,param_1);

    if (uVar2 < local_18 * 0xc) {

      return 0xfffffffb;

    }

  }

  else {

    buffered_fseek_ovl_0e64(param_1,param_3 + -0x10,1);

  }

  return 0;

}
