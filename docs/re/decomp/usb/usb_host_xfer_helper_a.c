/**
 * usb_host_xfer_helper_a @ 0x030e6652
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4

usb_host_xfer_helper_a(int param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4)



{

  undefined4 local_10;

  

  local_10 = param_4;

  buffered_fseek_ovl_0e64(*param_2,(param_1 - param_2[2]) * 4 + -4,1);

  param_2[2] = param_1;

  buffered_fread_ovl_0e65(&local_10,1,4,*param_2);

  return local_10;

}
