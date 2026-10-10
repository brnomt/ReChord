/**
 * SdioDev_SetBlockSize @ 0x030c756e
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void SdioDev_SetBlockSize(undefined4 *param_1,undefined4 param_2)



{

  SDIO_FBR_BASE(param_1[1]);

  SDIO_FBR_BASE(param_1[3]);

  *param_1 = 0;

  param_1[1] = 0;

  param_1[2] = 0;

  param_1[3] = 0;

  param_1[4] = 0;

  param_1[7] = 0xffffffff;

  param_1[5] = 0;

  param_1[8] = 0;

  param_1[9] = 0;

  param_1[10] = 0;

  param_1[0xd] = 0;

  param_1[0xb] = 0;

  param_1[0xc] = 0;

  param_1[0xe] = 0;

  param_1[0xf] = 0;

  param_1[0x10] = 0;

  param_1[0x11] = 0;

  param_1[6] = param_2;

  param_1[0x12] = 0;

  return;

}
