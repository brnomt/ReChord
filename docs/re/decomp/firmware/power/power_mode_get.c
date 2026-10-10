/**
 * power_mode_get @ 0x03027568
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


char power_mode_get(void)



{

  int iVar1;

  char cVar2;

  int iVar3;

  undefined1 *puVar4;

  

  cVar2 = PmuSetPowerMode();

  if (cVar2 == -1) {

    cVar2 = PmuSetPowerMode();

    iVar1 = DAT_03027874;

    if (cVar2 != '\0') {

      iVar3 = *(int *)(DAT_03027874 + 0x5c);

      *(char *)(iVar3 + -1) = cVar2;

      puVar4 = (undefined1 *)(iVar3 + -2);

      *(undefined1 **)(iVar1 + 0x5c) = puVar4;

      *puVar4 = 0xff;

    }

    cVar2 = -1;

  }

  return cVar2;

}
