/**
 * flash_status_get @ 0x03019288
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 flash_status_get(void)



{

  undefined4 *puVar1;

  int iVar2;

  

  if (*(byte *)(DAT_0301963c + 0x12) < *(byte *)(DAT_0301963c + 0x1e)) {

    *(byte *)(DAT_0301963c + 0x12) = *(byte *)(DAT_0301963c + 0x12) + 1;

  }

  puVar1 = DAT_0301964c;

  iVar2 = FUN_02ff2834(*DAT_0301964c,DAT_03019648);

  if (iVar2 == 1) {

    flash_erase_helper(0xff,0);

  }

  iVar2 = FUN_02ff2834(*puVar1,DAT_03019650);

  if (iVar2 == 1) {

    FUN_0302da66(0xb6,0);

  }

  func_0x030037a8(0x33);

  return 1;

}
