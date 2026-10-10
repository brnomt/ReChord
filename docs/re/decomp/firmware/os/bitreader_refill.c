/**
 * bitreader_refill @ 0x0301e760
 * Tags: os, bitstream
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void bitreader_refill(undefined4 *param_1,int param_2,undefined4 param_3,undefined4 param_4)



{

  int iVar1;

  uint *puVar2;

  uint uVar3;

  int iVar4;

  undefined4 uStack_10;

  

  iVar1 = param_1[3];

  param_1[3] = iVar1 + param_2;

  if (0x1f < (uint)(iVar1 + param_2)) {

    *param_1 = param_1[1];

    if (((uint)param_1[6] < 5) && (param_1[8] != 1)) {

      uStack_10 = param_4;

      if (param_1[6] != 0) {

        rom_dma_or_copy(param_1[5],param_1[4]);

      }

      iVar1 = rkos_queue_send(param_1[5] + param_1[6],0x2000,&uStack_10);

      param_1[8] = uStack_10;

      if (iVar1 == -1) {

        iVar1 = 0;

      }

      else if (iVar1 == 0) {

        param_1[8] = 1;

      }

      iVar4 = param_1[6];

      param_1[6] = iVar1 + iVar4;

      if ((uint)(iVar1 + iVar4) < 4) {

        param_1[6] = 4;

      }

      param_1[4] = param_1[5];

    }

    puVar2 = (uint *)param_1[4];

    param_1[4] = puVar2 + 1;

    uVar3 = *puVar2;

    param_1[6] = param_1[6] + -4;

    param_1[1] = uVar3 << 0x18 | (uVar3 >> 8 & 0xff) << 0x10 | (uVar3 >> 0x10 & 0xff) << 8 |

                 uVar3 >> 0x18;

    param_1[3] = param_1[3] + -0x20;

    return;

  }

  return;

}
