/**
 * sdio_read_cis @ 0x030c6c0a
 * Baseline-1.2.5 match (SDK callee-overlap validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int sdio_read_cis(undefined4 *param_1)



{

  int iVar1;

  uint uVar2;

  int iVar3;

  int iVar4;

  undefined1 auStack_28 [24];

  

  iVar4 = 0;

  cis_tpl_parse(auStack_28,*param_1);

  iVar1 = sbuf_byte_at(auStack_28,0x1a);

  iVar3 = 0;

  if (0 < iVar1) {

    do {

      uVar2 = sbuf_byte_at(auStack_28,iVar3 + 0x1b);

      if (uVar2 < 0xff) {

        iVar4 = iVar4 + 1;

      }

      iVar3 = iVar3 + 1;

    } while (iVar3 < iVar1);

  }

  return iVar4;

}
