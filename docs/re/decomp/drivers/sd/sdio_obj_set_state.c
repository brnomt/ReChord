/**
 * sdio_obj_set_state @ 0x030cb4dc
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void sdio_obj_set_state(int *param_1,undefined4 param_2,int param_3,int param_4)



{

  if (*param_1 == -1) {

    return;

  }

  (*(code *)param_1[0x25])();

  param_1[2] = param_3;

  param_1[3] = param_4;

  sdio_reset_func(param_1[6]);

  return;

}
