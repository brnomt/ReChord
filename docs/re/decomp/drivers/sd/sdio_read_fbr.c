/**
 * sdio_read_fbr @ 0x030c75ac
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 sdio_read_fbr(undefined4 *param_1)



{

  if (param_1 != (undefined4 *)0x0) {

    SDIO_FBR_BASE(*param_1);

    memset_byte(param_1,0,0x20);

  }

  return 0;

}
