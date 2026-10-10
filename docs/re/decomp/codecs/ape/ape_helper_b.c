/**
 * ape_helper_b @ 0x030f7928
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 ape_helper_b(undefined4 param_1,uint *param_2)



{

  byte bVar1;

  undefined4 *puVar2;

  undefined4 *puVar3;

  undefined4 *puVar4;

  int iVar5;

  uint uVar6;

  bool bVar7;

  char local_28;

  byte local_27;

  byte local_26;

  byte local_25;

  byte local_20;

  byte local_1f;

  ushort local_1e;

  

  puVar3 = DAT_030f7afc;

  puVar2 = DAT_030f7af8;

  if (DAT_030f7af8[5] == 0) {

    uVar6 = (*(code *)*DAT_030f7afc)(param_1,DAT_030f7af8[9],*DAT_030f7af8);

    *param_2 = uVar6;

    puVar2[0xb] = puVar2[0xb] + 1;

    if (*param_2 != 0) {

      return 1;

    }

  }

  else {

    iVar5 = (*(code *)*DAT_030f7afc)(&local_28,0xc);

    puVar4 = DAT_030f7b00;

    if (iVar5 == 0xc) {

      uVar6 = 0xc;

      do {

        if (local_28 == 'D') {

          uVar6 = (uint)local_27;

        }

        if (local_28 == 'D' && uVar6 == 0x53) {

          uVar6 = (uint)local_26;

          bVar7 = uVar6 == 0x54;

          if (bVar7) {

            uVar6 = (uint)local_25;

          }

          if (bVar7 && uVar6 == 0x46) {

            uVar6 = (*(code *)*puVar3)(param_1,(local_1e & 0xff) << 8 | (uint)(local_1e >> 8) |

                                               (uint)local_1f << 0x10 | (uint)local_20 << 0x18,

                                       *puVar2);

            if (uVar6 != ((local_1e & 0xff) << 8 | (uint)(local_1e >> 8) | (uint)local_1f << 0x10 |

                         (uint)local_20 << 0x18)) {

              return 0;

            }

            *param_2 = (local_1e & 0xff) << 8 | (uint)(local_1e >> 8) | (uint)local_1f << 0x10 |

                       (uint)local_20 << 0x18;

            (*(code *)*puVar4)(local_1e >> 8 & 1,1,*puVar2);

            return 1;

          }

        }

        if (local_28 == 'D') {

          uVar6 = (uint)local_27;

        }

        if (local_28 != 'D' || uVar6 != 0x53) {

          return 0;

        }

        bVar1 = local_26;

        if (local_26 == 0x54) {

          bVar1 = local_25;

        }

        if (local_26 != 0x54 || bVar1 != 0x43) {

          return 0;

        }

        iVar5 = (*(code *)*puVar3)(&local_28,0xc,*puVar2);

        if (iVar5 == 0xc) {

          (*(code *)*puVar4)((local_1e & 0xff) << 8 | (uint)(local_1e >> 8) | (uint)local_1f << 0x10

                             | (uint)local_20 << 0x18,1,*puVar2);

          (*(code *)*puVar4)(local_1e >> 8 & 1,1,*puVar2);

        }

        uVar6 = (*(code *)*puVar3)(&local_28,0xc,*puVar2);

      } while (uVar6 == 0xc);

    }

  }

  return 0;

}
