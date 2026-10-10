/**
 * hifi_file_ioctl @ 0x0306cb84
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 hifi_file_ioctl(void)



{

  hifi_file_close_helper(0x1a,0x306cb17);

  FUN_0306c878(0x1a);

  FUN_0306c772(0x1a);

  *DAT_03073ba4 = *DAT_03073ba4 | 2;

  return 0;

}
