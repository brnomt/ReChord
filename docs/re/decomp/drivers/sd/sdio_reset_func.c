/**
 * sdio_reset_func @ 0x030c6c66
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void sdio_reset_func(int param_1)



{

  SDIO_FBR_BASE(*(undefined4 *)(param_1 + 8));

  *(undefined4 *)(param_1 + 8) = 0;

  *(undefined4 *)(param_1 + 4) = 0;

  *(undefined4 *)(param_1 + 0xc) = 0;

  *(undefined4 *)(param_1 + 0x10) = 0;

  *(undefined4 *)(param_1 + 0x14) = 0;

  *(undefined4 *)(param_1 + 0x18) = 0;

  return;

}
