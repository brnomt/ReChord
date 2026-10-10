/**
 * hifi_file_sync @ 0x0306cce4
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 hifi_file_sync(void)



{

  *DAT_0306ce50 = 0;

  hifi_file_close_helper(0x1b,0x306cba9);

  FUN_0306c878(0x1b);

  FUN_0306c772(0x1b);

  *DAT_03073ba4 = *DAT_03073ba4 | 4;

  return 0;

}
