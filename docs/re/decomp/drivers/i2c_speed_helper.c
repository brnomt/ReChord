/**
 * i2c_speed_helper @ 0x0301b2a8
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void i2c_speed_helper(void)



{

  *DAT_0301b2e4 = 1;

  *DAT_0301b2e8 = 0;

  *DAT_0301b2ec = 0;

  *DAT_0301b2f0 = 0;

  *DAT_0301b2f4 = 0;

  *DAT_0301b2f8 = 0;

  *DAT_0301b2fc = 0;

  DAT_0301b2fc[-4] = 1;

  return;

}
