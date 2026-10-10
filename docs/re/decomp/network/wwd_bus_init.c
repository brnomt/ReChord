/**
 * wwd_bus_init @ 0x0300c65a
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void wwd_bus_init(void)



{

  int iVar1;

  int *piVar2;

  int iVar3;

  ushort *puVar4;

  undefined2 local_20;

  undefined2 local_1e;

  undefined2 local_1c;

  undefined2 local_1a;

  

  piVar2 = DAT_0300c828;

  iVar1 = DAT_0300c824;

  puVar4 = *(ushort **)(DAT_0300c824 + 0xc);

  local_20 = 6;

  local_1e = 0x22;

  local_1c = 0xed;

  local_1a = 0x140;

  *DAT_0300c828 = (uint)*puVar4 + *DAT_0300c828;

  *puVar4 = 0;

  FUN_02fe755c(puVar4 + 1,0x50);

  iVar3 = wwd_bus_sdio_set_oob_interrupt(puVar4 + 1,piVar2[1],&local_20,(int)*DAT_0300c82c);

  if (iVar3 == 0) {

    puVar4[1] = 0;

    *puVar4 = 0;

  }

  else {

    *puVar4 = (ushort)iVar3;

    piVar2[1] = iVar3 + piVar2[1];

  }

  *(undefined4 *)(iVar1 + 0xc) = *(undefined4 *)(puVar4 + 0x54);

  return;

}
