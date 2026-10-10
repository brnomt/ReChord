/**
 * usb_host_xfer_helper_b @ 0x030e7e94
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint usb_host_xfer_helper_b(undefined4 param_1,int param_2,uint param_3)



{

  uint uVar1;

  uint uVar2;

  uint uVar3;

  uint *puVar4;

  uint uVar5;

  uint uVar6;

  uint uVar7;

  

  uVar2 = 1;

  uVar1 = *(uint *)(param_2 + 0xa8);

  if (uVar1 == 0) {

    uVar1 = *(uint *)(param_2 + 0xb4);

  }

  if (uVar1 < param_3) {

    return *(int *)(param_2 + 0x78) + 1;

  }

  puVar4 = *(uint **)(DAT_030e7f40 + 0x38);

  uVar5 = *puVar4;

  uVar1 = puVar4[1];

  uVar5 = uVar5 << 0x18 | (uVar5 >> 8 & 0xff) << 0x10 | (uVar5 >> 0x10 & 0xff) << 8 | uVar5 >> 0x18;

  uVar6 = uVar1 << 0x18 | (uVar1 >> 8 & 0xff) << 0x10 | (uVar1 >> 0x10 & 0xff) << 8 | uVar1 >> 0x18;

  uVar1 = uVar5 * uVar6;

  if (uVar1 < param_3) {

    do {

      if (*(uint *)(param_2 + 0x90) <= uVar2) break;

      uVar2 = uVar2 + 1;

      uVar6 = puVar4[uVar2 * 2 + -2];

      uVar7 = puVar4[uVar2 * 2 + -1];

      uVar3 = uVar6 << 0x18 | (uVar6 >> 8 & 0xff) << 0x10 | (uVar6 >> 0x10 & 0xff) << 8 |

              uVar6 >> 0x18;

      uVar6 = uVar7 << 0x18 | (uVar7 >> 8 & 0xff) << 0x10 | (uVar7 >> 0x10 & 0xff) << 8 |

              uVar7 >> 0x18;

      uVar1 = uVar3 * uVar6 + uVar1;

      uVar5 = uVar5 + uVar3;

    } while (uVar1 < param_3);

  }

  uVar1 = (uVar5 - (uVar1 - param_3) / uVar6) + 1;

  if (*(uint *)(param_2 + 0x78) < uVar1) {

    uVar1 = *(uint *)(param_2 + 0x78);

  }

  return uVar1;

}
