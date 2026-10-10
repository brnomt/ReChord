/**
 * SNAND_Write @ 0x03019246
 * SDK-matched (structural+combined, documented in MULTI-VERSION-PLAN §10).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Control flow encountered bad instruction data */

/* WARNING: Possible PIC construction at 0x03019274: Changing call to branch */

/* WARNING: Removing unreachable block (ram,0x03019278) */



void SNAND_Write(void)



{

  int iVar1;

  int iVar2;

  uint uVar3;

  

  iVar1 = DAT_0301963c;

  *DAT_03019638 = (uint)*(byte *)(DAT_0301963c + 0x74);

  iVar2 = DAT_03019640;

  uVar3 = 0;

  do {

    *(int *)(iVar2 + uVar3 * 4) = (int)*(short *)(iVar1 + uVar3 * 2 + 0x77);

    uVar3 = uVar3 + 1;

  } while (uVar3 < 5);

  *DAT_03019644 = 0;

  FUN_02ffcf3a();

                    /* WARNING: Bad instruction - Truncating control flow here */

  halt_baddata();

}
